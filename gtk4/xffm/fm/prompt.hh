#ifndef XF_PROMPT_HH
#define XF_PROMPT_HH
#include "run.hh"
namespace xf {
  class Prompt {

    public:
    GtkBox *promptBox(void){ return promptBox_;}
    GtkBox *buttonSpace(void){ return buttonSpace_;}
    GtkTextView *input(void){ return input_;}
    private:
    GtkBox *promptBox_;
    GtkTextView *input_;
    GtkBox *buttonSpace_;
    
    GtkButton *promptButton_;
    GtkButton *clearButton_;
    GtkScale *sizeScale_;

    public:

    Prompt(void) {
        promptBox_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        buttonSpace_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_hexpand(GTK_WIDGET(promptBox_), TRUE);
        auto dollar = createPrompt();
        input_ = createInput(); 
        g_object_set_data(G_OBJECT(input_), "buttonSpace", buttonSpace_);
        g_object_set_data(G_OBJECT(promptBox_), "buttonSpace", buttonSpace_);

        // child does not yet exist.
        //auto child =Util::getCurrentChild();
        //g_object_set_data(G_OBJECT(child), "buttonSpace", buttonSpace_);
        //DBG ("Prompt::childWidget= %p, buttonSpace = %p\n", child, buttonSpace_);


        auto keyController = gtk_event_controller_key_new();
        gtk_widget_add_controller(GTK_WIDGET(input_), keyController);
        g_signal_connect (G_OBJECT (keyController), "key-pressed", 
            G_CALLBACK (this->on_keypress), (void *)input_);
        Util::boxPack0 (promptBox_, GTK_WIDGET(dollar), FALSE, FALSE, 0);
        Util::boxPack0 (promptBox_, GTK_WIDGET(input_), TRUE, TRUE, 0);
        Util::boxPack0 (promptBox_, GTK_WIDGET(buttonSpace_), FALSE, TRUE, 0);
    }
    private:
    static pid_t
    run(GtkTextView *output, const gchar *command, bool withRunButton, bool showTextPane, GtkBox *buttonSpace){
      DBG("run: %s\n", command);
        pid_t child = 0;
        auto workdir = Util::getWorkdir();
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
      DBG("escaped run: %s\n", command);

        gchar *newWorkdir =NULL;
        gchar ** commands = NULL;
        commands = Util::getVector(command, ";");
      DBG("commands[0]: %s\n", commands[0]);
        RunButton *runButton;
        for (gchar **c=commands; c && *c; c++){
            if (strncmp(*c,"cd", strlen("cd"))==0){
              auto w = Util::getVector(*c, " ");
              Util::cd((const char **)w);
              g_strfreev(w);
              DBG("internal command=%s\n", *c);
              continue;
            }
            // automatic shell determination:
            if (!g_file_test(workdir, G_FILE_TEST_IS_DIR)) {
                if (chdir(g_get_home_dir()) < 0){
                    DBG("Cannot chdir to %s\n", g_get_home_dir());
                    DBG("aborting command: \"%s\"\n", command);
                    continue;
                }
            } else {
                if (chdir(workdir) < 0){
                    DBG("Cannot chdir to %s\n", workdir);
                    DBG("aborting command: \"%s\"\n", command);
                    continue;
                }
            }
            gboolean scrollup = FALSE;
            if (strncmp(command, "man", strlen("man"))==0) {
                scrollup = TRUE;
                Util::clear_text(output);
            }
            Util::print(output, g_strdup_printf("DBG> final run: %s\n",*c));
            child = Run::shell_command(output, *c, scrollup, showTextPane);
            if (withRunButton) {
              runButton = new (RunButton);
              runButton->init(runButton, *c, child, output, Util::getWorkdir(), buttonSpace);
            }
            DBG("command loop...\n");
//            if (withRunButton) newRunButton(*c, child);
        }
        g_strfreev(commands);
        g_free(ncommand); 
        return child;
    }

