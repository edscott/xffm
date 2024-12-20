#ifndef FILERESPONSEPATHBAR_HH
#define FILERESPONSEPATHBAR_HH

namespace xf {

  class FileResponsePathbar {
  private:
    GtkBox *pathbar_;
    gchar *path_ =  NULL;
    gchar *workdir_;
    GtkWidget *back_;
    GtkWidget *next_;
    GList *historyBack_;
    GList *historyNext_;

    void *reloadFunction_;
    void *reloadData_;

  public:
    void reloadFunction(void *value){reloadFunction_ = value;}
    void reloadData(void *value){reloadData_ = value;}

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

   GList *historyBack(void){ return historyBack_;}
   GList *historyNext(void){ return historyNext_;}
   void setHistoryBack(GList *historyBack){ historyBack_ = historyBack;}
   void setHistoryNext(GList *historyNext){ historyNext_ = historyNext;}
   
   
    FileResponsePathbar(void *reloadFunction, void *reloadData) {
      reloadFunction_ = reloadFunction;
      reloadData_ = reloadData;
    //FileResponsePathbar(void) {

      pathbar_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
      gtk_widget_set_hexpand(GTK_WIDGET(pathbar_), false);
      gtk_widget_set_vexpand(GTK_WIDGET(pathbar_), false);

      auto eventBox1 = eventButton("xf-go-previous", "RFM_GOTO", "xffm:back", _("Previous"));
      auto eventBox2 = eventButton("xf-go-next", "RFM_GOTO", "xffm:next", _("Next"));
      auto eventBox3 = eventButton("xf-go-to", "RFM_GOTO", "xffm:goto", _("Go to"));
      back_ = GTK_WIDGET(eventBox1);
      next_ = GTK_WIDGET(eventBox2);
      g_object_set_data(G_OBJECT(pathbar_), "back", back_);
      g_object_set_data(G_OBJECT(pathbar_), "next", next_);

      gtk_widget_set_sensitive(GTK_WIDGET(eventBox1), FALSE);
      gtk_widget_set_sensitive(GTK_WIDGET(eventBox2), FALSE);

      addSignals(eventBox1, "xffm:back");
      addSignals(eventBox2, "xffm:next");
      addSignals(eventBox3, "xffm:goto");

      Basic::boxPack0 (pathbar_, GTK_WIDGET(eventBox1), FALSE, FALSE, 0);
      Basic::boxPack0 (pathbar_, GTK_WIDGET(eventBox3), FALSE, FALSE, 0);
      Basic::boxPack0 (pathbar_, GTK_WIDGET(eventBox2), FALSE, FALSE, 0);

      // bookmarks button:
      auto pb_button = UtilBasic::pathbarLabelButton(".");
      gtk_widget_add_css_class (GTK_WIDGET(back_), "pathbarbox" );          
      gtk_widget_add_css_class (GTK_WIDGET(next_), "pathbarbox" );          
      gtk_widget_add_css_class (GTK_WIDGET(pb_button), "pathbarbox" );          

      Basic::boxPack0 (pathbar_, GTK_WIDGET(pb_button), FALSE, FALSE, 0);
      g_object_set_data(G_OBJECT(pb_button), "name", g_strdup("RFM_ROOT"));
      g_object_set_data(G_OBJECT(pb_button), "path", g_strdup(_("Bookmarks")));

            auto gesture1 = gtk_gesture_click_new();
            gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture1),1);
            g_signal_connect (G_OBJECT(gesture1) , "released", EVENT_CALLBACK (reloadFunction_), reloadData_);
            gtk_widget_add_controller(GTK_WIDGET(pb_button), GTK_EVENT_CONTROLLER(gesture1));

