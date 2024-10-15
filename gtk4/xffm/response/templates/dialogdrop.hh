#ifndef DIALOGDROP_HH
# define DIALOGDROP_HH
namespace xf
{

  template <class dialogClass>
  class DialogDrop : public DialogBasic<dialogClass>{

    public:
    DialogDrop(void){
      auto dialog = this->dialog();
      auto contentArea = this->contentArea();
      //auto buttons = this->subClass()->getButtons();
      auto box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
      gtk_box_append(contentArea, GTK_WIDGET(box));

      /*for (int i=0; buttons && buttons[i]; i++){
        g_object_set_data(G_OBJECT(buttons[i]), "dialog", dialog);
        gtk_box_append(box, GTK_WIDGET(buttons[i]));
        gtk_widget_set_halign (GTK_WIDGET(buttons[i]),GTK_ALIGN_END);

        g_signal_connect(G_OBJECT(buttons[i]), "clicked", G_CALLBACK(setResponse), GINT_TO_POINTER(i+1));
      }*/
      
      // enough space, seems to resize on each label grow change...
      gtk_widget_set_size_request(GTK_WIDGET(dialog), 200, 75);

      gtk_widget_realize(GTK_WIDGET(dialog));
      Basic::setAsDialog(GTK_WIDGET(dialog), "dialog", "Dialog");
      gtk_window_present(dialog);

    }
  /*  static void setDefaults(GtkWindow *dialog, GtkLabel *label){
      auto string = g_strconcat(
          "<span color=\"red\" size=\"large\"><b>",_("Copying"),
           "</b></span>\n"," \n",
          "<span color=\"blue\"><b>", 
          _("Copy files"), 
          "</b></span>", NULL);
      gtk_label_set_markup(label, string);
      gtk_widget_set_halign(GTK_WIDGET(label), GTK_ALIGN_CENTER);
      g_free(string);
    }   */
   
    private:
  /*  static void setResponse(GtkButton *button, void *data){
      auto dialog = g_object_get_data(G_OBJECT(button), "dialog");
      g_object_set_data(G_OBJECT(dialog), "response", data);
    }*/
    

  };
}
#endif

