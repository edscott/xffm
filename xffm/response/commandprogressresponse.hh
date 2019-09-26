#if 0
#ifndef XF_COMMANDPROGRESSRESPONSE_HH
#define XF_COMMANDPROGRESSRESPONSE_HH
namespace xf {
template <class Type>

class CommandProgressResponse: 
    public BaseProgressResponse<Type>(const gchar *message, const gchar *icon) 
{
    using pixbuf_c = Pixbuf<double>;
    using gtk_c = Gtk<double>;
    using util_c = Util<double>;

public:
    static GtkWindow *
    CommandProgressResponse(const gchar *message, const gchar *icon, 
	    const gchar *command,
	    GList *fileList,
	    const gchar *target):
        BaseProgressResponse(message, icon);

    {
	gint items = g_list_length(fileList);
	if (!items) return NULL;

	auto dialog = this->dialog;
	auto progress = GTK_PROGRESS_BAR(g_object_get_data(G_OBJECT(dialog), "progress"));

        gtk_window_set_title(dialog, command);
	gtk_widget_show_all (GTK_WIDGET(dialog));

	
	gint count = 0;

        for (auto l = fileList; l && l->data; l=l->next) {
	    gchar *text = g_strdup_printf("%s %d/%d", _("Items:"), count+1, items); 
	    gtk_progress_bar_set_text (progress, text);
	    g_free(text);
	    gtk_progress_bar_set_show_text (progress, TRUE);
	    gtk_progress_bar_set_fraction(progress, (double)count/items);
	    while (gtk_events_pending()) gtk_main_iteration(); 
            gchar **argv;
            gint argc;
	    g_shell_parse_argv (command,
                    &argc,
                    &argv,
                    NULL);
            const gchar *arg[argc+3];
            for (int i=0; i<argc; i++){
                arg[i] = argv[i];
            }
            arg[argc] = (const gchar *)l->data;
            arg[argc+1] = target;
            arg[argc+2] = 0;
            Run<Type>::thread_run(NULL, arg, 
                    Run<Type>::run_operate_stdout, 
                    Run<Type>::run_operate_stderr, 
                    NULL);
            g_strfreev(argv);
	    count++;
	}
	gtk_widget_destroy(GTK_WIDGET(dialog));
	return dialog;
    }

};
}

#endif
#endif

