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
      auto path = g_object_get_data(G_OBJECT(menu), "path");
      //ClipBoard::pasteClip(path);
      DBG("paste to %s...(currently disabled at menucallbacks.hh)\n", (char *)path);
    }
    static void
    showPaste(GtkButton *self, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(self), "menu"));
      gtk_popover_popdown(menu);
//      DBG("showPaste...valid = %d\n", Clipboard::validClipBoard());
      ClipBoard::printClipBoard();
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
      auto w = (MainWindow *)g_object_get_data(G_OBJECT(MainWidget), "MainWindow");
      auto path = (const char *)g_object_get_data(G_OBJECT(menu), "path");
      if (!path){
        auto info = G_FILE_INFO(g_object_get_data(G_OBJECT(menu), "info"));
        if (!info){
          DBG("*** Error: neither path nor info set for menu.\n");
          return;
        }
        auto file = G_FILE(g_file_info_get_attribute_object (info, "standard::file"));
        auto _path = g_file_get_path(file);
        w->addPage(_path);
        g_free(_path);
      } else {
        w->addPage(path);
      }
      return;
    }

    static void
    close(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu"));
      gtk_popover_popdown(menu);
      gtk_widget_set_visible(MainWidget, FALSE);
      gtk_window_destroy(GTK_WINDOW(MainWidget));
    }

    static void 
    copyTxt(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto txt = (const char *)data;
      if (txt && strcmp(txt, "output")==0) 
        ClipBoard::copyClipboardTxt(Child::getOutput());
      if (txt && strcmp(txt, "input")==0) 
        ClipBoard::copyClipboardTxt(Child::getInput());
    }

    static void 
    cutTxt(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto txt = (const char *)data;
      if (txt && strcmp(txt, "output")==0) 
        ClipBoard::cutClipboardTxt(Child::getOutput());
      if (txt && strcmp(txt, "input")==0) 
        ClipBoard::cutClipboardTxt(Child::getInput());
    }

    static void 
    deleteTxt(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto txt = (const char *)data;
      DBG("menucallbacks.hh:: deleteTxt inactive\n");
      // FIXME
      /*if (txt && strcmp(txt, "output")==0) 
        ClipBoard::cutClipboardTxt(Child::getOutput());
      if (txt && strcmp(txt, "input")==0) 
        ClipBoard::cutClipboardTxt(Child::getInput());*/
    }

    static void 
    pasteTxt(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto txt = (const char *)data;
      if (txt && strcmp(txt, "output")==0) 
        ClipBoard::pasteClipboardTxt(Child::getOutput());
      if (txt && strcmp(txt, "input")==0) 
        ClipBoard::pasteClipboardTxt(Child::getInput());
      
    }

    static void
    selectAllTxt(GtkButton *button, void *data){
      auto menu = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(GTK_POPOVER(menu));
      auto txt = (const char *)data;
      GtkTextView *textView = NULL;

      if (txt && strcmp(txt, "output")==0) textView = Child::getOutput();
      if (txt && strcmp(txt, "input")==0)  textView =Child::getInput();
      if (!textView) return;

      auto buffer = gtk_text_view_get_buffer(textView);  
      GtkTextIter start, end;
      gtk_text_buffer_get_bounds (buffer, &start, &end);
      gtk_text_buffer_select_range(buffer, &start, &end);     
    }

    static void
    clearAllTxt(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto output = Child::getOutput();
      auto txt = (const char *)data;
      GtkTextView *textView = NULL;

      if (txt && strcmp(txt, "output")==0) textView = Child::getOutput();
      if (txt && strcmp(txt, "input")==0)  textView =Child::getInput();
      if (!textView) return;

      Print::clearText(textView);
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
 

  };


}
#endif
