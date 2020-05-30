#ifndef XF_GIO_HH
#define XF_GIO_HH

# define MODE_RM                1 // If directories not empty, does a thread_runReap to rm.
# define MODE_SHRED             2
# define MODE_TRASH             3
// These are not good for directories...
# define MODE_COPY              4  // cannot handle recursive copy of directories 
# define MODE_MOVE              5  // if different device, does copy/delete with directories caveat
# define MODE_RENAME            6  // 
// These are too simple...
# define MODE_LINK              7
# define MODE_NEW               8   
# define MODE_MKDIR             9
// This is already done otherwise (check and see if advantage in replacement, for freeBSD)
# define MODE_MOUNT             10     
# define MODE_UMOUNT            11      

gint asyncReference = 0;
namespace xf {

template <class Type> class Progress;
template <class Type> 
class Gio {
    /*static void *progress(void *data){
	auto arg = (void **)data;
	auto message = (const gchar *)arg[0];
	auto icon = (const gchar *)arg[1];
	auto title = (const gchar *)arg[2];
	auto text = (const gchar *)arg[3];
	// open follow dialog for long commands...
	auto dialog = Progress<Type>::dialogPulse(message, icon, title, text);
	return (void *)dialog;
    }*/

public:
    // rm/shred/trash (by popup)
    static gboolean 
    execute(GtkDialog *rmDialog, const gchar *path, gint mode){
        GList *list = g_list_prepend(NULL,(void *)path);
        auto retval = execute(rmDialog,  list, mode);
        g_list_free(list);
        return retval;
    }
    static gboolean 
    execute(GtkDialog *rmDialog,  GList *list, gint mode){
        auto retval = multiDoIt(rmDialog, list,mode);
        return retval;
    }
private:
    static void *
    createProgressObject(void *data){
        void **arg = (void **)data;
        auto message = (const gchar *)arg[0];
        auto icon = (const gchar *)arg[1];
        auto title = (const gchar *)arg[2];
        auto text =(const gchar *)arg[3];
        auto progress = new(Progress<Type>)(message, icon, title, text);
        return (void *) progress;
    }

