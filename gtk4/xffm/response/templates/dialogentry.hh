#ifndef DIALOGENTRY_HH
# define DIALOGENTRY_HH
namespace xf
{
  template <class Type>
  class DialogEntry : public DialogTimeout<Type>{
    protected:
      
      GtkBox *entryBox_;
      GtkEntry *entry_;
    
   public:
    GtkEntry *entry(void){return entry_;}
    DialogEntry(void){
       entryBox_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3));
       gtk_widget_set_hexpand(GTK_WIDGET(entryBox_), false);
       gtk_widget_set_halign (GTK_WIDGET(entryBox_),GTK_ALIGN_CENTER);
       gtk_box_append(GTK_BOX (this->contentArea()), GTK_WIDGET(entryBox_));
        
       auto entryLabel = GTK_LABEL (gtk_label_new (""));
       entry_ = GTK_ENTRY(gtk_entry_new ());
       gtk_widget_set_size_request(GTK_WIDGET(entry_), 200, -1);
       Basic::boxPack0(entryBox_, GTK_WIDGET(entryLabel), false, false, 1);
       Basic::boxPack0(entryBox_, GTK_WIDGET(entry_), true, true, 5);


       //gtk_box_append(GTK_BOX (entryBox_), GTK_WIDGET(entry_));
       gtk_widget_set_halign (GTK_WIDGET(entry_),GTK_ALIGN_CENTER);
       g_object_set_data(G_OBJECT(this->dialog()),"entry", entry_);
       g_object_set_data(G_OBJECT(this->dialog()),"entryLabel", entryLabel);
       g_signal_connect (G_OBJECT (entry_), "activate", 
                ENTRY_CALLBACK (this->activate), this->dialog());
       auto apply = this->applyBox();
       gtk_box_append(GTK_BOX (entryBox_), apply);
      // auto cancel = this->cancelBox();
       //gtk_box_append(GTK_BOX (entryBox_), cancel);
       
       g_object_set_data(G_OBJECT(entry_),"dialog", this->dialog());
       /*g_signal_connect (G_OBJECT (entry_), "activate", 
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