      auto motion = gtk_event_controller_motion_new();
      gtk_event_controller_set_propagation_phase(motion, GTK_PHASE_CAPTURE);
      gtk_widget_add_controller(GTK_WIDGET(pb_button), motion);
      g_signal_connect (G_OBJECT(motion) , "enter", EVENT_CALLBACK (pathbar_white), (void *)this);
      g_signal_connect (G_OBJECT(motion) , "leave", EVENT_CALLBACK (pathbar_blue), (void *)this);
        
    
 /*       auto gesture1 = gtk_gesture_click_new();
        gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture1),1);
        g_signal_connect (G_OBJECT(gesture1) , "released", EVENT_CALLBACK (Workdir<DirectoryClass>::pathbar_go), (void *)pathbar_);
        gtk_widget_add_controller(GTK_WIDGET(pb_button), GTK_EVENT_CONTROLLER(gesture1));
 
*/
    }

    ~FileResponsePathbar(){
      g_free(path_);
      // free history
    }


    void updatePathbarBox(const char *path, bool updateHistory, void *pathbar_go_f){

      // Here path may differ from path_
/*
        if (updateHistory) {
          
          GList *historyBack = (GList *)g_object_get_data(G_OBJECT(pathbar_), "historyBack");
          if (historyBack){
            if (strcmp(path, (const char *)historyBack->data) != 0){
              historyBack = g_list_prepend(historyBack, g_strdup(path));
              TRACE("updatePathbar: 1 updating historyBack with path = %s\n", path);
            }
          } else {
              historyBack = g_list_prepend(historyBack, g_strdup(path));
              TRACE("updatePathbar: 2 updating historyBack with path = %s\n", path);
          }
          g_object_set_data(G_OBJECT(pathbar_), "historyBack", historyBack);
          // wipe next history 
          GList *historyNext = (GList *)g_object_get_data(G_OBJECT(pathbar_), "historyNext");
          for (GList *l=historyNext; l && l->data; l=l->next) g_free(l->data);
          g_list_free(historyNext);
          g_object_set_data(G_OBJECT(pathbar_), "historyNext", NULL);
        }
*/
         
        // Get basenames
        gchar **paths;
        if (strcmp(path, G_DIR_SEPARATOR_S)==0){
            paths = (gchar **)calloc(2, sizeof(gchar *));
            if (!paths){
                g_warning("updatePathbar(): cannot malloc\n");
                return ;
            }
            paths[1]=NULL;
        } else {
            paths = g_strsplit(path, G_DIR_SEPARATOR_S, -1);
            g_free(paths[0]);
        }
        paths[0]= g_strdup(G_DIR_SEPARATOR_S);

        // Get previous children of the pathbar
        GList *children_list = Basic::getChildren(pathbar_);
        gint i=0;
        gchar *pb_path = NULL;
        for (GList *children = children_list;children && children->data; children=children->next){
            gchar *name = (gchar *)g_object_get_data(G_OBJECT(children->data), "name");
            if (!name){
              DBG("***Error utilpathbar.hh name is not set (should not happen)\n");
              continue;
            }
            // Leave back, next and bookmark buttons untouched:
            if (strcmp(name, "RFM_ROOT")==0||strcmp(name, "RFM_GOTO")==0) continue;

            TRACE( "(%d) comparing %s <--> %s\n", i, name, paths[i]);
            if (paths[i] && strcmp(name, paths[i]) == 0){
                g_free(pb_path);
                const gchar *p = (const gchar *)g_object_get_data(G_OBJECT(children->data), "path");
                pb_path = g_strdup(p);
                i++; 
                continue;
            }

            if (paths[i] == NULL) break;
            // Eliminate tail (only if tail will differ)
            TRACE( "Zapping tail: \"%s\"\n", paths[i]);
            GList *tail = children;
            for (;tail && tail->data; tail = tail->next){
                gchar *name  = (gchar *)g_object_get_data(G_OBJECT(tail->data), "name");
                gchar *path  = (gchar *)g_object_get_data(G_OBJECT(tail->data), "path");
                TRACE( "Zapping tail item: \"%s\"\n", name);
                g_free(name);
                g_free(path);
                gtk_widget_unparent(GTK_WIDGET(tail->data));                
            }
            break;
        }
        g_list_free(children_list);

        // Add new tail
        for (;paths[i]; i++){
            auto pb_button = 
                UtilBasic::pathbarLabelButton(strlen(paths[i])?paths[i]:G_DIR_SEPARATOR_S);

            auto gesture1 = gtk_gesture_click_new();
            gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture1),1);
            g_signal_connect (G_OBJECT(gesture1) , "released", EVENT_CALLBACK (reloadFunction_), reloadData_);
            gtk_widget_add_controller(GTK_WIDGET(pb_button), GTK_EVENT_CONTROLLER(gesture1));
   


            Basic::boxPack0 (pathbar_, GTK_WIDGET(pb_button), FALSE, FALSE, 0);
            gchar *g = (pb_path!=NULL)?
                g_strdup_printf("%s%s%s",pb_path, 
                        strcmp(pb_path,G_DIR_SEPARATOR_S)? 
                        G_DIR_SEPARATOR_S:"", paths[i]):
                g_strdup(paths[i]);
            g_free(pb_path);
            pb_path = g;

            TRACE( "+++***** setting pbpath %s --> %s\n", paths[i], pb_path);
            g_object_set_data(G_OBJECT(pb_button), "path", g_strdup(pb_path));
            
            TRACE("adding motion for %s (%s)\n", paths[i], pb_path)
            auto motion = gtk_event_controller_motion_new();
            gtk_event_controller_set_propagation_phase(motion, GTK_PHASE_CAPTURE);
            gtk_widget_add_controller(GTK_WIDGET(pb_button), motion);
            g_signal_connect (G_OBJECT(motion) , "enter", EVENT_CALLBACK (pathbar_white), (void *)this);
            g_signal_connect (G_OBJECT(motion) , "leave", EVENT_CALLBACK (pathbar_blue), (void *)this);
            gtk_widget_set_visible(GTK_WIDGET(pb_button), TRUE);           


        }
        g_free(pb_path);
        g_strfreev(paths);

        
        // show what fits, differentiate active button.
        togglePathbar(path);

        // finally, add the menu item for each pathbar_ item.
        // This should only be done if no menu is already set
        // for the item.
        // Also, should skip back, next and goto buttons.
        children_list = Basic::getChildren(pathbar_);
        for (GList *children = children_list;children && children->data; children=children->next){
           auto widget = GTK_WIDGET(children->data);
           if (g_object_get_data(G_OBJECT(widget), "skipMenu")) continue;
           if (g_object_get_data(G_OBJECT(widget), "menu")) continue;
           auto parent = widget;

            auto gesture = gtk_gesture_click_new();
            gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),1);
            if (pathbar_go_f) // FIXME: should reload gliststore
              g_signal_connect (G_OBJECT(gesture) , "released", EVENT_CALLBACK (pathbar_go_f), (void *)this);
            gtk_widget_add_controller(GTK_WIDGET(widget), GTK_EVENT_CONTROLLER(gesture));
 
        }
        g_list_free(children_list);

        // Now process back and next buttons
        {
          auto next = GTK_WIDGET(g_object_get_data(G_OBJECT(pathbar_), "next"));
          auto back = GTK_WIDGET(g_object_get_data(G_OBJECT(pathbar_), "back"));
          GList *historyNext = (GList *)g_object_get_data(G_OBJECT(pathbar_), "historyNext");
          GList *historyBack = (GList *)g_object_get_data(G_OBJECT(pathbar_), "historyBack");
          TRACE("length historyNext=%d\n", g_list_length(historyNext));
          TRACE("length historyBack=%d\n", g_list_length(historyBack));
          if (g_list_length(historyNext) <= 0) {
            gtk_widget_remove_css_class (GTK_WIDGET(next), "pathbarboxNegative" );
            gtk_widget_add_css_class (GTK_WIDGET(next), "pathbarbox" );          
          }
          gtk_widget_set_sensitive(next, g_list_length(historyNext) > 0);
          // History back contains the first path visited. 
          if (g_list_length(historyBack) <= 1) {
            gtk_widget_remove_css_class (GTK_WIDGET(back), "pathbarboxNegative" );
            gtk_widget_add_css_class (GTK_WIDGET(back), "pathbarbox" );          
          }
          gtk_widget_set_sensitive(back, g_list_length(historyBack) > 1); 
        }
    } 


    void 
    togglePathbar(const gchar *path){
        TRACE("*** togglePathbar: %s\n", path);
        GList *children_list = Basic::getChildren(pathbar_);

        if (gtk_widget_get_realized(MainWidget)) showWhatFits(path, children_list);
        else {TRACE("MainWidget not yet realized...\n");}

        GList *children = g_list_first(children_list);
        for (;children && children->data; children=children->next){
            auto buttonPath = (const char *)g_object_get_data(G_OBJECT(children->data), "path");
            TRACE("*** compare %s with %s (%s)\n", path, buttonPath, path_);
            if (strcmp(path_, buttonPath)==0) {
              gtk_widget_remove_css_class (GTK_WIDGET(children->data), "pathbarbox" );
              gtk_widget_add_css_class (GTK_WIDGET(children->data), "pathbarboxRed" );
            } else {
              gtk_widget_remove_css_class (GTK_WIDGET(children->data), "pathbarboxRed" );
              gtk_widget_add_css_class (GTK_WIDGET(children->data), "pathbarbox" );
            }
        }
        g_list_free(children_list);
    }
    private:

    void         
    showWhatFits(const gchar *path, GList *children_list){
      GtkRequisition minimum;
      graphene_rect_t bounds;
      bounds.size.width = 0;

      if (!gtk_widget_get_realized(GTK_WIDGET(pathbar_))){
        // Take window width.
        if (gtk_widget_get_realized(GTK_WIDGET(MainWidget))){
          if (!gtk_widget_compute_bounds(GTK_WIDGET(MainWidget), GTK_WIDGET(MainWidget), &bounds)) {
            DBG("***Error:: gtk_widget_compute_bounds(MainWidget). Widget realized?\n");
          }
        }
      } else {
        if (!gtk_widget_compute_bounds(GTK_WIDGET(pathbar_), GTK_WIDGET(pathbar_), &bounds)) {
          DBG("***Error:: gtk_widget_compute_bounds(pathbar_). Widget realized?\n");
        }
      }
      TRACE("Window is realized =%d\n", gtk_widget_get_realized(MainWidget));
      TRACE("pathbar_ is realized =%d\n", gtk_widget_get_realized(GTK_WIDGET(pathbar_)));
      auto size = &(bounds.size);
      auto width = size->width;
      // if width == 0, then the realized test was not enough
      if (width == 0) return;
      TRACE("initial width = %f\n", width);

        // First we hide all buttons, except "RFM_ROOT"
        //      and go buttons
        GList *children = g_list_last(children_list);
        for (;children && children->data; children=children->prev){
            gchar *name = (gchar *)g_object_get_data(G_OBJECT(children->data), "name");
            if (strcmp(name, "RFM_ROOT")==0) {
                gtk_widget_get_preferred_size(GTK_WIDGET(children->data), 
                        &minimum, NULL);
                width -= minimum.width;
                continue;
            }
            if (strcmp(name, "RFM_GOTO")==0) continue;
            auto pb_path = (const gchar *)g_object_get_data(G_OBJECT(children->data), "path");
            gtk_widget_set_visible(GTK_WIDGET(children->data), FALSE);
        }

        // Find first item to place in pathbar_.
        // This item *must* be equal to path, if path is in buttons.

        children = g_list_last(children_list);
        GList *active = children;
        // If path is not in the buttons, then the first to map
        // will be the last path visited.
        if (path) for (;children && children->data; children=children->prev){
            gchar *name = (gchar *)g_object_get_data(G_OBJECT(children->data), "name");
            auto pb_path = (const gchar *)g_object_get_data(G_OBJECT(children->data), "path");
            if (!pb_path) continue;
            if (strcmp(path, pb_path)==0) {
                active = children;
                break;
            }
        }
         // Show active button
        gtk_widget_set_visible(GTK_WIDGET(active->data), TRUE);

        gtk_widget_get_preferred_size(GTK_WIDGET(active->data), &minimum, NULL);
            TRACE("#### width, minimum.width %d %d\n",width,  minimum.width);
        width -= minimum.width;
     
        // Work backwards from active button we show buttons that will fit.
        // Active is already shown above.
        children = active->prev;

        for (;children && children->data; children=children->prev){
            gchar *name = (gchar *)g_object_get_data(G_OBJECT(children->data), "name");
            if (strcmp(name, "RFM_ROOT")==0) continue;

            if (!gtk_widget_compute_bounds(GTK_WIDGET(children->data), GTK_WIDGET(children->data), &bounds)) {
              DBG("***Error:: showWhatFits():gtk_widget_compute_bounds(). Widget realized?\n");
            }

            TRACE("#### width, allocaltion.width %f %f\n",width,  bounds.size.width);
            width -= bounds.size.width;
            if (width < 0) {
              TRACE("**pathbar_ width=%f\n", width);
              break;
            }
            gtk_widget_set_visible(GTK_WIDGET(children->data), TRUE);
        }

        // Now we work forwards, showing buttons that fit.
        children = active->next;
        for (;children && children->data; children=children->next){
           gchar *name = (gchar *)g_object_get_data(G_OBJECT(children->data), "name");
            if (strcmp(name, "RFM_ROOT")==0) continue;

            if (!gtk_widget_compute_bounds(GTK_WIDGET(children->data), GTK_WIDGET(children->data), &bounds)) {
              DBG("***Error:: showWhatFits():gtk_widget_compute_bounds(). Widget realized?\n");
            }
            width -= bounds.size.width;

            if (width < 0) break;
            gtk_widget_set_visible(GTK_WIDGET(children->data), TRUE);
        }
    }

    static gboolean
    pathbar_white ( GtkEventControllerMotion* self,
                    gdouble x,
                    gdouble y,
                    gpointer data) 
    {
        //auto pathbar_p = (FileResponsePathbar *)data;
        //auto pathbar = pathbar_p->pathbar();
        auto eventBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
        auto path = (const char *) g_object_get_data(G_OBJECT(eventBox), "path");
        TRACE("pathbar_white:%s\n", path);
        gtk_widget_remove_css_class (eventBox, "pathbarbox" );
        gtk_widget_add_css_class (eventBox, "pathbarboxNegative" );
        return FALSE;
    }

    static gboolean
    pathbar_blue (GtkEventControllerMotion* self,
                    gdouble x,
                    gdouble y,
                    gpointer data) {
        auto pathbar_p = (FileResponsePathbar *)data;
        //auto pathbar_p = (FileResponse *)data;
        auto eventBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
        auto path = (const char *) g_object_get_data(G_OBJECT(eventBox), "path");
        TRACE("pathbar_blue:%s\n", path);
        
        gtk_widget_remove_css_class (eventBox, "pathbarboxNegative" );
        if (strcmp(pathbar_p->path(), path) == 0){
          gtk_widget_add_css_class (eventBox, "pathbarboxRed" );
        } else {
          gtk_widget_add_css_class (eventBox, "pathbarbox" );
        }
        return FALSE;

    }

  private:

   void addSignals(GtkBox *eventBox, const char *path){
      //g_object_set_data(G_OBJECT(eventBox), "skipMenu", GINT_TO_POINTER(1));
      
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
    static gboolean
    goJump (
              GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer data ) 
    {

      DBG("goJump currently disabled.\n");
    return true;
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
        //FIXME   Workdir<DirectoryClass>::setWorkdir(previous, GTK_BOX(pathbar), false);
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
       //FIXME     Workdir<DirectoryClass>::setWorkdir(current, GTK_BOX(pathbar), false);
           return TRUE;
         }
      }
      /*if (strcmp(path, "xffm:goto") == 0){
        auto dialogObject = new DialogPrompt<jumpResponse<DirectoryClass> >;
        auto dialog = dialogObject->dialog();
        dialogObject->setParent(GTK_WINDOW(MainWidget));
        dialogObject->run();
      }*/
      
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
