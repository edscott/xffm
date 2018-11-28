#ifndef XF_FSTABMONITOR__HH
# define XF_FSTABMONITOR__HH
#include "fm/view/base/basemonitor.hh"
namespace xf
{
template <class Type> class BaseMonitor;
template <class Type> class Fstab;
// Linux files:
// (/proc/mounts), /proc/partitions
// 
template <class Type>
class FstabMonitor: public BaseMonitor<Type> {
    // Please note, sending signal to monitor avoid race condition if
    // this thread tries to do more than this...
    static void *sendChangeSignal(void *data){
       auto arg = (void **)data;
       auto baseMonitor = (BaseMonitor<Type> *)arg[0];
       auto path = (gchar *)arg[1];
       GFile *child = g_file_new_for_path (path);
       g_free(path);
       g_file_monitor_emit_event (baseMonitor->monitor(),
                   child, NULL, G_FILE_MONITOR_EVENT_CHANGED);
        //? g_object_unref(child);
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

    /*static void // iterates on items hashtable
    checkIfMounted_f(gpointer key,
               gpointer value,
               gpointer data){

        // Mounted but not in mounts hash:
        if (Fstab<Type>::isMounted((const gchar *)value) 
             &&   !g_hash_table_lookup((GHashTable *)data, key)){
            DBG("*** Send change signal for %s (now mounted)\n", (const gchar *)value);
            // Do this with a g_add_timeout
        }
    }
    static void // iterates on /proc/mounts hashtable
    checkIfNotMounted_f(gpointer key,
               gpointer value,
               gpointer data){
        // Not mounted but in mounts hash:
        if (!Fstab<Type>::isMounted((const gchar *)value) 
                && g_hash_table_lookup((GHashTable *)data, key)){
            DBG("*** Send change signal for %s (now unmounted)\n", (const gchar *)value);
            // Do this with a g_add_timeout
        }

    }*/
public:    
    FstabMonitor(GtkTreeModel *treeModel, BaseView<Type> *baseView):
        BaseMonitor<Type>(treeModel, baseView)
    {       
    }
    ~FstabMonitor(void){
        TRACE("Destructor:~local_monitor_c()\n");
    }
    void
    start_monitor(GtkTreeModel *treeModel, const gchar *path){
        this->startMonitor(treeModel, path, (void *)monitor_f);
    }

    static void *
    mountThreadF(void *data){
        void **arg = (void **)data;
        auto baseMonitor = (BaseMonitor<Type> *)arg[0];
        g_object_set_data(G_OBJECT(baseMonitor->treeModel()), "baseMonitor", (void *)baseMonitor);
        auto itemsH = (GHashTable *)arg[2];
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
                DBG("thread itemshash=%p\n", itemsH);
                gtk_tree_model_foreach(baseMonitor->treeModel(), checkIfMounted, (void *)hash);
                //g_hash_table_foreach(itemsH, checkIfMounted_f, (void *)hash);
                //
                // Any new umounts?
                // Foreach item in hash, check if 
                // if (!isMounted(item) && in itemsHash_)
                // then
                //     sendSignal change for gfile(item)
                gtk_tree_model_foreach(baseMonitor->treeModel(), checkIfNotMounted, (void *)hash);
                //g_hash_table_foreach(hash, checkIfNotMounted_f, (void *)itemsH);
                //
                // Update hash.
                hash = getMountHash(hash);
            }
            /*
	    static gboolean sendChangeSignal(void *data){
                auto path = (const gchar *)data;
		GFile *child = g_file_new_for_path (path);
		g_file_monitor_emit_event (baseMonitor->monitor(),
                           child, NULL, G_FILE_MONITOR_EVENT_CHANGED);
		//? g_object_unref(child);
	    }
            */
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
        DBG("***now exiting mountThreadF()\n");
        g_hash_table_destroy(hash);
        // g_free(data);
        arg[3] = NULL; // arg[3] is semaphore to calling thread.
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
/*
     static GList *getMountPaths(void){
	// Get first two items per line of /proc/mounts
	// and add both to list.
        GList *list = NULL;
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
            if (g_path_is_absolute(items[0]) && g_path_is_absolute(items[1])){
                for (gint i=0; i<2; i++) list = g_list_prepend(list, items[i]);
            }
        }
        fclose(mounts);
	return list;
    }
 */   
    static gchar *
    uuid2Partition(const gchar *partuuid){
        const gchar *command = "ls -l /dev/disk/by-partuuid";
	FILE *pipe = popen (command, "r");
	if(pipe == NULL) {
	    ERROR("Cannot pipe from %s\n", command);
	    return NULL;
	}
        gchar line[256];
        memset(line, 0, 256);
        gchar *partition = NULL;
	while (fgets (line, 255, pipe) && !feof(pipe)) {
            TRACE("%s: %s\n", partuuid, line);
            if (strstr(line, "->") && strstr(line, partuuid)) {
                if (strchr(line, '\n')) *strchr(line, '\n') = 0;
                partition = g_strdup_printf("/dev/%s", strrchr(line, '/')+1);
                g_strstrip(partition);
                if (strcmp("/dev",partition)==0) {
                    g_free(partition);
                    partition=NULL;
                }
                break;
            }
	}
        pclose (pipe);
	return partition;

    }
    
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
        auto p = (FstabMonitor<Type> *)data;
        gchar *path;
        gchar *fsType;
        switch (event){
            case G_FILE_MONITOR_EVENT_DELETED:
            case G_FILE_MONITOR_EVENT_MOVED_OUT:
                TRACE("Received DELETED  (%d): \"%s\", \"%s\"\n", event, f, s);
                TRACE("rm %s \n", f);
                p->remove_item(f);
                break;
            case G_FILE_MONITOR_EVENT_CREATED:
            case G_FILE_MONITOR_EVENT_MOVED_IN:
                TRACE("Received  CREATED (%d): \"%s\", \"%s\"\n", event, f, s);
                path = uuid2Partition(f);
                TRACE("adding partition %s\n", path);
		fsType = Fstab<Type>::fsType(path);
                Fstab<Type>::addPartition(GTK_TREE_MODEL(p->store_), path, fsType);
                
		g_hash_table_replace(p->itemsHash(), g_strdup(f), GINT_TO_POINTER(1));
		g_free(fsType);
                break;

            case G_FILE_MONITOR_EVENT_CHANGED:
                TRACE("Received  CHANGED (%d): \"%s\", \"%s\"\n", event, f, s);
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



};
}
#endif

