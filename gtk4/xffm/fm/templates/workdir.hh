#ifndef WORKDIR_HH
#define WORKDIR_HH
namespace xf {
  template <class Type>
  class Workdir  {
    private:
      static void  updateGridView(const char *path){
        TRACE("updateGridView(): Serial=%d->%d\n", Child::getSerial(), Child::getSerial()+1);
        // On creating a new GtkGridView, we send pointer to function to process directory change (gridViewClick).
       
       // cancel monitor if any 
        auto oldObject = (GridView<Type> *)Child::getGridviewObject();
        if (oldObject){
          auto fstabMonitor = oldObject->fstabMonitor();
          if (fstabMonitor){
            fstabMonitor->stopMonitor();
          } 
        }  
        Child::incrementSerial();

        auto child = Child::getChild();
        int iconsize = Settings::getInteger("xfterm", "iconsize", 24);
        g_object_set_data(G_OBJECT(child), "iconsize", GINT_TO_POINTER(iconsize));
        auto m = g_object_get_data(G_OBJECT(child), "monitor");
        GFileMonitor *monitor = NULL;
        if (Child::validMonitor(m)) {
          Child::removeMonitor(m);
          monitor = G_FILE_MONITOR(m);
          g_object_set_data(G_OBJECT(child), "monitor", NULL);
          pthread_mutex_lock(&monitorMutex);   
          g_object_set_data(G_OBJECT(monitor), "inactive", GINT_TO_POINTER(1));
          pthread_mutex_unlock(&monitorMutex);   
          auto dirFile = G_FILE(g_object_get_data(G_OBJECT(monitor), "file"));

          g_file_monitor_cancel(monitor);
          g_object_unref(monitor);
          //*
          auto path = g_file_get_path(dirFile);
          TRACE("local monitor for %s cancelled.\n", path);
          g_free(path);

          g_object_unref(dirFile);
        }
       
        // cancel threadpool for previews, if any. Wait on condition
        if (strcmp(path, "Disk Mounter") == 0) {
          auto viewObject = new GridView<FstabDir>(path, (void *)gridViewClick);
          viewObject->child(child);
          auto store = viewObject->listStore();
            
          monitor = G_FILE_MONITOR(g_object_get_data(G_OBJECT(store), "monitor"));
          if (monitor){
            TRACE("local monitor started for %s\n", path);
          } else {
            TRACE("no local monitor for %s\n", path);
          }

          //viewObject->monitor(monitor);
          //g_object_set_data(G_OBJECT(child), "monitor", monitor);

          auto view = viewObject->view();
          Child::setGridview(view); // This is the GtkGridView.
          TRACE("oldObject: %p\n", oldObject);
          if (oldObject) delete oldObject;
          Child::setGridviewObject(viewObject);  // This is the object from GridView template.   
        } else {
          auto viewObject = new GridView<LocalDir >(path, (void *)gridViewClick);
          TRACE("new object: %p\n", viewObject);
          viewObject->child(child);
          auto store = viewObject->listStore();
            
          monitor = G_FILE_MONITOR(g_object_get_data(G_OBJECT(store), "monitor"));
          if (monitor){
            TRACE("local monitor started on %p\n", monitor);
          }

          viewObject->monitor(monitor);
          //if (monitor) TRACE("*** monitor = %p child=%p\n", monitor);
          g_object_set_data(G_OBJECT(child), "monitor", monitor);

          auto view = viewObject->view();
          Child::setGridview(view); // This is the GtkGridView.
          auto oldObject = Child::getGridviewObject();
          TRACE("oldObject: %p\n", oldObject);
          if (oldObject) {
            auto object = (GridView<Type> *) oldObject;
            delete object;
          }
          Child::setGridviewObject(viewObject);  // This is the object from GridView template.   
        }



        TRACE("updateGridView: ok\n");
      }

    public:
    static const gchar *getWorkdir(GtkWidget *child){
      return Child::getWorkdir(child);
    }
    static const gchar *getWorkdir(void){
      return Child::getWorkdir();
    }

    static bool pleaseWait(void){
      auto size = THREADPOOL->size();
      if (size > 0) {
        THREADPOOL->clear();
        /*
char buffer[4096];

        snprintf(buffer, 4096, " %s\n%s: %d->%d (%s).\n",
              _("There are unfinished jobs: please wait until they are finished."),
              _("Parallel threads:"), size, Thread::threadPoolSize(), 
              _("Unfinished Jobs in Queue") );
        TRACE("%s", buffer);
        Print::printInfo(Child::getCurrentOutput(), "emblem-important", g_strdup(buffer));
        Thread::clearThreadPool();
        //return true;
        */
      }
      return false;
    }

