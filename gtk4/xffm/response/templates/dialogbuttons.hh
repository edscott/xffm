#ifndef DIALOGBUTTONS_HH
# define DIALOGBUTTONS_HH
namespace xf
{

  template <class Type>
  class DialogButtons : public DialogTimeout<Type>{
//  class DialogButtons : public DialogTimeout<Type>{

    public:
    DialogButtons(void){
      auto dialog = this->dialog();
      auto contentArea = this->contentArea();
      gtk_widget_set_vexpand(GTK_WIDGET(contentArea), false);
      auto buttons = this->subClass()->getButtons();
      auto box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
      gtk_widget_set_vexpand(GTK_WIDGET(box), false);
      gtk_box_append(contentArea, GTK_WIDGET(box));

      for (int i=0; buttons && buttons[i]; i++){
        g_object_set_data(G_OBJECT(buttons[i]), "dialog", dialog);
        g_object_set_data(G_OBJECT(dialog), "buttons", buttons);
        gtk_box_append(box, GTK_WIDGET(buttons[i]));
        gtk_widget_set_halign (GTK_WIDGET(buttons[i]),GTK_ALIGN_CENTER);
        g_signal_connect(G_OBJECT(buttons[i]), "clicked", G_CALLBACK(setResponse), GINT_TO_POINTER(i+1));
      }
      gtk_window_set_decorated(dialog, true);
      
      gtk_widget_realize(GTK_WIDGET(dialog));
      Basic::setAsDialog(GTK_WINDOW(dialog));
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

