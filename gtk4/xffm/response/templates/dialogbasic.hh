#ifndef DIALOGBASIC_HH
# define DIALOGBASIC_HH
namespace xf
{

  template <class dialogClass>
  class DialogBasic {
    
  //GtkEventControllerMotion *raiseController_ = NULL;
  GtkEventController *raiseController_ = NULL;
  GtkBox *contentArea_;
  GtkBox *actionArea_;
  GtkBox *vbox_;
  GtkBox *vbox2_;
  GtkBox *labelBox_;
  GtkBox *closeBox_;
  GtkWindow *dialog_;
  GtkWindow *parent_ = NULL;
  dialogClass *subClass_;
  GtkLabel *label_;
  pthread_cond_t cond_ = PTHREAD_COND_INITIALIZER;
  pthread_mutex_t condMutex_ = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_t mutex_ = PTHREAD_MUTEX_INITIALIZER;
  bool cancelled_ = false;
  GtkFrame *frame_;
  
  const char *path_;

    static gboolean
    on_keypress (GtkEventControllerKey* self,
          guint keyval,
          guint keycode,
          GdkModifierType state,
          gpointer data){
        bool escKey = (keyval == GDK_KEY_Escape);
        if (escKey) {
          auto object = (DialogBasic<dialogClass> *)data;
          object->cancelCallback(NULL, 1, 0., 0., data);
          return true;
        }
        return false;
    }
    void addKeyController(GtkWidget  *widget){
        auto keyController = gtk_event_controller_key_new();
        gtk_event_controller_set_propagation_phase(keyController, GTK_PHASE_CAPTURE);
        gtk_widget_add_controller(GTK_WIDGET(widget), keyController);
        g_signal_connect (G_OBJECT (keyController), "key-pressed", 
            G_CALLBACK (this->on_keypress), (void *)this);
    }
    protected:

  public:
    GtkEventController *raiseController(void){return raiseController_;}

    dialogClass *subClass(void){ return subClass_;}
    GtkWindow *parent(void){ return parent_;}
    GtkWindow *dialog(void){ return dialog_;}
    GtkBox *contentArea(void){ return contentArea_;}
    GtkBox *actionArea(void){ return actionArea_;}
    GtkLabel *label(void){ return label_;}
    GtkBox *labelBox(void){ return labelBox_;}
    GtkBox *vbox(void){ return vbox_; }
    GtkBox *vbox2(void){ return vbox2_; }
    GtkFrame *frame(){return frame_;}

    pthread_cond_t *cond_p(void){return &cond_;}
    pthread_mutex_t *condMutex_p(void){return &condMutex_;}
    bool cancelled(void){ return cancelled_;}
    void cancel(void){
      this->lockResponse();
      cancelled_ = true;
      this->unlockResponse();
    }
    void lockCondition(void){pthread_mutex_lock(&condMutex_);}
    void unlockCondition(void){pthread_mutex_unlock(&condMutex_);}
    void lockResponse(void){pthread_mutex_lock(&mutex_);}
    void unlockResponse(void){pthread_mutex_unlock(&mutex_);}
    static gboolean
    presentDialog ( GtkEventControllerMotion* self,
                    gdouble x,
                    gdouble y,
                    gpointer data) 
    {
      auto dialog = GTK_WINDOW(data);
      gtk_window_present(dialog);
      return FALSE;
    }
  private:

     void setRaise(void){
      auto content = GTK_WIDGET(g_object_get_data(G_OBJECT(parent_), "frame"));
      gtk_widget_set_sensitive(GTK_WIDGET(content), false);
      //gtk_widget_set_sensitive(GTK_WIDGET(parent_), false);
      TRACE("*** set raise for %p to %p\n", parent_, dialog_);
      raiseController_ = gtk_event_controller_motion_new();
      gtk_event_controller_set_propagation_phase(raiseController_, GTK_PHASE_CAPTURE);
      gtk_widget_add_controller(GTK_WIDGET(parent_), raiseController_);
      g_signal_connect (G_OBJECT (raiseController_), "enter", 
              G_CALLBACK (presentDialog), dialog_);      
    }
     
    static void *unsetRaise_f(void *data){
      auto object = (DialogBasic<dialogClass> *)data;
      auto content = GTK_WIDGET(g_object_get_data(G_OBJECT(object->parent()), "frame"));
      gtk_widget_set_sensitive(GTK_WIDGET(content), true);
      //gtk_widget_set_sensitive(GTK_WIDGET(object->parent()), true); 
      gtk_widget_remove_controller(GTK_WIDGET(object->parent()), 
          object->raiseController());
      // aparently not necessary:
      // g_object_unref(G_OBJECT(object->raiseController()));
      TRACE("*** set unraise for %p\n", object->parent());
      return NULL;
    }

  public:
    void setParent(GtkWindow *parent){
      parent_ = parent;
      if (parent_) {
        // only allow one subdialog (modal)
        setRaise();
      }
    }
    
    ~DialogBasic(void){
      // This is done by thread, so send all gtk/gdk stuff
      // to the main context thread.
      if (parent_) {
        Basic::context_function(unsetRaise_f, this);
        // only allow one subdialog (modal)
      }
      TRACE("*** ~DialogBasic dialog %p \n", dialog_);
      // deprecated Basic::popDialog(dialog_);
      Basic::destroy(dialog_);
      // race
      // Basic::present(GTK_WINDOW(MainWidget));

      TRACE("window %p destroyed\n", dialog_);
      delete subClass_;
      TRACE("subClass_ deleted\n");
      TRACE("destructor for %p\n", this);
    }

