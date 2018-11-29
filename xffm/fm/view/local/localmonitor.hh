#ifndef XF_LOCALMONITOR__HH
# define XF_LOCALMONITOR__HH
#include "fm/view/base/basemonitor.hh"
namespace xf
{

template <class Type>
class LocalMonitor: public BaseMonitor<Type>
{
    void **mountArg_; // Needs to exist until destructor is called.
public:    
    LocalMonitor(GtkTreeModel *treeModel, BaseView<Type> *baseView):
        BaseMonitor<Type>(treeModel, baseView)
    {       
    }
    ~LocalMonitor(void){
        TRACE("***Destructor:~local_monitor_c()\n");
        // stop mountThread
        mountArg_[1] = NULL;
        while (mountArg_[3]){
            TRACE("***Waiting for mountThread to exit\n");
        }
        g_hash_table_destroy(this->itemsHash());
        g_free(mountArg_);
        TRACE("***Destructor:~local_monitor_c() complete\n");
    }
    void
    start_monitor(GtkTreeModel *treeModel, const gchar *path){
        this->startMonitor(treeModel, path, (void *)monitor_f);
        // start mountThread
        pthread_t mountThread;
                DBG("LocalMonitor thread itemshash=%p\n", this->itemsHash());
        mountArg_ = (void **)calloc(4, sizeof(void *));
        mountArg_[0] = (void *)this;
        mountArg_[1] = GINT_TO_POINTER(TRUE);
        mountArg_[2] = (void *)this->itemsHash();
        mountArg_[3] = GINT_TO_POINTER(TRUE);
	gint retval = pthread_create(&mountThread, NULL, FstabMonitor<Type>::mountThreadF, (void *)this->mountArg_);
	if (retval){
	    ERROR("thread_create(): %s\n", strerror(retval));
	    //return retval;
	}
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
        gboolean showHidden = (Settings<Type>::getSettingInteger("LocalView", "ShowHidden") > 0);
        if (xd_p) {
	    TRACE("add_new_item ...(%s) shows:hidden=%d\n", xd_p->d_name, showHidden);
	    if (xd_p->d_name[0] == '.' && !showHidden) return FALSE;
            // here we should insert according to sort order...
            LocalView<Type>::insertLocalItem(this->store_, xd_p);
            // this just appends:
            //LocalView<Type>::add_local_item(store_, xd_p);
            // use hashkey
            gchar *key = Hash<Type>::get_hash_key(xd_p->path, 10);
            g_hash_table_replace(this->itemsHash(), key, g_strdup(xd_p->path));
            LocalView<Type>::free_xd_p(xd_p);
            return TRUE;
        } 
        return FALSE;
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
        TRACE("localmonitor stat_func(): iconname=%s\n", iconName);
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
        // First we use a hash to check if item is in treemodel.
        // Then, if found, we go on to find the item in the treemodel and update.
        gchar *path = g_file_get_path(src);
        gboolean showHidden = (Settings<Type>::getSettingInteger("LocalView", "ShowHidden") > 0);
	if (path[0] == '.' && !showHidden) {
	    g_free(path);
	    return FALSE;
	}
        // use hashkey
        
        gchar *key = Hash<Type>::get_hash_key(path, 10);
	
        if (!g_hash_table_lookup(this->itemsHash(), key)) {
            g_free(path);
            g_free(key);
            return FALSE; 
        }
        g_free(key);
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
                WARN("monitor_f(): Received  CHANGED (%d): \"%s\", \"%s\"\n", event, f, s);
                p->restat_item(first);
                // reload icon
                //FIXME:  if image, then reload the pixbuf
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
                TRACE("Received  MOVED (%d): \"%s\", \"%s\"\n", event, f, s);
                p->remove_item(first);
                p->add_new_item(second);
                break;
            case G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT:
               TRACE("Received  CHANGES_DONE_HINT (%d): \"%s\", \"%s\"\n", event, f, s);
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

