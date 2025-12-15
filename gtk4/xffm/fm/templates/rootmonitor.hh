#ifndef ROOTMONITOR_HH
#define ROOTMONITOR_HH
namespace xf {

  template <class Type> class RootMonitor {

    void **args = NULL;
    char *reloadPath_ = NULL;
    GtkWidget *child_ = NULL;
    GridView<Type> *gridView_ = NULL;
    public:
      RootMonitor(GridView<Type> *gridview, const char *reloadPath, int size){
        TRACE("*** RootMonitor(GridView<Type>) constructor %p(%p)\n",
            gridview->child(), gridview);
        reloadPath_ = g_strdup(reloadPath);
        child_ = gridview->child();
        gridView_ = gridview;
        TRACE("*** RootMonitor(GridView<Type>) constructor, child=%p\n",child_);

        args = (void **)calloc(3, sizeof(void *));
        args[0] = (void *) this;
        args[1] = GINT_TO_POINTER(size);
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
      files[0] = g_strdup("/proc/mounts");
      files[1] = g_strdup("/proc/partitions");
      files[2] = bookmarks_p->getBookmarksFilename();
      files[3] = EfsResponse<Type>::efsKeyFile(); // static function



        void **arg = (void **)data;
        auto monitorObject = (RootMonitor<Type> *)arg[0];
        auto size = GPOINTER_TO_INT(arg[1]);
        auto gridView_p = monitorObject->gridView();
        GtkWidget *child_p = monitorObject->child();
        char *path = g_strdup(gridView_p->path());

        DBG("\n***Root Monitor %p(%p) started '%s'\n", child_p, gridView_p, path);

loop:
        // gridView may change.
        Child::lockGridView("Root Monitor");
        auto valid = Child::validGridView(gridView_p);
        Child::unlockGridView("Root Monitor");
        DBG("\n*** valid gridView %p(%p) %s = %d\n",
            child_p, gridView_p, path, valid); 
        if (!arg[2] || !valid) {
          DBG("\n***Root Monitor cleanup %p(%p) continue=%d, valid=%d %s\n", 
              child_p, gridView_p, arg[2], valid, path);
done:
          g_free(path);
          g_strfreev(sum);
          g_strfreev(files);
          g_free(data);
          return NULL;
        }
       
        // get initial md5sum
        for (int i=0; i<size; i++){
          g_free(sum[i]);
          sum[i] = Basic::md5sum(files[i]);
          if (sum[i] == NULL) {
            ERROR_("Error:: Exiting threadF1 %p(%p) on md5sum error (sum)\n", 
                child_p, gridView_p);
            goto done;
          }
        }
    
        while (arg[2]){
          usleep(250000);
          if (!arg[1])continue;

          bool test[5];
          for (int i=0; i<size; i++){
            test[i] = Basic::checkSumFile(files[i], &(sum[i]));
          }
           
          TRACE("tests: %d %d %d %d\n", test[0],test[1],test[2],test[3]);
          if (!test[0] && !test[1] && !test[2] && !test[3]) continue;
          Child::removeGridView(gridView_p); // gridView_p will no longer be valid.
          // This is sent to main context to avoid race with gridView invalidation.
          DBG("\n***RootMonitor checksum changed %p(%p) '%s'\n",
              child_p, gridView_p, path);
          void *v[] = {(void *)path, (void *)child_p, (void *)gridView_p, NULL};
          Basic::context_function(reload_f, v);
          break; // We break because we need to update checksums.
        }

        sleep(1);
        

        goto loop;
    }
     
    static void *reload_f(void *data){
      auto v = (void **)data;
      auto path = (const char *)v[0];
      auto child = GTK_WIDGET(v[1]);
      auto gridview = (GridView<Type> *)v[2];

      auto monitorObject = (RootMonitor<Type> *) data;
      DBG("*** RootMonitor reload(%s) with %p(%p)\n", 
            path, child, gridview);
      g_object_set_data(G_OBJECT(child), "selection", NULL);
      
      Workdir<Type>::setWorkdir(path, child);
      return NULL;
    }
  };

}
#endif
