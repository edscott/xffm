#ifndef DIALOGCOMPLEX_HH
# define DIALOGCOMPLEX_HH
namespace xf
{

  template <class subClass_t>
  class DialogComplex : public DialogBasic<subClass_t> {
    using complexDialog_t = DialogComplex<subClass_t>;
    GtkBox *mainBox_;
    GtkWindow *parent_;
    public:
    
    void setupRun(void){
      auto frame = this->frame();
      gtk_frame_set_child(frame, GTK_WIDGET(mainBox_));
      this->setParent(parent_);
      
      auto dialog = this->dialog();
      this->subClass()->dialog(dialog);
      TRACE("*** DialogComplex setupRun: setting this->subClass()->dialog to %p\n", dialog);
          gtk_widget_set_hexpand(GTK_WIDGET(dialog), true);

      /*auto cbox = GTK_BOX(g_object_get_data(G_OBJECT(dialog), "cbox"));
      auto foo = gtk_label_new("foo ok");
      gtk_box_prepend(cbox, foo);*/

      gtk_window_set_decorated(dialog, true);
      gtk_widget_realize(GTK_WIDGET(dialog));
      
      gtk_widget_set_size_request(GTK_WIDGET(mainBox_), 650, -1);
      Basic::setAsDialog(dialog);
      gtk_window_present(dialog);

      // This fires off the dialog controlling thread, and will delete
      // object when dialog is destroyed.
      this->run();
    }

    DialogComplex(GtkWindow *parent, const char *folder, const char *path){
      mainBox_ = this->subClass()->mainBox(folder, path);
      parent_ = parent;
      setupRun();
    }

    DialogComplex(GtkWindow *parent, const char *folder){
      mainBox_ = this->subClass()->mainBox(folder);
      parent_ = parent;
      setupRun();
    }

    DialogComplex(void){
    }

    // void setSubClassDialog(void)
    // Sets a pointer in the subClass object to refer to 
    // the GtkWindow dialog, in order for the subClass
    // object async main context thread callback to act
    // upon the dialog.
    //
    void setSubClassDialog(void){
      this->subClass()->dialog(this->dialog());
    }
    
    // Overloads:
    static void *contextDelete_f(void *data){
      auto dialogObject = (complexDialog_t *)data;
      delete dialogObject;       
      return NULL;
    }

    static void *runWait_f(void *data){
      TRACE("*** runWait_f for complexDialog_t\n");
      auto dialogObject = (complexDialog_t *)data;
      //auto dialog = dialogObject->dialog();

      TRACE("runWait_f...\n");
      pthread_t thread;
      Thread::threadCount(true,  &thread, "DialogBasic::runWait_f");
      int retval = pthread_create(&thread, NULL, run_f, (void *)dialogObject);
      void *response_p;
      pthread_join(thread, &response_p);
      Thread::threadCount(false,  &thread, "DialogBasic::runWait_f");
      TRACE("run joined, *response_p = %p\n", response_p);
      // 
      //delete dialogObject; // FIXME: do in main context
      Basic::context_function(contextDelete_f, data);
      return response_p;
    }

    static void *run_f(void *data){
      TRACE("*** run_f for complexDialog_t\n");
      auto dialogObject = (complexDialog_t *)data;
      auto dialog = dialogObject->dialog();
      void *response = NULL;
      TRACE("*** run_f (thread)\n");
      do {
        dialogObject->lockResponse();
        response = g_object_get_data(G_OBJECT(dialog), "response");
        dialogObject->unlockResponse();
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
      if (Child::mainWidget() && GTK_IS_WINDOW (Child::mainWidget())) {
        Basic::present(GTK_WINDOW(Child::mainWidget()));
      }
      TRACE("run_f:: Response is %p\n", response);
      // object will now be deleted.
      return response;
    }

    int run(void){
      TRACE("*** complexDialog_t run...\n");
      pthread_t thread;
      Thread::threadCount(true,  &thread, "DialogComplex::run");
      int retval = pthread_create(&thread, NULL, runWait_f, this);
      pthread_detach(thread);
      Thread::threadCount(false,  &thread, "DialogComplex::run");
      TRACE("run detached...\n");
      return 0;
    }

  };

  
}
#endif

