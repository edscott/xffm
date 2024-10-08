#ifndef DIALOGPROMPT_HH
#define DIALOGPROMPT_HH
namespace xf {


  template <class dialogClass>
  class DialogPrompt : public DialogTimeout<dialogClass>{
    GtkTextView *input_;
    GtkWidget *child_;
    Prompt<dialogClass> *prompt_p;

    public:
    char *getText(void){return Print::inputText(input_);}

    ~DialogPrompt(void){
      delete prompt_p;
    }
    
    DialogPrompt(void){
      child_ = Child::getChild();
      prompt_p = (Prompt<dialogClass> *) new Prompt<dialogClass>(child_);
      g_object_set_data(G_OBJECT(child_), "prompt", prompt_p);
      
      
      auto hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
      gtk_widget_set_hexpand(GTK_WIDGET(hbox), true);
      gtk_widget_set_halign (GTK_WIDGET(hbox),GTK_ALIGN_CENTER);
      input_ = prompt_p->input();
      gtk_widget_set_size_request(GTK_WIDGET(input_), 200, -1);

      Basic::boxPack0(GTK_BOX (hbox), GTK_WIDGET(input_), TRUE, TRUE, 3);
      gtk_box_append(GTK_BOX (this->contentArea()), GTK_WIDGET(hbox));
      
        
       // fixme; use a keypress to filter enter and escape
       //g_signal_connect (G_OBJECT (entry), "activate", 
         //       ENTRY_CALLBACK (this->activate), this->dialog());
       auto apply = this->applyBox();
       gtk_box_append(GTK_BOX (hbox), apply);
       
       //g_object_set_data(G_OBJECT(entry),"prompt_p", this->dialog());
       /*g_signal_connect (G_OBJECT (entry), "activate", 
                ENTRY_CALLBACK (activate_entry), (void *)dialog);*/
      gtk_widget_realize(GTK_WIDGET(this->dialog()));
      Basic::setAsDialog(GTK_WIDGET(this->dialog()), "dialog", "Dialog");
      gtk_window_present(this->dialog());

    }

    private:
    /*static void activate(GtkEntry *entry, void *dialog){
      g_object_set_data(G_OBJECT(dialog), "response", GINT_TO_POINTER(2));
    }*/
  };
}
#endif
