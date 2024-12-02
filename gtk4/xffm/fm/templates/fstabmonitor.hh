#ifndef XF_FSTABMONITOR__HH
# define XF_FSTABMONITOR__HH
#ifdef HAVE_MNTENT_H
#define USE_MOUNTTHREAD
#endif

namespace xf
{
// Linux files:
// (/proc/mounts), /proc/partitions
// 
template <class DirectoryClass>
class FstabMonitor {
        
  pthread_cond_t waitSignal = PTHREAD_COND_INITIALIZER;
  pthread_mutex_t waitMutex = PTHREAD_MUTEX_INITIALIZER;
  void **mountArg_ = NULL; 
  char *path_;
  GridView<DirectoryClass> *gridView_p = NULL;

public:    

    FstabMonitor(GridView<LocalDir> *gridview)
    {   
        path_ = g_strdup(gridview->path());
        DBG("*** fstab_monitor started for LocalDir %s\n", path_);
        gridView_p = gridview;       
        setMountArg();

        pthread_t thread;
        pthread_create(&thread, NULL, mountThreadF1, (void *)mountArg_);
        pthread_detach(thread);
    }
    FstabMonitor(GridView<FstabDir> *gridview)
    {   
        path_ = g_strdup(gridview->path());
        DBG("*** fstab_monitor started for LocalDir %s\n", path_);
        gridView_p = gridview;       
        setMountArg();

        pthread_t thread;
        pthread_create(&thread, NULL, mountThreadF2, (void *)mountArg_);
        pthread_detach(thread);
    }
    ~FstabMonitor(void){
        // stop mountThread
        mountArg_[1] = NULL;
        DBG("fstab monitor cancelled for %s\n", path_);
        //sleep(1);
        g_free(path_);
    }

private:
    void setMountArg(void){
      mountArg_ = (void **)calloc(3, sizeof(void *));
      mountArg_[0] = (void *)gridView_p;
      mountArg_[1] = GINT_TO_POINTER(TRUE);
    }

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
          gchar buffer[1024];
          memset (buffer, 0, 1024);
          if (!fgets(buffer, 1023, pipe)){
             DBG("fgets(%s): %s\n", command, "no characters read.");
          }
          g_free(command);
          pclose(pipe);
          if (strlen(buffer)) return g_strdup(buffer);
          return NULL;
      }



    static GHashTable *getMntHash(GridView<DirectoryClass> *gridView_p){
      GHashTable *hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
      auto listModel = gridView_p->listModel();
      auto items = g_list_model_get_n_items (listModel);
      for (guint i=0; i<items; i++){
        auto info = G_FILE_INFO(g_list_model_get_item(listModel, i)); // GFileInfo
        auto path = Basic::getPath(info);
        if (!g_file_test(path, G_FILE_TEST_IS_DIR)){
          g_free(path);
          continue;
        }
        //if (FstabUtil::isInFstab(path) || FstabUtil::isMounted(path)){
        if (FstabUtil::isMounted(path)){
          g_hash_table_insert(hash, g_strdup(path), GINT_TO_POINTER(1));
          TRACE("getMntHash insert: %s\n", path);
        }
        g_free(path);

      }
      return hash;
    }

    static GHashTable *getFstabHash(GridView<DirectoryClass> *gridView_p){
      GHashTable *hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
      auto listModel = gridView_p->listModel();
      auto items = g_list_model_get_n_items (listModel);
      for (guint i=0; i<items; i++){
        auto info = G_FILE_INFO(g_list_model_get_item(listModel, i)); // GFileInfo
        auto path = Basic::getPath(info);
        if (!g_file_test(path, G_FILE_TEST_IS_DIR)){
          g_free(path);
          continue;
        }
        if (FstabUtil::isInFstab(path)){
          g_hash_table_insert(hash, g_strdup(path), GINT_TO_POINTER(1));
          TRACE("getFstabHash insert: %s\n", path);
        }
        g_free(path);

      }
      return hash;
    }


