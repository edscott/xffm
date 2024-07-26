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
    
    GtkTextView *createInput(void){
        GtkTextView *input = GTK_TEXT_VIEW(gtk_text_view_new ());
        gtk_text_view_set_pixels_above_lines (input, 10);
        gtk_text_view_set_monospace (input, TRUE);
        gtk_text_view_set_editable (input, TRUE);
        gtk_text_view_set_cursor_visible (input, TRUE);
        gtk_text_view_place_cursor_onscreen(input);
        gtk_text_view_set_wrap_mode (input, GTK_WRAP_CHAR);
        gtk_widget_set_can_focus(GTK_WIDGET(input), TRUE);
        return input;
    }
#define DEFAULT_FIXED_FONT_SIZE 13
    GtkScale *newSizeScale(const gchar *tooltipText){
        auto size_scale = GTK_SCALE(gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 6.0, 24.0, 6.0));
        // Load saved value fron xffm+/settings.ini file (if any)
        //gint size = Settings<Type>::getInteger("xfterm", "fontSize");//FIXME
        gint size = -1;
        if (size < 0) size = DEFAULT_FIXED_FONT_SIZE;
        gtk_range_set_value(GTK_RANGE(size_scale), size);
        gtk_range_set_increments (GTK_RANGE(size_scale), 2.0, 6.0);
        gtk_widget_set_size_request (GTK_WIDGET(size_scale),75,-1);
        gtk_scale_set_value_pos (size_scale,GTK_POS_RIGHT);
        gtk_adjustment_set_upper (gtk_range_get_adjustment(GTK_RANGE(size_scale)), 24.0);
        gtk_widget_set_tooltip_markup (GTK_WIDGET(size_scale),tooltipText);        
        return size_scale;
    }

    public:
    GtkBox *promptBox(void){ return promptBox_;}
    Prompt(void) {
        promptBox_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_hexpand(GTK_WIDGET(promptBox_), TRUE);
        clearButton_ =  Util::newButton(EDIT_CLEAR, _("Clear Log"));
        input_ = createInput(); 
        sizeScale_ = newSizeScale(_("Terminal font"));
        promptButton_ = Util::newButton("media-view-subtitles", _("Show/hide grid."));
        
        Util::boxPack0 (promptBox_, GTK_WIDGET(promptButton_), FALSE, FALSE, 0);

        auto label = gtk_label_new("<b>$</b>foobar");
        Util::boxPack0 (promptBox_, GTK_WIDGET(label), TRUE, TRUE, 0);
        Util::boxPack0 (promptBox_, GTK_WIDGET(input_), TRUE, TRUE, 0);

        Util::packEnd (promptBox_, GTK_WIDGET(sizeScale_));
        Util::packEnd (promptBox_, GTK_WIDGET(clearButton_));
    }
  };
}
#endif
