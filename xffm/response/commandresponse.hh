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
	 g_object_set_data(G_OBJECT(dialog), "label", label);
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
	 g_object_set_data(G_OBJECT(progress), "label", label);

         gtk_box_pack_start(vbox, GTK_WIDGET(progress), FALSE, FALSE,0);


	 gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
         return dialog;
    }
};
template <class Type>
class ProgressDialog {

public:
    static GtkWindow *
    dialog(const gchar *message, const gchar *icon, 
	    const gchar *title, const gchar *text)
    {
	auto dialog = BaseProgressResponse<Type>::dialog(message, icon);
	auto progress = GTK_PROGRESS_BAR(g_object_get_data(G_OBJECT(dialog), "progress"));
	g_object_set_data(G_OBJECT(dialog),"progress", progress);
	g_object_set_data(G_OBJECT(progress),"dialog", dialog);

        gtk_window_set_title(dialog, title?title:_("Running"));
        /*auto text = g_strdup_printf("%s (pid: %d)", 
                 _("Waiting for operation to finish..."),
                 Tubo<Type>::getChild (pid));*/

	gtk_progress_bar_set_text (progress, text?text:_("Waiting for operation to finish..."));
	gtk_progress_bar_set_show_text (progress, TRUE);
        gtk_progress_bar_pulse(progress);
	gtk_widget_realize (GTK_WIDGET(dialog));
	 
	gtk_widget_show_all (GTK_WIDGET(dialog));
        Response<Type>::placeDialog(dialog);
	return dialog;
    }
    static GtkWindow *
    dialogPulse(const gchar *message, const gchar *icon, 
	    const gchar *title, const gchar *text){
	GtkWindow *dialogP = dialog(message, icon,title,text);
        auto arg2 = (void **)calloc(2, sizeof (void *));
        arg2[0]=g_object_get_data(G_OBJECT(dialogP),"progress");
        arg2[1]=(void *)dialogP;
        g_timeout_add(250, simplePulse_f, (void *)arg2);
	return dialogP;
    }

    static gboolean simplePulse_f(void *data) {
        auto arg = (void **)data;
        auto progress = GTK_PROGRESS_BAR(arg[0]);
	auto dialog = GTK_WIDGET(arg[1]);
	if (!GTK_IS_PROGRESS_BAR(progress)){
	    return FALSE;
	}
	if (g_object_get_data(G_OBJECT(dialog), "stop")){
	    gtk_widget_hide(GTK_WIDGET(dialog));
	    gtk_widget_destroy(GTK_WIDGET(dialog));
	    return FALSE;
	}
        gtk_progress_bar_pulse(progress);
        return TRUE;
    }  
};

template <class Type>
class CommandResponse {
    using pixbuf_c = Pixbuf<double>;
    using gtk_c = Gtk<double>;
    using util_c = Util<double>;
public:
    
    static GtkWindow *
    dialog(const gchar *message, const gchar *icon, const gchar *command)
    {
        if (!command) {
	    return NULL ;
	}
	auto dialog = ProgressDialog<Type>::dialog(message, icon, NULL, NULL);
	pid_t controller = Run<Type>::thread_run(Fm<Type>::getCurrentTextview(),command, FALSE);
        
        auto arg2 = (void **)calloc(3, sizeof (void *));
        arg2[0]=g_object_get_data(G_OBJECT(dialog),"progress");
        arg2[1]=(void *)dialog;
        arg2[2]=GINT_TO_POINTER(Tubo<Type>::getChild(controller));
        g_timeout_add(250, pulse_f, (void *)arg2);
	return dialog;
	
    }
    
    static GtkWindow *
    dialog(const gchar *message, const gchar *icon, const gchar **arg)
    {
        if (!arg || arg[0] == NULL) {
	    return NULL ;
	}
	auto dialog = ProgressDialog<Type>::dialog(message, icon, NULL, NULL);
	pid_t controller = Run<Type>::thread_run(
		NULL, //(void *)dialog, // data to fork_finished_function
		arg,
		Run<Type>::run_operate_stdout,
		Run<Type>::run_operate_stderr,
		NULL); //commandDone);
        auto arg2 = (void **)calloc(3, sizeof (void *));
        arg2[0]=g_object_get_data(G_OBJECT(dialog),"progress");
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
	TRACE("%s alive=%d\n", c, alive);
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

