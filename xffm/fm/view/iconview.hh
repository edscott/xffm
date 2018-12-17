#ifndef XF_ICONVIEW__HH
# define XF_ICONVIEW__HH
namespace xf
{

template <class Type>
class IconView {

public: 
    static GtkIconView *createIconview(BaseView<Type> *baseView){
        auto iconView = GTK_ICON_VIEW(gtk_icon_view_new());
        g_object_set(G_OBJECT(iconView), "has-tooltip", TRUE, NULL);
        gtk_icon_view_set_item_width (iconView, 60);
        gtk_icon_view_set_activate_on_single_click(iconView, TRUE);

 	
	g_object_set_data(G_OBJECT(baseView->treeModel()), "iconview", iconView); //do we really need this?
	gtk_icon_view_set_model(iconView, baseView->treeModel());

	gtk_icon_view_set_text_column (iconView, DISPLAY_NAME);
	gtk_icon_view_set_pixbuf_column (iconView,  DISPLAY_PIXBUF);
	gtk_icon_view_set_selection_mode (iconView, GTK_SELECTION_SINGLE);
        setUpSignals(baseView, G_OBJECT(iconView));
       
        return iconView;
    }

private:
    static void
    setUpSignals(BaseView<Type> *baseView, GObject * iconView){
        g_signal_connect (iconView, "item-activated", 
            ICONVIEW_CALLBACK (BaseViewSignals<Type>::activate), 
            (void *)baseView);
        g_signal_connect (iconView, "motion-notify-event", 
            ICONVIEW_CALLBACK (motionNotifyEvent), 
            (void *)baseView);
         //g_signal_connect (iconView, "query-tooltip", 
           //     G_CALLBACK (query_tooltip_f), 
           //     (void *)baseView);
 

         g_signal_connect (iconView, "button-release-event",
             G_CALLBACK(buttonRelease), 
             (void *)baseView);
         g_signal_connect (iconView, "button-press-event",
             G_CALLBACK(buttonPress), 
             (void *)baseView);
         // Why not "clicked" signal? 
         // Because this is to filter cancelled dnd event from
         // actual activated events.
         g_signal_connect (iconView, "button-release-event",
             G_CALLBACK(buttonClick), 
             (void *)baseView);
      
        // destination widget
        g_signal_connect (G_OBJECT (baseView->destination()), 
             "drag-data-received", G_CALLBACK (DragDataReceive),
             (void *)baseView);

        // "drag-motion" is not necessary with GTK_DEST_DEFAULT_MOTION
	// while using default iconview dnd, but this is not
	// our case. But seems to make no difference qith gtk+-3.24
        // Nonetheless, Drop targets will not be highlighted if
        // this is not set.
        g_signal_connect (G_OBJECT (baseView->destination()), 
             "drag-motion", 
	     G_CALLBACK (DragMotion),
	     (void *)baseView);
        
        // source widget
        g_signal_connect (G_OBJECT (baseView->source()), 
             "drag-begin", DRAG_CALLBACK (DragBegin),
             (void *)baseView);
        
        g_signal_connect (G_OBJECT (baseView->source()), 
             "drag-data-get", G_CALLBACK (BaseViewSignals<Type>::DragDataSend), 
             (void *)baseView);

        // Not necessary with GTK_DEST_DEFAULT_DROP
        //
        //g_signal_connect (G_OBJECT (baseView->destination()), 
        //    "drag-drop", G_CALLBACK (DragDrop),
        //    (void *)baseView);
        

    }

    ////////////////////   signal handlers iconview specific  ///////////////////

