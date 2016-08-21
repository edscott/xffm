#include "view_c.hpp"
#include "window_c.hpp"


static gboolean unhighlight (gpointer key, gpointer value, gpointer data);

/////////////////////////////////////////
// simple callbacks and thread functions:
/////////////////////////////////////////
gboolean
motion_notify_event (GtkWidget *widget,
               GdkEvent  *event,
               gpointer   data){
    view_c * view_p = (view_c *)data;
    if (view_p->get_dir_count() > 500) return FALSE;
    if (!data) g_error("motion_notify_event: data cannot be NULL\n");
    GdkEventMotion *e = (GdkEventMotion *)event;
    view_p->set_highlight(e->x, e->y);
    view_p->highlight();
    return FALSE;
}
gboolean
leave_notify_event (GtkWidget *widget,
               GdkEvent  *event,
               gpointer   data){
    view_c * view_p = (view_c *)data;
    if (!data) g_error("leave_notify_event: data cannot be NULL\n");
    //fprintf(stderr, "leave_notify_event\n");
    view_p->clear_highlights(NULL);
}


static void 
on_remove_page_button(GtkWidget *page_label_button, gpointer data){
    view_c *view_p = (view_c *)data;
    window_c *window_p = (window_c *)view_p->get_window_p();
    view_p->remove_page();       
    // delete object:
    window_p->remove_view_from_list((void *)view_p);
    
}


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

////////////// iconview specific signal bindings: /////////////////////////
static void
item_activated (GtkIconView *iconview,
                GtkTreePath *path,
                gpointer     data)
{
    view_c *view_p = (view_c *)data;
    GtkTreeModel *tree_model = gtk_icon_view_get_model (iconview);
    GtkTreeIter iter;
    if (!gtk_tree_model_get_iter (tree_model, &iter, path)) return;

    GValue value = G_VALUE_INIT;
    gtk_tree_model_get_value (tree_model, &iter,
                          COL_ACTUAL_NAME,
                          &value);
    gchar *dname = g_strdup_value_contents (&value);
    // hack to unquote...
    gchar *p;
    for (p=dname; p && *p; p++){
        if (*p == '"') *p = ' ';
    } g_strstrip(dname);    

    gchar *dir = g_get_current_dir();

    gchar *full_path = g_strconcat(dir, G_DIR_SEPARATOR_S, dname, NULL);
    fprintf(stderr, "dname = %s, path = %s\n", dname, full_path);

    g_value_unset(&value);

    if (g_file_test(full_path, G_FILE_TEST_IS_DIR)){
        view_p->reload(full_path);
    }
    g_free(dname);
    g_free(full_path);



}


/////////////  notebook specific signal bindings: /////////////////////////
    
static gboolean
change_current_page (GtkNotebook *notebook,
               gint         arg1,
               gpointer     data)
{
    fprintf(stderr, "change_current_page\n");
    return FALSE;
}

static GtkNotebook*
create_window (GtkNotebook *notebook,
               GtkWidget   *page,
               gint         x,
               gint         y,
               gpointer     data)
{
    fprintf(stderr, "create_window\n");
    return NULL;
}

static gboolean
focus_tab (GtkNotebook   *notebook,
               GtkNotebookTab arg1,
               gpointer       data)
{
    fprintf(stderr, "focus_tab\n");
    return FALSE;
}

static void
move_focus_out (GtkNotebook     *notebook,
               GtkDirectionType arg1,
               gpointer         data)
{
    fprintf(stderr, "move_focus_out\n");
}

static void
page_added (GtkNotebook *notebook,
               GtkWidget   *child,
               guint        page_num,
               gpointer     data)
{
    fprintf(stderr, "page_added\n");
}

static void
page_removed (GtkNotebook *notebook,
               GtkWidget   *child,
               guint        page_num,
               gpointer     data)
{
    fprintf(stderr, "page_removed\n");
}

static void
page_reordered (GtkNotebook *notebook,
               GtkWidget   *child,
               guint        page_num,
               gpointer     data)
{
    fprintf(stderr, "page_reordered\n");
}

static gboolean
reorder_tab (GtkNotebook     *notebook,
               GtkDirectionType arg1,
               gboolean         arg2,
               gpointer         data)
{
    fprintf(stderr, "reorder_tab\n");
    return FALSE;
}

static gboolean
select_page (GtkNotebook *notebook,
               gboolean     arg1,
               gpointer     data)
{
    fprintf(stderr, "select_page\n");
    return FALSE;
}

