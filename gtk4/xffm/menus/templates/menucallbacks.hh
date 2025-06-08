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
      auto child = Child::getChild();
      
        
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(child), "output"));
      auto pathbar = GTK_BOX(g_object_get_data(G_OBJECT(output), "pathbar"));
      const char *v[]={"cd", g_get_home_dir(), NULL};
      auto retval = Util<Type>::cd((const gchar **)v, child);

      auto path = Child::getWorkdir(child);
      // FIXME UtilPathbar::updatePathbar(path, pathbar, true);
      if (retval){
        //Print::print(output, g_strdup_printf("%s\n", Child::getWorkdir(child)));
        if (!History::add("cd")) ERROR_("History::add(%s) failed\n", "cd" );
      } else {
        Print::print(output, g_strdup_printf(_("failed to chdir to $HOME")));
      }
      return;
    }

    static bool workSpaceExists(const char *tt){
      auto command = "i3-msg -t get_workspaces";
      auto line = Basic::pipeCommandFull(command);
      auto childWidget =Child::getChild();
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(childWidget), "output"));
      auto token = g_strdup_printf("\"name\":\"%s\"", tt);
      auto p = strstr(line, token);
      //Print::showText(output);
      //Print::print(output, line); // This will free line.
      g_free(line);
      g_free(token);
      if (p) return true;
      return false;
   }

    static char *currentWorkSpace(void){
      auto command = "i3-msg -t get_workspaces";
      auto line = Basic::pipeCommandFull(command);
      char *tt;
      //Print::showText(output);
      //Print::print(output, line);
      auto v = g_strsplit(line, "}", -1);
      g_free(line);
      for (auto p=v; p && *p; p++){
        if (strstr(*p, "\"visible\":true")==NULL) continue;
        auto w = g_strsplit(*p, ",", -1);
        for (auto q=w; q && *q; q++){
          if (strstr(*q, "name")==NULL) continue;
          auto s=strchr(*q, ':');
          if (!s){
            ERROR_("No name entry from i3-msg: %s\n", *q);
            g_strfreev(w);
            g_strfreev(v);
            return NULL;
          }
          tt = g_strdup(s+1);
          for (auto t=tt; t && *t; t++){
            if (*t == '\"') *t = ' ';
          }
          g_strstrip(tt);
          //Print::print(output,g_strdup("split> ")); 
          //Print::print(output,g_strdup(tt)); 
          //Print::print(output,g_strdup("\n")); 
        }
        g_strfreev(w);
      }
      g_strfreev(v);
      return tt;
    }

    static void
    queryWS(void){
      using subClass_t = wsResponse<Type>;
      using dialog_t = DialogEntry<subClass_t>;
      auto dialogObject = new dialog_t;
      dialogObject->setParent(GTK_WINDOW(Child::mainWidget()));
      
      dialogObject->run();
      return;
    }

    static void
    moveWS(GtkButton *self, void *data){

      static char *lastWS=NULL;
      auto childWidget =Child::getChild();
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(childWidget), "output"));
      auto buttonSpace = GTK_BOX(g_object_get_data(G_OBJECT(childWidget), "buttonSpace"));

      auto tt = currentWorkSpace();
      if (!tt) return;

      if (lastWS && !workSpaceExists(lastWS)){
        Print::showText(output);
        auto a = g_strconcat("\"",lastWS, "\"", NULL);
        auto msg = g_strdup_printf(_("Workspace %s"), a);
        auto msg2 = g_strconcat(" ",msg, " ",  _("does not exist"), "\n",NULL);
        Print::printError(output, g_strdup_printf("%s\n", msg2));
        g_free(a);
        g_free(msg);

        g_free(lastWS);
        lastWS = NULL;
        // query for workspace... (lastWS)
        lastWS = NULL;
        queryWS(); return;
      } else if (lastWS) {
        /*Print::showText(output);
        auto a = g_strconcat("\"",lastWS, "\"", NULL);
        auto msg = g_strdup_printf(_("Workspace %s"), a);
        auto msg2 = g_strconcat(" ",msg, " ",  _("does not exist"), "\n",NULL);
        Print::print(output, g_strdup_printf("%s\n", msg2));
        g_free(a);
        g_free(msg);*/
      }
      
      if (strcmp(tt, "xffm4")==0){
        // Here we are at xffm4 desktop
        if (lastWS == NULL){
          // Here we entered xffm4 destop not by moveWS().
          g_free(tt);
          queryWS(); return;
          return;
        }
      }

      moveTo( (const char *)lastWS?lastWS:"xffm4");
      if (lastWS == NULL){
        // We moved out to xffm4.
        lastWS = g_strdup(tt);
      } else {
        // We moved back from xffm4.
        // Switch desktop to lastWS.
        switchWS(lastWS);
        g_free(lastWS);
        lastWS = NULL;
      }
      return;
    }
   
    static void switchWS(const char *tab){
      auto msg = g_find_program_in_path("i3-msg");
      if (!msg){
        ERROR_("i3-msg program not found\n");
        return;
      }

      const char *arg[]={(const char *)msg, 
        "-q",
        "workspace",
        tab, NULL};
      auto pid = fork();
      if (!pid){
        TRACE("switching to %s\n", tab);
        execvp(msg, (char * const *)arg);
        _exit(1);
      }
      usleep(500);
      waitpid(pid, NULL, WNOHANG);
      g_free(msg);

      return;

    }

   
    static void moveTo(const char *tab){
      auto msg = g_find_program_in_path("i3-msg");
      if (!msg){
        ERROR_("i3-msg program not found\n");
        return;
      }

      const char *arg[]={(const char *)msg, 
        "-q","move", "container", "to", "workspace",
        tab, NULL};
      auto pid = fork();
      if (!pid){
        TRACE("moving to %s\n", tab);
        execvp(msg, (char * const *)arg);
        _exit(1);
      }
      usleep(500);
      waitpid(pid, NULL, WNOHANG);
      g_free(msg);

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

      auto runButton = new RunButton<Type>(OPEN_TERMINAL, NULL);
      runButton->init(terminal, childPid, output, workDir, buttonSpace);
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

      auto runButton = new RunButton<Type>(EMBLEM_NEW_WINDOW,NULL);
      runButton->init(xffm4, childPid, output, workDir, buttonSpace);
      g_free(xffm4);
      return;
    }

    static void
    openFind(GtkButton *self, void *data){
      auto childWidget =Child::getChild();
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(childWidget), "output"));
      auto buttonSpace = GTK_BOX(g_object_get_data(G_OBJECT(childWidget), "buttonSpace"));
      auto workDir = Child::getWorkdir(childWidget);

