#ifndef FILERESPONSE_HH
#define FILERESPONSE_HH

namespace xf {

  class FileResponsePathbar {
    GtkBox *pathbar_;
    gchar *path_;
    gchar *workdir_;
    GtkWidget *back_;
    GtkWidget *next_;
    GList *historyBack_;
    GList *historyNext_;

  public:
   GtkBox *pathbar(void){return pathbar_;} 
   const gchar *path(void){ return path_;}
   void path(const char *value){
     g_free(path_);
     path_ = g_strdup(value);     
   }

   GList *historyBack(void){ return historyBack_;}
   GList *historyNext(void){ return historyNext_;}
   void setHistoryBack(GList *historyBack){ historyBack_ = historyBack;}
   void setHistoryNext(GList *historyNext){ historyNext_ = historyNext;}
    
    FileResponsePathbar(void) {
      pathbar_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
      gtk_widget_set_hexpand(GTK_WIDGET(pathbar_), false);
      gtk_widget_set_vexpand(GTK_WIDGET(pathbar_), false);
    

      auto eventBox1 = eventButton("xf-go-previous", "RFM_GOTO", "xffm:back", _("Previous"));
      auto eventBox2 = eventButton("xf-go-next", "RFM_GOTO", "xffm:next", _("Next"));
      back_ = GTK_WIDGET(eventBox1);
      next_ = GTK_WIDGET(eventBox2);
      g_object_set_data(G_OBJECT(pathbar_), "back", back_);
      g_object_set_data(G_OBJECT(pathbar_), "next", next_);

      gtk_widget_set_sensitive(GTK_WIDGET(eventBox1), FALSE);
      gtk_widget_set_sensitive(GTK_WIDGET(eventBox2), FALSE);

      addSignals(eventBox1, "xffm:back");
      addSignals(eventBox2, "xffm:next");

      Basic::boxPack0 (pathbar_, GTK_WIDGET(eventBox1), FALSE, FALSE, 0);
      Basic::boxPack0 (pathbar_, GTK_WIDGET(eventBox2), FALSE, FALSE, 0);
        
    
 /*       auto gesture1 = gtk_gesture_click_new();
        gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture1),1);
        g_signal_connect (G_OBJECT(gesture1) , "released", EVENT_CALLBACK (Workdir<DirectoryClass>::pathbar_go), (void *)pathbar_);
        gtk_widget_add_controller(GTK_WIDGET(pb_button), GTK_EVENT_CONTROLLER(gesture1));
 
*/
    }

    ~FileResponsePathbar(){
      g_free(path_);
    }