    static bool setWorkdir(const gchar *path, GtkWidget *child){
      TRACE("setWorkdir...A\n");
      if (pleaseWait()) return false;
      auto wd = (char *)getWorkdir(child);
      if (strcmp(wd, path)){
        g_free(wd);
        g_object_set_data(G_OBJECT(child), "path", g_strdup(path));
      }
      Child::setWindowTitle(child);
      UtilPathbar<Type>::updatePathbar(false, (void *)pathbar_go);
      updateGridView(path);
      return true;
    }
    static bool setWorkdir(const gchar *path){
      TRACE("setWorkdir...A\n");
      if (pleaseWait()) return false;
      auto child = Child::getChild();
      auto wd = (gchar *)g_object_get_data(G_OBJECT(child), "path");
      g_free(wd);
        // FIXME leak: path      
      g_object_set_data(G_OBJECT(child), "path", g_strdup(path));
      Child::setWindowTitle(child);
      UtilPathbar<Type>::updatePathbar(true, (void *)pathbar_go);
      updateGridView(path);
      return true;
    }
    static bool setWorkdir(const gchar *path, bool updateHistory){
      TRACE("setWorkdir B...path=%s\n",path);
      if (pleaseWait()) return false;
      auto child = Child::getChild();
      auto wd = (gchar *)g_object_get_data(G_OBJECT(child), "path");
      g_free(wd);
      g_object_set_data(G_OBJECT(child), "path", g_strdup(path));
      Child::setWindowTitle(child);
      UtilPathbar<Type>::updatePathbar(updateHistory, (void *)pathbar_go);
      updateGridView(path);
      return true;
    }
    static bool setWorkdir(const gchar *path, GtkBox *pathbar, bool updateHistory){
      TRACE("setWorkdir...C\n");
      if (pleaseWait()) return false;
      if (!Child::mainWidget()) return false;
      auto child = GTK_WIDGET(g_object_get_data(G_OBJECT(pathbar), "child"));
      auto wd = (gchar *)g_object_get_data(G_OBJECT(child), "path");
      TRACE("setWorkdir: path=%s, wd path=%s\n", path, wd);
      g_free(wd);
      g_object_set_data(G_OBJECT(child), "path", g_strdup(path));
      Child::setWindowTitle(child);
      UtilPathbar<Type>::updatePathbar(path, pathbar, updateHistory, (void *)pathbar_go);
      updateGridView(path);
      return true;
    }
    
    static gboolean // on release... Coordinates are in icon's frame of reference.
    gridViewClick(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer object){
        
      auto d = (Dnd<Type> *)g_object_get_data(G_OBJECT(Child::mainWidget()), "Dnd");
      d->dropDone(false);
      d->dragOn(true);

      auto eventController = GTK_EVENT_CONTROLLER(self);
      auto event = gtk_event_controller_get_current_event(eventController);
      auto box = gtk_event_controller_get_widget(eventController);
      
      auto gridView_p = (GridView<Type> *)g_object_get_data(G_OBJECT(box), "gridView_p");

      //gridView_p->x,y are in gridview's frame of reference.
      double distance = sqrt(pow(gridView_p->x() - x,2) + pow(gridView_p->y() - y,2));

      //auto size = Settings::getInteger("xfterm", "iconsize",24);
      //For single click action, must not be released further
      //than size, starting at GridView::down_f position
      int size = 5;
      TRACE("down at %lf,%lf, up at %lf,%lf. Distance=%lf, size=%d\n", 
          gridView_p->x(), gridView_p->y(), x, y, distance, size);
      if (distance > size) {
        return true;
      }

      // proceed with double click action...
      /*
      auto imageBox = gtk_event_controller_get_widget(eventController);
      auto gridView_p = (GridView<TypeType> *)g_object_get_data(G_OBJECT(imageBox), "gridView_p");
      auto store = gridView_p->store();
      guint positionF;
      auto item = gtk_list_item_get_item(GTK_LIST_ITEM(object));
      auto found = g_list_store_find_with_equal_func(store, item, equalItem, &positionF);
      if (!found){
        TRACE("gridViewClick(): this should not happen.\n");
        exit(1);
      } 
      TRACE("Found at %d\n", positionF);
      auto selectionModel = gridView_p->selectionModel();
      gtk_selection_model_select_item(selectionModel, positionF, false);
*/                 
      auto modType = gdk_event_get_modifier_state(event);

      TRACE("modType = 0x%x\n", modType);
      if (modType & GDK_CONTROL_MASK) return FALSE;
      if (modType & GDK_SHIFT_MASK) return FALSE;
      
      TRACE("gestureClick; object=%p button=%d\n", object,
          gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(self)));

