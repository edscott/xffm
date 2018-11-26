#ifndef XF_BASEMONITOR__HH
# define XF_BASEMONITOR__HH
namespace xf
{
template <class Type>
class BaseMonitor {
protected:
    GCancellable *cancellable_;
    GFile *gfile_;
    GFileMonitor *monitor_;
    GHashTable *itemsHash_;
    GtkListStore *store_;
    BaseView<Type> *baseView_;
        
    gint
    countItems(void){
        GtkTreeIter iter;
        gint items = 0;
        if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL(store_), &iter)) {
            while (gtk_tree_model_iter_next(GTK_TREE_MODEL(store_), &iter)) items++;
        }
        return items;
    }
    void updateFileCountLabel(void){
        if (!BaseView<Type>::validBaseView(baseView())) return;
        auto items = countItems();
        auto fileCount = g_strdup_printf("%0d", items);
        auto text = g_strdup_printf(_("Files: %s"), fileCount); 
        baseView()->page()->updateStatusLabel(text);
        g_free(text);
        g_free(fileCount);
    }

public:    
    BaseMonitor(GtkTreeModel *treeModel, BaseView<Type> *baseView){
        baseView_ = baseView;
        gboolean showHidden = (Settings<Type>::getSettingInteger("LocalView", "ShowHidden") > 0);
        itemsHash_ = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
        cancellable_ = g_cancellable_new ();
        gfile_ = NULL;
        store_ = GTK_LIST_STORE(treeModel);
        monitor_ = NULL;
        
    }
    ~BaseMonitor(void){
        TRACE("Destructor:~local_monitor_c()\n");
        //g_cancellable_cancel (cancellable);
        //g_object_unref(cancellable);
        if (monitor_) {
            stop_monitor();
            g_hash_table_destroy(itemsHash_);
            g_object_unref(monitor_);
        }
        if (gfile_) g_object_unref(gfile_);
    }
    
    BaseView<Type> *baseView(void) { return baseView_;}

private:
    static gboolean
    add2hash (GtkTreeModel *model,
                            GtkTreePath *path,
                            GtkTreeIter *iter,
                            gpointer data){
        auto hash = (GHashTable *)data;
        gchar *name;
	gtk_tree_model_get (model, iter, ACTUAL_NAME, &name, -1);  
        g_hash_table_replace(hash, name, GINT_TO_POINTER(1));
        return FALSE;
    }

public:
    void
    startMonitor(GtkTreeModel *treeModel, const gchar *path, void *monitor_f){
        // add all initial items to hash
        gtk_tree_model_foreach (treeModel, add2hash, (void *)itemsHash_);
        store_ = GTK_LIST_STORE(treeModel);
        TRACE( "*** start_monitor: %s\n", path);
        if (gfile_) g_object_unref(gfile_);
        gfile_ = g_file_new_for_path (path);
        GError *error=NULL;
        if (monitor_) g_object_unref(monitor_);
        monitor_ = g_file_monitor (gfile_, G_FILE_MONITOR_WATCH_MOVES, cancellable_,&error);
        if (error){
            ERROR("g_file_monitor_directory(%s) failed: %s\n",
                    path, error->message);
            g_object_unref(gfile_);
            gfile_=NULL;
            return;
        }
        g_signal_connect (monitor_, "changed", 
                G_CALLBACK (monitor_f), (void *)this);
    }

    void 
    stop_monitor(void){
        if (!monitor_) {
            WARN("no monitor to stop\n");
            return;
        }
        if (gfile_) {
	    gchar *p = g_file_get_path(gfile_);
	    TRACE("*** stop_monitor at: %s\n", p);
	    g_free(p);
        }
	g_file_monitor_cancel(monitor_);
	while (gtk_events_pending())gtk_main_iteration();  
	g_hash_table_remove_all(itemsHash_);
	// hash table remains alive (but empty) until destructor.
    }

protected:
    gboolean 
    remove_item(const gchar *basename){
        // find the iter and remove iteam
        TRACE("remove item...\n");
        g_hash_table_remove(itemsHash_, basename); 
        gtk_tree_model_foreach (GTK_TREE_MODEL(store_), rm_func, (gpointer) basename); 
        return TRUE;
    }
    gboolean 
    remove_item(GFile *file){
        // find the iter and remove item
        TRACE("remove item...\n");
        gchar *basename = g_file_get_basename(file);
        remove_item(basename);
        g_free(basename);
        return TRUE;
    }

private:
    static gboolean rm_func (GtkTreeModel *model,
				GtkTreePath *path,
				GtkTreeIter *iter,
				gpointer data){
	gchar *text;
	gtk_tree_model_get (model, iter, 
		ACTUAL_NAME, &text, 
		-1);  
	
	if (strcmp(text, (gchar *)data)){
	    g_free(text);
	    return FALSE;
	}
	TRACE("removing %s from treemodel.\n", text);
	GtkListStore *store = GTK_LIST_STORE(model);

	gtk_list_store_remove(store, iter);
	g_free(text);
	return TRUE;
    }



};
}
#endif

