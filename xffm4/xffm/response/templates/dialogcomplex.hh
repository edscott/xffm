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
      const char * folder = Child::getWorkdir(NULL);
      mainBox_ = this->subClass()->mainBox(folder);
      parent_ = GTK_WINDOW(_mainWidget);
      setupRun();
    }
    void setupRun(void){ // exclusive to DialogComplex
      auto frame = this->frame();

      this->setParent(parent_);
       
      auto vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
      gtk_widget_set_vexpand(GTK_WIDGET(vbox), false);
      gtk_widget_set_hexpand(GTK_WIDGET(vbox), true);
      gtk_frame_set_child(GTK_FRAME(frame), GTK_WIDGET(vbox));  
      Basic::boxPack0 (GTK_BOX (vbox),GTK_WIDGET(mainBox_), TRUE, TRUE, 0);
      // Termux-x11 speed up showing dialog trick:
      auto progress = GTK_PROGRESS_BAR(gtk_progress_bar_new());
      Basic::boxPack0 (GTK_BOX (vbox),GTK_WIDGET(progress), false, false, 0);
      g_timeout_add(50, Basic::pulseProgress, (void *)progress);
      //gtk_frame_set_child(frame, GTK_WIDGET(mainBox_));

      auto dialog = this->dialog();
      this->subClass()->dialog(dialog);
      TRACE("*** DialogComplex setupRun: setting this->subClass()->dialog to %p\n", dialog);
          gtk_widget_set_hexpand(GTK_WIDGET(dialog), true);
          gtk_widget_set_vexpand(GTK_WIDGET(dialog), true);

      /*auto cbox = GTK_BOX(g_object_get_data(G_OBJECT(dialog), "cbox"));
      auto foo = gtk_label_new("foo ok");
      gtk_box_prepend(cbox, foo);*/

      gtk_window_set_decorated(dialog, true);
      gtk_widget_realize(GTK_WIDGET(dialog));
      
      int w = -1;
      int h = -1;
      if (g_object_get_data(G_OBJECT(mainBox_), "width"))
        w = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(mainBox_), "width"));
      if (g_object_get_data(G_OBJECT(mainBox_), "height"))
        h = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(mainBox_), "height"));
      //gtk_widget_set_size_request(GTK_WIDGET(mainBox_), w, h);
      gtk_window_set_default_size(GTK_WINDOW(dialog), w, h);

      auto fixed = g_object_get_data(G_OBJECT(mainBox_), "fixed");
      gtk_window_set_resizable(GTK_WINDOW(dialog), fixed == NULL);  

      Basic::setAsDialog(dialog);
      gtk_window_present(dialog);
      // This fires off the dialog controlling thread, and will delete
      // object when dialog is destroyed.
      this->run();
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
    
    static void *runWait_f(void *data){
      auto dialogObject = (complexDialog_t *)data;
      //auto dialog = dialogObject->dialog();
      TRACE("*** runWait_f for dialog_t\n");
      //Basic::moveToPointer(dialogObject->dialog()); //Centers on the pointer screen (not always).
      run_f(dialogObject);
      //delete dialogObject;
      Basic::context_function(DialogBasic<subClass_t>::contextDelete_f, data);
 
      return NULL;
    }
    

  };

  
}
#endif

