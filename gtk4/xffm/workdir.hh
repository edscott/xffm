#ifndef UTILWORKDIR_HH
#define UTILWORKDIR_HH
#include "texture.hh"
namespace xf {
      static GtkPopover *popper=NULL;
  class Workdir {
    public:
    static bool setWorkdir(const gchar *path, GtkWidget *child){
      TRACE("setWorkdir...A\n");
      auto wd = (char *)getWorkdir(child);
      if (strcmp(wd, path)){
        g_free(wd);
        g_object_set_data(G_OBJECT(child), "path", g_strdup(path));
      }
      updatePathbar(false);
      updateGridView(child, path);
      return true;
    }
    static bool setWorkdir(const gchar *path){
      TRACE("setWorkdir...A\n");
      auto child = Child::getCurrentChild();
      auto wd = (gchar *)g_object_get_data(G_OBJECT(child), "path");
      g_free(wd);
      g_object_set_data(G_OBJECT(child), "path", g_strdup(path));
      updatePathbar(true);
      updateGridView(child, path);
      return true;
    }
    static bool setWorkdir(const gchar *path, bool updateHistory){
      TRACE("setWorkdir B...path=%s\n",path);
      auto child = Child::getCurrentChild();
      auto wd = (gchar *)g_object_get_data(G_OBJECT(child), "path");
      g_free(wd);
      g_object_set_data(G_OBJECT(child), "path", g_strdup(path));
      updatePathbar(updateHistory);
      updateGridView(child, path);
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
      updateGridView(child, path);
      return true;
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
//////////////////////////  gridview  ///////////////////////////
  public:
      static void
      updateGridView(GtkWidget *child, const char *path){
        auto gridScrolledWindow = GTK_SCROLLED_WINDOW(g_object_get_data(G_OBJECT(child), "gridScrolledWindow"));
        // This is not necesary since it will unref gridScrolledWindow, apparently.
        //auto old = gtk_scrolled_window_get_child(gridScrolledWindow);
        //if (old && GTK_IS_WIDGET(old)) gtk_widget_unparent(old);
        auto view = getGridView(path);
        gtk_scrolled_window_set_child(gridScrolledWindow, view);

      }
  private:
    static gboolean
    gestureClick(GtkGestureClick* self,
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
        Workdir::setWorkdir("xffm:root");
        return TRUE;
      }
      TRACE("gestureClick; file=%p\n", file);
      auto path = g_file_get_path(file);
      TRACE("gestureClick; path=%p\n", path);
      TRACE("click on %s\n", path);
      auto type = g_file_info_get_file_type(info);
      if ((type == G_FILE_TYPE_DIRECTORY )||(symlinkToDir(info, type))) {
        TRACE("Go to action...\n");
        auto child = UtilBasic::getCurrentChild();
        Workdir::setWorkdir(path);
      } else {
        DBG("mimetype action...\n");
      }
      g_free(path);
      return TRUE;
    }

/*    static gboolean
    showOpen (
      GtkWidget* self,
      gint x,
      gint y,
      gboolean keyboard_mode,
      GtkTooltip* tooltip,
      gpointer user_data
    ) {
      auto texture = Texture::load(info);
      gtk_tooltip_set_icon(tooltip, texture);
      return TRUE;
    }*/
    
