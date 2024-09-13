#ifndef OUTPUTMENU_HH
#define OUTPUTMENU_HH
#include "menu.hh"
namespace xf {
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
        _("Show Clipboard"), // 0x04
        NULL
      };
      return keys_;
    }
    MenuInfo_t *iconNames(void){
      static MenuInfo_t menuIconNames_[] = { // Need not be complete with regards to keys_.
        {_("Clear"),(void *) NULL}, 
        {_("Copy"),(void *) NULL}, 
        {_("Select All"),(void *) NULL}, 
        {_("Foreground color"),(void *) NULL}, 
        {_("Background color"), (void *) NULL},
        {_("Default Color"),(void *) NULL}, 
        {_("Show Clipboard"),(void *) NULL}, 
        {NULL, NULL}
      }; 
      return menuIconNames_;
    }
    MenuInfo_t *callbacks(void){
      static MenuInfo_t menuCallbacks_[] = { // Need not be complete with regards to keys_.
        {_("Clear"),(void *) clearAll}, 
        {_("Copy"),(void *) copy}, 
        {_("Select All"),(void *) selectAll}, 
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
        {_("Clear"),(void *) NULL}, 
        {_("Copy"),(void *) NULL}, 
        {_("Select All"),(void *) NULL}, 
        {_("Foreground color"),(void *) "outputFg"}, 
        {_("Background color"), (void *) "outputBg"},
        {_("Default Colors"),(void *) "output"}, 
        {_("Show Clipboard"),(void *) NULL}, 
        {NULL, NULL}
      };
      return menuData_;      
    }

  private:
    static void
    copy(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
    }

    static void
    selectAll(GtkButton *button, void *data){
      auto menu = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(GTK_POPOVER(menu));

      auto textview = GTK_TEXT_VIEW(gtk_widget_get_parent(menu));
      auto buffer = gtk_text_view_get_buffer(textview);  
      GtkTextIter start, end;
      gtk_text_buffer_get_bounds (buffer, &start, &end);
      gtk_text_buffer_select_range(buffer, &start, &end);
      
    }

    static void
    clearAll(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto output = Child::getOutput();
      Print::clearText(output);
    }
  };


}
#endif