    void updatePathbarBox(const char *path, GtkBox *pathbar, void *pathbar_go_f){
        //Nonexiting paths, use homedir
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
            g_signal_connect (G_OBJECT(motion) , "leave", EVENT_CALLBACK (pathbar_blue), (void *)this);
            
 
            gtk_widget_set_visible(GTK_WIDGET(pb_button), TRUE);
            if (strcmp(pb_path, path) == 0) {
              gtk_widget_remove_css_class (GTK_WIDGET(pb_button), "pathbarbox" );
              gtk_widget_add_css_class (GTK_WIDGET(pb_button), "pathbarboxRed" );
            } else {
              gtk_widget_remove_css_class (GTK_WIDGET(pb_button), "pathbarboxRed" );
              gtk_widget_add_css_class (GTK_WIDGET(pb_button), "pathbarbox" );
            }

            
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
           /* FIXME
           auto myItemMenu = new Menu<PathbarMenu<bool>>(path);
           myItemMenu->setMenu(widget, parent, path);
           delete myItemMenu;
           */

            auto gesture = gtk_gesture_click_new();
            gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),1);
            if (pathbar_go_f) // FIXME: should reload treeviewmodel
              g_signal_connect (G_OBJECT(gesture) , "released", EVENT_CALLBACK (pathbar_go_f), (void *)pathbar);
            gtk_widget_add_controller(GTK_WIDGET(widget), GTK_EVENT_CONTROLLER(gesture));
 
        }

        // Now process to back and next buttons
        {
          auto next = GTK_WIDGET(g_object_get_data(G_OBJECT(pathbar), "next"));
          auto back = GTK_WIDGET(g_object_get_data(G_OBJECT(pathbar), "back"));
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
        }
    } 

    private:
  static void resetPathbarCSS(GtkBox *pathbar){
    //const gchar *path = workdir_;
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
        auto pathbar_p = (FileResponsePathbar *)data;
        auto pathbar = pathbar_p->pathbar();
        resetPathbarCSS(pathbar);
        auto eventBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
        auto path = (const char *) g_object_get_data(G_OBJECT(eventBox), "path");

        /* FIXME: use pathbar path here
        auto gridview_p = (GridView<LocalDir> *)Child::getGridviewObject();

        gtk_widget_remove_css_class (eventBox, "pathbarboxNegative" );
        if (strcmp(path, gridview_p->path())==0){
          gtk_widget_add_css_class (eventBox, "pathbarboxRed" );
        } else {
          gtk_widget_add_css_class (eventBox, "pathbarbox" );
        }
        */
        gtk_widget_remove_css_class (eventBox, "pathbarboxNegative" );
        gtk_widget_add_css_class (eventBox, "pathbarbox" );


        // red enabled (borken):
    /*    if (strcmp(pathbar_p->path(), path) == 0){
          DBG("set %s,%s to red\n", path, pathbar_p->path());
          gtk_widget_remove_css_class (eventBox, "pathbarboxNegative" );
          gtk_widget_remove_css_class (eventBox, "pathbarbox" );
          gtk_widget_add_css_class (eventBox, "pathbarboxRed" );
        } else {
          DBG("set %s,%s to blue\n", path, pathbar_p->path());

          gtk_widget_remove_css_class (eventBox, "pathbarboxNegative" );
          gtk_widget_remove_css_class (eventBox, "pathbarboxRed" );
          gtk_widget_add_css_class (eventBox, "pathbarbox" );
        }*/

        return FALSE;

    }

    void 
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
        /* FIXME: use pathbar path
        GList *children = g_list_first(children_list);
        auto gridview_p = (GridView<LocalDir> *)Child::getGridviewObject();
        for (;children && children->data; children=children->next){
            auto path = (const char *)g_object_get_data(G_OBJECT(children->data), "path");
            const char *css = "pathbarbox";
            if (gridview_p && gridview_p->path() && strcmp(path, gridview_p->path())==0) css = "pathbarboxRed";
            setPathButtonText(GTK_WIDGET(children->data), path, css);
        }
        g_list_free(children_list);
        */
        GList *children = g_list_first(children_list);
        for (;children && children->data; children=children->next){
            auto buttonPath = (const char *)g_object_get_data(G_OBJECT(children->data), "path");
            DBG("*** compare %s with %s (%s)\n", path, buttonPath, path_);
            if (strcmp(path, buttonPath)==0) {
              gtk_widget_remove_css_class (GTK_WIDGET(children->data), "pathbarbox" );
              gtk_widget_add_css_class (GTK_WIDGET(children->data), "pathbarboxRed" );
            } else {
              gtk_widget_remove_css_class (GTK_WIDGET(children->data), "pathbarboxRed" );
              gtk_widget_add_css_class (GTK_WIDGET(children->data), "pathbarbox" );
            }
        }
        g_list_free(children_list);


        auto lastPath = (char *) g_object_get_data(G_OBJECT(pathbar), "path");
        g_free(lastPath);
        g_object_set_data(G_OBJECT(pathbar), "path", g_strdup(path));
    }

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

  private:

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

  class FileResponse{
   GtkBox *mainBox_ = NULL;
   GtkWindow *dialog_ = NULL;
   char *title_ = _("Select Directory");
   const char *iconName_;
   GtkEntry *remoteEntry_ = NULL;
   GtkEntry *mountPointEntry_ = NULL;
   GtkTextView *output_;
   FileResponsePathbar *fileResponsePathbar_ = NULL;
public:
    const char *title(void){ return title_;}
    const char *iconName(void){ return "emblem-run";}
    const char *label(void){return "xffm::efs";}
    GtkEntry *remoteEntry(void){return remoteEntry_;}
    GtkEntry *mountPointEntry(void){return mountPointEntry_;}

    ~FileResponse (void){
      delete fileResponsePathbar_;
    }

    FileResponse (void){
      fileResponsePathbar_ = new FileResponsePathbar;
    }

     static void *asyncYes(void *data){
      auto dialogObject = (DialogComplex<FileResponse> *)data;
      DBG("%s", "hello world\n");
      return NULL;
    }

    static void *asyncNo(void *data){
      auto dialogObject = (DialogComplex<FileResponse> *)data;
      DBG("%s", "goodbye world\n");
      return NULL;
    }
    static gint 
    compareFunction(const void *a, const void *b, void *data){

        GFileInfo *infoA = G_FILE_INFO(a);
        GFileInfo *infoB = G_FILE_INFO(b);

        auto nameA = g_file_info_get_name(infoA);
        auto nameB = g_file_info_get_name(infoB);
        

        if (strcmp(g_file_info_get_name(infoA), "..")==0) return -1;
        if (strcmp(g_file_info_get_name(infoB), "..")==0) return 1;
        
        // by name 
        return strcasecmp(nameA, nameB);
    }
    
    static GListModel *getChildModel (gpointer listItem, void *data)
    {
      auto info = G_FILE_INFO(listItem);
      auto path = Basic::getPath(info);
      DBG("getChildModel %s \n", path);
      auto listModel = getListModel(path);
      g_free(path);
      return listModel;
    }

    static GListModel *getListModel(const char *path){
        GError *error_ = NULL;
        auto store = g_list_store_new(G_TYPE_FILE_INFO);
        GFile *file = g_file_new_for_path(path);
        GFileEnumerator *dirEnum = 
          g_file_enumerate_children (file,"standard::",G_FILE_QUERY_INFO_NONE,NULL, &error_);
        if (error_) {
          TRACE("*** Error::g_file_enumerate_children: %s\n", error_->message);
          Print::printError(Child::getOutput(), g_strdup(error_->message));
          g_error_free(error_);
          return NULL;
        }
        GFile *outChild = NULL;
        GFileInfo *outInfo = NULL;
        int items = 0;
        do {
          g_file_enumerator_iterate (dirEnum, &outInfo, &outChild,
              NULL, // GCancellable* cancellable,
              &error_);
          if (error_) {
            DBG("*** Error::g_file_enumerator_iterate: %s\n", error_->message);
            return NULL;
          }
          if (!outInfo || !outChild) break;
          if (g_file_info_get_is_symlink(outInfo)) continue;
          if (g_file_info_get_is_backup(outInfo)) continue;
          if (g_file_info_get_is_hidden(outInfo)) continue;
          auto path = g_file_get_path(outChild);

          if (g_file_test(path, G_FILE_TEST_IS_DIR)){
            //Important: if this is not set, then the GFile cannot be obtained from the GFileInfo:
            g_file_info_set_attribute_object(outInfo, "standard::file", G_OBJECT(outChild));          
            g_list_store_insert_sorted(store, G_OBJECT(outInfo), compareFunction, NULL);
            TRACE("insert path=%s info=%p\n", g_file_get_path(outChild), outInfo);
            items++;
          }
          g_free(path);
          /*auto _path = g_file_get_path(outChild);
          setPaintableIcon(outInfo, _path);
          g_free(_path);*/
        } while (true);
        g_object_unref(file);
        if (!items){
          g_object_unref(store);
          return NULL;
        }
        return G_LIST_MODEL(store);
    }

      static void
      factorySetup1(GtkSignalListItemFactory *self, GObject *object, void *data){
        auto box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        TRACE("factorySetup1...\n");        
        auto expander = gtk_tree_expander_new();
        gtk_tree_expander_set_child(GTK_TREE_EXPANDER(expander), box);
        gtk_list_item_set_child(GTK_LIST_ITEM(object), expander); 
        g_object_set_data(G_OBJECT(object), "box", box);
        if (data) {
          auto paintable = Texture<bool>::load16("folder");
          auto image = gtk_image_new_from_paintable(paintable);
          gtk_box_append(GTK_BOX(box), image);
        }
        auto label = gtk_label_new("");
        gtk_box_append(GTK_BOX(box), label);
        g_object_set_data(G_OBJECT(object), "label", label);
        TRACE("factorySetup2...\n");        
      }

      static void
      factoryBind1(GtkSignalListItemFactory *factory, GObject *object, void *data){
        TRACE("factoryBind1...\n");

        auto list_item =GTK_LIST_ITEM(object);
        auto treeListRow = GTK_TREE_LIST_ROW(gtk_list_item_get_item(list_item));
        auto info = G_FILE_INFO(gtk_tree_list_row_get_item(treeListRow));
        if (data){
          auto expander = gtk_list_item_get_child( GTK_LIST_ITEM(object) );
          gtk_tree_expander_set_list_row(GTK_TREE_EXPANDER(expander), treeListRow);
        }
        //auto info = G_FILE_INFO(object);
        DBG("info name = %s\n", g_file_info_get_name(info));

        auto label = GTK_LABEL(g_object_get_data(object, "label"));
        char *markup = NULL;
        if (data) {
          const char *name = g_file_info_get_name(info);          
          auto maxLen = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(factory), "maxLen"));
          auto format = g_strdup_printf("<tt>%%-%ds", maxLen);
          char buffer[128];
          snprintf(buffer, 128, (const char *)format, name);
          markup = g_strdup_printf("%s</tt>", buffer);                    
          gtk_label_set_markup(label, markup);
        } else {
          auto path = Basic::getPath(info);
          struct stat st;
          lstat(path, &st);
          g_free(path);
          auto m1 = Basic::statInfo(&st);
          markup = g_strdup("<tt> <span color=\"blue\" size=\"small\">");
          Basic::concat(&markup, m1);
          Basic::concat(&markup, "</span></tt>");           
          g_free(m1);                   
          gtk_label_set_markup(label, markup);
        }
        g_free(markup);
        
      }
