#include <string.h>

#include "window_c.hpp"
#include "widgets_c.hpp"

widgets_c::widgets_c(void *window_v, GtkWidget *data){
    notebook = data;
    if (!notebook){
        g_warning("widgets_c::widgets_c(): notebook cannot be NULL\n");
        throw 1;
    }
    window_c *window_p = (window_c *)window_v;
    gtk_p = window_p->get_gtk_p();
    create();
    setup_diagnostics();
    setup_scolled_windows();
    setup_size_scale();
    gtk_p->setup_image_button(clear_button, "edit-clear-all",  _("Clear"));
    gtk_p->setup_image_button(page_label_button, "window-close", _("Close Tab"));
}
widgets_c::~widgets_c(void){
}

GtkWidget *widgets_c::get_page_label_button(void){ return page_label_button;}
GtkWidget *widgets_c::get_page_child_box(void){ return page_child_box;}
GtkWidget *widgets_c::get_vpane(void){ return vpane;}
GtkWidget *widgets_c::get_diagnostics(void){ return diagnostics;}

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
    //page_label_button_eventbox = gtk_event_box_new ();
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

