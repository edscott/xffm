#ifndef XF_PROMPT_HH
#define XF_PROMPT_HH
namespace xf {
  template <class Type>
  class Prompt {

    public:
    GtkBox *promptBox(void){ return promptBox_;}
    GtkBox *buttonSpace(void){ return buttonSpace_;}
    GtkTextView *input(void){ return input_;}
    GtkTextView *dollar(void){ return dollar_;}
    private:
    GtkBox *promptBox_ = NULL;
    GtkTextView *input_ = NULL;
    GtkTextView *dollar_ = NULL;
    GtkBox *buttonSpace_ = NULL;
    
    GtkButton *promptButton_ = NULL;
    GtkButton *clearButton_ = NULL;
    GtkScale *sizeScale_ = NULL;

    public:
    
    Prompt(GtkTextView *output) {
      TRACE("constructor 3\n");
      input_ = UtilBasic::createInput(); 
      g_object_set_data(G_OBJECT(input_), "output", output);
      
      //g_object_set_data(G_OBJECT(input_), "child", child);

      auto keyController = gtk_event_controller_key_new();
      gtk_widget_add_controller(GTK_WIDGET(input_), keyController);
      g_signal_connect (G_OBJECT (keyController), "key-pressed", 
            G_CALLBACK (this->on_keypress), (void *)input_);
    }

    Prompt(GtkWidget *child) {
      GtkBox *buttonSpace = Child::getButtonSpace(child);
      GtkTextView *output = Child::getOutput(child);
      TRACE("constructor 1\n");
        buttonSpace_ = buttonSpace;
        input_ = UtilBasic::createInput(); 
        g_object_set_data(G_OBJECT(input_), "output", output);
        g_object_set_data(G_OBJECT(input_), "child", child);

        
        g_object_set_data(G_OBJECT(input_), "buttonSpace", buttonSpace_);
  
        auto keyController = gtk_event_controller_key_new();
        gtk_widget_add_controller(GTK_WIDGET(input_), keyController);
        g_signal_connect (G_OBJECT (keyController), "key-pressed", 
            G_CALLBACK (this->on_keypress), (void *)input_);
    }

