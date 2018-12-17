#ifndef  XF_BASEMODEL__HH
# define XF_BASEMODEL__HH

// Flag bits:
#define IS_NOTSELECTABLE(F) ((0x01<<1)&F)
#define SET_NOTSELECTABLE(F) (F|=(0x01<<1))

enum
{
    ROOTVIEW_TYPE,
    LOCALVIEW_TYPE,
    FSTAB_TYPE,
    NFS_TYPE,
    SSHFS_TYPE,
    ECRYPTFS_TYPE,
    CIFS_TYPE,
    PKG_TYPE
};

enum
{
  FLAGS,
  TREEVIEW_PIXBUF,
  DISPLAY_PIXBUF,
  NORMAL_PIXBUF,
  HIGHLIGHT_PIXBUF,
  TOOLTIP_PIXBUF,
  DISPLAY_NAME,
  ACTUAL_NAME,
  PATH,
  SIZE,
  DATE,
  TOOLTIP_TEXT,
  ICON_NAME,
  TYPE,
  MIMETYPE, 
  PREVIEW_PATH,
  PREVIEW_TIME,
  PREVIEW_PIXBUF,
  NUM_COLS
};

enum {
    TARGET_URI_LIST,
    TARGETS
};

static GtkTargetEntry targetTable[] = {
    {(gchar *)"text/uri-list", 0, TARGET_URI_LIST},
};

#define NUM_TARGETS (sizeof(targetTable)/sizeof(GtkTargetEntry))
/* Overkill... Simple target list in baseview.hh
 * enum {
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
};*/

#define URIFILE "file://"
static GHashTable *highlight_hash=NULL;
static GHashTable *validBaseViewHash = NULL;
static gint dragMode=0;
static GList *localMonitorList = NULL;

namespace xf
{
template <class Type> class LocalMonitor;
template <class Type> class LocalView;
template <class Type> class LocalDnd;
template <class Type> class FstabMonitor;
template <class Type> class BaseViewSignals;
template <class Type> class RootView;
template <class Type> class Fstab;

template <class Type>
class BaseModel
{
    using util_c = Util<Type>;
    using page_c = Page<Type>;
    
    Page<Type> *page_;
    GtkWidget *source_;
    GtkWidget *destination_;
    gint viewType_;

protected:
    gchar *path_;
    GList *selectionList_;
    LocalMonitor<Type> *localMonitor_;
    FstabMonitor<Type> *fstabMonitor_;
    GtkTreeModel *treeModel_;
    
public:    
    BaseModel(Page<Type> *page){
	page_ = page; 
        path_ = NULL;
        selectionList_ = NULL;
        localMonitor_ = NULL;
        fstabMonitor_ = NULL;
	treeModel_ = mkTreeModel();
        if (!highlight_hash) highlight_hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
        if (!validBaseViewHash) {
	    validBaseViewHash = g_hash_table_new(g_direct_hash, g_direct_equal); 
	}
	g_hash_table_replace(validBaseViewHash,
		(void *)this, GINT_TO_POINTER(1));
        source_ = GTK_WIDGET(this->page()->pageChild());
        destination_ = GTK_WIDGET(this->page()->pageChild());
        // Enable dnd by default.
        // Local object will disable if not required.
         createSourceTargetList(source_);
        // 
        // Only enable destination drops.
        createDestTargetList(destination_);
        // Overkill?
        /*g_signal_connect (G_OBJECT (this->source()), 
                "drag-data-delete", G_CALLBACK (BaseViewSignals<Type>::signal_drag_delete), (void *)this);
*/
        g_signal_connect (G_OBJECT (this->source()), 
                "drag-failed", DRAG_CALLBACK (signal_drag_failed), (void *)this);
        g_signal_connect (G_OBJECT (this->destination()), 
                "drag-leave", G_CALLBACK (signal_drag_leave), (void *)this);
        g_signal_connect (G_OBJECT (this->source()), 
                "drag-end", DRAG_CALLBACK (signal_drag_end), (void *)this);

        // source widget
        g_signal_connect (G_OBJECT (this->source()), 
             "drag-begin", DRAG_CALLBACK (DragBegin),
             (void *)this);
        
        g_signal_connect (G_OBJECT (this->source()), 
             "drag-data-get", G_CALLBACK (DragDataSend), 
             (void *)this);
        // destination widget
        g_signal_connect (G_OBJECT (this->destination()), 
             "drag-data-received", G_CALLBACK (DragDataReceive),
             (void *)this);

        // "drag-motion" is not necessary with GTK_DEST_DEFAULT_MOTION
	// while using default iconview dnd, but this is not
	// our case. But seems to make no difference qith gtk+-3.24
        // Nonetheless, Drop targets will not be highlighted if
        // this is not set.
        g_signal_connect (G_OBJECT (this->destination()), 
             "drag-motion", 
	     G_CALLBACK (DragMotion),
	     (void *)this);
        
        // Not necessary with GTK_DEST_DEFAULT_DROP
        //
        //g_signal_connect (G_OBJECT (baseView->destination()), 
        //    "drag-drop", G_CALLBACK (DragDrop),
        //    (void *)baseView);
        

        
    }
    ~BaseModel(void){
        TRACE("BaseModel destructor.\n");
	g_hash_table_remove(validBaseViewHash, (void *)this);
        g_free(path_); 
        g_object_unref(treeModel_);
    }

    

