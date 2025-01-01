#ifndef DIALOGENTRY_HH
# define DIALOGENTRY_HH
namespace xf
{
  template <class Type> class FileResponse;
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
    protected:
      
      GtkBox *entryBox_;
      GtkEntry *entry_;

    static void getDirectory(GtkButton *button, void *data){
      TRACE("getDirectory\n");
      auto subClass = (dialogClass *)data;
      //auto subClass = (mountResponse *)data;
      auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "entry"));
      //subClass->getDirectoryObject(subClass, entry);
      getDirectoryObject(subClass, entry);
    }

    private:  
    static void getDirectoryObject(dialogClass *object, GtkEntry *entry){
      auto startFolder = "/";

      TRACE("*** newFileDialog1 startFolder = %s\n", startFolder);
      auto newObject = new DialogComplex<FileResponse<dialogClass> >(startFolder);
      newObject->subClass()->parentEntry(entry);
      
        auto _dialog = newObject->dialog();
        newObject->setParent(object->dialog()); 

      gtk_window_set_decorated(_dialog, true);
      gtk_widget_realize(GTK_WIDGET(_dialog)); 
      Basic::setAsDialog(GTK_WIDGET(_dialog), "dialog", "Dialog");
      gtk_window_present(_dialog);

      // This fires off the dialog controlling thread, and will delete
      // object when dialog is destroyed.
      newObject->run();
    }
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

  template <class dialogClass>
  class DialogEntryPath : public DialogEntry<dialogClass>{
    public:
    DialogEntryPath(void){
      auto button = Basic::mkButton("document-open", NULL);
      g_object_set_data(G_OBJECT(button), "entry", this->entry_);
      g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(this->getDirectory), this);
      gtk_box_append(this->entryBox_, GTK_WIDGET(button));

    }

  };
}
#endif

