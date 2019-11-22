#ifndef DND__HH
# define DND__HH

template <class Type>
class DnD {

    GtkTargetList *targets_;
    gboolean dragOn_;
    gint buttonPressX_;
    gint buttonPressY_;
    GList *selectionList_;
    GtkTreeModel *treeModel_;
    GtkListStore *listStore_;
    GtkTreeStore *treeStore_;
    GtkTreeView *treeView_;
    GtkWidget *window_;
    gint pathColumn_;
    GtkMenu *menu_;
    gboolean ignoreRelease_;
    GdkDragContext *context_;

    void setUpSignals(void){
        // source widget
        g_signal_connect (G_OBJECT (window_), 
                "drag-data-delete", G_CALLBACK (dragDelete),
                this);
        g_signal_connect (G_OBJECT (window_), 
                "drag-failed", G_CALLBACK (dragFailed),
                this);
        g_signal_connect (G_OBJECT (window_), 
                "drag-leave", G_CALLBACK (dragLeave), 
                this);
        g_signal_connect (G_OBJECT (window_), 
                "drag-end", G_CALLBACK (dragEnd), 
                this);


        g_signal_connect (G_OBJECT (window_), 
             "drag-begin", G_CALLBACK(dragBegin),
             this);       
        g_signal_connect (G_OBJECT (window_), 
             "drag-data-get", G_CALLBACK(dragDataSend), 
             this);

         g_signal_connect (window_, 
             "button-release-event", G_CALLBACK(buttonRelease), 
             this);
         g_signal_connect (treeView_, 
             "button-press-event",  G_CALLBACK(buttonPress), 
             this);
        
        // source widget
        g_signal_connect (treeView_,
            "motion-notify-event", G_CALLBACK (motionNotifyEvent), 
            this);
    }


public:
    void setMenu(GtkMenu *menu){menu_ = menu;}
    GtkMenu *menu(void){return menu_;}

    DnD(GtkWidget *window, GtkTreeView *treeView, gint pathColumn, gint flag) {
        context_ = NULL;
        dragOn_ = FALSE;
	ignoreRelease_ = FALSE;
	menu_ = NULL;
        buttonPressX_=-1;
        buttonPressY_=-1;
        selectionList_=NULL;
        targets_=NULL;
        treeModel_ = gtk_tree_view_get_model(treeView);
        if (GTK_IS_TREE_STORE(treeModel_)){
            treeStore_ = GTK_TREE_STORE(treeModel_);
            listStore_ = NULL;
        } else {
            treeStore_ = NULL;
            listStore_ = GTK_LIST_STORE(treeModel_);
        }
        treeView_ = treeView;
        window_ = window;
        pathColumn_ = pathColumn;
        setUpSignals();
        // createSourceTargetListFull(window_);
    }

    void setIgnoreRelease(gboolean state) {dragOn_ = state;}
    void setDragState(gboolean state) {dragOn_ = state;}
    void setButtonPress(gint x, gint y){buttonPressX_ = x, buttonPressY_=y;}
    void setSelectionList(GList *list){selectionList_=list;}
    gboolean dragState(void){ return dragOn_;}
    gint buttonPressX(void){return buttonPressX_;}
    gint buttonPressY(void){return buttonPressY_;}
    GList *selectionList(void){return selectionList_;}
    gboolean buttonPressed(void){ return buttonPressX_ >= 0 && buttonPressY_ >= 0;}
    GtkTreeModel *treeModel(void){ return treeModel_;}
    GtkTreeStore *treeStore(void){ return treeStore_;}
    GtkListStore *listStore(void){ return listStore_;}
    GtkTreeView *treeView(void){ return treeView_;}
    gint pathColumn(void){ return pathColumn_;}
    GtkWidget *window(void){ return window_;}
    gboolean ignoreRelease(void){return ignoreRelease_;}
    void setContext(GdkDragContext *context){context_ = context;} 


    void cancelDragState(void){
        setButtonPress(-1,-1);
        setDragState(FALSE);
    }
    void dragCancel(void){
        if (context_) gtk_drag_cancel (context_);
    }


//#define NUM_TARGETS (sizeof(targetTable)/sizeof(GtkTargetEntry))

#define DND_NUM_TARGETS 1
#define DND_TARGET_URI_LIST 0
    static GtkTargetEntry *
    targetTable(void) {
        static GtkTargetEntry targetTable_[] = {
            {(gchar *)"text/uri-list", 0, DND_TARGET_URI_LIST},
        };
        return targetTable_;
    }

    GtkTargetList *targets(void){
        if (!targets_) targets_= gtk_target_list_new (targetTable(),DND_NUM_TARGETS);
        return targets_;
    }

