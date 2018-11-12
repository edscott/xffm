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
        auto items = countItems();
        auto fileCount = g_strdup_printf("%0d", items);
        auto text = g_strdup_printf(_("Files: %s"), fileCount); 
        baseView()->page()->updateStatusLabel(text);
        g_free(text);
        g_free(fileCount);
    }

public:    
    LocalMonitor(GtkTreeModel *treeModel, const gchar *path, BaseView<Type> *baseView){
        baseView_ = baseView;
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

    BaseView<Type> *baseView(void) { return baseView_;}

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
        TRACE( "*** start_monitor: %s\n", path);
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
	    WARN("add_new_item ...(%s)\n", xd_p->d_name);
	    if (xd_p->d_name[0] == '.' && !shows_hidden_) return FALSE;
            // here we should insert according to sort order...
            LocalView<Type>::insertLocalItem(store_, xd_p);
            // this just appends:
            //LocalView<Type>::add_local_item(store_, xd_p);
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


    gboolean 
    remove_item(GFile *file){
        // find the iter and remove item
        TRACE("remove item...\n");
        gchar *basename = g_file_get_basename(file);
        g_hash_table_remove(itemsHash_, basename); 
        gtk_tree_model_foreach (GTK_TREE_MODEL(store_), rm_func, (gpointer) basename); 
        g_free(basename);
        return TRUE;
    }

    static gboolean stat_func (GtkTreeModel *model,
				GtkTreePath *tpath,
				GtkTreeIter *iter,
				gpointer data){
	gchar *text;
        gchar *path;
	gchar *basename = g_path_get_basename((gchar *)data);
	gtk_tree_model_get (model, iter, 
		ACTUAL_NAME, &text, 
                PATH, &path,
		-1);  
	
	if (strcmp(basename, text)){
	    g_free(text);
            g_free(path);
	    g_free(basename);
	    return FALSE;
	}
	g_free(text);
	g_free(basename);

	GtkListStore *store = GTK_LIST_STORE(model);
	struct stat st;
	if (stat((gchar *)data, &st) != 0){
	    TRACE( "localmonitor stat_func() stat: %s\n", strerror(errno));
            g_free(path);
	    return FALSE;
	}

        gchar *iconName = LocalView<Type>::get_iconname(path);
        WARN("localmonitor stat_func(): iconname=%s\n", iconName);
        GdkPixbuf *pixbuf = Pixbuf<Type>::get_pixbuf(iconName,  GTK_ICON_SIZE_DIALOG);

	gtk_list_store_set (store, iter, 
                SIZE, st.st_size, 
                DATE, st.st_mtim.tv_sec ,
                ICON_NAME, iconName,
                DISPLAY_PIXBUF, pixbuf,
                NORMAL_PIXBUF, pixbuf,
		-1);
        g_free(iconName);
        g_free(path);


	return TRUE;
    }

    gboolean 
    restat_item(GFile *src){
        TRACE("restat_item ...\n");
        gchar *basename = g_file_get_basename(src);
	if (basename[0] == '.' && !shows_hidden_) {
	    g_free(basename);
	    return FALSE;
	}
	
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
       

        TRACE("*** monitor_f call...\n");
        auto p = (LocalMonitor<Type> *)data;

        switch (event){
            case G_FILE_MONITOR_EVENT_DELETED:
            case G_FILE_MONITOR_EVENT_MOVED_OUT:
                TRACE("Received DELETED  (%d): \"%s\", \"%s\"\n", event, f, s);
                p->remove_item(first);
                p->updateFileCountLabel();
                break;
            case G_FILE_MONITOR_EVENT_CREATED:
            case G_FILE_MONITOR_EVENT_MOVED_IN:
                TRACE("Received  CREATED (%d): \"%s\", \"%s\"\n", event, f, s);
                p->add_new_item(first);
                p->updateFileCountLabel();
                break;

            case G_FILE_MONITOR_EVENT_CHANGED:
                TRACE("Received  CHANGED (%d): \"%s\", \"%s\"\n", event, f, s);
                p->restat_item(first);
                // reload icon
                //FIXME:  if image, then reload the pixbuf
                break;
            case G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED:
                TRACE("Received  ATTRIBUTE_CHANGED (%d): \"%s\", \"%s\"\n", event, f, s);
                p->restat_item(first);
                break;
            case G_FILE_MONITOR_EVENT_PRE_UNMOUNT:
                TRACE("Received  PRE_UNMOUNT (%d): \"%s\", \"%s\"\n", event, f, s);
                break;
            case G_FILE_MONITOR_EVENT_UNMOUNTED:
                TRACE("Received  UNMOUNTED (%d): \"%s\", \"%s\"\n", event, f, s);
                break;
            case G_FILE_MONITOR_EVENT_MOVED:
            case G_FILE_MONITOR_EVENT_RENAMED:
                TRACE("Received  MOVED (%d): \"%s\", \"%s\"\n", event, f, s);
                p->remove_item(first);
                p->add_new_item(second);
                break;
            case G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT:
               TRACE("Received  CHANGES_DONE_HINT (%d): \"%s\", \"%s\"\n", event, f, s);
                //p->restat_item(first);
                // if image, then reload the pixbuf
                break;        }
        g_free(f);
        g_free(s);
    }



};
}
#endif

