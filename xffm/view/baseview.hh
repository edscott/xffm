#ifndef XF_BASEVIEW__HH
# define XF_BASEVIEW__HH


enum
{
  FLAGS,
  DISPLAY_PIXBUF,
  NORMAL_PIXBUF,
  HIGHLIGHT_PIXBUF,
  TOOLTIP_PIXBUF,
  DISPLAY_NAME,
  ACTUAL_NAME,
  PATH,
  TOOLTIP_TEXT,
  ICON_NAME,
  TYPE,
  MIMETYPE, 
  MIMEFILE, 
  ST_DATA,
  PREVIEW_PATH,
  PREVIEW_TIME,
  PREVIEW_PIXBUF,
  NUM_COLS
};

#include "view/rootview.hh"
#include "view/local/localview.hh"
#include "view/local/localmonitor.hh"
#include "signals/baseview.hh"

// Flag bits:
#define IS_NOTSELECTABLE(F) ((0x01<<1)&F)
#define SET_NOTSELECTABLE(F) (F|=(0x01<<1))

namespace xf
{

template <class Type>
class BaseView{
    using util_c = Util<Type>;
    using page_c = Page<Type>;
    GList *selectionList_;
    
    gint display_pixbuf_column_;
    gint normal_pixbuf_column_;
    gint actual_name_column_;


    GtkTreeModel *treeModel_;
    //GtkTreeModelSort *sortModel_;
    gint dirCount_; 
    GtkIconView *iconView_;
    
    GtkWidget *source_;
    GtkWidget *destination_;

    gchar *path_;
    page_c *page_;

    LocalMonitor<Type> *localMonitor_;

/*    static gint nameCompare(GtkTreeModel *model,
                           GtkTreeIter *a,
                           GtkTreeIter *b,
                           gpointer data){
        gchar *aName;
        gchar *bName;
	gtk_tree_model_get (model, a, ACTUAL_NAME, &aName, -1);
	gtk_tree_model_get (model, b, ACTUAL_NAME, &bName, -1);
        if (aName == NULL && bName == NULL) return 0;
        if (aName == NULL) {
            g_free(bName);
            return -1;
        }
        if (bName == NULL){
            g_free(aName);
            return 1;
        }
        gint result = strcmp(aName, bName);
        g_free(aName);
        g_free(bName);
        return result;
    }*/