    GtkTreePath * 
    getTpath(GdkEventButton  *event){
        GtkTreePath *tpath = NULL;
        if (!gtk_tree_view_get_path_at_pos (treeView_, event->x, event->y, &tpath, NULL, NULL, NULL)){
           tpath = NULL;
        }
        return tpath;
    }

    gchar *
    getSelectionData(){
        GList *list = selectionList_;
        gchar *data = NULL;
        
        for(GList *tmp = list; tmp && tmp->data; tmp = tmp->next) {
            GtkTreePath *tpath = (GtkTreePath *)tmp->data;
            gchar *path;
            GtkTreeIter iter;
            gtk_tree_model_get_iter (treeModel_, &iter, tpath);
            gtk_tree_model_get (treeModel_, &iter, pathColumn(), &path, -1);
            if (g_file_test(path, G_FILE_TEST_EXISTS)){
                if (!data) data = g_strconcat("file://", path, "\n", NULL);
                else {
                    gchar *e = g_strconcat(data, "file://", path, "\n", NULL);
                    g_free(data);
                    data = e;
                }
                TRACE("getSelectionData(): append: %s -> \"%s\"\n", path, data);
            }
            g_free(path);
        }
        return data;
    }


    // For drops targeted at window, I guess...
    void
    createSourceTargetListFull (GtkWidget *widget) {
        TRACE("createSourceTargetList..\n");
        gtk_drag_source_set (widget,
                     (GdkModifierType) 0, //GdkModifierType start_button_mask,
                     targetTable(),
                     DND_NUM_TARGETS,
                     (GdkDragAction)
                                    ((gint)GDK_ACTION_MOVE|
                                     (gint)GDK_ACTION_COPY|
                                     (gint)GDK_ACTION_LINK));
        return;
    }

/////////   static ////////  
private:
    static gboolean 
    viewPopUp(DnD<Type> *object, GdkEventButton  *event){
        TRACE("Base::signals::button press event: button 3 should do popup, as well as longpress...\n");
	if (!object->menu()) return FALSE;

        gboolean retval = FALSE;
	/*
        GtkMenu *menu = NULL;
        gchar *path = NULL;
        GList *selectionList;
	    auto selection = gtk_tree_view_get_selection (view->treeView());
	    auto treeModel = view->treeModel();
	    selectionList = gtk_tree_selection_get_selected_rows (selection, &treeModel);
	    retval=TRUE;
        view->setSelectionList(selectionList);
        if (selectionList && g_list_length(selectionList) == 1) {
            GtkTreeIter iter;
            gtk_tree_model_get_iter(view->treeModel(), &iter, 
                    (GtkTreePath *)selectionList->data);
            gtk_tree_model_get(view->treeModel(), &iter, PATH, &path, -1);
            TRACE("Base::signals::selected path is %s\n", path);

	}

	
        gboolean items = (g_list_length(selectionList) >0);
        setMenuData(view, path, items);
        menu = configureMenu(view, items);

        if (menu) {
            gtk_menu_popup_at_pointer (menu, (const GdkEvent *)event);
        }
        return retval;
	*/
          
	// FIXME: colosal crash...
	//gtk_menu_popup_at_pointer (object->menu(), (const GdkEvent *)event);

        return TRUE;
    }
public:
    
#define XCONTROL_MODE (event->state & GDK_CONTROL_MASK)

    static gboolean
    buttonPress (GtkWidget *widget,
                   GdkEventButton  *event,
                   gpointer   data)
    {
        auto object = (DnD<Type> *)data;
	auto selection = gtk_tree_view_get_selection (object->treeView());
        gtk_tree_selection_set_select_function
                               (selection,
                                selectOn, NULL, NULL);
	
        if (event->button == 3){
	    return FALSE;
  	  // FIXME: colosal crash on doing gtk_menu_popup_at_pointer()...
            GtkTreeViewColumn *column;
            gint cellX, cellY;
	    GtkTreePath *tpath = NULL;
            if (gtk_tree_view_get_path_at_pos (object->treeView(), 
                               event->x, event->y, &tpath,
                              NULL, // &column,
                               &cellX, &cellY)){
                /*if (!LocalView<Type>::isSelectable(object->treeModel(),tpath)){
                    object->setIgnoreRelease(TRUE);
		    gtk_tree_path_free(tpath);
                    return FALSE;
                }*/
		GtkTreeIter iter;
		gtk_tree_model_get_iter(object->treeModel(), &iter, tpath);
		if (!gtk_tree_selection_iter_is_selected (selection, &iter)){
		    // if not selected, 
		    if (!XCONTROL_MODE){
			gtk_tree_selection_unselect_all (selection);
		    } 
		    gtk_tree_selection_select_path (selection, tpath);
		    gtk_tree_path_free(tpath);
		}
	    }
            TRACE("(%lf,%lf) -> %d,%d\n", event->x, event->y, cellX, cellY);
	    return viewPopUp(object, event);
	    
	}
	
        if (event->button != 1) return FALSE;
        TRACE("buttonpress 1\n");
        object->cancelDragState();
        GtkTreePath *tpath = NULL;
        gint mode = 0;
        tpath = object->getTpath(event);
        if (tpath == NULL){ 
            return FALSE;
        } else {
            if (object->selectionList()){
                g_list_free_full (object->selectionList(), (GDestroyNotify) gtk_tree_path_free);
                object->setSelectionList(NULL);
            } else TRACE("no selection list\n");
            TRACE("button press %d mode %d\n", event->button, mode);
            object->setButtonPress(event->x, event->y);
            gtk_tree_path_free(tpath);
        }
        return FALSE;
    }