static void
switch_page (GtkNotebook *notebook,
               GtkWidget   *page,
               guint        page_num,
               gpointer     data)
{
    fprintf(stderr, "switch_page, page_num=%d\n" ,page_num);
    //page_status(data);
    view_c *view_p = (view_c *)data;
    window_c *window_p = (window_c *)view_p->get_window_p();
    gint current_page = gtk_notebook_get_current_page (notebook);
    fprintf(stderr, "   current=%d,  pagecount=%d\n",
            current_page, gtk_notebook_get_n_pages(notebook));

}

////////////////////////////////////////
// class function definitions
////////////////////////////////////////


// Public:
view_c::view_c(void *window_v, GtkNotebook *notebook) : widgets_c(window_v, notebook){
    window_p = window_v; 
    xfdir_p = NULL;
    dirty_hash = FALSE;
    highlight_hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    g_object_set_data(G_OBJECT(notebook), "window_p", window_p);
    g_object_set_data(G_OBJECT(notebook), "view_p", (void *)this);
    g_object_set_data(G_OBJECT(page_label_button), "view_p", (void *)this);
    //signals_p = new signals_c();
    init();
    icon_view = GTK_ICON_VIEW(gtk_icon_view_new());
    gtk_icon_view_set_item_width (icon_view, 60);
    gtk_icon_view_set_activate_on_single_click(icon_view, TRUE);
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
    if (xfdir_p) delete xfdir_p;
    pthread_mutex_destroy(&population_mutex);
    pthread_cond_destroy(&population_cond);
    pthread_rwlock_destroy(&population_lock);
}

void
view_c::set_page_label(void){
    gchar *tab_label = g_path_get_basename(xfdir_p->get_label());
    gtk_label_set_markup(GTK_LABEL(page_label), tab_label);
    g_free(tab_label);
}

gint 
view_c::get_dir_count(void){ return xfdir_p->get_dir_count();}
void
view_c::reload(const gchar *data){
    // clear highlight hash
    clear_highlights(NULL);
    xfdir_p->reload(data);
    set_view_details();
    while (gtk_events_pending()) gtk_main_iteration();
    if (get_dir_count() <= 500) highlight();
}

void
view_c::remove_page(void){
    gint page_num = gtk_notebook_page_num (notebook, page_child_box);
    gint current_page = gtk_notebook_get_current_page (notebook);

    gtk_notebook_remove_page (notebook, page_num);
    if (current_page == page_num) {
        gtk_notebook_set_current_page (notebook, page_num-1);
    }    
}

xfdir_c *
view_c::get_xfdir_p(void) {return xfdir_p;}

void *
view_c::get_window_p(void){return window_p;}


void
view_c::set_treemodel(xfdir_c *data){
    xfdir_c *old_xfdir_p = xfdir_p;
    xfdir_p = data;
    GtkTreeModel *tree_model = xfdir_p->get_tree_model();
    if (tree_model) gtk_widget_hide(GTK_WIDGET(icon_view));
    gtk_icon_view_set_model(icon_view, tree_model);
    gtk_icon_view_set_text_column (icon_view, COL_DISPLAY_NAME);
    gtk_icon_view_set_pixbuf_column (icon_view, COL_PIXBUF);
    gtk_icon_view_set_selection_mode (icon_view, GTK_SELECTION_MULTIPLE);
    set_view_details();
    gtk_widget_show(GTK_WIDGET(icon_view));
    if (old_xfdir_p) delete old_xfdir_p;
}
///////////////////////////// Private:
void
view_c::set_view_details(void){
    set_page_label();
    set_window_title();
    set_application_icon();

}

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

static gboolean
unhighlight (gpointer key, gpointer value, gpointer data){
    void **arg = (void **)data;
    xfdir_c *xfdir_p = (xfdir_c *)arg[0];
    gchar *tree_path_string = (gchar *)arg[1];
    if (tree_path_string && strcmp(tree_path_string, (gchar *)key)==0) return FALSE;
    fprintf(stderr, "unhighlight %s\n", (gchar *)key);
    GtkTreeModel *model = xfdir_p->get_tree_model();
    GtkTreeIter iter;
    gchar *icon_name;
            
    GtkTreePath *tpath = gtk_tree_path_new_from_string ((gchar *)key);
    if (tpath) {
        gtk_tree_model_get_iter (model, &iter, tpath);
        gtk_tree_model_get (model, &iter, COL_ICON_NAME, &icon_name, -1);
        gchar *name;
        gtk_tree_model_get (model, &iter, COL_ACTUAL_NAME, &name, -1);
        gint icon_size = xfdir_p->get_icon_size(name);
        g_free(name);
        gtk_list_store_set (GTK_LIST_STORE(model), &iter,
            COL_PIXBUF, xfdir_p->get_pixbuf(icon_name, icon_size ), 
            -1);
        g_free(icon_name);
        gtk_tree_path_free (tpath);
    }

    return TRUE;
}

