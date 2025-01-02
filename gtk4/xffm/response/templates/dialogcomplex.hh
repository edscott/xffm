#ifndef DIALOGCOMPLEX_HH
# define DIALOGCOMPLEX_HH
namespace xf
{

  template <class dialogClass>
  class DialogComplex : public DialogBasic<dialogClass> {
    public:
    
    DialogComplex(const char *folder, const char *path){
      TRACE("DialogComplex1\n");
      auto frame = this->frame();
      auto mainBox = this->subClass()->mainBox(folder, path);
      TRACE("DialogComplex12\n");
      gtk_frame_set_child(frame, GTK_WIDGET(mainBox));
      TRACE("DialogComplex123\n");
      setSubClassDialog();
    }
    

    DialogComplex(const char *folder){
      TRACE("DialogComplex1\n");
      auto frame = this->frame();
      auto mainBox = this->subClass()->mainBox(folder);
      TRACE("DialogComplex12\n");
      gtk_frame_set_child(frame, GTK_WIDGET(mainBox));
      TRACE("DialogComplex123\n");
      setSubClassDialog();
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
 /*   void clearVbox(void){
      gtk_widget_unparent(GTK_WIDGET(this->contentArea()));
      gtk_widget_unparent(GTK_WIDGET(this->actionArea()));
      gtk_widget_unparent(GTK_WIDGET(this->labelBox()));
      gtk_widget_unparent(GTK_WIDGET(this->vbox2()));
      gtk_widget_set_hexpand(GTK_WIDGET(this->vbox()), false);
      gtk_widget_set_vexpand(GTK_WIDGET(this->vbox()), false);
      //gtk_widget_set_visible(GTK_WIDGET(), false);
    }*/

  };

  
}
#endif

