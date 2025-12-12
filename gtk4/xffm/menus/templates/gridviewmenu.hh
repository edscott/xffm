#ifndef GRIDVIEWMENU_HH
#define GRIDVIEWMENU_HH
namespace xf {
  template <class Type> class MainMenu;
  template <class Type> class Prompt;
  template <class Type> class MenuCallbacks;
  template <class Type> class Workdir;
  class LocalDir;
  template <class Type>
  class GridviewMenu {
    using clipboard_t = ClipBoard<LocalDir>;
    public:
    GridviewMenu(void){}
    ~GridviewMenu(void){}

    static const char **keys(void){
      static const char *keys_[] = { // Order is important.
        _("auto"), //
        //_("Toggle Text Mode"),
        _("Open in new tab"), //
        _("Open with"), //
        _("Create a compressed archive with the selected objects"),
        _("Mount Volume"),
        _("Unmount Volume"),
        _("Add bookmark"),
        _("Remove bookmark"),
        _("Copy"),
        _("Cut"),
        _("Paste"), // 
        _("Clipboard is empty."), // 
        _("Rename"),
        _("Duplicate"),
        _("Link"),
        _("Properties"),
        _("Delete"),
        _("Encrypt File..."),
        _("Decrypt File..."),
#ifdef WSL
        _("Close"), 
#endif
        NULL
      };
      return keys_;
    }

    MenuInfo_t *iconNames(void){
      static MenuInfo_t menuIconNames_[] = { // Need not be complete with regards to keys_.
        {_("Mount Volume"),(void *) EMBLEM_GREEN_BALL}, 
        {_("Unmount Volume"),(void *) EMBLEM_RED_BALL}, 
        {_("Open in new tab"),(void *) EMBLEM_NEW_TAB}, 
        {_("Open with"),(void *) EMBLEM_RUN}, 
        {_("auto"),(void *) EMBLEM_RUN}, 
        {_("Rename"),(void *) EMBLEM_SAVE_AS}, 
        {_("Duplicate"),(void *) EMBLEM_SAVE}, 
        {_("Link"),(void *) EMBLEM_SYMLINK}, 
        {_("Delete"),(void *) EMBLEM_DELETE}, 
        {_("Properties"),(void *) EMBLEM_PROPERTIES}, 
        {_("Copy"),(void *) EMBLEM_COPY}, 
        {_("Cut"),(void *) EMBLEM_CUT}, 
        {_("Paste"),(void *) EMBLEM_PASTE}, 
        {_("Add bookmark"),(void *) EMBLEM_FAVOURITE}, 
        {_("Remove bookmark"),(void *) EMBLEM_RED_BALL}, 
        {_("Encrypt File..."),(void *) EMBLEM_BLOWFISH}, 
        {_("Decrypt File..."),(void *) EMBLEM_BLOWFISH}, 
        {_("Create a compressed archive with the selected objects"),(void *) EMBLEM_PACKAGE},
        {_("Close"),(void *) WINDOW_CLOSE},
       {NULL, NULL}
      }; 
      return menuIconNames_;
    }
    MenuInfo_t *callbacks(void){
      static MenuInfo_t menuCallbacks_[] = { // Need not be complete with regards to keys_.
        {_("Mount Volume"),(void *) mount}, 
        {_("Unmount Volume"),(void *) unmount}, 
        {_("Toggle Text Mode"),(void *) MenuCallbacks<Type>::popCall}, 
        {_("Open in new tab"),(void *) MenuCallbacks<Type>::openNewTab}, 
        {_("Open with"),(void *) openWith}, 
        {_("auto"),(void *) run}, 
        {_("Duplicate"),(void *) duplicate}, 
        {_("Link"),(void *) link}, 
        {_("Rename"),(void *) move}, 
        {_("Copy"),(void *) copy}, 
        {_("Cut"),(void *) cut}, 
        {_("Paste"),(void *) MenuCallbacks<Type>::paste}, 
        {_("Delete"),(void *) remove}, 
        {_("Add bookmark"),(void *) addB}, 
        {_("Remove bookmark"),(void *) removeB}, 
      //  {_("Select All"),(void *) selectAll}, 
        {_("Properties"),(void *) properties}, 
        {_("Encrypt File..."),(void *) bcrypt}, 
        {_("Decrypt File..."),(void *) bcrypt}, 
        {_("Create a compressed archive with the selected objects"),(void *) tar},
        {_("Close"),(void *) MainMenu<Type>::closeMenu},
       
        {NULL, NULL}
      };
      return menuCallbacks_;
    }
    MenuInfo_t *data(void){
      static MenuInfo_t menuData_[] = { // Need not be complete with regards to keys_ nor menuCallbacks_.
       {NULL, NULL}
      };
      return menuData_;      
    }
    const char **checkboxes(void){
      static const char *boxes_[] = { NULL};
      return boxes_;      
    }
const char **radioboxes(void){
      static const char *boxes_[] = { NULL};
      return boxes_;      
    }

