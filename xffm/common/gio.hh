#ifndef XF_GIO_HH
#define XF_GIO_HH

# define MODE_RM                1 // If directories not empty, does a thread_run to rm.
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
 
namespace xf {
template <class Type> class TimeoutResponse;
template <class Type> class BaseProgressResponse;
template <class Type> 
class Gio {

    static void
    GNUln(const gchar *path, const gchar *target){
        const gchar *arg[] = {
            "ln",
            "-s",
            "-b",
            "-f",
            path,
            target,
            NULL
        };
        Run<Type>::thread_run(NULL, arg, 
                Run<Type>::run_operate_stdout, 
                Run<Type>::run_operate_stderr, 
                NULL);

    }

    static void
    GNUmv(const gchar *path, const gchar *target){
        const gchar *arg[] = {
            "mv",
            "-f",
            "-b",
            path,
            target,
            NULL
        };
        Run<Type>::thread_run(NULL, arg, 
                Run<Type>::run_operate_stdout, 
                Run<Type>::run_operate_stderr, 
                NULL);

    }

    static void
    GNUcp(const gchar *path, const gchar *target){
        const gchar *arg[] = {
            "cp",
            "-R",
            "-b",
            "-f",
            path,
            target,
            NULL
        };
        Run<Type>::thread_run(NULL, arg, 
                Run<Type>::run_operate_stdout, 
                Run<Type>::run_operate_stderr, 
                NULL);

    }

    static void
    GNUrm(const gchar *path){
        const gchar *arg[] = {
            "rm",
            "-f",
            "--one-file-system",
            "--preserve-root",
            "-R",
            (const gchar *)path,
            NULL
        };
        Run<Type>::thread_run(NULL, arg, 
                Run<Type>::run_operate_stdout, 
                Run<Type>::run_operate_stderr, 
                NULL);

    }

