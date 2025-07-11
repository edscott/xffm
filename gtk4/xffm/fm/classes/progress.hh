#ifndef XF_BASEPROGRESSRESPONSE_HH
#define XF_BASEPROGRESSRESPONSE_HH
namespace xf {
class BaseProgress {
    GtkWindow *dialog_;
    GtkProgressBar *progressBar_;
    GtkLabel *label_;
public:

    GtkWindow *dialog(void){return dialog_;}
    GtkProgressBar *progressBar(void){return progressBar_;}
    GtkLabel *label(void){ return label_;}

    BaseProgress(const gchar *message, const gchar *icon){
        dialog_ = dialog(message, icon);
    }
private:
    GtkWindow *
    dialog(const gchar *message, const gchar *icon){
         // Create the widgets
         auto dialog = GTK_WINDOW(gtk_window_new ());
         //gtk_window_set_transient_for (dialog,GTK_WINDOW(mainWindow));
         //

         //gtk_window_set_type_hint (dialog,GDK_WINDOW_TYPE_HINT_DIALOG);
         auto vbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
         gtk_window_set_child(dialog,GTK_WIDGET(vbox));
         //gtk_container_add (GTK_CONTAINER (dialog), GTK_WIDGET(vbox));
         label_ = GTK_LABEL(gtk_label_new (""));
         auto markup = 
            g_strdup_printf("   <span color=\"blue\" size=\"larger\"><b>%s</b></span>   ", message);           
         gtk_label_set_markup(label_, markup);
         g_free(markup);
         
         // Add the label, and show everything we have added
         if (icon){
            auto image = Texture<bool>::getImage(icon, 48); 
            Basic::boxPack0(vbox, GTK_WIDGET(image), FALSE, FALSE,0);
         }
         Basic::boxPack0(vbox, GTK_WIDGET(label_), FALSE, FALSE,0);
         progressBar_ = GTK_PROGRESS_BAR(gtk_progress_bar_new());

         Basic::boxPack0(vbox, GTK_WIDGET(progressBar_), FALSE, FALSE,0);


         //gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
         return dialog;
    }
};
class Progress : public BaseProgress{

gboolean stop_;

public:
    Progress(const gchar *message, const gchar *icon, 
            const gchar *title, const gchar *text):
        BaseProgress(message, icon)
    {
        stop_ = FALSE;
        progressDialog(title, text);
    }

    void stop(void){stop_=TRUE;}
    gboolean getStop(void){ return stop_;}

private:
    GtkWindow *
    progressDialog(const gchar *title, const gchar *text)
    {
        auto dialog = this->dialog();
        auto progressBar = this->progressBar();

        gtk_window_set_title(dialog, title?title:_("Running"));
        /*auto text = g_strdup_printf("%s (pid: %d)", 
                 _("Waiting for operation to finish..."),
                 Tubo<Type>::getChild (pid));*/

        gtk_progress_bar_set_text (progressBar, text?text:_("Waiting for operation to finish..."));
        gtk_progress_bar_set_show_text (progressBar, TRUE);
        gtk_progress_bar_pulse(progressBar);
        gtk_widget_realize (GTK_WIDGET(dialog));
         
        Basic::setAsDialog(GTK_WINDOW(dialog));
        //gtk_widget_show_all (GTK_WIDGET(dialog));
        //Dialogs<Type>::placeDialog(dialog);
        g_timeout_add(250, simplePulse_f, (void *)this);


        return dialog;
    }

    static gboolean simplePulse_f(void *data) {
        auto progress = (Progress *)data;
        auto progressBar = progress->progressBar();
        auto dialog = progress->dialog();
        if (!GTK_IS_PROGRESS_BAR(progressBar)){
            ERROR_("simplePulse_f() not a progressbar\n");
            return FALSE;
        }
        if (progress->getStop()){
            gtk_widget_set_visible(GTK_WIDGET(dialog), false);
            gtk_window_destroy(GTK_WINDOW(dialog));
            delete(progress);
            return FALSE;
        }
        gtk_progress_bar_pulse(progressBar);
        return TRUE;
    }  
};
}

#endif

