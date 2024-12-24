#ifndef BASICPATHBAR_HH
#define BASICPATHBAR_HH
namespace xf {

  class PathbarHistory {
    GList *historyBack_=NULL;
    GList *historyNext_=NULL;
  public:

   ~PathbarHistory(void){
     for (auto l=historyBack_; l && l->data; l=l->next) g_free(l->data);
     for (auto l=historyNext_; l && l->data; l=l->next) g_free(l->data);
     g_list_free(historyBack_);
     g_list_free(historyNext_);
     return;
   }

   bool historyNext(void) {
     if (historyNext_) return true; 
     return false;
   }
   bool historyBack(void) {
     if (historyBack_ && g_list_length(historyBack_) > 1) return true;
     return false;
   }

   const char *nextHistory(void){
     if (!historyNext_ || historyNext_->data == NULL){
      //if (!historyNext_){ TRACE("no next history List\n"); } else { TRACE("no next history next\n"); }
      return NULL;
     }
     char *current = (char *) historyNext_->data;
     DBG("next path is %s\n", (const char *) current);
     historyBack_ = g_list_prepend(historyBack_, current);
     historyNext_ = g_list_remove(historyNext_, current);
     return current;
   }

   const char *backHistory(void){
     if (!historyBack_ || historyBack_->next == NULL) {
      //if (!historyBack_){ TRACE("no back history List\n"); } else { TRACE("no back history next\n"); }
      return NULL;
     }
     auto current = (const char *) historyBack_->data;
     auto previous = (const char *) historyBack_->next->data;
     DBG("Back path is %s\n", previous);
     // No need to free memory, since we just move from one list to the other.
     historyNext_ = g_list_prepend(historyNext_, (void *)current);
     historyBack_ = g_list_remove(historyBack_,  (void *)current);
     //for (GList *l=historyNext_; l && l->data; l=l->next){TRACE("historyNext list = %s\n", (char *)l->data);}
     return previous;    
   }

   void push(const char *path){
          DBG("BasicPAthbar:: pushing %s\n", path);
      if (historyBack_ && historyBack_->data != NULL){
        if (strcmp(path, (const char *)historyBack_->data) != 0){
          // update with different or non existing path.
          historyBack_ = g_list_prepend(historyBack_, g_strdup(path));
          DBG("push: 1 updating historyBack_ with path = %s\n", path);
        }
      } else {
          // update with new path. 
          historyBack_ = g_list_prepend(historyBack_, g_strdup(path));
          DBG("push: 2 updating historyBack with path = %s\n", path);
      }
      // wipe next history 
      for (GList *l=historyNext_; l && l->data; l=l->next) g_free(l->data);
      g_list_free(historyNext_);
      historyNext_ = NULL;
      return;
   }
  
  };


  class BasicPathbar {

    static GtkBox *eventBox(const char **p){

      auto box = eventButton(p[0], p[1], p[2], p[3]);

      auto motionB = gtk_event_controller_motion_new();
      gtk_event_controller_set_propagation_phase(motionB, GTK_PHASE_CAPTURE);
      gtk_widget_add_controller(GTK_WIDGET(box), motionB);
      g_signal_connect (G_OBJECT(motionB) , "leave", EVENT_CALLBACK (buttonPositive), NULL);
      g_signal_connect (G_OBJECT(motionB) , "enter", EVENT_CALLBACK (buttonNegative), NULL);

      return box;
    }
    public:


