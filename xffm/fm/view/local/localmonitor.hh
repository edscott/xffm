#ifndef XF_LOCALMONITOR__HH
# define XF_LOCALMONITOR__HH
#include "fm/view/base/basemonitor.hh"
namespace xf
{

template <class Type>
class LocalMonitor: public BaseMonitor<Type>
   // ,public ThreadControl<Type>
{
    pthread_t mountThread;
    void **mountArg;
    //pthread_t clipboardThread;

    static gchar *md5sum(const gchar *file){
        gchar *md5sum = g_find_program_in_path("md5sum");
        if (!md5sum){
            ERROR("cannot find md5sum program\n");
            return NULL;
        }
        g_free(md5sum);
        gchar *command = g_strdup_printf("md5sum %s", file);
        FILE *pipe = popen(command, "r");
        if (!pipe){
            ERROR("cannot pipe to %s\n", command);
            g_free(command);
            return NULL;
        }
        g_free(command);
        gchar buffer[1024];
        memset (buffer, 0, 1024);
        fgets(buffer, 1023, pipe);
        pclose(pipe);
        if (strlen(buffer)) return g_strdup(buffer);
        return NULL;
    }
            
    static void *
    mountThreadF(void *data){
        // get initial md5sum
        static gchar *sum = md5sum("/proc/mounts");
        if (!sum) return NULL;
        DBG("initial md5sum=%s", sum);
        void **arg = (void **)data;
        auto localMonitor = (LocalMonitor<Type> *)arg[0];
        while (arg[1]){
            //usleep()
            sleep(1);
            gchar *newSum = md5sum("/proc/mounts");
            if (!newSum) return NULL;
            DBG("new md5sum /proc/mounts = %s\n", newSum);
            if (strcmp(newSum, sum)){
                WARN("now we test whether icon update is necessary...\n");
                g_free(sum);
                sum = newSum;
            }

            // if changed from md5sum
            //   get new md5sum
            //   trigger an icon reload:
            //     Check if path is applicable 
            //      (dirname matches baseview->path)
            //      if so, trigger an attribute change
            //      for folder so that monitor will update 
            //      the icon
        }
        g_free(sum);
        DBG("now exiting mountThreadF()\n");
        g_free(data);
        return NULL;
    }
public:    
    LocalMonitor(GtkTreeModel *treeModel, BaseView<Type> *baseView):
        BaseMonitor<Type>(treeModel, baseView)
    {       
    }
    ~LocalMonitor(void){
        TRACE("Destructor:~local_monitor_c()\n");
        // stop mountThread
        mountArg[1] = NULL;
    }
    void
    start_monitor(GtkTreeModel *treeModel, const gchar *path){
        this->startMonitor(treeModel, path, (void *)monitor_f);
        // start mountThread
        mountArg = (void **)calloc(2, sizeof(void *));
        mountArg[0] = (void *)this;
        mountArg[1] = GINT_TO_POINTER(TRUE);
	gint retval = pthread_create(&mountThread, NULL, mountThreadF, (void *)mountArg);
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
            g_hash_table_replace(this->itemsHash_, g_strdup(xd_p->path), GINT_TO_POINTER(1));
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
        gchar *basename = g_file_get_basename(src);
        gboolean showHidden = (Settings<Type>::getSettingInteger("LocalView", "ShowHidden") > 0);
	if (basename[0] == '.' && !showHidden) {
	    g_free(basename);
	    return FALSE;
	}
	
        if (!g_hash_table_lookup(this->itemsHash_, basename)) {
            g_free(basename);
            return FALSE; 
        }
        g_free(basename);
        gchar *fullpath = g_file_get_path(src);
        gtk_tree_model_foreach (GTK_TREE_MODEL(this->store_), stat_func, (gpointer) fullpath); 
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
                DBG("Received  CHANGED (%d): \"%s\", \"%s\"\n", event, f, s);
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
                break;        }
        g_free(f);
        g_free(s);
    }



};
}
#endif