    static gboolean
    buttonRelease (GtkWidget *widget,
                   GdkEventButton  *event,
                   gpointer   data)
    {
        auto object = (DnD<Type> *)data;
        TRACE("buttonRelease\n");
	auto selection = gtk_tree_view_get_selection (object->treeView());
        gtk_tree_selection_set_select_function
                               (selection,
                                selectOn, NULL, NULL);
        if (object->ignoreRelease()){
            object->setIgnoreRelease(FALSE);
            return FALSE;
        }
        //GdkEventButton *event_button = (GdkEventButton *)event;
        if (!object->dragState()){
	    GtkTreePath *tpath;
	    //Cancel DnD prequel.
            object->cancelDragState();
	    return TRUE;
        }

        object->cancelDragState();
        return FALSE;
    }
    static gboolean
    selectOn (GtkTreeSelection *selection,
                             GtkTreeModel *model,
                             GtkTreePath *path,
                             gboolean path_currently_selected,
                             gpointer data){
        return TRUE;
    }
    static gboolean
    selectOff (GtkTreeSelection *selection,
                             GtkTreeModel *model,
                             GtkTreePath *path,
                             gboolean path_currently_selected,
                             gpointer data){
        return FALSE;
    }
    static gboolean
    motionNotifyEvent (GtkWidget *widget,
                   GdkEvent  *ev,
                   gpointer   data)
    {
        auto e = (GdkEventMotion *)ev;
        auto event = (GdkEventButton  *)ev;
        auto object = (DnD<Type> *)data;

	auto selection = gtk_tree_view_get_selection (object->treeView());
        gtk_tree_selection_set_select_function
                               (selection,
                                selectOff, NULL, NULL);
        if (object->buttonPressed()){
            auto x = object->buttonPressX();
            auto y = object->buttonPressY();
	    TRACE("buttonPressX >= 0 && buttonPressY >= 0\n");
	    if (sqrt(pow(e->x - x,2) + pow(e->y - y, 2)) > 10){
                auto selection = gtk_tree_view_get_selection (object->treeView());
                auto treeModel = GTK_TREE_MODEL(object->treeStore());
                // free selection list
                auto selectionList = object->selectionList();
                if (!selectionList) selectionList = gtk_tree_selection_get_selected_rows (selection, &treeModel);
                object->setSelectionList(selectionList);
                if (selectionList==NULL) {
                    return FALSE;
                }

                auto tpath = (GtkTreePath *)selectionList->data;
                GtkTreeIter iter;
                gtk_tree_model_get_iter (treeModel, &iter, tpath);
                gchar *realpath;
                gtk_tree_model_get (treeModel, &iter, object->pathColumn(), &realpath, -1);
                TRACE("realpath=%s\n", realpath);
                if (realpath){
                    g_free(realpath);
                    // start DnD (multiple selection)
                    TRACE("dragOn_ = TRUE\n");
                    object->setDragState(TRUE);

                    auto targets= object->targets();

                    //context =
                        gtk_drag_begin_with_coordinates (object->window(), targets,
                                 (GdkDragAction)(((gint)GDK_ACTION_MOVE)|
                                 ((gint)GDK_ACTION_COPY)|
                                 ((gint)GDK_ACTION_LINK)), //GdkDragAction actions,
                                 1, //gint button,
                                 (GdkEvent *)event, //GdkEvent *event,
                                 event->x, event->y);
                }
                object->setButtonPress(-1,-1);
	    }
        }
        return FALSE;
    }