    static gboolean
    motionNotifyEvent (GtkWidget *widget,
                   GdkEvent  *ev,
                   gpointer   data)
    {
        auto e = (GdkEventMotion *)ev;
        auto event = (GdkEventButton  *)ev;
	auto baseView = (BaseView<Type> *)data;
        if (!data) {
            DBG("BaseView::motion_notify_event: data cannot be NULL\n");
            return FALSE;
        }
	TRACE("motion_notify_event, dragmode= %d\n", baseView->dragMode());

        if (buttonPressX >= 0 && buttonPressY >= 0){
	    TRACE("buttonPressX >= 0 && buttonPressY >= 0\n");
	    if (sqrt(pow(e->x - buttonPressX,2) + pow(e->y - buttonPressY, 2)) > 10){
                baseView->selectables();
                GList *selection_list = gtk_icon_view_get_selected_items (baseView->iconView());
                baseView->setSelectionList(selection_list);
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
		    gtk_icon_view_select_path (baseView->iconView(), tpath);
		    gtk_tree_path_free(tpath);
		    }

		}

                if (!targets) targets= gtk_target_list_new (targetTable,TARGETS);

		context =
		    gtk_drag_begin_with_coordinates (baseView->source(),
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
        baseView->highlight(e->x, e->y);

    
        return FALSE;
    }

    static gboolean
    buttonRelease (GtkWidget *widget,
                   GdkEventButton  *event,
                   gpointer   data)
    {
        //GdkEventButton *event_button = (GdkEventButton *)event;
	auto baseView = (BaseView<Type> *)data;
        if (!dragOn_){
	    GtkTreePath *tpath;

	    if (rubberBand_) {
                baseView->selectables();
		return FALSE;
	    }
	    if (!gtk_icon_view_get_item_at_pos (baseView->iconView(),
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
	    gtk_icon_view_unselect_all (baseView->iconView());
	    // reselect item to activate
	    gtk_icon_view_select_path (baseView->iconView(),tpath);
	    WARN("Here we do a call to activate item.\n");
	    BaseViewSignals<Type>::activate(tpath, data);
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
        auto baseView = (BaseView<Type> *)data;
        buttonPressX = buttonPressY = -1;
        dragOn_ = FALSE;

        GtkTreePath *tpath;
        if (event->button == 1) {
	    controlMode = FALSE;
            gboolean retval = FALSE;
            gint mode = 0;
            if (gtk_icon_view_get_item_at_pos (baseView->iconView(),
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
		    if (gtk_icon_view_path_is_selected (baseView->iconView(), tpath)) {
			// if selected
			gtk_icon_view_unselect_path (baseView->iconView(), tpath);
		    } else { // not selected
			//if (baseView->isSelectable(tpath)) 
                        gtk_icon_view_select_path (baseView->iconView(), tpath);
                        baseView->selectables();
		    }
		} else if (SHIFT_MODE) {
		    dragMode = -1; // move
                    viewShiftSelect(baseView, tpath);
		} else {
		    // unselect all
		    gtk_icon_view_unselect_all (baseView->iconView());
		    // select single item
		    gtk_icon_view_select_path (baseView->iconView(), tpath);
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
        return viewPopUp(baseView, event);
    }

    static gboolean
    buttonClick (GtkWidget *widget,
                   GdkEventButton  *event,
                   gpointer   data)
    {
	//auto baseView = (BaseView<Type> *)data;
	WARN("no action on button_click_f\n");
        return FALSE;
    }
    
/////////////////////////////////  DnD   ///////////////////////////
//receiver:

    static void
    DragDataReceive (GtkWidget * widget,
                      GdkDragContext * context,
                      gint x, gint y, 
                      GtkSelectionData * selection_data, 
                      guint info, 
                      guint time, 
                      gpointer data){
        DBG( "DND>> signal_drag_data\n");
	auto baseView = (BaseView<Type> *)data;




        GdkDragAction action = gdk_drag_context_get_selected_action(context);
        
        TRACE("rodent_mouse: DND receive, info=%d (%d,%d)\n", info, TARGET_STRING, TARGET_URI_LIST);
        if(info != TARGET_URI_LIST) {
            ERROR("signal_drag_data_receive: info != TARGET_URI_LIST\n");
            // not needed with GTK_DEST_DEFAULT_DROP
            // gtk_drag_finish(context, FALSE, FALSE, time);
            return;
        }
        if(action != GDK_ACTION_MOVE && 
           action != GDK_ACTION_COPY &&
           action != GDK_ACTION_LINK) {
            ERROR("Drag drop mode is not GDK_ACTION_MOVE | GDK_ACTION_COPY | GDK_ACTION_LINK\n");
            // not needed with GTK_DEST_DEFAULT_DROP
            // gtk_drag_finish(context, FALSE, FALSE, time);
            return;
        }

        gchar *target = NULL;
        GtkTreePath *tpath=NULL;
        if (gtk_icon_view_get_item_at_pos (baseView->iconView(),
                                   x, y, &tpath, NULL))
        {
            GtkTreeIter iter;
            gtk_tree_model_get_iter (baseView->treeModel(), &iter, tpath);
            gtk_tree_model_get (baseView->treeModel(), &iter, PATH, &target, -1);	
	    TRACE("target1=%s\n", target);
            if (!g_file_test(target, G_FILE_TEST_IS_DIR)){
                g_free(target);
                target=NULL;
            }
	    TRACE("target2=%s\n", target);
        } else {
	    TRACE("target3=%s\n", target);
	    tpath=NULL;
	}

                    // nah
       /* gtk_icon_view_get_drag_dest_item (view_p->get_iconview(),
                                      &tpath,
                                      GtkIconViewDropPosition *pos);*/
   
        
        if (tpath) gtk_tree_path_free(tpath);
        auto dndData = (const char *)gtk_selection_data_get_data (selection_data);
	DBG("dndData = \"\n%s\"\n", dndData);
        
        switch (baseView->viewType()) {
            case (LOCALVIEW_TYPE):
            {
                auto result = LocalDnd<Type>::receiveDndData(baseView, target, selection_data, action);
                TRACE("drag finish result=%d\n", result);
                break;
            }

            default :
                DBG("BaseViewSignals:: receiveDndData not defined for view type %d\n", baseView->viewType());
                break;
        }
        g_free(target);

     /*   gtk_drag_finish (context, result, 
                (action == GDK_ACTION_MOVE) ? result : FALSE, 
                time); */
        TRACE("DND receive, drag_over\n");
        return;

    } 

    static gboolean
    DragMotion (GtkWidget * widget, 
            GdkDragContext * dc, gint drag_x, gint drag_y, 
            guint t, gpointer data) {
	auto baseView = (BaseView<Type> *)data;
        WARN("signal_drag_motion\n");

        GtkTreePath *tpath;
                                        
        GtkIconViewDropPosition pos;
        gint actions = gdk_drag_context_get_actions(dc);
        if(actions == GDK_ACTION_MOVE)
            gdk_drag_status (dc, GDK_ACTION_MOVE, t);
        else if(actions == GDK_ACTION_COPY)
            gdk_drag_status (dc, GDK_ACTION_COPY, t);
        else if(actions == GDK_ACTION_LINK)
            gdk_drag_status (dc, GDK_ACTION_LINK, t);
        else
            gdk_drag_status (dc, GDK_ACTION_MOVE, t);
            
        if (gtk_icon_view_get_dest_item_at_pos (baseView->iconView(),
                                        drag_x, drag_y,
                                        &tpath,
                                        &pos)){
            GtkTreeIter iter;
            gtk_tree_model_get_iter (baseView->treeModel(), &iter, tpath);
            gchar *g;
            gtk_tree_model_get (baseView->treeModel(), &iter, PATH, &g, -1);
            // drop into?
            // must be a directory (XXX this is quite local stuff...)
            if (g_file_test(g, G_FILE_TEST_IS_DIR)){
                BaseModel<Type>::highlight(tpath, baseView);
            } else {
                BaseModel<Type>::highlight(NULL, baseView);
            }
	    g_free(g);
        } else {
            BaseModel<Type>::highlight(NULL, baseView);
        }
        return FALSE;
    }


    static void
    DragBegin (GtkWidget * widget, GdkDragContext * context, gpointer data) {
        WARN("signal_drag_begin\n");
	auto baseView = (BaseView<Type> *)data;
    //  single or multiple item selected?
        GList *selection_list = gtk_icon_view_get_selected_items (baseView->iconView());
        baseView->setSelectionList(selection_list);
        if (g_list_length(selection_list) > 1) {
            GdkPixbuf *pixbuf = Pixbuf<Type>::get_pixbuf("edit-copy", -48);
            gtk_drag_set_icon_pixbuf (context, pixbuf,10,10);
        } else {
            auto tpath = (GtkTreePath *)selection_list->data;
            GtkTreeIter iter;
            GdkPixbuf *pixbuf;
            gtk_tree_model_get_iter (baseView->treeModel(), &iter, tpath);
            gtk_tree_model_get (baseView->treeModel(), &iter, NORMAL_PIXBUF, &pixbuf, -1);
            gtk_drag_set_icon_pixbuf (context, pixbuf,10,10);
        }
	/*
        cairo_surface_t *icon;
        if (g_list_length(selection_list)==1){
            DBG("Single selection\n");
            icon = gtk_icon_view_create_drag_icon(baseView->iconView(), (GtkTreePath *)selection_list->data);
        } else if (g_list_length(selection_list)>1){
            DBG("Multiple selection\n");
            GdkPixbuf *pixbuf = Pixbuf<Type>::get_pixbuf("edit-copy", -96);

            gint width = gdk_pixbuf_get_width (pixbuf);
            gint height = gdk_pixbuf_get_height (pixbuf);
            ERROR("width=%d height=%d\n",width, height);
            GdkWindow *window = gtk_widget_get_parent_window (GTK_WIDGET(baseView->iconView()));
            if (!window) ERROR("gdk winodw is null\n");
            icon = gdk_window_create_similar_surface(window, CAIRO_CONTENT_COLOR_ALPHA, width, height);

            cairo_surface_set_device_offset(icon, 3,3);
         //   icon = Cairo<Type>::pixbuf_cairo_surface(pixbuf);
            cairo_t *cr = cairo_create (icon);
                    
            cairo_set_source_rgb (cr, (double)0x4a/0xff, (double)0x90/0xff, (double)0xd9/0xff);
            cairo_rectangle (cr, 0, 0, width, height);
            cairo_fill (cr);

            gdk_cairo_set_source_pixbuf (cr, pixbuf, 0, 0);
            cairo_paint (cr);
            cairo_destroy(cr);


        } else return;
   
        gtk_drag_set_icon_surface(context, icon);
        cairo_surface_destroy(icon);
	*/
    }
/////////////////////////////////////////////////////////////////////////////////////////

    
    static void
    viewShiftSelect(BaseView<Type> *baseView, GtkTreePath *tpath){
        // select all items in interval
        gtk_icon_view_select_path (baseView->iconView(), tpath);
        auto items = gtk_icon_view_get_selected_items (baseView->iconView());
        baseView->setSelectionList(items);

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
        gtk_icon_view_unselect_all (baseView->iconView());
        GtkTreePath *tp;
        for (int i=start; i<=end; i++){
                gchar *item = g_strdup_printf("%0d", i);
                WARN("selecting %s(%d)\n", item, i);
                tp = gtk_tree_path_new_from_string(item);
                g_free(item);
                //if (baseView->isSelectable(tpath)) 
                    
                gtk_icon_view_select_path (baseView->iconView(), tp);
            gtk_tree_path_free(tp);
        }
        baseView->selectables();
    }

    static gboolean 
    viewPopUp(BaseView<Type> *baseView, GdkEventButton  *event){
        TRACE("button press event: button 3 should do popup, as well as longpress...\n");
        GtkTreePath *tpath;
        gboolean retval = FALSE;
        GtkMenu *menu = NULL;
        if (gtk_icon_view_get_item_at_pos (baseView->iconView(),
                                   event->x,
                                   event->y,
                                   &tpath, NULL))
        if (CONTROL_MODE) {
           gtk_icon_view_unselect_all (baseView->iconView());
        } else {
            gtk_icon_view_select_path (baseView->iconView(), tpath);
	}
        gtk_tree_path_free(tpath);
           
       
        gchar *path = NULL;
        GList *selection_list = gtk_icon_view_get_selected_items (baseView->iconView());
        baseView->setSelectionList(selection_list);
        if (selection_list && g_list_length(selection_list) == 1) {
            GtkTreeIter iter;
            gtk_tree_model_get_iter(baseView->treeModel(), &iter, 
                    (GtkTreePath *)selection_list->data);
            gtk_tree_model_get(baseView->treeModel(), &iter, PATH, &path, -1);
        }
	if ((CONTROL_MODE) || g_list_length(selection_list) == 0) {
	    setMenuData(baseView, path, FALSE);
	    menu = configureMenu(baseView, FALSE);
	} else {
	    setMenuData(baseView, path,TRUE);
	    menu = configureMenu(baseView,TRUE);
	} 
        if (menu) {
            gtk_menu_popup_at_pointer (menu, (const GdkEvent *)event);
        }   
        return retval;
    }
public:
    static void
    setMenuData(BaseView<Type> * baseView, gchar *path, gboolean  items){
        GtkMenu *menu = NULL;
        switch (baseView->viewType()){
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
                    Fstab<Type>::popUpItem():
                    Fstab<Type>::popUp();
                break;
            default:
                ERROR("ViewType %d not defined.\n", baseView->viewType());
                break;
        }
        if (menu) {
           auto oldPath = (gchar *)g_object_get_data(G_OBJECT(menu),"path");
            g_free(oldPath);
            DBG("*** set menu data path=%s\n", path);
            g_object_set_data(G_OBJECT(menu),"path", path);
            g_object_set_data(G_OBJECT(menu),"baseView", (void *)baseView);
        }
    }
    
    static GtkMenu *
    configureMenu(BaseView<Type> * baseView, gboolean items){
        GtkMenu *menu = NULL;
        switch (baseView->viewType()){
            case (ROOTVIEW_TYPE):
                if (items) {
                    menu = rootItemPopUp;
                    RootView<Type>::resetMenuItems();
                } else {
                    menu = rootPopUp;
                    RootView<Type>::resetLocalPopup();
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
                    Fstab<Type>::resetMenuItems();
                } else {
                    menu = fstabPopUp;
                    Fstab<Type>::resetPopup();
                }
                break;
            default:
                ERROR("ViewType %d not defined.\n", baseView->viewType());
                break;
        }
        return menu;
    }


    
};
}
#endif
