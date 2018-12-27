#ifndef XF_ICONVIEW__HH
# define XF_ICONVIEW__HH
namespace xf
{

template <class Type>
class IconView {

public: 
    static GtkIconView *createIconview(View<Type> *view){
        auto iconView = GTK_ICON_VIEW(gtk_icon_view_new());
        g_object_set(G_OBJECT(iconView), "has-tooltip", TRUE, NULL);
        gtk_icon_view_set_item_width (iconView, 60);
        gtk_icon_view_set_activate_on_single_click(iconView, TRUE);

 	
	g_object_set_data(G_OBJECT(view->treeModel()), "iconview", iconView); //do we really need this?
	gtk_icon_view_set_model(iconView, view->treeModel());

	gtk_icon_view_set_text_column (iconView, DISPLAY_NAME);
	gtk_icon_view_set_pixbuf_column (iconView,  DISPLAY_PIXBUF);
	gtk_icon_view_set_selection_mode (iconView, GTK_SELECTION_SINGLE);
        setUpSignals(view, G_OBJECT(iconView));
       
        return iconView;
    }

private:
    static void
    setUpSignals(View<Type> *view, GObject * iconView){
        g_signal_connect (iconView, "item-activated", 
            ICONVIEW_CALLBACK (BaseSignals<Type>::activate), 
            (void *)view);
        g_signal_connect (iconView, "motion-notify-event", 
            ICONVIEW_CALLBACK (IconView<Type>::motionNotifyEvent), 
            (void *)view);
         //g_signal_connect (iconView, "query-tooltip", 
           //     G_CALLBACK (query_tooltip_f), 
           //     (void *)view);
 

         g_signal_connect (iconView, "button-release-event",
             G_CALLBACK(IconView<Type>::buttonRelease), 
             (void *)view);
         g_signal_connect (iconView, "button-press-event",
             G_CALLBACK(IconView<Type>::buttonPress), 
             (void *)view);
         // Why not "clicked" signal? 
         // Because this is to filter cancelled dnd event from
         // actual activated events.
         g_signal_connect (iconView, "button-release-event",
             G_CALLBACK(IconView<Type>::buttonClick), 
             (void *)view);
      

    }

    ////////////////////   signal handlers iconview specific  ///////////////////

    static gboolean
    motionNotifyEvent (GtkWidget *widget,
                   GdkEvent  *ev,
                   gpointer   data)
    {
        auto e = (GdkEventMotion *)ev;
        auto event = (GdkEventButton  *)ev;
	auto view = (View<Type> *)data;
        if (!data) {
            DBG("View::motion_notify_event: data cannot be NULL\n");
            return FALSE;
        }
	TRACE("motion_notify_event, dragmode= %d\n", view->dragMode());

        if (buttonPressX >= 0 && buttonPressY >= 0){
	    TRACE("buttonPressX >= 0 && buttonPressY >= 0\n");
	    if (sqrt(pow(e->x - buttonPressX,2) + pow(e->y - buttonPressY, 2)) > 10){
                view->selectables();
                GList *selection_list = gtk_icon_view_get_selected_items (view->iconView());
                view->setSelectionList(selection_list);
                if (selection_list==NULL) {
                    return FALSE;
                }
	        // start DnD (multiple selection)
		WARN("dragOn_ = TRUE\n");
		dragOn_ = TRUE;
		// in control mode, reselect item at x,y
		if(controlMode) {
		    GtkTreePath * tpath;
		    if (gtk_icon_view_get_item_at_pos (GTK_ICON_VIEW(widget),
                                   buttonPressX, buttonPressY,
                                   &tpath, NULL)) {
		    gtk_icon_view_select_path (view->iconView(), tpath);
		    gtk_tree_path_free(tpath);
		    }

		}

                if (!targets) targets= gtk_target_list_new (targetTable,TARGETS);

		context =
		    gtk_drag_begin_with_coordinates (view->source(),
			     targets,
			     (GdkDragAction)(((gint)GDK_ACTION_MOVE)|
                   ((gint)GDK_ACTION_COPY)|
                   ((gint)GDK_ACTION_LINK)), //GdkDragAction actions,
			     1, //gint button,
			     (GdkEvent *)event, //GdkEvent *event,
			     event->x, event->y);
                             
		buttonPressX = buttonPressY = -1;
                //g_object_ref(G_OBJECT(context)); 
	    }
        }

	// XXX: Why this limitation?
        // if (view_p->get_dir_count() > 500) return FALSE;
        view->highlight(e->x, e->y);

    
        return FALSE;
    }

