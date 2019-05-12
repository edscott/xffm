#ifndef SIGNALS_HH
#define SIGNALS_HH

namespace xf {

gboolean dragOn_ = FALSE;
gint buttonPressX=-1;
gint buttonPressY=-1;

enum {
    TARGET_URI_LIST,
    TARGETS
};
static GtkTargetList *targets=NULL;

static GtkTargetEntry targetTable[] = {
    {(gchar *)"text/uri-list", 0, TARGET_URI_LIST},
};

#define NUM_TARGETS (sizeof(targetTable)/sizeof(GtkTargetEntry))

template <class Type>
class Signals {

    static GtkTreePath * 
    getTpath(GdkEventButton  *event){
        GtkTreePath *tpath = NULL;
        if (!gtk_tree_view_get_path_at_pos (treeView, event->x, event->y, &tpath, NULL, NULL, NULL)){
           tpath = NULL;
        }
        return tpath;
    }
 
    static gchar *
    getSelectionData(){
        GList *selection_list = selectionList;
        gchar *data = NULL;
        
        for(GList *tmp = selection_list; tmp && tmp->data; tmp = tmp->next) {
            GtkTreePath *tpath = (GtkTreePath *)tmp->data;
            gchar *path;
            GtkTreeIter iter;
            gtk_tree_model_get_iter (GTK_TREE_MODEL(treeStore), &iter, tpath);
            gtk_tree_model_get (GTK_TREE_MODEL(treeStore), &iter, REALPATH, &path, -1);
            if (!data) data = g_strconcat(URIFILE, path, "\n", NULL);
            else {
                gchar *e = g_strconcat(data, URIFILE, path, "\n", NULL);
                g_free(data);
                data = e;
            }
            TRACE("getSelectionData(): append: %s -> \"%s\"\n", path, data);
            g_free(path);
        }
        return data;
    }

public:
    static void
    DragDataSend (GtkWidget * widget,
                       GdkDragContext * context, 
                       GtkSelectionData * selection_data, 
                       guint info, 
                       guint time,
                       gpointer data) {
        TRACE("signal_drag_data_send\n");
        /* prepare data for the receiver */
        if (info != TARGET_URI_LIST) {
            ERROR("signal_drag_data_send: invalid target");
        }
        TRACE( ">>> DND send, TARGET_URI_LIST\n"); 
        gchar *dndData = NULL;
        dndData = getSelectionData();


        if (dndData){
            gtk_selection_data_set (selection_data, 
                gtk_selection_data_get_selection(selection_data),
                8, (const guchar *)dndData, strlen(dndData)+1);
        }
                    
    }

    // sender:
    static void
    DragBegin (GtkWidget * widget, GdkDragContext * context, gpointer data) {
        TRACE("signal_drag_begin\n");
        auto treeModel = GTK_TREE_MODEL(treeStore);
        auto selection = gtk_tree_view_get_selection (treeView);
        if (selectionList){
            g_list_free_full (selectionList, (GDestroyNotify) gtk_tree_path_free);
            selectionList = NULL;
        }
        selectionList = gtk_tree_selection_get_selected_rows (selection, &treeModel);
        //GdkPixbuf *pixbuf = Pixbuf<Type>::get_pixbuf("edit-copy", -48);
        //gtk_drag_set_icon_pixbuf (context, pixbuf,10,10);
    }
    static void
    createSourceTargetList (GtkWidget *widget) {
	TRACE("createSourceTargetList..\n");
	gtk_drag_source_set (widget,
		     (GdkModifierType) 0, //GdkModifierType start_button_mask,
		     targetTable,
		     NUM_TARGETS,
		     (GdkDragAction)
				    ((gint)GDK_ACTION_MOVE|
				     (gint)GDK_ACTION_COPY|
				     (gint)GDK_ACTION_LINK));
	return;
    }
    static gboolean delete_event (GtkWidget *widget,
	       GdkEvent  *event,
	       gpointer   user_data){
	gtk_widget_hide(widget);
	while (gtk_events_pending()) gtk_main_iteration();
	gtk_main_quit();
	_exit(123);
	return TRUE;
    }

    static void exitApp (GtkButton *widget,
	       gpointer   user_data){
	gtk_widget_hide(GTK_WIDGET(mainWindow));
	while (gtk_events_pending()) gtk_main_iteration();
	gtk_main_quit();
	_exit(123);
	return;
    }

//gtk_drag_set_icon_pixbuf (context, pixbuf,10,10);
    }