    // This mkTreeModel should be static...
    static GtkTreeModel *
    mkTreeModel (void)
    {

	GtkTreeIter iter;
	GtkListStore *list_store = gtk_list_store_new (NUM_COLS, 
	    G_TYPE_UINT,      // flags
	    GDK_TYPE_PIXBUF, // icon in display
	    GDK_TYPE_PIXBUF, // normal icon reference
	    GDK_TYPE_PIXBUF, // highlight icon reference
	    GDK_TYPE_PIXBUF, // preview, tooltip image (cache)
	    G_TYPE_STRING,   // name in display (UTF-8)
	    G_TYPE_STRING,   // name from filesystem (verbatim)
	    G_TYPE_STRING,   // path (verbatim)
	    G_TYPE_STRING,   // tooltip text (cache)
	    G_TYPE_STRING,   // icon identifier (name or composite key)
	    G_TYPE_INT,      // mode (to identify directories)
	    G_TYPE_STRING,   // mimetype (further identification of files)
	    G_TYPE_STRING,   // mimefile (further identification of files)
	    G_TYPE_POINTER,  // stat record or NULL 
	    G_TYPE_STRING,   // Preview path
	    G_TYPE_INT,      // Preview time
	    GDK_TYPE_PIXBUF  // Preview pixbuf
            ); // 
	return GTK_TREE_MODEL (list_store);
        /*GtkTreeSortable *sortModel = GTK_TREE_SORTABLE(gtk_tree_model_sort_new_with_model (GTK_TREE_MODEL(list_store)));		
        gtk_tree_sortable_set_sort_column_id (sortModel,ACTUAL_NAME, GTK_SORT_ASCENDING);
        gtk_tree_sortable_set_sort_func (sortModel,
                                 ACTUAL_NAME,
                                 nameCompare,
                                 NULL,
                                 NULL);
	return GTK_TREE_MODEL (sortModel);*/
    }

public:
    page_c *page(void){return page_;}

    
    BaseView(page_c *page, const gchar *path){
	page_ = page; 
        path_ = NULL;
        selectionList_ = NULL;
        localMonitor_ = NULL;
        
        iconView_=createIconview();

        source_ = GTK_WIDGET(page_->pageChild());
        //source_ = GTK_WIDGET(page_->top_scrolled_window());
        //source_ = GTK_WIDGET(iconView_);
        destination_ = GTK_WIDGET(iconView_);
	
        //sortModel_ = GTK_TREE_MODEL_SORT(mkTreeModel());
	//treeModel_ = gtk_tree_model_sort_get_model(sortModel_);
	treeModel_ = mkTreeModel();
        // Enable dnd by default.
        // Local object will disable if not required.
         createSourceTargetList(source_);
         //createSourceTargetList();
        // 
        // Only enable destination drops.
        //createDestTargetList();
        createDestTargetList(destination_);
        
	g_object_set_data(G_OBJECT(treeModel_), "iconview", iconView_);
	gtk_icon_view_set_model(iconView_, treeModel_);

	gtk_icon_view_set_text_column (iconView_, DISPLAY_NAME);
	gtk_icon_view_set_pixbuf_column (iconView_,  DISPLAY_PIXBUF);
	gtk_icon_view_set_selection_mode (iconView_, GTK_SELECTION_SINGLE);


        if (!highlight_hash) highlight_hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
        g_signal_connect (this->iconView_, "item-activated", 
            ICONVIEW_CALLBACK (BaseViewSignals<Type>::item_activated), (void *)this);
        g_signal_connect (this->iconView_, "motion-notify-event", 
                ICONVIEW_CALLBACK (BaseViewSignals<Type>::motion_notify_event), (void *)this);
         //g_signal_connect (this->iconView_, "query-tooltip", 
           //     G_CALLBACK (query_tooltip_f), (void *)this);

         // Why not "clicked" signal? 
         // Because this is to filter cancelled dnd event from
         // actual activated events.
         g_signal_connect (this->iconView_, "button-release-event",
                G_CALLBACK(BaseViewSignals<Type>::button_click_f), (void *)this);
         g_signal_connect (this->iconView_, "button-release-event",
                G_CALLBACK(BaseViewSignals<Type>::button_release_f), (void *)this);
         g_signal_connect (this->iconView_, "button-press-event",
                G_CALLBACK(BaseViewSignals<Type>::button_press_f), (void *)this);

        // source widget
        g_signal_connect (G_OBJECT (source_), 
                "drag-begin", DRAG_CALLBACK (BaseViewSignals<Type>::signal_drag_begin), (void *)this);
        g_signal_connect (G_OBJECT (source_), 
                "drag-data-get", G_CALLBACK (BaseViewSignals<Type>::signal_drag_data_send), (void *)this);
        g_signal_connect (G_OBJECT (source_), 
                "drag-end", DRAG_CALLBACK (BaseViewSignals<Type>::signal_drag_end), (void *)this);
      
        // destination widget
        g_signal_connect (G_OBJECT (destination_), 
                "drag-data-received", G_CALLBACK (BaseViewSignals<Type>::signal_drag_data_receive), (void *)this);
        g_signal_connect (G_OBJECT (destination_), 
                "drag-leave", G_CALLBACK (BaseViewSignals<Type>::signal_drag_leave), (void *)this);

        // Not necessary with GTK_DEST_DEFAULT_MOTION
        //g_signal_connect (G_OBJECT (destination_), 
        //      "drag-motion", G_CALLBACK (BaseViewSignals<Type>::signal_drag_motion), (void *)this);
        // Not necessary with GTK_DEST_DEFAULT_DROP
        //
        //g_signal_connect (G_OBJECT (destination_), 
        //        "drag-drop", G_CALLBACK (BaseViewSignals<Type>::signal_drag_drop), (void *)this);
        
        // Overkill?
        /*g_signal_connect (G_OBJECT (source_), 
                "drag-data-delete", G_CALLBACK (BaseViewSignals<Type>::signal_drag_delete), (void *)this);

        g_signal_connect (G_OBJECT (source_), 
                "drag-failed", DRAG_CALLBACK (BaseViewSignals<Type>::signal_drag_failed), (void *)this);*/

        loadModel(path);
            
    }

    ~BaseView(void){
        TRACE("BaseView destructor.\n");
        g_free(path_); 
        g_object_unref(treeModel_);
    }


