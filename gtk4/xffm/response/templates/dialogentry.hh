#ifndef DIALOGENTRY_HH
# define DIALOGENTRY_HH
namespace xf
{
  template <class dialogClass>
  class DialogPasswd : public DialogBasic<dialogClass>{

    public:
    DialogPasswd(void){
       auto entryBox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3));
       gtk_widget_set_hexpand(GTK_WIDGET(entryBox), false);
       gtk_widget_set_halign (GTK_WIDGET(entryBox),GTK_ALIGN_CENTER);
       gtk_box_append(GTK_BOX (this->contentArea()), GTK_WIDGET(entryBox));
        
       auto entry = GTK_ENTRY(gtk_entry_new ());
       auto buffer = gtk_password_entry_buffer_new();
       gtk_entry_set_buffer (entry,buffer);
       Basic::boxPack0(entryBox, GTK_WIDGET(entry), true, true, 5);
       //gtk_box_append(GTK_BOX (entryBox), GTK_WIDGET(entry));
       gtk_widget_set_halign (GTK_WIDGET(entry),GTK_ALIGN_CENTER);
       g_object_set_data(G_OBJECT(this->dialog()),"entry", entry);
       g_signal_connect (G_OBJECT (entry), "activate", 
                ENTRY_CALLBACK (this->activate), this->dialog());
       auto apply = this->applyBox();
       gtk_box_append(GTK_BOX (entryBox), apply);
       
       g_object_set_data(G_OBJECT(entry),"dialog", this->dialog());
       /*g_signal_connect (G_OBJECT (entry), "activate", 
                ENTRY_CALLBACK (activate_entry), (void *)dialog);*/
      gtk_widget_realize(GTK_WIDGET(this->dialog()));
      Basic::setAsDialog(GTK_WIDGET(this->dialog()), "dialog", "Dialog");
      gtk_window_present(this->dialog());

    }
    private:
    static void activate(GtkEntry *entry, void *dialog){
      g_object_set_data(G_OBJECT(dialog), "response", GINT_TO_POINTER(2));
    }
  };

  template <class dialogClass>
  class DialogEntry : public DialogTimeout<dialogClass>{
//  class DialogEntry : public DialogTimeout<dialogClass>{

    public:
    DialogEntry(void){
       auto entryBox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3));
       gtk_widget_set_hexpand(GTK_WIDGET(entryBox), false);
       gtk_widget_set_halign (GTK_WIDGET(entryBox),GTK_ALIGN_CENTER);
       gtk_box_append(GTK_BOX (this->contentArea()), GTK_WIDGET(entryBox));
        
       auto entry = GTK_ENTRY(gtk_entry_new ());
       Basic::boxPack0(entryBox, GTK_WIDGET(entry), true, true, 5);
       //gtk_box_append(GTK_BOX (entryBox), GTK_WIDGET(entry));
       gtk_widget_set_halign (GTK_WIDGET(entry),GTK_ALIGN_CENTER);
       g_object_set_data(G_OBJECT(this->dialog()),"entry", entry);
       g_signal_connect (G_OBJECT (entry), "activate", 
                ENTRY_CALLBACK (this->activate), this->dialog());
       auto apply = this->applyBox();
       gtk_box_append(GTK_BOX (entryBox), apply);
      // auto cancel = this->cancelBox();
       //gtk_box_append(GTK_BOX (entryBox), cancel);
       
       g_object_set_data(G_OBJECT(entry),"dialog", this->dialog());
       /*g_signal_connect (G_OBJECT (entry), "activate", 
                ENTRY_CALLBACK (activate_entry), (void *)dialog);*/
      gtk_widget_realize(GTK_WIDGET(this->dialog()));
      Basic::setAsDialog(GTK_WIDGET(this->dialog()), "dialog", "Dialog");
      gtk_window_present(this->dialog());

    }
    private:
    static void activate(GtkEntry *entry, void *dialog){
      g_object_set_data(G_OBJECT(dialog), "response", GINT_TO_POINTER(2));
    }
  };
}
#endif

