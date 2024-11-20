#ifndef PROGRESS_HH
#define PROGRESS_HH

namespace xf {
class ProgressBar {
    GtkProgressBar *progress_;
    time_t timeout_;
    GtkWindow *dialog_ = NULL;
    GtkProgressBar *timeoutProgress_;
public:
    GtkProgressBar *progressBar(void){return timeoutProgress_;}
    time_t timeout(void){return timeout_;}
    void timeout(time_t value){timeout_ = value;}
    GtkWindow *dialog(void){return dialog_;}

    ProgressBar(void){
      timeoutProgress_ = GTK_PROGRESS_BAR(gtk_progress_bar_new());
      //g_timeout_add(500, updateProgress, (void *)this);
    }

    ~ProgressBar(void){
    }

    void setDialog(GtkWindow *dialog){
      dialog_ = dialog;
      if (dialog){
        //g_timeout_add(100, updateProgress, (void *)this);
        /*auto keyController = gtk_event_controller_key_new();
        gtk_event_controller_set_propagation_phase(keyController, GTK_PHASE_CAPTURE);
        gtk_widget_add_controller(GTK_WIDGET(this->dialog()), keyController);
        g_signal_connect (G_OBJECT (keyController), "key-pressed", 
            G_CALLBACK (this->on_keypress), (void *)this);*/
      } else {
        timeoutProgress_ = NULL;
      }
    }

private:
    static gboolean
    updateProgress(void * data){
      // This is main context.
        auto object = (ProgressBar *)data;
        auto dialog  = object->dialog();
        if (!dialog) {
          return G_SOURCE_REMOVE;
        }

        //if (!GTK_IS_WINDOW(dialog))  return G_SOURCE_REMOVE;
        //if (!GTK_IS_PROGRESS_BAR(progress))  return G_SOURCE_REMOVE;

        // While window is active, pause progress bar.
        // causes crash when user cancels dialog.
       /* if (gtk_window_is_active(GTK_WINDOW(dialog))){
          TRACE("gtk_window_is_active\n");
          return G_SOURCE_CONTINUE;
        }*/
        TRACE("timeout = %lf\n", timeout);

        auto progress = object->progressBar();
        if (!progress) return G_SOURCE_REMOVE;
        // Add fraction to progressbar.
        auto fraction = gtk_progress_bar_get_fraction(progress);
        if (fraction < 1.0) {
            fraction += (1.0 / 100.0);
            TRACE("gtk_progress_bar_set_fraction %lf\n", fraction);
            gtk_progress_bar_set_fraction(progress, fraction);
        } 
        // Complete fraction, cancel dialog and .
        if (fraction >= 1.0) {
          auto message = g_strdup_printf(" %s (%d)\n", _("Cancelled"), _("Idle Timeout"));
          Print::printWarning(Child::getOutput(), message);

          TRACE("cancel dialog by timeout\n");
          
          g_object_set_data(G_OBJECT(dialog), "response", GINT_TO_POINTER(-3)); 
        }
        return G_SOURCE_CONTINUE;
    }

/*
    static gboolean
    on_keypress (GtkEventControllerKey* self,
          guint keyval,
          guint keycode,
          GdkModifierType state,
          gpointer data){

       auto object = (DialogTimeout<dialogClass> *)data;
       auto progress = object->progress();
        if (!progress || !GTK_IS_PROGRESS_BAR(progress)) return FALSE;
        gtk_progress_bar_set_fraction(progress, 0.0);
        if(keyval == GDK_KEY_Escape) { 
          g_object_set_data(G_OBJECT(object->dialog()), "response", GINT_TO_POINTER(-2)); 
          return TRUE;
        }
        if(keyval == GDK_KEY_Return) { 
          g_object_set_data(G_OBJECT(object->dialog()), "response", GINT_TO_POINTER(2)); 
          return TRUE;
        }
        return FALSE;      
    }
*/
    

    static void *asyncYes(void *data){
      //auto dialogObject = (DialogTimeoutButtons<infoResponse> *)data;
      //dialogObject->timeout(-1);
      
      TRACE("%s", "hello world\n");
      return NULL;
    }

    static void *asyncNo(void *data){
      //auto dialogObject = (DialogTimeoutButtons<infoResponse> *)data;
      //dialogObject->timeout(-1);
      TRACE("%s", "bye world\n");
      return NULL;
    }
 
private:

    static void activate(GtkEntry *entry, void *dialog){
      g_object_set_data(G_OBJECT(dialog), "response", GINT_TO_POINTER(2));
    }



    static gboolean 
    response_delete(GtkWidget *dialog, GdkEvent *event, gpointer data){
        return TRUE;
    }

};


}
#endif
