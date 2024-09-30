#ifndef DIALOG_HH
# define DIALOG_HH
namespace xf
{
  template <class dialogClass>
  class Dialog {
  GtkBox *contentArea_;
  GtkBox *actionArea_;
  GtkBox *labelBox_;
  GtkWindow *dialog_;
  GtkWindow *parent_ = NULL;
  dialogClass *subClass_;
  GtkLabel *label_;

  const char *path_;
    protected:

  public:
 
void setParent(GtkWindow *parent){
      parent_ = parent;
      if (parent) {
        TRACE("will destroy with parent\n");
        gtk_window_set_destroy_with_parent(dialog_, true);
}
    }
    
    ~Dialog(void){
      MainDialog = NULL;
      Basic::destroy(dialog_);
      // race
      // Basic::present(GTK_WINDOW(MainWidget));

      TRACE("window %p destroyed\n", dialog_);
      delete subClass_;
      TRACE("subClass_ deleted\n");
      TRACE("destructor for %p\n", this);
    }

    Dialog(void){ // must run in main context.
      subClass_ = new dialogClass;
      TRACE("Dialog::parent process = %d\n", parentProcess);
      mkWindow();
      TRACE("dialog is %p\n", dialog_);
      MainDialog = dialog_;
      mkTitle();
      mkLabel();      
    }
    dialogClass *subClass(void){ return subClass_;}
    GtkWindow *parent(void){ return parent_;}
    GtkWindow *dialog(void){ return dialog_;}
    GtkBox *contentArea(void){ return contentArea_;}
    GtkBox *actionArea(void){ return actionArea_;}
    GtkLabel *label(void){ return label_;}

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
      if (!subClass_->title()) return;
      gtk_window_set_title(dialog_, subClass_->title());
    }

    void mkLabel(void){
      label_ = GTK_LABEL(gtk_label_new(""));
      gtk_box_append(labelBox_, GTK_WIDGET(label_));
      gtk_widget_set_hexpand(GTK_WIDGET(label_), true);

      gtk_widget_set_halign (GTK_WIDGET(label_),GTK_ALIGN_CENTER);
      TRACE("subclass label is %s\n",subClass_->label());     
      setLabelText(subClass_->label());
    }

 /*   void mkIcon(void){
      const char *iconName = subClass_->iconName();
      if (!iconName) return;
      auto paintable = Texture::load(iconName, 24);
      if (paintable) {
        auto image = gtk_image_new_from_paintable(paintable);
        gtk_widget_set_size_request(GTK_WIDGET(image), 24, 24);
        if (image) {
          gtk_box_prepend(labelBox_, image);
        }
      }      
    }*/

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
        if (exitDialogs) response = GINT_TO_POINTER(-1);
        usleep(2500);
      } while (!response);
      // hide
      auto subClass = dialogObject->subClass_;
      
      if (GPOINTER_TO_INT(response) > 0){
        Basic::context_function(subClass->asyncYes, data);
      } else {
        Basic::context_function(subClass->asyncNo, data);
      }
      TRACE("run_f:: Response is %p\n", response);
      // object will now be deleted.
      return response;
    }



    GtkWidget *buttonBox(const char *iconName, const char *tooltip, void *callback){
      auto box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 2));
      auto paintable = Texture::load(iconName, 18);
      auto image = gtk_image_new_from_paintable(paintable);
      gtk_widget_set_size_request(image, 18,18);
      gtk_widget_set_sensitive(image, false);
      Basic::boxPack0(box, GTK_WIDGET(image), true, true, 1);
      gtk_widget_set_tooltip_markup(GTK_WIDGET(box), tooltip);
      // motion
      auto motion = gtk_event_controller_motion_new();
      gtk_event_controller_set_propagation_phase(motion, GTK_PHASE_CAPTURE);
      gtk_widget_add_controller(GTK_WIDGET(box), motion);
      g_signal_connect (G_OBJECT(motion) , "enter", EVENT_CALLBACK (Basic::sensitive), (void *)image);
      g_signal_connect (G_OBJECT(motion) , "leave", EVENT_CALLBACK (Basic::insensitive), (void *)image);
      // click
      auto gesture = gtk_gesture_click_new();
      gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),1);
      g_signal_connect (G_OBJECT(gesture) , "released", EVENT_CALLBACK (callback), (void *)this);
      gtk_widget_add_controller(GTK_WIDGET(box), GTK_EVENT_CONTROLLER(gesture));
      
      return GTK_WIDGET(box);
    }

    GtkWidget *closeBox(void){
      return buttonBox("close", _("Cancel"), (void *)cancel);
    }
