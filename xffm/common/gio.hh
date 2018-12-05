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

public:
    
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
    multiDoIt(GtkDialog *rmDialog, const gchar *message, const gchar *icon, GList *fileList, gint mode)
    {
        if (mode != MODE_RM && mode != MODE_TRASH && mode != MODE_SHRED)
	    return FALSE;
	gint items = g_list_length(fileList);
	if (!items) return FALSE;

	auto dialog = BaseProgressResponse<Type>::dialog(message, icon);
	auto progress = GTK_PROGRESS_BAR(g_object_get_data(G_OBJECT(dialog), "progress"));

        gtk_window_set_title(dialog, message);
	gtk_widget_show_all (GTK_WIDGET(dialog));

	
	gint count = 0;
        gboolean retval;
        for (auto l = fileList; l && l->data; l=l->next) {
	    gchar *text = g_strdup_printf("%s %d/%d", _("Items:"), count+1, items); 
	    gtk_progress_bar_set_text (progress, text);
	    g_free(text);
	    gtk_progress_bar_set_show_text (progress, TRUE);
	    gtk_progress_bar_set_fraction(progress, (double)count/items);
	    while (gtk_events_pending()) gtk_main_iteration(); 
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
	gtk_widget_destroy(GTK_WIDGET(dialog));
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
