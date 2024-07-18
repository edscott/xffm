#ifndef XF_COMMANDRESPONSE_HH
#define XF_COMMANDRESPONSE_HH

#undef WITH_PRETTY_STUFF
#define WITH_PRETTY_STUFF

namespace xf {

template <class Type>
class CommandResponse {
    GtkWindow *dialog_;
    GtkLabel *label_;
    GtkProgressBar *progressBar_;

    pid_t controller_; 
    gchar *arg0_;
    gchar *message_;
public:
    GtkWindow *dialog(void){return GTK_WINDOW(dialog_);}
    GtkLabel *label(void){return label_;}
    GtkProgressBar *progressBar(void){return progressBar_;}
   
    gchar *getArg0(void){return arg0_;}
    gchar *getMessage(void){return message_;}
    pid_t controller(void){return controller_;}

    ~CommandResponse(void)
    {
        gtk_widget_hide(GTK_WIDGET(this->dialog()));
        gtk_widget_destroy(GTK_WIDGET(this->dialog()));
    }
    CommandResponse(gchar *message, const gchar *icon, const gchar **arg, void (*afterFunction)(void *)=NULL, void *data=NULL)
    {
        message_ = message;
        label_ = NULL;
        arg0_ = g_strdup(arg[0]);
        dialog_ =  GTK_WINDOW(createDialog(message, icon));
        controller_ = Run<Type>::thread_runReap(
                data, // data to fork finished function
                arg,
                Run<Type>::run_operate_stdout,
                Run<Type>::run_operate_stderr,
                afterFunction // comand done function
                );
        addPulse();
        return ;           
    }
private:
    void addPulse(void){
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
  
#ifdef WITH_PRETTY_STUFF

    static gboolean pulse_f(void *data) {
        auto object = (CommandResponse<Type> *)data;
        auto progressBar = GTK_PROGRESS_BAR(object->progressBar());
        if (!GTK_IS_PROGRESS_BAR(progressBar)){
            return FALSE;
        }
        gint pid = Tubo<Type>::getChild(object->controller());
        if (!CommandResponse<Type>::isPidAlive(pid)){
            auto arg0 = object->getArg0();
            auto message = object->getMessage();
            g_free(message);
            g_free(arg0);
            delete(object);
            return FALSE;
        }
        gtk_progress_bar_pulse(progressBar);
        return TRUE;
    }   
    
    GtkWindow *createDialog(const gchar *message, const gchar *icon){
        // Create the widgets
        auto dialog = GTK_WINDOW(gtk_window_new (GTK_WINDOW_TOPLEVEL));
        //gtk_window_set_transient_for (dialog,GTK_WINDOW(mainWindow));
        gtk_window_set_type_hint (dialog,GDK_WINDOW_TYPE_HINT_DIALOG);
        auto vbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        gtk_container_add (GTK_CONTAINER (dialog), GTK_WIDGET(vbox));
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
                    compat<bool>::boxPackStart(vbox, image, FALSE, FALSE,0);
                    gtk_widget_show (image);
                }
            }
        }
        compat<bool>::boxPackStart(vbox, GTK_WIDGET(label_), FALSE, FALSE,0);
        progressBar_ = GTK_PROGRESS_BAR(gtk_progress_bar_new());
        compat<bool>::boxPackStart(vbox, GTK_WIDGET(progressBar_), FALSE, FALSE,0);
        setTitle(NULL, dialog);
        setProgressBarText(NULL, progressBar_);
        gtk_widget_realize (GTK_WIDGET(dialog));
         
        gtk_widget_show_all (GTK_WIDGET(dialog));
        Dialogs<Type>::placeDialog(dialog);
        return (dialog);
    }

    void setTitle(const gchar *title, GtkWindow *dialog){
        gtk_window_set_title(dialog, title?title:_("Running"));
    }

    void setProgressBarText(const gchar *text, GtkProgressBar *progressBar){
        gtk_progress_bar_set_text (progressBar, text?text:_("Waiting for operation to finish..."));
        gtk_progress_bar_set_show_text (progressBar, TRUE);
        gtk_progress_bar_pulse(progressBar);
    }
#else
    GtkDialog *createDialog(gchar *message, const gchar *icon){return NULL;}
    void setTitle(const gchar *title, GtkDialog *dialog){}
    void setProgressBarText(const gchar *text){}
    static gboolean pulse_f(void *data) {
        auto object = (CommandResponse<Type> *)data;
        auto arg0 = object->getArg0();
        auto message = object->getMessage();
        gint pid = Tubo<Type>::getChild(object->controller());
        if (!CommandResponse<Type>::isPidAlive(pid)){
            g_free(message);
            g_free(arg0);
            delete(object);
            return FALSE;
        }
        DBG("in progress:%d %s, %s\n", pid, arg0, message);
        return TRUE;
    }    
#endif

};
}

#endif

