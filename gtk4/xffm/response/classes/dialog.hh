#ifndef DIALOG_HH
# define DIALOG_HH

namespace xf
{
  class Dialog {
    public:

    static GtkWidget *buttonBox(const char *iconName, const char *tooltip, void *callback, void *data){

      auto box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 2));
      gtk_widget_set_vexpand(GTK_WIDGET(box), false);
      //auto picture = Texture<bool>::getPicture(iconName, 24);
      //gtk_widget_set_sensitive(GTK_WIDGET(picture), true);
      //Basic::boxPack0(box, GTK_WIDGET(picture), true, true, 1);
      auto image = Texture<bool>::getImage(iconName, 24);
      gtk_widget_set_sensitive(GTK_WIDGET(image), true);
      Basic::boxPack0(box, GTK_WIDGET(image), true, true, 1);

      Basic::setTooltip(GTK_WIDGET(box), tooltip);
      // motion
      auto motion = gtk_event_controller_motion_new();
      gtk_event_controller_set_propagation_phase(motion, GTK_PHASE_CAPTURE);
      gtk_widget_add_controller(GTK_WIDGET(box), motion);
     // click
      if (callback ){
        auto gesture = gtk_gesture_click_new();
        gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),1);
        g_signal_connect (G_OBJECT(gesture) , "released", EVENT_CALLBACK (callback), data);
        gtk_widget_add_controller(GTK_WIDGET(box), GTK_EVENT_CONTROLLER(gesture));
      }
      
      Basic::addMotionController(GTK_WIDGET(box));
      return GTK_WIDGET(box);
    }
  };
}
#endif
