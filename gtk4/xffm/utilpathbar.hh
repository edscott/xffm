#ifndef UTILPATHBAR_HH
#define UTILPATHBAR_HH

namespace xf {
  class UtilPathbar  :  public  UtilBasic{
    public:
    static void 
    updatePathbar(const gchar *path, GtkBox *pathbar){
        DBG( "update pathbar to %s\n", path);
        void *arg[]={(void *)(path?g_strdup(path):NULL), (void *)pathbar};
        context_function(update_pathbar_f, arg);
    }
    static GList *getChildren(GtkBox *box){
      GList *list = NULL;
      GtkWidget *w = gtk_widget_get_first_child(GTK_WIDGET(box));
      if (w) list = g_list_append(list, w);
      while ((w=gtk_widget_get_next_sibling(w)) != NULL) list = g_list_append(list, w);
      return list;
    }
    
    static void *
    update_pathbar_f(void *data){
        void **arg = (void **)data;
        ///Pathbar *pathbar_p = (Pathbar *)arg[0];
        auto path = (gchar *)arg[0];
        auto pathbar = GTK_BOX(arg[1]);
        DBG( "update_pathbar_f:: %s\n", path);

        if (!pathbar) return NULL;
        if (!path){
            DBG("##### togglePathbar(NULL, pathbar)\n");
            togglePathbar(NULL, pathbar);
//            pathbar_p->toggle_pathbar(NULL);
            return NULL;
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

        GList *children_list = getChildren(pathbar);
        //GList *children_list = gtk_container_get_children(GTK_CONTAINER(pathbar));
        gint i=0;
        gchar *pb_path = NULL;
        for (GList *children = children_list;children && children->data; children=children->next){
            gchar *name = (gchar *)g_object_get_data(G_OBJECT(children->data), "name");
            if (strcmp(name, "RFM_ROOT")==0 || strcmp(name, "RFM_GOTO")==0) continue;
            //gchar *p = g_strdup_printf("%s%c", paths[i], G_DIR_SEPARATOR);
            DBG( "(%d) comparing %s <--> %s\n", i, name, paths[i]);
            if (paths[i] && strcmp(name, paths[i]) == 0){
                g_free(pb_path);
                const gchar *p = (const gchar *)g_object_get_data(G_OBJECT(children->data), "path");
                pb_path = g_strdup(p);
                i++; 
                continue;
            }
            // Eliminate tail (only if tail will differ)
            if (paths[i] == NULL) break;
            DBG( "Zapping tail: \"%s\"\n", paths[i]);
            GList *tail = children;
            for (;tail && tail->data; tail = tail->next){
                gchar *name  = (gchar *)g_object_get_data(G_OBJECT(tail->data), "name");
                DBG( "Zapping tail item: \"%s\"\n", name);
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
                pathbarLabelButton(strlen(paths[i])?paths[i]:G_DIR_SEPARATOR_S);

            boxPack0 (pathbar, GTK_WIDGET(pb_button), FALSE, FALSE, 0);
            //gtk_container_add(GTK_CONTAINER(pathbar), GTK_WIDGET(pb_button));

            gchar *g = (pb_path!=NULL)?
                g_strdup_printf("%s%s%s",pb_path, 
                        strcmp(pb_path,G_DIR_SEPARATOR_S)? 
                        G_DIR_SEPARATOR_S:"", paths[i]):
                g_strdup(paths[i]);
            g_free(pb_path);
            pb_path = g;
            DBG( "+++***** setting pbpath --> %s\n", pb_path);
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
        return NULL;
    }
    static gboolean
    pathbar_white ( GtkEventControllerMotion* self,
                    gdouble x,
                    gdouble y,
                    gpointer data) 
    {
        auto pathbar = GTK_BOX(data);
        auto eventBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
        auto path = (const char *) g_object_get_data(G_OBJECT(pathbar), "path");

        UtilPathbar::setPathButtonText(eventBox, path, "white", "#acaaa5");
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
    static gboolean
    pathbarGo (
              GtkEventControllerLegacy* self,
              GdkEvent* event,
              gpointer data ) 
    {
        auto eventBox = GTK_BOX(data);
        auto type = gdk_event_get_event_type(event);
        if (type != GDK_BUTTON_RELEASE) return FALSE;
        DBG("button release...\n");

        return TRUE;
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
        if (button == 1){
          DBG("pathbar goto...\n");
          return TRUE;
        }
        if (button == 3){
          DBG("pathbar menu...\n");
          return TRUE;
        }
        //DBG("pathbar_go...name=%s, path=%s button=%d\n", name, path, button);
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

    static void 
    togglePathbar(const gchar *path, GtkBox *pathbar){
        // Hiding stuff which does not fit does not work until
        // window has been shown. This is not yet the case on
        // initial startup, so we skip that on first pass.
        DBG("*** togglePathbar\n");
        GList *children_list = getChildren(pathbar);

        if (gtk_widget_get_realized(MainWidget)) showWhatFits(pathbar, path, children_list);
        else {DBG("MainWidget not yet realized...\n");}

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
    static GtkBox *
    pathbarLabelButton (const char *text) {
        auto label = GTK_LABEL(gtk_label_new(""));
        auto eventBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        if (text) {
            auto v = utf_string(text);
            auto g = g_markup_escape_text(v, -1);
            g_free(v);
            auto markup = g_strdup_printf("   <span size=\"small\">  %s  </span>   ", g);
            g_free(g);
            gtk_label_set_markup(label, markup);
            g_free(markup);
        } else {
            gtk_label_set_markup(label, "");
        }
        boxPack0 (eventBox, GTK_WIDGET(label), FALSE, FALSE, 0);
        g_object_set_data(G_OBJECT(eventBox), "label", label);
        g_object_set_data(G_OBJECT(eventBox), "name", text?g_strdup(text):g_strdup("RFM_ROOT"));
        return eventBox;
    }
  private:
    static void         
    showWhatFits(GtkBox *pathbar, const gchar *path, GList *children_list){
      GtkRequisition minimum;

      graphene_rect_t bounds;
      if (!gtk_widget_compute_bounds(GTK_WIDGET(pathbar), GTK_WIDGET(pathbar), &bounds)) {
        DBG("***Error:: showWhatFits():gtk_widget_compute_bounds()\n");
      }
      auto size = &(bounds.size);
      auto width = size->width;

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
            gtk_widget_set_visible(GTK_WIDGET(children->data), FALSE);
        }

        // Find first item to place in pathbar.
        // This item *must* be equal to path, if path is in buttons.

        children = g_list_last(children_list);
        GList *active = children;
        // If path is not in the buttons, then the first to map
        // will be the last path visited.
        if (path) for (;children && children->data; children=children->prev){
            auto pb_path = (const gchar *)
                g_object_get_data(G_OBJECT(children->data), "path");
            if (!pb_path) continue;
            if (strcmp(path, pb_path)==0) {
                active = children;
                break;
            }
        }
         // Show active button
        gtk_widget_set_visible(GTK_WIDGET(active->data), TRUE);

        gtk_widget_get_preferred_size(GTK_WIDGET(active->data), &minimum, NULL);
            DBG("#### width, minimum.width %d %d\n",width,  minimum.width);
        width -= minimum.width;
     
        // Work backwards from active button we show buttons that will fit.
        children = active->prev;
        for (;children && children->data; children=children->prev){
            gchar *name = (gchar *)g_object_get_data(G_OBJECT(children->data), "name");
            if (strcmp(name, "RFM_ROOT")==0) continue;

            if (!gtk_widget_compute_bounds(GTK_WIDGET(children->data), GTK_WIDGET(children->data), &bounds)) {
              DBG("***Error:: showWhatFits():gtk_widget_compute_bounds()\n");
            }

            DBG("#### width, allocaltion.width %f %f\n",width,  bounds.size.width);
            width -= bounds.size.width;
            if (width < 0) break;
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
            gchar *v = utf_string(name);
            gchar *g = g_markup_escape_text(v, -1);
            g_free(v);
            gchar *markup = g_strdup_printf("<span %s color=\"%s\" bgcolor=\"%s\">  %s  </span>", fontSize, bgcolor?"white":"red", bgcolor?bgcolor:"#dcdad5", g);
            auto label = GTK_LABEL(g_object_get_data(G_OBJECT(eventBox), "label"));
            gtk_label_set_markup(label, markup);

            g_free(g);
            g_free(markup);
        }
        else {
            gchar *v = utf_string(name);
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

