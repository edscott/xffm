#ifndef XF_BASEVIEWSIGNALS__HH
# define XF_BASEVIEW__HH

#define SET_DIR(x) x|=0x01
#define IS_DIR (x&0x01)

GHashTable *highlight_hash=NULL;
#define CONTROL_MODE (event->state & GDK_CONTROL_MASK)
#define SHIFT_MODE (event->state & GDK_SHIFT_MASK)
enum {
    TARGET_URI_LIST,
    TARGET_MOZ_URL,
    TARGET_PLAIN,
    TARGET_UTF8,
    TARGET_STRING,
    TARGETS
};

static GtkTargetEntry targetTable[] = {
    {(gchar *)"text/uri-list", 0, TARGET_URI_LIST},
    {(gchar *)"text/x-moz-url", 0, TARGET_MOZ_URL},
    {(gchar *)"text/plain", 0, TARGET_PLAIN},
    {(gchar *)"UTF8_STRING", 0, TARGET_UTF8},
    {(gchar *)"STRING", 0, TARGET_STRING}
};

#define NUM_TARGETS (sizeof(targetTable)/sizeof(GtkTargetEntry))
static gboolean dragOn_=FALSE;
static gboolean rubberBand_=FALSE;
static gint buttonPressX=-1;
static gint buttonPressY=-1;
static gint dragMode_=0;

namespace xf
{
template <class Type> class BaseView;

template <class Type> 
class BaseViewSignals {
    using fmDialog_c = fmDialog<double>;
public:
    static void
    //item_activated (GtkIconView *iconview,
    item_activated (
                    const GtkTreePath *tpath,
                    gpointer     data)
    {
        // Get activated path.
        auto baseView = (BaseView<Type> *)data;

	gchar *path;
        GtkTreeIter iter;
        auto treeModel = gtk_icon_view_get_model(baseView->iconView());
        gtk_tree_model_get_iter (treeModel, &iter, (GtkTreePath *)tpath);
        //this is wrong here: gtk_tree_path_free (tpath);
        GdkPixbuf *normal_pixbuf;

        gtk_tree_model_get (treeModel, &iter, PATH, &path, -1);
	
        DBG("BaseView::item activated: %s\n", path);
        auto lastPath = g_strdup(baseView->path());
	if (!baseView->loadModel(path)){
            WARN("reloading %s\n", lastPath);
            baseView->loadModel(lastPath);
        }
	g_free(path);
	g_free(lastPath);
    }
   

    static gboolean
    unhighlight (gpointer key, gpointer value, gpointer data){
        auto baseView = (BaseView<Type> *)data;
        if (!baseView)  return FALSE;
        TRACE("unhighlight %s\n", (gchar *)key);
        GtkTreeModel *model =baseView->get_tree_model();
                
        GtkTreePath *tpath = gtk_tree_path_new_from_string ((gchar *)key);
        if (!tpath) return FALSE;

        GtkTreeIter iter;
        gboolean result = gtk_tree_model_get_iter (model, &iter, tpath);
        gtk_tree_path_free (tpath);

        if (!result) return TRUE;
        GdkPixbuf *normal_pixbuf;

        gtk_tree_model_get (model, &iter, 
                NORMAL_PIXBUF, &normal_pixbuf, -1);
        gtk_list_store_set (GTK_LIST_STORE(model), &iter,
                DISPLAY_PIXBUF, normal_pixbuf, 
            -1);
       return TRUE;

    }


    static gboolean
    button_click_f (GtkWidget *widget,
                   GdkEventButton  *event,
                   gpointer   data)
    {
	//auto baseView = (BaseView<Type> *)data;
	WARN("no action on button_click_f\n");
        return FALSE;
    }

    static gboolean
    button_release_f (GtkWidget *widget,
                   GdkEventButton  *event,
                   gpointer   data)
    {
        //GdkEventButton *event_button = (GdkEventButton *)event;
	auto baseView = (BaseView<Type> *)data;
        if (!dragOn_){
	    GtkTreePath *tpath;
	    if (!gtk_icon_view_get_item_at_pos (baseView->iconView(),
                                   event->x, event->y,
                                   &tpath,NULL)){
		WARN("button down cancelled.\n");
		if (rubberBand_) return FALSE;
		return TRUE;
	    }
	    if (rubberBand_) {
		gtk_tree_path_free(tpath);
		return FALSE;
	    }
	    //Cancel DnD prequel.
	    buttonPressX = buttonPressY = -1;
	    dragOn_ = FALSE;
	    if (dragMode_ ) { // copy/move/link mode
		return TRUE;
	    }
	    // default mode:

	    // unselect everything
	    gtk_icon_view_unselect_all (baseView->iconView());
	    // reselect item to activate
	    //gtk_icon_view_select_path (baseView->iconView(),tpath);
	    WARN("Here we do a call to activate item.\n");
	    item_activated(tpath, data);
	    gtk_tree_path_free(tpath);
	    return TRUE;
        }

	buttonPressX = buttonPressY = -1;
        dragOn_ = FALSE;
	
        dragMode_ = 0;
        return FALSE;
    }

