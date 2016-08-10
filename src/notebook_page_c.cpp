
#include "notebook_page_c.hpp"



notebook_page_c::notebook_page_c(window_c *data): widgets_c(data){
    window_p = data;
    if (!window_p->get_notebook()){
        g_warning("notebook_page_c::notebook_page_c(): notebook cannot be NULL\n");
        throw 1;
    }
    notebook = window_p->get_notebook();
    icon_view = gtk_icon_view_new();

    pack();
    signals();
#if 0
    /* drag and drop events */
    rodent_create_target_list (view_p);

    // FIXME: need to set proper treemodel...
    gtk_widget_show (iconview);
    
    // FIXME:
    rfm_hide_text(&(view_p->widgets));

    // set vpane allocation.
    // FIXME:
    rfm_layout_set_vpane_allocation(view_p);

    // FIXME:
    // rfm_view_thread_create(view_p, rfm_load_sh_command_history, (gpointer) view_p, "rfm_load_sh_command_history");
#endif
}

notebook_page_c::~notebook_page_c(void){
}

///////////////////////////// Private:

static void
clear_text_callback(GtkWidget *widget, gpointer data){
    notebook_page_c *notebook_page_p = 
        (notebook_page_c *) g_object_get_data(G_OBJECT(widget), "object");
    if (!notebook_page_p) return;
    notebook_page_p->clear_diagnostics();
}
void
notebook_page_c::clear_diagnostics(void){
    clear_text(diagnostics);
}
void
notebook_page_c::pack(void){
    // Add widgets to page_label_box:
    gtk_box_pack_start (GTK_BOX (page_label_box), page_label_icon_box, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (page_label_box), page_label, TRUE, TRUE, 0);
    gtk_box_pack_end (GTK_BOX (page_label_box), page_label_button, TRUE, TRUE, 0);
    gtk_widget_show_all (page_label_box);
    //gtk_widget_hide (page_label_button);
    // Add widgets to menu_label_box:
    gtk_box_pack_start (GTK_BOX (menu_label_box), menu_image, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (menu_label_box), menu_label, FALSE, FALSE, 0);
    gtk_widget_show_all (menu_label_box);
    // path bar... 
    // gtk_box_pack_start (GTK_BOX (page_child_box), pathbar, FALSE, FALSE, 0);
    // gtk_widget_show(pathbar);

    gtk_box_pack_start (GTK_BOX (page_child_box), vpane, TRUE, TRUE, 0);
    gtk_paned_set_position (GTK_PANED (vpane), 1000);
    gtk_widget_show (vpane);

    gtk_paned_pack1 (GTK_PANED (vpane), top_scrolled_window, FALSE, TRUE);
    gtk_paned_pack2 (GTK_PANED (vpane), bottom_scrolled_window, TRUE, TRUE);
    
    gtk_container_add (GTK_CONTAINER (top_scrolled_window), icon_view);
    gtk_container_add (GTK_CONTAINER (bottom_scrolled_window), diagnostics);
    gtk_widget_show (icon_view);
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
    gint new_page_position = 
        gtk_notebook_page_num (GTK_NOTEBOOK(notebook), window_p->get_add_child());
    gint position = (next_position <= new_page_position)? next_position:
                                                  new_page_position;
    gtk_notebook_insert_page (GTK_NOTEBOOK(notebook),
            page_child_box, 
            page_label_box, 
            position);
    gtk_notebook_set_tab_reorderable (GTK_NOTEBOOK(notebook), page_child_box, TRUE);
    gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), position);
}

