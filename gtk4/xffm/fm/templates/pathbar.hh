#ifndef PATHBAR_HH
#define PATHBAR_HH
namespace xf {

  
  template <class DirectoryClass>
  class Pathbar : public UtilPathbar<DirectoryClass>, public PathbarHistory<DirectoryClass>
  {
    GtkBox *pathbar_;
    gchar *path_ = NULL;
    void *reloadFunction_ = NULL;
    void *reloadData_ = NULL;
    void *jumpFunction_ = NULL;
    void *jumpData_ = NULL;
    GtkBox *eventBox1_;     
    GtkBox *eventBox2_;     
    GtkBox *eventBox3_;     
    GtkBox *bookmarkButton_;     
  public:


    void reloadFunction(void *value){reloadFunction_ = value;}
    void reloadData(void *value){reloadData_ = value;}
    void jumpFunction(void *value){jumpFunction_ = value;}
    void jumpData(void *value){jumpData_ = value;}

   GtkBox *pathbar(void){return pathbar_;} 
   const gchar *path(void){ return path_;}
   void path(const char *value){
     g_free(path_);
     if (!value){
       path_ = g_strdup(g_get_home_dir());
       return;
     }

     //Nonexisting paths, use homedir
     if (!g_file_test(value, G_FILE_TEST_EXISTS)) {
       path_ = g_strdup(g_get_home_dir());
       return;
     } 
     path_ = g_strdup(value);
   }
 
   ~Pathbar(void){
      g_free(path_);
     //delete myPathbarMenu_;
   }
   Pathbar(void) {
     // Default goJump functionality.
     // createPathbar with 1 argument
      reloadData_ = this;
      jumpFunction_ = (void *)goJump;
      jumpData_ = this;
      pathbar_ = createPathbar((void *)Workdir<DirectoryClass>::pathbar_go);
      g_object_set_data(G_OBJECT(pathbar_), "pathbarHistory", this);
      g_object_set_data(G_OBJECT(pathbar_), "pathbar_p", this);
      TRACE("*** pathbar constructor, goJump=%p pathbar_p=%p\n", goJump, this);
      TRACE("*** Pathbar...historyBack_= %p, historyNext_=%p\n",
              this->historyBack_, this->historyNext_);
   }
   Pathbar(void *goFunction, void *goFunctionData, void *jumpFunction, void *jumpData) {
     // Alternate class template constructor with specific goFunction.
     // createPathbar with 2 arguments
      jumpFunction_ = jumpFunction;
      jumpData_ = jumpData;
      pathbar_ = createPathbar(goFunction, goFunctionData);
      g_object_set_data(G_OBJECT(pathbar_), "pathbarHistory", this);
      g_object_set_data(G_OBJECT(pathbar_), "pathbar_p", this);
   }

  private:

   GtkBox *newPathbarBox(void){
      auto pathbarBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
      g_object_set_data(G_OBJECT(pathbarBox), "pathbar_p", this);
      eventBox1_ = eventButton("xf-go-previous", "RFM_GOTO", "xffm:back", _("Previous"));
      eventBox2_ = eventButton("xf-go-next", "RFM_GOTO", "xffm:next", _("Next"));
      eventBox3_ = eventButton("xf-go-to", "RFM_GOTO", "xffm:goto", _("Go to"));
      auto back = GTK_WIDGET(eventBox1_);
      auto next = GTK_WIDGET(eventBox2_);
      g_object_set_data(G_OBJECT(pathbarBox), "back", back);
      g_object_set_data(G_OBJECT(pathbarBox), "next", next);

      gtk_widget_set_sensitive(GTK_WIDGET(eventBox1_), FALSE);
      gtk_widget_set_sensitive(GTK_WIDGET(eventBox2_), FALSE);

      Basic::boxPack0 (pathbarBox, GTK_WIDGET(eventBox1_), FALSE, FALSE, 0);
      Basic::boxPack0 (pathbarBox, GTK_WIDGET(eventBox3_), FALSE, FALSE, 0);
      Basic::boxPack0 (pathbarBox, GTK_WIDGET(eventBox2_), FALSE, FALSE, 0);
      
      // bookmarks button:
      bookmarkButton_ = UtilBasic::pathbarLabelButton(".");
      g_object_set_data(G_OBJECT(bookmarkButton_), "skipMenu", GINT_TO_POINTER(1));
      
      Basic::boxPack0 (pathbarBox, GTK_WIDGET(bookmarkButton_), FALSE, FALSE, 0);
      g_object_set_data(G_OBJECT(bookmarkButton_), "name", g_strdup("RFM_ROOT"));
      g_object_set_data(G_OBJECT(bookmarkButton_), "path", g_strdup(_("Bookmarks")));

      auto motion = gtk_event_controller_motion_new();
      gtk_event_controller_set_propagation_phase(motion, GTK_PHASE_CAPTURE);
      gtk_widget_add_controller(GTK_WIDGET(bookmarkButton_), motion);
      g_signal_connect (G_OBJECT(motion) , "enter", EVENT_CALLBACK (UtilPathbar<DirectoryClass>::pathbar_white), (void *)pathbarBox);
      g_signal_connect (G_OBJECT(motion) , "leave", EVENT_CALLBACK (UtilPathbar<DirectoryClass>::pathbar_blue), (void *)pathbarBox);
      return pathbarBox;
  
   } 

   GtkBox *createPathbar(void *goFunction){
     // goFunction() is executed on click of a pathbar eventBox.
     // jumpFunction() is executed for back/next/goto buttons.
      auto pathbarBox = newPathbarBox();
DBG("*** createPathbar1\n");
      reloadFunction_ = goFunction;

      addSignals(eventBox1_, jumpFunction_, jumpData_);
      addSignals(eventBox2_, jumpFunction_, jumpData_);
      addSignals(eventBox3_, jumpFunction_, jumpData_);
  
      auto gesture1 = gtk_gesture_click_new();
      gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture1),1);
      g_signal_connect (G_OBJECT(gesture1) , "released", EVENT_CALLBACK (reloadFunction_), this);
      gtk_widget_add_controller(GTK_WIDGET(bookmarkButton_), GTK_EVENT_CONTROLLER(gesture1));
      return pathbarBox;
   }

   GtkBox *createPathbar(void *goFunction, void *goFunctionData){
      reloadFunction_ = goFunction;
      reloadData_ = goFunctionData;
DBG("*** createPathbar2 reloadFunction_=%p, reloadData_=%p\n", reloadFunction_, reloadData_);

      auto pathbarBox = newPathbarBox();

      addSignals(eventBox1_, jumpFunction_, jumpData_);
      addSignals(eventBox2_, jumpFunction_, jumpData_);
      addSignals(eventBox3_, jumpFunction_, jumpData_);
DBG("*** createPathbar2 addSignals\n");

      auto gesture1 = gtk_gesture_click_new();
      gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture1),1);
      g_signal_connect (G_OBJECT(gesture1) , "released", EVENT_CALLBACK (reloadFunction_), reloadData_);
      gtk_widget_add_controller(GTK_WIDGET(bookmarkButton_), GTK_EVENT_CONTROLLER(gesture1));
DBG("*** createPathbar2 gesture1\n");
  
      return pathbarBox;
    
    }

    static gboolean
    goJump (
              GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              void *data ) 
    {
      DBG("*** gojump pathbar_p = %p\n",data);
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
        if (previous) Workdir<DirectoryClass>::setWorkdir(previous, pathbar_p, false);
//        if (previous) Workdir<DirectoryClass>::setWorkdir(previous, GTK_BOX(pathbar), false);
      }
      if (strcmp(path, "xffm:next") == 0){
        auto current = pathbar_p->nextHistory();
        if (current)  Workdir<DirectoryClass>::setWorkdir(current, pathbar_p, false);
//        if (current)  Workdir<DirectoryClass>::setWorkdir(current, GTK_BOX(pathbar), false);
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
      if (!jumpFunction) return;
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