    static GtkBox *pathbarBox(void){
        GtkBox *pathbar = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        const char *p1[] = {"xf-go-previous", "RFM_GOTO", "xffm:back", _("Previous"), NULL};
        auto eventBox1 = eventBox(p1);
        g_object_set_data(G_OBJECT(pathbar), "back", eventBox1);

        const char *p2[] = {"xf-go-next", "RFM_GOTO", "xffm:next", _("Next"), NULL};
        auto eventBox2 = eventBox(p2);
        g_object_set_data(G_OBJECT(pathbar), "next", eventBox2);

        const char *p3[] = {"xf-go-to", "RFM_GOTO", "xffm:goto", _("Go to"), NULL};
        auto eventBox3 = eventBox(p3);
        g_object_set_data(G_OBJECT(pathbar), "goto", eventBox3);

        gtk_widget_set_sensitive(GTK_WIDGET(eventBox1), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(eventBox2), FALSE);

        Basic::boxPack0 (pathbar, GTK_WIDGET(eventBox1), FALSE, FALSE, 0);
        Basic::boxPack0 (pathbar, GTK_WIDGET(eventBox3), FALSE, FALSE, 0);
        Basic::boxPack0 (pathbar, GTK_WIDGET(eventBox2), FALSE, FALSE, 0);
        
        // bookmarks button:
        auto pb_button = UtilBasic::pathbarLabelButton(".");
        g_object_set_data(G_OBJECT(pb_button), "skipMenu", GINT_TO_POINTER(1));
        g_object_set_data(G_OBJECT(pathbar), "pb_button", pb_button);
        
        Basic::boxPack0 (pathbar, GTK_WIDGET(pb_button), FALSE, FALSE, 0);
        g_object_set_data(G_OBJECT(pb_button), "name", g_strdup("RFM_ROOT"));
        g_object_set_data(G_OBJECT(pb_button), "path", g_strdup(_("Bookmarks")));

        auto motion = gtk_event_controller_motion_new();
        gtk_event_controller_set_propagation_phase(motion, GTK_PHASE_CAPTURE);
        gtk_widget_add_controller(GTK_WIDGET(pb_button), motion);
        g_signal_connect (G_OBJECT(motion) , "enter", EVENT_CALLBACK (pathbar_white), (void *)pathbar);
        g_signal_connect (G_OBJECT(motion) , "leave", EVENT_CALLBACK (pathbar_blue), (void *)pathbar);
        return pathbar;
    }

  static void setRed(GtkBox *pathbar, const char *redPath){
    GList *children_list = Basic::getChildren(pathbar);
    GList *children = g_list_first(children_list);
    for (;children && children->data; children=children->next){
      auto widget = GTK_WIDGET(children->data);
      auto path = (const char *)g_object_get_data(G_OBJECT(widget), "path");
      if (strcmp(path, redPath)==0) {
        gtk_widget_remove_css_class (widget, "pathbarbox" );
        gtk_widget_add_css_class (widget, "pathbarboxRed" );
      }
      else gtk_widget_add_css_class (widget, "pathbarbox" );
    }
    g_list_free(children_list);
  }