    static gboolean
    button_press_f (GtkWidget *widget,
                   GdkEventButton  *event,
                   gpointer   data)
    {
        auto baseView = (BaseView<Type> *)data;
        buttonPressX = buttonPressY = -1;
        dragOn_ = FALSE;

        GtkTreePath *tpath;
        if (event->button == 1) {
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
		    dragMode_ = -3; // link mode
		} else if (CONTROL_MODE) {
		    dragMode_ = -2; // copy
		    // select item and add to selection list
		    if (gtk_icon_view_path_is_selected (baseView->iconView(), tpath)) {
			// if selected
			gtk_icon_view_unselect_path (baseView->iconView(), tpath);
		    } else { // not selected
			if (baseView->isSelectable(tpath)) gtk_icon_view_select_path (baseView->iconView(), tpath);
		    }
		} else if (SHIFT_MODE) {
		    // select all items in interval
		    dragMode_ = -1; // move
		    gtk_icon_view_select_path (baseView->iconView(), tpath);
		    auto items = gtk_icon_view_get_selected_items (baseView->iconView());

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
		    for (int i=start; i<=end; i++){
			    gchar *item = g_strdup_printf("%0d", i);
			    WARN("selecting %s(%d)\n", item, i);
			    GtkTreePath *tp = gtk_tree_path_new_from_string(item);
			    g_free(item);
			    if (baseView->isSelectable(tpath)) gtk_icon_view_select_path (baseView->iconView(), tp);
			    gtk_tree_path_free(tp);
		    }

		} else {
		    // unselect all
		    //gtk_icon_view_unselect_all (baseView->iconView());
		    // select single item
		    gtk_icon_view_select_path (baseView->iconView(), tpath);
		    dragMode_ = 0; // default (move)
		}
                retval = TRUE; 
		gtk_tree_path_free(tpath);
		tpath = NULL; // just in case.
            } else { 
                tpath=NULL;
		dragMode_ = 0;
		rubberBand_ = TRUE;
            }

            return retval;
        }

        // long press or button 3 should do popup menu...
        if (event->button != 3) return FALSE;

        WARN("button press event: button 3 should do popup, as well as longpress...\n");
        if (gtk_icon_view_get_item_at_pos (GTK_ICON_VIEW(widget),
                                   event->x,
                                   event->y,
                                   &tpath, NULL)) {

		gtk_tree_path_free(tpath);

        }

        
        gboolean retval = FALSE;

        /* FIXME: enable popup...       
        if (tpath) {
            retval = ((view_c *)data)->get_xfdir_p()->popup(tpath);
            gtk_tree_path_free(tpath);
        } else {
            retval = ((view_c *)data)->get_xfdir_p()->popup();
        }
        */

        return retval;
    }

    

    static gboolean
    motion_notify_event (GtkWidget *widget,
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
	    WARN("buttonPressX >= 0 && buttonPressY >= 0\n");
	    if (sqrt(pow(e->x - buttonPressX,2) + pow(e->y - buttonPressY, 2)) > 10){
	        // start DnD (multiple selection)
		WARN("dragOn_ = TRUE\n");
		dragOn_ = TRUE;
		auto targets= gtk_target_list_new (targetTable,TARGETS);
		auto context =
		    gtk_drag_begin_with_coordinates (GTK_WIDGET(baseView->iconView()),
			     targets,
			     GDK_ACTION_MOVE, //GdkDragAction actions,
			     1, //gint button,
			     (GdkEvent *)event, //GdkEvent *event,
			     event->x, event->y);
		buttonPressX = buttonPressY = -1;
	    }
        }

	// XXX: Why this limitation?
        // if (view_p->get_dir_count() > 500) return FALSE;
        baseView->highlight(e->x, e->y);

    
        return FALSE;
    }


