#ifndef  XF_BASEMODEL__HH
# define XF_BASEMODEL__HH
#include "signals.hh"
#include "popup.hh"


namespace xf
{
template <class Type> class View;
template <class Type> class LocalMonitor;
template <class Type> class FstabMonitor;

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
	treeModel_ = BaseSignals<Type>::mkTreeModel();
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
        BaseSignals<Type>::createSourceTargetList(source_);
        // 
        // Only enable destination drops.
        BaseSignals<Type>::createDestTargetList(destination_);
        // Overkill?
        /*g_signal_connect (G_OBJECT (this->source()), 
                "drag-data-delete", G_CALLBACK (BaseViewSignals<Type>::signal_drag_delete), (void *)this);
*/
        g_signal_connect (G_OBJECT (this->source()), 
                "drag-failed", DRAG_CALLBACK (BaseSignals<Type>::signal_drag_failed), (void *)this);
        g_signal_connect (G_OBJECT (this->destination()), 
                "drag-leave", G_CALLBACK (BaseSignals<Type>::signal_drag_leave), (void *)this);
        g_signal_connect (G_OBJECT (this->source()), 
                "drag-end", DRAG_CALLBACK (BaseSignals<Type>::signal_drag_end), (void *)this);

        // source widget
        g_signal_connect (G_OBJECT (this->source()), 
             "drag-begin", DRAG_CALLBACK (BaseSignals<Type>::DragBegin),
             (void *)this);
        
        g_signal_connect (G_OBJECT (this->source()), 
             "drag-data-get", G_CALLBACK (BaseSignals<Type>::DragDataSend), 
             (void *)this);
        // destination widget
        g_signal_connect (G_OBJECT (this->destination()), 
             "drag-data-received", G_CALLBACK (BaseSignals<Type>::DragDataReceive),
             (void *)this);

        // "drag-motion" is not necessary with GTK_DEST_DEFAULT_MOTION
	// while using default iconview dnd, but this is not
	// our case. But seems to make no difference qith gtk+-3.24
        // Nonetheless, Drop targets will not be highlighted if
        // this is not set.
        g_signal_connect (G_OBJECT (this->destination()), 
             "drag-motion", 
	     G_CALLBACK (BaseSignals<Type>::DragMotion),
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
        TRACE("baseModel key\n");
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
        BaseSignals<Type>::createSourceTargetList(source_);
        BaseSignals<Type>::createDestTargetList(destination_);
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
};
}
#endif
