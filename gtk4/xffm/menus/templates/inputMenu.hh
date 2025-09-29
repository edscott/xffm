#ifndef INPUTMENU_HH
#define INPUTMENU_HH
namespace xf {
  template <class Type> class Util;
  template <class Type> class MenuCallbacks;
  template <class Type> class InputMenu {
    public:
    const char **keys(void){
      static const char *keys_[] = { // Order is important.
        _("Clear"),
        _("Copy"), 
        _("Cut"), // 
        _("Paste"), // 
        _("Delete"), // 
        _("Select All"), //
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
        {_("Select All"),(void *) EMBLEM_SELECT_ALL}, 
        {_("Clear"),(void *) EMBLEM_CLEAR}, 
        {_("Copy"),(void *) EMBLEM_COPY}, 
        {_("Cut"),(void *) EMBLEM_CUT}, 
        {_("Paste"),(void *) EMBLEM_PASTE}, 
        {_("Delete"),(void *) EMBLEM_DELETE}, 
        {_("Foreground color"),(void *) EMBLEM_GRAPHICS}, 
        {_("Background color"), (void *) EMBLEM_GRAPHICS},
        {_("Default Colors"),(void *) EMBLEM_UNDO}, 
        {_("Close"),(void *) WINDOW_CLOSE},
        {NULL, NULL}
      }; 
      return menuIconNames_;
    }
    MenuInfo_t *callbacks(void){
      static MenuInfo_t menuCallbacks_[] = { // Need not be complete with regards to keys_.
        {_("Select All"),(void *) MenuCallbacks<Type>::selectAllTxt}, 
        {_("Clear"),(void *) MenuCallbacks<Type>::clearAllTxt}, 
        {_("Copy"),(void *) MenuCallbacks<Type>::copyTxt}, 
        {_("Cut"),(void *) MenuCallbacks<Type>::cutTxt}, 
        {_("Paste"),(void *) MenuCallbacks<Type>::pasteTxt}, 
        {_("Delete"),(void *) MenuCallbacks<Type>::deleteTxt}, 
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
        {_("Select All"),(void *) "input"}, 
        {_("Clear"),(void *) "input"}, 
        {_("Copy"),(void *) "input"}, 
        {_("Cut"),(void *) "input"}, 
        {_("Paste"),(void *) "input"}, 
        {_("Delete"),(void *) "input"}, 
        {_("Foreground color"),(void *) "inputFg"}, 
        {_("Background color"), (void *) "inputBg"},
        {_("Default Colors"),(void *) "input"}, 
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
  };


}
#endif
