#ifndef WORKDIR_HH
#define WORKDIR_HH
namespace xf {
  template <class Type>
  class Workdir  {
    private:
      static void  updateGridView(const char *path){
#ifdef ENABLE_GRIDVIEW
        TRACE("updateGridView(): Serial=%d->%d\n", Child::getSerial(), Child::getSerial()+1);
        // On creating a new GtkGridView, we send pointer to function to process directory change (gridViewClick).
        Child::incrementSerial();
        auto viewObject = new GridView<LocalDir>(path, (void *)gridViewClick);
        DBG("new object: %p\n", viewObject);
        auto view = viewObject->view();
        //auto view = GridView<LocalDir>::getGridView(path, (void *)gridViewClick);
        Child::setGridview(view);
        auto oldObject = Child::getGridviewObject();
        DBG("oldObject: %p\n", oldObject);
        if (oldObject) {
          auto object = (GridView<LocalDir> *) oldObject;
          delete object;
        }
        Child::setGridviewObject(viewObject);       
#endif
      }

      static void  updatePathbar(bool addHistory, void *pathbar_go){
        UtilPathbar<Type>::updatePathbar(addHistory, pathbar_go);
      }
      static void  updatePathbar(const gchar *path, GtkBox *pathbar, 
          bool addHistory, void *pathbar_go){
        UtilPathbar<Type>::updatePathbar(path, pathbar, addHistory, pathbar_go);
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
      updatePathbar(false, (void *)pathbar_go);
      updateGridView(path);
      return true;
    }
    static bool setWorkdir(const gchar *path){
      TRACE("setWorkdir...A\n");
      if (pleaseWait()) return false;
      auto child = Child::getChild();
      auto wd = (gchar *)g_object_get_data(G_OBJECT(child), "path");
      g_free(wd);
      g_object_set_data(G_OBJECT(child), "path", g_strdup(path));
      updatePathbar(true, (void *)pathbar_go);
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
      updatePathbar(updateHistory, (void *)pathbar_go);
      updateGridView(path);
      return true;
    }
    static bool setWorkdir(const gchar *path, GtkBox *pathbar, bool updateHistory){
      TRACE("setWorkdir...C\n");
      if (pleaseWait()) return false;
      if (!MainWidget) return false;
      auto child = GTK_WIDGET(g_object_get_data(G_OBJECT(pathbar), "child"));
      auto wd = (gchar *)g_object_get_data(G_OBJECT(child), "path");
      TRACE("setWorkdir: path=%s, wd path=%s\n", path, wd);
      g_free(wd);
      g_object_set_data(G_OBJECT(child), "path", g_strdup(path));
      Child::setWindowTitle(child);
      updatePathbar(path, pathbar, updateHistory, (void *)pathbar_go);
      updateGridView(path);
      return true;
    }

    static gboolean
    gridViewClick(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer object){

      auto eventController = GTK_EVENT_CONTROLLER(self);
      auto event = gtk_event_controller_get_current_event(eventController);
      
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
        setWorkdir("xffm:root");
        return TRUE;
      }
      TRACE("gestureClick; file=%p\n", file);
      auto path = g_file_get_path(file);
      TRACE("gestureClick; path=%p\n", path);
      TRACE("click on %s\n", path);
      auto type = g_file_info_get_file_type(info);
      if ((type == G_FILE_TYPE_DIRECTORY )||(LocalDir::symlinkToDir(info, type))) {
        TRACE("Go to action...\n");
        auto child = Child::getChild();
        setWorkdir(path);
      } else {
        TRACE("mimetype action...\n");
        new OpenWith<bool>(GTK_WINDOW(MainWidget), path);
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
          TRACE("pathbar goto... name=%s, path=%s\n", name, path);
        if (button == 1){
          TRACE("pathbar goto...\n");
          //if (strcmp(path, "xffm:root")==0) setWorkdir(g_get_home_dir(), pathbar, true);
          //else setWorkdir(path, pathbar, true);
          setWorkdir(path, pathbar, true);
          return TRUE;
        }
        //TRACE("pathbar_go...name=%s, path=%s button=%d\n", name, path, button);
        return FALSE;
        /*
        
        if (event->button == 1) {
            pathbar_p->pathbar_ok(eventBox);
        }

        if (event->button == 3) {
            auto view = pathbar_p->pathbarView();
            const gchar *path = pathbar_p->getClickPath(eventBox);
            TRACE("***clickpath=%s\n", path);
            GtkMenu *menu = NULL;
            if (g_file_test(path, G_FILE_TEST_IS_DIR)){ 
                menu = LocalPopUp<Type>::popUp();
                Popup<Type>::setWidgetData(menu, "path", path);
                g_object_set_data(G_OBJECT(menu),"view", NULL);
                BaseSignals<Type>::configureViewMenu(LOCALVIEW_TYPE);
            } else {
                // do xffm:root menu
                RootPopUp<Type>::resetPopup();
                menu = RootPopUp<Type>::popUp();
            }
            if (menu) {
                gtk_menu_popup_at_pointer (menu, (const GdkEvent *)event);
            }          
        }

        return FALSE;*/
    }
   

  };
}
#endif
