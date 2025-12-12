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
  GHashTable *mountHash_ = NULL;

public:    
  GridView<Type> *gridView(void){ 
    return gridView_;
  }
  GHashTable *mountHash(void){return mountHash_;}
  //pthread_cond_t *condition() {return &waitCondition_;}
  //pthread_mutex_t *mutex() {return &waitMutex_;}
  //pthread_mutex_t *endMutex() {return &endMutex_;}


    FstabMonitor(GridView<LocalDir> *gridview)
    {   
        mountHash_ = xf::FstabUtil::createMountHash();    
        path_ = g_strdup(gridview->path());
        TRACE("*** fstab_monitor started for LocalDir %s\n", path_);
        gridView_ = gridview;   

        mountArg_ = (void **)calloc(3, sizeof(void *));
        mountArg_[0] = (void *) this;
        mountArg_[1] = (void *) gridView_;
        mountArg_[2] = GINT_TO_POINTER(TRUE);


        pthread_t thread;
      Thread::threadCount(true,  &thread, "FstabMonitor");
//        pthread_create(&thread, NULL, waitThread1, (void *)mountArg_);
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
        TRACE("*** fstab monitor cancelled for %s\n", path_);
        //sleep(1);
        g_free(path_);
        // go ahead for mountThread to cleanup.
        pthread_t thread;
      Thread::threadCount(true,  &thread, "~FstabMonitor");
        pthread_create(&thread, NULL, cleanup, mountArg_);
        pthread_detach(thread);
      Thread::threadCount(false,  &thread, "~FstabMonitor");
        TRACE("FstabMonitor: destructor done...\n");    
      if (mountHash_) g_hash_table_destroy( mountHash_);  
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

#if 0
/*    static void *sendSignal_f(void *data){
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
    }*/

    static void sendSignal(FstabMonitor<LocalDir> *monitorObject, GFileInfo *info){
      // This is already running in context mode.
      if (!monitorObject->gridView()->monitor()){ 
        ERROR_("no fstab monitor active.\n");
        //pthread_mutex_unlock(monitorObject->mutex());
        return;
      }
      auto monitor = (GFileMonitor *)monitorObject->gridView()->monitor();
      auto file = Basic::getGfile(info);
      g_file_monitor_emit_event (monitor,
                   file, NULL, G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED);
    /*  
      auto file = Basic::getGfile(info);
      void *arg[] = {(void *)monitorObject, (void *)file, NULL};
      TRACE("thread send signal %s\n", Basic::getPath(info));
      TRACE("Basic::context_function for sendSignal_f\n");
      Basic::context_function(sendSignal_f, arg);*/
    }
#endif

    static bool update_f(GridView<LocalDir> *gridView_p, GFileInfo *info, 
                           const char *path,
                           GHashTable * mntHash, GHashTable * fstabHash){
      bool isInMountHash = GPOINTER_TO_INT(g_hash_table_lookup(mntHash, path));
      bool isMounted = FstabUtil::isMounted(path);

      TRACE("*** update_f: %s isMounted=%d isInMountHash=%d\n", path);
        GFileInfo *updateInfo = NULL;
        if (isMounted && !isInMountHash){
          TRACE("update_f:update icon for mounted %s\n", path);
          g_hash_table_insert(mntHash, g_strdup(path), GINT_TO_POINTER(1));
          updateInfo = info;
        }
        else if (!isMounted && isInMountHash){
          TRACE("update_f:update icon for unmounted %s\n", path);
          g_hash_table_remove(mntHash, path);
          updateInfo = info;
        }

        else if (fstabHash && FstabUtil::isInFstab(path) && !g_hash_table_lookup(fstabHash, path)){
          g_hash_table_insert(fstabHash, g_strdup(path), GINT_TO_POINTER(1));
          TRACE("update icon for removed from fstab %s\n", path);
          updateInfo = info;
        }
        else if (fstabHash && !FstabUtil::isInFstab(path) && g_hash_table_lookup(fstabHash, path)){
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
      char *newSum = Basic::md5sum("/proc/mounts");
        TRACE("md5sum compare %s / %s\n", *sum, newSum);
      if (strcmp(newSum, *sum)) {
        TRACE("md5sum mismatch %s / %s\n", *sum, newSum);
        g_free(*sum);
        *sum = newSum;
        return true;
      }
      g_free(newSum);
      return false;
    }

    static bool checkSumPart(char **sum){
      char *newSum = Basic::md5sum("/proc/partitions");
      if (strcmp(newSum, *sum)) {
        g_free(*sum);
        *sum = newSum;
        return true;
      }
      g_free(newSum);
      return false;
    }
 

    static void *contextMonitor(void *data){
        void **arguments = (void **)data;
        TRACE("*** now at contextMonitor arg[0]=%p arg[1]=%p\n",arguments[0],arguments[1]); 
        auto monitorObject = (FstabMonitor<LocalDir> *)arguments[0];
        //auto monitorObject = (FstabMonitor<LocalDir> *)data;
        auto gridView_p =(GridView<LocalDir> *)arguments[1];
        if (!Child::validGridView(gridView_p)) {
          TRACE("*** contextMonitor: %p is not valid gridview\n", gridView_p); 
          return GINT_TO_POINTER(1);
        }

        auto mntHash = getMntHash(gridView_p);
        //auto fstabHash = getFstabHash(gridView_p);
        GHashTable *fstabHash = NULL;

        auto listModel = gridView_p->listModel();
        auto items = g_list_model_get_n_items (listModel);
        for (guint i=0; i<items; i++){
            auto info = G_FILE_INFO(g_list_model_get_item(listModel, i)); // GFileInfo
            auto path = Basic::getPath(info);
            if (!g_file_test(path, G_FILE_TEST_IS_DIR)) continue;
            if (update_f(gridView_p, info, path, monitorObject->mountHash(), NULL)){
              monitorObject->updateItem(gridView_p, path);
              // gtk4 broke this: sendSignal(monitorObject, info);
            } 
            g_free(path);
        }
        if (mntHash) g_hash_table_destroy(mntHash);
        //g_hash_table_destroy(fstabHash);
        return GINT_TO_POINTER(1);
    }

 /*   static void *waitThread1(void *data){
        void **arg = (void **)data;
        auto monitorObject = (FstabMonitor<LocalDir> *)arg[0];
        auto gridView_p = monitorObject->gridView();
        char *path = g_strdup(gridView_p->path());

        pthread_t thread;

        pthread_create(&thread, NULL, mountThreadF1, (void *)mountArg_);

    }*/

    static void *
    mountThreadF1(void *data){
        void **arg = (void **)data;
        auto monitorObject = (FstabMonitor<LocalDir> *)arg[0];
        auto gridView_p = (GridView<LocalDir> *)arg[1];
        GtkWidget *child = gridView_p->child();
        char *path = g_strdup(gridView_p->path());

        TRACE("***mountThreadF1 for gridview_p %p \n", gridView_p);

        // get initial md5sum
        char *sum = NULL;
loop:
        // gridview may change.
        if (!Child::validGridView(gridView_p)) {
          TRACE("*** fstab monitor, child %p --> %p\n",child,gridView_p->child());
          TRACE("*** fstab monitor, gridView has changed from %p\n", gridView_p);
          g_free(path);
          g_free(sum);
          return NULL;
        }
        g_free(sum);
        sum = Basic::md5sum("/proc/mounts");
        if (!sum) {
            ERROR_("Error:: Exiting mountThreadF2(%p) on md5sum error (sum)\n", gridView_p);
            g_free(path);
            g_free(sum);
            return NULL;
        }
        TRACE("FstabMonitor::mountThreadF(): initial md5sum=%s ", sum);

    
        while (arg[1]){
          usleep(250000);
          if (!arg[1])continue;
          if (!checkSumMnt(&sum)) continue;    
          // This is sent to main context to avoid race with gridview invalidation.
          TRACE("***checksum changed. context function.with %p, %p\n",monitorObject, gridView_p);
          TRACE("***checksum changed. context function.with %p, %p\n",arg[0], arg[1]);
          auto retval = Basic::context_function(contextMonitor, data);
          TRACE("***mountThreadF1(): return value from context_function is %p\n");
          if (retval != NULL) break; // We break because we need to update checksums.
        }

        sleep(1);
        TRACE("*** valid gridview (%p) = %d\n",
            gridView_p,Child::validGridView(gridView_p)); 
        goto loop;
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
        char *sum = Basic::md5sum("/proc/mounts");
        char *sumPartitions = Basic::md5sum("/proc/partitions");
        if (!sum || !sumPartitions) {
            ERROR_("Error:: Exiting mountThreadF2(%p) on md5sum error (sum)\n", gridView_p);
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
      TRACE("Basic::context_function for reload_f (fstab)\n");

            Basic::context_function(reload_f, (void *)"Disk Mounter");
            // After reload, gridView ceases to be valid.
            // Another monitor will be reloaded with new gridview.
            break;
        }
        g_free(sum);
        g_free(sumPartitions);
        if (mntHash) g_hash_table_destroy(mntHash);
        if (fstabHash) g_hash_table_destroy(fstabHash);
        arg[2] = GINT_TO_POINTER(1);
        TRACE("*** mountThreadF2 all done for gridview %p.\n", gridView_p);
        return NULL;
    }

    // Should be run in main context, for now it is called from
    //  contextMonitor which is called from main context.
    void updateItem(GridView<LocalDir> *gridView_p, const char *path){
     // from gridview.hh monitor function...
      if (!Child::validGridView(gridView_p)) return;
      guint positionF;
      auto model = gridView_p->listModel();  // Model to find
      auto store = gridView_p->store();      // Store to remove/add
      auto child = gridView_p->child(); 

      TRACE("updateItem \"%s\"\n", path);
      auto found = LocalDir::findPositionModel2(model, path, &positionF);
      if (found) {
         Child::incrementSerial(child);
         // Position in store not necesarily == to model (filter)
         LocalDir::findPositionStore(store, path, &positionF);
         g_list_store_remove(store, positionF);
         TRACE("removing %s\n",path);
         Child::incrementSerial(child);
         LocalDir::insert(store, path, false);                        
         TRACE("inserting %s\n",path);
      } else {
        TRACE("%s not found!\n", path);
      }
    }
};
}
#endif