     static const char *getDefaultApp(const char *path){       
      const char *extension = NULL;
      if (strchr(path, '.')) extension = strrchr(path, '.') + 1;
      const char *defaultApp = NULL;
      if (extension){
        defaultApp = Settings::getString("MimeTypeApplications", extension);
      }
        TRACE("defaultApp = %s\n", defaultApp);
      
      if (!defaultApp){
        auto mimetype = MimeMagic::mimeMagic(path); 
        TRACE("%s: mimetype= %s\n", path, mimetype);
        auto apps = MimeApplication::locate_apps(mimetype);
        if (apps) defaultApp = apps[0]; // first item
      }
      return defaultApp;
   }

    private:
    
    static char *getPath(GtkPopover *menu){
      auto data =   g_object_get_data(G_OBJECT(menu), "info");
      if (!data) return NULL;
      auto info = G_FILE_INFO(data);
      return Basic::getPath(info);
    }
    
    static bool isEFSinfo(GtkPopover *menu){
      auto data =   g_object_get_data(G_OBJECT(menu), "info");
      if (!data) return false;
      auto info = G_FILE_INFO(data);
      auto x = g_file_info_get_attribute_object(info, "xffm:efsInfo");
      if (x) return true;
      return false;
    }
    
    static bool isEcryptfs(GtkPopover *menu){
      auto data =   g_object_get_data(G_OBJECT(menu), "info");
      if (!data) return false;
      auto info = G_FILE_INFO(data);
      auto x = g_file_info_get_attribute_object(info, "xffm:ecryptfs");
      if (x) return true;
      return false;
    }

    static void 
    bcrypt(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      MainMenu<Type>::closePopover(menu);
      auto info = G_FILE_INFO(g_object_get_data(G_OBJECT(menu), "info"));
      auto path = Basic::getPath(info);
      if (strrchr(path, '.') && strcmp(strrchr(path, '.'), ".gpg") == 0){
        // decrypt
        TRACE("Decrypt file \"%s\"\n", path);
      } else {
        TRACE("Encrypt file \"%s\"\n", path);
      }
      TRACE("open bcrypt dialog...\n");
      auto parent = GTK_WINDOW(Child::mainWidget());
      try {
        new Bfish<Type>(parent, path);
      } catch(int errorCode) {
        ERROR_("Catch errorCode %d at gridviewmenu.hh::bcrypt()\n", errorCode);
      }
      g_free(path);
    }

    static void
    tar(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      MainMenu<Type>::closePopover(menu);
      auto info = G_FILE_INFO(g_object_get_data(G_OBJECT(menu), "info"));
      auto path = Basic::getPath(info);
      auto parent = GTK_WINDOW(Child::mainWidget());
      try {
        new Tar<Type>(parent, path);
      } catch(int errorCode) {
        ERROR_("Catch errorCode %d at gridviewmenu.hh::bcrypt()\n", errorCode);
      }
      g_free(path);

      return;
    }

