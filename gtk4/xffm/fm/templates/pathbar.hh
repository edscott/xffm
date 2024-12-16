#ifndef PATHBAR_HH
#define PATHBAR_HH
namespace xf {

  
  template <class DirectoryClass>
  class Pathbar : public UtilPathbar<DirectoryClass>, public PathbarHistory<DirectoryClass>
  {
    GtkBox *pathbar_;
    gchar *path_;
     
  public:
   GtkBox *pathbar(void){return pathbar_;} 
   const gchar *path(void){ return path_;}
 
   ~Pathbar(void){
     //delete myPathbarMenu_;
   }
   Pathbar(void) {
     // Default goJump functionality.
      pathbar_ = createPathbar((void *)Workdir<DirectoryClass>::pathbar_go, (void *)goJump, (void *)this);
      g_object_set_data(G_OBJECT(pathbar_), "pathbarHistory", this);
      g_object_set_data(G_OBJECT(pathbar_), "pathbar_p", this);
   }
   Pathbar(void *goFunction, void *jumpFunction) {
     // Alternate class template constructor with specific jumpFunction.
      pathbar_ = createPathbar(goFunction, jumpFunction, (void *)this);
      g_object_set_data(G_OBJECT(pathbar_), "pathbarHistory", this);
      g_object_set_data(G_OBJECT(pathbar_), "pathbar_p", this);
   }

