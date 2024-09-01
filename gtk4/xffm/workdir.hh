#ifndef UTILWORKDIR_HH
#define UTILWORKDIR_HH
#include "texture.hh"
#include "fm/gridview.hh"
namespace xf {
  template <class GridviewClass>
  class Workdir  {
    private:
      static void  updateGridView(const char *path){
        auto view = GridviewClass::getGridView(path, (void *)gridViewClick);
        auto gridScrolledWindow = Child::getGridScrolledWindow();
        gtk_scrolled_window_set_child(gridScrolledWindow, view);
      }

    public:
    static bool setWorkdir(const gchar *path, GtkWidget *child){
      TRACE("setWorkdir...A\n");
      auto wd = (char *)getWorkdir(child);
      if (strcmp(wd, path)){
        g_free(wd);
        g_object_set_data(G_OBJECT(child), "path", g_strdup(path));
      }
      updatePathbar(false);
      updateGridView(path);
      return true;
    }
    static bool setWorkdir(const gchar *path){
      TRACE("setWorkdir...A\n");
      auto child = Child::getCurrentChild();
      auto wd = (gchar *)g_object_get_data(G_OBJECT(child), "path");
      g_free(wd);
      g_object_set_data(G_OBJECT(child), "path", g_strdup(path));
      updatePathbar(true);
      updateGridView(path);
      return true;
    }
    static bool setWorkdir(const gchar *path, bool updateHistory){
      TRACE("setWorkdir B...path=%s\n",path);
      auto child = Child::getCurrentChild();
      auto wd = (gchar *)g_object_get_data(G_OBJECT(child), "path");
      g_free(wd);
      g_object_set_data(G_OBJECT(child), "path", g_strdup(path));
      updatePathbar(updateHistory);
      updateGridView(path);
      return true;
    }
    static bool setWorkdir(const gchar *path, GtkBox *pathbar, bool updateHistory){
      TRACE("setWorkdir...C\n");
      if (!MainWidget) return false;
      auto child = GTK_WIDGET(g_object_get_data(G_OBJECT(pathbar), "child"));
      auto wd = (gchar *)g_object_get_data(G_OBJECT(child), "path");
      g_free(wd);
      g_object_set_data(G_OBJECT(child), "path", g_strdup(path));
      UtilBasic::setWindowTitle(child);
      updatePathbar(path, pathbar, updateHistory);
      updateGridView(path);
      return true;
    }

    static gboolean
    gridViewClick(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer object){

      auto eventController = GTK_EVENT_CONTROLLER(self);
      auto event = gtk_event_controller_get_current_event(eventController);
      
      auto modType = gdk_event_get_modifier_state(event);

      TRACE("modType = 0x%x\n", modType);
      if (modType & GDK_CONTROL_MASK) return FALSE;
      if (modType & GDK_SHIFT_MASK) return FALSE;
      
      TRACE("gestureClick; object=%p button=%d\n", object,
          gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(self)));

      auto info = G_FILE_INFO(gtk_list_item_get_item(GTK_LIST_ITEM(object)));
      auto file = G_FILE(g_file_info_get_attribute_object (info, "standard::file"));
      auto root = g_file_info_get_attribute_object (info, "xffm::root");
      if (root){
        setWorkdir("xffm:root");
        return TRUE;
      }
      TRACE("gestureClick; file=%p\n", file);
      auto path = g_file_get_path(file);
      TRACE("gestureClick; path=%p\n", path);
      TRACE("click on %s\n", path);
      auto type = g_file_info_get_file_type(info);
      if ((type == G_FILE_TYPE_DIRECTORY )||(GridviewClass::symlinkToDir(info, type))) {
        TRACE("Go to action...\n");
        auto child = UtilBasic::getCurrentChild();
        setWorkdir(path);
      } else {
        DBG("mimetype action...\n");
      }
      g_free(path);
      return TRUE;
    }
   