    static gboolean
    buttonRelease (GtkWidget *widget,
                   GdkEventButton  *event,
                   gpointer   data)
    {
        //GdkEventButton *event_button = (GdkEventButton *)event;
	auto view = (View<Type> *)data;
        if (!dragOn_){
	    GtkTreePath *tpath;

	    if (rubberBand_) {
                view->selectables();
		return FALSE;
	    }
	    if (!gtk_icon_view_get_item_at_pos (view->iconView(),
                                   event->x, event->y,
                                   &tpath,NULL)){
		WARN("button down cancelled.\n");
		return TRUE;
	    }

	    //Cancel DnD prequel.
	    buttonPressX = buttonPressY = -1;
	    dragOn_ = FALSE;
	    if (dragMode) { // copy/move/link mode
		return TRUE;
	    }
	    // default mode:

	    // unselect everything
	    gtk_icon_view_unselect_all (view->iconView());
	    // reselect item to activate
	    gtk_icon_view_select_path (view->iconView(),tpath);
	    WARN("Here we do a call to activate item.\n");
	    BaseSignals<Type>::activate(tpath, data);
	    gtk_tree_path_free(tpath);
	    return TRUE;
        }

	buttonPressX = buttonPressY = -1;
        dragOn_ = FALSE;
	
        dragMode = 0;
        return FALSE;
    }

    static gboolean
    buttonPress (GtkWidget *widget,
                   GdkEventButton  *event,
                   gpointer   data)
    {
        auto view = (View<Type> *)data;
        buttonPressX = buttonPressY = -1;
        dragOn_ = FALSE;

        GtkTreePath *tpath;
        if (event->button == 1) {
	    controlMode = FALSE;
            gboolean retval = FALSE;
            gint mode = 0;
            if (gtk_icon_view_get_item_at_pos (view->iconView(),
                                   event->x, event->y,
                                   &tpath,NULL)) {
                
		DBG("button press %d mode %d\n", event->button, mode);
		buttonPressX = event->x;
		buttonPressY = event->y;
		rubberBand_ = FALSE;
                if (CONTROL_MODE && SHIFT_MODE) {
		    dragMode = -3; // link mode
		} else if (CONTROL_MODE) {
		    controlMode = TRUE;
		    dragMode = -2; // copy
		    // select item and add to selection list
		    if (gtk_icon_view_path_is_selected (view->iconView(), tpath)) {
			// if selected
			gtk_icon_view_unselect_path (view->iconView(), tpath);
		    } else { // not selected
			//if (view->isSelectable(tpath)) 
                        gtk_icon_view_select_path (view->iconView(), tpath);
                        view->selectables();
		    }
		} else if (SHIFT_MODE) {
		    dragMode = -1; // move
                    viewShiftSelect(view, tpath);
		} else {
		    // unselect all
		    gtk_icon_view_unselect_all (view->iconView());
		    // select single item
		    gtk_icon_view_select_path (view->iconView(), tpath);
		    dragMode = 0; // default (move)
		}
                retval = TRUE; 
		gtk_tree_path_free(tpath);
		tpath = NULL; // just in case.
            } else { 
                tpath=NULL;
		dragMode = 0;
		rubberBand_ = TRUE;
            }

            return retval;
        }

        // long press or button 3 should do popup menu...
        if (event->button != 3) return FALSE;
        return viewPopUp(view, event);
    }

    static gboolean
    buttonClick (GtkWidget *widget,
                   GdkEventButton  *event,
                   gpointer   data)
    {
	//auto view = (View<Type> *)data;
	WARN("no action on button_click_f\n");
        return FALSE;
    }
    
/////////////////////////////////  DnD   ///////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////

    
    static void
    viewShiftSelect(View<Type> *view, GtkTreePath *tpath){
        // select all items in interval
        gtk_icon_view_select_path (view->iconView(), tpath);
        auto items = gtk_icon_view_get_selected_items (view->iconView());
        view->setSelectionList(items);

        gchar *item = gtk_tree_path_to_string (tpath);
        gchar *startItem;
        if (strrchr(item, ':')) startItem = g_strdup(strrchr(startItem, ':')+1);
        else startItem = g_strdup(item);
        g_free(item);

        WARN("Starting from %s\n", startItem);

        void *startPath;

        for (GList *l=items; l && l->data; l=l->next){
            gchar *item = gtk_tree_path_to_string ((GtkTreePath *)l->data);
            if (strcmp(item, startItem)==0){
                startPath = l->data;
                g_free(item);
                break;
            }
            g_free(item);
        }

        GList *s = g_list_find(items, startPath);
        gint start, end;

        if (s->prev == NULL) {
            gchar *lastitem = gtk_tree_path_to_string ((GtkTreePath *)g_list_last(items)->data);
            end = atoi(startItem);
            start = atoi(lastitem);
            WARN("lastitem %s\n", lastitem);
            g_free(lastitem);

        } else {
            gchar *firstitem = gtk_tree_path_to_string ((GtkTreePath *)g_list_first(items)->data);
            end = atoi(firstitem);
            start = atoi(startItem);
            WARN("firstitem %s\n", firstitem);
            g_free(firstitem);
        }
        g_free(startItem);
        // To free the return value, use:
        g_list_free_full (items, (GDestroyNotify) gtk_tree_path_free);
        WARN("loop %d -> %d\n", start, end);
        gtk_icon_view_unselect_all (view->iconView());
        GtkTreePath *tp;
        for (int i=start; i<=end; i++){
                gchar *item = g_strdup_printf("%0d", i);
                WARN("selecting %s(%d)\n", item, i);
                tp = gtk_tree_path_new_from_string(item);
                g_free(item);
                //if (view->isSelectable(tpath)) 
                    
                gtk_icon_view_select_path (view->iconView(), tp);
            gtk_tree_path_free(tp);
        }
        view->selectables();
    }

