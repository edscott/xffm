#ifndef MENUCALLBACKS_HH
#define MENUCALLBACKS_HH
namespace xf {
  template <class Type> class MainWindow;
  template <class Type> class Run;
  template <class Type> class Util;
  template <class Type> class RunButton;
  template <class Type> class GridView;
  
  template <class Type>
  class MenuCallbacks {
    using clipboard_t = ClipBoard<LocalDir>;
    public:
    static void popCall(GtkButton *self, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(self), "menu"));
      gtk_popover_popdown(menu);
      auto f = G_CALLBACK(data);
      f();
    }

    static void
    toggleVpane (GtkButton *self, void *data){
      auto vpane = Child::getPane();
      auto position = gtk_paned_get_position(vpane);
      int height = gtk_widget_get_height(GTK_WIDGET(vpane));
      TRACE("position=%d, height=%d, 3/4height=%d\n", position, height, height * 3 / 4);
      if (position < height * 3 / 4) gtk_paned_set_position(vpane, height);
      else gtk_paned_set_position(vpane, 0);
      return ;
}

    static void
    goHome(GtkButton *self, void *data){
      //DBG("goHome....\n");
      auto child = Child::getChild();
      
        
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(child), "output"));
      auto pathbar = GTK_BOX(g_object_get_data(G_OBJECT(output), "pathbar"));
      const char *v[]={"cd", g_get_home_dir(), NULL};
      auto retval = Util<Type>::cd((const gchar **)v, child);

      auto path = Child::getWorkdir(child);
      // FIXME UtilPathbar::updatePathbar(path, pathbar, true);
      if (retval){
        //Print::print(output, g_strdup_printf("%s\n", Child::getWorkdir(child)));
        if (!History::add("cd")) DBG("History::add(%s) failed\n", "cd" );
      } else {
        Print::print(output, g_strdup_printf(_("failed to chdir to $HOME")));
      }
      return;
    }
    static void
    openTerminal(GtkButton *self, void *data){
      auto childWidget =Child::getChild();
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(childWidget), "output"));
      auto buttonSpace = GTK_BOX(g_object_get_data(G_OBJECT(childWidget), "buttonSpace"));
      auto workDir = Child::getWorkdir(childWidget);
        TRACE ("openTerminal::childWidget= %p, buttonSpace = %p workdir=%s\n", 
            childWidget, buttonSpace, workDir);

      auto terminal = Basic::getTerminal();
      pid_t childPid = Run<Type>::shell_command(output, terminal, false, false);

      auto runButton = new (RunButton<Type>);
      runButton->init(runButton, terminal, childPid, output, workDir, buttonSpace);
      return;
    }

    static void
    newWindow(GtkButton *self, void *data){
      auto childWidget =Child::getChild();
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(childWidget), "output"));
      auto buttonSpace = GTK_BOX(g_object_get_data(G_OBJECT(childWidget), "buttonSpace"));
      auto workDir = Child::getWorkdir(childWidget);
      if (strcmp(_("Bookmarks"), workDir) == 0) workDir = "";

      auto xffm4 = g_strdup_printf("xffm4 %s", workDir);
      pid_t childPid = Run<Type>::shell_command(output, xffm4, false, false);

      auto runButton = new (RunButton<Type>);
      runButton->init(runButton, xffm4, childPid, output, workDir, buttonSpace);
      g_free(xffm4);
      return;
    }

    static void
    openFind(GtkButton *self, void *data){
      auto childWidget =Child::getChild();
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(childWidget), "output"));
      auto buttonSpace = GTK_BOX(g_object_get_data(G_OBJECT(childWidget), "buttonSpace"));
      auto workDir = Child::getWorkdir(childWidget);

      auto find = g_strdup_printf("xffm --find %s", workDir);
      pid_t childPid = Run<Type>::shell_command(output, find, false, false);

      auto runButton = new (RunButton<Type>);
      runButton->init(runButton, find, childPid, output, workDir, buttonSpace);
      g_free(find);
      return;
    }