    ///////////////////   pathbar  ///////////////////////////////////
    private:
    static void 
    updatePathbar(bool updateHistory){
        const gchar *path = getWorkdir();
        GtkBox *pathbar = Child::getPathbar();
        TRACE( "update pathbar to %s (update=%d)\n", path, updateHistory);
        void *arg[]={(void *)(path?g_strdup(path):NULL), (void *)pathbar, GINT_TO_POINTER(updateHistory)};
        UtilBasic::context_function(update_pathbar_f, arg);
    }
    static void 
    updatePathbar(const gchar *path, GtkBox *pathbar, bool updateHistory){
        TRACE( "update pathbar to %s (update=%d)\n", path, updateHistory);
        void *arg[]={(void *)(path?g_strdup(path):NULL), (void *)pathbar, GINT_TO_POINTER(updateHistory)};
        UtilBasic::context_function(update_pathbar_f, arg);
    }
    static void *
    update_pathbar_f(void *data){
        void **arg = (void **)data;
        ///Pathbar *pathbar_p = (Pathbar *)arg[0];
        auto path = (gchar *)arg[0];
        auto pathbar = GTK_BOX(arg[1]);
        auto updateHistory = GPOINTER_TO_INT(arg[2]);
        TRACE( "update_pathbar_f:: %s\n", path);

        if (!pathbar) return NULL;
        if (!path){
            TRACE("##### togglePathbar(NULL, pathbar)\n");
            togglePathbar(NULL, pathbar);
//            pathbar_p->toggle_pathbar(NULL);
            return NULL;
        }
        if (updateHistory) {
          
          GList *historyBack = (GList *)g_object_get_data(G_OBJECT(pathbar), "historyBack");
          if (historyBack){
            if (strcmp(path, (const char *)historyBack->data) != 0){
              historyBack = g_list_prepend(historyBack, g_strdup(path));
            }
          } else {
              historyBack = g_list_prepend(historyBack, g_strdup(path));
          }
          g_object_set_data(G_OBJECT(pathbar), "historyBack", historyBack);
          // wipe next history 
          GList *historyNext = (GList *)g_object_get_data(G_OBJECT(pathbar), "historyNext");
          for (GList *l=historyNext; l && l->data; l=l->next) g_free(l->data);
          g_list_free(historyNext);
          g_object_set_data(G_OBJECT(pathbar), "historyNext", NULL);
        }
        

        // Trim pathbar.
        gchar **paths;
        if (strcmp(path, G_DIR_SEPARATOR_S)==0){
            paths = (gchar **)calloc(2, sizeof(gchar *));
            if (!paths){
                g_warning("updatePathbar(): cannot malloc\n");
                return NULL;
            }
            paths[1]=NULL;
        } else {
            paths = g_strsplit(path, G_DIR_SEPARATOR_S, -1);
            g_free(paths[0]);
        }
        paths[0]= g_strdup(G_DIR_SEPARATOR_S);

        GList *children_list = UtilBasic::getChildren(pathbar);
        //GList *children_list = gtk_container_get_children(GTK_CONTAINER(pathbar));
        gint i=0;
        gchar *pb_path = NULL;
        for (GList *children = children_list;children && children->data; children=children->next){
            gchar *name = (gchar *)g_object_get_data(G_OBJECT(children->data), "name");
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

            UtilBasic::boxPack0 (pathbar, GTK_WIDGET(pb_button), FALSE, FALSE, 0);
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
            
         /*   auto click = gtk_event_controller_legacy_new();
            g_signal_connect (G_OBJECT(click) , "event", EVENT_CALLBACK (pathbarGo), (void *)pb_button);
            gtk_widget_add_controller(GTK_WIDGET(pb_button), GTK_EVENT_CONTROLLER(click));*/

            auto gesture1 = gtk_gesture_click_new();
            gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture1),1);
            g_signal_connect (G_OBJECT(gesture1) , "released", EVENT_CALLBACK (pathbar_go), (void *)pathbar);
            gtk_widget_add_controller(GTK_WIDGET(pb_button), GTK_EVENT_CONTROLLER(gesture1));
            