    static void *sendSignal_f(void *data){
       TRACE( "sendSignal_f\n");
       auto arg = (void **)data;
       auto monitor = G_FILE_MONITOR(arg[0]);
       auto *file = G_FILE(arg[1]);
       g_file_monitor_emit_event (monitor,
                   file, NULL, G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED);
                  // file, NULL, G_FILE_MONITOR_EVENT_CHANGED);
       // Seems that the event will manage reference to g_file
       // or at least the following unref does not wreak havoc...
        return NULL;
    }

    static void sendSignal(GridView<LocalDir> *gridView_p, GFileInfo *info){
      if (!gridView_p->monitor()){ //
        DBG("no fstab monitor active.\n");
        return;
      }
      auto file = Basic::getGfile(info);

      void *arg[] = {(void *)gridView_p->monitor(), (void *)file, NULL};
      arg[0] = (void *)gridView_p->monitor();
      arg[1] = (void *)file;
      Basic::context_function(sendSignal_f, arg);
    }

    static bool update_f(GridView<LocalDir> *gridView_p, GFileInfo *info, 
                           const char *path,
                           GHashTable * mntHash, GHashTable * fstabHash){
      DBG("*** update_f: %s\n", path);
        GFileInfo *updateInfo = NULL;
        if (FstabUtil::isMounted(path) && !g_hash_table_lookup(mntHash, path)){
          TRACE("update icon for mounted %s\n", path);
          g_hash_table_insert(mntHash, g_strdup(path), GINT_TO_POINTER(1));
          updateInfo = info;
        }
        else if (!FstabUtil::isMounted(path) &&  g_hash_table_lookup(mntHash, path)){
          TRACE("update icon for unmounted %s\n", path);
          g_hash_table_remove(mntHash, path);

          updateInfo = info;
        }
        else if (FstabUtil::isInFstab(path) && !g_hash_table_lookup(fstabHash, path)){
          g_hash_table_insert(fstabHash, g_strdup(path), GINT_TO_POINTER(1));
          TRACE("update icon for removed from fstab %s\n", path);
          updateInfo = info;
        }
        else if (!FstabUtil::isInFstab(path) && g_hash_table_lookup(fstabHash, path)){
          g_hash_table_remove(fstabHash, path);
          TRACE("update icon for added to fstab %s\n", path);
          updateInfo = info;
        }
        if (updateInfo) {
          sendSignal(gridView_p, updateInfo);
          return true;
        }
        return false;
        
    }

    static bool updateItem(GridView<LocalDir> *gridView_p, GFileInfo *info, 
                           const char *path,
                           GHashTable * mntHash, GHashTable * fstabHash){
        if (!g_file_test(path, G_FILE_TEST_IS_DIR)) return false;
        return update_f(gridView_p, info, path, mntHash, fstabHash);
    }

    static bool updateItem(GridView<LocalDir> *gridView_p, GFileInfo *info, 
                           const char *path,
                           GHashTable * fstabHash){
        return false;
    }

/*
    static bool findInStore(GridView<FstabDir> *gridView_p, const char *path){
        auto listModel = gridView_p->listModel();
        auto items = g_list_model_get_n_items (listModel);
        for (guint i=0; i<items; i++){
          auto info = G_FILE_INFO(g_list_model_get_item(listModel, i)); // GFileInfo
          auto itemPath = Basic::getPath(info);
          if (strcmp(path, itemPath) == 0){
            g_free(itemPath);
            return true;
          }
          g_free(itemPath);
        }
        return false; 
    }
*/
    static bool checkSumMnt(GridView<DirectoryClass> *gridView_p, char **sum){
      char *newSum = md5sum("/proc/mounts");
      if (strcmp(newSum, *sum)) {
        g_free(*sum);
        *sum = newSum;
        return true;
      }
      g_free(newSum);
      return false;
    }

    static bool checkSumPart(GridView<FstabDir> *gridView_p, char **sum){
      char *newSum = md5sum("/proc/partitions");
      if (strcmp(newSum, *sum)) {
        g_free(*sum);
        *sum = newSum;
        return true;
      }
      g_free(newSum);
      return false;
    }
 
    static bool checkSumPart(GridView<LocalDir> *gridView_p, char **sum){
      return false;
    }