    void setViewType(gint value){viewType_ = value;}
    gint viewType(void){ return viewType_;}
    GtkWidget *source(){ return source_;}
    GtkWidget *destination(){ return destination_;}


    gint
    keyboardEvent( GdkEventKey * event) {
        DBG("baseModel key\n");
        return TRUE;
    }

    guint
    setSelectable(gchar *name, guint flags){
	if (strcmp(name, "..")==0) return SET_NOTSELECTABLE(flags);
	return flags;
    }

    guint
    isSelectable(GtkTreePath *tpath ) {
        GtkTreeIter iter;
	guint flags;
        gtk_tree_model_get_iter (this->treeModel(), &iter, tpath);
        gtk_tree_model_get (this->treeModel(), &iter, 
                FLAGS , &flags, -1);
        return !IS_NOTSELECTABLE(flags);
    }

    
    
    void disableDnD(void){
        gtk_drag_source_unset(source_);
        gtk_drag_dest_unset(destination_);
    }
    
    void enableDnD(void){
        createSourceTargetList(source_);
        createDestTargetList(destination_);
    }

    Page<Type> *page(void){return page_;}
    const gchar *path(){return path_;}
    GtkTreeModel *treeModel(void){return treeModel_;}

    void setPath(const gchar *path){
        g_free(path_);
        if (path) path_ = g_strdup(path);
        else {
            ERROR("baseView::setPath(NULL)\n");
            exit(1);
        }

        TRACE("Baseview:: setPath()\n");
        if (g_file_test(path_, G_FILE_TEST_IS_DIR)){
            page_->setPageWorkdir(path_);
        } else {
            page_->setPageWorkdir(g_get_home_dir());
        }
    }
    gchar *
    get_window_name (void) {
        gchar *iconname;
        if(!path_) {
            iconname = util_c::utf_string (g_get_host_name());
        } else if(g_path_is_absolute(path_) &&
                g_file_test (path_, G_FILE_TEST_EXISTS)) {
            gchar *basename = g_path_get_basename (path_);
            gchar *pathname = g_strdup (path_);
            gchar *b = util_c::utf_string (basename);   // non chopped
            util_c::chop_excess (pathname);
            gchar *q = util_c::utf_string (pathname);   // non chopped

            g_free (basename);
            g_free (pathname);
            //iconname = g_strconcat (display_host, ":  ", b, " (", q, ")", NULL);
            iconname = g_strconcat (b, " (", q, ")", NULL);
            g_free (q);
            g_free (b);
        } else {
            iconname = util_c::utf_string (path_);
            util_c::chop_excess (iconname);
        }

#ifdef DEBUG
        gchar *gg = g_strdup_printf("%s-%d-D", iconname, getpid());
        g_free(iconname);
        iconname = gg;
#else
#ifdef CORE
        gchar *gg = g_strdup_printf("%s-%d-C", iconname, getpid());
        g_free(iconname);
        iconname = gg;
#endif
#endif
        return (iconname);
    }

