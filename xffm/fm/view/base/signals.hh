#ifndef XF_BASEVIEWSIGNALS__HH
# define XF_BASEVIEWSIGNALS__HH

#define SET_DIR(x) x|=0x01
#define IS_DIR (x&0x01)
#define CONTROL_MODE (event->state & GDK_CONTROL_MASK)
#define SHIFT_MODE (event->state & GDK_SHIFT_MASK)

// Flag bits:
#define IS_NOTSELECTABLE(F) ((0x01<<1)&F)
#define SET_NOTSELECTABLE(F) (F|=(0x01<<1))

enum {
    TARGET_URI_LIST,
    TARGETS
};

static GtkTargetEntry targetTable[] = {
    {(gchar *)"text/uri-list", 0, TARGET_URI_LIST},
};

#define NUM_TARGETS (sizeof(targetTable)/sizeof(GtkTargetEntry))

namespace xf
{

template <class Type> class View;
template <class Type> class RootView;
template <class Type> class FstabView;
template <class Type> class LocalView;
template <class Type> class PkgView;
template <class Type> class BaseModel;

template <class Type> class PkgPopUp;
template <class Type> class LocalPopUp;
template <class Type> class RootPopUp;
template <class Type> class FstabPopUp;

static gboolean dragOn_=FALSE;
static gboolean rubberBand_=FALSE;
static gint buttonPressX=-1;
static gint buttonPressY=-1;

static GtkTargetList *targets=NULL;
static GdkDragContext *context=NULL;

static gboolean controlMode = FALSE;

static GHashTable *highlight_hash=NULL;
static GHashTable *validBaseViewHash = NULL;
static gint dragMode=0;


template <class Type> 
class BaseSignals {
public:

    static gboolean
    motionNotifyEvent (GtkWidget *widget,
                   GdkEvent  *ev,
                   gpointer   data)
    {
        auto e = (GdkEventMotion *)ev;
        auto event = (GdkEventButton  *)ev;
	auto view = (View<Type> *)data;
        if (!data) {
            ERROR("fm/base/signals.hh::motion_notify_event: data cannot be NULL\n");
            return FALSE;
        }
	TRACE("motion_notify_event, dragmode= %d\n", view->dragMode());

        if (buttonPressX >= 0 && buttonPressY >= 0){
	    TRACE("buttonPressX >= 0 && buttonPressY >= 0\n");
	    if (sqrt(pow(e->x - buttonPressX,2) + pow(e->y - buttonPressY, 2)) > 10){
		GList *selectionList;
                view->selectables();
		if (isTreeView) {
		    auto selection = gtk_tree_view_get_selection (view->treeView());
		    auto treeModel = view->treeModel();
		    selectionList = gtk_tree_selection_get_selected_rows (selection, &treeModel);
		} else {
		    selectionList = gtk_icon_view_get_selected_items (view->iconView());
		}
                view->setSelectionList(selectionList);
                if (selectionList==NULL) {
                    return FALSE;
                }
	        // start DnD (multiple selection)
		TRACE("dragOn_ = TRUE\n");
		dragOn_ = TRUE;
		// in control mode, reselect item at x,y
		if(!isTreeView && controlMode) {
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
	//      Because it really slows highlight down
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
	    if (!isTreeView && !gtk_icon_view_get_item_at_pos (view->iconView(),
                                   event->x, event->y,
                                   &tpath,NULL)){
		TRACE("button down cancelled.\n");
		return TRUE;
	    }

	    //Cancel DnD prequel.
	    buttonPressX = buttonPressY = -1;
	    dragOn_ = FALSE;
	    if (dragMode) { // copy/move/link mode
		return TRUE;
	    }
	    // default mode:
	    if (CONTROL_MODE) return TRUE;

	    if (isTreeView){
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
	    } else {
		// unselect everything
		gtk_icon_view_unselect_all (view->iconView());
		// reselect item to activate
		gtk_icon_view_select_path (view->iconView(),tpath);
	    }
	    if (tpath) {
		TRACE("Here we do a call to activate item.\n");
		for (auto popup=popUpArray; popup && *popup; popup++){
		    g_object_set_data(G_OBJECT(*popup), "baseModel", (void *)view);
		    g_object_set_data(G_OBJECT(*popup), "view", (void *)view);
		}
		BaseSignals<Type>::activate(tpath, data);
		gtk_tree_path_free(tpath);
	    }
	    return TRUE;
        }

	buttonPressX = buttonPressY = -1;
        dragOn_ = FALSE;
	
        dragMode = 0;
        view->selectables();
        return FALSE;
    }

    static GtkTreePath * 
    getTpath(View<Type> *view, GdkEventButton  *event){
        GtkTreePath *tpath = NULL;
        if (isTreeView){
           if (!gtk_tree_view_get_path_at_pos (view->treeView(), event->x, event->y, &tpath, NULL, NULL, NULL)){
               tpath = NULL;
           }
        } else {
            if (!gtk_icon_view_get_item_at_pos (view->iconView(), event->x, event->y, &tpath,NULL)) {
               tpath = NULL;
            }
        }
        return tpath;
    }

    static void 
    controlSelect(View<Type> *view, GtkTreePath *tpath){
        if (isTreeView){
            GtkTreeIter iter;
            gtk_tree_model_get_iter(view->treeModel(), &iter, tpath);
            auto selection = gtk_tree_view_get_selection (view->treeView());
            if (gtk_tree_selection_iter_is_selected (selection, &iter)){
                gtk_tree_selection_unselect_path (selection, tpath);
            } else { 
                if (view->isSelectable(tpath)){
                    gtk_tree_selection_select_path (selection, tpath);
                }
            }   
        } else {
            if (gtk_icon_view_path_is_selected (view->iconView(), tpath)) {
                gtk_icon_view_unselect_path (view->iconView(), tpath);
            } else {
                if (view->isSelectable(tpath)){
                    gtk_icon_view_select_path (view->iconView(), tpath);
                }
            }
        }
    }

    static void 
    reSelect(View<Type> *view, GtkTreePath *tpath){
        if (isTreeView){
            GtkTreeIter iter;
            gtk_tree_model_get_iter(view->treeModel(), &iter, tpath);
            auto selection = gtk_tree_view_get_selection (view->treeView());
            gtk_tree_selection_unselect_all (selection);
            if (!gtk_tree_selection_iter_is_selected (selection, &iter)){
                if (view->isSelectable(tpath)){
                   gtk_tree_selection_select_path (selection, tpath);
                } 
            } 
        } else {
            gtk_icon_view_unselect_all (view->iconView());
            if (!gtk_icon_view_path_is_selected (view->iconView(), tpath)) {
                if (view->isSelectable(tpath)){
                    gtk_icon_view_select_path (view->iconView(), tpath);
                }
            }
        }
    }

    static gboolean
    buttonPress (GtkWidget *widget,
                   GdkEventButton  *event,
                   gpointer   data)
    {
        auto view = (View<Type> *)data;
        buttonPressX = buttonPressY = -1;
        dragOn_ = FALSE;

        GtkTreePath *tpath = NULL;
        if (event->button == 1) {
	    controlMode = FALSE;
            gint mode = 0;
            tpath = getTpath(view, event);
	    dragMode = 0; // default (move)
            if (tpath == NULL){ 
		rubberBand_ = TRUE;
                return FALSE;
            } else {
		TRACE("button press %d mode %d\n", event->button, mode);
		buttonPressX = event->x;
		buttonPressY = event->y;
		rubberBand_ = FALSE;

                if (CONTROL_MODE && SHIFT_MODE) dragMode = -3; // link mode
	        else if (CONTROL_MODE) dragMode = -2; // copy
                else if (SHIFT_MODE)   dragMode = -1; // move

                if (CONTROL_MODE){ 
	            controlMode = TRUE;
                    controlSelect(view, tpath);
                    return TRUE;
                }

                if (SHIFT_MODE) {
                    viewShiftSelect(view, tpath);
                    return TRUE;
		}

                reSelect(view, tpath);
		gtk_tree_path_free(tpath);
            }

            return TRUE;
        }

        // long press or button 3 should do popup menu...
        if (event->button != 3) return FALSE;
	if (isTreeView){
            GtkTreeViewColumn *column;
            gint cellX, cellY;
            if (gtk_tree_view_get_path_at_pos (view->treeView(), 
                               event->x, event->y, &tpath,
                              NULL, // &column,
                               &cellX, &cellY)){
		GtkTreeIter iter;
		auto selection = gtk_tree_view_get_selection (view->treeView());
		gtk_tree_model_get_iter(view->treeModel(), &iter, tpath);
		if (!gtk_tree_selection_iter_is_selected (selection, &iter)){
		    // if not selected, 
		    if (!CONTROL_MODE){
			gtk_tree_selection_unselect_all (selection);
		    } 
		    gtk_tree_selection_select_path (selection, tpath);
		    gtk_tree_path_free(tpath);
		}
	    }
            TRACE("(%lf,%lf) -> %d,%d\n", event->x, event->y, cellX, cellY);
	} else {
	    if (gtk_icon_view_get_item_at_pos (view->iconView(),
				       event->x,
				       event->y,
				       &tpath, NULL)){
		if (!gtk_icon_view_path_is_selected (view->iconView(), tpath)) {
		    // If item is not selected, unselect all and select item.
		    // Skip unselect all in CONTROL_MODE
		    if (!CONTROL_MODE) {
		       gtk_icon_view_unselect_all (view->iconView());
		    } 
		    gtk_icon_view_select_path (view->iconView(), tpath);
		    gtk_tree_path_free(tpath);
		}
		// if item is selected, proceed.
	    }
	}
        return viewPopUp(view, event);
    }
    
    static void
    viewShiftSelect(View<Type> *view, GtkTreePath *tpath){
        // select all items in interval
	GList * items;
        GtkTreeSelection *selection;
        if (isTreeView) {
            selection = gtk_tree_view_get_selection (view->treeView());
	    gtk_tree_selection_select_path (selection, tpath);
	    auto treeModel = view->treeModel();
	    items = gtk_tree_selection_get_selected_rows (selection, &treeModel);
        } else {
            gtk_icon_view_select_path (view->iconView(), tpath);
            items = gtk_icon_view_get_selected_items (view->iconView());
        }
        view->setSelectionList(items);

        gchar *item = gtk_tree_path_to_string (tpath);
        gchar *startItem;
        if (strrchr(item, ':')) startItem = g_strdup(strrchr(startItem, ':')+1);
        else startItem = g_strdup(item);
        g_free(item);

        TRACE("Starting from %s\n", startItem);

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
                //if (LocalView<Type>::isSelectable(view->treeModel(), tpath)){

        if (s->prev == NULL) {
            gchar *lastitem = gtk_tree_path_to_string ((GtkTreePath *)g_list_last(items)->data);
            end = atoi(startItem);
            start = atoi(lastitem);
            TRACE("lastitem %s\n", lastitem);
            g_free(lastitem);

        } else {
            gchar *firstitem = gtk_tree_path_to_string ((GtkTreePath *)g_list_first(items)->data);
            end = atoi(firstitem);
            start = atoi(startItem);
            TRACE("firstitem %s\n", firstitem);
            g_free(firstitem);
        }
        g_free(startItem);
        // To free the return value, use:
        // g_list_free_full (items, (GDestroyNotify) gtk_tree_path_free);
        // This is done when we reset the view selection list...
        TRACE("loop %d -> %d\n", start, end);
        
        if (isTreeView){
	    gtk_tree_selection_unselect_all (selection);
        } else {
            gtk_icon_view_unselect_all (view->iconView());
        }
        GtkTreePath *tp;
        auto first = (start <= end)?start:end;
        auto last = (end >= start)?end:start;
        for (int i=first; i<=last; i++){
            gchar *item = g_strdup_printf("%0d", i);
            TRACE("selecting %s(%d)\n", item, i);
            tp = gtk_tree_path_new_from_string(item);
            g_free(item);
            //if (view->isSelectable(tpath)) 
                
            if (isTreeView){
                gtk_tree_selection_select_path (selection, tp);
            } else {
                gtk_icon_view_select_path (view->iconView(), tp);
            }
            gtk_tree_path_free(tp);
        }
        view->selectables();
    }

    static gboolean 
    viewPopUp(View<Type> *view, GdkEventButton  *event){
        TRACE("Base::signals::button press event: button 3 should do popup, as well as longpress...\n");
        gboolean retval = FALSE;
        GtkMenu *menu = NULL;
        gchar *path = NULL;
        GList *selectionList;
	if (isTreeView) {
	    auto selection = gtk_tree_view_get_selection (view->treeView());
	    auto treeModel = view->treeModel();
	    selectionList = gtk_tree_selection_get_selected_rows (selection, &treeModel);
	    retval=TRUE;
	} else {
	    selectionList = gtk_icon_view_get_selected_items (view->iconView());
	}
        view->setSelectionList(selectionList);
        if (selectionList && g_list_length(selectionList) == 1) {
            GtkTreeIter iter;
            gtk_tree_model_get_iter(view->treeModel(), &iter, 
                    (GtkTreePath *)selectionList->data);
            gtk_tree_model_get(view->treeModel(), &iter, PATH, &path, -1);
            TRACE("Base::signals::selected path is %s\n", path);
	    //hack here
	    //auto t = getViewType(path);
	    //view->setViewType(t);
	}

	
        gboolean items = (g_list_length(selectionList) >0);
        setMenuData(view, path, items);
        menu = configureMenu(view, items);
/*	if ((CONTROL_MODE) || g_list_length(selectionList) == 0) {
	    setMenuData(view, path, FALSE);
	    menu = configureMenu(view, FALSE);
	} else {
	    setMenuData(view, path,TRUE);
	    menu = configureMenu(view,TRUE);
	} */
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
                    RootPopUp<Type>::popUpItem():
                    RootPopUp<Type>::popUp();
                break;
            case (LOCALVIEW_TYPE):
                menu = (items)?
                    LocalPopUp<Type>::popUpItem():
                    LocalPopUp<Type>::popUp();
                break;
            case (FSTAB_TYPE):
                 menu = (items)?
                    FstabPopUp<Type>::popUpItem():
                    FstabPopUp<Type>::popUp();
                break;
            case (PKG_TYPE):
                 menu = (items)?
                    PkgPopUp<Type>::popUpItem():
                    PkgPopUp<Type>::popUp();
                break;
            default:
                ERROR("fm/base/signals.hh::ViewType %d not defined.\n", view->viewType());
                break;
        }
        if (menu) {
           auto oldPath = (gchar *)g_object_get_data(G_OBJECT(menu),"path");
            g_free(oldPath);
            TRACE("*** set menu data path=%s\n", path);
            g_object_set_data(G_OBJECT(menu),"path", path);
            g_object_set_data(G_OBJECT(menu),"view", (void *)view);
        }
    }
    
    static GtkMenu *
    configureMenu(View<Type> * view, gboolean items){
        GtkMenu *menu = NULL;
        switch (view->viewType()){
            case (ROOTVIEW_TYPE):
		TRACE("configureMenu  ROOTVIEW_TYPE\n");
                if (items) {
                    menu = rootItemPopUp;
                    RootPopUp<Type>::resetMenuItems();
                } else {
                    menu = rootPopUp;
                    RootPopUp<Type>::resetPopup();
                }
                break;
            case (LOCALVIEW_TYPE):
		TRACE("configureMenu  LOCALVIEW_TYPE\n");
                if (items) {
                    menu = localItemPopUp;
                    LocalPopUp<Type>::resetMenuItems();
                } else {
                    menu = localPopUp;
                    LocalPopUp<Type>::resetLocalPopup();
                }
                break;
            case (FSTAB_TYPE):
		TRACE("configureMenu  FSTAB_TYPE\n");
                if (items) {
                    menu = fstabItemPopUp;
                    FstabPopUp<Type>::resetMenuItems();
                } else {
                    menu = fstabPopUp;
                    FstabPopUp<Type>::resetPopup();
                }
                break;
            case (PKG_TYPE):
		TRACE("configureMenu  PKG_TYPE\n");
                if (items) {
                    menu = pkgItemPopUp;
                    PkgPopUp<Type>::resetMenuItems();
                } else {
                    menu = pkgPopUp;
                    PkgPopUp<Type>::resetPopup();
                }
                break;
            default:
                ERROR("fm/base/signals.hh::ViewType %d not defined.\n", view->viewType());
                break;
        }
        return menu;
    }


    static gboolean
    unhighlight (gpointer key, gpointer value, gpointer data){
        auto view = (View<Type> *)data;
        if (!view)  return FALSE;
        TRACE("unhighlight %s\n", (gchar *)key);
        GtkTreeModel *model =view->treeModel();
                
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
    static void
    activate (GtkTreePath *tpath, gpointer data)
    {
        // Get activated path.
        auto view = (View<Type> *)data;

	gchar *path;
        GtkTreeIter iter;
        auto treeModel = view->treeModel();
        if (!gtk_tree_model_get_iter (treeModel, &iter, (GtkTreePath *)tpath)){
            DBG("tpath does not exist. Aborting activate signal.\n");
            return;
        }
        GdkPixbuf *normal_pixbuf;

        gtk_tree_model_get (treeModel, &iter, PATH, &path, -1);
	
        TRACE("base-signals::activate: %s\n", path);
        //auto lastPath = g_strdup(view->path());
	if (!view->loadModel(treeModel, tpath, path)){
            ERROR("base-signals:activate():cannot load %s\n", path);
            //view->loadModel(lastPath);
        }
	g_free(path);
	//g_free(lastPath);
    }

    // DnD highlight only:
    static void
    highlight(GtkTreePath *tpath, gpointer data){
        if (isTreeView) return;
        GtkTreeIter iter;
	auto baseModel = (BaseModel<Type> *)data;
        if (baseModel->items() > 260){ 
            gtk_icon_view_set_drag_dest_item(baseModel->iconView(), NULL, GTK_ICON_VIEW_DROP_INTO);
            if (tpath == NULL) {
                return;
            } 
            if (gtk_tree_model_get_iter (baseModel->treeModel(), &iter, tpath)){
                guint type;
                gtk_tree_model_get (baseModel->treeModel(), &iter, 
                    FLAGS, &type, -1);
                type &= 0xff;
                if (type == DT_DIR) {
                    gtk_icon_view_set_drag_dest_item(baseModel->iconView(), tpath, GTK_ICON_VIEW_DROP_INTO);
                } else {
                    gtk_icon_view_set_drag_dest_item(baseModel->iconView(), tpath, GTK_ICON_VIEW_NO_DROP);
                }
            }

            return; 
        }     
        gchar *tree_path_string = NULL;
        
        if (tpath == NULL){
            // No item at position?
            // Do we need to clear hash table?
            TRACE("highlight clear_highlights\n");
            clear_highlights(data);
            return;
        }
        gtk_tree_model_get_iter (baseModel->treeModel(), &iter, tpath);


        // Already highlighted?
        tree_path_string = gtk_tree_path_to_string (tpath);
	if (g_hash_table_lookup(highlight_hash, tree_path_string)) {
            //TRACE("%s already in hash\n", tree_path_string);
            g_free (tree_path_string);
            gtk_tree_path_free (tpath);
            return;
        }
        TRACE("highlight \n");

        // Not highlighted. First clear any other item which highlight remains.
        clear_highlights(data);
        // Now do highlight dance. 
        g_hash_table_insert(highlight_hash, tree_path_string, GINT_TO_POINTER(1));
        
        GdkPixbuf *highlight_pixbuf;
        gtk_tree_model_get (baseModel->treeModel(), &iter, 
                HIGHLIGHT_PIXBUF, &highlight_pixbuf, -1);
        gtk_list_store_set (GTK_LIST_STORE(baseModel->treeModel()), &iter,
                DISPLAY_PIXBUF, highlight_pixbuf, 
                -1);
        return;
    }

    static void
    clear_highlights(gpointer data){
        if (!highlight_hash || g_hash_table_size(highlight_hash) == 0) return;
        g_hash_table_foreach_remove (highlight_hash, unhighlight, data);
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

    static void
    createDestTargetList (GtkWidget *widget) {
        TRACE("createDestTargetList..\n");
        gtk_drag_dest_set (widget,
                     (GtkDestDefaults)
                                   ((gint)GTK_DEST_DEFAULT_DROP|
                                    (gint)GTK_DEST_DEFAULT_MOTION),
                     targetTable,
                     NUM_TARGETS,
                     (GdkDragAction)
                                    ((gint)GDK_ACTION_MOVE|
                                     (gint)GDK_ACTION_COPY|
                                     (gint)GDK_ACTION_LINK));
        return;
   }
    
    static gboolean validBaseView(BaseModel<Type> *baseModel) {
	return GPOINTER_TO_INT(g_hash_table_lookup(validBaseViewHash, (void *)baseModel));
    }
    
    static gint
    getViewType(const gchar *path){
	TRACE("getViewType: %s\n", path);
        if (!path) return ROOTVIEW_TYPE;
        if (g_file_test(path, G_FILE_TEST_EXISTS)) return (LOCALVIEW_TYPE);
        if (strcmp(path, "/dev/disks")==0) return (LOCALVIEW_TYPE);
	if (g_path_is_absolute(path)){
	    gchar *m;
	    if (strstr(path,"/.local/share/Trash")) {
		m = g_strdup(_("Trash is empty"));
	    } else {
		m = g_strdup_printf("%s: %s\n",_("Directory does not exist."), path); 
	    }
	    Gtk<Type>::quickHelp(mainWindow, m, "dialog-error");
	    ERROR("fm/base/signals.hh::getViewType: %s\n",m);
	    g_free(m);
	}
        if (strcmp(path, "xffm:local")==0) return (LOCALVIEW_TYPE);
        if (strcmp(path, "xffm:root")==0) return (ROOTVIEW_TYPE);
        if (strcmp(path, "xffm:fstab")==0) return (FSTAB_TYPE);
        if (strcmp(path, "xffm:nfs")==0) return (NFS_TYPE);
        if (strcmp(path, "xffm:sshfs")==0) return (SSHFS_TYPE);
        if (strcmp(path, "xffm:ecryptfs")==0) return (ECRYPTFS_TYPE);
        if (strcmp(path, "xffm:cifs")==0) return (CIFS_TYPE);
        if (strncmp(path, "xffm:pkg", strlen("xffm:pkg"))==0) return (PKG_TYPE);
	
        ERROR("fm/base/signals.hh::getViewType() %s not defined.\n", path);
        return (-1);
    }

    /////////////////////   gsignals  /////////////////////////
/////////////////////////////////  DnD   ///////////////////////////
    static gboolean
    DragMotion (GtkWidget * widget, 
            GdkDragContext * dc, gint drag_x, gint drag_y, 
            guint t, gpointer data) {
	auto view = (View<Type> *)data;
        TRACE("signal_drag_motion\n");

        GtkTreePath *tpath;
                                        
        gint actions = gdk_drag_context_get_actions(dc);
        if(actions == GDK_ACTION_MOVE)
            gdk_drag_status (dc, GDK_ACTION_MOVE, t);
        else if(actions == GDK_ACTION_COPY)
            gdk_drag_status (dc, GDK_ACTION_COPY, t);
        else if(actions == GDK_ACTION_LINK)
            gdk_drag_status (dc, GDK_ACTION_LINK, t);
        else
            gdk_drag_status (dc, GDK_ACTION_MOVE, t);
            
       // Treeview or iconview?
        if (isTreeView) return FALSE;
        
        GtkIconViewDropPosition pos;
        if (gtk_icon_view_get_dest_item_at_pos (view->iconView(),
                                        drag_x, drag_y,
                                        &tpath,
                                        &pos)){
            GtkTreeIter iter;
            gtk_tree_model_get_iter (view->treeModel(), &iter, tpath);
            gchar *g;
            gtk_tree_model_get (view->treeModel(), &iter, PATH, &g, -1);
            // drop into?
            // must be a directory (XXX this is quite local stuff...)
            if (g_file_test(g, G_FILE_TEST_IS_DIR)){
                highlight(tpath, view);
            } else {
                highlight(NULL, view);
            }
	    g_free(g);
        } else {
            highlight(NULL, view);
            highlight(NULL, view);
        }
        return FALSE;
    }

    static void
    DragDataSend (GtkWidget * widget,
                       GdkDragContext * context, 
                       GtkSelectionData * selection_data, 
                       guint info, 
                       guint time,
                       gpointer data) {
        TRACE("signal_drag_data_send\n");
        //g_free(files);
        
        //int drag_type;

	auto view = (View<Type> *)data;
        /* prepare data for the receiver */
        if (info != TARGET_URI_LIST) {
            ERROR("fm/base/signals.hh::signal_drag_data_send: invalid target");
        }
        TRACE( ">>> DND send, TARGET_URI_LIST\n"); 
        gchar *dndData = NULL;
        switch (view->viewType()) {
            case (LOCALVIEW_TYPE):
            {
                dndData = Dnd<Type>::sendDndData(view);
                TRACE("drag finish result=%d\n", result);
                break;
            }

            default :
                ERROR("fm/base/signals.hh::sendDndData not defined for view type %d\n", view->viewType());
                break;
        }

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
	auto view = (View<Type> *)data;
        auto treeModel = view->treeModel();
        // Treeview or iconview?
        //  single or multiple item selected?
        GList *selectionList;
        if (isTreeView){
            auto selection = gtk_tree_view_get_selection (view->treeView());
            selectionList = gtk_tree_selection_get_selected_rows (selection, &treeModel);
        } else {
            selectionList = gtk_icon_view_get_selected_items (view->iconView());
        }
        view->setSelectionList(selectionList);
        if (g_list_length(selectionList) > 1) {
            GdkPixbuf *pixbuf = Pixbuf<Type>::get_pixbuf("edit-copy", -48);
            gtk_drag_set_icon_pixbuf (context, pixbuf,10,10);
        } else {
            auto tpath = (GtkTreePath *)selectionList->data;
            GtkTreeIter iter;
            GdkPixbuf *pixbuf;
            gtk_tree_model_get_iter (view->treeModel(), &iter, tpath);
            gtk_tree_model_get (view->treeModel(), &iter, NORMAL_PIXBUF, &pixbuf, -1);
            gtk_drag_set_icon_pixbuf (context, pixbuf,10,10);
        }
    }
//receiver:

    static void
    DragDataReceive (GtkWidget * widget,
                      GdkDragContext * context,
                      gint x, gint y, 
                      GtkSelectionData * selection_data, 
                      guint info, 
                      guint time, 
                      gpointer data){
        TRACE( "DND>> signal_drag_data\n");
	auto view = (View<Type> *)data;

        // Treeview or iconview?
        GdkDragAction action = gdk_drag_context_get_selected_action(context);
        
        TRACE("rodent_mouse: DND receive, info=%d (%d,%d)\n", info, TARGET_STRING, TARGET_URI_LIST);
        if(info != TARGET_URI_LIST) {
            ERROR("fm/base/signals.hh::signal_drag_data_receive: info != TARGET_URI_LIST\n");
            // not needed with GTK_DEST_DEFAULT_DROP
            // gtk_drag_finish(context, FALSE, FALSE, time);
            return;
        }
        if(action != GDK_ACTION_MOVE && 
           action != GDK_ACTION_COPY &&
           action != GDK_ACTION_LINK) {
            ERROR("fm/base/signals.hh::Drag drop mode is not GDK_ACTION_MOVE | GDK_ACTION_COPY | GDK_ACTION_LINK\n");
            // not needed with GTK_DEST_DEFAULT_DROP
            // gtk_drag_finish(context, FALSE, FALSE, time);
            return;
        }

        gchar *target = NULL;
        GtkTreePath *tpath=NULL;
        if (isTreeView) {
            // Simple drop for now since xy coordinates differ from treeview coordinates...
            target = g_strdup(view->path());
        } else {
            if (gtk_icon_view_get_item_at_pos (view->iconView(),
                                       x, y, &tpath, NULL))
            {
                GtkTreeIter iter;
                gtk_tree_model_get_iter (view->treeModel(), &iter, tpath);
                gtk_tree_model_get (view->treeModel(), &iter, PATH, &target, -1);	
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
        }
        
        if (tpath) gtk_tree_path_free(tpath);
        auto dndData = (const char *)gtk_selection_data_get_data (selection_data);
	TRACE("dndData = \"\n%s\"\n", dndData);
        
        switch (view->viewType()) {
            case (LOCALVIEW_TYPE):
            {
                auto result = Dnd<Type>::receiveDndData(view, target, selection_data, action);
                TRACE("drag finish result=%d\n", result);
                break;
            }

            default :
                ERROR("fm/base/signals.hh::receiveDndData not defined for view type %d\n", view->viewType());
                break;
        }
        g_free(target);

     /*   gtk_drag_finish (context, result, 
                (action == GDK_ACTION_MOVE) ? result : FALSE, 
                time); */
        TRACE("DND receive, drag_over\n");
        return;
    }


    static void
    signal_drag_end (GtkWidget * widget, GdkDragContext * context, gpointer data) {
        TRACE("signal_drag_end\n");
        
	auto view = (View<Type> *)data;

        dragMode = 0;
      
    }

    static gboolean
    signal_drag_failed (GtkWidget      *widget,
                   GdkDragContext *context,
                   GtkDragResult   result,
                   gpointer        user_data){
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
        ERROR("fm/base/signals.hh::Drag was not accepted: %s\n", message);
        return TRUE;

    }


    static void
    signal_drag_leave (GtkWidget * widget, GdkDragContext * drag_context, guint time, gpointer data) {
        TRACE("signal_drag_leave\n");

    }

    static void
    signal_drag_delete (GtkWidget * widget, GdkDragContext * context, gpointer data) {
        ERROR("fm/base/signals.hh::signal_drag_delete\n");
    }
    
public:

    static gboolean
    buttonClick (GtkWidget *widget,
                   GdkEventButton  *event,
                   gpointer   data)
    {
	//auto view = (View<Type> *)data;
	TRACE("no action on button_click_f\n");
        return FALSE;
    }


};
}
#endif
