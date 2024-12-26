#ifndef FILERESPONSEPATHBAR_HH
#define FILERESPONSEPATHBAR_HH

namespace xf {
  class FileResponsePathbar {
    GtkBox *pathbar_;
    gchar *path_;
    GtkWidget *back_;
    GtkWidget *next_;
    PathbarHistory *pathbarHistory_p;

    void *reloadFunction_;
    void *reloadData_;

    void *parent_ = NULL;
    
  public:
    void parent(void *value){parent_ = value;}
    void *parent(void){return parent_;}
    
   GtkBox *pathbar(void){return pathbar_;} 
   const gchar *path(void){ return path_;}

    void *reloadFunction(void) {return reloadFunction_;}
    void *reloadData(void) {return reloadData_;}


   ~FileResponsePathbar(void){
     delete pathbarHistory_p;
     //delete myPathbarMenu_;
   }
   FileResponsePathbar(void *reloadFunction, void *reloadData) {
      reloadFunction_ = reloadFunction;
      reloadData_ = reloadData;

        pathbar_ = BasicPathbar::pathbarBox();
        g_object_set_data(G_OBJECT(pathbar_), "pathbar", this); 
        pathbarHistory_p = new PathbarHistory;
        g_object_set_data(G_OBJECT(pathbar_), "pathbarHistory_p", pathbarHistory_p); 

        auto eventBox1 = GTK_BOX(g_object_get_data(G_OBJECT(pathbar_), "back"));
        auto eventBox2 = GTK_BOX(g_object_get_data(G_OBJECT(pathbar_), "next"));
        auto eventBox3 = GTK_BOX(g_object_get_data(G_OBJECT(pathbar_), "goto"));
        // Goto disabled (we would need to put in an intermediate template,
        // as with the gridview pathbar).
        gtk_widget_set_sensitive(GTK_WIDGET(eventBox3), false);
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
            EVENT_CALLBACK (reloadFunction), reloadData);

        gtk_widget_add_controller(GTK_WIDGET(eventBox1), GTK_EVENT_CONTROLLER(gesture1));
        gtk_widget_add_controller(GTK_WIDGET(eventBox2), GTK_EVENT_CONTROLLER(gesture2));
        gtk_widget_add_controller(GTK_WIDGET(eventBox3), GTK_EVENT_CONTROLLER(gesture3));
        gtk_widget_add_controller(GTK_WIDGET(pb_button), GTK_EVENT_CONTROLLER(gesture4));

        g_object_set_data(G_OBJECT(eventBox1), "skipMenu", GINT_TO_POINTER(1));
        g_object_set_data(G_OBJECT(eventBox2), "skipMenu", GINT_TO_POINTER(1));
        g_object_set_data(G_OBJECT(eventBox3), "skipMenu", GINT_TO_POINTER(1));
        g_object_set_data(G_OBJECT(pb_button), "skipMenu", GINT_TO_POINTER(1));
        
    }
     
/*   static void 
    updatePathbar(const gchar *path, GtkBox *pathbar, bool updateHistory, void *pathbar_go_f){
        TRACE("Utilpathbar:: updatePathbar2\n");
        BasicPathbar::updatePathbar(path, pathbar, updateHistory, pathbar_go_f);
    }*/
   
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
      auto pathbar_p = (FileResponsePathbar *)data;

      auto pathbar = pathbar_p->pathbar();
      auto pathbarHistory_p = pathbar_p->pathbarHistory_p;
      auto eventBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
      DBG("gojump: \n");
      auto name = (char *) g_object_get_data(G_OBJECT(eventBox), "name");
      DBG("gojump: name=%s\n", name);
      auto path = (char *) g_object_get_data(G_OBJECT(eventBox), "path");
      DBG("gojump: path=%s\n", path);
   
      auto reload_f = pathbar_p->reloadFunction();
      auto reload_data = pathbar_p->reloadData();

      DBG("gojump: eventBox path=%s\n", path);
      if (strcmp(path, "xffm:back") == 0){
        auto previous = pathbarHistory_p->backHistory();
        DBG("previous = %s\n", previous);
        if (previous) BasicPathbar::updatePathbar(previous, pathbar, false, reload_f, reload_data);
        BasicPathbar::setRed(pathbar, previous);
        // Too convoluted:
        //auto r = (void *((*) (GtkGestureClick*, gint, gdouble, gdouble, gpointer)) )reload_f;
        
//        if (previous) Workdir<DirectoryClass>::setWorkdir(previous, GTK_BOX(pathbar), false);
      }
      if (strcmp(path, "xffm:next") == 0){
        auto current = pathbarHistory_p->nextHistory();
        if (current)  BasicPathbar::updatePathbar(current, pathbar, false, reload_f, reload_data);
        BasicPathbar::setRed(pathbar, current);
        /* nope
        auto parent = (FileResponse *)pathbar_p->parent();
        auto columnView = parent->getColumnView(path);

        auto sw = GTK_SCROLLED_WINDOW(parent->sw());
        if (columnView) gtk_scrolled_window_set_child(sw, GTK_WIDGET(columnView));
        else {
          auto label = gtk_label_new("empty");
          gtk_scrolled_window_set_child(sw, label);
        }
        */
//        if (current)  Workdir<DirectoryClass>::setWorkdir(current, GTK_BOX(pathbar), false);
      }
      if (strcmp(path, "xffm:goto") == 0){
        TRACE("xffm:goto\n");
    /*    auto dialogObject = new DialogPrompt<jumpResponse<DirectoryClass> >;
        auto dialog = dialogObject->dialog();
        dialogObject->setParent(GTK_WINDOW(MainWidget));
        dialogObject->run();*/
      }
      
      return TRUE;

    }
    
  }; 
}
#endif
