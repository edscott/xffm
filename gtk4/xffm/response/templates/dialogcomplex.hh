#ifndef DIALOGCOMPLEX_HH
# define DIALOGCOMPLEX_HH
namespace xf
{

  template <class dialogClass>
  class DialogComplex : public DialogBasic<dialogClass> {
    public:
    DialogComplex(void){
      auto frame = this->frame();
      auto mainBox = this->subClass()->mainBox();
      gtk_frame_set_child(frame, GTK_WIDGET(mainBox));
      
    /*  clearVbox();
      auto mainBox = this->subClass()->mainBox();
      gtk_box_prepend(this->vbox(), GTK_WIDGET(mainBox));*/
    }
    void setSubClassDialog(){
      this->subClass()->setSubClassDialog(this->dialog());
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
