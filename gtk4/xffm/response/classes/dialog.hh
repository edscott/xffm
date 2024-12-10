#ifndef DIALOG_HH
# define DIALOG_HH

namespace xf
{
  class Dialog {
    public:

    static GtkWidget *buttonBox(const char *iconName, const char *tooltip, void *callback, void *data){

      auto box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 2));
      auto paintable = Texture<bool>::load(iconName);
      auto image = gtk_image_new_from_paintable(paintable);
      gtk_widget_set_size_request(image, 25, 25); 
      //gtk_widget_set_sensitive(image, false);
      gtk_widget_set_sensitive(image, true);
      Basic::boxPack0(box, GTK_WIDGET(image), true, true, 1);
      gtk_widget_set_tooltip_markup(GTK_WIDGET(box), tooltip);
      // motion
      auto motion = gtk_event_controller_motion_new();
      gtk_event_controller_set_propagation_phase(motion, GTK_PHASE_CAPTURE);
      gtk_widget_add_controller(GTK_WIDGET(box), motion);
      // signal connect incompatible with setCloseBox()
      //g_signal_connect (G_OBJECT(motion) , "enter", EVENT_CALLBACK (Basic::sensitive), (void *)image);
      //g_signal_connect (G_OBJECT(motion) , "leave", EVENT_CALLBACK (Basic::insensitive), (void *)image);
     // click
      auto gesture = gtk_gesture_click_new();
      gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),1);
      g_signal_connect (G_OBJECT(gesture) , "released", EVENT_CALLBACK (callback), data);
      gtk_widget_add_controller(GTK_WIDGET(box), GTK_EVENT_CONTROLLER(gesture));
      
      Basic::addMotionController(GTK_WIDGET(box));
      return GTK_WIDGET(box);
    }
  };
}
#endif