  public:
    static void 
    gestureProperties(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer data){
      
      auto eventController = GTK_EVENT_CONTROLLER(self);
      auto widget = gtk_event_controller_get_widget(eventController);
      properties(widget, NULL);
    }

    static void 
    gestureEFSinfo(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer data){
      
      auto eventController = GTK_EVENT_CONTROLLER(self);
      auto widget = gtk_event_controller_get_widget(eventController);
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(widget), "menu")); 
      auto info = G_FILE_INFO(g_object_get_data(G_OBJECT(menu), "info"));
      auto path = Basic::getPath(info);
      auto parent = GTK_WINDOW(Child::mainWidget());
      new EFS<Type>(parent, path, true);
    }

    static void 
    gestureEcryptfs(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer data){
      
      auto mountDir = g_strconcat(g_get_home_dir(), G_DIR_SEPARATOR_S, "mnt", NULL);
      if (mkdir(mountDir, 0750) < 0){
        TRACE("mkdir %s: %s\n", mountDir, strerror(errno));
      }
      auto parent = GTK_WINDOW(Child::mainWidget());
      new EFS<Type>(parent, mountDir);
      g_free(mountDir);
    }

    static void 
    gestureFstab(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer data){
      
      auto eventController = GTK_EVENT_CONTROLLER(self);
      auto widget = gtk_event_controller_get_widget(eventController);
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(widget), "menu")); 
      auto info = G_FILE_INFO(g_object_get_data(G_OBJECT(menu), "info"));
      Workdir<Type>::setWorkdir("Disk Mounter", true);
    }

    static void
    showItem(GtkPopover *menu, const char *id){
      auto w = GTK_WIDGET(g_object_get_data(G_OBJECT(menu), id));
      if (w) gtk_widget_set_visible(w, true);
    }

    static void
    hideAll(GtkPopover *menu){
      for (auto p=keys(); p && *p; p++){
        auto w = GTK_WIDGET(g_object_get_data(G_OBJECT(menu), *p));
        gtk_widget_set_visible(w, false);
      } 
    }

    static void
    showAll(GtkPopover *menu){
      for (auto p=keys(); p && *p; p++){
        auto w = GTK_WIDGET(g_object_get_data(G_OBJECT(menu), *p));
        gtk_widget_set_visible(w, true);
      } 
    }

  private: 
    
    static void 
    properties(GtkWidget *widget, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(widget), "menu")); 
      TRACE("properties: widget is %p\n", widget);
      if (!menu){
        TRACE("menu is null\n");
        return;
      }
      if (!GTK_IS_POPOVER(menu)){
        TRACE("%p is not popover\n", menu);
        return;
      }
      MainMenu<Type>::closePopover(menu);
      auto info = G_FILE_INFO(g_object_get_data(G_OBJECT(menu), "info"));
      //TRACE("path= %s, info=%p\n", path, info);
      new Properties<bool>(info);
      //g_free(path);
    }

    static void addB(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      MainMenu<Type>::closePopover(menu);
      gchar *path = getPath(menu);
      
      if (path) {
        TRACE("path is %s\n", path);
        auto bookmarks_p = (Bookmarks *) bookmarksObject;
        bookmarks_p->addBookmark(path);
        g_free(path);
        auto gridView_p = (GridView<Type> *)g_object_get_data(G_OBJECT(menu), "gridView_p");
        const char *p = gridView_p->path();
        Workdir<Type>::setWorkdir(p);
        
      }
    }
    static void removeB(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      MainMenu<Type>::closePopover(menu);
      gchar *path = getPath(menu);
      
      if (path) {
        auto bookmarks_p = (Bookmarks *) bookmarksObject;
        bookmarks_p->removeBookmark(path);
        g_free(path);
        auto gridView_p = (GridView<Type> *)g_object_get_data(G_OBJECT(menu), "gridView_p");
        const char *p = gridView_p->path();
        TRACE("*** removeB set workdir to %s\n", p);
        Workdir<Type>::setWorkdir(p);
      }
    }

    static void 
    openWith(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      MainMenu<Type>::closePopover(menu);
      auto path = getPath(menu);

      if (!path) {
        auto gridView_p = (GridView<Type> *)g_object_get_data(G_OBJECT(menu), "gridView_p");
        auto selectionList = gridView_p->getSelectionList();
        if (selectionList) {
          TRACE("selectionList = %p\n", selectionList);
         // auto list = getSelectionList();
         // new OpenWith<bool>(GTK_WINDOW(Child::mainWidget()), path);
          new OpenWith<bool>(GTK_WINDOW(Child::mainWidget()), NULL, selectionList);
        }  
      } else {
        new OpenWith<bool>(GTK_WINDOW(Child::mainWidget()), path, NULL);
      }
      g_free(path);
    }
     
    static void mount(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      MainMenu<Type>::closePopover(menu);
      auto path = getPath(menu);
      if (!path) return;
      else {TRACE("mount item path is %s\n", path);}
      auto info = G_FILE_INFO(g_object_get_data(G_OBJECT(menu), "info"));
      if (GridView<Type>::isEfsInfo((void *)info)){
        auto parent = GTK_WINDOW(Child::mainWidget());
        new EFS<Type>(parent, path, true);
        g_free(path);
        return;
      }
      auto folder = g_strconcat(g_get_home_dir(), G_DIR_SEPARATOR_S, "mnt", NULL);
      auto parent = GTK_WINDOW(Child::mainWidget());
      new Mount<Type>(parent, folder, path);
      g_free(folder);
      g_free(path);
    }
     
    static void *umountThread(void *data){
      char *path = (char *)data;
      char *arg[]={(char *)"sudo", (char *)"-A", (char *)"umount", path, NULL};
      auto output = Child::getOutput();
      Run<bool>::thread_run(output, (const char **)arg, true);
      return NULL;
    }

    static void *umountThreadMaster(void *data){
      pthread_t thread;
      pthread_create(&thread, NULL, umountThread, (void *)data);
      Thread::threadCount(true,  &thread, "umountThreadMaster");
      void *retval;
      pthread_join(thread, &retval);
      Thread::threadCount(false,  &thread, "umountThreadMaster");
      return NULL;
    }

    static void unmount(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      MainMenu<Type>::closePopover(menu);
      auto path = getPath(menu);
      if (!path) return;
      else {TRACE("unmount item %s\n", path);}
      pthread_t thread;
      Thread::threadCount(true,  &thread, "unmount");
      pthread_create(&thread, NULL, umountThreadMaster, (void *)path);
      pthread_detach(thread);
      Thread::threadCount(false,  &thread, "umountThreadMaster");
    }
   
    static void duplicate(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      MainMenu<Type>::closePopover(menu);
      auto path = getPath(menu);
      if (!path) return;
      else {TRACE("path is %s\n", path);}
      dialogPath<cpResponse>::action(path);
      g_free(path);
    }

    static void move(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      MainMenu<Type>::closePopover(menu);
      auto path = getPath(menu);
      if (!path) return;
      else {TRACE("path is %s\n", path);}
      dialogPath<mvResponse>::action(path);
      g_free(path);
    }

    static void link(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      MainMenu<Type>::closePopover(menu);
      auto path = getPath(menu);
      if (!path) return;
      else {TRACE("path is %s\n", path);}
      dialogPath<lnResponse>::action(path);
      g_free(path);
    }

    static void remove(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      MainMenu<Type>::closePopover(menu);
      auto gridView_p = (GridView<Type> *)g_object_get_data(G_OBJECT(menu), "gridView_p");

      auto selectionList = gridView_p->getSelectionList();
      if (selectionList){
        Dialogs::rmList(menu, selectionList);
      /*  TRACE("multiple selection...list=%p menu=%p\n", selectionList, menu);
        // do your thing
        clipboard_t::copyClipboardList(selectionList);
        // cleanup
        g_list_free(selectionList);
        g_object_set_data(G_OBJECT(menu), "selectionList", NULL);*/
        return;
      }

      auto info = G_FILE_INFO(g_object_get_data(G_OBJECT(menu), "info"));
      Dialogs::rm(info);
    }

    static void copy(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      MainMenu<Type>::closePopover(menu);
      auto gridView_p = (GridView<Type> *)g_object_get_data(G_OBJECT(menu), "gridView_p");
      auto selectionList = gridView_p->getSelectionList();
      auto c =(clipboard_t *)g_object_get_data(G_OBJECT(Child::mainWidget()), "ClipBoard");
      if (selectionList){
        TRACE("multiple selection...list=%p menu=%p\n", selectionList, menu);
        // do your thing
        c->copyClipboardList(selectionList);
        // cleanup
        // No need to reload since copy items not emblemed (factory.hh)
        // MainWindow<Type>::update(g_strdup(Child::getWorkdir()));
        Basic::freeSelectionList(selectionList);
        gtk_selection_model_unselect_all(Child::selection());
        return;
      }

      auto path = getPath(menu);
      if (!path) return;
      else {TRACE("path is %s\n", path);}
      c->copyClipboardPath(path);
      g_free(path);
    }

    static void cut(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      MainMenu<Type>::closePopover(menu);
      auto gridView_p = (GridView<Type> *)g_object_get_data(G_OBJECT(menu), "gridView_p");
      auto selectionList = gridView_p->getSelectionList();
      auto c =(clipboard_t *)g_object_get_data(G_OBJECT(Child::mainWidget()), "ClipBoard");
      if (selectionList){
        TRACE("multiple selection...list=%p menu=%p\n", selectionList, menu);
        // do your thing
        c->cutClipboardList(selectionList);
        // cleanup
        // Disabling reload since deadlock encountered 2025-02-06 
        // but might not be related to deadlock, reenabled 2025-08-21.
        MainWindow<Type>::update(g_strdup(Child::getWorkdir()));
        Basic::freeSelectionList(selectionList);
        gtk_selection_model_unselect_all(Child::selection());
        return;
      }
      auto path = getPath(menu);
      if (!path) return;
      else {TRACE("path is %s\n", path);}
      c->cutClipboardPath(path);
      g_free(path);
    }

    static void 
    run(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      MainMenu<Type>::closePopover(menu);
      
      auto path = getPath(menu);
      if (!path) return;
      else {TRACE("path is %s\n", path);}

      auto defaultApp = getDefaultApp(path);
      bool inTerminal = false;
      char *command = NULL;
      if (Basic::alwaysTerminal(defaultApp)) inTerminal = true;
      auto e = Basic::esc_string (path);

      if (inTerminal) {
        command = Run<Type>::mkTerminalLine(defaultApp, e);
      }
      else {
        command = Run<Type>::mkCommandLine(defaultApp, e);
      }
      g_free(e);

      TRACE("run %s \n", command);
      auto output = Child::getOutput();
      auto buttonSpace = Child::getButtonSpace();
      Prompt<Type>::run(output, command, true, true, buttonSpace);
      //object->prompt_p->run(output, command, true, true, object->buttonSpace);
      //object->prompt_p->run(output, command, true, true, object->buttonSpace);
      g_free(command);

      g_free(path);
      

      /*
        auto imageBox = G_OBJECT(g_object_get_data(G_OBJECT(menu), "imageBox"));
        auto info = G_FILE_INFO(g_object_get_data(G_OBJECT(imageBox), "info"));
        auto file = G_FILE(g_file_info_get_attribute_object (info, "standard::file"));
        auto path = g_file_get_path(file);

      new OpenWith<bool>(GTK_WINDOW(Child::mainWidget()), path);
      g_free(path);*/
 
    }
    
  };
}

#endif
