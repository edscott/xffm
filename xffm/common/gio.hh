#ifndef XF_GIO_HH
#define XF_GIO_HH

# define MODE_RM                1 // If directories not empty, does a thread_runReap to rm.
# define MODE_SHRED             2
# define MODE_TRASH             3
// These are not good for directories...
# define MODE_COPY              4  // cannot handle recursive copy of directories 
# define MODE_MOVE              5  // if different device, does copy/delete with directories caveat
// These are too simple...
# define MODE_LINK              6
# define MODE_NEW               7   
# define MODE_MKDIR             8
// This is already done otherwise (check and see if advantage in replacement, for freeBSD)
# define MODE_MOUNT             9     
# define MODE_UMOUNT            10      

gint asyncReference = 0;
namespace xf {

template <class Type> class TimeoutResponse;
template <class Type> class BaseProgressResponse;
template <class Type> 
class Gio {
public:
    static gboolean 
    execute(GtkDialog *rmDialog, const gchar *path, gint mode){
        GList *list = g_list_prepend(NULL,(void *)path);
        auto retval = multiDoIt(rmDialog, list,mode);
        g_list_free(list);
        return retval;
    }
    static gboolean 
    execute(GtkDialog *rmDialog,  GList *list, gint mode){
        auto retval = multiDoIt(rmDialog, list,mode);
        return retval;
    }
   
    static gboolean 
    execute(const gchar *path, const gchar *target, gint mode){
        DBG("***FIXME enable a standalone progress dialog\n");
        GList *list = g_list_prepend(NULL,(void *)path);
        auto retval = multiDoIt(list,target,mode);
        g_list_free(list);
        return retval;
    }
   
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
            DBG("Gio::execute: source and target are the same\n");
            return FALSE;
        }
	g_free(source);

        GList *fileList = removeUriFormat(files);
        multiDoIt(fileList, target, mode);

        for (auto l=fileList; l && l->data; l= l->next) g_free(l->data);
        g_list_free(fileList);
        return result;
    }
    
