#ifndef XF_COMMANDRESPONSE_HH
#define XF_COMMANDRESPONSE_HH
namespace xf {
    
template <class Type>
class BaseCommandResponse {
    GtkWindow *dialog_;
    GtkLabel *label_;
    GtkProgressBar *progressBar_;
    pid_t controller_;
    
public:
    GtkWindow *dialog(void){return dialog_;}
    GtkLabel *label(void){return label_;}
    GtkProgressBar *progressBar(void){return progressBar_;}
    pid_t controller(void){return controller_;}
    void setController(pid_t value){ controller_ = value;}
    
    BaseCommandResponse(const gchar *message, const gchar *icon){
        // Create the widgets
        dialog_ = GTK_WINDOW(gtk_window_new (GTK_WINDOW_TOPLEVEL));
        //gtk_window_set_transient_for (dialog_,GTK_WINDOW(mainWindow));
        gtk_window_set_type_hint (dialog_,GDK_WINDOW_TYPE_HINT_DIALOG);
        auto vbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        gtk_container_add (GTK_CONTAINER (dialog_), GTK_WIDGET(vbox));
        label_ = GTK_LABEL(gtk_label_new (""));

        auto markup = 
            g_strdup_printf("   <span color=\"blue\" size=\"larger\"><b>%s</b></span>   ", message);           
        gtk_label_set_markup(label_, markup);
        g_free(markup);
         
         // Add the label, and show everything we have added
        if (icon){
            auto pixbuf = Pixbuf<Type>::getPixbuf(icon, -96);
            if (pixbuf) {
                auto image = gtk_image_new_from_pixbuf(pixbuf);
                if (image) {
                    gtk_box_pack_start(vbox, image, FALSE, FALSE,0);
                    gtk_widget_show (image);
                }
            }
        }
        gtk_box_pack_start(vbox, GTK_WIDGET(label_), FALSE, FALSE,0);
        progressBar_ = GTK_PROGRESS_BAR(gtk_progress_bar_new());
        gtk_box_pack_start(vbox, GTK_WIDGET(progressBar_), FALSE, FALSE,0);
        setTitle(NULL);
        setProgressBarText(NULL);
        gtk_widget_realize (GTK_WIDGET(dialog_));
         
        gtk_widget_show_all (GTK_WIDGET(dialog_));
        Dialogs<Type>::placeDialog(dialog_);

    }

    void setTitle(const gchar *title){
        gtk_window_set_title(dialog_, title?title:_("Running"));
    }

    void setProgressBarText(const gchar *text){
        gtk_progress_bar_set_text (progressBar_, text?text:_("Waiting for operation to finish..."));
        gtk_progress_bar_set_show_text (progressBar_, TRUE);
        gtk_progress_bar_pulse(progressBar_);
    }


};

template <class Type>
class CommandResponse: public BaseCommandResponse<Type> {
    pid_t controllerPid_; 
public:

    ~CommandResponse(void)
    {
        gtk_widget_hide(GTK_WIDGET(this->dialog()));
        gtk_widget_destroy(GTK_WIDGET(this->dialog()));
    }
    CommandResponse(const gchar *message, const gchar *icon, const gchar **arg, void (*afterFunction)(void *)=NULL, void *data=NULL):
        BaseCommandResponse<Type>(message, icon)
    {
        controllerPid_ = Run<Type>::thread_runReap(
                data, // data to fork finished function
                arg,
                Run<Type>::run_operate_stdout,
                Run<Type>::run_operate_stderr,
                afterFunction // comand done function
                );
        addPulse();
        return ;           
    }
/*
    CommandResponse(const gchar *message, const gchar *icon, const gchar *command):
        BaseCommandResponse<Type>(message, icon)
    {
        controllerPid_ = Run<Type>::thread_run(Fm<Type>::getCurrentTextview(),command, FALSE);
        addPulse();
        return ;        
    }
*/   
private:
    void addPulse(void){
        this->setController(controllerPid_);        
        g_timeout_add(250, pulse_f, (void *)this);
    }

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
        auto object = (CommandResponse<Type> *)data;
        auto progressBar = GTK_PROGRESS_BAR(object->progressBar());
        if (!GTK_IS_PROGRESS_BAR(progressBar)){
            return FALSE;
        }
        gint pid = Tubo<Type>::getChild(object->controller());
        if (!CommandResponse<Type>::isPidAlive(pid)){
            delete(object);
            return FALSE;
        }
        gtk_progress_bar_pulse(progressBar);
        return TRUE;
    }    


};
}

#endif

