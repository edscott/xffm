#ifndef XF_PROMPT_HH
#define XF_PROMPT_HH
namespace xf {
  class Prompt {
    private:
    GtkBox *promptBox_;
    GtkButton *promptButton_;
    GtkTextView *input_;
    GtkButton *clearButton_;
    GtkScale *sizeScale_;
    
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
    static 
    gboolean on_keypress(GtkEventControllerKey* self,
          guint keyval,
          guint keycode,
          GdkModifierType state,
          gpointer data)
    {
        auto input = GTK_TEXT_VIEW(data);
        gint gotcha[]={
          GDK_KEY_Return,
          GDK_KEY_KP_Enter,
          0
        };
        for (gint i=0; gotcha[i]; i++) {
            if(keyval ==  gotcha[i]) {
              auto buffer = gtk_text_view_get_buffer(input);
              GtkTextIter  start, end;
              gtk_text_buffer_get_bounds (buffer, &start, &end);
              auto text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
              Util::clear_text(input);
              Util::print(input, g_strdup("$ "));
              auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(input), "output"));
              if (strchr(text, '$')) *strchr(text, '$')=' ';
              g_strstrip(text);
              Util::print(output, g_strdup("$ "));
              Util::print(output, text);
              Util::print(output, g_strdup("\n"));

              DBG("window_keyboard_event: key gotcha\n");
                // get cursor position.
                return TRUE;
            }
        }
        return FALSE;
    }

    public:
    GtkBox *promptBox(void){ return promptBox_;}
    GtkTextView *input(void){ return input_;}
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
  };
}
#endif
