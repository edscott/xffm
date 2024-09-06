#ifndef PATHBARMENU_HH
#define PATHBARMENU_HH
#include "menu.hh"
namespace xf {
  class PathbarMenu {
    public:
    const char **keys(void){
      static const char *keys_[] = { // Order is important.
        _("Open in new tab"), //0x10
        _("Open in New Window"),
        _("Paste"), // 0x04
        _("Show Clipboard"), // 0x04
        _("Clipboard is empty."), // 0x04
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
        {_("Open in new tab"),(void *) openNewTab}, 
        {_("Open in New Window"),(void *) NULL}, 
        {_("Paste"),(void *) paste}, 
        {_("Clipboard is empty."),(void *) NULL}, 
        {_("Show Clipboard"),(void *) showPaste}, 

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
    private:
    static void
    openNewTab(GtkButton *self, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(self), "menu"));
      DBG("openNewTab...\n");
      gtk_popover_popdown(menu);

    }
    static void
    paste(GtkButton *self, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(self), "menu"));
      DBG("paste...\n");
      gtk_popover_popdown(menu);
    }
    static void
    showPaste(GtkButton *self, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(self), "menu"));
      DBG("showPaste...\n");
      printClipBoard();
      gtk_popover_popdown(menu);
    }

    static void
    readDone(GObject* source_object, GAsyncResult* result,  gpointer data){
      GdkClipboard *clipBoard = GDK_CLIPBOARD(source_object);
      GError *error_ = NULL;
      auto string = gdk_clipboard_read_text_finish(clipBoard, result, &error_);
      if (error_){
        DBG("Error:: readDone(): %s\n", error_->message);
        g_error_free(error_);
        return;
      }
      TRACE("readDone: string = %s\n", string);
      auto output = Child::getCurrentOutput();
      Print::showText(output);
      Print::print(output, "blue/default_output_bg", string);
    }
    static void 
    printClipBoard(void){
      GdkClipboard *clipBoard = gdk_display_get_clipboard(gdk_display_get_default());
      if (!clipBoard) return;
      //gdk_clipboard_set_text (clipBoardTxt, "foo");
      gdk_clipboard_read_text_async (clipBoard, NULL, readDone, NULL);
      return;
    }
    
  };
}

#endif
