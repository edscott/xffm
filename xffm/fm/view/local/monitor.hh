#ifndef XF_LOCALMONITOR__HH
# define XF_LOCALMONITOR__HH
#ifdef HAVE_MNTENT_H
#define USE_MOUNTTHREAD
#endif

namespace xf
{

template <class Type>
class LocalMonitor: public BaseMonitor<Type>
{
    void *mountArg_[5]; // Needs to exist until destructor is called.
    static gboolean findInModel (GtkTreeModel *treeModel,
                            GtkTreePath *tpath,
                            GtkTreeIter *iter,
                            gpointer data){
        auto arg = (void **)data;
        auto needle = (const gchar *)arg[0];
        gchar *path;
        gtk_tree_model_get (treeModel, iter, PATH, &path, -1);
        if (strcmp(path, needle) == 0){
            arg[1] = GINT_TO_POINTER(1);
            g_free(path);
            return TRUE;
        }
        g_free(path);
        return FALSE;
    }
    static gboolean isInModel(GtkTreeModel *treeModel, const gchar *path){
        void *arg[2] = {
            (void *)path,
            NULL}
        ;
        gtk_tree_model_foreach(treeModel, findInModel, (void *)arg);
        if (arg[1]) return TRUE;
        return FALSE;
    }
public:    
    LocalMonitor(GtkTreeModel *treeModel, View<Type> *view):
        BaseMonitor<Type>(treeModel, view)
    {       
    }
    ~LocalMonitor(void){
#ifndef USE_LOCAL_MONITOR
	return;
#endif
        TRACE("***Destructor:~local_monitor_c()\n");
#ifdef USE_MOUNTTHREAD
        // stop mountThread
        mountArg_[1] = NULL;
        while (mountArg_[2]){
            TRACE("***Waiting for mountThread to exit\n");
        }
#endif
        //g_hash_table_destroy(this->itemsHash());
        TRACE("***Destructor:~local_monitor_c() complete\n");
    }
    void
    start_monitor(GtkTreeModel *treeModel, const gchar *path){
#ifndef USE_LOCAL_MONITOR
	DBG("*** Local monitor at %s s disabled.\n", path);
#else 
        this->startMonitor(treeModel, path, (void *)monitor_f);
# ifdef USE_MOUNTTHREAD
        // start mountThread
        pthread_t mountThread;
        //TRACE("LocalMonitor thread itemshash=%p\n", this->itemsHash());
        mountArg_[0] = (void *)this;
        mountArg_[1] = GINT_TO_POINTER(TRUE);
        mountArg_[2] = GINT_TO_POINTER(TRUE);
	gint retval = pthread_create(&mountThread, NULL, FstabMonitor<Type>::mountThreadF, (void *)mountArg_);
	if (retval){
	    ERROR("fm/view/local/monitor::thread_create(): %s\n", strerror(retval));
	    //return retval;
	}
# endif
#endif
    }

    xd_t *
    get_xd_p(GFile *first){
	gchar *path = g_file_get_path(this->gfile_);
	gchar *basename = g_file_get_basename(first);
	struct dirent *d; // static pointer
	TRACE("looking for %s info\n", basename);
	DIR *directory = opendir(path);
	xd_t *xd_p = NULL;
	if (directory) {
	  while ((d = readdir(directory))  != NULL) {
	    if(strcmp (d->d_name, basename)) continue;
	    xd_p = LocalView<Type>::get_xd_p(path, d, TRUE);
	    break;
	  }
	  closedir (directory);
	} else {
	  ERROR("fm/view/local/monitor::monitor_f(): opendir %s: %s\n", path, strerror(errno));
	}
	g_free(basename); 
	g_free(path); 
	return xd_p;
    }


    gboolean
    add_new_item(GFile *file){
       xd_t *xd_p = get_xd_p(file);
        gboolean showHidden = (Settings<Type>::getSettingInteger("LocalView", "ShowHidden") > 0);
        if (xd_p) {
	    if (xd_p->d_name[0] == '.' && !showHidden) return FALSE;
            // here we should insert according to sort order...
            LocalView<Type>::insertLocalItem(this->store_, xd_p);
            // this just appends:
            //LocalView<Type>::add_local_item(store_, xd_p);
            // use hashkey
            gchar *key = Hash<Type>::get_hash_key(xd_p->path, 10);
	    TRACE("add_new_item ...(%s --> %s) shows:hidden=%d\n", key, xd_p->path, showHidden);
            //g_hash_table_replace(this->itemsHash(), key, g_strdup(xd_p->path));
            LocalView<Type>::free_xd_p(xd_p);
            return TRUE;
        } 
        return FALSE;
    }

