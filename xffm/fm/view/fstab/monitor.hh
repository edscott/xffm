#ifndef XF_FSTABMONITOR__HH
# define XF_FSTABMONITOR__HH
#ifdef HAVE_MNTENT_H
#define USE_MOUNTTHREAD
#endif

namespace xf
{
template <class Type> class LocalView;
template <class Type> class BaseMonitor;
template <class Type> class FstabView;
// Linux files:
// (/proc/mounts), /proc/partitions
// 
template <class Type>
class FstabMonitor: public BaseMonitor<Type> {

    // moved to basemonitor for use in other monitors (local monitor):
    //void *mountArg_[5]; // Needs to exist until destructor is called.
    // Please note, sending signal to monitor avoid race condition if
    // this thread tries to do more than this...
#ifdef BSD_FOUND
#else
public:    
    FstabMonitor(GtkTreeModel *treeModel, View<Type> *view):
        BaseMonitor<Type>(treeModel, view)
    {   
        
    }
    ~FstabMonitor(void){
        TRACE("Destructor:~local_monitor_c()\n");
#ifdef xUSE_MOUNTTHREAD
        // stop mountThread
        this->mountArg_[1] = NULL;
        while (this->mountArg_[2]){
            TRACE("***Waiting for mountThread to exit\n");
            sleep(1);
        }
#endif
        //g_hash_table_destroy(this->itemsHash());
        TRACE("***Destructor:~local_monitor_c() complete\n");
    }

    void
    start_monitor(View<Type> *view, const gchar *path){
	TRACE("Starting monitor for path:%s\n", path);
        this->startMonitor(view->treeModel(), path, (void *)monitor_f);
        view->setMonitorObject(this);
        // start mountThread
#ifdef USE_MOUNTTHREAD
        pthread_t mountThread;
        this->mountArg_[0] = (void *)this;
        this->mountArg_[1] = GINT_TO_POINTER(TRUE);
        this->mountArg_[2] = GINT_TO_POINTER(TRUE);
	// mountThreadF will monitor if items are mounted or unmounted
	gint retval = pthread_create(&mountThread, NULL, FstabMonitor<Type>::mountThreadF, (void *)this->mountArg_);
	if (retval){
	    ERROR("fm/view/fstab/monitor::thread_create(): %s\n", strerror(retval));
	    //return retval;
	}
#endif
    }

    static void *
    mountThreadF(void *data){
        void **arg = (void **)data;
        auto baseMonitor = (BaseMonitor<Type> *)arg[0];
        TRACE("*** baseMonitor = %p\n", baseMonitor);
        g_object_set_data(G_OBJECT(baseMonitor->treeModel()), "baseMonitor", (void *)baseMonitor);
        // get initial md5sum
        gchar *sum = Util<Type>::md5sum("/proc/mounts");
        gchar *sumPartitions = Util<Type>::md5sum("/proc/partitions");
        if (!sum) {
            ERROR("fm/view/fstab/monitor::Exiting mountThreadF() on md5sum error (sum)\n");
            g_free(data);
            return NULL;
        }
        TRACE("FstabMonitor::mountThreadF(): initial md5sum=%s basemonitor=%p", sum, baseMonitor);
        
	auto hash = getMountHash(NULL);
        while (arg[1]){// arg[1] is semaphore to thread
            usleep(250000);
	    if (!baseMonitor->active())continue;
            //sleep(1);
            TRACE("mountThreadF loop for arg=%p\n", data);
            gchar *newSum = Util<Type>::md5sum("/proc/mounts");
            gchar *newSumPartitions = Util<Type>::md5sum("/proc/partitions");
            if (!newSum){
                ERROR("fm/view/fstab/monitor::Exiting mountThreadF() on md5sum error (newSum)\n");
                g_hash_table_destroy(hash);
                g_free(sum);
                return NULL;
            }
            if (strcmp(newSum, sum)){
                TRACE("new md5sum /proc/mounts = %s (%s)\n", newSum, sum);
                TRACE("now we test whether icon update is necessary...\n");
                g_free(sum);
                sum = newSum;
                // Any new mounts?
                // Foreach item in itemsHash_ check
                // if (isMounted() and not in hash)
                // if so, then send change signal for gfile path. 
                // This should set the greenball.
                //TRACE("thread itemshash=%p\n", itemsH);
                gtk_tree_model_foreach(baseMonitor->treeModel(), checkIfMounted, (void *)hash);
                //
                // Any new umounts?
                // Foreach item in hash, check if 
                // if (!isMounted(item) && in itemsHash_)
                // then
                //     sendSignal change for gfile(item)
                gtk_tree_model_foreach(baseMonitor->treeModel(), checkIfNotMounted, (void *)hash);
                //
                // Update hash.
                hash = getMountHash(hash);
            }
            if (strcmp(newSumPartitions, sumPartitions)){
                TRACE("new md5sum /proc/partitions = %s (%s)\n", newSumPartitions, sumPartitions);
	    }
        }
        g_free(sum);
        TRACE("***now exiting mountThreadF(), baseMonitor=%p\n", baseMonitor);
        g_hash_table_destroy(hash);
        arg[2] = NULL; // arg[2] is semaphore to calling thread.
        return NULL;
    }


private:

