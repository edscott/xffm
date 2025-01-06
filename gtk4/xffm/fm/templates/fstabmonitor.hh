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
template <class Type>
class FstabMonitor {
        
  //pthread_cond_t waitCondition_ = PTHREAD_COND_INITIALIZER;
  //pthread_mutex_t waitMutex_ = PTHREAD_MUTEX_INITIALIZER;
  //pthread_mutex_t endMutex_ = PTHREAD_MUTEX_INITIALIZER;
  void **mountArg_ = NULL; 
  char *path_;
  GridView<Type> *gridView_ = NULL;

public:    
  GridView<Type> *gridView(void){ 
    return gridView_;
  }
  //pthread_cond_t *condition() {return &waitCondition_;}
  //pthread_mutex_t *mutex() {return &waitMutex_;}
  //pthread_mutex_t *endMutex() {return &endMutex_;}

    FstabMonitor(GridView<LocalDir> *gridview)
    {   
        path_ = g_strdup(gridview->path());
        TRACE("*** fstab_monitor started for LocalDir %s\n", path_);
        gridView_ = gridview;   
        setMountArg();

        pthread_t thread;
      Thread::threadCount(true,  &thread, "FstabMonitor");
        pthread_create(&thread, NULL, mountThreadF1, (void *)mountArg_);
        pthread_detach(thread);
      Thread::threadCount(false,  &thread, "FstabMonitor");
    }
    FstabMonitor(GridView<FstabDir> *gridview)
    {   
        path_ = g_strdup(gridview->path());
        TRACE("*** fstab_monitor started for LocalDir %s\n", path_);
        gridView_ = gridview;       
        setMountArg();

        pthread_t thread;
      Thread::threadCount(true,  &thread, "FstabMonitor2");
        pthread_create(&thread, NULL, mountThreadF2, (void *)mountArg_);
        pthread_detach(thread);
      Thread::threadCount(false,  &thread, "FstabMonitor2");
    }
    ~FstabMonitor(void){
        // stop mountThread (if still running)
        stopMonitor();
        TRACE("fstab monitor cancelled for %s\n", path_);
        //sleep(1);
        g_free(path_);
        // go ahead for mountThread to cleanup.
        pthread_t thread;
      Thread::threadCount(true,  &thread, "~FstabMonitor");
        pthread_create(&thread, NULL, cleanup, mountArg_);
        pthread_detach(thread);
      Thread::threadCount(false,  &thread, "~FstabMonitor");
        TRACE("FstabMonitor: destructor done...\n");        
    }
    static void *cleanup(void *data){
      TRACE("FstabMonitor cleanup thread...\n");        
      auto mountArg = (void **)data;
      bool wait = true;
      while (wait){
        //pthread_mutex_lock(endMutex());
        if (mountArg[2] != NULL) wait = false;
        //pthread_mutex_lock(endMutex());
        usleep(250000);
      }
      g_free(mountArg);
      TRACE("FstabMonitor cleanup done...\n");    
      return NULL;    
    }
    void stopMonitor(void){
        mountArg_[1] = NULL;
    }


private:
    void setMountArg(void){
      mountArg_ = (void **)calloc(3, sizeof(void *));
      mountArg_[0] = (void *) this;
      mountArg_[1] = GINT_TO_POINTER(TRUE);
      mountArg_[2] = NULL;
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



    static GHashTable *getMntHash(GridView<Type> *gridView_p){
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

    static GHashTable *getFstabHash(GridView<Type> *gridView_p){
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
       auto monitorObject = (FstabMonitor<LocalDir> *)arg[0];
       //if (!monitorObject->checkSerial()) {
       //  return NULL;;
       //}
       auto monitor = (GFileMonitor *)monitorObject->gridView()->monitor();
       auto *file = G_FILE(arg[1]);
       g_file_monitor_emit_event (monitor,
                   file, NULL, G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED);
       return NULL;
    }

    static void sendSignal(FstabMonitor<LocalDir> *monitorObject, GFileInfo *info){
      if (!monitorObject->gridView()->monitor()){ 
        DBG("no fstab monitor active.\n");
        //pthread_mutex_unlock(monitorObject->mutex());
        return;
      }
      
      auto file = Basic::getGfile(info);
      void *arg[] = {(void *)monitorObject, (void *)file, NULL};
      TRACE("thread send signal %s\n", Basic::getPath(info));
      Basic::context_function(sendSignal_f, arg);
    }

    static bool update_f(GridView<LocalDir> *gridView_p, GFileInfo *info, 
                           const char *path,
                           GHashTable * mntHash, GHashTable * fstabHash){
      TRACE("*** update_f: %s\n", path);
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
          return true;
        }
        return false;
        
    }

    static bool checkSumMnt(char **sum){
      char *newSum = md5sum("/proc/mounts");
      if (strcmp(newSum, *sum)) {
        g_free(*sum);
        *sum = newSum;
        return true;
      }
      g_free(newSum);
      return false;
    }

    static bool checkSumPart(char **sum){
      char *newSum = md5sum("/proc/partitions");
      if (strcmp(newSum, *sum)) {
        g_free(*sum);
        *sum = newSum;
        return true;
      }
      g_free(newSum);
      return false;
    }
 

    static void *contextMonitor(void *data){
        auto monitorObject = (FstabMonitor<LocalDir> *)data;
        auto gridView_p = monitorObject->gridView();
        if (!Child::validGridView(gridView_p)) return GINT_TO_POINTER(1);

        auto mntHash = getMntHash(gridView_p);
        auto fstabHash = getFstabHash(gridView_p);
        //auto selectionModel = gridView_p->selectionModel();

        auto listModel = gridView_p->listModel();
        auto items = g_list_model_get_n_items (listModel);
        for (guint i=0; i<items; i++){
            auto info = G_FILE_INFO(g_list_model_get_item(listModel, i)); // GFileInfo
            auto path = Basic::getPath(info);
            if (!g_file_test(path, G_FILE_TEST_IS_DIR)) continue;
            if (update_f(gridView_p, info, path, mntHash, fstabHash)){
              sendSignal(monitorObject, info);
            } 
            g_free(path);
        }
        g_hash_table_destroy(mntHash);
        g_hash_table_destroy(fstabHash);
        return NULL;
    }

    static void *
    mountThreadF1(void *data){
        void **arg = (void **)data;
        //sleep(1); // race condition check...
        auto monitorObject = (FstabMonitor<LocalDir> *)arg[0];
        auto gridView_p = monitorObject->gridView();

        TRACE("***mountThreadF1 for gridview_p %p\n", gridView_p);
        // get initial md5sum
        char *sum = md5sum("/proc/mounts");
        char *sumPartitions = md5sum("/proc/partitions");
        if (!sum || !sumPartitions) {
            DBG("Error:: Exiting mountThreadF2(%p) on md5sum error (sum)\n", gridView_p);
            g_free(sum);
            g_free(sumPartitions);
            return NULL;
        }
        TRACE("FstabMonitor::mountThreadF(): initial md5sum=%s ", sum);

    
        while (arg[1]){
          usleep(250000);
          if (!arg[1])continue;
          if (!checkSumMnt(&sum)) continue;    
          // This is sent to main context to avoid race with gridview invalidation.
          if (Basic::context_function(contextMonitor, data) != NULL) break;
        }
        g_free(sum);
        g_free(sumPartitions);
        arg[2] = GINT_TO_POINTER(1);
        TRACE("******* mountThreadF1 all done for gridview %p.\n", gridView_p);
        return NULL;
    }

    static void *reload_f(void *data){
        auto dir = (const char *)data;
        Workdir<FstabDir>::setWorkdir(dir);
        return NULL;
    }

    static void *
    mountThreadF2(void *data){
        void **arg = (void **)data;
        //sleep(1); // race condition check...
        auto monitorObject = (FstabMonitor<FstabDir> *)arg[0];
        auto gridView_p = monitorObject->gridView();

        TRACE("***mountThreadF2 for gridview_p %p\n", gridView_p);
        // get initial md5sum
        char *sum = md5sum("/proc/mounts");
        char *sumPartitions = md5sum("/proc/partitions");
        if (!sum || !sumPartitions) {
            DBG("Error:: Exiting mountThreadF2(%p) on md5sum error (sum)\n", gridView_p);
            g_free(sum);
            g_free(sumPartitions);
            return NULL;
        }
        TRACE("FstabMonitor::mountThreadF(): initial md5sum=%s ", sum);

        Child::lockGridView("mountThreadF2");
        if (!Child::validGridView(gridView_p)) {
            Child::unlockGridView();
            g_free(sum);
            g_free(sumPartitions);
            TRACE("***abort2 mountThreadF2 for gridview_p %p\n", gridView_p);
            return NULL;
        }
        auto mntHash = getMntHash(gridView_p);
        auto fstabHash = getFstabHash(gridView_p);
        auto selectionModel = gridView_p->selectionModel();
        auto listModel = gridView_p->listModel();
        Child::unlockGridView();
        
        bool reload = false;
        while (arg[1]){
            usleep(250000);
            if (!arg[1])continue;
            
            bool dirChange = checkSumMnt(&sum);
            bool partChange = checkSumPart(&sumPartitions);
            if (!partChange && !dirChange) continue;
            TRACE(" Reload fstab gridview\n");
            Child::lockGridView("mountThreadF2-2");
            if (!Child::validGridView(gridView_p)) {
              Child::unlockGridView();
              TRACE("***abort3 mountThreadF2 for gridview_p %p\n", gridView_p);
              break;
            }
            Child::unlockGridView();

            Basic::context_function(reload_f, (void *)_("Disk Mounter"));
            // After reload, gridView ceases to be valid.
            // Another monitor will be reloaded with new gridview.
            break;
        }
        g_free(sum);
        g_free(sumPartitions);
        g_hash_table_destroy(mntHash);
        g_hash_table_destroy(fstabHash);
        arg[2] = GINT_TO_POINTER(1);
        TRACE("******* mountThreadF2 all done for gridview %p.\n", gridView_p);
        return NULL;
    }

    ///////////////////////////////////////////////////
    


    ///////////////////////////////////////////////////

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

