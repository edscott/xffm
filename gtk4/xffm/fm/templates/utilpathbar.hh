#ifndef UTILPATHBAR_HH
#define UTILPATHBAR_HH
namespace xf {
  template <class Type>
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
     TRACE("next path is %s\n", (const char *) current);
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
     TRACE("Back path is %s\n", previous);
     // No need to free memory, since we just move from one list to the other.
     historyNext_ = g_list_prepend(historyNext_, (void *)current);
     historyBack_ = g_list_remove(historyBack_,  (void *)current);
     //for (GList *l=historyNext_; l && l->data; l=l->next){TRACE("historyNext list = %s\n", (char *)l->data);}
     return previous;    
   }

   void push(const char *path){
      if (historyBack_ && historyBack_->data != NULL){
        if (strcmp(path, (const char *)historyBack_->data) != 0){
          // update with different or non existing path.
          historyBack_ = g_list_prepend(historyBack_, g_strdup(path));
          TRACE("updatePathbar: 1 updating historyBack_ with path = %s\n", path);
        }
      } else {
          // update with new path. 
          historyBack_ = g_list_prepend(historyBack_, g_strdup(path));
          TRACE("updatePathbar: 2 updating histohistoryBack_ryBack with path = %s\n", path);
      }
      // wipe next history 
      for (GList *l=historyNext_; l && l->data; l=l->next) g_free(l->data);
      g_list_free(historyNext_);
      historyNext_ = NULL;
      return;
   }
  
  };
  
  
  template <class Type>
  class UtilPathbar {
    public:
    ///////////////////   pathbar  ///////////////////////////////////
    
    
    static void 
    updatePathbar(bool updateHistory, void *pathbar_go_f){
        const gchar *path = Child::getWorkdir();
        GtkBox *pathbar = Child::getPathbar();
        updatePathbar(path, pathbar, updateHistory, pathbar_go_f);
        resetPathbarCSS(pathbar);
    }
    
    static void 
    updatePathbar(const gchar *path, GtkBox *pathbar, bool updateHistory, void *pathbar_go_f){
        TRACE( "update pathbar to %s (update=%d)\n", path, updateHistory);
        TRACE( "update_pathbar_f:: %s\n", path);

        if (!pathbar) return ;
        if (!path){
            TRACE("##### togglePathbar(NULL, pathbar)\n");
            togglePathbar(NULL, pathbar);
//            pathbar_p->toggle_pathbar(NULL);
            return ;
        }
        auto pathbarHistory_p = (PathbarHistory<Type> *)g_object_get_data(G_OBJECT(pathbar), "pathbarHistory");
        if (updateHistory) {
          pathbarHistory_p->push(path);
 /*         
          GList *historyBack = (GList *)g_object_get_data(G_OBJECT(pathbar), "historyBack");
          if (historyBack){
            if (strcmp(path, (const char *)historyBack->data) != 0){
              historyBack = g_list_prepend(historyBack, g_strdup(path));
              TRACE("updatePathbar: 1 updating historyBack with path = %s\n", path);
            }
          } else {
              historyBack = g_list_prepend(historyBack, g_strdup(path));
              TRACE("updatePathbar: 2 updating historyBack with path = %s\n", path);
          }
          g_object_set_data(G_OBJECT(pathbar), "historyBack", historyBack);
          // wipe next history 
          GList *historyNext = (GList *)g_object_get_data(G_OBJECT(pathbar), "historyNext");
          for (GList *l=historyNext; l && l->data; l=l->next) g_free(l->data);
          g_list_free(historyNext);
          g_object_set_data(G_OBJECT(pathbar), "historyNext", NULL);
      */
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


/*          
          GList *historyNext = (GList *)g_object_get_data(G_OBJECT(pathbar), "historyNext");
          GList *historyBack = (GList *)g_object_get_data(G_OBJECT(pathbar), "historyBack");
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
*/
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
            //gchar *p = g_strdup_printf("%s%c", paths[i], G_DIR_SEPARATOR);
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
                //gtk_container_remove(GTK_CONTAINER(pathbar), GTK_WIDGET(tail->data));
            }
            break;
        }
        g_list_free(children_list);

        // Add new tail
        for (;paths[i]; i++){
            auto pb_button = 
                UtilBasic::pathbarLabelButton(strlen(paths[i])?paths[i]:G_DIR_SEPARATOR_S);

            Basic::boxPack0 (pathbar, GTK_WIDGET(pb_button), FALSE, FALSE, 0);
            //gtk_container_add(GTK_CONTAINER(pathbar), GTK_WIDGET(pb_button));

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
            g_signal_connect (G_OBJECT(motion) , "enter", EVENT_CALLBACK (pathbar_white), (void *)pathbar);
            g_signal_connect (G_OBJECT(motion) , "leave", EVENT_CALLBACK (pathbar_blue), (void *)pathbar);
            
 
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
        auto gridView_p = Child::getGridviewObject();
        for (GList *children = children_list;children && children->data; children=children->next){
           auto widget = GTK_WIDGET(children->data);
           if (g_object_get_data(G_OBJECT(widget), "skipMenu")) continue;
           if (g_object_get_data(G_OBJECT(widget), "menu")) continue;
           auto parent = widget;
           auto path = (const char *)g_object_get_data(G_OBJECT(widget), "path");
           auto myItemMenu = new Menu<PathbarMenu<bool>>(path);
           myItemMenu->setMenu(widget, parent, path);
           delete myItemMenu;

            auto gesture = gtk_gesture_click_new();
            gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),1);
            g_signal_connect (G_OBJECT(gesture) , "released", EVENT_CALLBACK (pathbar_go_f), (void *)pathbar);
            gtk_widget_add_controller(GTK_WIDGET(widget), GTK_EVENT_CONTROLLER(gesture));
 
        }

        resetPathbarCSS(pathbar);
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

        /*if (gtk_widget_is_visible(GTK_WIDGET(mainWindow))) showWhatFits(pathbar_, path, children_list);
        else gtk_widget_show_all(GTK_WIDGET(pathbar_));*/


        // Finally, we differentiate active button.
        GList *children = g_list_first(children_list);
        auto gridview_p = (GridView<LocalDir> *)Child::getGridviewObject();
        for (;children && children->data; children=children->next){
            auto path = (const char *)g_object_get_data(G_OBJECT(children->data), "path");
            const char *css = "pathbarbox";
            if (gridview_p && gridview_p->path() && strcmp(path, gridview_p->path())==0) css = "pathbarboxRed";
            setPathButtonText(GTK_WIDGET(children->data), path, css);
        }
        g_list_free(children_list);


        auto lastPath = (char *) g_object_get_data(G_OBJECT(pathbar), "path");
        g_free(lastPath);
        g_object_set_data(G_OBJECT(pathbar), "path", g_strdup(path));
    }
    public:

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

        auto gridview_p = (GridView<LocalDir> *)Child::getGridviewObject();

        gtk_widget_remove_css_class (eventBox, "pathbarboxNegative" );
