#ifndef XF_BASEMONITOR__HH
# define XF_BASEMONITOR__HH
namespace xf
{
static GList *localMonitorList = NULL;
template <class Type>
class BaseMonitor {
    gboolean active_;
    GHashTable *itemsHash_;
    GList *reSelectList_;
public:
    void setMonitorStore(GtkListStore *store){store_ = store;}
protected:
    void *mountArg_[5]; // Needs to exist until destructor is called.
    GCancellable *cancellable_;
    GFile *gfile_;
    GFileMonitor *monitor_;
    GtkListStore *store_;
    View<Type> *baseView_;
        
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
        if (!BaseSignals<Type>::validBaseView(view())) return;
        auto items = countItems();
        auto fileCount = g_strdup_printf("%0d", items);
        auto text = g_strdup_printf(_("Files: %s"), fileCount); 
        view()->page()->updateStatusLabel(text);
        g_free(text);
        g_free(fileCount);
    }
    void add2reSelect(const gchar *path){
        if (!isSelected(path)) return;
        reSelectList_ =  g_list_prepend(reSelectList_, g_strdup(path));
    }
    gchar *isReselect(const gchar *path){
        for (auto l=reSelectList_; l && l->data; l=l->next){
            if (strcmp(path, (const gchar *)l->data)==0) return (gchar *)l->data;
        }
        return NULL;
    }
    void removeReselect(const gchar *path){
        auto item=isReselect(path);
        if (item){
            reSelectList_ =  g_list_remove(reSelectList_, item);
            g_free(item);
        }
    }
    gboolean isSelected(const gchar *path){
        gboolean selected=FALSE;
        GList *selectionList=NULL;
        auto treeModel = GTK_TREE_MODEL(store_);
        if (isTreeView) {
            auto selection = gtk_tree_view_get_selection (view()->treeView());
            selectionList = gtk_tree_selection_get_selected_rows (selection, &treeModel);
        } else {
            selectionList = gtk_icon_view_get_selected_items (view()->iconView());
        }
        for (auto l=selectionList; l && l->data; l=l->next){
            GtkTreeIter iter;
            if (gtk_tree_model_get_iter(treeModel, &iter, (GtkTreePath *)l->data))
            {
                gchar *itemPath=NULL;
                gtk_tree_model_get (treeModel, &iter, PATH, &itemPath, -1);
                if (strcmp(path, itemPath)==0){
                    selected = TRUE;
                    g_free(itemPath);
                    break;
                }
                g_free(itemPath);               
            }
        }
        g_list_free_full(selectionList, (GDestroyNotify)gtk_tree_path_free);
        return selected;
    }
    static gboolean reselect_func (GtkTreeModel *model,
                                GtkTreePath *tpath,
                                GtkTreeIter *iter,
                                gpointer data){
        gchar *path;
        auto arg = (void **)data;
        auto inPath = (const gchar *)arg[0];
        auto view = (View<Type> *)arg[1];

        gtk_tree_model_get (model, iter, PATH, &path, -1);  

        TRACE("reselect_func: %s <--> %s\n", path, inPath);
        if (strcmp(path, inPath)){
            g_free(path);
            return FALSE;
        }
        g_free(path);
        TRACE("*** reselect_func: gotcha %s\n", inPath);
        if (isTreeView){
            auto selection = gtk_tree_view_get_selection (view->treeView());
            gtk_tree_selection_select_path(selection, tpath);
        } else {
            gtk_icon_view_select_path(view->iconView(), tpath);
        }

        return TRUE;
    }
    void reSelect(const gchar *path){
        if (!isReselect(path)) return;
        TRACE("*** reselect %s\n", path);
        removeReselect(path);
        void *arg[]={(void *)(path), (void *)baseView_};
        gtk_tree_model_foreach (GTK_TREE_MODEL(store_), reselect_func, (gpointer) arg); 
        
        //auto treeModel = view->treeModel();
        //gtk_tree_model_foreach (GTK_TREE_MODEL(treeModel), stat_func, (gpointer) arg); 

    }

public:    
    GHashTable *itemsHash(void){return itemsHash_;}
    GFileMonitor *monitor(void) {return monitor_;}
    GtkTreeModel *treeModel(void){return GTK_TREE_MODEL(store_);}
    gboolean active(void){return active_;}
    void setActive(gboolean state){active_ = state;}
    
    BaseMonitor(GtkTreeModel *treeModel, View<Type> *view){
        reSelectList_ = NULL;
        itemsHash_ = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
          //      TRACE("BaseMonitor thread itemshash=%p\n", itemsHash_);
        baseView_ = view;
        gboolean showHidden = (Settings<Type>::getSettingInteger("LocalView", "ShowHidden") > 0);
        cancellable_ = g_cancellable_new ();
        gfile_ = NULL;
        store_ = GTK_LIST_STORE(treeModel);
        monitor_ = NULL;
        active_ = FALSE;
        
    }
    ~BaseMonitor(void){
        TRACE("***Destructor:~base monitor_c()\n");
        //g_cancellable_cancel (cancellable);
        //g_object_unref(cancellable);
        if (monitor_) {
            TRACE("***Destructor:~base monitor_c(): stop_monitor\n");
            stop_monitor();
        }
        if (gfile_) g_object_unref(gfile_);
        g_hash_table_destroy(itemsHash_);
    }
    
