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
      
      //Workdir<bool>::setWorkdir(g_get_home_dir());
        
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
private:
  static void *setLabel_f(void *data){
    void **arg = (void **)data;
    auto markup = (char *)arg[0];
    auto label = GTK_LABEL(arg[1]);
    auto dialog = GTK_WINDOW(arg[2]);
    gtk_label_set_markup(label, markup);
    return NULL;
  }

  static void *thread1(void *data){
    pthread_t thread;
    pthread_create(&thread, NULL, thread2, data);
    void *retval;
    pthread_join(thread, &retval);
    DBG("thread2 joined, copy complete.\n");
    return NULL;
  }
  static void *thread2(void *data){
    void **arg = (void **)data;
    auto dialogObject = (DialogDrop<cpDropResponse> *)arg[0];
    auto list = (GList *)arg[1];
    auto path = (char *)arg[2];
    for (auto l=list; l && l->data; l=l->next){
      auto file = (char *)l->data;
      auto markup = g_strconcat("<span color=\"red\">",_("Copying"), 
          " ---> ", path,
          "</span>\n", file, NULL);
      void *arg2[] = { (void *)markup,
        (void *)dialogObject->label(),
        (void *)dialogObject->dialog(),
        NULL};
      Basic::context_function(setLabel_f, arg2);
      // requiere main context:dialogObject->setLabelText(label);
      DBG("thread2 %s --> %s\n", file, path);
      g_free(markup);
      g_free(file);
      sleep(1);
    }
    g_list_free(list);
    g_free(path);
    auto dialog = dialogObject->dialog();
    g_object_set_data(G_OBJECT(dialog), "response", GINT_TO_POINTER(1)); //yes

    return NULL;
  }
public:
    static void
    paste(GtkButton *self, void *data){
      /* for path, in menu create a hidden label with the path.
       * use label get text to retrieve path without memory leak. */
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(self), "menu"));
      gtk_popover_popdown(menu);
      auto info = G_FILE_INFO(g_object_get_data(G_OBJECT(menu), "info"));
      char *path = NULL;
      if (info) path = Basic::getPath(info);
      else {
        auto gridView_p = (GridView<Type> *)g_object_get_data(G_OBJECT(menu), "gridView_p");
        if (!gridView_p){
          DBG("Neither info nor gridView_p specified in menu.\n");
          return;
        }
        path = g_strdup(gridView_p->path());
      }

      //ClipBoard::pasteClip(path);
      DBG("paste to %s ...info=%p (currently disabled at menucallbacks.hh)\n", 
          path, info);

      auto c =(ClipBoard *)g_object_get_data(G_OBJECT(MainWidget), "ClipBoard");
      auto text = c->clipBoardCache();
      gchar **files = g_strsplit(text, "\n", -1);
    
      auto dialogObject = new DialogDrop<cpDropResponse>;
      auto dialog = dialogObject->dialog();
      g_object_set_data(G_OBJECT(dialog), "files", files);
      dialogObject->setParent(GTK_WINDOW(MainWidget));
      //dialogObject->subClass()->setDefaults(dialog, dialogObject->label());
      
      dialogObject->run(); // running in a thread...
      auto list = removeUriFormat(files);
      g_strfreev(files);
      void **arg = (void **)calloc(4, sizeof (void *));
      arg[0] = (void *)dialogObject;
      arg[1] = (void *)list;
      arg[2] = (void *)path;
      pthread_t thread;
      pthread_create(&thread, NULL, thread1, arg);
      pthread_detach(thread);

      DBG("thread 1 detached\n");
      

    /*  DBG("pasteClip(target=%s):\n%s\n", path, text);
      if (strncmp(text, "copy\n", strlen("copy\n")) == 0){
        
      } else if (strncmp(text, "move\n", strlen("move\n")) == 0){
      } else {
          DBG("ClipBoard::pasteClip: Invalid clipboard contents.\n");
      }
*/
      
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
      auto runButton = new (RunButton<Type>);
      runButton->init(runButton, xffm, childPid, output, path, buttonSpace);
      g_free(xffm);
    }
    
    static GList *
    removeUriFormat(gchar **files) {
        GList *fileList = NULL;
        for (auto f=files; f && *f; f++){
            gchar *file = *f;
            if (!strstr(file, URIFILE)) continue;
            if (strlen(file) > strlen(URIFILE)){
                if (strncmp(file, URIFILE, strlen(URIFILE))==0){
                    file = *f + strlen(URIFILE);
                }
            }
            fileList = g_list_prepend(fileList, g_strdup(file));
        }
        fileList = g_list_reverse(fileList);
        return fileList;
    }
 

  };


}
#endif
