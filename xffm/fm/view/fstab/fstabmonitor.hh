#ifndef XF_FSTABMONITOR__HH
# define XF_FSTABMONITOR__HH
#include "fm/view/base/basemonitor.hh"
namespace xf
{
template <class Type> class Fstab;
// Linux files:
// (/proc/mounts), /proc/partitions
// 
template <class Type>
class FstabMonitor: public BaseMonitor<Type> {

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

private:

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
                
		g_hash_table_replace(p->itemsHash_, g_strdup(f), GINT_TO_POINTER(1));
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

