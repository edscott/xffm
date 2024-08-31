#ifndef ICONCOLORMENU_HH
#define ICONCOLORMENU_HH
#include "menu.hh"
namespace xf {
  class IconColorMenu {
    public:
    const char **keys(void){
      static const char *keys_[] = { // Order is important.
        _("Foreground color"),
        _("Background color"), 
        _("Default Colors"), 
        NULL
      };
      return keys_;
    }
    MenuInfo_t *iconNames(void){
      static MenuInfo_t menuIconNames_[] = { // Need not be complete with regards to keys_.
        {_("Foreground color"),(void *) NULL}, 
        {_("Background color"), (void *) NULL},
        {_("CuDefault Colorst"),(void *) NULL}, 
        {NULL, NULL}
      }; 
      return menuIconNames_;
    }
    MenuInfo_t *callbacks(void){
      static MenuInfo_t menuCallbacks_[] = { // Need not be complete with regards to keys_.
        {_("Foreground color"),(void *) Util::terminalColors}, 
        {_("Background color"), (void *) Util::terminalColors},
        {_("Default Colors"),(void *) Util::defaultColors}, 
        {NULL, NULL}
      };
      return menuCallbacks_;
    }
    MenuInfo_t *data(void){
      static MenuInfo_t menuData_[] = { // Need not be complete with regards to keys_ nor menuCallbacks_.
        {_("Foreground color"),(void *) "iconsFg"}, 
        {_("Background color"), (void *) "iconsBg"},
        {_("Default Colors"),(void *) "icons"}, 
        {NULL, NULL}
      };
      return menuData_;      
    }

  private:
  };


}
#endif