    //rename/link/duplicate (by popup)
    // fireup chain of 2 threads.
    // thread 1 will fire up thread 2 and wait
    // thread 2 will do the action
    static void *thread2(void *data){
	gboolean retval=FALSE;
        TRACE("thread2 startup\n");
	auto arg = (void **)data;
	auto list = (GList *)arg[0];
	auto target = (const gchar *)arg[1];
	auto mode = GPOINTER_TO_INT(arg[2]);
	auto progressObject = (Progress<Type> *)arg[3];

        retval = multiDoItFore(list,target,mode,progressObject);
        TRACE("thread2 done\n");
        return GINT_TO_POINTER(retval);
    }
    static void *thread1(void *data){
        TRACE("thread1 startup\n");
	void *retval=NULL;
	pthread_t thread;
	auto arg = (void **)data;
	auto list = (GList *)arg[0];
	auto target = (gchar *)arg[1];
	auto mode = GPOINTER_TO_INT(arg[2]);

	const gchar *title;
	switch (mode) {
	    case MODE_COPY:
		title = _("Copying files"); break;
	    case MODE_MOVE:
		title = _("Moving files"); break;
	    case MODE_LINK:
		title = _("linking"); break;
	    case MODE_RENAME:
		title = _("rename"); break;
	    default:
		title = "Fxime";
	}

        const gchar *contextArg[]={ 
            (const gchar *)list->data, 
	    "system-run", 
            title, 
            ""
        };
	auto progressObject = 
	    (Progress<Type> *)(Util<Type>::context_function(createProgressObject, (void *)contextArg));
	arg[3] = (void *)progressObject;

	if (pthread_create(&thread, NULL, thread2,data) != 0){
	    ERROR("thread1(): Unable to create thread2\n");
	} 
	  
	asyncReference++;
	if (pthread_join(thread, &retval)!=0){
	    ERROR("thread1(): Unable to join thread1\n");
	}
	asyncReference--;

        progressObject->stop(); // This will destroy object.
        for (auto l=list; l && l->data; l= l->next) g_free(l->data);
	g_list_free(list);
	g_free(target);
	g_free(arg);
        TRACE("thread1 done: retval= %p\n", retval);
	//pthread_exit();
        return GINT_TO_POINTER(retval);
    }
public:
    static gboolean 
    execute(const gchar *path, const gchar *target, gint mode){
	TRACE("***execute(%s, %s, %d)\n", path, target, mode);
	pthread_t thread;
	auto arg = (void **)calloc(4,sizeof(void *));
	if (!arg){
	    ERROR("execute(): calloc: %s\n", strerror(errno));
	    exit(1);
	}
        GList *list = g_list_prepend(NULL,(void *)g_strdup(path));
	arg[0] = (void *)list;
	arg[1] = (void *)g_strdup(target);
	arg[2] = GINT_TO_POINTER(mode);
	arg[3] = NULL;
        TRACE("thread1 create\n");
	if (pthread_create(&thread, NULL, thread1, (void *)arg) != 0){
	    ERROR("execute(): Unable to create thread1\n");
	}
        pthread_detach(thread);
        TRACE("return to event loop\n");

	return TRUE;
    }
    //copy/move/symlink (by dnd or clipboard)
    static gboolean 
    executeURL(gchar **files, const gchar *target, gint mode){
        if (!files) {
            ERROR("!files\n");
            return FALSE;
        }
        if (*files==NULL) {
            ERROR("files==NULL\n");
            return FALSE;
        }
        TRACE("*** target=%s, *files%s\n",target, (*files)+strlen(URIFILE));
        if (strcmp(target, (*files)+strlen(URIFILE))==0){
            TRACE("gio.hh:: cannot dnd a file unto itself: %s\n", target);
            return FALSE;
        }
            
        gchar *source = g_path_get_dirname(*files);
	if (strncmp(source, URIFILE, strlen(URIFILE))==0){
	    gchar *g = g_strdup(source + strlen(URIFILE));
	    g_free(source);
	    source=g;
	}
        if (!target){
	    ERROR("LocalDnd::execute: target cannot be NULL\n");
            return FALSE;
        }
        TRACE("execute: source=%s target=%s command=%s\n", source, target, 
                mode==MODE_COPY?"copy":mode==MODE_MOVE?"move":"link");
        gboolean result = FALSE;
        if (strcmp(source, target) ) result = TRUE;
        else {
	    g_free(source);
            TRACE("Gio::execute: source and target are the same\n");
            return FALSE;
        }
	g_free(source);

	// Proceed...
	pthread_t thread;
	auto arg = (void **)calloc(4,sizeof(void *));
	if (!arg){
	    ERROR("execute(): calloc: %s\n", strerror(errno));
	    exit(1);
	}
        auto list = removeUriFormat(files);
	arg[0] = (void *)list;
	arg[1] = (void *)g_strdup(target);
	arg[2] = GINT_TO_POINTER(mode);
        TRACE("dnd thread1 create\n");
	if (pthread_create(&thread, NULL, thread1, (void *)arg) != 0){
	    ERROR("execute(): Unable to create thread1\n");
	}
        pthread_detach(thread);
        TRACE("dnd return to event loop\n");

        return result;
    }
    
private:
    static void 
    fore(const gchar **arg){
 	auto notebookP = Fm<Type>::getCurrentNotebook();
	auto pageP = notebookP->currentPageObject();
	pageP->run_lp_command(pageP->output(), pageP->workDir(), 
		Util<Type>::argv2command(arg));
/*      auto pid=fork();
       if (pid){
           gint status;
           waitpid(pid, &status, 0);
	   TRACE("fore(): wait complete\n");
           return;
       }
        execvp(arg[0], (gchar * const *)arg);
	   TRACE("fore(): execvp failed.\n");
        _exit(123);*/
    }

    static void
    backup(const gchar *path, const gchar *target){
        auto base = g_path_get_basename(path);
        auto srcTarget = g_strconcat(target, G_DIR_SEPARATOR_S, base, NULL);
	g_free(base);
	if (g_file_test(srcTarget, G_FILE_TEST_EXISTS)){
	    auto backup = g_strconcat(srcTarget, "~", NULL);
	    const gchar *arg[] = { "mv", "-f", srcTarget, backup, NULL };
            //const gchar *arg[] = { "mv", "-v", "-f", srcTarget, backup, NULL };
	    TRACE("backup: %s -> %s\n", srcTarget, backup); 
	    fore(arg);
	    g_free(backup);
	}
	g_free(srcTarget);
    }

