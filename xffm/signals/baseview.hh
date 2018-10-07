#ifndef XF_BASEVIEWSIGNALS__HH
# define XF_BASEVIEW__HH

#define SET_DIR(x) x|=0x01
#define IS_DIR (x&0x01)

GHashTable *highlight_hash=NULL;
#define CONTROL_MODE (event->state & GDK_CONTROL_MASK)
#define SHIFT_MODE (event->state & GDK_SHIFT_MASK)
enum {
    TARGET_URI_LIST,
    TARGET_PLAIN,
    TARGET_UTF8,
    TARGET_STRING,
    TARGET_ROOTWIN,
    TARGET_MOZ_URL,
    TARGET_XDS,
    TARGET_RAW,
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

namespace xf
{
template <class Type> class BaseView;

template <class Type> 
class BaseViewSignals {
    using fmDialog_c = fmDialog<double>;
public:
    static void
    item_activated (GtkIconView *iconview,
                    GtkTreePath *tpath,
                    gpointer     data)
    {
        DBG("BaseView::item activated\n");
	gchar *path = Type::item_activated(iconview, tpath, data);
	auto dialog = (fmDialog_c *)data;
	dialog->load(path); 

        //auto baseView = (BaseView<Type> *)data;
	//baseView->reload(path);
	g_free(path);
	
        /*
        auto view = (BaseView *)data;
        xfdir_c *xfdir_p = view_p->get_xfdir_p();
        xfdir_p->item_activated(iconview, tpath, data);
        */
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
        gtk_tree_model_get_iter (model, &iter, tpath);
        gtk_tree_path_free (tpath);
        GdkPixbuf *normal_pixbuf;

        gtk_tree_model_get (model, &iter, 
                baseView->normalPixbufC(), &normal_pixbuf, -1);
        gtk_list_store_set (GTK_LIST_STORE(model), &iter,
                baseView->iconColumn(), normal_pixbuf, 
            -1);
       return TRUE;

    }
    static gboolean
    button_release_f (GtkWidget *widget,
                   GdkEventButton  *event,
                   gpointer   data)
    {
        //GdkEventButton *event_button = (GdkEventButton *)event;
	auto baseView = (BaseView<Type> *)data;
        baseView->setDragMode(0);
        if (!gtk_icon_view_get_item_at_pos (baseView->iconView(),
                                   event->x, event->y,
                                   NULL,NULL)){
            // What here?
        }
        return FALSE;
    }

    static gboolean
    button_click_f (GtkWidget *widget,
                   GdkEventButton  *event,
                   gpointer   data)
    {
	auto baseView = (BaseView<Type> *)data;
        if (baseView->clickCancel()) return TRUE;
        return FALSE;
    }



