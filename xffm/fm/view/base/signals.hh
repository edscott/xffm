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
GtkWidget *popupImage = NULL;

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

static gboolean ignoreClick=FALSE;
static gboolean ignoreRelease=FALSE;
static gboolean dragOn_=FALSE;
static gboolean noMotion=TRUE;
static gboolean rubberBand_=FALSE;
static gint buttonPressX=-1;
static gint buttonPressY=-1;

static GtkTargetList *targets=NULL;
static GdkDragContext *context=NULL;

static gboolean controlMode = FALSE;

static GHashTable *highlight_hash=NULL;
static GHashTable *validBaseViewHash = NULL;
static gint dragMode=0;

static gint longPressTime;
static pthread_mutex_t longPressMutex=PTHREAD_MUTEX_INITIALIZER;

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
	    
	noMotion=FALSE;

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
        gint longPressCount;
	auto view = (View<Type> *)data;
        pthread_mutex_lock(&longPressMutex);
            longPressCount = longPressTime;
            longPressTime = -1;
        pthread_mutex_unlock(&longPressMutex);
        if ( noMotion and longPressCount >= 10) {
            TRACE("Long press detected...\n");
            buttonPressX = buttonPressY = -1;
            return doPopupMenu(event, data);
        }
        if (ignoreRelease){
            ignoreRelease=FALSE;
            return FALSE;
        }
        //GdkEventButton *event_button = (GdkEventButton *)event;
        if (!dragOn_){

	    if (rubberBand_) {
                view->selectables();
		return FALSE;
	    }

	    auto tpath = getTpath(view, event);
	    if (!tpath){
		TRACE("!getTpath(): button down cancelled.\n");
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
	    unselectAll(data);
	    reSelect(data, tpath);

	    TRACE("Here we do a call to activate item.\n");
	    for (auto popup=popUpArray; popup && *popup; popup++){
		g_object_set_data(G_OBJECT(*popup), "baseModel", (void *)view);
		g_object_set_data(G_OBJECT(*popup), "view", (void *)view);
	    }
	    activate(tpath, data);
	    gtk_tree_path_free(tpath);
	    return TRUE;
        }

	buttonPressX = buttonPressY = -1;
        dragOn_ = FALSE;
	
        dragMode = 0;
        view->selectables();
        return FALSE;
    }

    static GtkTreePath *
    getTpath(gpointer data, GdkEventButton  *event){
	auto view = (View<Type> *)data;
        GtkTreePath *tpath = NULL;
        if (isTreeView){
           if (!gtk_tree_view_get_path_at_pos (view->treeView(), 
		       event->x, event->y, &tpath, NULL, NULL, NULL))
	   {
               tpath = NULL;
           }
        } else {
            if (!gtk_icon_view_get_item_at_pos (view->iconView(), 
			event->x, event->y, &tpath,NULL)) 
	    {
               tpath = NULL;
            }
	}
        return tpath;
    }

    static void 
    unselectAll(gpointer data){
	auto view = (View<Type> *)data;
	// unselect everything
	if (isTreeView){
	    auto selection = 
	    gtk_tree_view_get_selection (view->treeView());
	    gtk_tree_selection_unselect_all (selection);
	} else {
	    gtk_icon_view_unselect_all (view->iconView());
	    
	}
    }

    static void 
    reSelect(gpointer data, GtkTreePath *tpath){
	auto view = (View<Type> *)data;
	if (!tpath) return;
        if (!view->isSelectable(tpath))return;
	// reselect item to activate
	;
	if (isTreeView){
	    auto selection = 
		gtk_tree_view_get_selection (view->treeView());
	    gtk_tree_selection_select_path (selection, tpath);
	} else {
	    gtk_icon_view_select_path (view->iconView(),tpath);
	}
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

    static void *
    showImage(void *data){
        gtk_widget_show(GTK_WIDGET(data));
        return NULL;
    }

    static void *longPressCheck_f(void *data){
        pthread_mutex_lock(&longPressMutex);
            longPressTime = 0;
        pthread_mutex_unlock(&longPressMutex);
        for (gint i=0; i<10; i++){
            usleep(100000);
            pthread_mutex_lock(&longPressMutex);
            if (longPressTime < 0) {
                pthread_mutex_unlock(&longPressMutex);
                return NULL;
            }
            longPressTime++;
            pthread_mutex_unlock(&longPressMutex);
        }
        if (noMotion) Util<Type>::context_function(showImage, (void *)popupImage);
        return NULL;
    }


    static GtkMenu *
    setItemPath(View<Type> *view){
        TRACE("Base::signals::button press event: button 3 should do popup, as well as longpress...\n");
        gchar *itemPath = NULL;
        GList *selectionList;
	if (isTreeView) {
	    auto selection = gtk_tree_view_get_selection (view->treeView());
	    auto treeModel = view->treeModel();
	    selectionList = gtk_tree_selection_get_selected_rows (selection, &treeModel);
	} else {
	    selectionList = gtk_icon_view_get_selected_items (view->iconView());
	}
        view->setSelectionList(selectionList);
        if (selectionList && g_list_length(selectionList) == 1) {
            GtkTreeIter iter;
            gtk_tree_model_get_iter(view->treeModel(), &iter, 
                    (GtkTreePath *)selectionList->data);
            gtk_tree_model_get(view->treeModel(), &iter, PATH, &itemPath, -1);
	}
	//if (!path) path = g_strdup(view->path());
        
        TRACE("Base::signals::setItemPath: selected path is %s\n", itemPath);

        auto items = (g_list_length(selectionList) >0);
        auto menu = configureMenu(view, items, itemPath?itemPath:view->path());
	TRACE("menu is %p\n", menu);
        g_object_set_data(G_OBJECT(menu),"view", (void *)view);
	TRACE("g_object_set_data %p->%p\n", menu, view);
        TRACE("**Base::signals::setItemPath: selected path is %s\n", itemPath);
	Popup<Type>::setWidgetData(menu, "itemPath", itemPath?itemPath:view->path());

	g_free(itemPath);
        return menu; 
    }

    static GtkMenu *
    setPopupSelection(GdkEventButton  *event, gpointer data){
        TRACE("setPopupSelection\n");
	auto view = (View<Type> *)data;
        if (SHIFT_MODE) {
	    TRACE("SHIFT_MODE button 3\n");
	    unselectAll(data);  
	    return NULL;
	}
        GtkTreePath *tpath = getTpath(data, event);
	// If item is not selected, unselect all and select item.
	// Skip unselect all in CONTROL_MODE
        if (tpath && view->isSelectable(tpath)){
	    if (isTreeView){
		auto selection = 
		    gtk_tree_view_get_selection (view->treeView());
		    GtkTreeIter iter;
		    gtk_tree_model_get_iter(view->treeModel(), &iter, tpath);
		    if (!gtk_tree_selection_iter_is_selected (selection, &iter)
			    && !CONTROL_MODE){
			gtk_tree_selection_unselect_all (selection);
		    }
		    gtk_tree_selection_select_path (selection, tpath);
	    } else {
		if (!gtk_icon_view_path_is_selected (view->iconView(), tpath)
			&& !CONTROL_MODE ) {
		   gtk_icon_view_unselect_all (view->iconView());
		}
		gtk_icon_view_select_path (view->iconView(), tpath);
	    }

	    
	}
	if (tpath) gtk_tree_path_free(tpath);
        return setItemPath(view);
    }

    static gboolean doPopupMenu(GdkEventButton  *event, gpointer data){
        gtk_widget_hide(GTK_WIDGET(xf::popupImage));
	auto menu = setPopupSelection(event, data);
	if (menu) gtk_menu_popup_at_pointer (menu, (const GdkEvent *)event);

        return isTreeView;
    }


    static gboolean
    buttonPress (GtkWidget *widget,
                   GdkEventButton  *event,
                   gpointer   data)
    {
	noMotion=TRUE;
	if (ignoreClick) return TRUE;
        ignoreClick = TRUE; // don't process another click until this one is done.
        auto view = (View<Type> *)data;
        buttonPressX = buttonPressY = -1;
        dragOn_ = FALSE;

        GtkTreePath *tpath = NULL;
        if (event->button == 1) {
            pthread_mutex_lock(&longPressMutex);
            longPressTime = 0;
            pthread_mutex_unlock(&longPressMutex);
	    
            pthread_t longPressThread; 
            
	    pthread_create (&longPressThread, NULL, longPressCheck_f, NULL);
	    pthread_detach(longPressThread);
            
	    controlMode = FALSE;
            gint mode = 0;
            tpath = getTpath(data, event);
	    dragMode = 0; // default (move)
            if (tpath == NULL){ 
		rubberBand_ = TRUE;
                ignoreClick = FALSE;
		dragOn_ = TRUE;
                return FALSE;
            } else {
		buttonPressX = event->x;
		buttonPressY = event->y;
		rubberBand_ = FALSE;

                if (CONTROL_MODE && SHIFT_MODE) dragMode = -3; // link mode
	        else if (CONTROL_MODE) dragMode = -2; // copy
                else if (SHIFT_MODE)   dragMode = -1; // move

                if (CONTROL_MODE){ 
	            controlMode = TRUE;
                    controlSelect(view, tpath);
                    ignoreClick = FALSE;
                    return TRUE;
                }

                if (SHIFT_MODE) {
                    viewShiftSelect(view, tpath);
                    ignoreClick = FALSE;
                    return TRUE;
		}

		if (isTreeView){
		    auto selection = gtk_tree_view_get_selection (view->treeView());
		} else {
		    
		}
		unselectAll(data);
                reSelect(data, tpath);
		gtk_tree_path_free(tpath);
            }
            ignoreClick = FALSE;
            return TRUE;
        }

        // long press or button 3 should do popup menu...
        // long press will activate on button release.
        if (event->button != 3) {
            ignoreClick = FALSE;
            return FALSE;
        }
        auto result = doPopupMenu(event, data);
        ignoreClick = FALSE;
	return result;
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
    
public:
    
    static GtkMenu *
    configureMenu(View<Type> *view, gboolean items, const gchar *itemPath){
	TRACE("configureMenu\n" );
	GtkMenu *menu;
	if (items) {
	    menu = getItemsMenu(view);
	    Popup<Type>::setWidgetData(menu, "path", view->path());
	    Popup<Type>::setWidgetData(menu, "itemPath", itemPath);
	    configureItemsMenu(view->viewType());
	} else {
	    menu =  getViewMenu(view);
	    Popup<Type>::setWidgetData(menu, "path", view->path());
	    Popup<Type>::setWidgetData(menu, "itemPath", itemPath);
	    configureViewMenu(view->viewType());
	}

	return menu;
    }
    static GtkMenu *
    getItemsMenu(View<Type> *view){
	gint viewType = view->viewType();
	TRACE("getItemsMenu\n" );
	GtkMenu *menu;
        switch (viewType){
            case (ROOTVIEW_TYPE):
		    menu = RootPopUp<Type>::popUpItem(); 
                break;
            case (LOCALVIEW_TYPE):
		    menu = LocalPopUp<Type>::popUpItem();
                break;
#ifdef ENABLE_FSTAB_MODULE
            case (FSTAB_TYPE):
		    menu =  FstabPopUp<Type>::popUpItem();
                break;
            case (EFS_TYPE):
                break;
#endif
#ifdef ENABLE_PKG_MODULE
            case (PKG_TYPE):
		    menu =  PkgPopUp<Type>::popUpItem(); 
                break;
#endif

            default:
                ERROR("fm/base/signals.hh::getItemsMenu() ViewType %d not defined.\n", viewType);
		menu = RootPopUp<Type>::popUp(); 
                break;
        }
	TRACE("menu = %p\n", menu);
	g_object_set_data(G_OBJECT(menu),"view", view);
        return menu;
    }

    static void
    configureItemsMenu(gint viewType){
	TRACE("configureItemsMenu\n" );
	GtkMenu *menu;
        switch (viewType){
            case (ROOTVIEW_TYPE):
                    RootPopUp<Type>::resetMenuItems();
                break;
            case (LOCALVIEW_TYPE):
                    LocalPopUp<Type>::resetMenuItems();
                break;
#ifdef ENABLE_FSTAB_MODULE
            case (FSTAB_TYPE):
                    FstabPopUp<Type>::resetMenuItems();
                break;
            case EFS_TYPE:
                //EFS<Type>::doDialog();
                
                break;
#endif
#ifdef ENABLE_PKG_MODULE
            case (PKG_TYPE):
                    PkgPopUp<Type>::resetMenuItems();
                break;
#endif
            default:
                ERROR("fm/base/signals.hh::configureItemsMenu() ViewType %d not defined.\n", viewType);
                break;
        }
        return;
    }

    static GtkMenu *
    getViewMenu(View<Type> *view){
	gint viewType = view->viewType();
	TRACE("getViewMenu\n" );
	GtkMenu *menu;
        switch (viewType){
            case (ROOTVIEW_TYPE):
		    menu = RootPopUp<Type>::popUp();
                break;
            case (LOCALVIEW_TYPE):
		    menu = LocalPopUp<Type>::popUp();
                break;
#ifdef ENABLE_FSTAB_MODULE
            case (FSTAB_TYPE):
		    menu = FstabPopUp<Type>::popUp();
                break;
#endif
#ifdef ENABLE_PKG_MODULE
            case (PKG_TYPE):
		    menu = PkgPopUp<Type>::popUp();
                break;
#endif
            default:
		menu = RootPopUp<Type>::popUp(); 
                ERROR("fm/base/signals.hh::getViewMenu() ViewType %d not defined.\n", viewType);
                break;
        }
	g_object_set_data(G_OBJECT(menu),"view", view);
        return menu;
    }
    static void
    configureViewMenu(gint viewType){
	TRACE("configureViewMenu\n" );
        switch (viewType){
            case (ROOTVIEW_TYPE):
                    RootPopUp<Type>::resetPopup();
                break;
            case (LOCALVIEW_TYPE):
                    LocalPopUp<Type>::resetPopup();
                break;
#ifdef ENABLE_FSTAB_MODULE
            case (FSTAB_TYPE):
                    FstabPopUp<Type>::resetPopup();
                break;
#endif
#ifdef ENABLE_PKG_MODULE
            case (PKG_TYPE):
                    PkgPopUp<Type>::resetPopup();
                break;
#endif
            default:
                ERROR("fm/base/signals.hh::configureViewMenu() ViewType %d not defined.\n", viewType);
                break;
        }
        return ;
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
	if (!view->loadModel(treeModel, tpath, path)){
            TRACE("base-signals:activate():cannot load %s\n", path);
        }
	g_free(path);
    }

    // DnD highlight only:
    static void
    highlight(GtkTreePath *tpath, gpointer data){
        GtkTreeIter iter;
	auto baseModel = (BaseModel<Type> *)data;
        if (!isTreeView && baseModel->items() > 260){ 
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
            gtk_tree_path_free (tpath);
            return; 
        }     
        gchar *tree_path_string = NULL;
	if (isTreeView) {
	    gtk_tree_view_set_drag_dest_row (baseModel->treeView(),
                                 tpath,GTK_TREE_VIEW_DROP_INTO_OR_AFTER);
	}
        if (tpath == NULL){
	    //static gint count=0;
            // No item at position?
            // Do we need to clear hash table?
            TRACE("highlight clear_highlights %d\n", count++);
            clear_highlights(data);
            return;
        }
        gtk_tree_model_get_iter (baseModel->treeModel(), &iter, tpath);

        // Already highlighted?
        tree_path_string = gtk_tree_path_to_string (tpath);
	// no more use for tpath...
        gtk_tree_path_free (tpath);
	if (g_hash_table_lookup(highlight_hash, tree_path_string)) {
            //TRACE("%s already in hash\n", tree_path_string);
            g_free (tree_path_string);
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
	    Dialogs<Type>::quickHelp(mainWindow, m, "dialog-error");
	    ERROR("fm/base/signals.hh::getViewType: %s\n",m);
	    g_free(m);
	}
        if (strcmp(path, "xffm:local")==0) return (LOCALVIEW_TYPE);
        if (strcmp(path, "xffm:root")==0) return (ROOTVIEW_TYPE);
        if (strcmp(path, "xffm:fstab")==0) return (FSTAB_TYPE);
        if (strcmp(path, "xffm:nfs")==0) return (NFS_TYPE);
        if (strcmp(path, "xffm:sshfs")==0) return (SSHFS_TYPE);
        if (strcmp(path, "xffm:cifs")==0) return (CIFS_TYPE);
        if (strncmp(path, "xffm:pkg", strlen("xffm:pkg"))==0) return (PKG_TYPE);
        if (EFS<Type>::isEFS(path)) return (EFS_TYPE);
	
        ERROR("fm/base/signals.hh::getViewType() %s not defined.\n", path);
        return (-1);
    }

    /////////////////////   gsignals  /////////////////////////
/////////////////////////////////  DnD   ///////////////////////////
    static gint
    dragOffset(GtkWidget *widget){
        auto pathbar = GTK_WIDGET(g_object_get_data(G_OBJECT(widget), "pathbar"));
        GtkAllocation allocation;
        gtk_widget_get_allocation(pathbar, &allocation);
        TRACE("pathbar height = %d\n", allocation.height);
        return allocation.height;
    }
    
    static gboolean
    DragMotion (GtkWidget * widget, 
            GdkDragContext * dc, gint dragX, gint dragY, 
            guint t, gpointer data) {
	static gboolean highlighted = FALSE;
	auto view = (View<Type> *)data;
        TRACE("signal_drag_motion\n");
        gtk_widget_hide(GTK_WIDGET(xf::popupImage));

        GtkTreePath *tpath;
                                        
        gint actions = gdk_drag_context_get_actions(dc);
	TRACE("motion_notify_event, dragmode= %d\n", actions);
	const gchar *dragIcon = "text-x-generic/SE/edit-redo/4.0/220";
	
        if(actions == GDK_ACTION_MOVE){
            gdk_drag_status (dc, GDK_ACTION_MOVE, t);
	} else if(actions == GDK_ACTION_COPY){
            gdk_drag_status (dc, GDK_ACTION_COPY, t);
	    dragIcon = "text-x-generic/SE/list-add/4.0/220";
	} else if(actions == GDK_ACTION_LINK){
            gdk_drag_status (dc, GDK_ACTION_LINK, t);
	    dragIcon = "text-x-generic/SE/emblem-symbolic-link/4.0/220";
	}else{
            gdk_drag_status (dc, GDK_ACTION_MOVE, t);
	}

	GdkPixbuf *pixbuf = Pixbuf<Type>::get_pixbuf(dragIcon, -24);
	if (GDK_IS_DRAG_CONTEXT(context)) gtk_drag_set_icon_pixbuf (context, pixbuf,1,24);
            
        gboolean folderDND = FALSE;
        auto viewDragY = dragY - dragOffset(widget);
        if (viewDragY >= 0) {
            // Treeview or iconview?
            folderDND = isTreeView?
                gtk_tree_view_get_path_at_pos (view->treeView(),
                                   dragX, viewDragY, 
                                   &tpath, NULL, NULL, NULL):
                gtk_icon_view_get_dest_item_at_pos (view->iconView(),
                                            dragX,viewDragY,
                                            &tpath,
                                            NULL);
        }
	if (!folderDND) {
	    if (highlighted) {
		highlighted=FALSE;
		highlight(NULL, view);
	    }
	    return FALSE;
	}
	GtkTreeIter iter;
	gtk_tree_model_get_iter (view->treeModel(), &iter, tpath);
	gchar *g;
	gtk_tree_model_get (view->treeModel(), &iter, PATH, &g, -1);
	// drop into?
	// must be a directory (XXX this is quite local stuff...)
	if (g_file_test(g, G_FILE_TEST_IS_DIR)){
	    TRACE("%s is directory\n", g);
	    highlight(tpath, view);
	    highlighted=TRUE;
	} else if (highlighted) {
	    highlighted=FALSE;
	    highlight(NULL, view);
	}
	g_free(g);
	// tree path reference is handled in highlight function...
	    

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
        /*if (g_list_length(selectionList) > 1) {
            GdkPixbuf *pixbuf = Pixbuf<Type>::get_pixbuf("edit-copy", isTreeView?-16:-48);
            gtk_drag_set_icon_pixbuf (context, pixbuf,1,1);
        } else {
            auto tpath = (GtkTreePath *)selectionList->data;
            GtkTreeIter iter;
            GdkPixbuf *pixbuf;
            gtk_tree_model_get_iter (view->treeModel(), &iter, tpath);
            gtk_tree_model_get (view->treeModel(), &iter, NORMAL_PIXBUF, &pixbuf, -1);
            gtk_drag_set_icon_pixbuf (context, pixbuf,1,1);
        }*/
    }

//receiver:
private:
    static gchar *
    getDnDTarget(View<Type> *view, gint dragX, gint viewDragY){
	GtkTreeIter iter;
        gchar *target = NULL;
        GtkTreePath *tpath=NULL;
        if (isTreeView) {
            if (gtk_tree_view_get_path_at_pos (view->treeView(),
                               dragX, viewDragY, 
                               &tpath, NULL, NULL, NULL)){
                gtk_tree_model_get_iter (view->treeModel(), &iter, tpath);
                gtk_tree_model_get (view->treeModel(), &iter, PATH, &target, -1);
                if (g_file_test(target, G_FILE_TEST_IS_DIR)){
                    gtk_tree_view_set_drag_dest_row (view->treeView(),
                             tpath,GTK_TREE_VIEW_DROP_INTO_OR_AFTER);
                } else {
                    g_free(target);
                    target = g_strdup(view->path());
                }
            } else {
                target = g_strdup(view->path());
            }
        } else {
            if (gtk_icon_view_get_item_at_pos (view->iconView(),
                                       dragX, viewDragY, &tpath, NULL))
            {
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
        return target;
    }

public:
    static void
    DragDataReceive (GtkWidget * widget,
                      GdkDragContext * context,
                      gint dragX, gint dragY, 
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

        GtkTreeIter iter;
        auto viewDragY = dragY - dragOffset(widget);
        gchar *target = NULL;
        
        
        getDnDTarget(view, dragX, viewDragY);

        if (viewDragY < 0) {
            DBG("drop into pathbar...\n");
        } else {
            target=getDnDTarget(view, dragX, viewDragY);
        }
        if (!target){
            DBG("no DnD target found...\n");
        }
        
        
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