private:
    static void
    backup(const gchar *path, const gchar *target){
       auto pid=fork();
       if (pid){
           gint status;
           waitpid(pid, &status, 0);
           return;
       }
       auto base = g_path_get_basename(path);
       auto srcTarget = g_strconcat(target, G_DIR_SEPARATOR_S, base, NULL);
       auto backup = g_strconcat(srcTarget, "~", NULL);
       const gchar *arg[] = {
            "mv",
            "-f",
            srcTarget,
            backup,
            NULL
        };
       DBG("backup: %s -> %s\n", srcTarget, backup); 
        execvp(arg[0], (gchar * const *)arg);
        _exit(123);
    }

    static void
    link(const gchar *path, const gchar *target){
        backup(path, target);
        const gchar *arg[] = {
            "ln",
            "-s",
            "-f",
            path,
            target,
            NULL
        };
        Run<Type>::thread_runReap(NULL, arg, 
                Run<Type>::run_operate_stdout, 
                Run<Type>::run_operate_stderr, 
                NULL);

    }

    static void 
    move(const gchar *path, const gchar *target){
        backup(path, target);
        const gchar *arg[] = {
            "mv",
            "-f",
            path,
            target,
            NULL
        };
        Run<Type>::thread_runReap(NULL, arg, 
                Run<Type>::run_operate_stdout, 
                Run<Type>::run_operate_stderr, 
                NULL);
    }   
    
    static void
    copy(const gchar *path, const gchar *target){
        backup(path, target);
        const gchar *arg[] = {
            "cp",
            "-R",
            "-f",
            path,
            target,
            NULL
        };
        Run<Type>::thread_runReap(NULL, arg, 
                Run<Type>::run_operate_stdout, 
                Run<Type>::run_operate_stderr, 
                NULL);

    }   
    static void
    GNUrm(const gchar *path){
#ifdef FREEBSD_FOUND
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
        Run<Type>::thread_runReap(NULL, arg, 
                Run<Type>::run_operate_stdout, 
                Run<Type>::run_operate_stderr, 
                NULL);

    }


    static void
    GNUshred(const gchar *path){
#ifdef FREEBSD_FOUND
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
        Run<Type>::thread_runReap(NULL, arg, 
                Run<Type>::run_operate_stdout, 
                Run<Type>::run_operate_stderr, 
                                NULL);
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
    
    
    static gboolean doIt(const gchar *path, const gchar *target, gint mode){
        if (mode != MODE_COPY && mode != MODE_LINK && mode != MODE_MOVE) 
	    return FALSE;
        GFile *file = g_file_new_for_path(path);
        GError *error=NULL;
        gboolean retval=TRUE;
        switch (mode) {
            case MODE_COPY:
               if (g_file_test(path, G_FILE_TEST_IS_DIR)){
                   copy(path,target);
                   // g_file_copy is limited with respect to directories.
               } 
               else 
               {
                    auto flags = (guint)G_FILE_COPY_OVERWRITE | (guint)G_FILE_COPY_BACKUP | (guint) G_FILE_COPY_NOFOLLOW_SYMLINKS;
                    GFile *tgt = getTargetGfile(path, target);
                    asyncReference++;    
                    auto arg = (void **)calloc(2, sizeof(void *));
                    arg[0] = GINT_TO_POINTER(mode);
                    arg[1] = (void *)g_strdup(path);
                    g_file_copy_async (file, tgt, (GFileCopyFlags) flags,
                        G_PRIORITY_HIGH,
                        NULL, // GCancellable *cancellable,
                        progressCallback, // GFileProgressCallback,
                        (void *)arg, // progress callback data
                        asyncCallback, // GAsyncReadyCallback
                        (void *)arg); // user_data for ready callback
                    g_object_unref(tgt);
               }
               break;
            case MODE_MOVE:
                   move(path,target);
                   // There is currently no g_file_move_async() in documentation (jan2019)
               break;
            case MODE_LINK:
            {   
               // link() has backup option 
               // link(path, target);
               // g_file_make_symbolic_link does not have backup option
               GFile *link = getTargetGfile(path, target);

               retval = g_file_make_symbolic_link (link, 
                           path,
                           NULL,
                        &error);
               g_object_unref(link);
               break;
            }
        }
        if (error){
            // only applicable to MODE_LINK
            gchar *m = g_strdup_printf("%s --> \"%s\"", _("Create symbolic link"), path);
            gchar *message = g_strdup_printf("<span color=\"red\">%s</span>\n(%s)", m, error->message);
            TimeoutResponse<Type>::dialog(GTK_WINDOW(mainWindow), message, "dialog-error");
            g_free(m);
            g_free(message);
            TRACE("doIt(%s): %s\n", path, error->message);
            g_error_free(error);
        }
        return retval;
    }
    
    static gboolean doIt(GtkDialog *rmDialog, const gchar *path, gint mode){
        if (mode != MODE_RM && mode != MODE_TRASH && mode != MODE_SHRED) 
	    return FALSE;
        GFile *file = g_file_new_for_path(path);
        switch (mode) {
            case MODE_RM:
               if (g_file_test(path, G_FILE_TEST_IS_DIR)){
		   GNUrm(path);
               } else {
                   auto arg = (void **)calloc(2, sizeof(void *));
                   arg[0] = GINT_TO_POINTER(mode);
                   arg[1] = (void *)g_strdup(path);
                   asyncReference++;
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

    static gboolean
    multiDoIt(GList *fileList, const gchar *target, gint mode)
    {
        if (mode != MODE_COPY && mode != MODE_LINK && mode != MODE_MOVE) 
	    return FALSE;
	gint items = g_list_length(fileList);
	if (!items) return FALSE;

	gint count = 0;
        gboolean retval;
        for (auto l = fileList; l && l->data; l=l->next) {
	    auto path = (const gchar *)l->data;
            doIt(path, target, mode);
	    count++;
	}
	return retval;
    }


    static gboolean
    multiDoIt(GtkDialog *rmDialog, GList *fileList, gint mode)
    {
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
        DBG("progress %s: %ld/%ld\n", path, (long)currentBytes, (long)totalBytes);
        if (currentBytes == totalBytes) {
            DBG("progress %s %ld/%ld: complete\n", path, (long)currentBytes, (long)totalBytes);
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
            DBG("Success: process \"%s\" in mode %d\n", path, mode);
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