//      auto find = g_strdup_printf("xffm --find %s", workDir);
      auto find = g_strdup_printf("xffm4 --find %s", workDir);
      pid_t childPid = Run<Type>::shell_command(output, find, false, false);
      TRACE("*** command = %s\n", find);
      auto runButton = new RunButton<Type>(EMBLEM_FIND, NULL);
      runButton->init(find, childPid, output, workDir, buttonSpace);
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
            ERROR_("Neither info nor gridView_p nor path specified in menu.\n");
            return;
          }
        }
      }
      if (!target){
        ERROR_("menucallbacks.hh::paste() should not happen, target==NULL\n");
        exit(1);
      }


      cpDropResponse::performPasteAsync(target);
      g_free(target);

    }

    static void
    showPaste(GtkButton *self, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(self), "menu"));
      gtk_popover_popdown(menu);
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
      auto w = (MainWindow<Type> *)g_object_get_data(G_OBJECT(Child::mainWidget()), "MainWindow");
      auto path = (const char *)g_object_get_data(G_OBJECT(menu), "path");
      if (!path){
        auto info = G_FILE_INFO(g_object_get_data(G_OBJECT(menu), "info"));
        if (!info){
          auto gridView_p = (GridView<Type> *)g_object_get_data(G_OBJECT(menu), "gridView_p");
          if (!gridView_p){
            ERROR_("*** Error: neither path nor info nor gridView_p set for menu.\n");
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
      gtk_widget_set_visible(Child::mainWidget(), FALSE);
      exit(0);
      //gtk_window_destroy(GTK_WINDOW(Child::mainWidget()));
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
      TRACE("menucallbacks.hh:: deleteTxt inactive\n");
      // FIXME (what?)
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
    clearTxt(GtkButton *button, void *data){
      auto output = Child::getOutput();
      auto txt = (const char *)data;
      GtkTextView *textView = NULL;

      if (txt && strcmp(txt, "output")==0) textView = Child::getOutput();
      if (txt && strcmp(txt, "input")==0)  textView =Child::getInput();
      if (!textView) return;

      Print::clearText(textView);
    }

    static void
    clearAllTxt(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      clearTxt(button, data);
    }
      


private:
    static void
    openXffm(GtkPopover *menu, const char *path){
      gtk_popover_popdown(menu);
      auto output = Child::getOutput();
      auto buttonSpace = Child::getButtonSpace();
      auto xffm = g_strdup_printf("xffm -f %s", path);
      pid_t childPid = Run<bool>::shell_command(output, xffm, false, false);
      auto runButton = new RunButton<Type>(EMBLEM_NEW_WINDOW,NULL);
      runButton->init(xffm, childPid, output, path, buttonSpace);
      g_free(xffm);
    }
 

  };


}
#endif
