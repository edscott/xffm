#ifndef XF_BASEVIEW__HH
# define XF_BASEVIEW__HH

#include "baseviewsignals.hh"

namespace xf
{

template <class Type>
class BaseView{
    using util_c = Util<Type>;
    gint dragMode_;
    gint clickCancel_;
    GList *selectionList_;
    
    gint display_pixbuf_column_;
    gint normal_pixbuf_column_;
    gint actual_name_column_;

    //GtkTargetList *targetList_;
public:
    void init(const gchar *path){
        if (path) path = g_strdup(path);
        else path_ = NULL;
        dragMode_ = 0;
        clickCancel_ = 0;
        selectionList_ = NULL;
        
	normal_pixbuf_column_ = Type::normalPixbufC();
	display_pixbuf_column_ = Type::iconColumn();
	actual_name_column_ = Type::actualNameColumn();
	
        iconView_=createIconview();
        createSourceTargetList();
        createDestTargetList();
	
        if (!Type::enableDragSource()){
            gtk_icon_view_unset_model_drag_source (iconView_);
        }
        if (!Type::enableDragDest()){
            gtk_icon_view_unset_model_drag_dest (iconView_);
        }
	treeModel_ = Type::mkTreeModel(path);
        
	g_object_set_data(G_OBJECT(treeModel_), "iconview", iconView_);
	gtk_icon_view_set_model(iconView_, treeModel_);

	gtk_icon_view_set_text_column (iconView_, Type::textColumn());
	gtk_icon_view_set_pixbuf_column (iconView_,  iconColumn());
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

    g_signal_connect (G_OBJECT (this->iconView_), 
	    "drag-data-received", G_CALLBACK (BaseViewSignals<Type>::signal_drag_data), (void *)this);
    g_signal_connect (G_OBJECT (this->iconView_), 
	    "drag-data-get", G_CALLBACK (BaseViewSignals<Type>::signal_drag_data_get), (void *)this);
    g_signal_connect (G_OBJECT (this->iconView_), 
	    "drag-motion", G_CALLBACK (BaseViewSignals<Type>::signal_drag_motion), (void *)this);
    g_signal_connect (G_OBJECT (this->iconView_), 
	    "drag-leave", G_CALLBACK (BaseViewSignals<Type>::signal_drag_leave), (void *)this);
    g_signal_connect (G_OBJECT (this->iconView_), 
	    "drag-data-delete", G_CALLBACK (BaseViewSignals<Type>::signal_drag_delete), (void *)this);

    g_signal_connect (G_OBJECT (this->iconView_), 
	    "drag-end", DRAG_CALLBACK (BaseViewSignals<Type>::signal_drag_end), (void *)this);
    g_signal_connect (G_OBJECT (this->iconView_), 
	    "drag-begin", DRAG_CALLBACK (BaseViewSignals<Type>::signal_drag_begin), (void *)this);
        
    }
    BaseView(void){
        init(NULL);
    }

    BaseView(const gchar *path){
        init(path);
    }

    ~BaseView(void){
        // segfault:
        // g_free(path_); 
        //g_object_unref(treeModel_);
    }

