#include "view_c.hpp"
// Public:
view_c::view_c(void){
    init();
    
}


view_c::~view_c(void){
    pthread_mutex_destroy(population_mutex);
    g_free(population_mutex);
    pthread_cond_destroy(population_cond);
    g_free(population_cond);
    pthread_rwlock_destroy(population_lock);
    g_freepopulation_lock();
}


// Private:
void
view_c::init(void){
    gint result[3];
    population_mutex = (pthread_mutex_t *)calloc(1, sizeof(pthread_mutex_t));
    population_cond = (pthread_cond_t *)calloc(1, sizeof(pthread_cond_t));
    population_lock = (pthread_rwlock_t *)calloc(1, sizeof(pthread_rwlock_t));
    if (!population_mutex || !population_cond || !population_lock) {
        cerr << "view_c::init(): calloc failed\n";
        if (!population_mutex) throw 1;
        if (!population_cond) throw 2;
        if (!population_lock) throw 3;
    }
    result[0] = pthread_mutex_init(population_mutex, NULL);
    result[1] = pthread_cond_init(population_cond, NULL);
    result[2] = pthread_rwlock_init(population_lock, NULL);


    if (result [0] | result [1] | result[2]){
        gint r = result[0]? result[0]: result[1]?result[1]:result[2];
        cerr << "view_c::init(): " << strerror(r) << "\n";
        if (result [0]) throw 4;
        if (result [1]) throw 5;
        if (result[2]) throw 6;
    }

    population_condition = 0;



}

void
notebook_page_c::create(void){
    page_child_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    page_label_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    page_label_icon_box = gtk_box_new (FALSE, 0);
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
    status = gtk_text_view_new ();
    rename = NULL; // create on demand...
    button_space = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    // FIXME
    clear_button = rfm_mk_little_button (
	    "xffm/stock_clear", 
	    (void *) rfm_clear_text_window, 
	    (void *) (&(view_p->widgets)), _("Clear"));

    size_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.0, 96.0, 12.0);

    
}

void
notebook_page_c::setup(void){
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
    // XXX: with iconview, top_scrolled window may not be needed...
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (top_scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (bottom_scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

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

    gtk_scale_set_draw_value (GTK_SCALE(size_scale), FALSE);
    gtk_widget_set_can_focus (size_scale, FALSE);
    gtk_widget_set_size_request (size_scale, 75, 30);

    
}

void
notebook_page_c::show(void){
    // Add widgets to page_label_box:
    gtk_box_pack_start (GTK_BOX (page_label_box), page_label_icon_box, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (page_label_box), page_label, TRUE, TRUE, 0);
    gtk_box_pack_end (GTK_BOX (page_label_box), page_label_button, TRUE, TRUE, 0);
    gtk_widget_show_all (page_label_box);
    gtk_widget_hide (page_label_button);
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
    gtk_widget_show (top_scrolled_window);
    gtk_container_add (GTK_CONTAINER (bottom_scrolled_window), diagnostics);
    gtk_widget_show (bottom_scrolled_window);
    gtk_widget_show (diagnostics);


    gtk_box_pack_start (GTK_BOX (page_child_box), button_space, FALSE, FALSE, 0);
    gtk_box_pack_end (GTK_BOX (button_space), size_scale, FALSE, FALSE, 0);
    gtk_widget_show (size_scale);
    gtk_widget_show (button_space);

    // Insert page into notebook:
    gtk_widget_show (page_child_box);
    gint position=gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))+1;
    gtk_notebook_insert_page_menu (GTK_NOTEBOOK(notebook), page_child_box, page_label_box, menu_label_box, position);
    gtk_notebook_set_tab_reorderable (GTK_NOTEBOOK(notebook), page_child_box, TRUE);
    gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), position);

    gtk_box_pack_end (GTK_BOX (button_space), clear_button, FALSE, FALSE, 0);
    gtk_widget_show (clear_button);

    gtk_widget_show (iconview);
    

}

void
notebook_page_c::signals(void){
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



}


void 
notebook_page_c::notebook_page_c(GtkWidget *notebook){
    if (!notebook){
        g_warning("notebook_page_c::notebook_page_c(): notebook cannot be NULL\n");
        throw 1;
    }

    create();
    setup();
    show();
    signals();

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
    



}
