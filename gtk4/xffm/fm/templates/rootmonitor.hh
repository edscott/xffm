#ifndef ROOTMONITOR_HH
#define ROOTMONITOR_HH
namespace xf {

  template <class Type> class RootMonitor {

    void **args = NULL;
    char *reloadPath_ = NULL;
    GtkWidget *child_ = NULL;
    GridView<Type> *gridView_ = NULL;
    public:
      RootMonitor(GridView<Type> *gridview, const char *reloadPath){
        TRACE("*** RootMonitor(GridView<Type>) constructor\n");
        reloadPath_ = g_strdup(reloadPath);
        child_ = gridview->child();
        gridView_ = gridview;
        DBG("*** RootMonitor(GridView<Type>) constructor, child=%p\n",child_);

        args = (void **)calloc(3, sizeof(void *));
        args[0] = (void *) this;
        args[1] = (void *) gridView_;
        args[2] = GINT_TO_POINTER(TRUE);
        pthread_t thread;
        pthread_create(&thread, NULL, threadF1, (void *)args);
        pthread_detach(thread);
      }
      ~RootMonitor(){
        args[2] = (NULL);
        TRACE("*** root monitor is gone.\n");
      }

      GtkWidget *child(void){return child_;}
      char *reloadPath(void){return reloadPath_;}
      GridView<Type> *gridView(void) {return gridView_;}

    private:

      
    static void *
    threadF1(void *data){
      char **sum = (char **)calloc(5, sizeof(char *));
      char **files = (char **)calloc(5, sizeof(char *));
       
      auto bookmarks_p = (Bookmarks *) bookmarksObject; // bookmarksObject is global
      files[0] = bookmarks_p->getBookmarksFilename();
      files[1] = EfsResponse<Type>::efsKeyFile(); // static function
      files[2] = g_strdup("/proc/mounts");
      files[3] = g_strdup("/proc/partitions");



        void **arg = (void **)data;
        auto monitorObject = (RootMonitor<Type> *)arg[0];
        auto gridView_p = (GridView<Type> *)arg[1];
        GtkWidget *child = monitorObject->child();
        char *path = g_strdup(gridView_p->path());

        DBG("***Root Monitor %p started\n", gridView_p);

loop:
        // gridView may change.
        if (!arg[2] || !Child::validGridView(gridView_p)) {
          TRACE("*** RootMonitor, child %p --> %p\n",child,gridView_p->child());
          TRACE("*** RootMonitor, gridView has changed from %p\n", gridView_p);
done:
          DBG("*** root monitor %p now has exited.\n", gridView_p);
          g_free(path);
          g_strfreev(sum);
          g_strfreev(files);
          g_free(data);
          return NULL;
        }
       
        // get initial md5sum
        auto q = sum;
        for (auto p=files; p && *p && q && *q; p++, q++){
          g_free(*q);
          *q = Basic::md5sum(*p);
        }


        for (auto q=sum; q && *q; q++){
          if (*q == NULL) {
            ERROR_("Error:: Exiting threadF1(%p) on md5sum error (sum)\n", gridView_p);
            goto done;
          }
        }

    
        while (arg[2]){
          usleep(250000);
          if (!arg[1])continue;

          bool test[5];
          int k=0;
          for (auto p=files; p && *p; p++, k++){
            test[k] = Basic::checkSumFile(*p, &(sum[k]));
          }
           
          DBG("tests: %d %d %d %d\n", test[0],test[1],test[2],test[3]);
          if (!test[0] && !test[1] && !test[2] && !test[3]) continue;

          // This is sent to main context to avoid race with gridView invalidation.
          TRACE("***checksum changed. context function.with %p, %p child=%p\n",
              arg[0], arg[1],child);
          Basic::context_function(reload_f, arg[0]);
          break; // We break because we need to update checksums.
        }

        sleep(1);
        TRACE("*** valid gridView (%p) = %d\n",
            gridView_p,Child::validGridView(gridView_p)); 
        goto loop;
    }
     
    static void *reload_f(void *data){
      auto monitorObject = (RootMonitor<Type> *) data;
      TRACE("*** reload with path=%s, child = %p\n", 
                 monitorObject->reloadPath(), monitorObject->child());
      Workdir<Type>::setWorkdir(monitorObject->reloadPath(), monitorObject->child());
      return NULL;
    }
  };

}
#endif
