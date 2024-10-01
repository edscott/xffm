#ifndef DIALOGTIMEOUT_HH
# define DIALOGTIMEOUT_HH
namespace xf
{

  template <class dialogClass>
  class DialogTimeout: public Dialog<dialogClass> {
    time_t timeout_;
    GtkProgressBar *timeoutProgress_;
public:
    GtkProgressBar *progress(void){return timeoutProgress_;}
    time_t timeout(void){return timeout_;}
    void timeout(time_t value){timeout_ = value;}


    ~DialogTimeout(void){ 
      
    }
    DialogTimeout(void){ 
      // progress bar timeout       
      timeoutProgress_ = GTK_PROGRESS_BAR(gtk_progress_bar_new());
      Basic::boxPack0 (GTK_BOX (this->actionArea()),GTK_WIDGET(timeoutProgress_), TRUE, TRUE, 0);
      
      g_timeout_add(500, updateProgress, (void *)this);

    
      auto keyController = gtk_event_controller_key_new();
      gtk_event_controller_set_propagation_phase(keyController, GTK_PHASE_CAPTURE);
      gtk_widget_add_controller(GTK_WIDGET(this->dialog()), keyController);
      g_signal_connect (G_OBJECT (keyController), "key-pressed", 
          G_CALLBACK (this->on_keypress), (void *)this);
    }

    int run(){ // overload
      TRACE("run...\n");
      pthread_t thread;
      int retval = pthread_create(&thread, NULL, runWait_f, this);
      pthread_detach(thread);
      TRACE("run detached...\n");

      return 0;
    }

    static void *runWait_f(void *data){
      auto dialogObject = (DialogTimeout<dialogClass> *)data;
      auto dialog = dialogObject->dialog();

      TRACE("runWait_f...\n");
      pthread_t thread;
      int retval = pthread_create(&thread, NULL, run_f, (void *)dialogObject);
      void *response_p;
      pthread_join(thread, &response_p);
      TRACE("run joined, *response_p = %p\n", response_p);
      // Here we set timeout to -3 
      dialogObject->timeout(-3);
      // delete dialogObject;
      return response_p;
    }

    static void *run_f(void *data){
      auto dialogObject = (Dialog<dialogClass> *)data;
      auto dialog = dialogObject->dialog();
      void *response = NULL;
      do {
        response = g_object_get_data(G_OBJECT(dialog), "response");
        if (exitDialogs) response = GINT_TO_POINTER(-1);
        usleep(2500);
      } while (!response);
      // hide
      auto subClass = dialogObject->subClass();
      
      if (GPOINTER_TO_INT(response) > 0){
        Basic::context_function(subClass->asyncYes, data);
      } else {
        Basic::context_function(subClass->asyncNo, data);
      }
      TRACE("run_f:: Response is %p\n", response);
      // object will now be deleted.
      return response;
    }

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

    static gboolean
    updateProgress(void * data){
        auto object = (DialogTimeout<dialogClass> *)data;
        auto arg = (void **)data;
        auto timeout = object->timeout();
        auto progress = object->progress();
        auto dialog  = object->dialog();
            
        // Shortcircuit to delete.
        if (timeout < 0) {
          TRACE("timeout done...\n");
          delete object;
          return G_SOURCE_REMOVE;
        }

        if (!GTK_IS_WINDOW(dialog))  return G_SOURCE_REMOVE;
        if (!GTK_IS_PROGRESS_BAR(progress))  return G_SOURCE_REMOVE;

        // While window is active, pause progress bar.
        if (gtk_window_is_active(GTK_WINDOW(dialog))){
          TRACE("gtk_window_is_active\n");
          return G_SOURCE_CONTINUE;
        }
        TRACE("timeout = %lf\n", timeout);

        // Add fraction to progress bar.
        auto fraction = gtk_progress_bar_get_fraction(progress);
        if (fraction < 1.0) {
            fraction += (1.0 / 30.0);
            TRACE("gtk_progress_bar_set_fraction %lf\n", fraction);
            gtk_progress_bar_set_fraction(progress, fraction);
        } 
        // Complete fraction, cancel dialog and .
        if (fraction >= 1.0) {
          DBG("cancel dialog by timeout\n");
          //object->timeout(-3); // remove timeout 
          g_object_set_data(G_OBJECT(dialog), "response", GINT_TO_POINTER(-3)); 
          delete object;
          return G_SOURCE_REMOVE;
        }
        return G_SOURCE_CONTINUE;
    }
    
  };

  
}
#endif