    static gboolean 
    viewPopUp(View<Type> *view, GdkEventButton  *event){
        TRACE("button press event: button 3 should do popup, as well as longpress...\n");
        GtkTreePath *tpath;
        gboolean retval = FALSE;
        GtkMenu *menu = NULL;
        if (gtk_icon_view_get_item_at_pos (view->iconView(),
                                   event->x,
                                   event->y,
                                   &tpath, NULL))
        if (CONTROL_MODE) {
           gtk_icon_view_unselect_all (view->iconView());
        } else {
            gtk_icon_view_select_path (view->iconView(), tpath);
	}
        gtk_tree_path_free(tpath);
           
       
        gchar *path = NULL;
        GList *selection_list = gtk_icon_view_get_selected_items (view->iconView());
        view->setSelectionList(selection_list);
        if (selection_list && g_list_length(selection_list) == 1) {
            GtkTreeIter iter;
            gtk_tree_model_get_iter(view->treeModel(), &iter, 
                    (GtkTreePath *)selection_list->data);
            gtk_tree_model_get(view->treeModel(), &iter, PATH, &path, -1);
        }
	if ((CONTROL_MODE) || g_list_length(selection_list) == 0) {
	    setMenuData(view, path, FALSE);
	    menu = configureMenu(view, FALSE);
	} else {
	    setMenuData(view, path,TRUE);
	    menu = configureMenu(view,TRUE);
	} 
        if (menu) {
            gtk_menu_popup_at_pointer (menu, (const GdkEvent *)event);
        }   
        return retval;
    }
public:
    static void
    setMenuData(View<Type> * view, gchar *path, gboolean  items){
        GtkMenu *menu = NULL;
        switch (view->viewType()){
            case (ROOTVIEW_TYPE):
                 menu = (items)?
                    RootView<Type>::popUpItem():
                    RootView<Type>::popUp();
                break;
            case (LOCALVIEW_TYPE):
                menu = (items)?
                    LocalView<Type>::popUpItem():
                    LocalView<Type>::popUp();
                break;
            case (FSTAB_TYPE):
                 menu = (items)?
                    FstabView<Type>::popUpItem():
                    FstabView<Type>::popUp();
                break;
            default:
                ERROR("ViewType %d not defined.\n", view->viewType());
                break;
        }
        if (menu) {
           auto oldPath = (gchar *)g_object_get_data(G_OBJECT(menu),"path");
            g_free(oldPath);
            DBG("*** set menu data path=%s\n", path);
            g_object_set_data(G_OBJECT(menu),"path", path);
            g_object_set_data(G_OBJECT(menu),"view", (void *)view);
        }
    }
    
    static GtkMenu *
    configureMenu(View<Type> * view, gboolean items){
        GtkMenu *menu = NULL;
        switch (view->viewType()){
            case (ROOTVIEW_TYPE):
                if (items) {
                    menu = rootItemPopUp;
                    RootView<Type>::resetMenuItems();
                } else {
                    menu = rootPopUp;
                    RootView<Type>::resetPopup();
                }
                break;
            case (LOCALVIEW_TYPE):
                if (items) {
                    menu = localItemPopUp;
                    LocalView<Type>::resetMenuItems();
                } else {
                    menu = localPopUp;
                    LocalView<Type>::resetLocalPopup();
                }
                break;
            case (FSTAB_TYPE):
                if (items) {
                    menu = fstabItemPopUp;
                    FstabView<Type>::resetMenuItems();
                } else {
                    menu = fstabPopUp;
                    FstabView<Type>::resetPopup();
                }
                break;
            default:
                ERROR("ViewType %d not defined.\n", view->viewType());
                break;
        }
        return menu;
    }


    
};
}
#endif