    GtkWidget *source(){ return source_;}
    
    void selectables(void){
        auto iconViewType = (const gchar *)g_object_get_data(G_OBJECT(this->iconView()), "iconViewType");
        
        if (strcmp("LocalView", iconViewType) == 0){
            LocalView<Type>::selectables(this->iconView());        
        } else {
            WARN("All icons are selectable for iconViewType: %s\n", iconViewType);
        }
        return;
    }

    const gchar *path(){return path_;}

    gboolean loadModel(const gchar *path){
        if (!path) path = "xffm:root";
        setPath(path);

	// Remove previous liststore rows, if any
    
        gboolean result;
        if (g_file_test(path, G_FILE_TEST_EXISTS)){
	    if (g_file_test(path, G_FILE_TEST_IS_DIR)){
		// stop current monitor
		if (localMonitor_) {
                    delete (localMonitor_);
                    localMonitor_ = NULL;
                }
		result = LocalView<Type>::loadModel(iconView_, path);
		if (!result){
		    ERROR("baseview.hh:loadModel: cannot load view for %s\n", path);
		} else {
		    // start new monitor
                    gint items = 0;
                    GtkTreeIter iter;
                    if (gtk_tree_model_get_iter_first (treeModel_, &iter)) {
                        items++;
                        while (gtk_tree_model_iter_next(treeModel_, &iter)) items++;
                    }
                    WARN("FIXME: Set filecount %d message in status button...\n", items);

                    

                    if (items <= 500) {
		        localMonitor_ = new(LocalMonitor<Type>)(treeModel_, path);
		        localMonitor_->start_monitor(treeModel_, path);
                    } else {
                        localMonitor_ = NULL;
                    }
		}
	    } else {
		result = LocalView<Type>::item_activated(this->iconView(), path);
	    }
            return result;
        } else if (!strcmp(path, "xffm:root")==0) {
           ERROR("baseview.hh::loadModel(): unknown path \"%s\"\n", path);
        }
        setPath("xffm:root");
        result = RootView<Type>::loadModel(iconView_);

        return result;
    }

    gint
    keyboardEvent( GdkEventKey * event) {
        DBG("baseView key\n");
        return TRUE;
    }

    GtkIconView *iconView(void){return iconView_;}
    GtkTreeModel *treeModel(void){return treeModel_;}
    //GtkTargetList *getTargetList(void){return targetList_;}
   GtkTreeModel *
    get_tree_model (void){return treeModel_;}

    void
    freeSelectionList(void){
        if (selectionList_) 
            g_list_free_full (selectionList_, (GDestroyNotify) gtk_tree_path_free);
        selectionList_ = NULL;
    }

    void
    clear_highlights(void){
        if (!highlight_hash || g_hash_table_size(highlight_hash) == 0) return;
        g_hash_table_foreach_remove (highlight_hash, BaseViewSignals<Type>::unhighlight, (void *)this);
    }

    void 
    highlight(gdouble X, gdouble Y){
        //if (!xfdir_p) return; // avoid race condition here.
        //highlight_x = X; highlight_y = Y;
        GtkTreeIter iter;
        
        GtkTreePath *tpath = gtk_icon_view_get_path_at_pos (iconView_, X, Y); 
        if (tpath) {
            highlight(tpath);
            //xfdir_p->tooltip(iconview_, gtk_tree_path_copy(tpath));
        }
        else clear_highlights();
    }

    
    void
    setSelectionList(GList *list){
        if (selectionList_) freeSelectionList();
        selectionList_ = list;
    }

    GList *
    selectionList(void){return selectionList_;}
#if 0
//#define format ""
#define format "file:/"
    gboolean
    setDndData(GtkSelectionData *selection_data, GList *selection_list){
        WARN( "setDndData() baseview default.\n");
        gchar *dndData = NULL;
        for(GList *tmp = selection_list; tmp; tmp = tmp->next) {
            GtkTreePath *tpath = (GtkTreePath *)tmp->data;
            gchar *path;
            GtkTreeIter iter;
            gtk_tree_model_get_iter (this->treeModel_, &iter, tpath);
            gtk_tree_model_get (this->treeModel_, &iter, PATH, &path, -1);
            if (!dndData) dndData = g_strconcat(format, path, NULL);
            else {
                gchar *e = g_strconcat(dndData, "\n", format, path, NULL);
                g_free(dndData);
                dndData = e;
            }
            WARN("dndData: \"%s\"\n", dndData);
            WARN("append: %s -> \"%s\"\n", path, dndData);
            g_free(path);
        }

        gtk_selection_data_set (selection_data, 
	    gtk_selection_data_get_selection(selection_data),
	    8, (const guchar *)dndData, strlen(dndData)+1);

       return TRUE;
        
    }
#endif