    DialogBasic(void){ // must run in main context.
      subClass_ = new dialogClass;
      mkWindow();
      TRACE("*** DialogBasic dialog %p\n", dialog_);
      // deprecated Basic::pushDialog(dialog_); 
      mkTitle();
      mkLabel();      
      addKeyController(GTK_WIDGET(dialog_)); 
    }

    int run(){
      TRACE("run...\n");
      pthread_t thread;
      int retval = pthread_create(&thread, NULL, runWait_f, this);
      pthread_detach(thread);
      TRACE("run detached...\n");

      return 0;
    }

    void setCloseBox(const char *iconName, const char *tooltip){
      auto old = gtk_widget_get_first_child (GTK_WIDGET(closeBox_));
      gtk_widget_unparent(old);
      auto paintable = Texture<bool>::load(iconName);
      auto image = gtk_image_new_from_paintable(paintable);
      gtk_widget_set_size_request(image, 18,18);
      gtk_widget_set_sensitive(image, true);
      Basic::boxPack0(closeBox_, GTK_WIDGET(image), true, true, 1);
      gtk_widget_set_tooltip_markup(GTK_WIDGET(closeBox_), tooltip);
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
      auto paintable = Texture<bool>::load(iconName, 24);
      if (paintable) {
        auto image = gtk_image_new_from_paintable(paintable);
        gtk_widget_set_size_request(GTK_WIDGET(image), 24, 24);
        if (image) {
          gtk_box_prepend(labelBox_, image);
        }
      }      
    }*/

    static void *runWait_f(void *data){
      auto dialogObject = (DialogBasic<dialogClass> *)data;
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
      auto dialogObject = (DialogBasic<dialogClass> *)data;
      auto dialog = dialogObject->dialog();
      void *response = NULL;
      do {
        dialogObject->lockResponse();
        response = g_object_get_data(G_OBJECT(dialog), "response");
        dialogObject->unlockResponse();
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
      if (MainWidget && GTK_IS_WINDOW (MainWidget)) {
        Basic::present(GTK_WINDOW(MainWidget));
      }
      TRACE("run_f:: Response is %p\n", response);
      // object will now be deleted.
      return response;
    }



    GtkWidget *closeBox(void){
      return Dialog::buttonBox("close", _("Close"), (void *)cancelCallback, (void *)this);
    }
protected:

    GtkWidget *cancelBox(void){
      return Dialog::buttonBox("no", _("Cancel"), (void *)cancelCallback, (void *)this);
    }
    GtkWidget *applyBox(void){
      return Dialog::buttonBox("apply", _("Apply"), (void *)ok, (void *)this);
    }
private:
    void mkWindow(void){
        dialog_ = GTK_WINDOW(gtk_window_new());
        g_object_set_data(G_OBJECT(dialog_), "dialogObject", this);
        gtk_window_set_decorated(dialog_, false);
        auto frame = GTK_FRAME(gtk_frame_new(NULL));
        g_object_set_data(G_OBJECT(dialog_), "frame", frame);
        gtk_frame_set_label_align(frame, 1.0);
        closeBox_ = GTK_BOX(closeBox());
        gtk_frame_set_label_widget(frame, GTK_WIDGET(closeBox_));
 
        gtk_window_set_child(dialog_, GTK_WIDGET(frame));
        frame_ = frame;
        
        if (parent_){
          //gtk_window_set_modal (GTK_WINDOW (dialog_), TRUE);
          gtk_window_set_transient_for (GTK_WINDOW (dialog_), GTK_WINDOW (parent_));
          gtk_window_set_destroy_with_parent(GTK_WINDOW (dialog_), true);
        }
        gtk_window_set_resizable (GTK_WINDOW (dialog_), TRUE);

        auto mainBox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        gtk_widget_set_hexpand(GTK_WIDGET(mainBox), true);
        gtk_widget_set_vexpand(GTK_WIDGET(mainBox), false);
        gtk_frame_set_child(GTK_FRAME(frame), GTK_WIDGET(mainBox));

        vbox_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        gtk_widget_set_vexpand(GTK_WIDGET(vbox_), true);
        gtk_widget_set_hexpand(GTK_WIDGET(vbox_), true);
        gtk_widget_set_valign (GTK_WIDGET(vbox_),GTK_ALIGN_CENTER);
        gtk_box_append(mainBox, GTK_WIDGET(vbox_));

        labelBox_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_vexpand(GTK_WIDGET(labelBox_), true);
        gtk_widget_set_hexpand(GTK_WIDGET(labelBox_), true);
        gtk_widget_set_valign (GTK_WIDGET(labelBox_),GTK_ALIGN_CENTER);
        gtk_box_append(vbox_, GTK_WIDGET(labelBox_));


        contentArea_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        gtk_widget_set_vexpand(GTK_WIDGET(contentArea_), true);
        gtk_widget_set_hexpand(GTK_WIDGET(contentArea_), true);
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
        vbox2_ = vbox2;

        return;
    }



    static void
    cancelCallback (GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer data) {
      auto object = (DialogBasic<dialogClass> *)data;
      auto dialog = object->dialog();
      object->lockResponse();
      g_object_set_data(G_OBJECT(dialog), "response", GINT_TO_POINTER(-1));
      object->unlockResponse();

    }

    static void
    ok (GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer data) {
      auto object = (DialogBasic<dialogClass> *)data;
      auto dialog = object->dialog();
      object->lockResponse();
      g_object_set_data(G_OBJECT(dialog), "response", GINT_TO_POINTER(1));
      object->unlockResponse();
    }

  };
  
}
#endif

