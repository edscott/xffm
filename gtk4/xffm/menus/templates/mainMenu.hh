#ifndef MAINMENU_HH
#define MAINMENU_HH
// This menu is not using the Menu class template.
namespace xf {
  template <class Type> class RunButton;
  template <class Type>
  class MainMenu {
    using clipboard_t = ClipBoard<LocalDir>;
    public:
    const char **keys(void){
      static const char *keys_[] = { // Order is important.
        "test",
//          _("Search"),
//          _("Open terminal"),
        _("Paste"), // 
        _("Open in New Window"), 
        _("Toggle Text Mode"),
        _("Add bookmark"),
        _("Remove bookmark"),
        
        _("Select All"), 
        _("Match regular expression"), 
        _("Empty trash bin"), 

        " ",
        _("Show"),
        _("Hidden files"),
        _("Backup files"),

        _("Sort mode"),

        _("Descending"),
        _("Name"),
        _("Date"),
        _("Size"),
        _("File type"),
        _("Apply modifications"),
/*        
        "test",
        _("Show Clipboard"), // 
        _("Clear Clipboard History"), // 
       _("Color settings"),
       */
        "", 
        _("Exit"), 
        NULL
      };
      return keys_;
    }
    MenuInfo_t *iconNames(void){
      static MenuInfo_t menuIconNames_[] = { // Need not be complete with regards to keys_.
        {_("Empty trash bin"),(void *) EMBLEM_TRASH_FULL}, 
        {_("Paste"),(void *) EMBLEM_PASTE}, 
        {_("Select All"),(void *) EMBLEM_SELECT_ALL}, 
        {_("Add bookmark"),(void *) EMBLEM_BOOKMARK}, 
        {_("Remove bookmark"),(void *) EMBLEM_BOOKMARK}, 
        {_("Toggle Text Mode"),(void *) EMBLEM_TOGGLE}, 
        {_("Search"),(void *) SEARCH}, 
        {_("Open terminal"),(void *) OPEN_TERMINAL}, 
        {_("Home"),(void *) GO_HOME}, 
        {_("Show Clipboard"),(void *) EMBLEM_IMPORTANT}, // 
        {_("Clear Clipboard History"),(void *) EDIT_CLEAR}, // 
        {_("Open in New Window"),(void *)EMBLEM_NEW_WINDOW}, 
        {_("Color settings"),(void *)DOCUMENT_PROPERTIES}, 
        {_("Exit"),(void *)  WINDOW_SHUTDOWN},
        {_("Apply modifications"),(void *) EMBLEM_APPLY},
        {NULL, NULL}
      }; 
      return menuIconNames_;
    }
    MenuInfo_t *callbacks(void){
      static MenuInfo_t menuCallbacks_[] = { 
        // Need not be complete with regards to keys_.
        {_("Empty trash bin"),(void *) MenuCallbacks<Type>::emptyTrash}, 
        {_("Paste"),(void *) MenuCallbacks<Type>::paste}, 
        {_("Toggle Text Mode"),(void *) MenuCallbacks<Type>::popCall}, 
        {_("Add bookmark"),(void *) addB}, 
        {_("Remove bookmark"),(void *) removeB}, 
        {_("Select All"),(void *) selectAll}, 
        {_("Search"),(void *) MenuCallbacks<Type>::popCall}, 
        
        {_("Open terminal"),(void *) MenuCallbacks<Type>::popCall}, 
        {_("Home"),(void *) MenuCallbacks<Type>::popCall}, 
        //{_("Color settings"),(void *) MenuCallbacks<Type>::popCall}, 

        {_("Open in New Window"),(void *)openXffmMain}, 
        {"test",(void *)test},
        {_("Show Clipboard"),(void *) showPaste}, 
        {_("Clear Clipboard History"),(void *) clearPaste}, 
        {_("Exit"),(void *) close},

        {_("Hidden files"),(void *) toggleItem},
        {_("Backup files"),(void *) toggleItem},
        {_("Descending"),(void *) toggleItem},
        {_("Name"),(void *) toggleItem},
        {_("Date"),(void *) toggleItem},
        {_("Size"),(void *) toggleItem},
        {_("File type"),(void *) toggleItem},
        {_("Apply modifications"),(void *) apply},

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
        //
        {_("Toggle Text Mode"),(void *) MenuCallbacks<Type>::toggleVpane}, 
       // {_("Copy"),(void *) cpResponse::action}, 
        {_("Hidden files"),(void *) _("Hidden files")},
        {_("Backup files"),(void *) _("Backup files")},
        {_("Descending"),(void *) _("Descending")},
        {_("Name"),(void *) _("Name")},
        {_("Date"),(void *) _("Date")},
        {_("Size"),(void *) _("Size")},
        {_("File type"),(void *) _("File type")},
        {NULL, NULL}
};
      return menuData_;      
    }
    const char **checkboxes(void){
      static const char *boxes_[] = {
        _("Hidden files"),
        _("Backup files"),
        _("Descending"),
        NULL
      };
      return boxes_;      
    }
    const char **radioboxes(void){
      static const char *boxes_[] = {
        _("Name"),
        _("Date"),
        _("Size"),
        _("File type"),
        NULL
      };
      return boxes_;      
    }

  private:


    static void
    toggleItem(GtkCheckButton *check, gpointer data)
    {
      auto item = (const gchar *)data;
      TRACE("toggleItem: %s\n", item);
      int bit = 0;
      if (strcmp(item,_("Hidden files")) == 0) bit = 0x01;
      if (strcmp(item,_("Backup files")) == 0) bit = 0x02;
      if (strcmp(item,_("Descending")) == 0) bit = 0x04;
      if (strcmp(item,_("Date")) == 0) bit = 0x08;
      if (strcmp(item,_("Size")) == 0) bit = 0x10;
      if (strcmp(item,_("File type")) == 0) bit = 0x20;
      if (strcmp(item,_("Name")) == 0) bit = 0x40;
      auto gridView_p = (GridView<Type> *)Child::getGridviewObject();
      auto flags = gridView_p->flags();

      if (gtk_check_button_get_active(check)) {
        // Date and size are mutually exclusive.
        auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(check), "menu")); 
        gridView_p->flagOn(bit);

        if (bit == 0x08){ // _("Date")
          gridView_p->flagOff(0x10); // Size off
          gridView_p->flagOff(0x20); // type off
          gridView_p->flagOff(0x40); // name off
        } else if (bit == 0x10){ // _("Size")
          gridView_p->flagOff(0x08); // Date off
          gridView_p->flagOff(0x20); // type off
          gridView_p->flagOff(0x40); // name off
        } else if (bit == 0x20){ // _("File type")
          gridView_p->flagOff(0x10); // Size off
          gridView_p->flagOff(0x08); // Date off
          gridView_p->flagOff(0x40); // name off
        } else if (bit == 0x40){ // _("Name") 
          gridView_p->flagOff(0x08); // Date off
          gridView_p->flagOff(0x10); // Size off
          gridView_p->flagOff(0x20); // type off
        }


      }
      else gridView_p->flagOff(bit);
      TRACE("bit=0x%x, flag 0x%x->0x%x\n", bit, flags, gridView_p->flags());
      auto configFlags = Settings::getInteger("flags", gridView_p->path());
      if (configFlags < 0) configFlags = 0x40;
      
      auto popover = g_object_get_data(G_OBJECT(check), _("menu"));
      auto apply = g_object_get_data(G_OBJECT(popover), _("Apply modifications"));
      gtk_widget_set_sensitive(GTK_WIDGET(apply), configFlags != gridView_p->flags());

      //toggleGroupItem(menuItem, "LocalView", item);
    }

    static void 
    apply(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);

      auto gridview_p = (GridView<Type> *)Child::getGridviewObject();

      TRACE("apply %s...\n", gridview_p->path());
      Settings::setInteger("flags", gridview_p->path(), gridview_p->flags());
      //gtk_widget_unparent(GTK_WIDGET(menu));
      auto store = gridview_p->store();
      if (g_object_get_data(G_OBJECT(store), "xffm::root")){
        Workdir<Type>::setWorkdir(_("Bookmarks"));
        //Workdir<Type>::setWorkdir(_("Bookmarks"), Child::getPathbar(), false);
      } else if (g_object_get_data(G_OBJECT(store), "xffm::fstab")){
        Workdir<Type>::setWorkdir(_("Disk Mounter"));
        //Workdir<Type>::setWorkdir(_("Bookmarks"), Child::getPathbar(), false);
      } else {
        Workdir<Type>::setWorkdir(gridview_p->path());
      }
    }

    
    static void addB(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto gridView_p = (GridView<Type> *)Child::getGridviewObject();
      Bookmarks::addBookmark(gridView_p->path());
     }
    static void removeB(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto gridView_p = (GridView<Type> *)Child::getGridviewObject();
      Bookmarks::removeBookmark(gridView_p->path());
    }

    static void selectAll(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto gridView_p = (GridView<Type> *)g_object_get_data(G_OBJECT(menu), "gridView_p");
      if (!gridView_p) {
        DBG("selectAll: no gridView_p\n");
        return;
      }
      THREADPOOL->clear();
      Child::incrementSerial();
      auto selectionModel = gridView_p->selectionModel();
      gtk_selection_model_select_all (GTK_SELECTION_MODEL(selectionModel));

    }
    

/////////////////////////////////////////////////////////////////////
  
    static void
    test(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      Dialogs::info("foo and bar");
      MainWindow<Type>::resetAdj(250);

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

      // Save last size.
      auto width = gtk_widget_get_size(MainWidget,  GTK_ORIENTATION_HORIZONTAL);
      auto height = gtk_widget_get_size(MainWidget,  GTK_ORIENTATION_VERTICAL);
      gtk_widget_set_visible(MainWidget, FALSE);
      Settings::setInteger("window", "width", width);
      Settings::setInteger("window", "height", height);
#if 0
      auto n = gtk_notebook_get_n_pages(mainNotebook);
      for (auto i=0; i<n; i++){
        auto child = gtk_notebook_get_nth_page(mainNotebook, i);
        char buffer[64];
        snprintf(buffer, 64, "%p", child);
        auto history = g_strconcat(XF_HISTORY,".",buffer,NULL);
        //FIXME: append history to xf_history
        unlink(history);
        g_free(history);
      }
#endif
      //gtk_widget_unparent(GTK_WIDGET(menu));
      //gtk_window_destroy(GTK_WINDOW(MainWidget));

      exitDialogs = true;
    }

    static void
    showPaste(GtkButton *self, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(self), "menu"));
      gtk_popover_popdown(menu);
      clipboard_t::printClipBoard();
    }

    static void
    clearPaste(GtkButton *self, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(self), "menu"));
      gtk_popover_popdown(menu);
      clipboard_t::clearPaste();
    }

  };


}
#endif
