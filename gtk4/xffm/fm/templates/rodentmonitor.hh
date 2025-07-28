#ifndef RODENT_MONITOR__HH
# define RODENT_MONITOR__HH

// not quite working yet...

namespace xf
{
template <class Type>
class RodentMonitor {
        
  //pthread_cond_t waitCondition_ = PTHREAD_COND_INITIALIZER;
  //pthread_mutex_t waitMutex_ = PTHREAD_MUTEX_INITIALIZER;
  //pthread_mutex_t endMutex_ = PTHREAD_MUTEX_INITIALIZER;
  char *path_;
  GridView<Type> *gridview_ = NULL;
  void **rodentArg_ = NULL; 
  GDateTime *dateTime_ = NULL;


public:  
  void dateTime(GDateTime *value){
    if (dateTime_) g_object_unref(G_OBJECT(dateTime_));
    dateTime_ = value;
  }
  GDateTime *dateTime(void){return dateTime_;}       
  GridView<Type> *gridview(void){ return gridview_; }

  //pthread_cond_t *condition() {return &waitCondition_;}
  //pthread_mutex_t *mutex() {return &waitMutex_;}
  //pthread_mutex_t *endMutex() {return &endMutex_;}

    RodentMonitor(void *data)
    {   
        gridview_ = ((GridView<Type> *)data);
        path_ = g_strdup(gridview_->path());
        DBG("*** rodent_monitor started for LocalDir %s\n", path_);
        setRodentArg();

        pthread_t thread;
      Thread::threadCount(true,  &thread, "RodentMonitor");
        pthread_create(&thread, NULL, rodentThreadF1, (void *)rodentArg_);
        pthread_detach(thread);
      Thread::threadCount(false,  &thread, "RodentMonitor");
    }

    ~RodentMonitor(void){
        // stop mountThread (if still running)
        stopMonitor();
        DBG("RodentMonitor cancelled for %s\n", path_);
        //sleep(1);
        g_free(path_);
        // go ahead to cleanup.
        pthread_t thread;
      Thread::threadCount(true,  &thread, "~RodentMonitor");
        pthread_create(&thread, NULL, cleanup, rodentArg_);
        pthread_detach(thread);
      Thread::threadCount(false,  &thread, "~RodentMonitor");
        DBG("RodentMonitor: destructor done...\n");        
    }

  private:
    static void *cleanup(void *data){
      DBG("RodentMonitor cleanup thread...\n");        
      auto rodentArg = (void **)data;
      bool wait = true;
      while (wait){
        //pthread_mutex_lock(endMutex());
        if (rodentArg[2] != NULL) wait = false;
        //pthread_mutex_lock(endMutex());
        usleep(250000);
      }
      g_free(rodentArg);
      DBG("RodentMonitor cleanup done...\n");    
      return NULL;    
    }

    void stopMonitor(void){
        rodentArg_[1] = NULL;
    }


private:
    void setRodentArg(void){
      rodentArg_ = (void **)calloc(3, sizeof(void *));
      rodentArg_[0] = (void *) this;
      rodentArg_[1] = GINT_TO_POINTER(TRUE);
      rodentArg_[2] = NULL;
    }

    static void *
    rodentThreadF1(void *data){
        void **arg = (void **)data;
        //sleep(1); // race condition check...
        auto monitorObject = (RodentMonitor<Type> *)arg[0];
        auto gridview_p = monitorObject->gridview();
        const char *path = gridview_p->path();

        struct stat st;
        if (stat(path, &st) != 0){
          arg[2] = GINT_TO_POINTER(1);
          return NULL;
        }
            
        DBG("oldtime %s  = %ld\n", path,  st.st_mtime);
        DBG("***rodentThreadF1 for gridview_p %p (%s)\n", gridview_p, path);

        while (arg[1]){
          struct stat newSt;
          usleep(250000);
          if (!arg[1])continue;
          if (stat(path, &newSt) != 0) break;
          if (st.st_mtime == newSt.st_mtime) continue;
          memcpy(&st, &newSt, sizeof(struct stat));
              
          DBG("checkTime has changed...%ld\n", st.st_mtime);
          // This is sent to main context to avoid race with gridview invalidation.
          if (Basic::context_function(contextMonitor, data) != NULL) break;
          if (contextMonitor((void *)monitorObject) != NULL) break;
        }
        arg[2] = GINT_TO_POINTER(1);
        DBG("******* mountThreadF1 all done for gridview %p.\n", gridview_p);
        return NULL;
    }

    static void *contextMonitor(void *data){
        auto monitorObject = (RodentMonitor<Type> *)data;
        auto gridview_p = monitorObject->gridview();
        if (!Child::validGridView(gridview_p)) return GINT_TO_POINTER(1);
        auto listModel = gridview_p->listModel();
        auto items = g_list_model_get_n_items (listModel);
        DBG("contextMonitor = %p items = %d\n", data, items);
        for (guint i=0; i<items; i++){
            auto info = G_FILE_INFO(g_list_model_get_item(listModel, i)); // GFileInfo
            auto path = Basic::getPath(info);

            // removed?
            if (g_file_test(path, G_FILE_TEST_EXISTS)){
              DBG("%s is gone.\n", path);
              continue;
            }
            struct stat st;
            if (stat(path, &st) != 0) continue;
            auto mtime = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(info), "st_mtime"));
            DBG("mtimes: %ld ++ %ld\n", mtime,st.st_mtime); 
            // changed?
            if (mtime != st.st_mtime){
              DBG("%s has changed.\n", path);
              continue;
            }
        }
        // for new items, we need to scan directory and
        // check if item is in listStore (not listModel)
        // We will use a hash table.
        return NULL;
    }

/*    static void *reload_f(void *data){
        auto dir = (const char *)data;
        Workdir<Type>::setWorkdir(dir);
        return NULL;
    }*/

};
}
#endif

