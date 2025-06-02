#ifndef DIALOGTIMEOUT_HH
# define DIALOGTIMEOUT_HH
#include "../classes/progressbar.hh"

namespace xf
{
  template <class Type>
#if 1
  class DialogTimeout: public DialogBasic<Type> {
  };
#else
  class DialogTimeout: public DialogBasic<Type> {
    using dialog_t = DialogTimeout<Type>;
    ProgressBar *progress_;
    ProgressBar progress_c;
public:

    ProgressBar *progressBar(void){ return progress_;}
    ~DialogTimeout(void){ 
    }
    DialogTimeout(void){ 
      progress_ = &progress_c;
      auto w = progress_->progressBar();
      Basic::boxPack0 (GTK_BOX (this->actionArea()),GTK_WIDGET(w), TRUE, TRUE, 0);
      progress_->setDialog(this->dialog());      
      
      gtk_widget_realize(GTK_WIDGET(this->dialog()));
      Basic::setAsDialog(GTK_WIDGET(this->dialog()), "dialog", "Dialog");
      gtk_window_present(this->dialog());
    }

    int run(){ // overload
      TRACE("run...\n");
      pthread_t thread;
      Thread::threadCount(true,  &thread, "DialogTimeout::run");
      int retval = pthread_create(&thread, NULL, runWait_f, this);
      pthread_detach(thread);
      Thread::threadCount(false,  &thread, "DialogTimeout::run");
      TRACE("run detached...\n");

      return 0;
    }

    static void *runWait_f(void *data){
      auto dialogObject = (dialog_t *)data;
      auto dialog = dialogObject->dialog();

      TRACE("runWait_f...\n");
      pthread_t thread;
      Thread::threadCount(true,  &thread, "DialogTimeout::runWait_f");
      int retval = pthread_create(&thread, NULL, run_f, (void *)dialogObject);
      void *response_p;
      pthread_join(thread, &response_p);
      Thread::threadCount(false,  &thread, "DialogTimeout::runWait_f");
      TRACE("run joined, *response_p = %p\n", response_p);
      delete dialogObject;
      return response_p;
    }

    static void *run_f(void *data){
      // This is run by thread.
      auto dialogObject = (dialog_t *)data;
      auto dialog = dialogObject->dialog();
      void *response = NULL;
      do {
        response = g_object_get_data(G_OBJECT(dialog), "response");
        if (exitDialogs) response = GINT_TO_POINTER(-1);
        usleep(2500);
      } while (!response);
      // delete progressBar object       
      dialogObject->progressBar()->setDialog(NULL);      
      // hide, main context...
      Basic::contextHide(GTK_WIDGET(dialog)); 
      auto subClass = dialogObject->subClass();

      
      if (GPOINTER_TO_INT(response) > 0){
        Basic::context_function(subClass->asyncYes, data);
      } else {
        Basic::context_function(subClass->asyncNo, data);
      }
      TRACE("run_f:: Response is %p\n", response);
      return response;
    }

    
  };
#endif

  
}
#endif