    static bool
    history(GtkTextView *input, guint keyval){
      switch (keyval){
        case GDK_KEY_Up:
          History::up(input);
          return true;
      case GDK_KEY_Down:
           History::down(input);
           return true;
      }
      return false;
    }
    static bool 
    pwd(GtkTextView *output, const char *text){
      if (strcmp(text, "pwd")) return false;
      auto workdir = Util::getWorkdir();
      Util::print(output, g_strdup_printf("$ %s\n", text));
      Util::print(output, g_strdup(workdir));
      Util::print(output, g_strdup("\n"));
      if (!History::add(text)) DBG("History::add(%s) failed\n", text );
      return true;
    }
    static bool
    history(GtkTextView *output, const char *text){
      if (strcmp(text, "history")) return false;
      char *t = History::history();
      Util::print(output, g_strdup_printf("$ %s\n", text));
      Util::print(output, g_strdup_printf("%s", t));
      g_free(t);
      Util::scroll_to_bottom(output);
      return true;
    }
    static bool
    cd(GtkTextView *output, const char *text){
      gchar **v = Util::getVector(text, " ");
      if (strcmp(v[0], "cd")) {
        g_strfreev(v);
        return false;
      }
      auto retval = Util::cd((const gchar **)v);
      Util::print(output, g_strdup_printf("$ %s\n", text));
      if (retval){
        Util::print(output, g_strdup_printf("%s\n", Util::getWorkdir()));
        if (!History::add(text)) DBG("History::add(%s) failed\n", text );
      } else {
        Util::print(output, g_strdup_printf(_("failed to chdir to %s"), v[1]));
      }
      g_strfreev(v);
      return true;
    }
    static bool
    exe(GtkTextView *input, GtkTextView *output, const char *text){
      if (!History::add(text)) DBG("History::add(%s) failed\n", text );
      gchar **v = Util::getVector(text, " ");
      char *inPath = g_find_program_in_path(v[0]);
      if (!inPath && g_file_test(v[0], G_FILE_TEST_IS_EXECUTABLE)) inPath = realpath(v[0], NULL);
      if (!inPath){
        Util::print(output, g_strdup_printf("$ %s\n", v[0]));
        Util::print(output, g_strdup_printf("%s: %s\n", v[0], _("Command not found.")));
        g_strfreev(v);
        return true;
      }
      Util::print(output, g_strdup_printf("$ %s", inPath));
      for (int i=1; v[i]; i++){
        Util::print(output, g_strdup_printf(" %s", v[i]));
      }
      Util::print(output, g_strdup_printf("\n"));
        
      auto buttonSpace = GTK_BOX(g_object_get_data(G_OBJECT(input), "buttonSpace"));
      
      run(output, text, true, true, buttonSpace);
      g_free(inPath);
      g_strfreev(v);
      return true;
    }
    static bool
    com(GtkTextView *input, GtkTextView *output, guint keyval){
      if (keyval != GDK_KEY_Return && keyval != GDK_KEY_KP_Enter) return false;
      auto text = Util::inputText(input);
      if (pwd(output, text)) return true;
      if (history(output, text)) return true;
      if (cd(output, text)) return true;
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
        TRACE("window_keyboard_event: keyval=%d (0x%x), keycode=%d (0x%x), modifying=%d, data= %p\n", 
            keyval, keyval, keycode, keycode, state, data);
        auto input = GTK_TEXT_VIEW(data);
        auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(data), "output"));

        if(keyval ==  GDK_KEY_Tab){
          Bash::complete(input, output);
          return TRUE;
        }

        if(keyval ==  GDK_KEY_Escape){
           gtk_widget_grab_focus(GTK_WIDGET(input));
           gtk_text_view_set_cursor_visible(input, TRUE);
           Util::flushGTK();
           return TRUE;
        }

        if (history(input, keyval)) return TRUE;
        History::reset();

        if (com(input, output, keyval)){
          Util::clear_text(input);
          return TRUE;
        }

        return FALSE;
    }
    
    GtkBox *createPrompt(void){
        auto dollarBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_hexpand(GTK_WIDGET(dollarBox), FALSE);
        GtkTextView *dollar = GTK_TEXT_VIEW(gtk_text_view_new ());
        gtk_widget_set_size_request(GTK_WIDGET(dollar), 20, -1);
        Util::print(dollar, g_strdup("$"));

        gtk_text_view_set_pixels_above_lines (dollar, 10);
        gtk_text_view_set_monospace (dollar, TRUE);
        gtk_text_view_set_editable (dollar, FALSE);
        gtk_text_view_set_cursor_visible (dollar, FALSE);
        gtk_widget_set_can_focus(GTK_WIDGET(dollar), FALSE);
        gtk_widget_add_css_class (GTK_WIDGET(dollar), "input" );
        Util::boxPack0 (dollarBox, GTK_WIDGET(dollar), FALSE, FALSE, 0);
        return dollarBox;
    }
    GtkTextView *createInput(void){
        GtkTextView *input = GTK_TEXT_VIEW(gtk_text_view_new ());
        gtk_text_view_set_pixels_above_lines (input, 10);
        gtk_text_view_set_monospace (input, TRUE);
        gtk_text_view_set_editable (input, TRUE);
        gtk_text_view_set_cursor_visible (input, TRUE);
        gtk_text_view_place_cursor_onscreen(input);
        gtk_text_view_set_wrap_mode (input, GTK_WRAP_CHAR);
        gtk_widget_set_can_focus(GTK_WIDGET(input), TRUE);
        gtk_widget_add_css_class (GTK_WIDGET(input), "input" );
        return input;
    }
    
  };
}
#endif
