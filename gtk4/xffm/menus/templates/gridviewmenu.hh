#ifndef GRIDVIEWMENU_HH
#define GRIDVIEWMENU_HH
namespace xf {
  template <class Type> class Prompt;
  template <class Type> class MenuCallbacks;
  template <class Type> class Workdir;
  class LocalDir;
  template <class DirectoryClass>
  class GridviewMenu {
    public:
    GridviewMenu(void){}
    ~GridviewMenu(void){}

    const char **keys(void){
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
       // _("Select All"), 
       // _("Match regular expression"), 


        
        NULL
      };
      return keys_;
    }

    MenuInfo_t *iconNames(void){
      static MenuInfo_t menuIconNames_[] = { // Need not be complete with regards to keys_.
        {_("Mount Volume"),(void *) "emblem-greenball"}, 
        {_("Unmount Volume"),(void *) "emblem-redball"}, 
        {_("Open in new tab"),(void *) DUAL_VIEW}, 
        {_("Open with"),(void *) "emblem-run"}, 
        {_("auto"),(void *) "emblem-run"}, 
        {_("Rename"),(void *) "document-save-as"}, 
        {_("Duplicate"),(void *) "document-save"}, 
        {_("Link"),(void *) "emblem-symlink"}, 
        {_("Delete"),(void *) "delete"}, 
        {_("Properties"),(void *) "properties"}, 
        {_("Copy"),(void *) "copy"}, 
        {_("Cut"),(void *) "cut"}, 
        {_("Paste"),(void *) "paste"}, 
        {_("Add bookmark"),(void *) "emblem-favourite"}, 
        {_("Remove bookmark"),(void *) "emblem-redball"}, 
       {NULL, NULL}
      }; 
      return menuIconNames_;
    }
    MenuInfo_t *callbacks(void){
      static MenuInfo_t menuCallbacks_[] = { // Need not be complete with regards to keys_.
        {_("Mount Volume"),(void *) mount}, 
        {_("Unmount Volume"),(void *) unmount}, 
        {_("Toggle Text Mode"),(void *) MenuCallbacks<DirectoryClass>::popCall}, 
        {_("Open in new tab"),(void *) MenuCallbacks<DirectoryClass>::openNewTab}, 
        {_("Open with"),(void *) openWith}, 
        {_("auto"),(void *) run}, 
        {_("Duplicate"),(void *) duplicate}, 
        {_("Link"),(void *) link}, 
        {_("Rename"),(void *) move}, 
        {_("Copy"),(void *) copy}, 
        {_("Cut"),(void *) cut}, 
        {_("Paste"),(void *) MenuCallbacks<DirectoryClass>::paste}, 
        {_("Delete"),(void *) remove}, 
        {_("Add bookmark"),(void *) addB}, 
        {_("Remove bookmark"),(void *) removeB}, 
      //  {_("Select All"),(void *) selectAll}, 
        {_("Properties"),(void *) properties}, 
       
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

    static void 
    properties(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto info = G_FILE_INFO(g_object_get_data(G_OBJECT(menu), "info"));
      //TRACE("path= %s, info=%p\n", path, info);
      new Properties<bool>(info);
      //g_free(path);
    }
    
    static void addB(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      gchar *path = getPath(menu);
      
      if (path) {
        TRACE("path is %s\n", path);
        Bookmarks::addBookmark(path);
        g_free(path);
        auto gridView_p = (GridView<DirectoryClass> *)g_object_get_data(G_OBJECT(menu), "gridView_p");
        const char *p = gridView_p->path();
        Workdir<DirectoryClass>::setWorkdir(p);
        
      }
    }
    static void removeB(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      gchar *path = getPath(menu);
      
      if (path) {
        Bookmarks::removeBookmark(path);
        g_free(path);
        auto gridView_p = (GridView<DirectoryClass> *)g_object_get_data(G_OBJECT(menu), "gridView_p");
        const char *p = gridView_p->path();
        Workdir<DirectoryClass>::setWorkdir(p);
      }
    }

    static void 
    openWith(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto path = getPath(menu);

      if (!path) {
        auto gridView_p = (GridView<DirectoryClass> *)g_object_get_data(G_OBJECT(menu), "gridView_p");
        auto selectionList = gridView_p->getSelectionList();
        if (selectionList) {
          TRACE("selectionList = %p\n", selectionList);
         // auto list = getSelectionList();
         // new OpenWith<bool>(GTK_WINDOW(MainWidget), path);
          new OpenWith<bool>(GTK_WINDOW(MainWidget), NULL, selectionList);
        }  
      } else {
        new OpenWith<bool>(GTK_WINDOW(MainWidget), path, NULL);
      }
      g_free(path);
    }
     
    static void mount(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto path = getPath(menu);
      if (!path) return;
      else {TRACE("mount item path is %s\n", path);}
      
      auto folder = g_strconcat(g_get_home_dir(), G_DIR_SEPARATOR_S, "mnt", NULL);
      auto parent = GTK_WINDOW(MainWidget);
      new Mount<DirectoryClass>(parent, folder, path);
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
      void *retval;
      pthread_join(thread, &retval);
      // FIXME: we need to monitor the fstab directory to update emblems... 
      return NULL;
    }

    static void unmount(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto path = getPath(menu);
      if (!path) return;
      else {TRACE("unmount item %s\n", path);}
      pthread_t thread;
      pthread_create(&thread, NULL, umountThreadMaster, (void *)path);
      pthread_detach(thread);
    }
   
    static void duplicate(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto path = getPath(menu);
      if (!path) return;
      else {TRACE("path is %s\n", path);}
      dialogPath<cpResponse>::action(path);
      g_free(path);
    }

    static void move(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto path = getPath(menu);
      if (!path) return;
      else {TRACE("path is %s\n", path);}
      dialogPath<mvResponse>::action(path);
      g_free(path);
    }

    static void link(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto path = getPath(menu);
      if (!path) return;
      else {TRACE("path is %s\n", path);}
      dialogPath<lnResponse>::action(path);
      g_free(path);
    }

    static void remove(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto gridView_p = (GridView<DirectoryClass> *)g_object_get_data(G_OBJECT(menu), "gridView_p");

      auto selectionList = gridView_p->getSelectionList();
      if (selectionList){
        Dialogs::rmList(menu, selectionList);
      /*  TRACE("multiple selection...list=%p menu=%p\n", selectionList, menu);
        // do your thing
        ClipBoard::copyClipboardList(selectionList);
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
      gtk_popover_popdown(menu);
      auto gridView_p = (GridView<DirectoryClass> *)g_object_get_data(G_OBJECT(menu), "gridView_p");
      auto selectionList = gridView_p->getSelectionList();
      if (selectionList){
        TRACE("multiple selection...list=%p menu=%p\n", selectionList, menu);
        // do your thing
        ClipBoard::copyClipboardList(selectionList);
        // cleanup
        Basic::freeSelectionList(selectionList);
        return;
      }

      auto path = getPath(menu);
      if (!path) return;
      else {TRACE("path is %s\n", path);}
      ClipBoard::copyClipboardPath(path);
      g_free(path);
    }

    static void cut(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto gridView_p = (GridView<DirectoryClass> *)g_object_get_data(G_OBJECT(menu), "gridView_p");
      auto selectionList = gridView_p->getSelectionList();
      if (selectionList){
        TRACE("multiple selection...list=%p menu=%p\n", selectionList, menu);
        // do your thing
        ClipBoard::cutClipboardList(selectionList);
        // cleanup
        Basic::freeSelectionList(selectionList);
        return;
      }
      auto path = getPath(menu);
      if (!path) return;
      else {TRACE("path is %s\n", path);}
      ClipBoard::cutClipboardPath(path);
      g_free(path);
    }

    static void 
    run(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      
      auto path = getPath(menu);
      if (!path) return;
      else {TRACE("path is %s\n", path);}

      auto defaultApp = getDefaultApp(path);
      bool inTerminal = false;
      char *command = NULL;
      if (Basic::alwaysTerminal(defaultApp)) inTerminal = true;
      auto e = Basic::esc_string (path);

      if (inTerminal) {
        command = Run<DirectoryClass>::mkTerminalLine(defaultApp, e);
      }
      else {
        command = Run<DirectoryClass>::mkCommandLine(defaultApp, e);
      }
      g_free(e);

      TRACE("run %s \n", command);
      auto output = Child::getOutput();
      auto buttonSpace = Child::getButtonSpace();
      Prompt<DirectoryClass>::run(output, command, true, true, buttonSpace);
      //object->prompt_p->run(output, command, true, true, object->buttonSpace);
      //object->prompt_p->run(output, command, true, true, object->buttonSpace);
      g_free(command);

      g_free(path);
      

      /*
        auto imageBox = G_OBJECT(g_object_get_data(G_OBJECT(menu), "imageBox"));
        auto info = G_FILE_INFO(g_object_get_data(G_OBJECT(imageBox), "info"));
        auto file = G_FILE(g_file_info_get_attribute_object (info, "standard::file"));
        auto path = g_file_get_path(file);

      new OpenWith<bool>(GTK_WINDOW(MainWidget), path);
      g_free(path);*/
 
    }
    
  };
}

#endif