    static void 
    renameFore(const gchar *path, const gchar *target){
        if (g_file_test(target, G_FILE_TEST_EXISTS)){
            auto backupName = g_strconcat(target, "~", NULL);
            while (g_file_test(backupName, G_FILE_TEST_EXISTS) ){
                auto g = g_strconcat(backupName,"~",NULL);
                g_free(backupName);
                backupName = g;
            }

            TRACE("backup: %s -> %s\n", target, backupName); 
            const gchar *arg[] = { "mv", "-f", target, backupName, NULL };
            fore(arg);
            g_free(backupName);
        }
        TRACE("rename: %s -> %s\n", path, target); 
        const gchar *arg[] = { "mv", "-f", path, target, NULL };
        //const gchar *arg[] = { "mv", "-v", "-f", path, target, NULL };
        fore(arg);
    }   

    static void 
    moveFore(const gchar *path, const gchar *target){
        backup(path, target);
        TRACE("moveFore: %s -> %s\n", path, target); 
        const gchar *arg[] = { "mv", "-f", path, target, NULL };
        //const gchar *arg[] = { "mv", "-v", "-f", path, target, NULL };
        fore(arg);
    }   
    
    static void
    copyFore(const gchar *path, const gchar *target){
        backup(path, target);
        TRACE("copyFore: %s -> %s\n", path, target); 
        const gchar *arg[] = { "cp", "-R", "-f", path, target, NULL };
        //const gchar *arg[] = { "cp", "-v", "-R", "-f", path, target, NULL };
        fore(arg);
     }   

    static void
    linkFore(const gchar *path, const gchar *target){
        backup(path, target);
        TRACE("linkFore: %s -> %s\n", path, target); 
#ifdef BSD_FOUND
        const gchar *arg[] = { "ln", "-s", "-f", path, target, NULL };
#else
        const gchar *arg[] = { "ln", "-v", "-s", "-f", path, target, NULL };
#endif
        fore(arg);
    }

    static void
    GNUrm(const gchar *path){
#ifdef BSD_FOUND
        const gchar *arg[] = {
            "rm",
            "-f",
            "-R",
            (const gchar *)path,
            NULL
        };
#else
        const gchar *arg[] = {
            "rm",
            "-f",
            "--one-file-system",
            "--preserve-root",
            "-R",
            (const gchar *)path,
            NULL
        };
#endif
        fore(arg);
    }


    static void
    GNUshred(const gchar *path){
#ifdef BSD_FOUND
        const gchar *arg[] = {
            "rm",
            "-f",
            "-P",
            "-R",
            (const gchar *)path,
            NULL
        };
#else
        const gchar *arg[] = {
            "shred",
            "-f",
            "-u",
            "-v",
            "-z",
            (const gchar *)path,
            NULL
        };
#endif         
	fore(arg);
    }

    static GList *
    removeUriFormat(gchar **files) {
        GList *fileList = NULL;
        for (auto f=files; f && *f; f++){
            gchar *file = *f;
            if (!strstr(file, URIFILE)) continue;
            if (strlen(file) > strlen(URIFILE)){
                if (strncmp(file, URIFILE, strlen(URIFILE))==0){
                    file = *f + strlen(URIFILE);
                }
            }
            fileList = g_list_prepend(fileList, g_strdup(file));
        }
        fileList = g_list_reverse(fileList);
        return fileList;
    }