protected:
    GtkWidget *applyBox(void){
      return buttonBox("apply", _("Apply"), (void *)ok);
    }
private:
    void mkWindow(void){
        dialog_ = GTK_WINDOW(gtk_window_new());
        gtk_window_set_decorated(dialog_, false);
        auto frame = GTK_FRAME(gtk_frame_new(NULL));
        gtk_frame_set_label_align(frame, 1.0);
        gtk_frame_set_label_widget(frame, closeBox());
 
        gtk_window_set_child(dialog_, GTK_WIDGET(frame));
        
        if (parent_){
          //gtk_window_set_modal (GTK_WINDOW (dialog_), TRUE);
          gtk_window_set_transient_for (GTK_WINDOW (dialog_), GTK_WINDOW (parent_));
          gtk_window_set_destroy_with_parent(GTK_WINDOW (dialog_), true);
        }
        gtk_window_set_resizable (GTK_WINDOW (dialog_), TRUE);

        auto mainBox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        gtk_widget_set_vexpand(GTK_WIDGET(mainBox), false);
        gtk_frame_set_child(GTK_FRAME(frame), GTK_WIDGET(mainBox));

        labelBox_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_vexpand(GTK_WIDGET(labelBox_), true);
        gtk_widget_set_valign (GTK_WIDGET(labelBox_),GTK_ALIGN_CENTER);
        gtk_box_append(mainBox, GTK_WIDGET(labelBox_));


        contentArea_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        gtk_widget_set_vexpand(GTK_WIDGET(contentArea_), true);
        gtk_widget_set_valign (GTK_WIDGET(contentArea_),GTK_ALIGN_START);
        gtk_box_append(mainBox, GTK_WIDGET(contentArea_));

        auto vbox2 = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        gtk_widget_set_vexpand(GTK_WIDGET(vbox2), false);
        gtk_widget_set_hexpand(GTK_WIDGET(vbox2), true);
        gtk_widget_set_valign (GTK_WIDGET(vbox2),GTK_ALIGN_CENTER);
        gtk_box_append(mainBox, GTK_WIDGET(vbox2));

        actionArea_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_hexpand(GTK_WIDGET(actionArea_), true);
        gtk_widget_set_halign (GTK_WIDGET(actionArea_),GTK_ALIGN_CENTER);
        gtk_box_append(vbox2, GTK_WIDGET(actionArea_));

        return;
    }


    static void
    cancel (GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer data) {
      auto object = (Dialog<dialogClass> *)data;
      auto dialog = object->dialog();
      g_object_set_data(G_OBJECT(dialog), "response", GINT_TO_POINTER(-1));

    }

    static void
    ok (GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer data) {
      auto object = (Dialog<dialogClass> *)data;
      auto dialog = object->dialog();
      g_object_set_data(G_OBJECT(dialog), "response", GINT_TO_POINTER(1));
    }

  };

  template <class dialogClass>
  class DialogTimeout: public Dialog<dialogClass> {
    time_t timeout_;
    GtkProgressBar *timeoutProgress_;
public:
    GtkProgressBar *progress(void){return timeoutProgress_;}
    time_t timeout(void){return timeout_;}
    void timeout(time_t value){timeout_ = value;}


    ~DialogTimeout(void){ 
      
    }
    DialogTimeout(void){ 
      // progress bar timeout       
      timeoutProgress_ = GTK_PROGRESS_BAR(gtk_progress_bar_new());
      Basic::boxPack0 (GTK_BOX (this->actionArea()),GTK_WIDGET(timeoutProgress_), TRUE, TRUE, 0);
      
      g_timeout_add(500, updateProgress, (void *)this);

    
      auto keyController = gtk_event_controller_key_new();
      gtk_event_controller_set_propagation_phase(keyController, GTK_PHASE_CAPTURE);
      gtk_widget_add_controller(GTK_WIDGET(this->dialog()), keyController);
      g_signal_connect (G_OBJECT (keyController), "key-pressed", 
          G_CALLBACK (this->on_keypress), (void *)this);
    }

    int run(){ // overload
      TRACE("run...\n");
      pthread_t thread;
      int retval = pthread_create(&thread, NULL, runWait_f, this);
      pthread_detach(thread);
      TRACE("run detached...\n");

      return 0;
    }

    static void *runWait_f(void *data){
      auto dialogObject = (DialogTimeout<dialogClass> *)data;
      auto dialog = dialogObject->dialog();

      TRACE("runWait_f...\n");
      pthread_t thread;
      int retval = pthread_create(&thread, NULL, run_f, (void *)dialogObject);
      void *response_p;
      pthread_join(thread, &response_p);
      TRACE("run joined, *response_p = %p\n", response_p);
      // Here we set timeout to -3 
      dialogObject->timeout(-3);
      // delete dialogObject;
      return response_p;
    }

    static void *run_f(void *data){
      auto dialogObject = (Dialog<dialogClass> *)data;
      auto dialog = dialogObject->dialog();
      void *response = NULL;
      do {
        response = g_object_get_data(G_OBJECT(dialog), "response");
        if (exitDialogs) response = GINT_TO_POINTER(-1);
        usleep(2500);
      } while (!response);
      // hide
      auto subClass = dialogObject->subClass();
      
      if (GPOINTER_TO_INT(response) > 0){
        Basic::context_function(subClass->asyncYes, data);
      } else {
        Basic::context_function(subClass->asyncNo, data);
      }
      TRACE("run_f:: Response is %p\n", response);
      // object will now be deleted.
      return response;
    }

    static gboolean
    on_keypress (GtkEventControllerKey* self,
          guint keyval,
          guint keycode,
          GdkModifierType state,
          gpointer data){

       auto object = (DialogTimeout<dialogClass> *)data;
       auto progress = object->progress();
        if (!progress || !GTK_IS_PROGRESS_BAR(progress)) return FALSE;
        gtk_progress_bar_set_fraction(progress, 0.0);
        if(keyval == GDK_KEY_Escape) { 
          g_object_set_data(G_OBJECT(object->dialog()), "response", GINT_TO_POINTER(-2)); 
          return TRUE;
        }
        if(keyval == GDK_KEY_Return) { 
          g_object_set_data(G_OBJECT(object->dialog()), "response", GINT_TO_POINTER(2)); 
          return TRUE;
        }
        return FALSE;      
    }

    static gboolean
    updateProgress(void * data){
        auto object = (DialogTimeout<dialogClass> *)data;
        auto arg = (void **)data;
        auto timeout = object->timeout();
        auto progress = object->progress();
        auto dialog  = object->dialog();
            
        // Shortcircuit to delete.
        if (timeout < 0) {
          TRACE("timeout done...\n");
          delete object;
          return G_SOURCE_REMOVE;
        }

        if (!GTK_IS_WINDOW(dialog))  return G_SOURCE_REMOVE;
        if (!GTK_IS_PROGRESS_BAR(progress))  return G_SOURCE_REMOVE;

        // While window is active, pause progress bar.
        if (gtk_window_is_active(GTK_WINDOW(dialog))){
          TRACE("gtk_window_is_active\n");
          return G_SOURCE_CONTINUE;
        }
        TRACE("timeout = %lf\n", timeout);

        // Add fraction to progress bar.
        auto fraction = gtk_progress_bar_get_fraction(progress);
        if (fraction < 1.0) {
            fraction += (1.0 / 30.0);
            TRACE("gtk_progress_bar_set_fraction %lf\n", fraction);
            gtk_progress_bar_set_fraction(progress, fraction);
        } 
        // Complete fraction, cancel dialog and .
        if (fraction >= 1.0) {
          DBG("cancel dialog by timeout\n");
          //object->timeout(-3); // remove timeout 
          g_object_set_data(G_OBJECT(dialog), "response", GINT_TO_POINTER(-3)); 
          delete object;
          return G_SOURCE_REMOVE;
        }
        return G_SOURCE_CONTINUE;
    }
    
  };

  template <class dialogClass>
  class DialogEntry : public Dialog<dialogClass>{
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