    static gboolean stat_func (GtkTreeModel *model,
				GtkTreePath *tpath,
				GtkTreeIter *iter,
				gpointer data){
        gchar *path;
	auto inPath = (gchar *)data;
	gtk_tree_model_get (model, iter, PATH, &path, -1);  

	TRACE("stat_func: %s <--> %s\n", path, inPath);
	if (strcmp(path, inPath)){
            g_free(path);
	    return FALSE;
	}
        g_free(path);
	TRACE("stat_func: gotcha %s\n", inPath);

	GtkListStore *store = GTK_LIST_STORE(model);

	auto directory = g_path_get_dirname(inPath);
	struct dirent d;
	auto basename = g_path_get_basename(inPath);
	strncpy(d.d_name, basename, 256);
	g_free(basename);
	auto xd_p = LocalModel<Type>::get_xd_p(directory, &d, TRUE);
	g_free(directory);
        gchar *iconName = xd_p->icon;

	
        TRACE("***localmonitor stat_func(): iconname=%s\n", iconName);
        GdkPixbuf *pixbuf = Pixbuf<Type>::get_pixbuf(iconName,  GTK_ICON_SIZE_DIALOG);
        auto highlight_pixbuf = gdk_pixbuf_copy(pixbuf);
        // Now decorate the pixbuf with emblem (types.h).
        void *arg[] = {NULL, (void *)highlight_pixbuf, NULL, NULL, (void *)HIGHLIGHT_OPEN_EMBLEM };
        // Done by main gtk thread:
        Util<Type>::context_function(Icons<Type>::insert_decoration_f, arg);
        


	gtk_list_store_set (store, iter, 
                SIZE, xd_p->st->st_size, 
                DATE, xd_p->st->st_mtim.tv_sec ,
                ICON_NAME, iconName,
                DISPLAY_PIXBUF, pixbuf,
                NORMAL_PIXBUF, pixbuf,
                HIGHLIGHT_PIXBUF, highlight_pixbuf, 
                FLAGS, xd_p->d_type,
		-1);
        LocalModel<Type>::free_xd_p(xd_p);
	return TRUE;
    }

    gboolean 
    restat_item(GFile *src){
        // First we use a hash to check if item is in treemodel.
        // Then, if found, we go on to find the item in the treemodel and update.
        gchar *path = g_file_get_path(src);
	TRACE("restat_item %s \n", path);
        gboolean showHidden = (Settings<Type>::getSettingInteger("LocalView", "ShowHidden") > 0);
	if (path[0] == '.' && !showHidden) {
	    g_free(path);
	    return FALSE;
	}
        // use hashkey
        // FIXME: itemsHash is out of sync because of backing store treemodel
#if 0        
        gchar *key = Hash<Type>::get_hash_key(path, 10);
	
        if (!g_hash_table_lookup(this->itemsHash(), key)) {
  	    DBG("restat_item %s --> %s is not in itemsHash()\n", key, path);
            g_free(path);
            g_free(key);
            return FALSE; 
        }
        g_free(key);
#endif
        gtk_tree_model_foreach (GTK_TREE_MODEL(this->store_), stat_func, (gpointer) path); 
        g_free(path);
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
        gchar *f= first? g_file_get_path (first):g_strdup("--");
        gchar *s= second? g_file_get_path (second):g_strdup("--");
       

        TRACE("*** monitor_f call...\n");
        auto p = (LocalMonitor<Type> *)data;
	if (!p->active()){
	    DBG("monitor_f(): monitor not currently active.\n");
	    return;
	}
        if (!BaseSignals<Type>::validBaseView(p->view())) return;

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
                /*if (isInModel(p->treeModel(), f)){
                    p->restat_item(first);
                } else*/ 
                {
                    p->add_new_item(first);
                    p->updateFileCountLabel();
                }
                break;

            case G_FILE_MONITOR_EVENT_CHANGED:
                TRACE("monitor_f(): Received  CHANGED (%d): \"%s\", \"%s\"\n", event, f, s);
                p->restat_item(first);
                // reload icon
                //FIXME:  if image, then reload the pixbuf
                break;
            case G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED:
                TRACE("***Received  ATTRIBUTE_CHANGED (%d): \"%s\", \"%s\"\n", event, f, s);
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
                if (isInModel(p->treeModel(), s))
                {
                    p->restat_item(second);
                } else p->add_new_item(second);
                break;
            case G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT:
               TRACE("***Received  CHANGES_DONE_HINT (%d): \"%s\", \"%s\"\n", event, f, s);
                //p->restat_item(first);
                // if image, then reload the pixbuf
                break;       
        }
        g_free(f);
        g_free(s);
    }



};
}
#endif

