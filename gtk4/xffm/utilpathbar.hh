#ifndef UTILPATHBAR_HH
#define UTILPATHBAR_HH

namespace xf {
  class UtilPathbar  :  public  UtilBasic{
    public:
    static
    GtkTextView *createInput(void){
        GtkTextView *input = GTK_TEXT_VIEW(gtk_text_view_new ());
        char *size = Settings::getString("xfterm", "size");
        if (!size) size = g_strdup("font4"); // medium
        gtk_widget_add_css_class (GTK_WIDGET(input), size );
        
        gtk_text_view_set_pixels_above_lines (input, 5);
        gtk_text_view_set_pixels_below_lines (input, 5);
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

