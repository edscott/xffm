#ifndef DIALOG_HH
# define DIALOG_HH
namespace xf
{
  template <class dialogClass>
  class Dialog {
  GtkBox *contentArea_;
  GtkBox *actionArea_;
  GtkBox *iconBox_;
  GtkWindow *dialog_;
  GtkWindow *parent_;
  dialogClass *subClass_;
  GtkLabel *label_;

  public:
 
    ~Dialog(void){
      Basic::destroy(dialog_);
      // race
      // Basic::present(GTK_WINDOW(MainWidget));

      TRACE("window %p destroyed\n", dialog_);
      delete subClass_;
      TRACE("subClass_ deleted\n");
      TRACE("destructor for %p\n", this);
      MainDialog = NULL;
    }

    Dialog(GtkWindow *parent){ // must run in main context.
      subClass_ = new dialogClass;
      parent_ = parent;
      mkWindow();
     TRACE("dialog is %p\n", dialog_);
      MainDialog = dialog_;
      templateSetup();
      gtk_widget_realize(GTK_WIDGET(dialog_));
      Basic::setAsDialog(GTK_WIDGET(dialog_), "dialog", "Dialog");
      gtk_window_present(dialog_);

    }
    dialogClass *subClass(void){ return subClass_;}



    GtkWindow *parent(void){ return parent_;}


    
    GtkWindow *dialog(void){ return dialog_;}
    
    GtkBox *contentArea(void){ return contentArea_;}
    
    GtkBox *actionArea(void){ return actionArea_;}

    int run(){
      TRACE("run...\n");
      pthread_t thread;
      int retval = pthread_create(&thread, NULL, runWait_f, this);
      pthread_detach(thread);
      TRACE("run detached...\n");

      return 0;
    }

    void setLabelText(const char *text){
      if (!text) return;
      TRACE("set label to %s\n", text);
      auto markup = g_strconcat("<span color=\"blue\"><b>", text, "</b></span>", NULL);
      gtk_label_set_markup(label_, markup);
      g_free(markup);
    }
private:

    void mkTitle(void){ 
      const char *title = subClass_->title();
      if (!title) return;
      gtk_window_set_title(dialog_, title);
    }

    void mkLabel(void){
      label_ = GTK_LABEL(gtk_label_new(""));
      gtk_box_append(iconBox_, GTK_WIDGET(label_));
      gtk_widget_set_valign (GTK_WIDGET(label_),GTK_ALIGN_END);
      DBG("subclass label is %s\n",subClass_->label());       
      setLabelText(subClass_->label());
    }

    void mkIcon(void){
      const char *iconName = subClass_->iconName();
      if (!iconName) return;
      auto paintable = Texture::load(iconName, 48);
      if (paintable) {
        auto image = gtk_image_new_from_paintable(paintable);
        gtk_widget_set_size_request(GTK_WIDGET(image), 48, 48);
        if (image) {
          auto child = gtk_widget_get_first_child(GTK_WIDGET(iconBox_));
          if (child) gtk_widget_unparent(child);
          gtk_box_append(iconBox_, image);
        }
      }
      
    }
    static void *runWait_f(void *data){
      auto dialogObject = (Dialog<dialogClass> *)data;
      auto dialog = dialogObject->dialog();

      TRACE("runWait_f...\n");
      pthread_t thread;
      int retval = pthread_create(&thread, NULL, run_f, (void *)dialogObject);
      void *response_p;
      pthread_join(thread, &response_p);
      TRACE("run joined, *response_p = %p\n", response_p);
      // 
      delete dialogObject;
      return response_p;
    }

    static void *run_f(void *data){
      auto dialogObject = (Dialog<dialogClass> *)data;
      auto dialog = dialogObject->dialog();
      void *response = NULL;
      do {
        response = g_object_get_data(G_OBJECT(dialog), "response");
        usleep(2500);
      } while (!response);
      // hide
      
      if (GPOINTER_TO_INT(response) > 0){
        auto subClass = dialogObject->subClass_;
        Basic::context_function(subClass->asyncStart, (void *)dialog);
        Basic::context_function(subClass->asyncEnd, (void *)dialog);
        //Basic::destroy(dialog);
      }
      TRACE("run_f:: Response is %p\n", response);

      return response;
    }

    void templateSetup(void){
      // template class setup
      // in main context:
      mkTitle();
      mkIcon();
      mkLabel();
      subClass_->content(dialog_, contentArea_);
      subClass_->action(dialog_, actionArea_);
    }

    void mkWindow(void){
        dialog_ = GTK_WINDOW(gtk_window_new());
        
        if (parent_){
          //gtk_window_set_modal (GTK_WINDOW (dialog_), TRUE);
          gtk_window_set_transient_for (GTK_WINDOW (dialog_), GTK_WINDOW (parent_));
          gtk_window_set_destroy_with_parent(GTK_WINDOW (dialog_), true);
        }
        gtk_window_set_resizable (GTK_WINDOW (dialog_), TRUE);

        auto mainBox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        gtk_widget_set_vexpand(GTK_WIDGET(mainBox), false);
        gtk_window_set_child(dialog_, GTK_WIDGET(mainBox));

        iconBox_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_hexpand(GTK_WIDGET(iconBox_), true);
        gtk_widget_set_valign (GTK_WIDGET(iconBox_),GTK_ALIGN_CENTER);
        gtk_box_append(mainBox, GTK_WIDGET(iconBox_));



        contentArea_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        gtk_widget_set_vexpand(GTK_WIDGET(contentArea_), true);
        gtk_widget_set_valign (GTK_WIDGET(contentArea_),GTK_ALIGN_START);
        gtk_box_append(mainBox, GTK_WIDGET(contentArea_));

        auto vbox2 = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        gtk_widget_set_vexpand(GTK_WIDGET(vbox2), false);
        gtk_widget_set_valign (GTK_WIDGET(vbox2),GTK_ALIGN_END);
        gtk_box_append(mainBox, GTK_WIDGET(vbox2));
        
        actionArea_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_hexpand(GTK_WIDGET(actionArea_), false);
        gtk_widget_set_halign (GTK_WIDGET(actionArea_),GTK_ALIGN_END);
        gtk_box_append(vbox2, GTK_WIDGET(actionArea_));

        auto no = Basic::mkButton("emblem-redball", _("Cancel"));
        gtk_box_append(GTK_BOX (actionArea_),GTK_WIDGET(no));
        gtk_widget_set_halign (GTK_WIDGET(no),GTK_ALIGN_END);
        g_signal_connect(G_OBJECT (no), "clicked", G_CALLBACK(cancel), this);


        auto yes = Basic::mkButton("emblem-greenball", _("Accept"));
        gtk_box_append(GTK_BOX (actionArea_),GTK_WIDGET(yes));
        gtk_widget_set_halign (GTK_WIDGET(yes),GTK_ALIGN_END);
        g_signal_connect(G_OBJECT (yes), "clicked", G_CALLBACK(ok), this);

        return;
    }


    static void
    cancel (GtkButton *button, gpointer data) {
      auto subClass = (Dialog<dialogClass> *)data;
      auto dialog = subClass->dialog();
      g_object_set_data(G_OBJECT(dialog), "response", GINT_TO_POINTER(-1));

    }

    static void
    ok (GtkButton *button, gpointer data) {
      auto subClass = (Dialog<dialogClass> *)data;
      auto dialog = subClass->dialog();
      g_object_set_data(G_OBJECT(dialog), "response", GINT_TO_POINTER(1));
    }

  };
}
#endif