/*
    static void addGestureClickDown(GtkWidget *self, GObject *item, GridView<DirectoryClass> *gridView_p){
      g_object_set_data(G_OBJECT(self), "item", item);
      auto gesture = gtk_gesture_click_new();
      gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),1); 
      // 1 for select
      TRACE("addGestureClickDown: self = %p, item=%p\n", self, item);
      g_signal_connect (G_OBJECT(gesture) , "released", EVENT_CALLBACK (reload_f), (void *)gridView_p);
      gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(gesture));
      gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture), 
          GTK_PHASE_CAPTURE);

    }    
*/
    GtkBox *mainBox(void) {
      const char *path = "/home/edscott";
        //auto dialog = gtk_dialog_new ();
        //gtk_window_set_type_hint(GTK_WINDOW(dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
        mainBox_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        //gtk_widget_set_size_request(GTK_WIDGET(mainBox_), 550, 400);
        gtk_widget_set_vexpand(GTK_WIDGET(mainBox_), false);
        gtk_widget_set_hexpand(GTK_WIDGET(mainBox_), false);

        auto label = gtk_label_new("file response dialog now...\n");
        gtk_box_append(mainBox_, label);
        auto pathbarBox = fileResponsePathbar_->pathbar();
        fileResponsePathbar_->path(path);
        fileResponsePathbar_->updatePathbarBox(path, pathbarBox, NULL);
        
        //UtilPathbar<LocalDir>::updatePathbar(path, pathbar, false, NULL);
        
        gtk_box_append(mainBox_, GTK_WIDGET(fileResponsePathbar_->pathbar()));

        auto sw = gtk_scrolled_window_new();
        gtk_widget_set_vexpand(GTK_WIDGET(sw), true);
        gtk_widget_set_hexpand(GTK_WIDGET(sw), true);
        gtk_box_append(mainBox_, sw);
        // listview...
        // gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(sw), GTK_WIDGET(output_));
        gtk_widget_set_size_request(GTK_WIDGET(sw), 680, 200);
      //  auto listModel = getListModel("/");
        auto listModel = getListModel(path);

  GtkTreeListModel * treemodel = gtk_tree_list_model_new (G_LIST_MODEL (listModel),
                                       FALSE, // passthrough
                                       FALSE, // autoexpand TRUE,
                                       getChildModel,
                                       NULL,
                                       NULL);
       
  auto filterModel = gtk_filter_list_model_new (G_LIST_MODEL (treemodel), NULL);
  GtkSingleSelection *selection = gtk_single_selection_new (G_LIST_MODEL (filterModel));
  
  auto maxLen = Basic::getMaxNameLen(listModel);
  auto columnView = gtk_column_view_new(GTK_SELECTION_MODEL(selection));
  gtk_column_view_set_show_column_separators (GTK_COLUMN_VIEW (columnView), false);
  GtkColumnViewColumn *column;

  GtkListItemFactory *factory1 = gtk_signal_list_item_factory_new();
  column = gtk_column_view_column_new (_("Name"), factory1);
  gtk_column_view_append_column (GTK_COLUMN_VIEW (columnView), column);
  g_signal_connect (factory1, "setup", G_CALLBACK (factorySetup1), GINT_TO_POINTER(1));
  g_signal_connect (factory1, "bind", G_CALLBACK (factoryBind1), GINT_TO_POINTER(1));
  g_object_unref (column);


  GtkListItemFactory *factory2 = gtk_signal_list_item_factory_new();
  column = gtk_column_view_column_new (_("Information"), factory2);
  gtk_column_view_append_column (GTK_COLUMN_VIEW (columnView), column);
  g_signal_connect (factory2, "setup", G_CALLBACK (factorySetup1), NULL);
  g_signal_connect (factory2, "bind", G_CALLBACK (factoryBind1), NULL);
  g_object_unref (column);
  
  //auto listview = gtk_list_view_new (GTK_SELECTION_MODEL(selection), factory);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(sw), GTK_WIDGET(columnView));
 
      

        auto action_area = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_vexpand(GTK_WIDGET(action_area), false);
        gtk_widget_set_hexpand(GTK_WIDGET(action_area), false);
        gtk_box_append(mainBox_, GTK_WIDGET(action_area));

        auto cancelButton = Basic::mkButton("emblem-redball", _("Cancel"));
        gtk_box_append(action_area,  GTK_WIDGET(cancelButton));
        gtk_widget_set_vexpand(GTK_WIDGET(cancelButton), false);

        auto saveButton = Basic::mkButton ("emblem-floppy", _("Accept"));
        gtk_box_append(action_area,  GTK_WIDGET(saveButton));
        gtk_widget_set_vexpand(GTK_WIDGET(saveButton), false);


        g_signal_connect (G_OBJECT (saveButton), "clicked", G_CALLBACK (button_save), this);
        g_signal_connect (G_OBJECT (cancelButton), "clicked", G_CALLBACK (button_cancel), this);

        // FIXME: 
        return mainBox_;
    }

    void setSubClassDialog(GtkWindow *dialog){
      dialog_ = dialog;
    }

    GtkWindow *dialog(void){return dialog_;}

    private:

    
 

    gboolean save(void){
      return true;        
    }

    static void
    button_save (GtkButton * button, gpointer data) {
      auto subClass = (FileResponse *)data;
      if (subClass->save()){
        g_object_set_data(G_OBJECT(subClass->dialog()), "response", GINT_TO_POINTER(1));
      }
    }

    static void
    button_cancel (GtkButton * button, gpointer data) {
      auto subClass = (FileResponse *)data;
      g_object_set_data(G_OBJECT(subClass->dialog()), "response", GINT_TO_POINTER(-1));
    }

  };

  class FileDialog {
    public:
    static void newFileDialog(void **newDialog){
      auto dialogObject = new DialogComplex<FileResponse>;
      //
      dialogObject->setParent(GTK_WINDOW(MainWidget));
      auto dialog = dialogObject->dialog();
      newDialog[0] = (void *)dialog;
      
      gtk_window_set_decorated(dialog, true);
      dialogObject->setSubClassDialog();

      gtk_widget_realize(GTK_WIDGET(dialog));
      Basic::setAsDialog(GTK_WIDGET(dialog), "dialog", "Dialog");
      gtk_window_present(dialog);
      
      TRACE("FileDialog:: newDialog[0] = %p\n", newDialog[0]);

      dialogObject->run();
      

    }



  };


}
#endif

