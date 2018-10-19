#ifndef XF_LOCALMONITOR__HH
# define XF_LOCALMONITOR__HH
namespace xf
{

template <class Type>
class LocalMonitor {
    GCancellable *cancellable_;
    GFile *gfile_;
    GFileMonitor *monitor_;
    GHashTable *itemsHash_;
    gboolean shows_hidden_;
    GtkListStore *store_;
public:    
    LocalMonitor(GtkTreeModel *treeModel, const gchar *path){
        itemsHash_ = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
        cancellable_ = g_cancellable_new ();
        gfile_ = g_file_new_for_path (path);
        store_ = GTK_LIST_STORE(treeModel);
        monitor_ = NULL;
        
    }
    ~LocalMonitor(void){
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
    void
    start_monitor(GtkTreeModel *treeModel, const gchar *path){
        // add all initial items to hash
        gtk_tree_model_foreach (treeModel, add2hash, (void *)itemsHash_);
        store_ = GTK_LIST_STORE(treeModel);
        WARN( "*** start_monitor: %s\n", path);
        if (gfile_) g_object_unref(gfile_);
        gfile_ = g_file_new_for_path (path);
        GError *error=NULL;
        if (monitor_) g_object_unref(monitor_);
        monitor_ = g_file_monitor_directory (gfile_, G_FILE_MONITOR_WATCH_MOVES, cancellable_,&error);
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
	gchar *p = g_file_get_path(gfile_);
	WARN("*** stop_monitor at: %s\n", p);
	g_free(p);
	g_file_monitor_cancel(monitor_);
	while (gtk_events_pending())gtk_main_iteration();  
	g_hash_table_remove_all(itemsHash_);
	// hash table remains alive (but empty) until destructor.
    }


    xd_t *
    get_xd_p(GFile *first){
	gchar *path = g_file_get_path(gfile_);
	gchar *basename = g_file_get_basename(first);
	struct dirent *d; // static pointer
	TRACE("looking for %s info\n", basename);
	DIR *directory = opendir(path);
	xd_t *xd_p = NULL;
	if (directory) {
	  while ((d = readdir(directory))  != NULL) {
	    if(strcmp (d->d_name, basename)) continue;
	    xd_p = LocalView<Type>::get_xd_p(path, d);
	    break;
	  }
	  closedir (directory);
	} else {
	  WARN("monitor_f(): opendir %s: %s\n", path, strerror(errno));
	}
	g_free(basename); 
	g_free(path); 
	return xd_p;
    }


    gboolean
    add_new_item(GFile *file){
       xd_t *xd_p = get_xd_p(file);
        if (xd_p) {
            LocalView<Type>::add_local_item(store_, xd_p);
            g_hash_table_replace(itemsHash_, g_strdup(xd_p->d_name), GINT_TO_POINTER(1));
            LocalView<Type>::free_xd_p(xd_p);
            return TRUE;
        } 
        return FALSE;
    }

    static gboolean rm_func (GtkTreeModel *model,
				GtkTreePath *path,
				GtkTreeIter *iter,
				gpointer data){
	gchar *text;
	struct stat *st_p=NULL;
	gtk_tree_model_get (model, iter, 
		STAT, &st_p, 
		ACTUAL_NAME, &text, 
		-1);  
	
	if (strcmp(text, (gchar *)data)){
	    g_free(text);
	    return FALSE;
	}
	DBG("removing %s from treemodel.\n", text);
	GtkListStore *store = GTK_LIST_STORE(model);

    //  free stat record, if any
	g_free(st_p);

	gtk_list_store_remove(store, iter);
	g_free(text);
	return TRUE;
    }


    gboolean 
    remove_item(GFile *file){
        // find the iter and remove item
        gchar *basename = g_file_get_basename(file);
        g_hash_table_remove(itemsHash_, basename); 
        gtk_tree_model_foreach (GTK_TREE_MODEL(store_), rm_func, (gpointer) basename); 
        g_free(basename);
        return TRUE;
    }

    static gboolean stat_func (GtkTreeModel *model,
				GtkTreePath *path,
				GtkTreeIter *iter,
				gpointer data){
	struct stat *st=NULL;
	gchar *text;
	gchar *basename = g_path_get_basename((gchar *)data);
	gtk_tree_model_get (model, iter, 
		ACTUAL_NAME, &text, 
		STAT, &st, 
		-1);  
	
	if (strcmp(basename, text)){
	    g_free(text);
	    g_free(basename);
	    return FALSE;
	}
	g_free(text);
	g_free(basename);
	g_free(st);

	GtkListStore *store = GTK_LIST_STORE(model);
	st = (struct stat *)calloc(1, sizeof(struct stat));
	if (stat((gchar *)data, st) != 0){
	    ERROR( "stat: %s\n", strerror(errno));
	    return FALSE;
	}

	gtk_list_store_set (store, iter, STAT,st, -1);

	return TRUE;
    }

    gboolean 
    restat_item(GFile *src){
        gchar *basename = g_file_get_basename(src);
        if (!g_hash_table_lookup(itemsHash_, basename)) {
            g_free(basename);
            return FALSE; 
        }
        g_free(basename);
        gchar *fullpath = g_file_get_path(src);
        gtk_tree_model_foreach (GTK_TREE_MODEL(store_), stat_func, (gpointer) fullpath); 
        g_free(fullpath);
        return TRUE;
    }

private:
    
    static void
    monitor_f (GFileMonitor      *mon,
              GFile             *first,
              GFile             *second,
              GFileMonitorEvent  event,
              gpointer           data)
    {
        gchar *f= first? g_file_get_basename (first):g_strdup("--");
        gchar *s= second? g_file_get_basename (second):g_strdup("--");
       

        DBG("*** monitor_f call...\n");
        auto p = (LocalMonitor<Type> *)data;

        switch (event){
            case G_FILE_MONITOR_EVENT_DELETED:
            case G_FILE_MONITOR_EVENT_MOVED_OUT:
                DBG("Received DELETED  (%d): \"%s\", \"%s\"\n", event, f, s);
                p->remove_item(first);
                break;
            case G_FILE_MONITOR_EVENT_CREATED:
            case G_FILE_MONITOR_EVENT_MOVED_IN:
                DBG("Received  CREATED (%d): \"%s\", \"%s\"\n", event, f, s);
                p->add_new_item(first);
                break;
            case G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT:
               DBG("Received  CHANGES_DONE_HINT (%d): \"%s\", \"%s\"\n", event, f, s);
                p->restat_item(first);
                // if image, then reload the pixbuf
                break;
            case G_FILE_MONITOR_EVENT_CHANGED:
                DBG("Received  CHANGED (%d): \"%s\", \"%s\"\n", event, f, s);
                break;
            case G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED:
                DBG("Received  ATTRIBUTE_CHANGED (%d): \"%s\", \"%s\"\n", event, f, s);
                p->restat_item(first);
                break;
            case G_FILE_MONITOR_EVENT_PRE_UNMOUNT:
                DBG("Received  PRE_UNMOUNT (%d): \"%s\", \"%s\"\n", event, f, s);
                break;
            case G_FILE_MONITOR_EVENT_UNMOUNTED:
                DBG("Received  UNMOUNTED (%d): \"%s\", \"%s\"\n", event, f, s);
                break;
            case G_FILE_MONITOR_EVENT_MOVED:
            case G_FILE_MONITOR_EVENT_RENAMED:
                DBG("Received  MOVED (%d): \"%s\", \"%s\"\n", event, f, s);
                p->remove_item(first);
                p->add_new_item(second);
                break;
        }
        g_free(f);
        g_free(s);
    }



};
}
#endif