    Prompt(void) {
      TRACE("constructor 0\n");
        promptBox_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        buttonSpace_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_hexpand(GTK_WIDGET(promptBox_), TRUE);
        auto dollarBox = createPrompt();
        input_ = UtilBasic::createInput(); 

        auto title = g_strconcat("",_("Input"),_(" TTY"), NULL);
        auto myInputMenu = new Menu<InputMenu<LocalDir > >(title);
        g_free(title);
        myInputMenu->setMenu(GTK_WIDGET(input_), GTK_WIDGET(input_), Child::getWorkdir(), true);
        delete myInputMenu;
        
        g_object_set_data(G_OBJECT(input_), "buttonSpace", buttonSpace_);
        g_object_set_data(G_OBJECT(input_), "promptBox", promptBox_);
        g_object_set_data(G_OBJECT(promptBox_), "buttonSpace", buttonSpace_);

        // child does not yet exist.
        //auto child =Child::getCurrentChild();
        //g_object_set_data(G_OBJECT(child), "buttonSpace", buttonSpace_);
        //TRACE ("Prompt::childWidget= %p, buttonSpace = %p\n", child, buttonSpace_);


        auto keyController = gtk_event_controller_key_new();
        gtk_widget_add_controller(GTK_WIDGET(input_), keyController);
        g_signal_connect (G_OBJECT (keyController), "key-pressed", 
            G_CALLBACK (this->on_keypress), (void *)input_);
        Basic::boxPack0 (promptBox_, GTK_WIDGET(dollarBox), FALSE, FALSE, 0);
        Basic::boxPack0 (promptBox_, GTK_WIDGET(input_), TRUE, TRUE, 0);
        Basic::boxPack0 (promptBox_, GTK_WIDGET(buttonSpace_), FALSE, TRUE, 0);
    }
    public:
    static pid_t
    run(GtkTextView *output, const gchar *command, bool withRunButton, bool showTextPane, GtkBox *buttonSpace){
      TRACE("Prompt::run: \"%s\"\n", command);

        auto child = GTK_WIDGET(g_object_get_data(G_OBJECT(output), "child"));
        pid_t childPID = 0;
        auto workdir = Child::getWorkdir(child);
        if (!command || !strlen(command)) return 0;
        // escape all quotes
        gchar *ncommand;
        if (strchr (command, '\"') || strchr(command,'\'')){
            gchar **g;
            if (strchr (command, '\"')) {
                g = g_strsplit(command, "\"", -1);
                ncommand = g_strjoinv ("\\\"", g);
                g_strfreev(g);
            } else {
                g = g_strsplit(command, "\'", -1);
                ncommand = g_strjoinv ("\\\'", g);
                g_strfreev(g);
            }
            TRACE("ncommand is %s\n", ncommand);
        } else ncommand = g_strdup(command);
        command = ncommand;
      TRACE("escaped run: %s\n", command);

        gchar *newWorkdir =NULL;
        gchar ** commands = NULL;
        commands = Basic::getVector(command, ";");
      TRACE("commands[0]: %s\n", commands[0]);
        RunButton<Type> *runButton;
        for (gchar **c=commands; c && *c; c++){
            if (strncmp(*c,"clear", strlen("clear"))==0){
              Print::clearText();
              continue;
            }
            if (strncmp(*c,"cd", strlen("cd"))==0){
              auto w = Basic::getVector(*c, " ");
              auto pathbar = GTK_BOX(g_object_get_data(G_OBJECT(output), "pathbar"));
              auto child = GTK_WIDGET(g_object_get_data(G_OBJECT(pathbar), "child"));
              if (Util<LocalDir >::cd((const char **)w, child)){
                auto path = Child::getWorkdir(child);
                Child::setWindowTitle(child);
                // FIXME UtilPathbar::updatePathbar(path, pathbar, true);
              }

              g_strfreev(w);
              TRACE("internal command=%s\n", *c);
              continue;
            }

            // automatic shell determination:
            if (!g_file_test(workdir, G_FILE_TEST_IS_DIR)) {
                if (chdir(g_get_home_dir()) < 0){
                    ERROR_("Cannot chdir to %s\n", g_get_home_dir());
                    TRACE("aborting command: \"%s\"\n", command);
                    continue;
                }
            } else {
                if (chdir(workdir) < 0){
                    ERROR_("Cannot chdir to %s\n", workdir);
                    TRACE("aborting command: \"%s\"\n", command);
                    continue;
                }
            }
            gboolean scrollup = FALSE;
            if (strncmp(command, "man", strlen("man"))==0) {
                scrollup = TRUE;
                Print::clear_text(output);
            }
            //Print::print(output, g_strdup_printf("TRACE> final run: %s\n",*c));
           /*
            * FIXME: enable automatic in terminal for always
            *        move always routine to utilBasic*/
        
            if (Basic::alwaysTerminal(*c)){
              command = Run<LocalDir >::mkTerminalLine(*c, "");
              childPID = Run<LocalDir >::shell_command(output, command, scrollup, showTextPane);
            } else {
              childPID = Run<LocalDir >::shell_command(output, *c, scrollup, showTextPane);
            }
            if (withRunButton) {
              runButton = new RunButton<Type>(NULL, command);
              runButton->init(*c, childPID, output, Child::getWorkdir(child), buttonSpace);
            }
            
            TRACE("command loop...\n");
//            if (withRunButton) newRunButton(*c, childPID);
        }
        g_strfreev(commands);
        g_free(ncommand); 
        return childPID;
    }
    private:
    static bool
    history(GtkTextView *input, guint keyval){
      switch (keyval){
        case GDK_KEY_Up:
        case GDK_KEY_KP_Up:
          History::up(input);
          return true;
      case GDK_KEY_Down:
      case GDK_KEY_KP_Down:
           History::down(input);
           return true;
      }
      return false;
    }
    static bool 
    pwd(GtkTextView *output, const char *text){
      auto pathbar = GTK_BOX(g_object_get_data(G_OBJECT(output), "pathbar"));
      auto child = GTK_WIDGET(g_object_get_data(G_OBJECT(pathbar), "child"));
      if (strcmp(text, "pwd")) return false;
      auto workdir = Child::getWorkdir(child);
      Print::print(output, g_strdup_printf("$ %s\n", text));
      Print::print(output, g_strdup(workdir));
      Print::print(output, g_strdup("\n"));
      if (!History::add(text)) ERROR_("History::add(%s) failed\n", text );
      return true;
    }
    static bool
    history(GtkTextView *output, const char *text){
      if (strcmp(text, "history")) return false;
      char *t = History::history();
      Print::print(output, g_strdup_printf("$ %s\n", text));
      Print::print(output, g_strdup_printf("%s", t));
      g_free(t);
      Print::scroll_to_bottom(output);
      return true;
    }
    