    static void *sendChangeSignal(void *data){
       auto arg = (void **)data;
       auto baseMonitor = (BaseMonitor<Type> *)arg[0];
       auto path = (gchar *)arg[1];
       GFile *child = g_file_new_for_path (path);
       g_free(path);
       g_file_monitor_emit_event (baseMonitor->monitor(),
                   child, NULL, G_FILE_MONITOR_EVENT_CHANGED);
       // Seems that the event will manage reference to g_file
       // or at least the following unref does not wreak havoc...
        g_object_unref(child);
        return NULL;
    }

    static gboolean 
    checkIfMounted(GtkTreeModel *treeModel,
                            GtkTreePath *tpath,
                            GtkTreeIter *iter,
                            gpointer data){
        gchar *path;
        gboolean retval = FALSE;
        // Mounted but not in mounts hash:
 	gtk_tree_model_get (treeModel, iter, PATH, &path, -1);
	TRACE("Fstab<>::checkIfMounted(%s)...\n", path);
        if (FstabView<Type>::isMounted(path)) {
            gchar *key = PixbufHash<Type>::get_hash_key(path, 10);
            if (!g_hash_table_lookup((GHashTable *)data, key)){
                // update the icon
                TRACE("*** Send change signal for %s (now mounted, monitor %p)\n", 
                        (gchar *)path, g_object_get_data(G_OBJECT(treeModel), "baseMonitor"));
                void *arg[] = { 
                    g_object_get_data(G_OBJECT(treeModel), "baseMonitor"),
                    (void *)path };
                Util<Type>::context_function(sendChangeSignal, arg);
            }
            g_free(key);
        }
        return retval;
    }
    static gboolean 
    checkIfNotMounted(GtkTreeModel *treeModel,
                            GtkTreePath *tpath,
                            GtkTreeIter *iter,
                            gpointer data){
        gchar *path;
        gboolean retval = FALSE;
        // Mounted but not in mounts hash:
 	gtk_tree_model_get (treeModel, iter, PATH, &path, -1);
        if (!FstabView<Type>::isMounted(path)) {
            gchar *key = PixbufHash<Type>::get_hash_key(path, 10);
            if (g_hash_table_lookup((GHashTable *)data, key)){
                // update the icon
                void *arg[] = { 
                    g_object_get_data(G_OBJECT(treeModel), "baseMonitor"),
                    (void *)path };
                TRACE("*** Send change signal for %s (now unmounted, monitor %p)\n", 
                        (gchar *)path, g_object_get_data(G_OBJECT(treeModel), "baseMonitor"));
                Util<Type>::context_function(sendChangeSignal, arg);
            }
            g_free(key);
        }
        return retval;
    }

    
    

     static GHashTable *getMountHash(GHashTable *oldHash){
         if (oldHash) g_hash_table_destroy(oldHash);
	// Get first two items per line of /proc/mounts
	// and add both to hash.
        GHashTable  *hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
        FILE *mounts = fopen("/proc/mounts", "r");
        if (!mounts) return NULL;
        gchar buffer[2048];
        memset(buffer, 0, 2048);
        while (fgets(buffer, 2047, mounts) && !feof(mounts)){
            gchar **items = g_strsplit(buffer, " ", 3);
            if (!items) continue;
            if (!items[0] || !items[1]) {
                g_strfreev(items);
                continue;
            }
            if (g_path_is_absolute(items[0]) || g_path_is_absolute(items[1])){
                for (gint i=0; i<2; i++){
                    // use hashkey
                    gchar *key = PixbufHash<Type>::get_hash_key(items[i], 10);
                    g_hash_table_replace(hash, key, g_strdup(items[i]));
                }
            }
        }
        fclose(mounts);
	return hash;
     }
   
