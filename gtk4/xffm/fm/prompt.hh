#ifndef XF_PROMPT_HH
#define XF_PROMPT_HH
namespace xf {
  class Prompt {

    public:
    GtkBox *promptBox(void){ return promptBox_;}
    GtkTextView *input(void){ return input_;}
    private:
    GtkBox *promptBox_;
    GtkTextView *input_;
    
    GtkButton *promptButton_;
    GtkButton *clearButton_;
    GtkScale *sizeScale_;

    public:

    Prompt(void) {
        promptBox_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_hexpand(GTK_WIDGET(promptBox_), TRUE);
        input_ = createInput(); 
        Util::print(input_,g_strdup("$ "));
        auto keyController = gtk_event_controller_key_new();
        gtk_widget_add_controller(GTK_WIDGET(input_), keyController);
        g_signal_connect (G_OBJECT (keyController), "key-pressed", 
            G_CALLBACK (this->on_keypress), (void *)input_);
        Util::boxPack0 (promptBox_, GTK_WIDGET(input_), TRUE, TRUE, 0);
    }
    private:
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
    com(GtkTextView *input, GtkTextView *output, guint keyval){
      if (keyval != GDK_KEY_Return && keyval != GDK_KEY_KP_Enter) return false;
      auto text = Util::inputText(input);
      if (pwd(output, text)) return true;
      if (history(output, text)) return true;
      if (cd(output, text)) return true;
      g_free(text);
      return false;
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
          BashCompletion::bash_completion(input, output, Util::getWorkdir());
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
          Util::print(input, g_strdup("$ "));
          return TRUE;
        }

        return FALSE;
    }
    
    GtkTextView *createPrompt(void){
        GtkTextView *input = GTK_TEXT_VIEW(gtk_text_view_new ());
        gtk_text_view_set_pixels_above_lines (input, 10);
        gtk_text_view_set_monospace (input, TRUE);
        gtk_text_view_set_editable (input, FALSE);
        gtk_text_view_set_cursor_visible (input, FALSE);
        //gtk_text_view_place_cursor_onscreen(input);
        //gtk_text_view_set_wrap_mode (input, GTK_WRAP_CHAR);
        gtk_widget_set_can_focus(GTK_WIDGET(input), FALSE);
        gtk_widget_add_css_class (GTK_WIDGET(input), "input" );

        return input;
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