    static GFile *
    getTargetGfile(const gchar *path, const gchar *target){
    GFile *tgt;
        if (g_file_test(target, G_FILE_TEST_IS_DIR)) {
            gchar *base = g_path_get_basename(path);
	    gchar *newPath = g_strconcat(target, G_DIR_SEPARATOR_S, base, NULL);
            GFile *tgt = g_file_new_for_path(newPath);
	    g_free(newPath);
            g_free(base);
            return tgt;
        } 
        return g_file_new_for_path(target);
    }
    
    
    static gboolean doItFore(const gchar *path, const gchar *target, gint mode){
	TRACE("doItFore...%s --> %s  (%d)\n", path, target, mode);
        if (mode != MODE_COPY && mode != MODE_LINK && mode != MODE_MOVE && mode != MODE_RENAME) 
	    return FALSE;
        gboolean retval=TRUE;
	if (!path || !target) return FALSE; // should not happen.
	    TRACE("%s .. %s \n", path, target);
	
	if (strcmp(path, target)==0){
	    // check if they are the same 
	    struct stat stSrc;
	    struct stat stTgt;
	    stat(path, &stSrc);
	    stat(target, &stTgt);
	    if (memcmp(&stSrc, &stTgt, sizeof(struct stat)) ==0){
		auto message = g_strdup_printf("<span size=\"larger\" color=\"blue\">%s\n<span color=\"red\">%s:\n</span></span><span size=\"larger\"> %s</span>\n",
			strerror(EFAULT), strerror(EEXIST), target);
		Dialogs<Type>::quickHelp(mainWindow, message, "dialog-error");
		g_free(message);
		return FALSE;
	    } 
	}
	if (g_file_test(target, G_FILE_TEST_EXISTS)){
	    // Overwrite? Backup will be created.
	    auto message = g_strdup_printf("<span size=\"larger\"><span color=\"blue\">%s:</span>\n%s\n<span color=\"red\">%s</span></span>\n", 
		    strerror(EEXIST),target,
		    _("Overwrite?")); 
	    	
	    auto yesNo = Dialogs<Type>::yesNo(message);
	    g_free(message);
	    auto response = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(yesNo), "response"));
	    gtk_widget_destroy(GTK_WIDGET(yesNo));
	    TRACE("response=%d\n", response);
	    if(response != 1){
		TRACE("No Overwrite\n");
		return FALSE;
	    }
	    TRACE("Overwrite with backup...\n");
	}
        switch (mode) {
            case MODE_COPY:
               copyFore(path,target);
               break;
            case MODE_MOVE:
               moveFore(path,target);
               break;
            case MODE_RENAME:
               renameFore(path,target);
               break;
            case MODE_LINK:
            {   
               linkFore(path,target);
               break;
            }
        }
        return retval;
    }

    static gboolean doIt(GtkDialog *rmDialog, const gchar *path, gint mode){
	TRACE("doIt...rm %s   (%d)\n", path,  mode);
        if (mode != MODE_RM && mode != MODE_TRASH && mode != MODE_SHRED) 
	    return FALSE;
        GFile *file = g_file_new_for_path(path);
	auto notebookP = Fm<Type>::getCurrentNotebook();
	auto pageP = notebookP->currentPageObject();
	Print<double>::show_text(pageP->output());
	//pageP->run_lp_command(pageP->output(), pageP->workDir(), command);        
	switch (mode) {
            case MODE_RM:
               if (g_file_test(path, G_FILE_TEST_IS_DIR)){
		   GNUrm(path);
               } else {
                   auto arg = (void **)calloc(2, sizeof(void *));
                   arg[0] = GINT_TO_POINTER(mode);
                   arg[1] = (void *)g_strdup(path);
                   asyncReference++;
		   Print<double>::print(pageP->output(), "green", 
			   g_strdup_printf("g_file_delete_async(%s)\n", path));
                   g_file_delete_async (file, G_PRIORITY_LOW, 
                            NULL,   // GCancellable *cancellable,
                            asyncCallback,
                            (void *)arg);
               }
               break;
            case MODE_TRASH:
               {
                    auto arg = (void **)calloc(2, sizeof(void *));
                    arg[0] = GINT_TO_POINTER(mode);
                    arg[1] = (void *)g_strdup(path);
                    asyncReference++;
		    Print<double>::print(pageP->output(), "green", 
			   g_strdup_printf("g_file_trash_async(%s)\n", path));
                    g_file_trash_async (file, G_PRIORITY_HIGH, 
                       NULL, asyncCallback, arg);
               }
               break;
            case MODE_SHRED:
	       GNUshred (path);
               break;
        }
        return TRUE;
    }

    static void *
    setProgressText(void *data){
	auto arg = (void **)data;
	auto progress = (Progress<Type> *) arg[0];
	auto item = GPOINTER_TO_INT(arg[1]);
	auto list = (GList *)arg[2];
	auto text = g_strdup_printf(_("(%d of %d)"), item, g_list_length(list));
	    
	gtk_progress_bar_set_text (progress->progressBar(), text);
	g_free(text);
	return NULL;
    }

    static void *
    setProgressMessage(void *data){
	auto arg = (void **)data;
	auto progress = (Progress<Type> *) arg[0];
	auto label = progress->label();
	auto markup = (const gchar *)arg[1];
	//TRACE("label(%p) %s\n", label, markup);
	gtk_label_set_markup(label, markup);

	    
	return NULL;
    }
	
    static gboolean
    multiDoItFore(GList *fileList, const gchar *target, gint mode, Progress<Type> *progress)
    {
	TRACE("multiDoItFore, mode %d...\n", mode);
        if (mode != MODE_COPY && mode != MODE_LINK && mode != MODE_MOVE && mode != MODE_RENAME) 
	    return FALSE;
	gint items = g_list_length(fileList);
	if (!items) return FALSE;

	gint count = 1;
        gboolean retval;
        for (auto l = fileList; l && l->data; l=l->next) {
	    // update progressBar dialog with filecount
	    void *arg[]={(void *)progress, GINT_TO_POINTER(count), (void *)fileList};
	    Util<Type>::context_function(setProgressText, (void *)arg);
	    auto path = (const gchar *)l->data;
	    // update progress dialog with path
	    void *arg2[]={(void *)progress, (void *)path};
	    Util<Type>::context_function(setProgressMessage, (void *)arg2);
            doItFore(path, target, mode);
	    //sleep(1);
	    count++;
	}
	// Progress dialog will be destroyed at thread1().
	return retval;
    }


    static gboolean
    multiDoIt(GtkDialog *rmDialog, GList *fileList, gint mode)
    {
	TRACE("multiDoIt, mode %d...\n", mode);
        if (mode != MODE_RM && mode != MODE_TRASH && mode != MODE_SHRED)
	    return FALSE;
	gint items = g_list_length(fileList);
	if (!items) return FALSE;

	gint count = 0;
        gboolean retval;
        for (auto l = fileList; l && l->data; l=l->next) {
	    auto path = (const gchar *)l->data;
            retval = doIt(rmDialog, path, mode);
	    count++;
	}
	return retval;
    }
