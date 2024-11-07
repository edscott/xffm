#ifndef MAINMENU_HH
#define MAINMENU_HH
// This menu is not using the Menu class template.
namespace xf {
  template <class Type> class RunButton;
  template <class Type>
  class MainMenu {
    public:
    const char **keys(void){
      static const char *keys_[] = { // Order is important.
//          _("Search"),
//          _("Open terminal"),
        _("Open in New Window"), 
        "test",
        _("Show Clipboard"), // 
        _("Clear Clipboard History"), // 
       _("Color settings"),
        _("Close"), 
        NULL
      };
      return keys_;
    }
    MenuInfo_t *iconNames(void){
      static MenuInfo_t menuIconNames_[] = { // Need not be complete with regards to keys_.
        {_("Show/hide grid."),(void *)"media-view-subtitles" }, 
        {_("Search"),(void *) SEARCH}, 
        {_("Open terminal"),(void *) OPEN_TERMINAL}, 
        {_("Home"),(void *) GO_HOME}, 
        {_("Show Clipboard"),(void *) "emblem-important"}, // 
        {_("Clear Clipboard History"),(void *) EDIT_CLEAR}, // 
        {_("Open in New Window"),(void *)OPEN_FILEMANAGER}, 
        {_("Color settings"),(void *)DOCUMENT_PROPERTIES}, 
        {_("Close"),(void *)  WINDOW_SHUTDOWN},
        {NULL, NULL}
      }; 
      return menuIconNames_;
    }
    MenuInfo_t *callbacks(void){
      static MenuInfo_t menuCallbacks_[] = { // Need not be complete with regards to keys_.
        {_("Search"),(void *) MenuCallbacks<Type>::popCall}, 
        {_("Open terminal"),(void *) MenuCallbacks<Type>::popCall}, 
        {_("Home"),(void *) MenuCallbacks<Type>::popCall}, 
        //{_("Color settings"),(void *) MenuCallbacks<Type>::popCall}, 

        {_("Open in New Window"),(void *)openXffmMain}, 
        {"test",(void *)test},
        {_("Show Clipboard"),(void *) showPaste}, 
        {_("Clear Clipboard History"),(void *) clearPaste}, 
        {_("Close"),(void *) close},

        {NULL, NULL}
      };
      return menuCallbacks_;
    }
    MenuInfo_t *data(void){
      static MenuInfo_t menuData_[] = { // Need not be complete with regards to keys_ nor menuCallbacks_.
        {_("Search"),(void *) MenuCallbacks<Type>::openFind}, 
        {_("Open terminal"),(void *) MenuCallbacks<Type>::openTerminal}, 
        {_("Home"),(void *) MenuCallbacks<Type>::goHome}, 
        // FIXME: this must be submenu, with known unresolved parent issues
        //{_("Color settings"),(void *) MenuCallbacks<Type>::}, 
        {NULL, NULL}
      };
      return menuData_;      
    }
    const char **checkboxes(void){
      static const char *boxes_[] = {NULL};
      return boxes_;      
    }

  private:
    static void
    test(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      Dialogs::info("foo and bar");
      return;
    }

 /*    static void
    test(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto dialogObject = new DialogEntry<EntryResponse>;
      dialogObject->setParent(GTK_WINDOW(MainWidget));
      DBG("create dialogObject=%p\n", dialogObject); 
      dialogObject->run();

      return;
    }*/
   
    static void
    openXffmMain(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto workDir = Child::getWorkdir();
      openXffm(menu, workDir);
      return;
    }
    static void
    openXffm(GtkPopover *menu, const char *path){
      gtk_popover_popdown(menu);
      auto output = Child::getOutput();
      auto buttonSpace = Child::getButtonSpace();
      auto xffm = g_strdup_printf("xffm -f %s", path);
      pid_t childPid = Run<bool>::shell_command(output, xffm, false, false);
      auto runButton = new (RunButton<Type>);
      runButton->init(runButton, xffm, childPid, output, path, buttonSpace);
      g_free(xffm);
    }

    static void
    close(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu"));
      gtk_popover_popdown(menu);
      gtk_widget_set_visible(MainWidget, FALSE);
      gtk_window_destroy(GTK_WINDOW(MainWidget));
      exitDialogs = true;
    }

    static void
    showPaste(GtkButton *self, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(self), "menu"));
      gtk_popover_popdown(menu);
      ClipBoard::printClipBoard();
    }

    static void
    clearPaste(GtkButton *self, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(self), "menu"));
      gtk_popover_popdown(menu);
      ClipBoard::clearPaste();
    }

  };


}
#endif
