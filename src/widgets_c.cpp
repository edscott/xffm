#include <string.h>

#include "window_c.hpp"
#include "widgets_c.hpp"

widgets_c::widgets_c(void *window_data, GtkNotebook *data)
{
    notebook = data;
    if (!notebook){
        g_warning("widgets_c::widgets_c(): notebook cannot be NULL\n");
        throw 1;
    }
    pathbar_p = new pathbar_c(window_data, data);
    window_c *window_p = (window_c *)window_data;
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

gtk_c *
widgets_c::get_gtk_p(void){ return gtk_p;}

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
    // pathbar is already created with pathbar_c object.
    //g_object_set_data(G_OBJECT(view_p->widgets.paper), "pathbar", pathbar);
    gtk_widget_show(pathbar_p->get_pathbar());
    gtk_box_pack_start (GTK_BOX (page_child_box), pathbar_p->get_pathbar(), FALSE, FALSE, 0);
    
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


void
widgets_c::pack(void){
    // Add widgets to page_label_box:
    gtk_box_pack_start (GTK_BOX (page_label_box), page_label_icon_box, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (page_label_box), page_label, TRUE, TRUE, 2);
//    gtk_box_pack_end (GTK_BOX (page_label_box), page_label_button_eventbox, TRUE, TRUE, 0);
//    gtk_container_add (GTK_CONTAINER (page_label_button_eventbox), page_label_button);
    gtk_box_pack_end (GTK_BOX (page_label_box), page_label_button, TRUE, TRUE, 0);
    gtk_widget_show_all (page_label_box);
    //gtk_widget_hide (page_label_button);
    // path bar... 
    // gtk_box_pack_start (GTK_BOX (page_child_box), pathbar, FALSE, FALSE, 0);
    // gtk_widget_show(pathbar);

    gtk_box_pack_start (GTK_BOX (page_child_box), vpane, TRUE, TRUE, 0);
    gtk_paned_set_position (GTK_PANED (vpane), 1000);
    gtk_widget_show (vpane);

    gtk_paned_pack1 (GTK_PANED (vpane), top_scrolled_window, FALSE, TRUE);
    gtk_paned_pack2 (GTK_PANED (vpane), bottom_scrolled_window, TRUE, TRUE);
    
    gtk_container_add (GTK_CONTAINER (top_scrolled_window), GTK_WIDGET(icon_view));
    gtk_container_add (GTK_CONTAINER (bottom_scrolled_window), diagnostics);
    gtk_widget_show (GTK_WIDGET(icon_view));
    gtk_widget_show (top_scrolled_window);
    gtk_widget_show (bottom_scrolled_window);

    gtk_widget_show (diagnostics);


    gtk_box_pack_end (GTK_BOX (button_space), size_scale, FALSE, FALSE, 0);
    gtk_widget_show (size_scale);
    gtk_box_pack_end (GTK_BOX (button_space), clear_button, FALSE, FALSE, 0);
    gtk_widget_show (clear_button);

    gtk_box_pack_start (GTK_BOX (page_child_box), button_space, FALSE, FALSE, 0);
    gtk_widget_show (button_space);

    gtk_widget_show (page_child_box);

    // Insert page into notebook:
    gint next_position = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))+1;
    gtk_notebook_insert_page (GTK_NOTEBOOK(notebook),
            page_child_box, 
            page_label_box, 
            next_position);
    gtk_notebook_set_tab_reorderable (GTK_NOTEBOOK(notebook), page_child_box, TRUE);
    gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), next_position);
}


