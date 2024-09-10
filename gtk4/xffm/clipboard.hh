#ifndef CLIPBOARD_HH
#define CLIPBOARD_HH
namespace xf {
  static GdkClipboard *clipBoardTxt=NULL;
  class Clipboard {
    public:
    static void
    readDone(GObject* source_object, GAsyncResult* result,  gpointer data){
      GError *error_ = NULL;
      auto string = gdk_clipboard_read_text_finish(clipBoardTxt, result, &error_);
      if (error_){
        DBG("Error:: putInClipBoard(): %s\n", error_->message);
        g_error_free(error_);
        return;
      }
      TRACE("readDone: string = %s\n", string);
      auto output = Child::getOutput();
      Print::showText(output);
      Print::print(output, "blue/default_output_bg", g_strdup(_("Clipboard contents")));
      Print::print(output, "blue/default_output_bg", g_strdup(":\n"));
      Print::print(output, "brown/default_output_bg", string);

      
    }
    static void 
    printClipBoard(GdkClipboard *clipBoard){
        if (!clipBoard) return;
        //gdk_clipboard_set_text (clipBoardTxt, "foo");
        gdk_clipboard_read_text_async (clipBoardTxt, NULL, readDone, NULL);
        return;
    }
    static void
    feedClipboard(GtkButton *self, void *data){
      auto menu = GTK_WIDGET(g_object_get_data(G_OBJECT(self), "menu")); 
      gtk_popover_popdown(GTK_POPOVER(menu));
      if (!clipBoardTxt) clipBoardTxt = gdk_display_get_clipboard(gdk_display_get_default());
      
      auto textview = GTK_TEXT_VIEW(gtk_widget_get_parent(menu));
      auto buffer = gtk_text_view_get_buffer(textview);
      auto cut = GPOINTER_TO_INT(data);
      if (cut) gtk_text_buffer_cut_clipboard (buffer,clipBoardTxt, TRUE);
      else gtk_text_buffer_copy_clipboard (buffer,clipBoardTxt);
      //printClipBoard(clipBoardTxt);
    }
    static void
    pasteClipboard(GtkButton *self, void *data){
      auto menu = GTK_WIDGET(g_object_get_data(G_OBJECT(self), "menu")); 
      gtk_popover_popdown(GTK_POPOVER(menu));
      if (!clipBoardTxt) clipBoardTxt = gdk_display_get_clipboard(gdk_display_get_default());

      auto textview = GTK_TEXT_VIEW(gtk_widget_get_parent(menu));
      auto buffer = gtk_text_view_get_buffer(textview);      
      gtk_text_buffer_paste_clipboard (buffer, clipBoardTxt, NULL, TRUE);
    }
    static void
    deleteSelectionTxt(GtkButton *self, void *data){
      auto menu = GTK_WIDGET(g_object_get_data(G_OBJECT(self), "menu")); 
      gtk_popover_popdown(GTK_POPOVER(menu));

      auto textview = GTK_TEXT_VIEW(gtk_widget_get_parent(menu));
      auto buffer = gtk_text_view_get_buffer(textview);      
      gtk_text_buffer_delete_selection (buffer, TRUE, TRUE);
    }
    static void
    selectAllTxt(GtkButton *self, void *data){
      auto menu = GTK_WIDGET(g_object_get_data(G_OBJECT(self), "menu")); 
      gtk_popover_popdown(GTK_POPOVER(menu));

      auto textview = GTK_TEXT_VIEW(gtk_widget_get_parent(menu));
      auto buffer = gtk_text_view_get_buffer(textview);  
      GtkTextIter start, end;
      gtk_text_buffer_get_bounds (buffer, &start, &end);
      gtk_text_buffer_select_range(buffer, &start, &end);
      
    }

  };

}

#endif
