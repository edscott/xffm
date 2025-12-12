#ifndef ROOTMONITOR_HH
#define ROOTMONITOR_HH
namespace xf {

  template <class Type> class RootMonitor {

    void **args = NULL;
    public:
      RootMonitor(GridView<FstabDir> *gridView){
        TRACE("*** RootMonitor(GridView<FstabDir>) constructor\n");
        args = (void **)calloc(3, sizeof(void *));
        args[0] = (void *) this;
        args[1] = (void *) gridView;
        args[2] = GINT_TO_POINTER(TRUE);
        pthread_t thread;
        pthread_create(&thread, NULL, threadF1, (void *)args);
        pthread_detach(thread);
      }

      RootMonitor(GridView<LocalDir> *gridView){
        TRACE("*** RootMonitor(GridView<LocalDir>) constructor\n");
        args = (void **)calloc(3, sizeof(void *));
        args[0] = (void *) this;
        args[1] = (void *) gridView;
        args[2] = GINT_TO_POINTER(TRUE);
        pthread_t thread;
        pthread_create(&thread, NULL, threadF1, (void *)args);
        pthread_detach(thread);
      }

      ~RootMonitor(){
        args[2] = (NULL);
        TRACE("*** root monitor is gone.\n");
      }

    private:

 
    static bool checkSumMnt(const char *file, char **sum){
      char *newSum = Basic::md5sum(file);
      if (*sum == NULL) {*sum = newSum; return false;}
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
      
    static void *
    threadF1(void *data){
        void **arg = (void **)data;
        auto monitorObject = (RootMonitor<LocalDir> *)arg[0];
        auto gridView_p = (GridView<LocalDir> *)arg[1];
        GtkWidget *child = gridView_p->child();
        char *path = g_strdup(gridView_p->path());
        char *sum = NULL;
        char *sumMnt = NULL;
        char *sumEfs = NULL;
        auto bookmarks_p = (Bookmarks *) bookmarksObject;
        auto bookmarksFile = bookmarks_p->getBookmarksFilename();

        TRACE("***threadF1 for gridview_p %p \n", gridView_p);
        auto efsFile = EfsResponse<Type>::efsKeyFile();
        auto mntFile = g_strdup("/proc/mounts");

loop:
        // gridView may change.
        if (!arg[2] || !Child::validGridView(gridView_p)) {
          TRACE("*** RootMonitor, child %p --> %p\n",child,gridView_p->child());
          TRACE("*** RootMonitor, gridView has changed from %p\n", gridView_p);
done:
          TRACE("*** root monitor thread now has exited.\n");
          g_free(path);
          g_free(sum);
          g_free(sumMnt);
          g_free(sumEfs);
          g_free(bookmarksFile);
          g_free(efsFile);
          g_free(mntFile);
          g_free(data);
          return NULL;
        }
       
        // get initial md5sum
        g_free(sum);
        g_free(sumMnt);
        g_free(sumEfs);
        sum = Basic::md5sum(bookmarksFile);
        sumMnt = Basic::md5sum(mntFile);
        sumEfs = Basic::md5sum(efsFile);

        if (!sum || !sumMnt || !sumEfs) {
          ERROR_("Error:: Exiting threadF1(%p) on md5sum error (sum)\n", gridView_p);
          goto done;
        }
        TRACE("***RootMonitor::threadF1(): initial sum=%s \n", sum);
        TRACE("***RootMonitor::threadF1(): initial sumMnt=%s \n", sumMnt);

    
        while (arg[2]){
          usleep(250000);
          if (!arg[1])continue;
          auto test1 = checkSumMnt(bookmarksFile, &sum);
          auto test2 = checkSumMnt("/proc/mounts", &sumMnt);
          if (!test1 && !test2) continue;   

          // This is sent to main context to avoid race with gridView invalidation.
          TRACE("***checksum changed. context function.with %p, %p\n",monitorObject, gridView_p);
          TRACE("***checksum changed. context function.with %p, %p\n",arg[0], arg[1]);
          Basic::context_function(reload_f, (void *)child);
          break; // We break because we need to update checksums.
        }

        sleep(1);
        TRACE("*** valid gridView (%p) = %d\n",
            gridView_p,Child::validGridView(gridView_p)); 
        goto loop;
    }
     
    static void *reload_f(void *data){
        auto child = GTK_WIDGET(data);
        Workdir<Type>::setWorkdir("Bookmarks", child);
        return NULL;
    }


  };

}
#endif