void
notebook_page_c::signals(void){
    setup_callback((void *) this, clear_button, "clicked", (void *)clear_text_callback, diagnostics);
    //setup_callback((void *) this, clear_button, "button-press-event", callback, callback_data);

    /*
    g_signal_connect (page_label_button, "clicked", G_CALLBACK (rmpage), view_p);

    g_signal_connect (G_OBJECT (size_scale), 
	    "value-changed", G_CALLBACK (size_scale_callback), &(view_p->widgets));
    g_signal_connect (G_OBJECT (size_scale), "scroll-event", 
	    G_CALLBACK (scroll_event_callback2), &(view_p->widgets));
    g_signal_connect (G_OBJECT (gtk_scrolled_window_get_vadjustment (scrolled_window)),
	    "value-changed", G_CALLBACK (adjustment_changed), view_p);

    g_signal_connect (G_OBJECT (view_p->widgets.paper), 
	    "size-allocate", G_CALLBACK (signal_on_size_paper), view_p);
    g_signal_connect (G_OBJECT (view_p->widgets.paper), 
	    "configure-event", G_CALLBACK (signal_on_configure_paper), view_p);
    g_signal_connect (G_OBJECT (view_p->widgets.paper), 
	    "button-press-event", G_CALLBACK (rodent_signal_on_button_press), view_p);
    g_signal_connect (G_OBJECT (view_p->widgets.paper), 
	    "button-release-event", G_CALLBACK (rodent_signal_on_button_release), view_p);
    g_signal_connect (G_OBJECT (view_p->widgets.paper), 
	    "enter-notify-event", G_CALLBACK (signal_on_enter), view_p);
    g_signal_connect (G_OBJECT (view_p->widgets.paper), 
	    "motion-notify-event", G_CALLBACK (rodent_signal_on_motion), view_p);
    g_signal_connect (G_OBJECT (view_p->widgets.paper), 
	    "leave-notify-event", G_CALLBACK (signal_on_leave_paper), view_p);
    g_signal_connect (G_OBJECT (view_p->widgets.paper), 
	    "drag-data-received", G_CALLBACK (rodent_signal_drag_data), view_p);
    g_signal_connect (G_OBJECT (view_p->widgets.paper), 
	    "drag-data-get", G_CALLBACK (rodent_signal_drag_data_get), view_p);
    g_signal_connect (G_OBJECT (view_p->widgets.paper), 
	    "drag-motion", G_CALLBACK (rodent_signal_drag_motion), view_p);
    g_signal_connect (G_OBJECT (view_p->widgets.paper), 
	    "drag-end", G_CALLBACK (rodent_signal_drag_end), view_p);
    g_signal_connect (G_OBJECT (view_p->widgets.paper), 
	    "drag-begin", G_CALLBACK (rodent_signal_drag_begin), view_p);
    g_signal_connect (G_OBJECT (view_p->widgets.paper), 
	    "drag-leave", G_CALLBACK (rodent_signal_drag_leave), view_p);
    g_signal_connect (G_OBJECT (view_p->widgets.paper), 
	    "drag-data-delete", G_CALLBACK (rodent_signal_drag_delete), view_p);
    view_p->signal_handlers[0] = 
	g_signal_connect (G_OBJECT (window), 
		"leave-notify-event", G_CALLBACK (signal_on_leave), view_p);

    g_signal_connect (G_OBJECT (view_p->widgets.paper), 
	    "scroll-event", G_CALLBACK (scroll_event_callback), &(view_p->widgets));

*/

}


static void *
hide_text_f (void * data) {
    // to be executed by gtk thread
    GtkWidget * widget = (GtkWidget *)data;
    if(widget == NULL) return NULL;
    
    GtkWidget *vpane = (GtkWidget *)g_object_get_data(G_OBJECT(widget), "vpane");
    if(!vpane) return FALSE;
    gtk_paned_set_position (GTK_PANED (vpane), 10000);
        
    return NULL;
}

void
notebook_page_c::hide_text (GtkWidget * widget) {
    if(widget == NULL) return;
    utility_p->context_function(hide_text_f, (void *)widget);
}

static void *
clear_text_f (void *data) {
    // to be executed by gtk thread
    GtkWidget * widget = (GtkWidget *)data;
    GtkTextIter start,
      end;
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW ((widget)));
    gtk_text_buffer_get_bounds (buffer, &start, &end);
    gtk_text_buffer_delete (buffer, &start, &end);
    if (widget==NULL) {
	// This is not applicable to diagnostics_window:
	hide_text_f (widget);
    }
    g_object_ref (G_OBJECT(buffer)); 
    gtk_text_view_set_buffer(GTK_TEXT_VIEW ((widget)), gtk_text_buffer_new(NULL));
    g_object_ref_sink (G_OBJECT(buffer));
    g_object_unref (G_OBJECT(buffer)); 
    return NULL;
}

void
notebook_page_c::clear_text (GtkWidget * widget) {
    if(widget == NULL) return;
    utility_p->context_function(clear_text_f, (void *)widget);
}