    static void *
    mountThreadF1(void *data){
      DBG("***mountThreadF1\n");

      // initial hold your horses
        sleep(1);
        void **arg = (void **)data;

        auto gridView_p = (GridView<LocalDir> *)arg[0];

        // get initial md5sum
        char *sum = md5sum("/proc/mounts");
        char *sumPartitions = md5sum("/proc/partitions");
        if (!sum || !sumPartitions) {
            ERROR("fm/view/fstab/monitor::Exiting mountThreadF() on md5sum error (sum)\n");
            g_free(data);
            return NULL;
        }
        TRACE("FstabMonitor::mountThreadF(): initial md5sum=%s ", sum);

        auto mntHash = getMntHash(gridView_p);
        auto fstabHash = getFstabHash(gridView_p);
        auto selectionModel = gridView_p->selectionModel();
        auto listModel = gridView_p->listModel();
        while (arg[1]){
            usleep(250000);
            if (!arg[1])continue;
            //sleep(1); // slow motion
            TRACE("mountThreadF loop for arg=%p\n", data);
            bool dirChange = checkSumMnt(gridView_p, &sum);
            bool partChange = checkSumPart(gridView_p, &sumPartitions);
            if (!dirChange && !partChange) continue;
            if (partChange){ // Modify elements in xffm::fstab view.
              auto items = g_list_model_get_n_items (listModel);
              for (guint i=0; i<items; i++){
                auto info = G_FILE_INFO(g_list_model_get_item(listModel, i)); // GFileInfo
                auto path = Basic::getPath(info);
                updateItem(gridView_p, info, path, fstabHash);
                g_free(path);
              }
            }
            if (dirChange){ // Modify directory mount status.
              auto items = g_list_model_get_n_items (listModel);
              for (guint i=0; i<items; i++){
                auto info = G_FILE_INFO(g_list_model_get_item(listModel, i)); // GFileInfo
                auto path = Basic::getPath(info);
                updateItem(gridView_p, info, path, mntHash, fstabHash);
                g_free(path);
              }
            }
        }
        g_free(sum);
        g_free(sumPartitions);
        //DBG("***now exiting mountThreadF(%s)\n", _path);
        g_free(arg); 
        g_hash_table_destroy(mntHash);
        g_hash_table_destroy(fstabHash);
        return NULL;
    }

    static void *reload_f(void *data){
        auto dir = (const char *)data;
        Workdir<FstabDir>::setWorkdir(dir);
        return NULL;
    }

    static void *
    mountThreadF2(void *data){

      DBG("***mountThreadF2\n");
        void **arg = (void **)data;

        auto gridView_p = (GridView<FstabDir> *)arg[0];
        // get initial md5sum
        char *sum = md5sum("/proc/mounts");
        char *sumPartitions = md5sum("/proc/partitions");
        if (!sum || !sumPartitions) {
            ERROR("fm/view/fstab/monitor::Exiting mountThreadF() on md5sum error (sum)\n");
            g_free(data);
            return NULL;
        }
        TRACE("FstabMonitor::mountThreadF(): initial md5sum=%s ", sum);

        auto mntHash = getMntHash(gridView_p);
        auto fstabHash = getFstabHash(gridView_p);
        //auto _path = g_strdup(gridView_p->path());
        auto selectionModel = gridView_p->selectionModel();
        auto listModel = gridView_p->listModel();
        bool reload = false;
        while (arg[1]){
            usleep(250000);
            if (!arg[1])continue;
            //sleep(1); // slow motion
            TRACE("mountThreadF loop for arg=%p\n", data);
            bool dirChange = checkSumMnt(gridView_p, &sum);
            bool partChange = checkSumPart(gridView_p, &sumPartitions);
            if (!partChange && !dirChange) continue;
            DBG(" Reload fstab gridview\n");
            Basic::context_function(reload_f, (void *)_("Disk Mounter"));
        }
        g_free(sum);
        g_free(sumPartitions);
        //DBG("***now exiting mountThreadF(%s)\n", _path);
        //g_free(_path); 
        g_free(arg); 
        g_hash_table_destroy(mntHash);
        g_hash_table_destroy(fstabHash);
        return NULL;
    }

#if 0

   
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
    
#endif


};
}
#endif

