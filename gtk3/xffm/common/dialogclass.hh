#ifndef DIALOGCLASS_HH
# define DIALOGCLASS_HH
namespace xf
{
  class DialogClass {
    GtkDialog *dialog_;
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
    }

    Dialog(GtkDialog *dialog){ // must run in main context.
      dialog_ = dialog;
    }
 
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


  
}
#endif