    View<Type> *view(void) { return baseView_;}

private:
    static gboolean
    add2hash (GtkTreeModel *model,
                            GtkTreePath *tpath,
                            GtkTreeIter *iter,
                            gpointer data){
        auto hash = (GHashTable *)data;
        gchar *path;
        gtk_tree_model_get (model, iter, PATH, &path, -1);  
        // use hashkey
        gchar *key = PixbufHash<Type>::get_hash_key(path, 10);
        g_hash_table_replace(hash, key, g_strdup(path));
        TRACE("add2hash: %s -> %s\n", key, path);
        g_free(path);
        return FALSE;
    }

public:
    
    gboolean pathInTreeHash(const gchar *path, GHashTable *hash){
        gboolean retval = FALSE;
        gchar *key = PixbufHash<Type>::get_hash_key(path, 10);
        TRACE("looking for %s (%s) in hash: %p\n", path, key, hash);
        if(hash && g_hash_table_lookup(hash,key)) {
            TRACE("found %s (%s) in hash: %p\n", path, key, hash);
            retval = TRUE;
        }
        g_free(key);
        return FALSE;
    }

    void
    startMonitor(GtkTreeModel *treeModel, const gchar *path, void *monitor_f){
        if (path && strcmp(path, "/")==0){
            WARN("Known glib bug: g_monitor function does not fully at / (base/monitor.hh)\n");
        }
        // add all initial items to hash
        if (itemsHash_) gtk_tree_model_foreach (treeModel, add2hash, (void *)itemsHash_);
        store_ = GTK_LIST_STORE(treeModel);
        TRACE( "*** start_monitor: %s\n", path);
        if (gfile_) g_object_unref(gfile_);
        gfile_ = g_file_new_for_path (path);
        GError *error=NULL;
        if (monitor_) g_object_unref(monitor_);
        monitor_ = g_file_monitor (gfile_, G_FILE_MONITOR_WATCH_MOVES, cancellable_,&error);
        TRACE("monitor_=%p g_file=%p\n", monitor_, gfile_);
        if (error){
            ERROR("fm/base/monitor::g_file_monitor_directory(%s) failed: %s\n",
                    path, error->message);
            g_object_unref(gfile_);
            gfile_=NULL;
            return;
        }
        g_signal_connect (monitor_, "changed", 
                G_CALLBACK (monitor_f), (void *)this);
        active_ = TRUE;
    }

    void 
    stop_monitor(void){
        active_ = FALSE;
        if (!monitor_) {
            DBG("no monitor to stop\n");
            return;
        }
        TRACE("*** stop monitor %p\n", monitor_);
        localMonitorList = g_list_remove(localMonitorList, (void *)monitor_);      
        if (gfile_) {
            gchar *p = g_file_get_path(gfile_);
            TRACE("*** stop_monitor at: %s\n", p);
            g_free(p);
        }
        g_file_monitor_cancel(monitor_);
        g_object_unref(monitor_);
        monitor_ = NULL;   
        // while (gtk_events_pending())gtk_main_iteration();  
        // hash table remains alive until mountThread finishes.
    }

protected:

    gboolean 
    remove_item(const gchar *path){
        // find the iter and remove iteam
        TRACE("remove item...\n");
        // use hashkey
        gchar *key = PixbufHash<Type>::get_hash_key(path, 10);
        if (itemsHash_) g_hash_table_remove(itemsHash_, key); 
        gtk_tree_model_foreach (GTK_TREE_MODEL(store_), rm_func, (gpointer) path); 
        return TRUE;
    }
    gboolean 
    remove_item(GFile *file){
        // find the iter and remove item
        TRACE("remove item...\n");
        gchar *path = g_file_get_path(file);
        remove_item(path);
        g_free(path);
        return TRUE;
    }

private:
    static gboolean rm_func (GtkTreeModel *model,
                                GtkTreePath *tpath,
                                GtkTreeIter *iter,
                                gpointer data){
        gchar *path;
        gtk_tree_model_get (model, iter, 
                PATH, &path, 
                -1);  
        
        if (strcmp(path, (gchar *)data)){
            g_free(path);
            return FALSE;
        }
        TRACE("removing %s from treemodel.\n", path);
        GtkListStore *store = GTK_LIST_STORE(model);

        gtk_list_store_remove(store, iter);
        g_free(path);
        return TRUE;
    }



};
}
#endif