    GtkIconView *iconView(void){return iconView_;}
    GtkTreeModel *treeModel(void){return treeModel_;}
    //GtkTargetList *getTargetList(void){return targetList_;}

protected:

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
                actualNameColumn(), &verbatim_name, -1);
        return verbatim_name;
    }


    GdkPixbuf *
    get_normal_pixbuf (GtkTreePath *tpath ) {
        GtkTreeIter iter;
        GdkPixbuf *pixbuf=NULL;
        gtk_tree_model_get_iter (treeModel_, &iter, tpath);
        gtk_tree_model_get (treeModel_, &iter, 
                Type::normalPixbufC()   , &pixbuf, -1);
        return pixbuf;
    }

    GdkPixbuf *
    get_tooltip_pixbuf (GtkTreePath *tpath ) {
        GtkTreeIter iter;
        GdkPixbuf *pixbuf=NULL;
        gtk_tree_model_get_iter (treeModel_, &iter, tpath);
        gtk_tree_model_get (treeModel_, &iter, 
                Type::tooltipPixbufC(), &pixbuf, -1);
        return pixbuf;
    }

    gchar *
    get_tooltip_text (GtkTreePath *tpath ) {
        GtkTreeIter iter;
        gchar *text=NULL;
        gtk_tree_model_get_iter (treeModel_, &iter, tpath);
        gtk_tree_model_get (treeModel_, &iter, 
                Type::tooltipTextC(), &text, -1);
        return text;
    }



    void
    set_tooltip_pixbuf (GtkTreePath *tpath, GdkPixbuf *pixbuf ) {
        GtkTreeIter iter;
        gtk_tree_model_get_iter (treeModel_, &iter, tpath);
        gtk_list_store_set (GTK_LIST_STORE(treeModel_), &iter,
                Type::tooltipPixbufC(), pixbuf, 
            -1);

        return ;
    }


    void
    set_tooltip_text (GtkTreePath *tpath, const gchar *text ) {
        GtkTreeIter iter;
        gtk_tree_model_get_iter (treeModel_, &iter, tpath);
        gtk_list_store_set (GTK_LIST_STORE(treeModel_), &iter,
                Type::tooltipTextC(), text, 
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

    /////////////////////////////////////////////////////////////////////////////////

 
   /* void 
    highlight(void){
        highlight(highlight_x, highlight_y);
    }*/

    void
    createSourceTargetList (void) {
        DBG("createSourceTargetList..\n");
        gtk_icon_view_enable_model_drag_source
                                   (iconView_,
                                    (GdkModifierType)
                                    0,
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


    

protected:

    gchar *path_;
    GtkTreeModel *treeModel_;
    gint dirCount_; 
    GtkIconView *iconView_;

public:
    GtkTreeModel *
    get_tree_model (void){return treeModel_;}

    gint
    normalPixbufC(void){return normal_pixbuf_column_;}
    gint 
    iconColumn(void){ return display_pixbuf_column_;}
    gint 
    actualNameColumn(void){ return actual_name_column_;}

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

    gboolean 
    clickCancel(void){
        if (clickCancel_ <= 0) return FALSE;
        return TRUE;
    }
    
    void setDragMode(gboolean state){ dragMode_ = state;};
    gint dragMode(void){return dragMode_;}


    void
    setSelectionList(GList *list){
        if (selectionList_) freeSelectionList();
        selectionList_ = list;
    }

    GList *
    selectionList(void){return selectionList_;}


    void setClickCancel(gint state){ 
        clickCancel_ = state;
        /*if (state <= 0) {
            click_cancel = state;
            return;
        }
        if (click_cancel == 0) click_cancel = state;*/
    }

    gboolean
    setDndData(GtkSelectionData * selection_data, GList *selection_list){
        WARN( "set_dnd_data() baseview default.\n");
        GtkTreeModel *treemodel = this->treeModel_;
        GList *tmp;
        const gchar *format = "file://";
        gint selection_len = 0;
        /* count length of bytes to be allocated */
        for(tmp = selection_list; tmp; tmp = tmp->next) {
            GtkTreePath *tpath = (GtkTreePath *)tmp->data;
            gchar *g;
            GtkTreeIter iter;
            gtk_tree_model_get_iter (treemodel, &iter, tpath);
            gtk_tree_model_get (treemodel, &iter, actualNameColumn(), &g, -1);
            // 2 is added for the \r\n 
            selection_len += (strlen (g) + strlen (format) + 2);
            g_free(g);
           /* 
            gchar *dndpath = g_build_filename(view_path, g, NULL);
            g_free(g);
            // 2 is added for the \r\n 
            selection_len += (strlen (dndpath) + strlen (format) + 2);
            g_free(dndpath);
            */
        }
        /* 1 is added for terminating null character */
        fprintf(stderr, "allocating %d bytes for dnd data\n",selection_len + 1);
        //files = (gchar *)calloc (selection_len + 1,1);
        /*if (!files) {
            g_error("signal_drag_data_get(): malloc %s", strerror(errno));
            return;
        }*/
        gchar *files = g_strdup("");
        for(tmp = selection_list; tmp; tmp = tmp->next) {
            GtkTreePath *tpath = (GtkTreePath *)tmp->data;
            gchar *g;
            GtkTreeIter iter;
            gtk_tree_model_get_iter (treemodel, &iter, tpath);
            gtk_tree_model_get (treemodel, &iter, 
                actualNameColumn(), &g, -1); 

            gchar *gg=g_strconcat(files,format,g,"\n", NULL);
            g_free(g);
            g_free(files);
            files=gg;
/*
            gchar *dndpath = g_build_filename(view_path, g, NULL);
            g_free(g);
            g=g_strconcat(files,format,dndpath,"\n", NULL);
            g_free(files);
            files=g;
*/
            /*sprintf (files, "%s%s\r\n", format, dndpath);
            files += (strlen (format) + strlen (dndpath) + 2);
            g_free(dndpath);*/
        }
        gtk_selection_data_set (selection_data, 
            gtk_selection_data_get_selection(selection_data),
            8, (const guchar *)files, selection_len);
        DBG(">>> DND send, drag data is:\n%s\n", files);
        g_free(files);
        return TRUE;
    }

    gboolean
    receive_dnd(const gchar *target, GtkSelectionData *data, GdkDragAction action){
        WARN("receive_dnd() not define for this class.\n");
        return FALSE;
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
                Type::highlightPixbufC(), &highlight_pixbuf, -1);
        gtk_list_store_set (GTK_LIST_STORE(treeModel_), &iter,
                Type::iconColumn(), highlight_pixbuf, 
                -1);
        return;
    }



private: 
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
