#ifndef PATHBAR_HH
#define PATHBAR_HH
namespace xf {

  
  template <class Type>
  class Pathbar : public UtilPathbar<Type>
  {
    GtkBox *pathbar_;
    gchar *path_;
    GtkWidget *back_;
    GtkWidget *next_;
    GList *historyBack_;
    GList *historyNext_;
     
  public:
   GtkBox *pathbar(void){return pathbar_;} 
   const gchar *path(void){ return path_;}
   GList *historyBack(void){ return historyBack_;}
   GList *historyNext(void){ return historyNext_;}
   void setHistoryBack(GList *historyBack){ historyBack_ = historyBack;}
   void setHistoryNext(GList *historyNext){ historyNext_ = historyNext;}
    
   void addSignals(GtkBox *eventBox, const char *path){
      g_object_set_data(G_OBJECT(eventBox), "skipMenu", GINT_TO_POINTER(1));
      
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
     //delete myPathbarMenu_;
   }
   Pathbar(void) {
        pathbar_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        //gtk_widget_set_parent(GTK_WIDGET(pathbar_), GTK_WIDGET(MainWidget));
     
//#ifdef ENABLE_MENU_CLASS
#if 0
            GtkPopover *menu = GTK_POPOVER(gtk_popover_new());
            //gtk_widget_set_parent(GTK_WIDGET(menu), GTK_WIDGET(pathbar_));
            gtk_widget_set_parent(GTK_WIDGET(menu), GTK_WIDGET(MainWidget));
            //gtk_popover_set_default_widget(menu, GTK_WIDGET(pathbar_));
            //gtk_popover_set_default_widget(menu, GTK_WIDGET(MainWidget));
            g_object_set_data(G_OBJECT(pathbar_), "menu", menu);
            auto label = gtk_label_new("foo bar");
            gtk_popover_set_child(menu, label);
#else
 /*       
        auto myPathbarMenu = new Menu<PathbarMenu>;
        auto title = g_strconcat("<span color=\"blue\">", _("Navigation Toolbar"), "</span>", NULL);
        auto menu = myPathbarMenu->getMenu(title);
        g_free(title);
        g_object_set_data(G_OBJECT(pathbar_), "menu", menu);
        // Important: must use both of the following instructions:
        //gtk_popover_set_default_widget(menu, GTK_WIDGET(pathbar_));
        gtk_widget_set_parent(GTK_WIDGET(menu), GTK_WIDGET(MainWidget));
//        gtk_widget_set_parent(GTK_WIDGET(menu), GTK_WIDGET(pathbar_));
        Util::addMenu(menu, GTK_WIDGET(pathbar_));
        //gtk_widget_realize(GTK_WIDGET(menu));
  */        
        //delete myPathbarMenu; // ???
#endif
//#endif
       
 
      

        auto eventBox1 = eventButton("xf-go-previous", "RFM_GOTO", "xffm:back", _("Previous"));
        auto eventBox2 = eventButton("xf-go-next", "RFM_GOTO", "xffm:next", _("Next"));
        auto eventBox3 = eventButton("xf-go-to", "RFM_GOTO", "xffm:goto", _("Go to"));
        back_ = GTK_WIDGET(eventBox1);
        next_ = GTK_WIDGET(eventBox2);
        g_object_set_data(G_OBJECT(pathbar_), "back", back_);
        g_object_set_data(G_OBJECT(pathbar_), "next", next_);

        gtk_widget_set_sensitive(GTK_WIDGET(eventBox1), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(eventBox2), FALSE);
        //gtk_widget_remove_css_class (GTK_WIDGET(eventBox2), "pathbarbox" );
        //gtk_widget_add_css_class (GTK_WIDGET(eventBox2), "pathbarboxInactive" );

        addSignals(eventBox1, "xffm:back");
        addSignals(eventBox2, "xffm:next");
        addSignals(eventBox3, "xffm:goto");

        Basic::boxPack0 (pathbar_, GTK_WIDGET(eventBox1), FALSE, FALSE, 0);
        Basic::boxPack0 (pathbar_, GTK_WIDGET(eventBox3), FALSE, FALSE, 0);
        Basic::boxPack0 (pathbar_, GTK_WIDGET(eventBox2), FALSE, FALSE, 0);
        


        // xffm:root button:
        auto pb_button = UtilBasic::pathbarLabelButton(".");
        g_object_set_data(G_OBJECT(pb_button), "skipMenu", GINT_TO_POINTER(1));

        
        Basic::boxPack0 (pathbar_, GTK_WIDGET(pb_button), FALSE, FALSE, 0);
        g_object_set_data(G_OBJECT(pb_button), "name", g_strdup("RFM_ROOT"));
        g_object_set_data(G_OBJECT(pb_button), "path", g_strdup("xffm:root"));

        auto motion = gtk_event_controller_motion_new();
        gtk_event_controller_set_propagation_phase(motion, GTK_PHASE_CAPTURE);
        gtk_widget_add_controller(GTK_WIDGET(pb_button), motion);
        g_signal_connect (G_OBJECT(motion) , "enter", EVENT_CALLBACK (UtilPathbar<Type>::pathbar_white), (void *)pathbar_);
        g_signal_connect (G_OBJECT(motion) , "leave", EVENT_CALLBACK (UtilPathbar<Type>::pathbar_blue), (void *)pathbar_);
    
        auto gesture1 = gtk_gesture_click_new();
        gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture1),1);
        g_signal_connect (G_OBJECT(gesture1) , "released", EVENT_CALLBACK (Workdir<Type>::pathbar_go), (void *)pathbar_);
        gtk_widget_add_controller(GTK_WIDGET(pb_button), GTK_EVENT_CONTROLLER(gesture1));
  /*     
        auto gesture3 = gtk_gesture_click_new();
        gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture3),3);
        g_signal_connect (G_OBJECT(gesture3) , "released", EVENT_CALLBACK (Workdir_c::pathbar_go), (void *)pathbar_);
        gtk_widget_add_controller(GTK_WIDGET(pb_button), GTK_EVENT_CONTROLLER(gesture3));
   */     

    }
  private:
    static gboolean
    goJump (
              GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer data ) 
    {
      TRACE("*** gojump\n");
      auto pathbar = GTK_WIDGET(data);
      auto eventBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
      auto name = (char *) g_object_get_data(G_OBJECT(eventBox), "name");
      auto path = (char *) g_object_get_data(G_OBJECT(eventBox), "path");
      auto location = GTK_WIDGET(g_object_get_data(G_OBJECT(pathbar), "location"));
      auto input = GTK_WIDGET(g_object_get_data(G_OBJECT(pathbar), "input"));
      auto promptBox = GTK_WIDGET(g_object_get_data(G_OBJECT(input), "promptBox"));
      TRACE("gojump: path=%s\n", path);
      if (strcmp(path, "xffm:back") == 0){
         GList *historyBack = (GList *)g_object_get_data(G_OBJECT(pathbar), "historyBack");
         if (!historyBack){
           TRACE("no back history List\n");
           return TRUE;
         }
         else if (historyBack->next == NULL){
           TRACE("no back history next\n");
           return TRUE;
         }
         else {
           auto current = (char *) historyBack->data;
           auto previous = (char *) historyBack->next->data;
           TRACE("Back path is %s\n", previous);
           GList *historyNext = (GList *)g_object_get_data(G_OBJECT(pathbar), "historyNext");
           historyNext = g_list_prepend(historyNext, current);
           historyBack = g_list_remove(historyBack,  current);
           for (GList *l=historyNext; l && l->data; l=l->next){
             TRACE("historyNext list = %s\n", (char *)l->data);
           }
           g_object_set_data(G_OBJECT(pathbar), "historyNext", historyNext);
           g_object_set_data(G_OBJECT(pathbar), "historyBack", historyBack);
           Workdir<Type>::setWorkdir(previous, GTK_BOX(pathbar), false);
           return TRUE;
         }
      }
      if (strcmp(path, "xffm:next") == 0){
         GList *historyNext = (GList *)g_object_get_data(G_OBJECT(pathbar), "historyNext");
         if (!historyNext){
           TRACE("no next history\n");
           return TRUE;
         }
         else {
           char *current = (char *) historyNext->data;
           TRACE("next path is %s\n", (const char *) current);
           GList *historyBack = (GList *)g_object_get_data(G_OBJECT(pathbar), "historyBack");
           historyBack = g_list_prepend(historyBack, current);
           historyNext = g_list_remove(historyNext, current);
           g_object_set_data(G_OBJECT(pathbar), "historyNext", historyNext);
           g_object_set_data(G_OBJECT(pathbar), "historyBack", historyBack);
           Workdir<Type>::setWorkdir(current, GTK_BOX(pathbar), false);
           return TRUE;
         }
      }
      if (strcmp(path, "xffm:goto") == 0){
        auto dialogObject = new DialogPrompt<jumpResponse<Type> >;
        auto dialog = dialogObject->dialog();
        dialogObject->setParent(GTK_WINDOW(MainWidget));
        dialogObject->run();

     /*   Print::clear_text(GTK_TEXT_VIEW(input));
        Print::print(GTK_TEXT_VIEW(input), g_strdup("cd ")); 
        gtk_widget_grab_focus(GTK_WIDGET(input));

        Basic::flushGTK();*/
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
    
    GtkBox *eventButton(const gchar *icon, const gchar *name, const gchar *path, const gchar *tooltip) 
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

  };
}
#endif