            auto gesture3 = gtk_gesture_click_new();
            gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture3),3);
            g_signal_connect (G_OBJECT(gesture3) , "released", EVENT_CALLBACK (pathbar_go), (void *)pathbar);
            gtk_widget_add_controller(GTK_WIDGET(pb_button), GTK_EVENT_CONTROLLER(gesture3));
 
            gtk_widget_set_visible(GTK_WIDGET(pb_button), TRUE);
        }
        g_free(pb_path);
        g_strfreev(paths);
        
        // show what fits
        togglePathbar(path, pathbar);
        g_free(path);

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
        return NULL;
    }

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
        GList *children_list = UtilBasic::getChildren(pathbar);

        if (gtk_widget_get_realized(MainWidget)) showWhatFits(pathbar, path, children_list);
        else {TRACE("MainWidget not yet realized...\n");}

        /*if (gtk_widget_is_visible(GTK_WIDGET(mainWindow))) showWhatFits(pathbar_, path, children_list);
        else gtk_widget_show_all(GTK_WIDGET(pathbar_));*/


        // Finally, we differentiate active button.
        GList *children = g_list_first(children_list);
        for (;children && children->data; children=children->next){
            setPathButtonText(GTK_WIDGET(children->data), path, "blue", NULL);
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
        auto path = (const char *) g_object_get_data(G_OBJECT(pathbar), "path");

        setPathButtonText(eventBox, path, "white", "#acaaa5");
        return FALSE;
    }

    static gboolean
    pathbar_blue (GtkEventControllerMotion* self,
                    gdouble x,
                    gdouble y,
                    gpointer data) {
        auto pathbar = GTK_BOX(data);
        auto eventBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
        auto path = (const char *) g_object_get_data(G_OBJECT(pathbar), "path");

        setPathButtonText(eventBox, path, "blue", NULL);
        return FALSE;

    }
    /* legacy
    static gboolean
    pathbarGo (
              GtkEventControllerLegacy* self,
              GdkEvent* event,
              gpointer data ) 
    {
        auto eventBox = GTK_BOX(data);
        auto type = gdk_event_get_event_type(event);
        if (type != GDK_BUTTON_RELEASE) return FALSE;
        TRACE("button release...\n");

        return TRUE;
    }*/
    private:
    static void
    openNewTab(GtkButton *self, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(self), "menu"));
      gtk_popover_popdown(menu);

    }
    static void
    paste(GtkButton *self, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(self), "menu"));
      gtk_popover_popdown(menu);
    }
    static void
    showPaste(GtkButton *self, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(self), "menu"));
      gtk_popover_popdown(menu);
    }
    public:
    static const gchar *getWorkdir(GtkWidget *child){
      TRACE("getWorkdir...\n");
      if (!MainWidget) return NULL;
      return (const gchar *)g_object_get_data(G_OBJECT(child), "path");
    }
    static const gchar *getWorkdir(void){
      auto child =  Child::getCurrentChild();
      return getWorkdir(child);
    }

    static gboolean
    pathbar_go (
              GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer data ) 
    {
        auto pathbar = GTK_BOX(data);
        auto eventBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
        auto name = (char *) g_object_get_data(G_OBJECT(eventBox), "name");
        auto path = (char *) g_object_get_data(G_OBJECT(eventBox), "path");
        auto button = gtk_gesture_single_get_button(GTK_GESTURE_SINGLE(self));
          TRACE("pathbar goto... name=%s, path=%s\n", name, path);
        if (button == 1){
          TRACE("pathbar goto...\n");
          //if (strcmp(path, "xffm:root")==0) setWorkdir(g_get_home_dir(), pathbar, true);
          //else setWorkdir(path, pathbar, true);
          setWorkdir(path, pathbar, true);
          return TRUE;
        }
        if (button == 3){
          TRACE("pathbar menu...\n");
          GtkPopover *menu = GTK_POPOVER(g_object_get_data(G_OBJECT(pathbar), "menu"));
          const char *text[] = {_("Open in new tab"), _("Paste"), _("Preview of data from clipboard"), NULL};
          GHashTable *mHash[3];
          mHash[0] = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);
          for (int i=1; i<3; i++) mHash[i] = g_hash_table_new(g_str_hash, g_str_equal);
          g_hash_table_insert(mHash[0], _("Open in new tab"), g_strdup(LIST_ADD));
          g_hash_table_insert(mHash[1], _("Open in new tab"), (void *) openNewTab); // callback
          g_hash_table_insert(mHash[2], _("Open in new tab"), NULL); // data
          g_hash_table_insert(mHash[0], _("Paste"), g_strdup(EDIT_PASTE));
          g_hash_table_insert(mHash[1], _("Paste"), (void *) paste);
          g_hash_table_insert(mHash[2], _("Paste"), NULL);
          g_hash_table_insert(mHash[0], _("Preview of data from clipboard"), g_strdup("view-reveal"));
          g_hash_table_insert(mHash[1], _("Preview of data from clipboard"), (void *)showPaste );
          g_hash_table_insert(mHash[2], _("Preview of data from clipboard"), NULL);

          if (!menu) {
            menu = UtilBasic::mkMenu(text, mHash, path);
            gtk_widget_set_parent(GTK_WIDGET(menu), GTK_WIDGET(pathbar));
            //gtk_widget_realize(GTK_WIDGET(menu));
            g_object_set_data(G_OBJECT(pathbar), "menu", menu);
          }
          UtilBasic::setMenuTitle(menu, path);
          gtk_popover_popup(menu);
                
          return TRUE;
        }
        //TRACE("pathbar_go...name=%s, path=%s button=%d\n", name, path, button);
        return FALSE;
        /*
        
        if (event->button == 1) {
            pathbar_p->pathbar_ok(eventBox);
        }

        if (event->button == 3) {
            auto view = pathbar_p->pathbarView();
            const gchar *path = pathbar_p->getClickPath(eventBox);
            TRACE("***clickpath=%s\n", path);
            GtkMenu *menu = NULL;
            if (g_file_test(path, G_FILE_TEST_IS_DIR)){ 
                menu = LocalPopUp<Type>::popUp();
                Popup<Type>::setWidgetData(menu, "path", path);
                g_object_set_data(G_OBJECT(menu),"view", NULL);
                BaseSignals<Type>::configureViewMenu(LOCALVIEW_TYPE);
            } else {
                // do xffm:root menu
                RootPopUp<Type>::resetPopup();
                menu = RootPopUp<Type>::popUp();
            }
            if (menu) {
                gtk_menu_popup_at_pointer (menu, (const GdkEvent *)event);
            }          
        }

        return FALSE;*/
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
            DBG("***Error:: gtk_widget_compute_bounds(MainWidget)\n");
          }
        }
      } else {
        if (!gtk_widget_compute_bounds(GTK_WIDGET(pathbar), GTK_WIDGET(pathbar), &bounds)) {
          DBG("***Error:: gtk_widget_compute_bounds(pathbar)\n");
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
              DBG("***Error:: showWhatFits():gtk_widget_compute_bounds()\n");
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
              DBG("***Error:: showWhatFits():gtk_widget_compute_bounds()\n");
            }
            width -= bounds.size.width;

            if (width < 0) break;
            gtk_widget_set_visible(GTK_WIDGET(children->data), TRUE);
        }
    }
  public:
    static void 
    setPathButtonText(GtkWidget *eventBox, const gchar *path, const gchar *color, const gchar *bgcolor){
        //const gchar *fontSize = "size=\"small\"";
        const gchar *fontSize = "";
        gchar *name = (gchar *)g_object_get_data(G_OBJECT(eventBox), "name");
        if (strcmp(name, "RFM_ROOT")==0) {
            // no path means none is differentiated.
            gchar *markup = g_strdup_printf("<span %s color=\"%s\" bgcolor=\"%s\">  %s  </span>", fontSize, color, bgcolor?bgcolor:"#dcdad5", ".");
            auto label = GTK_LABEL(g_object_get_data(G_OBJECT(eventBox), "label"));
            gtk_label_set_markup(label, markup);
            g_free(markup);
            return;
        } 
        if (strcmp(name, "RFM_GOTO")==0) {
            return;
        } 
        const gchar *pb_path = 
            (const gchar *)g_object_get_data(G_OBJECT(eventBox), "path");
        if (!pb_path){
            g_warning("rfm_update_pathbar(): pb_path is null\n");
            return;
        }
        if (!strlen(pb_path)) pb_path=G_DIR_SEPARATOR_S;//?
        if (strcmp(pb_path, path)==0) {
            gchar *v = UtilBasic::utf_string(name);
            gchar *g = g_markup_escape_text(v, -1);
            g_free(v);
            gchar *markup = g_strdup_printf("<span %s color=\"%s\" bgcolor=\"%s\">  %s  </span>", fontSize, bgcolor?"white":"red", bgcolor?bgcolor:"#dcdad5", g);
            auto label = GTK_LABEL(g_object_get_data(G_OBJECT(eventBox), "label"));
            gtk_label_set_markup(label, markup);

            g_free(g);
            g_free(markup);
        }
        else {
            gchar *v = UtilBasic::utf_string(name);
            gchar *g = g_markup_escape_text(v, -1);
            g_free(v);
            gchar *markup = g_strdup_printf("<span %s color=\"%s\" bgcolor=\"%s\">  %s  </span>", fontSize, color, bgcolor?bgcolor:"#dcdad5", g);
            auto label = GTK_LABEL(g_object_get_data(G_OBJECT(eventBox), "label"));
            gtk_label_set_markup(label, markup);

            g_free(g);
            g_free(markup);
        }
        return;
    }


  };
}
#endif
