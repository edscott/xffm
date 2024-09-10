#ifndef MENUCALLBACKS_HH
#define MENUCALLBACKS_HH
namespace xf {
  template <class Type> class Run;
  //template <class VbuttonClass, class PageClass> class MainWindow;
  
  //template <class VbuttonClass, class PageClass>
  template <class Type>
  class MenuCallbacks {
    public:
    static void
    paste(GtkButton *self, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(self), "menu"));
      gtk_popover_popdown(menu);
      DBG("paste...\n");
    }
    static void
    showPaste(GtkButton *self, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(self), "menu"));
      gtk_popover_popdown(menu);
      DBG("showPaste...\n");
      printClipBoard();
    }
 
    static void
    openXffmPathbar(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto path = (const char *)g_object_get_data(G_OBJECT(menu), "path");
      openXffm(menu, path);
      return;
    }

    static void
    openXffmMain(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto workDir = Child::getWorkdir();
      openXffm(menu, workDir);
      return;
    }

    static void
    openNewTab(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto path = (const char *)g_object_get_data(G_OBJECT(menu), "path");
      auto w = (MainWindow *)g_object_get_data(G_OBJECT(MainWidget), "MainWindow");
      w->addPage(path);
      return;
    }

    static void
    close(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu"));
      gtk_popover_popdown(menu);
      gtk_widget_set_visible(MainWidget, FALSE);
      gtk_window_destroy(GTK_WINDOW(MainWidget));
    }


private:
    static void
    openXffm(GtkPopover *menu, const char *path){
      gtk_popover_popdown(menu);
      auto output = Child::getOutput();
      auto buttonSpace = Child::getButtonSpace();
      auto xffm = g_strdup_printf("xffm -f %s", path);
      pid_t childPid = Run<bool>::shell_command(output, xffm, false, false);
      auto runButton = new (RunButton);
      runButton->init(runButton, xffm, childPid, output, path, buttonSpace);
      g_free(xffm);
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
      auto output = Child::getOutput();
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
