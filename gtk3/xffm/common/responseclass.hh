#ifndef RESPONSECLASS__HH
# define RESPONSECLASS__HH
namespace xf
{

template <class Type> class Util;
template <class Type>
class ResponseClass {
  public:
    static void runDialog(GtkWindow *dialog, void *response_f, void *data){
        gtk_widget_show (GTK_WIDGET(dialog));
        void  **arg = (void **)calloc(4, sizeof(void *));
        arg[0] = (void *)dialog;
        arg[1] = (void *)response_f;
        arg[2] = data;
      DBG("runDialog: dialog=%p response_f = %p response_data=%p\n", arg[0], arg[1], arg[2]);

        pthread_t thread;
        int retval = pthread_create(&thread, NULL, runWait_f, (void *)arg);
        pthread_detach(thread);
    }

    static gboolean
    on_destroy_event (GtkWidget * rmDialog, GdkEvent * event, gpointer data) {
        g_object_set_data(G_OBJECT(data), "response", data);
        return TRUE;
    }

    static void
    responseAction(GtkWidget * button, void *data){
        auto dialog = g_object_get_data(G_OBJECT(button), "dialog");
        g_object_set_data(G_OBJECT(dialog), "response", data);
    }

  private:
    static void *run_f(void *data){
      void **arg = (void **)data;
      auto dialog = GTK_WINDOW(arg[0]);
      auto response_f = PTHREAD_CALLBACK(arg[1]);
      //auto response_data = arg[2];
      MainDialog = dialog; // dialog is active...
      DBG("run_f: dialog=%p response_f = %p response_data=%p\n", arg[0], arg[1], arg[2]);
      void *response = NULL;
      do {
        response = g_object_get_data(G_OBJECT(dialog), "response");
        if (exitDialogs) response = GINT_TO_POINTER(-1);
        usleep(2500);
      } while (!response);
      MainDialog = NULL; // user has responded!
      if (response_f) Util<Type>::context_function(response_f, data);
      
      TRACE("run_f:: Response is %p\n", response);
      return response;
    }   

    static void *runWait_f(void *data){
      void **arg = (void **)data;
      auto dialog = GTK_WINDOW(arg[0]);
      //auto response_f = ((*)(void *))arg[1];
      //auto response_data = arg[2];
      DBG("runWait_f: dialog=%p response_f = %p response_data=%p\n", arg[0], arg[1], arg[2]);
    
      pthread_t thread;
      int retval = pthread_create(&thread, NULL, run_f, data);
      void *response_p;
      pthread_join(thread, &response_p);
      DBG("run joined, *response_p = %p\n", response_p);
       
      return NULL;
    }



};
}
#endif