    static gboolean
    buttonPress (GtkWidget *widget,
                   GdkEventButton  *event,
                   gpointer   data)
    {
        TRACE("buttonpress\n");
        buttonPressX = buttonPressY = -1;
        dragOn_ = FALSE;

        GtkTreePath *tpath = NULL;
        if (event->button == 1) {
            gint mode = 0;
        TRACE("buttonpress 1\n");
            tpath = getTpath(event);
            if (tpath == NULL){ 
                return FALSE;
            } else {
                if (selectionList){
                    g_list_free_full (selectionList, (GDestroyNotify) gtk_tree_path_free);
                    selectionList = NULL;
                } else TRACE("no selection list\n");
                TRACE("button press %d mode %d\n", event->button, mode);
		buttonPressX = event->x;
		buttonPressY = event->y;
		gtk_tree_path_free(tpath);
            }

            return FALSE;
        }
        return FALSE;

    }
    static gboolean
    buttonRelease (GtkWidget *widget,
                   GdkEventButton  *event,
                   gpointer   data)
    {
        TRACE("buttonRelease\n");
        //GdkEventButton *event_button = (GdkEventButton *)event;
        if (!dragOn_){
	    GtkTreePath *tpath;

	    //Cancel DnD prequel.
	    buttonPressX = buttonPressY = -1;
	    dragOn_ = FALSE;

	    /*if (isTreeView){
		auto selection = 
		    gtk_tree_view_get_selection (view->treeView());
		gtk_tree_view_get_path_at_pos (view->treeView(), 
				   event->x, event->y, &tpath, NULL,  NULL, NULL);
		if (tpath) {
		    // unselect everything
		    gtk_tree_selection_unselect_all (selection);
		    // reselect item to activate
		    gtk_tree_selection_select_path (selection, tpath);
		}
	    } 
	    if (tpath) {
		TRACE("Here we do a call to activate item.\n");
		for (auto popup=popUpArray; popup && *popup; popup++){
		    g_object_set_data(G_OBJECT(*popup), "baseModel", (void *)view);
		    g_object_set_data(G_OBJECT(*popup), "view", (void *)view);
		}
		BaseSignals<Type>::activate(tpath, data);
		gtk_tree_path_free(tpath);
	    }*/
	    return TRUE;
        }

	buttonPressX = buttonPressY = -1;
        dragOn_ = FALSE;
	
        return FALSE;
    }


    static gboolean
    motionNotifyEvent (GtkWidget *widget,
                   GdkEvent  *ev,
                   gpointer   data)
    {
        auto e = (GdkEventMotion *)ev;
        auto event = (GdkEventButton  *)ev;

        if (buttonPressX >= 0 && buttonPressY >= 0){
	    TRACE("buttonPressX >= 0 && buttonPressY >= 0\n");
	    if (sqrt(pow(e->x - buttonPressX,2) + pow(e->y - buttonPressY, 2)) > 10){
                auto selection = gtk_tree_view_get_selection (treeView);
                auto treeModel = GTK_TREE_MODEL(treeStore);
                // free selection list
                if (!selectionList) selectionList = gtk_tree_selection_get_selected_rows (selection, &treeModel);
                if (selectionList==NULL) {
                    return FALSE;
                }

                auto tpath = (GtkTreePath *)selectionList->data;
                GtkTreeIter iter;
                gtk_tree_model_get_iter (GTK_TREE_MODEL(treeStore), &iter, tpath);
                gchar *realpath;
                gtk_tree_model_get (GTK_TREE_MODEL(treeStore), &iter, REALPATH, &realpath, -1);
                TRACE("realpath=%s\n", realpath);
                if (realpath){
                    g_free(realpath);
                    // start DnD (multiple selection)
                    TRACE("dragOn_ = TRUE\n");
                    dragOn_ = TRUE;

                    if (!targets) targets= gtk_target_list_new (targetTable,TARGETS);

                    //context =
                        gtk_drag_begin_with_coordinates (GTK_WIDGET(mainWindow),
                                 targets,
                                 (GdkDragAction)(((gint)GDK_ACTION_MOVE)|
                       ((gint)GDK_ACTION_COPY)|
                       ((gint)GDK_ACTION_LINK)), //GdkDragAction actions,
                                 1, //gint button,
                                 (GdkEvent *)event, //GdkEvent *event,
                                 event->x, event->y);
                }
                             
		buttonPressX = buttonPressY = -1;
                //g_object_ref(G_OBJECT(context)); 
	    }
        }



    
        return FALSE;
    }

};
}


#endif
