#ifndef GRIDVIEWMENU_HH
#define GRIDVIEWMENU_HH
#include "menu.hh"
namespace xf {
  template <class Type> class Prompt;
  template <class Type> class MenuCallbacks;
  template <class Type>
  class GridviewMenu {
    public:
    GridviewMenu(void){}
    ~GridviewMenu(void){}

    const char **keys(void){
      static const char *keys_[] = { // Order is important.
        _("auto"), //
        _("Open in new tab"), //
        _("Open with"), //
        _("Create a compressed archive with the selected objects"),
        _("Mount the volume associated with this folder"),
        _("Unmount the volume associated with this folder"),
        _("Add bookmark"),
        _("Remove bookmark"),
        _("Copy"),
        _("Cut"),
        _("Paste"), // 
        _("Rename"),
        _("Duplicate"),
        _("Link"),
        _("Properties"),
        _("Delete"),
        _("Clipboard is empty."), // 
        NULL
      };
      return keys_;
    }

    MenuInfo_t *iconNames(void){
      static MenuInfo_t menuIconNames_[] = { // Need not be complete with regards to keys_.
        {_("Open in new tab"),(void *) DUAL_VIEW}, 
        {_("Open with"),(void *) "emblem-run"}, 
        {_("auto"),(void *) "emblem-run"}, 
        {_("Rename"),(void *) "document-save-as"}, 
        {_("Duplicate"),(void *) "document-save"}, 
        {_("Link"),(void *) "emblem-symbolic-link"}, 
        {_("Delete"),(void *) "delete"}, 
        {_("Properties"),(void *) "properties"}, 
        {_("Copy"),(void *) "copy"}, 
        {_("Cut"),(void *) "cut"}, 
        {_("Paste"),(void *) "paste"}, 
        {NULL, NULL}
      }; 
      return menuIconNames_;
    }
    MenuInfo_t *callbacks(void){
      static MenuInfo_t menuCallbacks_[] = { // Need not be complete with regards to keys_.
        {_("Open in new tab"),(void *) MenuCallbacks<Type>::openNewTab}, 
        {_("Open with"),(void *) openWith}, 
        {_("auto"),(void *) run}, 
        {_("Duplicate"),(void *) duplicate}, 
        {_("Link"),(void *) link}, 
        {_("Rename"),(void *) move}, 
        {_("Copy"),(void *) copy}, 
        {_("Cut"),(void *) cut}, 
        {_("Delete"),(void *) remove}, 
        {NULL, NULL}
      };
      return menuCallbacks_;
    }
    MenuInfo_t *data(void){
      static MenuInfo_t menuData_[] = { // Need not be complete with regards to keys_ nor menuCallbacks_.
        {_("Open in new tab"),(void *) NULL}, 
       // {_("Copy"),(void *) cpResponse::action}, 
        {NULL, NULL}
      };
      return menuData_;      
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
        auto info = G_FILE_INFO(g_object_get_data(G_OBJECT(menu), "info"));
        auto file = G_FILE(g_file_info_get_attribute_object (info, "standard::file"));
        return g_file_get_path(file);
    }


    static void 
    openWith(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto path = getPath(menu);
      new OpenWith<bool>(GTK_WINDOW(MainWidget), path);
      g_free(path);
    }

    static void duplicate(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto path = getPath(menu);
      DBG("path is %s\n", path);
      pathResponse<cpDialog>::action(path);
      g_free(path);
    }

    static void move(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto path = getPath(menu);
      DBG("path is %s\n", path);
      pathResponse<mvDialog>::action(path);
      g_free(path);
    }

    static void link(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto path = getPath(menu);
      DBG("path is %s\n", path);
      pathResponse<lnDialog>::action(path);
      g_free(path);
    }

    static void remove(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto path = getPath(menu);
      auto info = g_object_get_data(G_OBJECT(menu), "info");
      DBG("path = %s\n", path);
      auto dialogObject = new DialogButtons<rmResponse>;
      auto dialog = dialogObject->dialog();
      g_object_set_data(G_OBJECT(dialog), "path", path);
      g_object_set_data(G_OBJECT(dialog), "info", info);
      dialogObject->setParent(GTK_WINDOW(MainWidget));
      dialogObject->run();
    }

    static void copy(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto path = getPath(menu);
      ClipBoard::copyClipboardPath(path);
      g_free(path);
    }

    static void cut(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto path = getPath(menu);
      ClipBoard::cutClipboardPath(path);
      g_free(path);
    }

    static void 
    run(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      
      auto path = getPath(menu);
      /*auto imageBox = G_OBJECT(g_object_get_data(G_OBJECT(menu), "imageBox"));
      auto info = G_FILE_INFO(g_object_get_data(G_OBJECT(imageBox), "info"));
      auto file = G_FILE(g_file_info_get_attribute_object (info, "standard::file"));
      auto path = g_file_get_path(file);*/

      auto defaultApp = getDefaultApp(path);
      bool inTerminal = false;
      char *command = NULL;
      if (Basic::alwaysTerminal(defaultApp)) inTerminal = true;
      if (inTerminal) {
        command = Run<Type>::mkTerminalLine(defaultApp, path);
      }
      else {
        command = Run<Type>::mkCommandLine(defaultApp, path);
      }

      DBG("run %s \n", command);
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

      new OpenWith<bool>(GTK_WINDOW(MainWidget), path);
      g_free(path);*/
 
    }
    
  };
}

#endif
