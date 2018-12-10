#ifndef XF_COMMANDRESPONSE_HH
#define XF_COMMANDRESPONSE_HH
namespace xf {
template <class Type>
class BaseProgressResponse {
    using pixbuf_c = Pixbuf<double>;
    using gtk_c = Gtk<double>;
    using util_c = Util<double>;
public:
    static GtkWindow *
    dialog(const gchar *message, const gchar *icon){
         // Create the widgets
         auto dialog = GTK_WINDOW(gtk_window_new (GTK_WINDOW_TOPLEVEL));
         //gtk_window_set_transient_for (dialog,GTK_WINDOW(mainWindow));
         gtk_window_set_type_hint (dialog,GDK_WINDOW_TYPE_HINT_DIALOG);
         auto vbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
         gtk_container_add (GTK_CONTAINER (dialog), GTK_WIDGET(vbox));
         auto label = GTK_LABEL(gtk_label_new (""));
	 auto markup = 
	    g_strdup_printf("   <span color=\"blue\" size=\"larger\"><b>%s</b></span>   ", message);           
         gtk_label_set_markup(label, markup);
         g_free(markup);
         
         // Add the label, and show everything we have added
         if (icon){
            auto pixbuf = Pixbuf<Type>::get_pixbuf(icon, -96);
            if (pixbuf) {
                auto image = gtk_image_new_from_pixbuf(pixbuf);
                if (image) {
                    gtk_box_pack_start(vbox, image, FALSE, FALSE,0);
                    gtk_widget_show (image);
                }
            }
         }
         gtk_box_pack_start(vbox, GTK_WIDGET(label), FALSE, FALSE,0);
         auto progress = GTK_PROGRESS_BAR(gtk_progress_bar_new());
	 g_object_set_data(G_OBJECT(dialog), "progress", progress);

         gtk_box_pack_start(vbox, GTK_WIDGET(progress), FALSE, FALSE,0);


	 gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
         return dialog;
    }
};
template <class Type>
class CommandProgressResponse {
    using pixbuf_c = Pixbuf<double>;
    using gtk_c = Gtk<double>;
    using util_c = Util<double>;

public:
    // FIXME: operation is too fast to show dialog, operations are
    //        all queued to background, so the dialog is superfluos
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
                if (line[0] != '\n') DBG("CommandProgressResponse:: %s", line);
	    }
	    pclose (pipe);
            */
	    count++;
	}
	gtk_widget_destroy(GTK_WIDGET(dialog));
	return dialog;
    }

};

template <class Type>
class CommandResponse {
    using pixbuf_c = Pixbuf<double>;
    using gtk_c = Gtk<double>;
    using util_c = Util<double>;
public:
    
    static GtkWindow *
    dialog(const gchar *message, const gchar *icon, const gchar **arg)
    {
        if (!arg || arg[0] == NULL) {
	    return NULL ;
	}
	auto dialog = BaseProgressResponse<Type>::dialog(message, icon);
	auto progress = GTK_PROGRESS_BAR(g_object_get_data(G_OBJECT(dialog), "progress"));

        gtk_window_set_title(dialog, _("Running"));
        /*auto text = g_strdup_printf("%s (pid: %d)", 
                 _("Waiting for operation to finish..."),
                 Tubo<Type>::getChild (pid));*/

        auto text = g_strdup(_("Waiting for operation to finish..."));
	gtk_progress_bar_set_text (progress, text);
	g_free(text);
	gtk_progress_bar_set_show_text (progress, TRUE);
        gtk_progress_bar_pulse(progress);
	 
	gtk_widget_show_all (GTK_WIDGET(dialog));
	pid_t controller = Run<Type>::thread_run(
		NULL, //(void *)dialog, // data to fork_finished_function
		arg,
		Run<Type>::run_operate_stdout,
		Run<Type>::run_operate_stderr,
		NULL); //commandDone);
        auto arg2 = (void **)calloc(3, sizeof (void *));
        arg2[0]=(void *)progress;
        arg2[1]=(void *)dialog;
        arg2[2]=GINT_TO_POINTER(Tubo<Type>::getChild(controller));
        g_timeout_add(250, pulse_f, (void *)arg2);
	return dialog;
	
    }

   /* static void commandDone(void *data){
       auto dialog = GTK_WIDGET(data);
       gtk_widget_hide(dialog);
       gtk_widget_destroy(dialog);
       return;
    }*/
       

    static gboolean
    isPidAlive(pid_t pid){
        // pid active?
        gchar *c = g_strdup_printf("ps -p %d", pid); 
        gchar *s = g_strdup_printf("%d", pid);
        gboolean alive = FALSE;
        FILE *pipe = popen(c, "r");
        if (pipe) {
            gchar buffer[256];
            while (fgets(buffer, 255, pipe) && !feof(pipe)){
                buffer[255]=0;
                if (strstr(buffer, s)){
                    // alive
                    alive = TRUE;
                    break;
                }
            }
            pclose(pipe);
        }
	WARN("%s alive=%d\n", c, alive);
        g_free(c);
        g_free(s);
        return (alive);
    }

    static gboolean pulse_f(void *data) {
        auto arg = (void **)data;
        auto progress = GTK_PROGRESS_BAR(arg[0]);
	if (!GTK_IS_PROGRESS_BAR(progress)){
	    return FALSE;
	}
        gint pid = GPOINTER_TO_INT(arg[2]);
	if (!isPidAlive(pid)){
	    auto dialog = GTK_WIDGET(arg[1]);
	    gtk_widget_hide(GTK_WIDGET(dialog));
	    gtk_widget_destroy(GTK_WIDGET(dialog));
	    return FALSE;
	}
        gtk_progress_bar_pulse(progress);
        return TRUE;
    }    


};
}

#endif

