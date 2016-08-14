#include "view_c.hpp"
#include "window_c.hpp"
/////////////////////////////////////////
// simple callbacks and thread functions:
/////////////////////////////////////////

static void
clear_text(GtkWidget *widget, gpointer data){
    GtkWidget *diagnostics = GTK_WIDGET(data);

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
}

static void
hide_text(GtkWidget *widget, gpointer data){
    GtkWidget *vpane = GTK_WIDGET(data);
    gtk_paned_set_position (GTK_PANED (vpane), 10000);
    return;
}

/////////////  notebook specific: /////////////////////////
static gboolean
change_current_page (GtkNotebook *notebook,
               gint         arg1,
               gpointer     user_data)
{
    return FALSE;
}

static GtkNotebook*
create_window (GtkNotebook *notebook,
               GtkWidget   *page,
               gint         x,
               gint         y,
               gpointer     user_data)
{
    return NULL;
}

static gboolean
focus_tab (GtkNotebook   *notebook,
               GtkNotebookTab arg1,
               gpointer       user_data)
{
    return FALSE;
}

static void
move_focus_out (GtkNotebook     *notebook,
               GtkDirectionType arg1,
               gpointer         user_data)
{
}

static void
page_added (GtkNotebook *notebook,
               GtkWidget   *child,
               guint        page_num,
               gpointer     user_data)
{
}

static void
page_removed (GtkNotebook *notebook,
               GtkWidget   *child,
               guint        page_num,
               gpointer     data)
{
    view_c *view_p = (view_c *)data;
    gint add_page = gtk_notebook_page_num (notebook, view_p->get_page_child_box());
    gint current_page= gtk_notebook_get_current_page (notebook);
    fprintf(stderr, "removed=%d, current=%d, addpage=%d\n", page_num, current_page, add_page);
    if (current_page == add_page && current_page) {
  //      gtk_notebook_set_current_page (notebook, current_page-1);
    }
    // Must we destroy widget???
    // gtk_widget_destroy(child);
    //
    // Clean up class object, this is done by removing from view list.
    window_c *window_p = (window_c *)view_p->get_window_p();
    if (window_p){
        window_p->remove_view_from_list((void *)window_p);
    } else {
        fprintf(stderr, 
            "*** memory leak: window_p was not added to view_p list in window_c object\n");
    }
    // view_p = associated view_p
    // delete view_p 
}

static void
page_reordered (GtkNotebook *notebook,
               GtkWidget   *child,
               guint        page_num,
               gpointer     user_data)
{
}

static gboolean
reorder_tab (GtkNotebook     *notebook,
               GtkDirectionType arg1,
               gboolean         arg2,
               gpointer         user_data)
{
    return FALSE;
}

static gboolean
select_page (GtkNotebook *notebook,
               gboolean     arg1,
               gpointer     user_data)
{
    return FALSE;
}

static void
switch_page (GtkNotebook *notebook,
               GtkWidget   *page,
               guint        page_num,
               gpointer     user_data)
{
    fprintf(stderr, "switch_page\n");
    gint current_page = gtk_notebook_get_current_page (notebook) + 1;
    gint n_pages = gtk_notebook_get_n_pages (notebook);
    if (n_pages < 2) return;
    if (current_page == n_pages){
        fprintf(stderr, "moving down\n");
        // runaway loop with this...
        //gtk_notebook_set_current_page (notebook, n_pages-2);

    }
}

////////////////////////////////////////
// class function definitions
////////////////////////////////////////


// Public:
view_c::view_c(void *window_v, GtkWidget *notebook, GtkWidget *data) : widgets_c(window_v, notebook){
    new_tab_child = data;
    window_p = window_v; 
    g_object_set_data(G_OBJECT(notebook), "window_p", window_p);
    g_object_set_data(G_OBJECT(notebook), "view_p", (void *)this);
    g_object_set_data(G_OBJECT(page_label_button), "view_p", (void *)this);
    signals_p = new signals_c();
    init();
    icon_view = gtk_icon_view_new();
    gtk_icon_view_set_item_width (GTK_ICON_VIEW (icon_view), 60);
    signals();
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


void *
view_c::get_window_p(void){return window_p;}

// FIXME: enum is repeated in xfdir_c.cpp
enum
{
  COL_DISPLAY_NAME,
  COL_PIXBUF,
  NUM_COLS
};
void
view_c::set_treemodel(GtkTreeModel *data){
    if (tree_model) gtk_widget_hide(GTK_WIDGET(icon_view));
    GtkTreeModel *old_model=tree_model;
    tree_model = (GtkTreeModel *)data;
    gtk_icon_view_set_model(GTK_ICON_VIEW(icon_view), tree_model);
    gtk_icon_view_set_text_column (GTK_ICON_VIEW (icon_view),
                                 COL_DISPLAY_NAME);
    gtk_icon_view_set_pixbuf_column (GTK_ICON_VIEW (icon_view), COL_PIXBUF);
    gtk_icon_view_set_selection_mode (GTK_ICON_VIEW (icon_view),
                                    GTK_SELECTION_MULTIPLE);
   
    gtk_widget_show(GTK_WIDGET(icon_view));
    // FIXME: clean up old model now (in thread)
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
    fprintf(stderr, "DBG: clear diagnostics()\n");
    clear_text(NULL, (void *)diagnostics);
    hide_text(NULL, (void *)vpane);
}
void
view_c::pack(void){
    // Add widgets to page_label_box:
    gtk_box_pack_start (GTK_BOX (page_label_box), page_label_icon_box, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (page_label_box), page_label, TRUE, TRUE, 0);
//    gtk_box_pack_end (GTK_BOX (page_label_box), page_label_button_eventbox, TRUE, TRUE, 0);
//    gtk_container_add (GTK_CONTAINER (page_label_button_eventbox), page_label_button);
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

GtkWidget *
view_c::get_page_child_box(void){return page_child_box;}

void
view_c::signals(void){
    // complex connections:
    //signals_p->setup_callback((void *) this, clear_button, "clicked", 
      //      (void *)clear_text_callback, diagnostics);
    // Simple connections:
    // clear button:
    g_signal_connect (clear_button, "clicked", 
            G_CALLBACK (clear_text), (void *)diagnostics);
    g_signal_connect (clear_button, "clicked", 
            G_CALLBACK (hide_text), (void *)vpane);
    // notebook specific:
    g_signal_connect (notebook, "change-current-page", 
            G_CALLBACK (change_current_page), (void *)NULL);
    g_signal_connect (notebook, "create-window", 
            G_CALLBACK (create_window), (void *)NULL);
    g_signal_connect (notebook, "focus-tab", 
            G_CALLBACK (focus_tab), (void *)NULL);
    g_signal_connect (notebook, "move-focus-out", 
            G_CALLBACK (move_focus_out), (void *)NULL);
    g_signal_connect (notebook, "page-added", 
            G_CALLBACK (page_added), (void *)NULL);
    g_signal_connect (notebook, "page-removed", 
            G_CALLBACK (page_removed), (void *)this);
    g_signal_connect (notebook, "page-reordered", 
            G_CALLBACK (page_reordered), (void *)NULL);
    g_signal_connect (notebook, "reorder-tab", 
            G_CALLBACK (reorder_tab), (void *)NULL);
    g_signal_connect (notebook, "select-page", 
            G_CALLBACK (select_page), (void *)NULL);
    g_signal_connect (notebook, "switch-page", 
            G_CALLBACK (switch_page), (void *)NULL);


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

