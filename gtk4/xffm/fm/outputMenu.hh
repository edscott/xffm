#ifndef OUTPUTMENU_HH
#define OUTPUTMENU_HH
#include "menu.hh"
namespace xf {
  class OutputMenu {
    public:
    const char **keys(void){
      static const char *keys_[] = { // Order is important.
        _("Clear all"),
        _("Copy"), 
        _("Select All"), 
        _("Foreground color"), 
        _("Background color"), 
        _("Default Colors"), 
        NULL
      };
      return keys_;
    }
    MenuInfo_t *iconNames(void){
      static MenuInfo_t menuIconNames_[] = { // Need not be complete with regards to keys_.
        {_("Clear all"),(void *) NULL}, 
        {_("Copy"),(void *) NULL}, 
        {_("Select All"),(void *) NULL}, 
        {_("Foreground color"),(void *) NULL}, 
        {_("Foreground color"),(void *) NULL}, 
        {_("Background color"), (void *) NULL},
        {_("Default Color"),(void *) NULL}, 
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
        {_("Foreground color"),(void *) "outputFg"}, 
        {_("Background color"), (void *) "outputBg"},
        {_("Default Colors"),(void *) "output"}, 
        {NULL, NULL}
      };
      return menuData_;      
    }

  private:
  };


}
#endif
