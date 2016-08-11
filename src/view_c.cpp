#include "view_c.hpp"
// callbacks
static void clear_text_callback(GtkWidget *, gpointer);
// thread functions
static void *hide_text_f(void *);
static void *clear_text_f(void *);



// Public:
view_c::view_c(GtkWidget *notebook, GtkWidget *data) : widgets_c(notebook){
    new_tab_child = data;
    g_object_set_data(G_OBJECT(page_label_button), "view_p", (void *)this);
    signals_p = new signals_c();
    init();
    icon_view = gtk_icon_view_new();

    pack();
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


view_c::~view_c(void){
    pthread_mutex_destroy(&population_mutex);
    pthread_cond_destroy(&population_cond);
    pthread_rwlock_destroy(&population_lock);
    delete signals_p;
}


///////////////////////////// Private:
void
view_c::init(void){
    gint result;
    population_mutex = PTHREAD_MUTEX_INITIALIZER;
    population_cond = PTHREAD_COND_INITIALIZER;
    result = pthread_rwlock_init(&population_lock, NULL);

    if (result){
        cerr << "view_c::init(): " << strerror(result) << "\n";
        throw 1;
    }
    population_condition = 0;



}

void
view_c::clear_diagnostics(void){
    clear_text(diagnostics);
    hide_text(diagnostics);
}
void
view_c::pack(void){
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
        gtk_notebook_page_num (GTK_NOTEBOOK(notebook), new_tab_child);
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
view_c::hide_text (GtkWidget * widget) {
    utility_p->context_function(hide_text_f, (void *)widget);
}

void
view_c::clear_text (GtkWidget * widget) {
    utility_p->context_function(clear_text_f, (void *)widget);
}

void
view_c::signals(void){
    signals_p->setup_callback((void *) this, clear_button, "clicked", 
            (void *)clear_text_callback, diagnostics);
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

///////////////////////////////////////////////////////////////////
//    callbacks...
///////////////////////////////////////////////////////////////////


static void
clear_text_callback(GtkWidget *widget, gpointer data){
    view_c *view_p = (view_c *) g_object_get_data(G_OBJECT(widget), "object");
    if (!view_p) return;
    view_p->clear_diagnostics();
}

static void *
hide_text_f (void * data) {
    // to be executed by gtk thread
    if(data == NULL) return NULL;
    view_c *view_p = (view_c *)data;
    GtkWidget *vpane = view_p->get_vpane();
    gtk_paned_set_position (GTK_PANED (vpane), 10000);
    return NULL;
}

static void *
clear_text_f (void *data) {
    // to be executed by gtk thread
    if(data == NULL) return NULL;
    view_c *view_p = (view_c *)data;
    GtkWidget *diagnostics = view_p->get_diagnostics();
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW ((diagnostics)));

#if 0
    g_object_ref (G_OBJECT(buffer)); 
    gtk_text_view_set_buffer(GTK_TEXT_VIEW ((diagnostics)), gtk_text_buffer_new(NULL));
    // XXX or gtk_widget_destroy?
    g_object_ref_sink (G_OBJECT(buffer));
    g_object_unref (G_OBJECT(buffer)); 
#else
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds (buffer, &start, &end);
    gtk_text_buffer_delete (buffer, &start, &end);
#endif
    return NULL;
}

