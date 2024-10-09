#ifndef DIALOGTIMEOUT_HH
# define DIALOGTIMEOUT_HH
#include "../classes/progressbar.hh"

namespace xf
{

  template <class dialogClass>
  class DialogTimeout: public Dialog<dialogClass> {
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
      delete dialogObject;
      return response_p;
    }

    static void *run_f(void *data){
      // This is run by thread.
      auto dialogObject = (DialogTimeout<dialogClass> *)data;
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

  
}
#endif