public:

    static void
    emptyTrash(GtkButton *self, void *data){
      static char *trashDir = NULL;
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(self), "menu"));
      gtk_popover_popdown(menu);
      if (!trashDir) trashDir = g_strdup_printf("%s/.local/share/Trash/files",
          g_get_home_dir());
      char *arg[]={(char *)"rm", (char *)"-rfv", trashDir, NULL};
      auto output = Child::getOutput();
      Run<bool>::thread_run(output, (const char **)arg, true);
      // FIXME: if in xffm::root,  trash bin icon reloaded
    }

    static void
    paste(GtkButton *self, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(self), "menu"));
      gtk_popover_popdown(menu);
      auto info = G_FILE_INFO(g_object_get_data(G_OBJECT(menu), "info"));
      char *target = NULL;
      if (info) target = Basic::getPath(info);
      else {
        auto gridView_p = (GridView<Type> *)g_object_get_data(G_OBJECT(menu), "gridView_p");
        if (gridView_p){
          target = g_strdup(gridView_p->path());
        } else {
          auto p = (const char *)g_object_get_data(G_OBJECT(menu), "path");
          if (p){
            target = g_strdup(p);
          } else {
            DBG("Neither info nor gridView_p nor path specified in menu.\n");
            return;
          }
        }
      }
      if (!target){
        DBG("menucallbacks.hh::paste() should not happen, target==NULL\n");
        exit(1);
      }


      cpDropResponse::performPasteAsync(target);
      g_free(target);
      

    /*  DBG("pasteClip(target=%s):\n%s\n", path, text);
      if (strncmp(text, "copy\n", strlen("copy\n")) == 0){
        
      } else if (strncmp(text, "move\n", strlen("move\n")) == 0){
      } else {
          DBG("clipboard_t::pasteClip: Invalid clipboard contents.\n");
      }
*/
      
    }

    static void
    showPaste(GtkButton *self, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(self), "menu"));
      gtk_popover_popdown(menu);
//      DBG("showPaste...valid = %d\n", Clipboard::validClipBoard());
      clipboard_t::printClipBoard();
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
      auto w = (MainWindow<Type> *)g_object_get_data(G_OBJECT(MainWidget), "MainWindow");
      auto path = (const char *)g_object_get_data(G_OBJECT(menu), "path");
      if (!path){
        auto info = G_FILE_INFO(g_object_get_data(G_OBJECT(menu), "info"));
        if (!info){
          auto gridView_p = (GridView<Type> *)g_object_get_data(G_OBJECT(menu), "gridView_p");
          if (!gridView_p){
            DBG("*** Error: neither path nor info nor gridView_p set for menu.\n");
            return;
          }
          w->addPage(gridView_p->path());
        } else {
          auto file = G_FILE(g_file_info_get_attribute_object (info, "standard::file"));
          auto _path = g_file_get_path(file);
          w->addPage(_path);
          g_free(_path);
        }
      } else {
        w->addPage(path);
      }
      return;
    }
/*
    static void
    close(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu"));
      gtk_popover_popdown(menu);
      gtk_widget_set_visible(MainWidget, FALSE);
      exit(0);
      //gtk_window_destroy(GTK_WINDOW(MainWidget));
    }
*/
    static void 
    copyTxt(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto txt = (const char *)data;
      if (txt && strcmp(txt, "output")==0) 
        clipboard_t::copyClipboardTxt(Child::getOutput());
      if (txt && strcmp(txt, "input")==0) 
        clipboard_t::copyClipboardTxt(Child::getInput());
    }

    static void 
    cutTxt(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto txt = (const char *)data;
      if (txt && strcmp(txt, "output")==0) 
        clipboard_t::cutClipboardTxt(Child::getOutput());
      if (txt && strcmp(txt, "input")==0) 
        clipboard_t::cutClipboardTxt(Child::getInput());
    }

    static void 
    deleteTxt(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto txt = (const char *)data;
      DBG("menucallbacks.hh:: deleteTxt inactive\n");
      // FIXME
      /*if (txt && strcmp(txt, "output")==0) 
        clipboard_t::cutClipboardTxt(Child::getOutput());
      if (txt && strcmp(txt, "input")==0) 
        clipboard_t::cutClipboardTxt(Child::getInput());*/
    }

    static void 
    pasteTxt(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto txt = (const char *)data;
      if (txt && strcmp(txt, "output")==0) 
        clipboard_t::pasteClipboardTxt(Child::getOutput());
      if (txt && strcmp(txt, "input")==0) 
        clipboard_t::pasteClipboardTxt(Child::getInput());
      
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
      auto runButton = new (RunButton<Type>);
      runButton->init(runButton, xffm, childPid, output, path, buttonSpace);
      g_free(xffm);
    }
 

  };


}
#endif
