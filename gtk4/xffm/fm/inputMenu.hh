#ifndef INPUTMENU_HH
#define INPUTMENU_HH
#include "menu.hh"
namespace xf {
  class InputMenu {
    public:
    const char **keys(void){
      static const char *keys_[] = { // Order is important.
        _("Select All"), //0x10
        _("Clear all"),
        _("Copy"), 
        _("Cut"), // 0x01
        _("Paste"), // 0x04
        _("Delete"), // 0x04
        _("Foreground color"), 
        _("Background color"), 
        _("Default Colors"), 
        NULL
      };
      return keys_;
    }
    MenuInfo_t *iconNames(void){
      static MenuInfo_t menuIconNames_[] = { // Need not be complete with regards to keys_.
        {_("Select all"),(void *) NULL}, 
        {_("Clear all"),(void *) NULL}, 
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
        {_("Select all"),(void *) NULL}, 
        {_("Clear all"),(void *) NULL}, 
        {_("Copy"),(void *) NULL}, 
        {_("Cut"),(void *) NULL}, 
        {_("Paste"),(void *) NULL}, 
        {_("Delete"),(void *) NULL}, 
        {_("Foreground color"),(void *) Util::terminalColors}, 
        {_("Background color"), (void *) Util::terminalColors},
        {_("Default Colors"),(void *) Util::defaultColors}, 
        {NULL, NULL}
      };
      return menuCallbacks_;
    }
    MenuInfo_t *data(void){
      static MenuInfo_t menuData_[] = { // Need not be complete with regards to keys_ nor menuCallbacks_.
        {_("Select all"),(void *) NULL}, 
        {_("Clear all"),(void *) NULL}, 
        {_("Copy"),(void *) NULL}, 
        {_("Cut"),(void *) NULL}, 
        {_("Paste"),(void *) NULL}, 
        {_("Delete"),(void *) NULL}, 
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