    static void
    GNUshred(const gchar *path){
        const gchar *arg[] = {
            "shred",
            "-f",
    //	"-u",
            "-v",
            "-z",
            (const gchar *)path,
            NULL
        };
        Run<Type>::thread_run(NULL, arg, 
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
public:
    static gboolean 
    execute(gchar **files, const gchar *target, gint mode){
        if (!files) {
            WARN("!files\n");
            return FALSE;
        }
        if (*files==NULL) {
            WARN("files==NULL\n");
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
        WARN("execute: source=%s target=%s command=%s\n", source, target, 
                mode==MODE_COPY?"copy":mode==MODE_MOVE?"move":"link");
        gboolean result = FALSE;
        if (strcmp(source, target) ) result = TRUE;
        else {
	    g_free(source);
            WARN("LocalDnd::execute: source and target are the same\n");
            return FALSE;
        }
	g_free(source);

        GList *fileList = removeUriFormat(files);
        multiDoIt(fileList, target, mode);

        for (auto l=fileList; l && l->data; l= l->next) g_free(l->data);
        g_list_free(fileList);
        return result;
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
        gboolean retval;
        switch (mode) {
            case MODE_COPY:
               if (g_file_test(path, G_FILE_TEST_IS_DIR)){
		   GNUcp(path, target);
               } else {
                    auto flags = (guint)G_FILE_COPY_OVERWRITE | (guint)G_FILE_COPY_BACKUP | (guint) G_FILE_COPY_NOFOLLOW_SYMLINKS;
                    GFile *tgt = getTargetGfile(path, target);
                    retval = g_file_copy (file, tgt, (GFileCopyFlags) flags,
                        NULL, // GCancellable *cancellable,
                        NULL,
                        NULL,
                        &error);
                    g_object_unref(tgt);
               }
               break;
            case MODE_MOVE:
               if (g_file_test(path, G_FILE_TEST_IS_DIR)){
		   GNUmv(path, target);
               } else {
                    auto flags = (guint)G_FILE_COPY_OVERWRITE | (guint)G_FILE_COPY_BACKUP | (guint) G_FILE_COPY_NOFOLLOW_SYMLINKS;
                    GFile *tgt = getTargetGfile(path, target);
 
                    retval = g_file_move (file, tgt, (GFileCopyFlags) flags,
                        NULL, // GCancellable *cancellable,
                        NULL,
                        NULL,
                        &error);
                    g_object_unref(tgt);
               }
               break;
            case MODE_LINK:
            {   
               //GNUln(path, target);
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
            gchar *m;
            if (mode == MODE_COPY) 
                m = g_strdup_printf("%s %s", _("Could not copy item:"), path);
            else if (mode == MODE_MOVE) 
                m = g_strdup_printf("%s %s", _("Could not move item:"), path);
            else if (mode == MODE_LINK)
                m = g_strdup_printf("%s --> \"%s\"", _("Create symbolic link"), path);
            gchar *message = g_strdup_printf("<span color=\"red\">%s</span>\n(%s)", m, error->message);
            TimeoutResponse<Type>::dialog(GTK_WINDOW(mainWindow), message, "dialog-error");
            g_free(m);
            g_free(message);
            DBG("doIt(%s): %s\n", path, error->message);
            g_error_free(error);
        }
        return retval;
    }
    
    static gboolean doIt(GtkDialog *rmDialog, const gchar *path, gint mode){
        if (mode != MODE_RM && mode != MODE_TRASH && mode != MODE_SHRED) 
	    return FALSE;
        GFile *file = g_file_new_for_path(path);
        GError *error=NULL;
        gboolean retval;
        switch (mode) {
            case MODE_RM:
               if (g_file_test(path, G_FILE_TEST_IS_DIR)){
		   GNUrm(path);
               } else {
                    retval = g_file_delete (file, NULL, &error);
               }
               break;
            case MODE_TRASH:
               retval = g_file_trash (file, NULL, &error);
               break;
            case MODE_SHRED:
	       GNUshred (path);
               break;
        }
        if (error){
            gchar *m;
            if (mode == MODE_RM) 
                m = g_strdup_printf(_("Could not delete %s"), path);
            else if (mode == MODE_TRASH) 
                m = g_strdup_printf(_("Could not move %s to trash"), path);
            gchar *message = g_strdup_printf("<span color=\"red\">%s</span>\n(%s)", m, error->message);
            TimeoutResponse<Type>::dialog(GTK_WINDOW(rmDialog), message, "dialog-error");
            g_free(m);
            g_free(message);
            DBG("doIt(%s): %s\n", path, error->message);
            g_error_free(error);
        }
        return retval;
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
            // Try first item in foreground.
            if (!count) {
                retval = doIt(path, target, mode);
                if (!retval)break;
            } else {
                // send the rest in background
                GFile *file = g_file_new_for_path(path);
                if (mode == MODE_COPY) {
		    GNUcp(path, target);
                } else if (mode == MODE_LINK){
		    GNUln(path, target);
                }
		else if (mode == MODE_MOVE){
                    GNUmv(path, target);
                }            
	    }
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
            // Try first item in foreground.
            if (!count) {
                retval = doIt(rmDialog, path, mode);
                if (!retval)break;
            } else {
                // send the rest in background
                GFile *file = g_file_new_for_path(path);
                if (mode == MODE_RM) {
                    if (g_file_test(path, G_FILE_TEST_IS_DIR)){
		         GNUrm(path);
                         continue;
                    } else {
                        g_file_delete_async (file, G_PRIORITY_LOW, 
                            NULL,   // GCancellable *cancellable,
                            asyncCallback,
                            GINT_TO_POINTER(mode));
                    }
                } else if (mode == MODE_TRASH){
                    g_file_trash_async (file, G_PRIORITY_LOW, 
                        NULL,   // GCancellable *cancellable,
                        asyncCallback,
                        GINT_TO_POINTER(mode));

                }
		else if (mode == MODE_SHRED){
                    GNUshred(path);
                }            
	    }
	    count++;
	}
	return retval;
    }

    static void
    asyncCallback(GObject *obj,
                        GAsyncResult *res,
                        gpointer data){
        auto file = (GFile *)obj;
        DBG("asyncCallback: mode %d\n", GPOINTER_TO_INT(data));

        gchar errorMsg;
        gboolean success;
        switch (GPOINTER_TO_INT(data)){
            case MODE_RM:
                success = g_file_delete_finish (file, res, NULL);
                break;
            case MODE_TRASH:
                success = g_file_trash_finish (file, res, NULL);
                break;
        }
        if (!success){
            gchar *path = g_file_get_path(file);
            ERROR("Failed to process \"%s\" in mode %d\n", path, GPOINTER_TO_INT(data));
            g_free(path);
        }
        g_object_unref(file);

    }

};

}
#endif
