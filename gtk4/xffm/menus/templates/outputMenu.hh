#ifndef OUTPUTMENU_HH
#define OUTPUTMENU_HH
namespace xf {
  template <class Type> class Util;
  template <class Type> class MenuCallbacks;
  template <class Type> class OutputMenu {
    public:
    const char **keys(void){
      static const char *keys_[] = { // Order is important.
        _("Clear"),
        _("Copy"), 
        _("Select All"), 
        _("Foreground color"), 
        _("Background color"), 
        _("Default Colors"), 
        _("Show Clipboard"), // 
        _("Close"), 
        NULL
      };
      return keys_;
    }
    MenuInfo_t *iconNames(void){
      static MenuInfo_t menuIconNames_[] = { // Need not be complete with regards to keys_.
        {_("Clear"),(void *) EMBLEM_CLEAR}, 
        {_("Copy"),(void *) EMBLEM_COPY}, 
        {_("Select All"),(void *) EMBLEM_SELECT_ALL}, 
        {_("Foreground color"),(void *) EMBLEM_GRAPHICS}, 
        {_("Background color"), (void *) EMBLEM_GRAPHICS},
        {_("Default Color"),(void *) EMBLEM_UNDO}, 
        {_("Show Clipboard"),(void *) EMBLEM_ABOUT}, 
        {_("Close"),(void *) WINDOW_CLOSE},
        {NULL, NULL}
      }; 
      return menuIconNames_;
    }
    MenuInfo_t *callbacks(void){
      static MenuInfo_t menuCallbacks_[] = { // Need not be complete with regards to keys_.
        {_("Clear"),(void *) MenuCallbacks<Type>::clearAllTxt}, 
        {_("Copy"),(void *) MenuCallbacks<Type>::copyTxt}, 
        {_("Select All"),(void *) MenuCallbacks<Type>::selectAllTxt}, 
        {_("Foreground color"),(void *) Util<Type>::terminalColors}, 
        {_("Background color"), (void *) Util<Type>::terminalColors},
        {_("Default Colors"),(void *) Util<Type>::defaultColors}, 
        {_("Show Clipboard"),(void *) MenuCallbacks<Type>::showPaste}, 
        {_("Close"),(void *) MainMenu<Type>::closeMenu},
        {NULL, NULL}
      };
      return menuCallbacks_;
    }
    MenuInfo_t *data(void){
      static MenuInfo_t menuData_[] = { // Need not be complete with regards to keys_ nor menuCallbacks_.
        {_("Clear"),(void *) "output"}, 
        {_("Copy"),(void *) "output"}, 
        {_("Select All"),(void *) "output"}, 
        {_("Foreground color"),(void *) "outputFg"}, 
        {_("Background color"), (void *) "outputBg"},
        {_("Default Colors"),(void *) "output"}, 
        {_("Show Clipboard"),(void *) NULL}, 
        {NULL, NULL}
      };
      return menuData_;      
    }
    const char **checkboxes(void){
      static const char *boxes_[] = {NULL};
      return boxes_;      
    }
    const char **radioboxes(void){
      static const char *boxes_[] = { NULL};
      return boxes_;      
    }

  private:
    static void
    copy(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      MainMenu<Type>::closePopover(menu);
    }

  };


}
#endif