    static gchar *
    uuid2Partition(const gchar *partuuidPath){
        const gchar *command = "ls -l /dev/disk/by-partuuid";
	FILE *pipe = popen (command, "r");
	if(pipe == NULL) {
	    ERROR("fm/view/fstab/monitor::Cannot pipe from %s\n", command);
	    return NULL;
	}
        gchar *partuuid = g_path_get_basename(partuuidPath);
        gchar line[256];
        memset(line, 0, 256);
        gchar *partition = NULL;
            TRACE("uuid2Partition(): looking for : \"%s\"\n", partuuid);
	while (fgets (line, 255, pipe) && !feof(pipe)) {
            TRACE("uuid2Partition(): %s: %s\n", partuuid, line);
            if (!strstr(line, "->")) continue;
            if (!strstr(line, partuuid)) continue;
            TRACE("uuid2Partition(): GOTCHA: %s\n", line);
            gchar **f = g_strsplit(line, "->", 2);
            if (strchr(f[1], '\n')) *strchr(f[1], '\n') = 0;
            g_strstrip(f[1]);
            gchar *basename = g_path_get_basename(f[1]);
            TRACE("uuid2Partition(): GOTCHA: basename=%s\n", basename);
            g_strfreev(f);
            partition = g_strconcat ("/dev/", basename, NULL);
            g_free(basename);
            break;

            /*if (strstr(line, "->") && strstr(line, partuuid)) {
                if (strchr(line, '\n')) *strchr(line, '\n') = 0;
                partition = g_strdup_printf("/dev/%s", strrchr(line, '/')+1);
                g_strstrip(partition);
                if (strcmp("/dev",partition)==0) {
                    g_free(partition);
                    partition=NULL;
                }
                break;
            }*/
	}
        pclose (pipe);
        g_free(partuuid);
	return partition;

    }
    static gchar *
    id2Partition(const gchar *idPath){
        const gchar *command = "ls -l /dev/disk/by-id";
	FILE *pipe = popen (command, "r");
	if(pipe == NULL) {
	    ERROR("fm/view/fstab/monitor::Cannot pipe from %s\n", command);
	    return NULL;
	}
        gchar *base = g_path_get_basename(idPath);
        gchar line[256];
        memset(line, 0, 256);
        gchar *partition = NULL;
            TRACE("id2Partition(): looking for : \"%s\"\n", base);
	while (fgets (line, 255, pipe) && !feof(pipe)) {
            TRACE("id2Partition(): %s: %s\n", base, line);
            if (!strstr(line, "->")) continue;
            if (!strstr(line, base)) continue;
            TRACE("id2Partition(): GOTCHA: %s\n", line);
            gchar **f = g_strsplit(line, "->", 2);
            if (strchr(f[1], '\n')) *strchr(f[1], '\n') = 0;
            g_strstrip(f[1]);
            gchar *basename = g_path_get_basename(f[1]);
            TRACE("id2Partition(): GOTCHA: basename=%s\n", basename);
            g_strfreev(f);
            partition = g_strconcat ("/dev/", basename, NULL);
            g_free(basename);
            break;

            /*if (strstr(line, "->") && strstr(line, partuuid)) {
                if (strchr(line, '\n')) *strchr(line, '\n') = 0;
                partition = g_strdup_printf("/dev/%s", strrchr(line, '/')+1);
                g_strstrip(partition);
                if (strcmp("/dev",partition)==0) {
                    g_free(partition);
                    partition=NULL;
                }
                break;
            }*/
	}
        pclose (pipe);
        g_free(base);
	return partition;

    }
    
    static gboolean rm_func (GtkTreeModel *model,
				GtkTreePath *tpath,
				GtkTreeIter *iter,
				gpointer data){
	gchar *id;
	gchar *path;
	gtk_tree_model_get (model, iter, PATH, &path, DISK_ID, &id, -1);  

	TRACE("%s test for removing %s (%s) from treemodel.\n", path, id, (gchar *)data);
        if (!id) return FALSE;
	
	if (!strstr(id, (gchar *)data)){
	    g_free(id);
	    g_free(path);
	    return FALSE;
	}
	TRACE("removing %s (%s) from treemodel.\n", id, (gchar *)data);
	GtkListStore *store = GTK_LIST_STORE(model);

	gtk_list_store_remove(store, iter);
	g_free(path);
        g_free(id);
	return TRUE;
    }
    

