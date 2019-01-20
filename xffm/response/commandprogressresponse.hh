#ifndef XF_COMMANDPROGRESSRESPONSE_HH
#define XF_COMMANDPROGRESSRESPONSE_HH
namespace xf {
template <class Type>

class CommandProgressResponse {
    using pixbuf_c = Pixbuf<double>;
    using gtk_c = Gtk<double>;
    using util_c = Util<double>;

public:
    static GtkWindow *
    dialog(const gchar *message, const gchar *icon, 
	    const gchar *command,
	    GList *fileList,
	    const gchar *target)
    {
	gint items = g_list_length(fileList);
	if (!items) return NULL;

	auto dialog = BaseProgressResponse<Type>::dialog(message, icon);
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
          /*      
	    auto src = (const gchar *)l->data;
	    gchar *text2 = g_strdup_printf("%s \"%s\" \"%s\"",  command, src, target);
	    FILE *pipe = popen (text2, "r");
	    if(pipe == NULL) {
		ERROR("Cannot pipe from \'%s\'\n", text2);
		g_free(text2);
		return NULL;
	    }
	    g_free(text2);
	    gchar line[256];
	    memset(line, 0, 256);
	    while (fgets (line, 255, pipe) && !feof(pipe)) {
                if (line[0] != '\n') TRACE("CommandProgressResponse:: %s", line);
	    }
	    pclose (pipe);
            */
	    count++;
	}
	gtk_widget_destroy(GTK_WIDGET(dialog));
	return dialog;
    }

};
}

#endif