/////////////////////////////////  DnD   ///////////////////////////
//receiver:

    static void
    signal_drag_data_receive (GtkWidget * widget,
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
            gtk_drag_finish(context, FALSE, FALSE, time);
            return;
      //            goto drag_over;         /* of course */
        }

        WARN("rodent_mouse: DND receive, action=%d\n", action);
        WARN("actions mv/cp/ln: %d/%d/%d\n", GDK_ACTION_MOVE, GDK_ACTION_COPY, GDK_ACTION_LINK);
        if(action != GDK_ACTION_MOVE && 
           action != GDK_ACTION_COPY &&
           action != GDK_ACTION_LINK) {
            ERROR("Drag drop mode is not GDK_ACTION_MOVE | GDK_ACTION_COPY | GDK_ACTION_LINK\n");
            gtk_drag_finish(context, FALSE, FALSE, time);
            return;
      //      goto drag_over;         /* of course */
        }


        gchar *target = NULL;
        GtkTreePath *tpath=NULL;
        if (gtk_icon_view_get_item_at_pos (baseView->iconView(),
                                   x, y, &tpath, NULL))
        {
            GtkTreeIter iter;
            gtk_tree_model_get_iter (baseView->treeModel(), &iter, tpath);
            gtk_tree_model_get (baseView->treeModel(), &iter, PATH, &target, -1);	
            if (!g_file_test(target, G_FILE_TEST_IS_DIR)){
                g_free(target);
                target=NULL;
            }
        } else tpath=NULL;

                    // nah
       /* gtk_icon_view_get_drag_dest_item (view_p->get_iconview(),
                                      &tpath,
                                      GtkIconViewDropPosition *pos);*/
   
        
        if (tpath) gtk_tree_path_free(tpath);
        auto dndData = (const char *)gtk_selection_data_get_data (selection_data);
	DBG("dndData = \"\n%s\"\n", dndData);
        
        auto result = baseView->receiveDndData(target, selection_data, action);

        // FIXME: if sourcedir == targetdir, 
        // gtk_drag_finish(context, FALSE, FALSE);
        WARN("drag finish result=%d\n", result);
        gtk_drag_finish (context, result, 
                (action == GDK_ACTION_MOVE) ? result : FALSE, 
                time);

        DBG("DND receive, drag_over\n");
        return;

    } 


    static gboolean
    signal_drag_motion (GtkWidget * widget, 
            GdkDragContext * dc, gint drag_x, gint drag_y, 
            guint t, gpointer data) {
	auto baseView = (BaseView<Type> *)data;
        DBG("signal_drag_motion\n");
                                        
        GtkTreePath *tpath;
                                        
        GtkIconViewDropPosition pos;
            
        if (gtk_icon_view_get_dest_item_at_pos (baseView->iconView(),
                                        drag_x, drag_y,
                                        &tpath,
                                        &pos)){
            GtkTreeIter iter;
            gtk_tree_model_get_iter (baseView->treeModel(), &iter, tpath);
            gchar *g;
            gtk_tree_model_get (baseView->treeModel(), &iter, ACTUAL_NAME, &g, -1);
            // drop into?
            // must be a directory
            if (g_file_test(g, G_FILE_TEST_IS_DIR)){
                baseView->highlight(tpath);
            } else {
                baseView->highlight(NULL);
            }
        } else {
            baseView->highlight(NULL);
        }
        return FALSE;
    }

    // sender:
    static void
    signal_drag_end (GtkWidget * widget, GdkDragContext * context, gpointer data) {
        WARN("signal_drag_end\n");
        
	auto baseView = (BaseView<Type> *)data;

        dragMode_ = 0;
        //gtk_drag_source_unset(GTK_WIDGET(baseView->iconView()));
        baseView->freeSelectionList();
        
    }

    static void
    signal_drag_begin (GtkWidget * widget, GdkDragContext * context, gpointer data) {
        WARN("signal_drag_begin\n");
	auto baseView = (BaseView<Type> *)data;

    //  single or multiple item selected?
        GList *selection_list = gtk_icon_view_get_selected_items (baseView->iconView());
        baseView->setSelectionList(selection_list);
        if (g_list_length(selection_list)==1){
            DBG("Single selection\n");
        } else if (g_list_length(selection_list)>1){
            DBG("Multiple selection\n");
        } else return;
    
    //  set drag icon
    /*
        drag_view_p = view_p;
        rodent_hide_tip ();
        if (!view_p->en || !view_p->en->path) return; 
        write_drag_info(view_p->en->path, view_p->en->type);
        view_p->mouse_event.drag_event.context = drag_context;*/
    }

    static void
    signal_drag_data_send (GtkWidget * widget,
                       GdkDragContext * context, 
                       GtkSelectionData * selection_data, 
                       guint info, 
                       guint time,
                       gpointer data) {
        WARN("signal_drag_data_send\n");
        //g_free(files);
        
        //int drag_type;

	auto baseView = (BaseView<Type> *)data;


        /* prepare data for the receiver */
        switch (info) {
#if 10
          case TARGET_UTF8:
            DBG( ">>> DND send, TARGET_UTF8\n"); return;
#endif
          case TARGET_URI_LIST:
            {
                DBG( ">>> DND send, TARGET_URI_LIST\n"); 
                GList *selection_list = baseView->selectionList();
                gboolean result = baseView->setDndData(selection_data, selection_list);
              
            }
            break;
          default:
            DBG( ">>> DND send, non listed target\n"); 
            break;
        }
    }

    static void
    signal_drag_leave (GtkWidget * widget, GdkDragContext * drag_context, guint time, gpointer data) {
        ERROR("signal_drag_leave\n");

    }

    static void
    signal_drag_delete (GtkWidget * widget, GdkDragContext * context, gpointer data) {
        ERROR("signal_drag_delete\n");
    }
    
};
}
#endif