    gboolean
    receiveDndData(gchar *target, const GtkSelectionData *selection_data, GdkDragAction action){
        WARN("BaseView::receiveDndData\n");
        if (!selection_data) {
            WARN("!selection_data\n");
            return FALSE;
        }
        auto dndData = (const char *)gtk_selection_data_get_data (selection_data);

        gchar **files = g_strsplit(dndData, "\n", -1);
        
        if (!files) {
            WARN("!files\n");
            return FALSE;
        }
        if (*files==NULL) {
            WARN("files==NULL\n");
            g_strfreev(files);
            return FALSE;
        }

        gchar *source = g_path_get_dirname(*files);
        if (!target){
            if (strncmp(source, format, strlen(format))==0){
                target = g_strconcat(format, path_, NULL);
            } else target = g_strdup(path_);
        }
        WARN("BaseView::receiveDndData: source=%s target=%s action=%d\n", source, target, action);
        gboolean result = FALSE;
        if (strcmp(source, target) ) result = TRUE;
        else {
            WARN("receiveDndData: source and target are the same\n");
            return FALSE;
        }

        for (gchar **f = files; f && *f; f++){
            if (strlen(*f)==0) continue;
            gchar *src = *f;
            if (strncmp(src, format, strlen(format))==0) src += strlen(format);
            switch (action){
                case GDK_ACTION_MOVE:
                    WARN("DND move: %s --> %s\n", src, path_);
                    break;
                case GDK_ACTION_COPY:
                    WARN("DND copy: %s --> %s\n", src, path_);
                    break;
                case GDK_ACTION_LINK:
                    WARN("DND link: %s --> %s\n", src, path_);
                    break;
            }

        }
        g_strfreev(files);
        g_free(target);
        // not needed with GTK_DEST_DEFAULT_DROP
        /*gtk_drag_finish (context, result, 
                (action == GDK_ACTION_MOVE) ? result : FALSE, 
                time);*/


        return result;
    }

