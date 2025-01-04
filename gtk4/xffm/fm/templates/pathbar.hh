#ifndef PATHBAR_HH
#define PATHBAR_HH
namespace xf {

  
  template <class Type>
  class Pathbar 
  {
    GtkBox *pathbar_;
    gchar *path_;
    GtkWidget *back_;
    GtkWidget *next_;
    PathbarHistory *pathbarHistory_p;
     
  public:
   GtkBox *pathbar(void){return pathbar_;} 
   const gchar *path(void){ return path_;}

   ~Pathbar(void){
     delete pathbarHistory_p;
     //delete myPathbarMenu_;
   }
   Pathbar(void) {
        pathbar_ = BasicPathbar<Type>::pathbarBox();
        g_object_set_data(G_OBJECT(pathbar_), "withMenu", GINT_TO_POINTER(1));
        g_object_set_data(G_OBJECT(pathbar_), "pathbar", this); 
        pathbarHistory_p = new PathbarHistory;
        g_object_set_data(G_OBJECT(pathbar_), "pathbarHistory_p", pathbarHistory_p); 

        auto eventBox1 = GTK_BOX(g_object_get_data(G_OBJECT(pathbar_), "back"));
        auto eventBox2 = GTK_BOX(g_object_get_data(G_OBJECT(pathbar_), "next"));
        auto eventBox3 = GTK_BOX(g_object_get_data(G_OBJECT(pathbar_), "goto"));
        auto pb_button = GTK_WIDGET(g_object_get_data(G_OBJECT(pathbar_), "pb_button"));

        auto gesture1 = gtk_gesture_click_new();
        gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture1),1);
        g_signal_connect (G_OBJECT(gesture1) , "released", 
            EVENT_CALLBACK (goJump), (void *)this);

        auto gesture2 = gtk_gesture_click_new();
        gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture2),1);
        g_signal_connect (G_OBJECT(gesture2) , "released", 
            EVENT_CALLBACK (goJump), (void *)this);

        auto gesture3 = gtk_gesture_click_new();
        gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture3),1);
        g_signal_connect (G_OBJECT(gesture3) , "released", 
            EVENT_CALLBACK (goJump), (void *)this);

        auto gesture4 = gtk_gesture_click_new();
        gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture4),1);
        g_signal_connect (G_OBJECT(gesture4) , "released", 
            EVENT_CALLBACK (Workdir<Type>::pathbar_go), (void *)pathbar_);

        gtk_widget_add_controller(GTK_WIDGET(eventBox1), GTK_EVENT_CONTROLLER(gesture1));
        gtk_widget_add_controller(GTK_WIDGET(eventBox2), GTK_EVENT_CONTROLLER(gesture2));
        gtk_widget_add_controller(GTK_WIDGET(eventBox3), GTK_EVENT_CONTROLLER(gesture3));
        gtk_widget_add_controller(GTK_WIDGET(pb_button), GTK_EVENT_CONTROLLER(gesture4));

        g_object_set_data(G_OBJECT(eventBox1), "skipMenu", GINT_TO_POINTER(1));
        g_object_set_data(G_OBJECT(eventBox2), "skipMenu", GINT_TO_POINTER(1));
        g_object_set_data(G_OBJECT(eventBox3), "skipMenu", GINT_TO_POINTER(1));
        g_object_set_data(G_OBJECT(pb_button), "skipMenu", GINT_TO_POINTER(1));

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
      TRACE("*** gojump disabled...\n");
      //return true;
      auto pathbar_p = (Pathbar<Type> *)data;

      auto pathbar = GTK_WIDGET(pathbar_p->pathbar());
      auto pathbarHistory_p = (PathbarHistory *)g_object_get_data(G_OBJECT(pathbar), "pathbarHistory_p");
      auto eventBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
      auto name = (char *) g_object_get_data(G_OBJECT(eventBox), "name");
      auto path = (char *) g_object_get_data(G_OBJECT(eventBox), "path");
      auto location = GTK_WIDGET(g_object_get_data(G_OBJECT(pathbar), "location"));
      auto input = GTK_WIDGET(g_object_get_data(G_OBJECT(pathbar), "input"));
      auto promptBox = GTK_WIDGET(g_object_get_data(G_OBJECT(input), "promptBox"));
      DBG("gojump: eventBox path=%s\n", path);
      if (strcmp(path, "xffm:back") == 0){
        auto previous = pathbarHistory_p->backHistory();
        DBG("previous = %s\n", previous);
        if (previous) Workdir<Type>::setWorkdir(previous, GTK_BOX(pathbar), false);
      }
      if (strcmp(path, "xffm:next") == 0){
        auto current = pathbarHistory_p->nextHistory();
        if (current)  Workdir<Type>::setWorkdir(current, GTK_BOX(pathbar), false);
      }
      if (strcmp(path, "xffm:goto") == 0){
        auto dialogObject = new DialogPrompt<jumpResponse<Type> >;
        auto dialog = dialogObject->dialog();
        dialogObject->setParent(GTK_WINDOW(MainWidget));
        dialogObject->run();
      }
      
      return TRUE;
    }
    
 
  };
}
#endif