private:    
    static void
    progressCallback(goffset currentBytes, goffset totalBytes, void *data){
        auto arg = (void **)data;
        auto mode = GPOINTER_TO_INT(arg[0]);
        auto path =(gchar *)arg[1];
        TRACE("progress %s: %ld/%ld\n", path, (long)currentBytes, (long)totalBytes);
        if (currentBytes == totalBytes) {
            TRACE("progress %s %ld/%ld: complete\n", path, (long)currentBytes, (long)totalBytes);
        }
    }

    static void
    asyncCallback(GObject *obj,
                        GAsyncResult *res,
                        gpointer data){
        auto file = (GFile *)obj;

        GError *error=NULL;
        gboolean success;
        auto arg = (void **)data;
        auto mode = GPOINTER_TO_INT(arg[0]);
        auto path = (gchar *)arg[1];
        TRACE("asyncCallback: mode %d\n", mode);
        switch (mode){
            case MODE_RM:
                success = g_file_delete_finish (file, res, &error);
                break;
            case MODE_TRASH:
                success = g_file_trash_finish (file, res, &error);
                break;
            case MODE_COPY:
                success = g_file_copy_finish (file, res, &error);
                break;
        }
        if (!success){
            ERROR("Failed to process \"%s\" in mode %d\n", path, mode);
            if (error){
                ERROR("GError message: %s\n", error->message);
                g_error_free(error);
            }
        } else{
            TRACE("Success: process \"%s\" in mode %d\n", path, mode);
        }
        g_free(path);
        g_free(arg);
        g_object_unref(file);
        // decrement async reference
        asyncReference--;
    }

};

}
#endif