    void
    highlight(GtkTreePath *tpath){
            //TRACE("highlight %d, %d\n", highlight_x, highlight_y);
        gchar *tree_path_string = NULL;
        
        if (tpath == NULL){
            // No item at position?
            // Do we need to clear hash table?
            clear_highlights();
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

        // Not highlighted. First clear any other item which highlight remains.
        clear_highlights();
        // Now do highlight dance. 
        g_hash_table_insert(highlight_hash, tree_path_string, GINT_TO_POINTER(1));
        GtkTreeIter iter;
        gtk_tree_model_get_iter (treeModel_, &iter, tpath);
        
        GdkPixbuf *highlight_pixbuf;
        gtk_tree_model_get (treeModel_, &iter, 
                HIGHLIGHT_PIXBUF, &highlight_pixbuf, -1);
        gtk_list_store_set (GTK_LIST_STORE(treeModel_), &iter,
                DISPLAY_PIXBUF, highlight_pixbuf, 
                -1);
        return;
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
        gtk_tree_model_get_iter (treeModel_, &iter, tpath);
        gtk_tree_model_get (treeModel_, &iter, 
                FLAGS , &flags, -1);
        return !IS_NOTSELECTABLE(flags);
    }


private:

    void setPath(const gchar *path){
        g_free(path_);
        if (path) path_ = g_strdup(path);
        else {
            ERROR("baseView::setPath(NULL)\n");
            exit(1);
        }
        auto lastPath =  g_object_get_data(G_OBJECT(iconView_), "path");
        g_free(lastPath); 
        g_object_set_data(G_OBJECT(iconView_), "path", g_strdup(path_));
        if (g_file_test(path_, G_FILE_TEST_IS_DIR)){
            page_->setPageWorkdir(path_);
        } else {
            page_->setPageWorkdir(g_get_home_dir());
        }
    }
    gint get_dir_count(void){ return dirCount_;}


    
    gchar *
    make_tooltip_text (GtkTreePath *tpath ) {
        return g_strdup("tooltip_text not defined in treeModel_!\n");
    }

    gchar *
    get_verbatim_name (GtkTreePath *tpath ) {
        GtkTreeIter iter;
        gchar *verbatim_name=NULL;
        gtk_tree_model_get_iter (treeModel_, &iter, tpath);
        gtk_tree_model_get (treeModel_, &iter, 
                ACTUAL_NAME, &verbatim_name, -1);
        return verbatim_name;
    }

    GdkPixbuf *
    get_normal_pixbuf (GtkTreePath *tpath ) {
        GtkTreeIter iter;
        GdkPixbuf *pixbuf=NULL;
        gtk_tree_model_get_iter (treeModel_, &iter, tpath);
        gtk_tree_model_get (treeModel_, &iter, 
                NORMAL_PIXBUF , &pixbuf, -1);
        return pixbuf;
    }

    GdkPixbuf *
    get_tooltip_pixbuf (GtkTreePath *tpath ) {
        GtkTreeIter iter;
        GdkPixbuf *pixbuf=NULL;
        gtk_tree_model_get_iter (treeModel_, &iter, tpath);
        gtk_tree_model_get (treeModel_, &iter, 
                TOOLTIP_PIXBUF, &pixbuf, -1);
        return pixbuf;
    }

    gchar *
    get_tooltip_text (GtkTreePath *tpath ) {
        GtkTreeIter iter;
        gchar *text=NULL;
        gtk_tree_model_get_iter (treeModel_, &iter, tpath);
        gtk_tree_model_get (treeModel_, &iter, 
                TOOLTIP_TEXT, &text, -1);
        return text;
    }



    void
    set_tooltip_pixbuf (GtkTreePath *tpath, GdkPixbuf *pixbuf ) {
        GtkTreeIter iter;
        gtk_tree_model_get_iter (treeModel_, &iter, tpath);
        gtk_list_store_set (GTK_LIST_STORE(treeModel_), &iter,
                TOOLTIP_PIXBUF, pixbuf, 
            -1);

        return ;
    }


    void
    set_tooltip_text (GtkTreePath *tpath, const gchar *text ) {
        GtkTreeIter iter;
        gtk_tree_model_get_iter (treeModel_, &iter, tpath);
        gtk_list_store_set (GTK_LIST_STORE(treeModel_), &iter,
                TOOLTIP_TEXT, text, 
            -1);

        return ;
    }
    const gchar *
    get_label(void){
        return get_path();
    }

    const gchar *
    get_path(void){return (const gchar *)path_;}

    gint 
    get_icon_highlight_size(const gchar *name){
        return GTK_ICON_SIZE_DIALOG;
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
    createSourceTargetList (void) {
        DBG("createSourceTargetList..\n");
        gtk_icon_view_enable_model_drag_source
                                   (iconView_,
                                    (GdkModifierType) 0,
                                    targetTable,
                                    NUM_TARGETS,
                                    (GdkDragAction)
                                    ((gint)GDK_ACTION_MOVE|
                                     (gint)GDK_ACTION_COPY|
                                     (gint)GDK_ACTION_LINK));
        return;
    }

    void
    createSourceTargetList (GtkWidget *widget) {
        DBG("createSourceTargetList..\n");
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

    void
    createDestTargetList (GtkWidget *widget) {
        DBG("createDestTargetList..\n");
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

    void
    createDestTargetList (void) {
        DBG("createDestTargetList..\n");
        //if(target_list) return;
        //targetList_ = gtk_target_list_new (targetTable, NUM_TARGETS);
        // The default dnd action: move.

        gtk_icon_view_enable_model_drag_dest (iconView_,
                                          targetTable, 
                                          NUM_TARGETS,
                                          (GdkDragAction)
                                    ((gint)GDK_ACTION_MOVE|
                                     (gint)GDK_ACTION_COPY|
                                     (gint)GDK_ACTION_LINK));
        return;
    }
 
    static GtkIconView *createIconview(void){
        auto icon_view = GTK_ICON_VIEW(gtk_icon_view_new());
        g_object_set(G_OBJECT(icon_view), "has-tooltip", TRUE, NULL);
        gtk_icon_view_set_item_width (icon_view, 60);
        gtk_icon_view_set_activate_on_single_click(icon_view, TRUE);
        return icon_view;
    }
    
    
};
}
#endif
