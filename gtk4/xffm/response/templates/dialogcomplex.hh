#ifndef DIALOGCOMPLEX_HH
# define DIALOGCOMPLEX_HH
namespace xf
{

  template <class Type>
  class DialogComplex : public DialogBasic<Type> {
    GtkBox *mainBox_;
    GtkWindow *parent_;
    public:
    
    void setupRun(void){
      auto frame = this->frame();
      gtk_frame_set_child(frame, GTK_WIDGET(mainBox_));
      this->setParent(parent_);
      
      auto dialog = this->dialog();
      this->subClass()->dialog(dialog);

      gtk_window_set_decorated(dialog, true);
      gtk_widget_realize(GTK_WIDGET(dialog));
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

    // void setSubClassDialog(void)
    // Sets a pointer in the subClass object to refer to 
    // the GtkWindow dialog, in order for the subClass
    // object async main context thread callback to act
    // upon the dialog.
    //
    void setSubClassDialog(void){
      this->subClass()->dialog(this->dialog());
    }
    
    private:

  };

  
}
#endif

