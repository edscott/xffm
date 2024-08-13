#ifndef UTILPATHBAR_HH
#define UTILPATHBAR_HH

namespace xf {
  class UtilPathbar  :  private UtilBasic{
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
            // FIXME:
    /*        g_signal_connect (G_OBJECT(pb_button) , "button-press-event", EVENT_CALLBACK (pathbar_go), (void *)pathbar_p);
            g_signal_connect (G_OBJECT(pb_button) , "enter-notify-event", EVENT_CALLBACK (pathbar_white), (void *)pathbar_p);
            g_signal_connect (G_OBJECT(pb_button) , "leave-notify-event", EVENT_CALLBACK (pathbar_blue), (void *)pathbar_p);*/


            gtk_widget_set_visible(GTK_WIDGET(pb_button), TRUE);
        }
        g_free(pb_path);
        g_strfreev(paths);
        
        // show what fits
        togglePathbar(path, pathbar);
        g_free(path);
        return NULL;
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
      DBG("showWhatFits: pathbar width = %f\n", size->width);
        GtkAllocation allocation;
        gtk_widget_get_allocation(MainWidget, &allocation);
        DBG("pathbar width=%d\n", allocation.width);
        gint width = allocation.width;
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
            //gtk_widget_hide(GTK_WIDGET(children->data));
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
        //gtk_widget_show_all(GTK_WIDGET(active->data));

        gtk_widget_get_preferred_size(GTK_WIDGET(active->data), &minimum, NULL);
            DBG("#### width, minimum.width %d %d\n",width,  minimum.width);
        width -= minimum.width;
     
        // Work backwards from active button we show buttons that will fit.
        children = active->prev;
        for (;children && children->data; children=children->prev){
            gchar *name = (gchar *)g_object_get_data(G_OBJECT(children->data), "name");
            if (strcmp(name, "RFM_ROOT")==0) continue;
            gtk_widget_get_allocation(GTK_WIDGET(children->data), &allocation);
            DBG("#### width, allocaltion.width %d %d\n",width,  allocation.width);
            width -= allocation.width;
            if (width < 0) break;
            gtk_widget_set_visible(GTK_WIDGET(children->data), TRUE);
            //gtk_widget_show_all(GTK_WIDGET(children->data));
        }

        // Now we work forwards, showing buttons that fit.
        children = active->next;
        for (;children && children->data; children=children->next){
           gchar *name = (gchar *)g_object_get_data(G_OBJECT(children->data), "name");
            if (strcmp(name, "RFM_ROOT")==0) continue;
            gtk_widget_get_allocation(GTK_WIDGET(children->data), &allocation);
            width -= allocation.width;
            if (width < 0) break;
            gtk_widget_set_visible(GTK_WIDGET(children->data), TRUE);
            //gtk_widget_show_all(GTK_WIDGET(children->data));
        }
    }

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