  static void resetPathbarCSS(GtkBox *pathbar){
    GList *children_list = Basic::getChildren(pathbar);
    GList *children = g_list_first(children_list);
    for (;children && children->data; children=children->next){
      auto widget = GTK_WIDGET(children->data);
      gtk_widget_remove_css_class (widget, "pathbardrop" );
      gtk_widget_add_css_class (widget, "pathbarbox" );
    }
    g_list_free(children_list);
    auto redPath = (const char *) g_object_get_data(G_OBJECT(pathbar), "path");
    setRed(pathbar, redPath);
 }
    
    
    static void 
    updatePathbar(const gchar *path, GtkBox *pathbar, bool updateHistory, void *pathbar_go_f){
        TRACE( "update pathbar to %s (update=%d)\n", path, updateHistory);
        TRACE( "update_pathbar_f:: %s\n", path);

        if (!pathbar) return ;
        if (!path){
            TRACE("##### togglePathbar(NULL, pathbar)\n");
            togglePathbar(NULL, pathbar);
            return ;
        }
        auto pathbarHistory_p = (PathbarHistory *)g_object_get_data(G_OBJECT(pathbar), "pathbarHistory_p");
        if (updateHistory) {
          DBG("BasicPAthbar::updatePathbar, pushing %s\n", path);
          pathbarHistory_p->push(path);
        }
        // Now process to back and next buttons
        {
          auto next = GTK_WIDGET(g_object_get_data(G_OBJECT(pathbar), "next"));
          auto back = GTK_WIDGET(g_object_get_data(G_OBJECT(pathbar), "back"));
          if (pathbarHistory_p->historyNext()){
            gtk_widget_remove_css_class (GTK_WIDGET(next), "pathbarboxNegative" );
            gtk_widget_add_css_class (GTK_WIDGET(next), "pathbarbox" );          
            gtk_widget_set_sensitive(next, true);
          } else gtk_widget_set_sensitive(next, false);
          if (pathbarHistory_p->historyBack()){
            gtk_widget_remove_css_class (GTK_WIDGET(back), "pathbarboxNegative" );
            gtk_widget_add_css_class (GTK_WIDGET(back), "pathbarbox" );          
            gtk_widget_set_sensitive(back, true);
          } else gtk_widget_set_sensitive(back, false);

        }
       

        //Nonexisting paths, use homedir
        if (!g_file_test(path, G_FILE_TEST_EXISTS)) path = g_get_home_dir();
         
        // Trim pathbar.
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

        GList *children_list = Basic::getChildren(pathbar);
        //for (auto l=children_list; l && l->data; l=l->next);
        //GList *children_list = gtk_container_get_children(GTK_CONTAINER(pathbar));
        gint i=0;
        gchar *pb_path = NULL;
        for (GList *children = children_list;children && children->data; children=children->next){
            gchar *name = (gchar *)g_object_get_data(G_OBJECT(children->data), "name");
            if (!name){
              DBG("***Error utilpathbar.hh name is not set\n");
              continue;
            }
            if (strcmp(name, "RFM_ROOT")==0 || strcmp(name, "RFM_GOTO")==0) continue;
            TRACE( "(%d) comparing %s <--> %s\n", i, name, paths[i]);
            if (paths[i] && strcmp(name, paths[i]) == 0){
                g_free(pb_path);
                const gchar *p = (const gchar *)g_object_get_data(G_OBJECT(children->data), "path");
                pb_path = g_strdup(p);
                i++; 
                continue;
            }
            // Eliminate tail (only if tail will differ)
            if (paths[i] == NULL) break;
            TRACE( "Zapping tail: \"%s\"\n", paths[i]);
            GList *tail = children;
            for (;tail && tail->data; tail = tail->next){
                gchar *name  = (gchar *)g_object_get_data(G_OBJECT(tail->data), "name");
                TRACE( "Zapping tail item: \"%s\"\n", name);
                g_free(name);
                gtk_widget_unparent(GTK_WIDGET(tail->data));
            }
            break;
        }
        g_list_free(children_list);

        // Add new tail
        for (;paths[i]; i++){
            auto pb_button = 
                UtilBasic::pathbarLabelButton(strlen(paths[i])?paths[i]:G_DIR_SEPARATOR_S);

            Basic::boxPack0 (pathbar, GTK_WIDGET(pb_button), FALSE, FALSE, 0);

            gchar *g = (pb_path!=NULL)?
                g_strdup_printf("%s%s%s",pb_path, 
                        strcmp(pb_path,G_DIR_SEPARATOR_S)? 
                        G_DIR_SEPARATOR_S:"", paths[i]):
                g_strdup(paths[i]);
            g_free(pb_path);
            pb_path = g;
            TRACE( "+++***** setting pbpath --> %s\n", pb_path);
            g_object_set_data(G_OBJECT(pb_button), "path", g_strdup(pb_path));


            auto motion = gtk_event_controller_motion_new();
            gtk_event_controller_set_propagation_phase(motion, GTK_PHASE_CAPTURE);
            gtk_widget_add_controller(GTK_WIDGET(pb_button), motion);
            g_signal_connect (G_OBJECT(motion) , "enter", 
                EVENT_CALLBACK (BasicPathbar::pathbar_white), (void *)pathbar);
            g_signal_connect (G_OBJECT(motion) , "leave",
                EVENT_CALLBACK (BasicPathbar::pathbar_blue), (void *)pathbar);
 
            gtk_widget_set_visible(GTK_WIDGET(pb_button), TRUE);
        }
        g_free(pb_path);
        g_strfreev(paths);
        
        // show what fits
        togglePathbar(path, pathbar);

        // finally, add the menu item for each pathbar item.
        // This should only be done if no menu is already set
        // for the item.
        // Also, should skip back, next and goto buttons.
        children_list = Basic::getChildren(pathbar);
        for (GList *children = children_list;children && children->data; children=children->next){
           auto widget = GTK_WIDGET(children->data);
           if (g_object_get_data(G_OBJECT(widget), "skipMenu")) continue;
           if (g_object_get_data(G_OBJECT(widget), "menu")) continue;
           auto parent = widget;
           auto path = (const char *)g_object_get_data(G_OBJECT(widget), "path");
           // FIXME: no menu
           //auto myItemMenu = new Menu<PathbarMenu<bool>>(path);
           //myItemMenu->setMenu(widget, parent, path);
           //delete myItemMenu;
            
           if (g_object_get_data(G_OBJECT(widget), "hasGesture") == NULL)
           {
             DBG("Adding gesture to %s\n", path);
             auto gesture = gtk_gesture_click_new();
             gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),1);
             g_signal_connect (G_OBJECT(gesture) , "released", EVENT_CALLBACK (pathbar_go_f), (void *)pathbar);
             gtk_widget_add_controller(GTK_WIDGET(widget), GTK_EVENT_CONTROLLER(gesture));
             g_object_set_data( G_OBJECT(widget), "hasGesture", GINT_TO_POINTER(1));
           }
 
        }
        g_list_free(children_list);
        resetPathbarCSS(pathbar);
        setRed(pathbar, path);
        return ;
    }

    private:
    static void 
    togglePathbar(const gchar *path, GtkBox *pathbar){
        // Hiding stuff which does not fit does not work until
        // window has been shown. This is not yet the case on
        // initial startup, so we skip that on first pass.
        //
        // Probably bug if initial startup path width is
        // larger than initial window width, as 
        // everything will show. Maybe the window width 
        // will adjust?
        //
        TRACE("*** togglePathbar: %s\n", path);
        GList *children_list = Basic::getChildren(pathbar);
        if (gtk_widget_get_realized(MainWidget)) showWhatFits(pathbar, path, children_list);
        else {TRACE("MainWidget not yet realized...\n");}
 
        // set all texts in blue.
        GList *children = g_list_first(children_list);
        for (;children && children->data; children=children->next){
            const char *css = "pathbarbox";
            setPathButtonText(GTK_WIDGET(children->data), path, css);
        }
        g_list_free(children_list);

        auto lastPath = (char *) g_object_get_data(G_OBJECT(pathbar), "path");
        g_free(lastPath);
        g_object_set_data(G_OBJECT(pathbar), "path", g_strdup(path));
        setRed(pathbar, path);
    }
    
      
    static gboolean
    buttonNegative (GtkEventControllerMotion* self, double x, double y, void *data) 
    {
        auto eventBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
        gtk_widget_remove_css_class (GTK_WIDGET(eventBox), "pathbarbox" );
        gtk_widget_add_css_class (GTK_WIDGET(eventBox), "pathbarboxNegative" );
        Basic::flushGTK();
        return FALSE;
    }
    static gboolean
    buttonPositive ( GtkEventControllerMotion* self, double x, double y, void *data) 
    {
        auto eventBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
        gtk_widget_remove_css_class (GTK_WIDGET(eventBox), "pathbarboxNegative" );
        gtk_widget_add_css_class (GTK_WIDGET(eventBox), "pathbarbox" );
        Basic::flushGTK();
        return FALSE;
    }

    static gboolean
    pathbar_white ( GtkEventControllerMotion* self,
                    gdouble x,
                    gdouble y,
                    gpointer data) 
    {
        auto pathbar = GTK_BOX(data);
        auto eventBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
        auto path = (const char *) g_object_get_data(G_OBJECT(eventBox), "path");


        gtk_widget_remove_css_class (eventBox, "pathbarbox" );
        gtk_widget_add_css_class (eventBox, "pathbarboxNegative" );
        return FALSE;
    }

    static gboolean
    pathbar_blue (GtkEventControllerMotion* self,
                    gdouble x,
                    gdouble y,
                    gpointer data) {
        auto pathbar = GTK_BOX(data);
        resetPathbarCSS(pathbar);
        auto eventBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
        auto path = (const char *) g_object_get_data(G_OBJECT(eventBox), "path");
        auto redPath = (const char *) g_object_get_data(G_OBJECT(pathbar), "path");
        TRACE("redPath = %s\n", redPath);

        gtk_widget_remove_css_class (eventBox, "pathbarboxNegative" );
        gtk_widget_add_css_class (eventBox, "pathbarbox" );
        setRed(pathbar, redPath);
 
        return FALSE;

    }

    private:

    static void         
    showWhatFits(GtkBox *pathbar, const gchar *path, GList *children_list){
      GtkRequisition minimum;
      graphene_rect_t bounds;
      bounds.size.width = 0;

      if (!gtk_widget_get_realized(GTK_WIDGET(pathbar))){
        // Take window width.
        if (gtk_widget_get_realized(GTK_WIDGET(MainWidget))){
          if (!gtk_widget_compute_bounds(GTK_WIDGET(MainWidget), GTK_WIDGET(MainWidget), &bounds)) {
            DBG("***Error:: gtk_widget_compute_bounds(MainWidget). Widget realized?\n");
          }
        }
      } else {
        if (!gtk_widget_compute_bounds(GTK_WIDGET(pathbar), GTK_WIDGET(pathbar), &bounds)) {
          DBG("***Error:: gtk_widget_compute_bounds(pathbar). Widget realized?\n");
        }
      }
      TRACE("Window is realized =%d\n", gtk_widget_get_realized(MainWidget));
      TRACE("pathbar is realized =%d\n", gtk_widget_get_realized(GTK_WIDGET(pathbar)));
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

        // Find first item to place in pathbar.
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
              TRACE("**pathbar width=%f\n", width);
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

    static void 
    setPathButtonText(GtkWidget *eventBox, const gchar *path, const char *css){


        //const gchar *fontSize = "size=\"small\"";
        const gchar *fontSize = "";
        const char *name = (const char *)g_object_get_data(G_OBJECT(eventBox), "name");
        gtk_widget_add_css_class (eventBox, css );
        
        if (!name){
          DBG("setPathButtonText: name is null\n");
          name="FIXME";
        } else {
          if (strcmp(name, "RFM_ROOT")==0) {
              // no path means none is differentiated.
              gchar *markup = g_strdup_printf(" %s  ", ".");
              auto label = GTK_LABEL(g_object_get_data(G_OBJECT(eventBox), "label"));
              gtk_label_set_markup(label, markup);
              g_free(markup);
              return;
          } 
          if (strcmp(name, "RFM_GOTO")==0) {
              return;
          } 
        }
        gchar *v = Basic::utf_string(name);
        gchar *g = g_markup_escape_text(v, -1);
        g_free(v);
        gchar *markup = g_strdup_printf(" %s ", g);
        auto label = GTK_LABEL(g_object_get_data(G_OBJECT(eventBox), "label"));
        gtk_label_set_markup(label, markup);

        g_free(g);
        g_free(markup);
         return;
    }
    static GtkBox *eventButton(const gchar *icon, const gchar *name, 
                               const gchar *path, const gchar *tooltip) 
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

  

