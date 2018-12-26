#ifndef XF_FSTABMONITOR__HH
# define XF_FSTABMONITOR__HH
#include "response/timeoutresponse.hh"
#include "fm/base/monitor.hh"
namespace xf
{
template <class Type> class LocalView;
template <class Type> class BaseMonitor;
template <class Type> class Fstab;
// Linux files:
// (/proc/mounts), /proc/partitions
// 
template <class Type>
class FstabMonitor: public BaseMonitor<Type> {

    void *mountArg_[5]; // Needs to exist until destructor is called.
    // Please note, sending signal to monitor avoid race condition if
    // this thread tries to do more than this...
public:    
    FstabMonitor(GtkTreeModel *treeModel, BaseView<Type> *baseView):
        BaseMonitor<Type>(treeModel, baseView)
    {   
        
    }
    ~FstabMonitor(void){
        TRACE("Destructor:~local_monitor_c()\n");
        // stop mountThread
        mountArg_[1] = NULL;
        while (mountArg_[2]){
            TRACE("***Waiting for mountThread to exit\n");
            sleep(1);
        }
        g_hash_table_destroy(this->itemsHash());
        TRACE("***Destructor:~local_monitor_c() complete\n");
    }

    void
    start_monitor(GtkTreeModel *treeModel, const gchar *path){
        this->startMonitor(treeModel, path, (void *)monitor_f);
        // start mountThread
        pthread_t mountThread;
        mountArg_[0] = (void *)this;
        mountArg_[1] = GINT_TO_POINTER(TRUE);
        mountArg_[2] = GINT_TO_POINTER(TRUE);
	gint retval = pthread_create(&mountThread, NULL, FstabMonitor<Type>::mountThreadF, (void *)mountArg_);
	if (retval){
	    ERROR("thread_create(): %s\n", strerror(retval));
	    //return retval;
	}
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
        if (Fstab<Type>::isMounted(path)) {
            gchar *key = Hash<Type>::get_hash_key(path, 10);
            if (!g_hash_table_lookup((GHashTable *)data, key)){
                // update the icon
                DBG("*** Send change signal for %s (now mounted)\n", (gchar *)path);
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
        if (!Fstab<Type>::isMounted(path)) {
            gchar *key = Hash<Type>::get_hash_key(path, 10);
            if (g_hash_table_lookup((GHashTable *)data, key)){
                // update the icon
                DBG("*** Send change signal for %s (now unmounted)\n", (gchar *)path);
                void *arg[] = { 
                    g_object_get_data(G_OBJECT(treeModel), "baseMonitor"),
                    (void *)path };
                Util<Type>::context_function(sendChangeSignal, arg);
            }
            g_free(key);
        }
        return retval;
    }

public:
    static void *
    mountThreadF(void *data){
        void **arg = (void **)data;
        auto baseMonitor = (BaseMonitor<Type> *)arg[0];
        g_object_set_data(G_OBJECT(baseMonitor->treeModel()), "baseMonitor", (void *)baseMonitor);
        // get initial md5sum
        gchar *sum = Util<Type>::md5sum("/proc/mounts");
        if (!sum) {
            DBG("Exiting mountThreadF() on md5sum error (sum)\n");
            g_free(data);
            return NULL;
        }
        DBG("FstabMonitor::mountThreadF(): initial md5sum=%s", sum);
        
	auto hash = getMountHash(NULL);
        while (arg[1]){// arg[1] is semaphore to thread
            usleep(250000);
            //sleep(1);
            TRACE("mountThreadF loop for arg=%p\n", data);
            gchar *newSum = Util<Type>::md5sum("/proc/mounts");
            if (!newSum){
                DBG("Exiting mountThreadF() on md5sum error (newSum)\n");
                g_hash_table_destroy(hash);
                g_free(sum);
                return NULL;
            }
            if (strcmp(newSum, sum)){
                WARN("new md5sum /proc/mounts = %s (%s)\n", newSum, sum);
                WARN("now we test whether icon update is necessary...\n");
                g_free(sum);
                sum = newSum;
                // Any new mounts?
                // Foreach item in itemsHash_ check
                // if (isMounted() and not in hash)
                // if so, then send change signal for gfile path. 
                // This should set the greenball.
                //DBG("thread itemshash=%p\n", itemsH);
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
        }
        g_free(sum);
        DBG("***now exiting mountThreadF()\n");
        g_hash_table_destroy(hash);
        arg[2] = NULL; // arg[2] is semaphore to calling thread.
        return NULL;
    }

private:
    
    

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
                    gchar *key = Hash<Type>::get_hash_key(items[i], 10);
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
	    ERROR("Cannot pipe from %s\n", command);
	    return NULL;
	}
        gchar *partuuid = g_path_get_basename(partuuidPath);
        gchar line[256];
        memset(line, 0, 256);
        gchar *partition = NULL;
            WARN("uuid2Partition(): looking for : \"%s\"\n", partuuid);
	while (fgets (line, 255, pipe) && !feof(pipe)) {
            WARN("uuid2Partition(): %s: %s\n", partuuid, line);
            if (!strstr(line, "->")) continue;
            if (!strstr(line, partuuid)) continue;
            WARN("uuid2Partition(): GOTCHA: %s\n", line);
            gchar **f = g_strsplit(line, "->", 2);
            if (strchr(f[1], '\n')) *strchr(f[1], '\n') = 0;
            g_strstrip(f[1]);
            gchar *basename = g_path_get_basename(f[1]);
            WARN("uuid2Partition(): GOTCHA: basename=%s\n", basename);
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
    
    static gboolean rm_func (GtkTreeModel *model,
				GtkTreePath *tpath,
				GtkTreeIter *iter,
				gpointer data){
	gchar *partuuid;
	gchar *path;
	gtk_tree_model_get (model, iter, 
		PATH, &path, 
		PARTUUID, &partuuid, 
		-1);  
	
	if (strcmp(partuuid, (gchar *)data)){
	    g_free(partuuid);
	    g_free(path);
	    return FALSE;
	}
	TRACE("removing %s (%s) from treemodel.\n", path, partuuid);
	GtkListStore *store = GTK_LIST_STORE(model);

	gtk_list_store_remove(store, iter);
	g_free(path);
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
                WARN("Received DELETED  (%d): \"%s\", \"%s\"\n", event, f, s);
                DBG("*** rm %s \n", f);
                gchar *devicePath = uuid2Partition(f);
                gchar *baseuuid = g_path_get_basename(f);
	        gchar *key = Hash<Type>::get_hash_key(devicePath, 10);
                if (p->itemsHash()) g_hash_table_remove(p->itemsHash(), key); 
                gtk_tree_model_foreach (p->treeModel(), rm_func, (gpointer) baseuuid); 
                g_free(devicePath);
                g_free(baseuuid);
                break;
            }
            case G_FILE_MONITOR_EVENT_CREATED:
            case G_FILE_MONITOR_EVENT_MOVED_IN:
            {
                WARN("Received  CREATED (%d): \"%s\", \"%s\"\n", event, f, s);
                gchar *devicePath = uuid2Partition(f);
                WARN("adding partition %s \n", devicePath);
		fsType = Fstab<Type>::fsType(devicePath);
                Fstab<Type>::addPartition(GTK_TREE_MODEL(p->store_), devicePath, fsType);
                
		g_hash_table_replace(p->itemsHash(), g_strdup(f), GINT_TO_POINTER(1));


                g_free(devicePath);
		g_free(fsType);

                break;
            }

            case G_FILE_MONITOR_EVENT_CHANGED:
                DBG("*** Received  CHANGED (%d): \"%s\", \"%s\"\n", event, f, s);
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
        DBG("redoIcon %s ...\n", path);
        gchar *key = Hash<Type>::get_hash_key(path, 10);
        if (!g_hash_table_lookup(this->itemsHash(), key)) {
            g_free(key);
            DBG("*** %s not in itemsHash\n", path);
            return FALSE; 
        }
        g_free(key);
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
        
        DBG("fstabmonitor currentPath \"%s\" == \"%s\"\n", currentPath, path);
        if (strcmp(path, currentPath)){
            g_free(currentPath);
            return FALSE;
        }
        g_free(currentPath);
        DBG("*** fstabmonitor currentPath %s\n", currentPath, path);
	
	GtkListStore *store = GTK_LIST_STORE(model);

        gboolean mounted = Fstab<Type>::isMounted(path);
        auto iconName = (mounted)?"drive-harddisk/NW/greenball/3.0/180":
            "drive-harddisk/NW/grayball/3.0/180";
        DBG("fstabmonitor stat_func(): iconname=%s\n", iconName);
        GdkPixbuf *pixbuf = Pixbuf<Type>::get_pixbuf(iconName,  GTK_ICON_SIZE_DIALOG);
	gtk_list_store_set (store, iter, 
                ICON_NAME, iconName,
                DISPLAY_PIXBUF, pixbuf,
                NORMAL_PIXBUF, pixbuf,
		-1);

	return TRUE;
    }


};
}
#endif