    static void addGestureClick(GtkWidget *imageBox, GObject *object){
      TRACE("addGestureClick; object=%p\n", object);
      auto gesture = gtk_gesture_click_new();
      gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),1); 
      // 1 for action released, 3 for popover pressed
      // Add a different gtk_gesture_click_new for 3 and menu.
      g_signal_connect (G_OBJECT(gesture) , "pressed", EVENT_CALLBACK (gestureClick), (void *)object);
//      g_signal_connect (G_OBJECT(gesture) , "released", EVENT_CALLBACK (gestureClick), (void *)object);
      gtk_widget_add_controller(GTK_WIDGET(imageBox), GTK_EVENT_CONTROLLER(gesture));
      gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture), 
          GTK_PHASE_CAPTURE);
    }    

      static void
      factorySetup(GtkSignalListItemFactory *self, GObject *object, void *data){
        GtkWidget *box;
        if (Settings::getInteger("xfterm", "iconsize") <= 30)
        {
          box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        } else {
          box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
        }
        gtk_widget_add_css_class(box, "gridviewBox");
        gtk_widget_set_vexpand(GTK_WIDGET(box), FALSE);
        gtk_widget_set_hexpand(GTK_WIDGET(box), FALSE);
        GtkListItem *list_item = GTK_LIST_ITEM(object);
        gtk_list_item_set_child(list_item, box);

   }
 /*     static void popup(GtkWidget *parent, const char *text){
        if (!popper) {
          popper = GTK_POPOVER(gtk_popover_new ());
          gtk_popover_set_autohide(GTK_POPOVER(popper), TRUE);
          gtk_popover_set_has_arrow(GTK_POPOVER(popper), FALSE);
          gtk_widget_add_css_class (GTK_WIDGET(popper), "inquire" );
        }
        //gtk_widget_unparent (GTK_WIDGET(popper));

        // Important: must use both of the following instructions:
        gtk_widget_set_parent (GTK_WIDGET(popper), parent);
        gtk_popover_set_default_widget(popper, parent);
        
        gtk_popover_set_child (popper, gtk_label_new(text));
        
        gtk_popover_popup(popper);
   

      }
      static void popdown(void){
        if (!popper) return;
        gtk_popover_popdown(popper);
      }*/

      /* The bind function for the factory */
      static void
      factoryBind(GtkSignalListItemFactory *self, GObject *object, void *data)
      {
        auto list_item =GTK_LIST_ITEM(object);
        auto box = GTK_BOX(gtk_list_item_get_child( list_item ));
        auto list = UtilBasic::getChildren(box);
        for (auto l=list; l && l->data; l=l->next){
          gtk_widget_unparent(GTK_WIDGET(l->data));
        }
        
        auto info = G_FILE_INFO(gtk_list_item_get_item(list_item));

        
        int scaleFactor = 1;
        char *name = g_strdup(g_file_info_get_name(info));
        auto texture = Texture::load(name);
        if (texture) scaleFactor = 2;
        if (!texture) {
          //texture = Texture::loadIconName("emblem-archlinux");
          texture = Texture::load(info);
        }
        if (!texture) {
            TRACE("Iconmview::load(): Texture::load(info) == NULL\n");
        }
          
        GtkWidget *image = gtk_image_new_from_paintable(GDK_PAINTABLE(texture));
        auto size = Settings::getInteger("xfterm", "iconsize");

        auto type = g_file_info_get_file_type(info);
     //   if ((type == G_FILE_TYPE_DIRECTORY )||(symlinkToDir(info, type))) {
        if (type == G_FILE_TYPE_DIRECTORY ) {
          addDirectoryTooltip(image, info);
        }
        if (name[0] == '.' && name[1] != '.') {
          // We have texture...
          // What's the icon name?
          // We have 
          //   0. bug: check why no previews in ~/tmp...
          //   1. get icon names
          //   2. find icon source file in theme or
          //   3. find icon source file in search path
          //   4. must have backup in xffm+ icons.
          //   5. get the cairo surface
          //   6. apply mask
          //   7. render paintable
          //   8. return GtkImage
          //   9. For symlinks, add symlink emblem.
          //   10. For executables, add exe emblem.
          auto gIcon = g_file_info_get_icon(info);
          auto tIcon = G_THEMED_ICON(gIcon);
          auto names = g_themed_icon_get_names(tIcon);
          char *tname = gtk_icon_theme_get_theme_name(icon_theme);
          DBG("theme=%s\n", tname);
          g_free(tname);
          //for (auto p=names; p && *p; p++) DBG("%s tIcon name=%s\n", name, *p);
          //auto paths = gtk_icon_theme_get_search_path(icon_theme);
          //for (auto p=paths; p && *p; p++) DBG("%s paths=%s\n", name, *p);
          //auto paths = gtk_icon_theme_get_resource_path(icon_theme);
          //for (auto p=paths; p && *p; p++) DBG("%s resource paths=%s\n", name, *p);
          //
          auto file = "/usr/share/icons/Adwaita/scalable/mimetypes/application-certificate.svg";
          auto paintable = Cairo::getSvgPaintable(file, size, size);   
          image = gtk_image_new_from_paintable(paintable);

          //const char *testFile = "/home/edscott/tmp/testFile.png";
          //cairo_surface_write_to_png (surface, testFile); 
        }

        if (size < 0) size = 48;
        gtk_widget_set_size_request(image, size*scaleFactor, size*scaleFactor);

   

        //auto label = GTK_LABEL(g_object_get_data(G_OBJECT(vbox), "label"));

        if (name && strlen(name) > 15){
          name[15] = 0;
          name[14] ='~';
        }
        char *markup = g_strconcat("<span size=\"small\">", name, "</span>", NULL);
        auto label = gtk_label_new("");
        gtk_label_set_markup(GTK_LABEL(label), markup);
        g_free(markup);

        auto imageBox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_add_css_class(imageBox, "gridviewBox");
        auto labelBox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_add_css_class(labelBox, "gridviewBox");

        UtilBasic::boxPack0(GTK_BOX(imageBox), GTK_WIDGET(image), FALSE, FALSE, 0);    
        UtilBasic::boxPack0(GTK_BOX(box), imageBox, FALSE, FALSE, 0);    
        UtilBasic::boxPack0(GTK_BOX(labelBox), label, FALSE, FALSE, 0);    
        UtilBasic::boxPack0(GTK_BOX(box), labelBox, FALSE, FALSE, 0);    
        if (Settings::getInteger("xfterm", "iconsize") == 24 && strcmp(name, "..")){
          // file information string
          auto hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
          auto props = gtk_label_new("");
          char *markup = g_strconcat("<span size=\"small\" color=\"blue\">", "foo bar", "</span>", NULL);
          gtk_label_set_markup(GTK_LABEL(props), markup);
          g_free(markup);
          UtilBasic::boxPack0(GTK_BOX(hbox), props, FALSE, FALSE, 0);    
          UtilBasic::boxPack0(GTK_BOX(box), GTK_WIDGET(hbox), FALSE, FALSE, 0);    
        }
        g_free(name);
        addMotionController(labelBox);
        addMotionController(imageBox);
        addGestureClick(imageBox, object);

/*
        char *markup = g_strconcat("<span size=\"small\">", name, "</span>", NULL);
        gtk_label_set_markup( GTK_LABEL( label ), name );
//        gtk_label_set_markup( GTK_LABEL( label ), markup );
        g_free(name);
        g_free(markup);*/
        
        // HACK
   /*     auto owner = gtk_widget_get_parent(GTK_WIDGET(vbox));
        graphene_rect_t bounds;
        if (!gtk_widget_compute_bounds(owner, MainWidget, &bounds)){
          DBG("Error: gtk_widget_compute_bounds\n");
        } else {
          DBG("owner=%p, width=%lf height=%lf\n", owner, bounds.size.width, bounds.size.height);
        }
        gtk_widget_set_size_request(owner, 24,24);
        gtk_widget_set_vexpand(GTK_WIDGET(owner), FALSE);
        gtk_widget_set_hexpand(GTK_WIDGET(owner), FALSE);
        gtk_widget_compute_bounds(owner, owner, &bounds);
          DBG("owner=%p, width=%lf height=%lf\n", owner, bounds.size.width, bounds.size.height);
*/
        

      }


      static GtkMultiSelection *getSelectionModel(GListModel *store){
        GtkFilter *filter = 
          GTK_FILTER(gtk_custom_filter_new ((GtkCustomFilterFunc)filterFunction, NULL, NULL));
        GtkFilterListModel *filterModel = gtk_filter_list_model_new(G_LIST_MODEL(store), filter);
        // Chain link GtkFilterListModel to a GtkSortListModel.
        // Directories first, and alphabeta.
        GtkSorter *sorter = 
          GTK_SORTER(gtk_custom_sorter_new((GCompareDataFunc)compareFunction, NULL, NULL));
        GtkSortListModel *sortModel = gtk_sort_list_model_new(G_LIST_MODEL(filterModel), sorter);
        GtkMultiSelection *s = gtk_multi_selection_new(G_LIST_MODEL(sortModel));
        g_signal_connect(G_OBJECT(s), "selection-changed", G_CALLBACK(selection_changed), NULL);
        return s;
      }

      static void selection_changed ( GtkSelectionModel* self,
            guint position,
            guint n_items,
            void *data) {
        DBG("selection changed position=%d, items=%d\n", position, n_items);
        if (gtk_selection_model_is_selected(self, 0)){
          gtk_selection_model_unselect_item(self, 0);
        }
        return;
      }

      
      static GtkMultiSelection *standardSelectionModel(const char *path){
        // This does not have the up icon 
        auto gfile = g_file_new_for_path(path);
        GtkDirectoryList *dList = 
//          gtk_directory_list_new("", gfile); 
          gtk_directory_list_new("standard::", gfile); 
        return getSelectionModel(G_LIST_MODEL(dList));

      }

      static GtkMultiSelection *xfSelectionModel(const char *path){
       // This section adds the up icon.

        auto up = g_path_get_dirname(path);
        TRACE("path=%s up=%s\n", path, up);
        auto store = g_list_store_new(G_TYPE_FILE_INFO);
        auto upFile = g_file_new_for_path(up);
        GError *error_ = NULL;
        auto info = g_file_query_info(upFile, "standard::", G_FILE_QUERY_INFO_NONE, NULL, &error_);
        if (strcmp(path, up)==0) {
          g_file_info_set_attribute_object(info, "xffm::root", G_OBJECT(upFile));
        }
        g_free(up);
        g_file_info_set_name(info, "..");
        g_file_info_set_icon(info, g_themed_icon_new("up"));
        g_list_store_insert(store, 0, G_OBJECT(info));
        g_file_info_set_attribute_object(info, "standard::file", G_OBJECT(upFile));

        GFile *file = g_file_new_for_path(path);
        GFileEnumerator *dirEnum = 
          g_file_enumerate_children (file,"standard::",G_FILE_QUERY_INFO_NONE,NULL, &error_);
        if (error_) {
          DBG("*** Error::g_file_enumerate_children: %s\n", error_->message);
          return NULL;
        }
        GFile *outChild = NULL;
        GFileInfo *outInfo = NULL;
        int k = 1;
        do {
          g_file_enumerator_iterate (dirEnum, &outInfo, &outChild,
              NULL, // GCancellable* cancellable,
              &error_);
          if (error_) {
            DBG("*** Error::g_file_enumerator_iterate: %s\n", error_->message);
            return NULL;
          }
          if (!outInfo || !outChild) break;
          g_file_info_set_attribute_object(outInfo, "standard::file", G_OBJECT(outChild));
          TRACE("insert path (%s)\n", g_file_get_path(outChild));
          g_list_store_insert(store, k++, G_OBJECT(outInfo));
        } while (true);
        return getSelectionModel(G_LIST_MODEL(store));
      }

      static GtkMultiSelection *rootSelectionModel(void){
        GError *error_ = NULL;
        Bookmarks::initBookmarks();
        auto store = g_list_store_new(G_TYPE_FILE_INFO);
        auto list = Bookmarks::bookmarksList();
        for (auto l=list; l && l->data; l=l->next){
          auto p = (bookmarkItem_t *)l->data;
          if (!p->path) continue;
          DBG("adding bookmark %p -> %s\n", p, p->path);
          if (!g_path_is_absolute(p->path)) continue;
          if (!g_file_test(p->path, G_FILE_TEST_EXISTS)) {
              DBG("Bookmark %s does not exist\n", p->path);
              continue;
          }
          GFile *file = g_file_new_for_path(p->path);
          auto info = g_file_query_info(file, "standard::", G_FILE_QUERY_INFO_NONE, NULL, &error_);
          auto basename = g_path_get_basename(p->path);
          auto utf_name = UtilBasic::utf_string(basename);
          g_file_info_set_name(info, utf_name);
          g_free(basename);
          g_free(utf_name);
          g_file_info_set_icon(info, g_themed_icon_new(EMBLEM_BOOKMARK));
          g_list_store_insert(store, 0, G_OBJECT(info));
          g_file_info_set_attribute_object(info, "standard::file", G_OBJECT(file));          
        }
        return getSelectionModel(G_LIST_MODEL(store));
      }

      static GtkWidget *
      getGridView(const char *path){
        GtkMultiSelection *selection_model = NULL;
        if (strcmp(path, "xffm:root")==0) {
          selection_model = rootSelectionModel();
        } else {
          // Create the initial GtkDirectoryList (G_LIST_MODEL).
          selection_model = xfSelectionModel(path);
          //selection_model = standardSelectionModel(path);     
        }
       
        // GtkListItemFactory implements GtkSignalListItemFactory, which can be connected to
        // bind, setup, teardown and unbind
        GtkListItemFactory *factory = gtk_signal_list_item_factory_new();

        /* Connect handler to the factory.
         */
        g_signal_connect( factory, "setup", G_CALLBACK(factorySetup), NULL );
        g_signal_connect( factory, "bind", G_CALLBACK(factoryBind), NULL);

        GtkWidget *view;
        /* Create the view.
         */
        view = gtk_grid_view_new(GTK_SELECTION_MODEL(selection_model), factory);
        gtk_grid_view_set_max_columns(GTK_GRID_VIEW(view), 20);
        //gtk_grid_view_set_min_columns(GTK_GRID_VIEW(view), 10);
        gtk_widget_add_css_class(view, "gridviewColors");
        gtk_grid_view_set_enable_rubberband(GTK_GRID_VIEW(view), TRUE);
        return view;
      }
    private:
      static gboolean
      filterFunction(GObject *object, void *data){
        GFileInfo *info = G_FILE_INFO(object);
        return TRUE;
        if (strcmp(g_file_info_get_name(info), "..")==0) return TRUE;
        return !g_file_info_get_is_hidden(info);
      }
      static
      void addMotionController(GtkWidget  *widget){
        auto controller = gtk_event_controller_motion_new();
        gtk_event_controller_set_propagation_phase(controller, GTK_PHASE_CAPTURE);
        gtk_widget_add_controller(GTK_WIDGET(widget), controller);
        g_signal_connect (G_OBJECT (controller), "enter", 
            G_CALLBACK (negative), NULL);
        g_signal_connect (G_OBJECT (controller), "leave", 
            G_CALLBACK (positive), NULL);
    }
     // FIXME: if gridview Settings color for background is
      //        too close to #acaaa5, use a different css class color
      //
      //        Also: add highlight color for text box and change 
      //              text from label to entry, to allow inline
      //              renaming on longpress.
    static gboolean
    negative ( GtkEventControllerMotion* self,
                    gdouble x,
                    gdouble y,
                    gpointer data) 
    {
        auto eventBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
        gtk_widget_add_css_class (GTK_WIDGET(eventBox), "pathbarboxNegative" );
        UtilBasic::flushGTK();
        return FALSE;
    }
    static gboolean
    positive ( GtkEventControllerMotion* self,
                    gdouble x,
                    gdouble y,
                    gpointer data) 
    {
        auto eventBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
        gtk_widget_remove_css_class (GTK_WIDGET(eventBox), "pathbarboxNegative" );
        UtilBasic::flushGTK();
        return FALSE;
    }

    

      static bool
      symlinkToDir(GFileInfo* info, GFileType type){
        if (type == G_FILE_TYPE_SYMBOLIC_LINK){ 
          const char *path = g_file_info_get_symlink_target(info);
          struct stat st;
          stat(path, &st);
          if (S_ISDIR(st.st_mode)) return true;
        }
        return false;
      }

    // flags :
    // 0x01 : by date
    // 0x02 : by size
    // 0x04 : descending
    static gint 
    compareFunction(const void *a, const void *b, void *data){
        auto flags = GPOINTER_TO_INT(data);
        bool byDate = (flags & 0x01);
        bool bySize = (flags & 0x02);
        bool descending = (flags & 0x04);

        GFileInfo *infoA = G_FILE_INFO(a);
        GFileInfo *infoB = G_FILE_INFO(b);
        auto typeA = g_file_info_get_file_type(infoA);
        auto typeB = g_file_info_get_file_type(infoB);

        if (strcmp(g_file_info_get_name(infoA), "..")==0) return -1;
        if (strcmp(g_file_info_get_name(infoB), "..")==0) return 1;
        
        GFile *fileA = G_FILE(g_file_info_get_attribute_object(infoA, "standard::file"));
        GFile *fileB = G_FILE(g_file_info_get_attribute_object(infoB, "standard::file"));

        // compare by name, directories or symlinks to directories on top
        TRACE("compare %s --- %s\n", g_file_info_get_name(infoA), g_file_info_get_name(infoB));
        //XXX ".." is not a part of the dList...
        //if (strcmp(xd_a->d_name, "..")==0) return -1;
        //if (strcmp(xd_b->d_name, "..")==0) return 1;

        gboolean a_cond = FALSE;
        gboolean b_cond = FALSE;

        a_cond = ((typeA == G_FILE_TYPE_DIRECTORY )||(symlinkToDir(infoA, typeA)));
        b_cond = ((typeB == G_FILE_TYPE_DIRECTORY )||(symlinkToDir(infoB, typeB)));

        if (a_cond && !b_cond) return -1; 
        if (!a_cond && b_cond) return 1;

        auto nameA = g_file_info_get_name(infoA);
        auto nameB = g_file_info_get_name(infoB);
        if (a_cond && b_cond) {
            // directory comparison by name is default;
           if (byDate) {
              auto dateTimeA = g_file_info_get_modification_date_time(infoA);
              auto dateTimeB = g_file_info_get_modification_date_time(infoB);
              auto value = g_date_time_compare(dateTimeA, dateTimeB);
              g_free(dateTimeA);
              g_free(dateTimeB);
              return value;
           } else {
                if (descending) return -strcasecmp(nameA, nameB);
                return strcasecmp(nameA, nameB);
            }
        }
        // by date
        if (byDate){
          auto dateTimeA = g_file_info_get_modification_date_time(infoA);
          auto dateTimeB = g_file_info_get_modification_date_time(infoB);
          auto value = g_date_time_compare(dateTimeA, dateTimeB);
          g_free(dateTimeA);
          g_free(dateTimeB);
          if (descending) return -value;
          return value;
        } else if (bySize){
          auto sizeA = g_file_info_get_size(infoA);
          auto sizeB = g_file_info_get_size(infoB);
          if (descending) return sizeB - sizeA;
          return sizeA - sizeB;
        } 
        // by name 
        if (descending) return -strcasecmp(nameA, nameB);
        return strcasecmp(nameA, nameB);
    }
    
    static void
    addDirectoryTooltip(GtkWidget *image, GFileInfo *info){
      auto name = g_file_info_get_name(info);
      
      auto file = G_FILE(g_file_info_get_attribute_object(info, "standard::file"));
      GError *error_ = NULL;
      GFileEnumerator *dirEnum = 
          g_file_enumerate_children (file,"standard::name,standard::type",G_FILE_QUERY_INFO_NONE,NULL, &error_);
      if (error_){
        gtk_widget_set_tooltip_markup(image, error_->message);
        g_error_free(error_);
      } else {
        int limit = 20;
        int k=0;
        char *t = g_strdup_printf(_("Contents of '%s'"), name);
//            char *t = g_strdup_printf("%s:",_("contents"));
        UtilBasic::concat(&t, ":\n");
        do {
          GFile *outChild = NULL;
          GFileInfo *outInfo = NULL;
          g_file_enumerator_iterate (dirEnum, &outInfo, &outChild,
              NULL, // GCancellable* cancellable,
              &error_);
          if (error_) {
            DBG("*** Error::g_file_enumerator_iterate: %s\n", error_->message);
            g_error_free(error_);
          }
          if (!outInfo || !outChild) break;
          UtilBasic::concat(&t, "<span color=\"blue\" size=\"x-small\">  ");
          UtilBasic::concat(&t, g_file_info_get_name(outInfo));
          auto subtype = g_file_info_get_file_type(outInfo);
          if (subtype == G_FILE_TYPE_DIRECTORY )UtilBasic::concat(&t, "/");
          if (subtype == G_FILE_TYPE_SYMBOLIC_LINK)UtilBasic::concat(&t, "*");
          UtilBasic::concat(&t, "</span>\n");
          if (++k >= limit) {
            UtilBasic::concat(&t, "<span color=\"red\">");
            UtilBasic::concat(&t, _("More..."));
            UtilBasic::concat(&t, "</span>\n");
            break;
          }  
        } while (true);
        g_file_enumerator_close(dirEnum, NULL, NULL);
        gtk_widget_set_tooltip_markup(image, t);
        g_free(t);
      }
    }


  };
}
#endif
