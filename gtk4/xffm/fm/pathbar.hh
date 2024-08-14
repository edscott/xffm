#ifndef PATHBAR_HH
#define PATHBAR_HH
namespace xf {
  class Pathbar : public UtilPathbar
  {
    GtkBox *pathbar_;
    gchar *path_;

     
  public:
   GtkBox *pathbar(void){return pathbar_;} 
    const gchar *path(void){ return path_;}
    
   void addSignals(GtkBox *eventBox, const char *path){
      auto motionB = gtk_event_controller_motion_new();
      gtk_event_controller_set_propagation_phase(motionB, GTK_PHASE_CAPTURE);
      gtk_widget_add_controller(GTK_WIDGET(eventBox), motionB);
      g_signal_connect (G_OBJECT(motionB) , "leave", EVENT_CALLBACK (buttonPositive), NULL);
      g_signal_connect (G_OBJECT(motionB) , "enter", EVENT_CALLBACK (buttonNegative), NULL);
      auto gesture = gtk_gesture_click_new();
      gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),1);
      g_signal_connect (G_OBJECT(gesture) , "released", EVENT_CALLBACK (goJump), (void *)pathbar_);
      gtk_widget_add_controller(GTK_WIDGET(eventBox), GTK_EVENT_CONTROLLER(gesture));
   } 

   ~Pathbar(void){
    /* GList *historyList = (GList *)g_object_get_data(G_OBJECT(pathbar_), "historyList");
     if (historyList) {
       for (GList *l=historyList; l && l->data; l=l->next) g_free(l->data);
       g_list_free(historyList);
     }*/
   }
   Pathbar(void) {
        pathbar_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));

        //mkNavButtons(pathbar_);

        auto eventBox1 = eventButton("xf-go-previous", "RFM_GOTO", "xffm:back", _("Previous"));
        auto eventBox2 = eventButton("xf-go-next", "RFM_GOTO", "xffm:next", _("Next"));
        auto eventBox3 = eventButton("xf-go-to", "RFM_GOTO", "xffm:goto", _("Go to"));

        addSignals(eventBox1, "xffm:back");
        addSignals(eventBox2, "xffm:next");
        addSignals(eventBox3, "xffm:goto");

        boxPack0 (GTK_BOX (pathbar_), GTK_WIDGET(eventBox1), FALSE, FALSE, 0);
        boxPack0 (GTK_BOX (pathbar_), GTK_WIDGET(eventBox3), FALSE, FALSE, 0);
        boxPack0 (GTK_BOX (pathbar_), GTK_WIDGET(eventBox2), FALSE, FALSE, 0);
        


        // xffm:root button:
        auto pb_button = pathbarLabelButton(".");

        
        boxPack0 (GTK_BOX (pathbar_), GTK_WIDGET(pb_button), FALSE, FALSE, 0);
        g_object_set_data(G_OBJECT(pb_button), "name", g_strdup("RFM_ROOT"));
        g_object_set_data(G_OBJECT(pb_button), "path", g_strdup("xffm:root"));

    // FIXME : this iluminate background of "button".
    /*    
        g_signal_connect (G_OBJECT(pb_button) , "button-press-event", EVENT_CALLBACK (pathbar_go), (void *)this);
        */
        auto motion = gtk_event_controller_motion_new();
        gtk_event_controller_set_propagation_phase(motion, GTK_PHASE_CAPTURE);
        gtk_widget_add_controller(GTK_WIDGET(pb_button), motion);
        g_signal_connect (G_OBJECT(motion) , "enter", EVENT_CALLBACK (UtilPathbar::pathbar_white), (void *)pathbar_);
        g_signal_connect (G_OBJECT(motion) , "leave", EVENT_CALLBACK (UtilPathbar::pathbar_blue), (void *)pathbar_);
  
        auto gesture1 = gtk_gesture_click_new();
        gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture1),1);
        g_signal_connect (G_OBJECT(gesture1) , "released", EVENT_CALLBACK (UtilPathbar::pathbar_go), (void *)pathbar_);
        gtk_widget_add_controller(GTK_WIDGET(pb_button), GTK_EVENT_CONTROLLER(gesture1));
        
        auto gesture3 = gtk_gesture_click_new();
        gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture3),3);
        g_signal_connect (G_OBJECT(gesture3) , "released", EVENT_CALLBACK (UtilPathbar::pathbar_go), (void *)pathbar_);
        gtk_widget_add_controller(GTK_WIDGET(pb_button), GTK_EVENT_CONTROLLER(gesture3));
        
        //gtk_widget_show(GTK_WIDGET(pb_button));

    }
  private:
    static gboolean
    buttonNegative ( GtkEventControllerMotion* self,
                    gdouble x,
                    gdouble y,
                    gpointer data) 
    {
        auto eventBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
        gtk_widget_remove_css_class (GTK_WIDGET(eventBox), "pathbarbox" );
        gtk_widget_add_css_class (GTK_WIDGET(eventBox), "pathbarboxNegative" );
        Util::flushGTK();
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
        Util::flushGTK();
        return FALSE;
    }
    
    GtkBox *eventButton(const gchar *icon, const gchar *name, const gchar *path, const gchar *tooltip) 
    {
        auto eventBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        g_object_set_data(G_OBJECT(eventBox), "name", g_strdup(name));
        g_object_set_data(G_OBJECT(eventBox), "path", g_strdup(path));

        auto eventImage = gtk_image_new_from_icon_name(icon);
        boxPack0 (eventBox, GTK_WIDGET(eventImage), FALSE, FALSE, 0);
        gtk_widget_set_tooltip_markup(GTK_WIDGET(eventBox),tooltip);
        gtk_widget_add_css_class (GTK_WIDGET(eventBox), "pathbarbox" );
        // FIXME:
        // g_signal_connect (G_OBJECT(eventBox) , "button-press-event", EVENT_CALLBACK (callback), (void *)this);
        return eventBox;        
    }
    
    static gboolean
    goJump (
              GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer data ) 
    {
      auto eventBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
      auto name = (char *) g_object_get_data(G_OBJECT(eventBox), "name");
      auto path = (char *) g_object_get_data(G_OBJECT(eventBox), "path");
      auto pathbar = GTK_BOX(data);
      if (strcmp(path, "xffm:back") == 0){
         GList *historyBack = (GList *)g_object_get_data(G_OBJECT(pathbar), "historyBack");
         if (!historyBack){
           DBG("no back history List\n");
           return TRUE;
         }
         else if (historyBack->next == NULL){
           DBG("no back history next\n");
           return TRUE;
         }
         else {
           auto current = (char *) historyBack->data;
           auto previous = (char *) historyBack->next->data;
           DBG("Back path is %s\n", previous);
           GList *historyNext = (GList *)g_object_get_data(G_OBJECT(pathbar), "historyNext");
           historyNext = g_list_prepend(historyNext, current);
           historyBack = g_list_remove(historyBack,  current);
           for (GList *l=historyNext; l && l->data; l=l->next){
             DBG("historyNext list = %s\n", (char *)l->data);
           }
           g_object_set_data(G_OBJECT(pathbar), "historyNext", historyNext);
           g_object_set_data(G_OBJECT(pathbar), "historyBack", historyBack);
           setWorkdir(previous, pathbar, false);
           return TRUE;
         }
      }
      if (strcmp(path, "xffm:next") == 0){
         GList *historyNext = (GList *)g_object_get_data(G_OBJECT(pathbar), "historyNext");
         if (!historyNext){
           DBG("no next history\n");
           return TRUE;
         }
         else {
           char *current = (char *) historyNext->data;
           DBG("next path is %s\n", (const char *) current);
           GList *historyBack = (GList *)g_object_get_data(G_OBJECT(pathbar), "historyBack");
           historyBack = g_list_prepend(historyBack, current);
           historyNext = g_list_remove(historyNext, current);
           g_object_set_data(G_OBJECT(pathbar), "historyNext", historyNext);
           g_object_set_data(G_OBJECT(pathbar), "historyBack", historyBack);
           setWorkdir(current, pathbar, false);
           return TRUE;
         }
      }
      
      return TRUE;
    }
    static gboolean
    xgo_jump (GtkWidget *eventBox,
               GdkEvent  *event,
               gpointer   data) {
      //FIXME
/*        Pathbar *pathbar_p = (Pathbar *)data;
        // File chooser
        auto entryResponse = new(EntryResponse<Type>)(GTK_WINDOW(mainWindow), _("Go to"), "go-jump");
        auto markup = 
            g_strdup_printf("<span color=\"blue\" size=\"larger\"><b>%s</b></span>", _("Go to"));  
        
        entryResponse->setResponseLabel(markup);
        g_free(markup);

        entryResponse->setEntryLabel(_("Specify Output Directory..."));
        // get last used arguments...
        gchar *dirname = NULL;
        if (Settings<Type>::keyFileHasGroupKey("GoTo", "Default")){
            dirname = Settings<Type>::getString("GoTo", "Default");
        } 
        if (!dirname || !g_file_test(dirname, G_FILE_TEST_IS_DIR) ) {
            g_free(dirname);
            dirname = g_strdup("");
        }
        entryResponse->setEntryDefault(dirname);
        g_free(dirname);
        auto page = (Page<Type> *)pathbar_p;
        const gchar *wd = page->workDir();
        if (!wd) wd = g_get_home_dir();
        entryResponse->setEntryBashCompletion(wd);
        entryResponse->setInLineCompletion(TRUE);
        
        auto response = entryResponse->runResponse();
        DBG("response=%s\n", response);
        
        if (!response) return FALSE;
        if (strlen(response) > 1 && response[strlen(response)-1] == G_DIR_SEPARATOR){
            response[strlen(response)-1] = 0;
        }
        if (!g_file_test(response, G_FILE_TEST_IS_DIR)){
            gchar *message = g_strdup_printf("\n  %s:  \n  %s  \n", response, _("Not a directory"));
            Dialogs<Type>::quickHelp(GTK_WINDOW(mainWindow), message, "dialog-error");
            g_free(message);
        } else {
            auto view = (View<Type> *)
                g_object_get_data(G_OBJECT(page->topScrolledWindow()), "view");
            view->loadModel(response);
        }
        g_free(response);*/
        return FALSE;

    }

    

  };
}
#endif
