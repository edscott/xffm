#ifndef INPUTMENU_HH
#define INPUTMENU_HH
namespace xf {
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
        NULL
      };
      return keys_;
    }
    MenuInfo_t *iconNames(void){
      static MenuInfo_t menuIconNames_[] = { // Need not be complete with regards to keys_.
        {_("Select All"),(void *) NULL}, 
        {_("Clear"),(void *) NULL}, 
        {_("Copy"),(void *) NULL}, 
        {_("Cut"),(void *) NULL}, 
        {_("Paste"),(void *) NULL}, 
        {_("Delete"),(void *) NULL}, 
        {_("Foreground color"),(void *) NULL}, 
        {_("Background color"), (void *) NULL},
        {_("Default Colors"),(void *) NULL}, 
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
        {_("Foreground color"),(void *) Util::terminalColors}, 
        {_("Background color"), (void *) Util::terminalColors},
        {_("Default Colors"),(void *) Util::defaultColors}, 
        {_("Show Clipboard"),(void *) MenuCallbacks<Type>::showPaste}, 
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

  private:
  };


}
#endif