    static void
    dragDataSend (GtkWidget * widget,
                       GdkDragContext * context, 
                       GtkSelectionData * selection_data, 
                       guint info, 
                       guint time,
                       gpointer data) {
        TRACE("signal_drag_data_send\n");
        auto object = (DnD<Type> *)data;
        /* prepare data for the receiver */
        if (info != DND_TARGET_URI_LIST) {
            ERROR("signal_drag_data_send: invalid target");
        }
        TRACE( ">>> DND send, DND_TARGET_URI_LIST\n"); 
        gchar *dndData = NULL;
        dndData = object->getSelectionData();

        if (dndData){
            gtk_selection_data_set (selection_data, 
                gtk_selection_data_get_selection(selection_data),
                8, (const guchar *)dndData, strlen(dndData)+1);
        }
                    
    }


    // sender:
    static void
    dragBegin (GtkWidget * widget, GdkDragContext * context, gpointer data) {
        auto object = (DnD<Type> *)data;
        object->setContext(context);
        TRACE("signal_drag_begin\n");
        auto treeModel = GTK_TREE_MODEL(object->treeStore());
        auto selection = gtk_tree_view_get_selection (object->treeView());
        auto selectionList = object->selectionList();
        if (selectionList){
            g_list_free_full (selectionList, (GDestroyNotify) gtk_tree_path_free);
            selectionList = NULL;
        }
        selectionList = gtk_tree_selection_get_selected_rows (selection, &treeModel);
        object->setSelectionList(selectionList);
        //GdkPixbuf *pixbuf = Pixbuf<Type>::get_pixbuf("edit-copy", -48);
        //gtk_drag_set_icon_pixbuf (context, pixbuf,10,10);
    }


    static void
    dragEnd (GtkWidget * widget, GdkDragContext * context, gpointer data) {
        auto object = (DnD<Type> *)data;
        TRACE("signal_drag_end\n");
    }

    static gboolean
    dragFailed (GtkWidget      *widget,
                   GdkDragContext *context,
                   GtkDragResult   result,
                   gpointer        data){
        auto object = (DnD<Type> *)data;
        const gchar *message;
        switch (result) {
            case GTK_DRAG_RESULT_SUCCESS:
                message="The drag operation was successful.";
                break;
            case GTK_DRAG_RESULT_NO_TARGET:
                message="No suitable drag target.";
                break;
            case GTK_DRAG_RESULT_USER_CANCELLED:
                message="The user cancelled the drag operation.";
                break;
            case GTK_DRAG_RESULT_TIMEOUT_EXPIRED:
                message="The drag operation timed out.";
                break;
            case GTK_DRAG_RESULT_GRAB_BROKEN:
                message="The pointer or keyboard grab used for the drag operation was broken.";
                break;
            case GTK_DRAG_RESULT_ERROR:
                message="The drag operation failed due to some unspecified error.";
                break;
        }
        TRACE("Drag not accepted: %s\n", message);
        return TRUE;

    }


    static void
    dragLeave (GtkWidget * widget, GdkDragContext * drag_context, guint time, gpointer data) {
        auto object = (DnD<Type> *)data;
        TRACE("*** signal_drag_leave\n");

    }

    static gboolean removeDeleted (GtkTreeModel *treeModel,
                            GtkTreePath *tpath,
                            GtkTreeIter *iter,
                            gpointer data){
        auto object = (DnD<Type> *)data;
        gchar *path;
        gtk_tree_model_get (treeModel, iter, object->pathColumn(), &path, -1);
        if (!g_file_test(path, G_FILE_TEST_EXISTS)){
            if (object->treeStore()) {
                gtk_tree_store_remove(object->treeStore(), iter);
            } else {
                gtk_list_store_remove(object->listStore(), iter);
            } 
        }
        g_free(path);
        return FALSE;
    }

    static void
    dragDelete (GtkWidget * widget, GdkDragContext * context, gpointer data) {
        auto object = (DnD<Type> *)data;

        TRACE("*** signal_drag_delete\n");
        
        auto list = object->selectionList();
        GList *referenceList = NULL;
        for (;list && list->data; list=list->next){
            auto tpath = (GtkTreePath *)list->data;
            auto rowReference = gtk_tree_row_reference_new (object->treeModel(),tpath);
            referenceList=g_list_append(referenceList, rowReference);
        }
        for (list=referenceList;list && list->data; list=list->next){
            GtkTreeIter iter;
            auto rowReference = (GtkTreeRowReference *)list->data;
            auto tpath = gtk_tree_row_reference_get_path (rowReference);
            if (gtk_tree_model_get_iter (object->treeModel(), &iter, tpath)){
                if (object->treeStore()) {
                    gtk_tree_store_remove(object->treeStore(), &iter);
                } else {
                    gtk_list_store_remove(object->listStore(), &iter);
                } 
            }
            gtk_tree_row_reference_free(rowReference);
        }
        
    }
    
    
};
#endif