    static bool
    cd(GtkTextView *output, const char *text){
      gchar **v = Basic::getVector(text, " ");
      if (strcmp(v[0], "cd")) {
        g_strfreev(v);
        return false;
      }
      auto pathbar = GTK_BOX(g_object_get_data(G_OBJECT(output), "pathbar"));
      auto child = GTK_WIDGET(g_object_get_data(G_OBJECT(pathbar), "child"));
      auto retval = Util<LocalDir >::cd((const gchar **)v, child);
      auto path = Child::getWorkdir(child);
      // FIXME UtilPathbar::updatePathbar(path, pathbar, true);
      Print::print(output, g_strdup_printf("$ %s\n", text));
      if (retval){
        Print::print(output, g_strdup_printf("%s\n", Child::getWorkdir(child)));
        if (!History::add(text)) ERROR_("History::add(%s) failed\n", text );
      } else {
        Print::print(output, g_strdup_printf(_("failed to chdir to %s"), v[1]));
      }
      g_strfreev(v);
      return true;
    }
    static bool
    exe(GtkTextView *input, GtkTextView *output, const char *text){
      if (!History::add(text)) ERROR_("History::add(%s) failed\n", text );

/*
      gchar **v = Basic::getVector(text, " ");
      char *inPath = g_find_program_in_path(v[0]);
      if (!inPath && g_file_test(v[0], G_FILE_TEST_IS_EXECUTABLE)) inPath = realpath(v[0], NULL);
      if (!inPath && strncmp(v[0], "./", strlen("./")) == 0) inPath = g_strdup(v[0]);
        
      if (!inPath){
        Print::print(output, g_strdup_printf("$ %s\n", v[0]));
        Print::print(output, g_strdup_printf("%s: %s\n", v[0], _("Command not found.")));
        g_strfreev(v);
        return true;
      }
*/    
      /*
      Print::print(output, g_strdup_printf("$ %s", inPath));
      for (int i=1; v[i]; i++){
        Print::print(output, g_strdup_printf(" %s", v[i]));
      }
      Print::print(output, g_strdup_printf("\n"));
      g_free(inPath);
      g_strfreev(v);
      */

      auto buttonSpace = GTK_BOX(g_object_get_data(G_OBJECT(input), "buttonSpace"));
      
      run(output, text, true, true, buttonSpace);
      return true;
    }
    static bool
    com(GtkTextView *input, GtkTextView *output, guint keyval){
      if (keyval != GDK_KEY_Return && keyval != GDK_KEY_KP_Enter) return false;
      auto text = Print::inputText(input);
      if (pwd(output, text)) return true;
      if (history(output, text)) return true;
      if (cd(output, text)) return true;
      if (text && strcmp(text, "exit")==0) {
        // close window.
        auto notebook = GTK_NOTEBOOK(g_object_get_data(G_OBJECT(Child::mainWidget()), "notebook"));
        auto num = gtk_notebook_get_current_page(notebook);
        auto n = gtk_notebook_get_n_pages(notebook);
        if (n == 1) exit(0);
        gtk_notebook_remove_page(notebook,num);

        return true;
      }
      exe(input, output, text); 
      g_free(text);
      return true;
    }
    static 
    gboolean on_keypress(GtkEventControllerKey* self,
          guint keyval,
          guint keycode,
          GdkModifierType state,
          gpointer data)
    {
      //fprintf(stderr, "bar...\n");
        TRACE("prompt window_keyboard_event: keyval=%d (0x%x), keycode=%d (0x%x), modifying=%d, data= %p\n", 
            keyval, keyval, keycode, keycode, state, data);
        auto input = GTK_TEXT_VIEW(data);
        auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(input), "output"));

        if(keyval ==  GDK_KEY_Tab){
          Bash::complete(input, output);
          return TRUE;
        }

        if(keyval ==  GDK_KEY_Escape){
           gtk_widget_grab_focus(GTK_WIDGET(input));
           gtk_text_view_set_cursor_visible(input, TRUE);
           Basic::flushGTK();
           return TRUE;
        }

        if (history(input, keyval)) return TRUE;
        History::reset();

        if (com(input, output, keyval)){
          Print::clear_text(input);
          return TRUE;
        }

        return FALSE;
    }
    
    GtkBox *createPrompt(void){
        auto dollarBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_hexpand(GTK_WIDGET(dollarBox), FALSE);
        dollar_ = GTK_TEXT_VIEW(gtk_text_view_new ());
        
        UtilBasic::setFontCss(GTK_WIDGET(dollar_));
        gtk_widget_set_size_request(GTK_WIDGET(dollar_), 20, -1);
        
        Print::print(dollar_, g_strdup("$"));

        gtk_text_view_set_pixels_above_lines (dollar_, 5);
        gtk_text_view_set_pixels_below_lines (dollar_, 5);
        gtk_text_view_set_monospace (dollar_, TRUE);
        gtk_text_view_set_editable (dollar_, FALSE);
        gtk_text_view_set_cursor_visible (dollar_, FALSE);
        gtk_widget_set_can_focus(GTK_WIDGET(dollar_), FALSE);
        gtk_widget_add_css_class (GTK_WIDGET(dollar_), "input" );
        gtk_widget_add_css_class (GTK_WIDGET(dollar_), "inputview" );
        Basic::boxPack0 (dollarBox, GTK_WIDGET(dollar_), FALSE, FALSE, 0);
        return dollarBox;
    }
    
  };
}
#endif