   static GtkBox *createPathbar(void *goFunction, void *jumpFunction, void *jumpData){
     // goFunction() is executed on click of a pathbar eventBox.
     // jumpFunction() is executed for back/next/goto buttons.
      auto pathbarBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
      auto eventBox1 = eventButton("xf-go-previous", "RFM_GOTO", "xffm:back", _("Previous"));
      auto eventBox2 = eventButton("xf-go-next", "RFM_GOTO", "xffm:next", _("Next"));
      auto eventBox3 = eventButton("xf-go-to", "RFM_GOTO", "xffm:goto", _("Go to"));
      auto back = GTK_WIDGET(eventBox1);
      auto next = GTK_WIDGET(eventBox2);
      g_object_set_data(G_OBJECT(pathbarBox), "back", back);
      g_object_set_data(G_OBJECT(pathbarBox), "next", next);

      gtk_widget_set_sensitive(GTK_WIDGET(eventBox1), FALSE);
      gtk_widget_set_sensitive(GTK_WIDGET(eventBox2), FALSE);

      addSignals(eventBox1, jumpFunction, jumpData);
      addSignals(eventBox2, jumpFunction, jumpData);
      addSignals(eventBox3, jumpFunction, jumpData);


      Basic::boxPack0 (pathbarBox, GTK_WIDGET(eventBox1), FALSE, FALSE, 0);
      Basic::boxPack0 (pathbarBox, GTK_WIDGET(eventBox3), FALSE, FALSE, 0);
      Basic::boxPack0 (pathbarBox, GTK_WIDGET(eventBox2), FALSE, FALSE, 0);
      
      // bookmarks button:
      auto pb_button = UtilBasic::pathbarLabelButton(".");
      g_object_set_data(G_OBJECT(pb_button), "skipMenu", GINT_TO_POINTER(1));
      
      Basic::boxPack0 (pathbarBox, GTK_WIDGET(pb_button), FALSE, FALSE, 0);
      g_object_set_data(G_OBJECT(pb_button), "name", g_strdup("RFM_ROOT"));
      g_object_set_data(G_OBJECT(pb_button), "path", g_strdup(_("Bookmarks")));

      auto motion = gtk_event_controller_motion_new();
      gtk_event_controller_set_propagation_phase(motion, GTK_PHASE_CAPTURE);
      gtk_widget_add_controller(GTK_WIDGET(pb_button), motion);
      g_signal_connect (G_OBJECT(motion) , "enter", EVENT_CALLBACK (UtilPathbar<DirectoryClass>::pathbar_white), (void *)pathbarBox);
      g_signal_connect (G_OBJECT(motion) , "leave", EVENT_CALLBACK (UtilPathbar<DirectoryClass>::pathbar_blue), (void *)pathbarBox);
  
      auto gesture1 = gtk_gesture_click_new();
      gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture1),1);
      g_signal_connect (G_OBJECT(gesture1) , "released", EVENT_CALLBACK (goFunction), (void *)pathbarBox);
      gtk_widget_add_controller(GTK_WIDGET(pb_button), GTK_EVENT_CONTROLLER(gesture1));
      return pathbarBox;
   }
  private:
    static gboolean
    goJump (
              GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              void *data ) 
    {
      TRACE("*** gojump\n");
      auto pathbar_p = (Pathbar<DirectoryClass> *)data;

      auto pathbar = GTK_WIDGET(pathbar_p->pathbar());
      auto eventBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
      auto name = (char *) g_object_get_data(G_OBJECT(eventBox), "name");
      auto path = (char *) g_object_get_data(G_OBJECT(eventBox), "path");
      auto location = GTK_WIDGET(g_object_get_data(G_OBJECT(pathbar), "location"));
      auto input = GTK_WIDGET(g_object_get_data(G_OBJECT(pathbar), "input"));
      auto promptBox = GTK_WIDGET(g_object_get_data(G_OBJECT(input), "promptBox"));
      TRACE("gojump: path=%s\n", path);
      if (strcmp(path, "xffm:back") == 0){
        auto previous = pathbar_p->backHistory();
        if (previous) Workdir<DirectoryClass>::setWorkdir(previous, GTK_BOX(pathbar), false);
      }
      if (strcmp(path, "xffm:next") == 0){
        auto current = pathbar_p->nextHistory();
        if (current)  Workdir<DirectoryClass>::setWorkdir(current, GTK_BOX(pathbar), false);
      }
      if (strcmp(path, "xffm:goto") == 0){
        auto dialogObject = new DialogPrompt<jumpResponse<DirectoryClass> >;
        auto dialog = dialogObject->dialog();
        dialogObject->setParent(GTK_WINDOW(MainWidget));
        dialogObject->run();
      }
      
      return TRUE;
    }
    static gboolean
    buttonNegative ( GtkEventControllerMotion* self,
                    gdouble x,
                    gdouble y,
                    gpointer data) 
    {
        auto eventBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
        gtk_widget_remove_css_class (GTK_WIDGET(eventBox), "pathbarbox" );
        gtk_widget_add_css_class (GTK_WIDGET(eventBox), "pathbarboxNegative" );
        Basic::flushGTK();
        return FALSE;
    }
    static gboolean
    buttonPositive ( GtkEventControllerMotion* self,
                    gdouble x,
                    gdouble y,
                    gpointer data) 
    {
        auto eventBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
        gtk_widget_remove_css_class (GTK_WIDGET(eventBox), "pathbarboxNegative" );
        gtk_widget_add_css_class (GTK_WIDGET(eventBox), "pathbarbox" );
        Basic::flushGTK();
        return FALSE;
    }
    
    static GtkBox *eventButton(const gchar *icon, const gchar *name, const gchar *path, const gchar *tooltip) 
    {
        auto eventBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        g_object_set_data(G_OBJECT(eventBox), "name", g_strdup(name));
        g_object_set_data(G_OBJECT(eventBox), "path", g_strdup(path));

        auto eventImage = gtk_image_new_from_icon_name(icon);
        Basic::boxPack0 (eventBox, GTK_WIDGET(eventImage), FALSE, FALSE, 0);
        gtk_widget_set_tooltip_markup(GTK_WIDGET(eventBox),tooltip);
        gtk_widget_add_css_class (GTK_WIDGET(eventBox), "pathbarbox" );
        return eventBox;        
    }
    
    static void addSignals(GtkBox *eventBox, void *jumpFunction, void *jumpData){
      g_object_set_data(G_OBJECT(eventBox), "skipMenu", GINT_TO_POINTER(1));
      
      auto motionB = gtk_event_controller_motion_new();
      gtk_event_controller_set_propagation_phase(motionB, GTK_PHASE_CAPTURE);
      gtk_widget_add_controller(GTK_WIDGET(eventBox), motionB);
      g_signal_connect (G_OBJECT(motionB) , "leave", EVENT_CALLBACK (buttonPositive), NULL);
      g_signal_connect (G_OBJECT(motionB) , "enter", EVENT_CALLBACK (buttonNegative), NULL);

      auto gesture = gtk_gesture_click_new();
      gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),1);
      g_signal_connect (G_OBJECT(gesture) , "released", EVENT_CALLBACK (jumpFunction), jumpData);
      gtk_widget_add_controller(GTK_WIDGET(eventBox), GTK_EVENT_CONTROLLER(gesture));
   } 

  };
}
#endif
