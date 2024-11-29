#ifndef PATHBARMENU_HH
#define PATHBARMENU_HH
namespace xf {
  template <class Type> class MenuCallbacks;
  
  template <class Type>
  class PathbarMenu {
    public:
    const char **keys(void){
      static const char *keys_[] = { // Order is important.
        _("Open in new tab"), //
        _("Open in New Window"),
        _("Paste"), // 
        _("Show Clipboard"), // 
        _("Clipboard is empty."), // 
        NULL
      };
      return keys_;
    }
    MenuInfo_t *iconNames(void){
      static MenuInfo_t menuIconNames_[] = { // Need not be complete with regards to keys_.
        {_("Open in new tab"),(void *) NULL}, 
        {_("Open in New Window"),(void *) NULL}, 
        {_("Paste"),(void *) NULL}, 
        {_("Clipboard is empty."),(void *) NULL}, 
        {_("Show Clipboard"),(void *) NULL}, 
        {NULL, NULL}
      }; 
      return menuIconNames_;
    }
    MenuInfo_t *callbacks(void){
      static MenuInfo_t menuCallbacks_[] = { // Need not be complete with regards to keys_.
        {_("Open in new tab"),(void *) MenuCallbacks<Type>::openNewTab}, 
        {_("Open in New Window"),(void *) MenuCallbacks<Type>::openXffmPathbar}, 
        {_("Paste"),(void *)  MenuCallbacks<Type>::paste}, 
        {_("Clipboard is empty."),(void *) NULL}, 
        {_("Show Clipboard"),(void *) MenuCallbacks<Type>::showPaste}, 

        {NULL, NULL}
      };
      return menuCallbacks_;
    }
    MenuInfo_t *data(void){
      static MenuInfo_t menuData_[] = { // Need not be complete with regards to keys_ nor menuCallbacks_.
        {_("Open in new tab"),(void *) NULL}, 
        {_("Open in New Window"),(void *) NULL}, 
        {_("Paste"),(void *) NULL}, 
        {_("Clipboard is empty."),(void *) NULL}, 
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
    
  };
}

#endif
