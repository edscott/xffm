#ifndef DIALOGENTRY_HH
# define DIALOGENTRY_HH
namespace xf
{
  template <class subClass_t>
  class DialogEntry : public DialogTimeout<subClass_t>{
    using dialog_t = DialogEntry<subClass_t>; 
    protected:
      
      GtkBox *entryBox_=NULL;
      GtkEntry *entry_=NULL;
      GtkWidget *help_ = NULL;
      GtkLabel *entryLabel_=NULL;
    
   public:
    GtkEntry *entry(void){return entry_;}
    DialogEntry(void){
       
       entryBox_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3));
       gtk_widget_set_hexpand(GTK_WIDGET(entryBox_), false);
       gtk_widget_set_halign (GTK_WIDGET(entryBox_),GTK_ALIGN_CENTER);
       gtk_box_append(GTK_BOX (this->contentArea()), GTK_WIDGET(entryBox_));
        
       entryLabel_ = GTK_LABEL (gtk_label_new (""));
       entry_ = GTK_ENTRY(gtk_entry_new ());
       gtk_widget_set_size_request(GTK_WIDGET(entry_), 200, -1);
       Basic::boxPack0(entryBox_, GTK_WIDGET(entryLabel_), false, false, 1);
       Basic::boxPack0(entryBox_, GTK_WIDGET(entry_), true, true, 5);


       //gtk_box_append(GTK_BOX (entryBox_), GTK_WIDGET(entry_));
       gtk_widget_set_halign (GTK_WIDGET(entry_),GTK_ALIGN_CENTER);
       g_object_set_data(G_OBJECT(this->dialog()),"entry", entry_);
       g_object_set_data(G_OBJECT(this->dialog()),"entryLabel", entryLabel_);
       g_object_set_data(G_OBJECT(this->dialog()),"entryBox", entryBox_);
       g_signal_connect (G_OBJECT (entry_), "activate", 
                ENTRY_CALLBACK (this->activate), this->dialog());
       auto apply = this->applyBox();
       gtk_box_append(GTK_BOX (entryBox_), apply);
       
       help_ = Dialog::buttonBox(EMBLEM_QUESTION, _("Help"), 
               NULL, NULL);
       gtk_box_append(GTK_BOX (entryBox_), help_);
       gtk_widget_set_visible(GTK_WIDGET(help_), false);
       g_object_set_data(G_OBJECT(entry_),"help", help_);


      // auto cancel = this->cancelBox();
       //gtk_box_append(GTK_BOX (entryBox_), cancel);
       
       g_object_set_data(G_OBJECT(entry_),"dialog", this->dialog());
       /*g_signal_connect (G_OBJECT (entry_), "activate", 
                ENTRY_CALLBACK (activate_entry), (void *)dialog);*/
      gtk_widget_realize(GTK_WIDGET(this->dialog()));
      Basic::setAsDialog(GTK_WINDOW(this->dialog()));
      gtk_window_present(this->dialog());

    }
    private:
    static void activate(GtkEntry *entry, void *dialog){
      g_object_set_data(G_OBJECT(dialog), "response", GINT_TO_POINTER(2));
    }

  };
}
#endif

