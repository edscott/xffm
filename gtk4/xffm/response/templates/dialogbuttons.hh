#ifndef DIALOGBUTTONS_HH
# define DIALOGBUTTONS_HH
namespace xf
{

  template <class dialogClass>
  class DialogButtons : public DialogTimeout<dialogClass>{
//  class DialogButtons : public DialogTimeout<dialogClass>{

    public:
    DialogButtons(void){
      auto dialog = this->dialog();
      auto contentArea = this->contentArea();
      auto button = this->subClass()->getButtons();
      auto box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
      gtk_box_append(contentArea, GTK_WIDGET(box));

      for (int i=0; button && button[i]; i++){
        g_object_set_data(G_OBJECT(button[i]), "dialog", dialog);
        gtk_box_append(box, GTK_WIDGET(button[i]));
        g_signal_connect(G_OBJECT(button[i]), "clicked", G_CALLBACK(setResponse), GINT_TO_POINTER(i+1));
      }
      
      gtk_widget_realize(GTK_WIDGET(dialog));
      Basic::setAsDialog(GTK_WIDGET(dialog), "dialog", "Dialog");
      gtk_window_present(dialog);

    }
    private:
    static void setResponse(GtkButton *button, void *data){
      auto dialog = g_object_get_data(G_OBJECT(button), "dialog");
      g_object_set_data(G_OBJECT(dialog), "response", data);
    }
    

  };
}
#endif