    static gboolean
    button_press_f (GtkWidget *widget,
                   GdkEventButton  *event,
                   gpointer   data)
    {
        auto baseView = (BaseView<Type> *)data;

        if (event->button == 1) {
            gboolean retval = FALSE;
            //GList *selection_list = gtk_icon_view_get_selected_items (baseView->iconView());
            gint mode = 0;
            GtkTreePath *tpath;
            if (gtk_icon_view_get_item_at_pos (baseView->iconView(),
                                   event->x, event->y,
                                   &tpath,NULL)) {
                
                if (CONTROL_MODE && SHIFT_MODE) mode = -3; // link
                else if (CONTROL_MODE) mode = -2; // copy
                else if (SHIFT_MODE) mode = -1; // move
                else mode = -1; // default (move)
                baseView->setClickCancel(0);
            } else { 
                tpath=NULL;
                baseView->setClickCancel(-1);
            }
            DBG("button press %d mode %d\n", event->button, mode);
            baseView->setDragMode(mode);
            if (CONTROL_MODE){
                // select item
                gtk_icon_view_select_path (baseView->iconView(), tpath);
                retval = TRUE; 
            }
            if (tpath) gtk_tree_path_free(tpath);
            return retval;
        }

        // long press or button 3 should do popup menu...
        if (event->button != 3) return FALSE;
        GtkTreePath *tpath;

        WARN("button press event: button 3 should do popup, as well as longpress...\n");
        if (!gtk_icon_view_get_item_at_pos (GTK_ICON_VIEW(widget),
                                   event->x,
                                   event->y,
                                   &tpath, NULL)) {

            tpath = NULL;
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
        GdkEventMotion *e = (GdkEventMotion *)ev;
        GdkEventButton  *event = (GdkEventButton  *)ev;
	auto baseView = (BaseView<Type> *)data;
        if (!data) {
            DBG("BaseView::motion_notify_event: data cannot be NULL\n");
            return FALSE;
        }
	TRACE("motion_notify_event\n");

	// XXX: Why this limitation?
        // if (view_p->get_dir_count() > 500) return FALSE;
        baseView->highlight(e->x, e->y);
        return FALSE;
    }


/////////////////////////////////  DnD   ///////////////////////////
//receiver:

    static void
    signal_drag_data (GtkWidget * widget,
                      GdkDragContext * context,
                      gint x, gint y, 
                      GtkSelectionData * selection_data, 
                      guint info, 
                      guint time, 
                      gpointer data){
        DBG( "DND>> signal_drag_data\n");
	auto baseView = (BaseView<Type> *)data;

        gboolean result = FALSE;
        gchar *target = NULL;



        GdkDragAction action = gdk_drag_context_get_selected_action(context);
        
        TRACE("rodent_mouse: DND receive, info=%d (%d,%d)\n", info, TARGET_STRING, TARGET_URI_LIST);
        if(info != TARGET_URI_LIST) {
            gtk_drag_finish(context, FALSE, FALSE, time);
            return;
      //            goto drag_over;         /* of course */
        }

        WARN("rodent_mouse: DND receive, action=%d\n", action);
        WARN("actions mv/cp/ln: %d/%d/%d\n", GDK_ACTION_MOVE, GDK_ACTION_COPY, GDK_ACTION_LINK);
        if(action != GDK_ACTION_MOVE && 
           action != GDK_ACTION_COPY &&
           action != GDK_ACTION_LINK) {
            DBG("Drag drop mode is not GDK_ACTION_MOVE | GDK_ACTION_COPY | GDK_ACTION_LINK\n");
            gtk_drag_finish(context, FALSE, FALSE, time);
            return;
      //      goto drag_over;         /* of course */
        }


        GtkTreePath *tpath=NULL;
        if (gtk_icon_view_get_item_at_pos (baseView->iconView(),
                                   x, y, &tpath, NULL))
        {
            GtkTreeIter iter;
            gtk_tree_model_get_iter (baseView->treeModel(), &iter, tpath);
            gtk_tree_model_get (baseView->treeModel(), &iter, baseView->actualNameColumn(), &target, -1);	
        } else tpath=NULL;

                    // nah
       /* gtk_icon_view_get_drag_dest_item (view_p->get_iconview(),
                                      &tpath,
                                      GtkIconViewDropPosition *pos);*/
        // this stuff will be immersed in specific class
        // FIXME:
        // result = view_p->get_xfdir_p()->receive_dnd(target, selection_data, action);
   
        
        if (tpath) gtk_tree_path_free(tpath);
        g_free(target);
        auto dndData = (const char *)gtk_selection_data_get_data (selection_data);
        
	DBG("dndData = \"%s\"\n", dndData);

        // FIXME: if sourcedir == targetdir, 
        // gtk_drag_finish(context, FALSE, FALSE);
        gtk_drag_finish (context, TRUE, 
                (action == GDK_ACTION_MOVE) ? TRUE : FALSE, 
                time);

        DBG("rodent_mouse: DND receive, drag_over\n");
        return;

    } 


    static gboolean
    signal_drag_motion (GtkWidget * widget, 
            GdkDragContext * dc, gint drag_x, gint drag_y, 
            guint t, gpointer data) {
	auto baseView = (BaseView<Type> *)data;
        TRACE("signal_drag_motion\n");
                                        
        GtkTreePath *tpath;
                                        
        GtkIconViewDropPosition pos;
            
        if (gtk_icon_view_get_dest_item_at_pos (baseView->iconView(),
                                        drag_x, drag_y,
                                        &tpath,
                                        &pos)){
            GtkTreeIter iter;
            gtk_tree_model_get_iter (baseView->treeModel(), &iter, tpath);
            gchar *g;
            gtk_tree_model_get (baseView->treeModel(), &iter,  baseView->actualNameColumn(), &g, -1);
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

        baseView->setDragMode(0);
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
    signal_drag_data_get (GtkWidget * widget,
                       GdkDragContext * context, 
                       GtkSelectionData * selection_data, 
                       guint info, 
                       guint time,
                       gpointer data) {
        WARN("signal_drag_data_get\n");
        //g_free(files);
        
        //int drag_type;

	auto baseView = (BaseView<Type> *)data;


        /* prepare data for the receiver */
        switch (info) {
#if 10
          case TARGET_RAW:
            DBG( ">>> DND send, TARGET_RAW\n"); return;;
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
        DBG("signal_drag_leave\n");

    }

    static void
    signal_drag_delete (GtkWidget * widget, GdkDragContext * context, gpointer data) {
        DBG("signal_drag_delete\n");
    }
    
};
}
#endif