/*
        if (!gridview_p){
          DBG("UtilPathbar::pathbar_blue():Should not happen, gridview_p == NULL\n");
          return false;
        }
        if (!gridview_p->path()){
          DBG("UtilPathbar::pathbar_blue():Should not happen, gridview_p->path() == NULL\n");
          return false;
        }
        if (!path){
          DBG("UtilPathbar::pathbar_blue():Should not happen, eventBox path == NULL\n");
          return false;
        }
        */
        if (strcmp(path, gridview_p->path())==0){
          gtk_widget_add_css_class (eventBox, "pathbarboxRed" );
        } else {
          gtk_widget_add_css_class (eventBox, "pathbarbox" );
        }

        return FALSE;

    }

    public:

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
  public:  

  static void resetPathbarCSS(GtkBox *pathbar){
    const gchar *path = Child::getWorkdir();
    //auto gridview_p = (GridView<Type> *)Child::getGridviewObject();
    //if (!gridview_p || !gridview_p->path()) return;
    GList *children_list = Basic::getChildren(pathbar);
    GList *children = g_list_first(children_list);
    for (;children && children->data; children=children->next){
      auto widget = GTK_WIDGET(children->data);
      gtk_widget_remove_css_class (widget, "pathbardrop" );
      auto path = (const char *)g_object_get_data(G_OBJECT(widget), "path");
      if (strcmp(path, Child::getWorkdir())==0) {
        gtk_widget_remove_css_class (widget, "pathbarbox" );
        gtk_widget_add_css_class (widget, "pathbarboxRed" );
      }
      else gtk_widget_add_css_class (widget, "pathbarbox" );
    }
    g_list_free(children_list);
  }
    
  private:
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


  };
}
#endif