void 
view_c::clear_highlights(const gchar *tree_path_string){
    void *arg[]={(void *)xfdir_p, (void *)tree_path_string};
    g_hash_table_foreach_remove (highlight_hash, unhighlight, (void *)arg);
    dirty_hash = (tree_path_string != NULL)? TRUE: FALSE;
}

void 
view_c::set_highlight(gdouble X, gdouble Y){
    //    fprintf(stderr, "x=%lf y=%lf\n", X, Y);
    highlight_x = X + 0.5;
    highlight_y = Y + 0.5;
}

void 
view_c::highlight(void){
    //fprintf(stderr, "highlight %d, %d\n", highlight_x, highlight_y);
    gchar *tree_path_string = NULL;
    
    GtkCellRenderer *cell;
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_icon_view_get_model(icon_view);
    
    GtkTreePath *tpath = gtk_icon_view_get_path_at_pos (icon_view, highlight_x, highlight_y); 

    if (tpath){
        tree_path_string = gtk_tree_path_to_string (tpath);
        if (g_hash_table_lookup(highlight_hash, tree_path_string)) {
            //fprintf(stderr, "%s already in hash\n", tree_path_string);
            g_free (tree_path_string);
            gtk_tree_path_free (tpath);
            return;
        }
        // Not highlighted?
        fprintf(stderr, "yes: %s\n", tree_path_string);
        
        g_hash_table_insert(highlight_hash, tree_path_string, GINT_TO_POINTER(1));
        // Do highlight.
        gtk_tree_model_get_iter (model, &iter, tpath);
        gchar *name;
        gtk_tree_model_get (model, &iter, COL_ACTUAL_NAME, &name, -1);
        const gchar *icon_name = (strcmp(name, ".."))?"document-open":"go-up";
	g_free(name);

	gtk_list_store_set (GTK_LIST_STORE(model), &iter,
                COL_PIXBUF, xfdir_p->get_pixbuf(icon_name, GTK_ICON_SIZE_DIALOG ), 
		-1);
        gtk_tree_path_free (tpath);
        clear_highlights(tree_path_string);
    } else {
        // No item at position?
        // Do we need to clear hash table?
        if (dirty_hash){
            fprintf(stderr, "no (%d, %d)\n", highlight_x,highlight_y);
            clear_highlights(NULL);
        }
    }
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
    g_signal_connect (page_label_button, "clicked", 
            G_CALLBACK (on_remove_page_button), (void *)this);
    // iconview specific signal bindings:
    g_signal_connect (icon_view, "item-activated", 
            G_CALLBACK (item_activated), (void *)this);
    g_signal_connect (icon_view, "motion-notify-event", 
            G_CALLBACK (motion_notify_event), (void *)this);
    g_signal_connect (icon_view, "leave-notify-event", 
            G_CALLBACK (leave_notify_event), (void *)this);



    // notebook specific signal bindings:
    g_signal_connect (notebook, "change-current-page", 
            G_CALLBACK (change_current_page), (void *)this);
    g_signal_connect (notebook, "create-window", 
            G_CALLBACK (create_window), (void *)this);
    g_signal_connect (notebook, "focus-tab", 
            G_CALLBACK (focus_tab), (void *)this);
    g_signal_connect (notebook, "move-focus-out", 
            G_CALLBACK (move_focus_out), (void *)this);
    g_signal_connect (notebook, "page-added", 
            G_CALLBACK (page_added), (void *)this);
    g_signal_connect (notebook, "page-removed", 
            G_CALLBACK (page_removed), (void *)this);
    g_signal_connect (notebook, "page-reordered", 
            G_CALLBACK (page_reordered), (void *)this);
    g_signal_connect (notebook, "reorder-tab", 
            G_CALLBACK (reorder_tab), (void *)this);
    g_signal_connect (notebook, "select-page", 
            G_CALLBACK (select_page), (void *)this);
    g_signal_connect (notebook, "switch-page", 
            G_CALLBACK (switch_page), (void *)this);


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

// FIXME: should call this function when page changes
void
view_c::set_application_icon (void) {
    const gchar *iconname = xfdir_p->get_xfdir_iconname();
    GdkPixbuf *icon_pixbuf = xfdir_p->get_pixbuf (iconname, GTK_ICON_SIZE_DIALOG);
    if(icon_pixbuf) {
	GtkWindow *window = ((window_c *)window_p)->get_window();
        gtk_window_set_icon (window, icon_pixbuf);
    }
    // FIXME add to tab label (not here...)
}

void
view_c::set_window_title(void){
    gchar *window_title = xfdir_p->get_window_name();
    GtkWindow *window = GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET(notebook)));
    gtk_window_set_title (window, window_title);
    g_free (window_title);

}


