#include <string.h>

#include "widgets_c.hpp"

widgets_c::widgets_c(void){
    create();
    setup_diagnostics();
    setup_scolled_windows();
    setup_size_scale();
    setup_clear_button();

}
void 
widgets_c::add_custom_tooltip(GtkWidget *widget, GtkWidget *image, const gchar *tooltip_text){
    //FIXME
}

void
widgets_c::setup_clear_button (void){
    gtk_widget_set_can_focus (clear_button, FALSE);
    gtk_button_set_relief (GTK_BUTTON (clear_button), GTK_RELIEF_NONE);
    GtkWidget *image = gtk_image_new_from_icon_name ("edit-clear-all", 
            GTK_ICON_SIZE_SMALL_TOOLBAR);
    if (image) {
        gtk_container_add (GTK_CONTAINER (clear_button), image);
        gtk_widget_show (image);
    }
    add_custom_tooltip(clear_button, image, _("Clear"));
}  

void
widgets_c::setup_diagnostics(void){
    gtk_widget_set_can_focus(diagnostics, FALSE);
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (diagnostics), GTK_WRAP_WORD);
    gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (diagnostics), FALSE);
    gtk_container_set_border_width (GTK_CONTAINER (diagnostics), 2);
    gint size = 10;
    GtkStyleContext *style_context = gtk_widget_get_style_context (diagnostics);
    gtk_style_context_add_class(style_context, GTK_STYLE_CLASS_VIEW );
    GtkCssProvider *css_provider = gtk_css_provider_new();
    GError *error=NULL;
    gchar *data = g_strdup_printf("* {\
font-family: %s;\
font-size: %dpx;\
}", "monospace", size);
    gtk_css_provider_load_from_data (css_provider, data, -1, &error);
    g_free(data);
    if (error){
        fprintf(stderr, "gerror: %s\n", error->message);
        g_error_free(error);
    }
    gtk_style_context_add_provider (style_context, GTK_STYLE_PROVIDER(css_provider),
                                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

void 
widgets_c::setup_scolled_windows(void){
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (top_scrolled_window),
            GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (top_scrolled_window),
                                       GTK_SHADOW_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (bottom_scrolled_window),
            GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (bottom_scrolled_window),
                                       GTK_SHADOW_IN);
}

void
widgets_c::setup_size_scale(void){
    // Add image to close tab button: page_label_button
    // XXX : 
    //      1.we need pixbuf class
    //      2.we need callback procedure
    /*
    GdkPixbuf *pb = rfm_get_pixbuf ("xffm/stock_close", 8);
    GtkWidget *image = gtk_image_new_from_pixbuf (pb);
    g_object_unref(pb);
    gtk_widget_show (image);
    gtk_container_add (GTK_CONTAINER (page_label_button), image);
    g_object_set (page_label_button, "image", image, "relief", GTK_RELIEF_NONE, NULL);
    g_signal_connect (page_label_button, "clicked", G_CALLBACK (rmpage), view_p);
    */


    gtk_scale_set_draw_value (GTK_SCALE(size_scale), FALSE);
    gtk_widget_set_can_focus (size_scale, FALSE);
    gtk_widget_set_size_request (size_scale, 75, 30);

    
}


void
widgets_c::create(void){
    page_child_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    page_label_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    page_label_icon_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    page_label = gtk_label_new (_("Loading folder..."));
    page_label_button = gtk_button_new ();
    menu_label_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    menu_label = gtk_label_new ("menu_label");
    menu_image = gtk_image_new ();
    // pathbar = rodent_new_pathbar();
    vpane = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    top_scrolled_window = gtk_scrolled_window_new (NULL, NULL);
     
    bottom_scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    diagnostics = gtk_text_view_new ();
    g_object_set_data(G_OBJECT(diagnostics), "vpane", vpane);
    status = gtk_text_view_new ();
    rename = NULL; // create on demand...

    button_space = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    clear_button =  gtk_button_new ();
    size_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.0, 96.0, 12.0);
    
}