    void
    freeSelectionList(void){
        if (selectionList_) 
            g_list_free_full (selectionList_, (GDestroyNotify) gtk_tree_path_free);
        selectionList_ = NULL;
    }
    
    void
    setSelectionList(GList *list){
        if (selectionList_) freeSelectionList();
        selectionList_ = list;
    }

    GList *
    selectionList(void){return selectionList_;}

    //////////////////////   static   ////////////////////////////

    static void
    highlight(GtkTreePath *tpath, gpointer data){
            //TRACE("highlight %d, %d\n", highlight_x, highlight_y);
        gchar *tree_path_string = NULL;
        
        if (tpath == NULL){
            // No item at position?
            // Do we need to clear hash table?
            clear_highlights(data);
            return;
        }

        // Already highlighted?
        tree_path_string = gtk_tree_path_to_string (tpath);
        if (g_hash_table_lookup(highlight_hash, tree_path_string)) {
            //TRACE("%s already in hash\n", tree_path_string);
            g_free (tree_path_string);
            gtk_tree_path_free (tpath);
            return;
        }

	auto baseModel = (BaseModel<Type> *)data;
        // Not highlighted. First clear any other item which highlight remains.
        clear_highlights(data);
        // Now do highlight dance. 
        g_hash_table_insert(highlight_hash, tree_path_string, GINT_TO_POINTER(1));
        GtkTreeIter iter;
        gtk_tree_model_get_iter (baseModel->treeModel(), &iter, tpath);
        
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
        g_hash_table_foreach_remove (highlight_hash, BaseViewSignals<Type>::unhighlight, data);
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
    
    // This mkTreeModel should be static...
    static GtkTreeModel *
    mkTreeModel (void)
    {

	GtkTreeIter iter;
	GtkListStore *list_store = gtk_list_store_new (NUM_COLS, 
	    G_TYPE_UINT,      // flags
	    GDK_TYPE_PIXBUF, // icon in treeView display
	    GDK_TYPE_PIXBUF, // icon in display
	    GDK_TYPE_PIXBUF, // normal icon reference
	    GDK_TYPE_PIXBUF, // highlight icon reference
	    GDK_TYPE_PIXBUF, // preview, tooltip image (cache)
	    G_TYPE_STRING,   // name in display (UTF-8)
	    G_TYPE_STRING,   // name from filesystem (verbatim)
	    G_TYPE_STRING,   // path (verbatim)
            G_TYPE_UINT,     // date
            G_TYPE_UINT,     // size
	    G_TYPE_STRING,   // tooltip text (cache)
	    G_TYPE_STRING,   // icon identifier (name or composite key)
	    G_TYPE_INT,      // mode (to identify directories)
	    G_TYPE_STRING,   // mimetype (further identification of files)
	    G_TYPE_STRING,   // Preview path
	    G_TYPE_UINT,      // Preview time
	    GDK_TYPE_PIXBUF  // Preview pixbuf
            ); // 
	return GTK_TREE_MODEL (list_store);
    }
    
    static gint
    getViewType(const gchar *path){
        if (!path) return ROOTVIEW_TYPE;
        if (g_file_test(path, G_FILE_TEST_EXISTS)) return (LOCALVIEW_TYPE);
        if (strcmp(path, "xffm:local")==0) return (LOCALVIEW_TYPE);
        if (strcmp(path, "xffm:root")==0) return (ROOTVIEW_TYPE);
        if (strcmp(path, "xffm:fstab")==0) return (FSTAB_TYPE);
        if (strcmp(path, "xffm:nfs")==0) return (NFS_TYPE);
        if (strcmp(path, "xffm:sshfs")==0) return (SSHFS_TYPE);
        if (strcmp(path, "xffm:ecryptfs")==0) return (ECRYPTFS_TYPE);
        if (strcmp(path, "xffm:cifs")==0) return (CIFS_TYPE);
        if (strcmp(path, "xffm:pkg")==0) return (PKG_TYPE);
        ERROR("BaseView::loadModel() %s not defined.\n", path);
        return (ROOTVIEW_TYPE);
    }

    /////////////////////   gsignals  /////////////////////////
/////////////////////////////////  DnD   ///////////////////////////
    static gboolean
    DragMotion (GtkWidget * widget, 
            GdkDragContext * dc, gint drag_x, gint drag_y, 
            guint t, gpointer data) {
	auto baseView = (BaseView<Type> *)data;
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
        gboolean isTreeView = (Settings<Type>::getSettingInteger("window", "TreeView") > 0);
        if (isTreeView) return FALSE;
        
        GtkIconViewDropPosition pos;
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
    DragDataSend (GtkWidget * widget,
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
        if (info != TARGET_URI_LIST) {
            ERROR("signal_drag_data_send: invalid target");
        }
        DBG( ">>> DND send, TARGET_URI_LIST\n"); 
        gchar *dndData = NULL;
        switch (baseView->viewType()) {
            case (LOCALVIEW_TYPE):
            {
                dndData = LocalDnd<Type>::sendDndData(baseView);
                TRACE("drag finish result=%d\n", result);
                break;
            }

            default :
                DBG("BaseViewSignals:: sendDndData not defined for view type %d\n", baseView->viewType());
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
        WARN("signal_drag_begin\n");
	auto baseView = (BaseView<Type> *)data;
        auto treeModel = baseView->treeModel();
        // Treeview or iconview?
        gboolean isTreeView = (Settings<Type>::getSettingInteger("window", "TreeView") > 0);
        //  single or multiple item selected?
        GList *selectionList;
        if (isTreeView){
            auto selection = gtk_tree_view_get_selection (baseView->treeView());
            selectionList = gtk_tree_selection_get_selected_rows (selection, &treeModel);
        } else {
            selectionList = gtk_icon_view_get_selected_items (baseView->iconView());
        }
        baseView->setSelectionList(selectionList);
        if (g_list_length(selectionList) > 1) {
            GdkPixbuf *pixbuf = Pixbuf<Type>::get_pixbuf("edit-copy", -48);
            gtk_drag_set_icon_pixbuf (context, pixbuf,10,10);
        } else {
            auto tpath = (GtkTreePath *)selectionList->data;
            GtkTreeIter iter;
            GdkPixbuf *pixbuf;
            gtk_tree_model_get_iter (baseView->treeModel(), &iter, tpath);
            gtk_tree_model_get (baseView->treeModel(), &iter, NORMAL_PIXBUF, &pixbuf, -1);
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
        DBG( "DND>> signal_drag_data\n");
	auto baseView = (BaseView<Type> *)data;

        // Treeview or iconview?
        gboolean isTreeView = (Settings<Type>::getSettingInteger("window", "TreeView") > 0);
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
        if (isTreeView) {
            // Simple drop for now since xy coordinates differ from treeview coordinates...
            target = g_strdup(baseView->path());
        } else {
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
        }
        
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


    static void
    signal_drag_end (GtkWidget * widget, GdkDragContext * context, gpointer data) {
        WARN("signal_drag_end\n");
        
	auto baseView = (BaseView<Type> *)data;

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
        ERROR("Drag was not accepted: %s\n", message);
        return TRUE;

    }


    static void
    signal_drag_leave (GtkWidget * widget, GdkDragContext * drag_context, guint time, gpointer data) {
        DBG("signal_drag_leave\n");

    }

    static void
    signal_drag_delete (GtkWidget * widget, GdkDragContext * context, gpointer data) {
        ERROR("signal_drag_delete\n");
    }
    
};
}
#endif