    static void
    monitor_f (GFileMonitor      *mon,
              GFile             *first,
              GFile             *second,
              GFileMonitorEvent  event,
              gpointer           data)
    {

        // Here we enter with full path to partiuuid...
        gchar *f= first? g_file_get_path (first):g_strdup("--");
        gchar *s= second? g_file_get_path (second):g_strdup("--");
       

        TRACE("*** monitor_f call...\n");
        auto p = (FstabMonitor<Type> *)data;
        gchar *fsType;
        switch (event){
            case G_FILE_MONITOR_EVENT_DELETED:
            case G_FILE_MONITOR_EVENT_MOVED_OUT:
            {
                TRACE("Received DELETED  (%d): \"%s\", \"%s\"\n", event, f, s);
                TRACE("*** rm %s \n", f);
                gchar *base = g_path_get_basename(f);
                gchar *devicePath = id2Partition(f);
                TRACE("looking in hash for key=%s\n", devicePath);
                /*if (p->itemsHash()&& devicePath){
                    g_hash_table_remove(p->itemsHash(), devicePath); 
                }*/
                gtk_tree_model_foreach (p->treeModel(), rm_func, (gpointer)base); 
                g_free(base);
                g_free(devicePath);
                break;
            }
            case G_FILE_MONITOR_EVENT_CREATED:
            case G_FILE_MONITOR_EVENT_MOVED_IN:
            {
                TRACE("Received  CREATED (%d): \"%s\", \"%s\"\n", event, f, s);
                gchar *devicePath = id2Partition(f);
                TRACE("adding partition %s -> %s \n", f, devicePath);
                FstabView<Type>::addPartition(GTK_TREE_MODEL(p->store_), devicePath);

                /*if (!g_hash_table_lookup(p->itemsHash(), devicePath)){
                    TRACE("not in hash %s \n", devicePath);
                    FstabView<Type>::addPartition(GTK_TREE_MODEL(p->store_), devicePath);
                    g_hash_table_replace(p->itemsHash(), g_strdup(devicePath), g_strdup(f));
                }*/
                g_free(devicePath);
                break;
            }

            case G_FILE_MONITOR_EVENT_CHANGED:
                TRACE("*** Received  CHANGED (%d): \"%s\", \"%s\"\n", event, f, s);
                p->redoIcon(f);
                break;
            case G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED:
                TRACE("Received  ATTRIBUTE_CHANGED (%d): \"%s\", \"%s\"\n", event, f, s);
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
                break;
            case G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT:
               TRACE("Received  CHANGES_DONE_HINT (%d): \"%s\", \"%s\"\n", event, f, s);
                break;
        }

        g_free(f);
        g_free(s);
    }
    gboolean 
    redoIcon(const gchar *path){
        TRACE("redoIcon %s ...\n", path);
        /*gchar *key = PixbufHash<Type>::get_hash_key(path, 10);
        if (!g_hash_table_lookup(this->itemsHash(), key)) {
            g_free(key);
            TRACE("*** %s not in itemsHash\n", path);
            return FALSE; 
        }
        g_free(key);*/
        gtk_tree_model_foreach (GTK_TREE_MODEL(this->store_), changeIcon, (gpointer) path); 
        return TRUE;
    }

   static gboolean changeIcon (GtkTreeModel *model,
				GtkTreePath *tpath,
				GtkTreeIter *iter,
				gpointer data){
	auto path = (const gchar *)data;
        gchar *currentPath;
	gtk_tree_model_get (model, iter, PATH, &currentPath, -1);  
        
        TRACE("fstabmonitor currentPath \"%s\" == \"%s\"\n", currentPath, path);
        if (strcmp(path, currentPath)){
            g_free(currentPath);
            return FALSE;
        }
        g_free(currentPath);
        TRACE("*** fstabmonitor currentPath %s\n", currentPath, path);
	
	GtkListStore *store = GTK_LIST_STORE(model);

        gboolean mounted = FstabView<Type>::isMounted(path);
        auto iconName = (mounted)?"drive-harddisk/NW/greenball/3.0/180":
            "drive-harddisk/NW/grayball/3.0/180";
        TRACE("fstabmonitor stat_func(): iconname=%s\n", iconName);
        GdkPixbuf *pixbuf = Pixbuf<Type>::get_pixbuf(iconName,  GTK_ICON_SIZE_DIALOG);
	gtk_list_store_set (store, iter, 
                ICON_NAME, iconName,
                DISPLAY_PIXBUF, pixbuf,
                NORMAL_PIXBUF, pixbuf,
		-1);

	return TRUE;
    }

#endif

};
}
#endif

