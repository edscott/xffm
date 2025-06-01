#ifndef DIALOGCOMPLEX_HH
# define DIALOGCOMPLEX_HH
namespace xf
{

  template <class subClass_t>
  class DialogSimple : public DialogComplex<subClass_t> {
    using simpleDialog_t = DialogSimple<subClass_t>;
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

      /*auto cbox = GTK_BOX(g_object_get_data(G_OBJECT(dialog), "cbox"));
      auto foo = gtk_label_new("foo ok");
      gtk_box_prepend(cbox, foo);*/

      gtk_window_set_decorated(dialog, true);
      gtk_widget_realize(GTK_WIDGET(dialog));
      Basic::setAsDialog(dialog);
      gtk_window_present(dialog);

      // This fires off the dialog controlling thread, and will delete
      // object when dialog is destroyed.
      this->run();
    }

    DialogSimple(GtkWindow *parent, const char *txt){
      mainBox_ = this->subClass()->mainBox(txt);
      parent_ = parent;
      setupRun();
    }

  };

  
}
#endif