      auto info = G_FILE_INFO(gtk_list_item_get_item(GTK_LIST_ITEM(object)));
      auto file = G_FILE(g_file_info_get_attribute_object (info, "standard::file"));

      auto root = g_file_info_get_attribute_object (info, "xffm::root");
      if (root){
        setWorkdir("Bookmarks");
        return TRUE;
      }
      auto fstab = g_file_info_get_attribute_object (info, "xffm::fstab");
      if (fstab){
        setWorkdir("Disk Mounter");
        return TRUE;
      }
      auto ecryptfs = g_file_info_get_attribute_object (info, "xffm::ecryptfs");
      if (ecryptfs){
        auto efsmount = g_find_program_in_path("mount.ecryptfs");
        if (!efsmount){
          auto message = g_strdup_printf(_("The %s utility is not installed."), "ecryptfs");
          Print::printWarning(Child::getOutput(), g_strconcat(message, "(AUR: ecryptfs-utils)\n", NULL));
          g_free(message);
          return TRUE;
        }
        
        TRACE("Open new ecryptfs dialog.\n");
        auto mountDir = g_strconcat(g_get_home_dir(), G_DIR_SEPARATOR_S, "mnt", NULL);
        if (mkdir(mountDir, 0750) < 0){
          TRACE("mkdir %s: %s\n", mountDir, strerror(errno));
        }
        auto parent = GTK_WINDOW(Child::mainWidget());
        new EFS<Type>(parent, mountDir);
        g_free(mountDir);

        return TRUE;
      }
      auto trash = g_file_info_get_attribute_object (info, "xffm::trash");
      if (trash){
        auto trashDir = g_strdup_printf("%s/.local/share/Trash/files", g_get_home_dir());
        if (!g_file_test(trashDir, G_FILE_TEST_EXISTS)){
          auto message = g_strdup_printf(" %s (%s)\n", _("Trash is empty"), trashDir); 
          g_free(trashDir);
          Print::printWarning(Child::getOutput(), message);
          return TRUE;
        }
        setWorkdir(trashDir);
        g_free(trashDir);
        return TRUE;
      }


      TRACE("gestureClick; file=%p\n", file);
      auto path = g_file_get_path(file);
      TRACE("gestureClick; path=%p\n", path);
      TRACE("click on %s\n", path);
      // Partition?
      TRACE("workdir.hh:: isInPartitions(%s) = %d\n", path, FstabUtil::isInPartitions(path));
      TRACE("workdir.hh:: isMounted(%s) = %d\n", path, FstabUtil::isMounted(path));

      if (FstabUtil::isInPartitions(path)){
        if (!FstabUtil::isMounted(path)){
          auto message = g_strdup_printf(_("The volume '%s' is not mounted."), path);
          Print::printWarning(Child::getOutput(), g_strdup_printf(" %s\n", message));
          g_free(message);
          return TRUE;
        }
        auto mountPoint = FstabUtil::tabMountPoint(path);
        if (mountPoint){
          setWorkdir(mountPoint);
          g_free(mountPoint);
        }
        return TRUE;
      }
      auto type = g_file_info_get_file_type(info);
      if ((type == G_FILE_TYPE_DIRECTORY )||(LocalDir::symlinkToDir(info, type))) {
        TRACE("Go to action...\n");
        auto child = Child::getChild();
        setWorkdir(path);
      } else {
        TRACE("mimetype action...\n");
        new OpenWith<bool>(GTK_WINDOW(Child::mainWidget()), path, NULL);
      }
      g_free(path);
      return TRUE;
    }

    static gboolean
    pathbar_go (
              GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer data ) 
    {
        auto pathbar = GTK_BOX(data);
        auto eventBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
        auto name = (char *) g_object_get_data(G_OBJECT(eventBox), "name");
        auto path = (char *) g_object_get_data(G_OBJECT(eventBox), "path");
        auto button = gtk_gesture_single_get_button(GTK_GESTURE_SINGLE(self));
          TRACE("***pathbar goto... name=%s, path=%s\n", name, path);
        if (button == 1){
          //if (strcmp(path, "Bookmarks")==0) setWorkdir(g_get_home_dir(), pathbar, true);
          //else setWorkdir(path, pathbar, true);
          setWorkdir(path, pathbar, true);
          return TRUE;
        }
        //TRACE("pathbar_go...name=%s, path=%s button=%d\n", name, path, button);
        return FALSE;

    }
   

  };
}
#endif
