#include "xffm_c.hpp"
#include "view_c.hpp"
#include "window_c.hpp"
#include "pathbar_c.hpp"

///////////////////////////////////////////////////
//         static thread functions  (used)       //
///////////////////////////////////////////////////

static gboolean unhighlight(void *, void *, void *);
static gboolean motion_notify_event(GtkWidget *, GdkEvent *, void *);
static gboolean leave_notify_event(GtkWidget *, GdkEvent *, void *);
static void on_remove_page_button(GtkWidget *, void *);
static void *show_text_f(GtkWidget *, void *);
static void toggle_hidden_f(GtkWidget *, void *);
static void clear_text_f(GtkWidget *, void *);
static void clear_status_f(GtkWidget *, void *);
static void hide_text_f(GtkWidget *, void *);
static void item_activated (GtkIconView *, GtkTreePath *, void *);
static gboolean change_current_page (GtkNotebook *notebook, gint, void *);
static GtkNotebook* create_window (GtkNotebook *notebook, GtkWidget *, gint, gint, void *);
static gboolean focus_tab (GtkNotebook *, GtkNotebookTab, void *);
static void move_focus_out (GtkNotebook *, GtkDirectionType, void *);
static void page_added (GtkNotebook *, GtkWidget *, guint, void *);
static void page_removed (GtkNotebook *, GtkWidget *, guint, void *);
static void page_reordered (GtkNotebook *, GtkWidget *, guint, void *);
static gboolean reorder_tab (GtkNotebook *, GtkDirectionType, gboolean , void *);
static gboolean select_page (GtkNotebook *, gboolean , void *);
static void switch_page (GtkNotebook *, GtkWidget *, guint, void *);
static gboolean
query_tooltip_f (GtkWidget  *widget,
               gint        x,
               gint        y,
               gboolean    keyboard_mode,
               GtkTooltip *tooltip,
               gpointer    data);
static gboolean
button_press_f (GtkWidget *widget,
               GdkEvent  *event,
               gpointer   data);

void
signal_drag_data (GtkWidget * widget,
                  GdkDragContext * context,
                  gint x, gint y, 
		  GtkSelectionData * selection_data, 
		  guint info, 
		  guint time, 
		  gpointer data) {
    fprintf(stderr, "signal_drag_data\n");
}
void
signal_drag_leave (GtkWidget * widget, GdkDragContext * drag_context, guint time, gpointer data) {
    fprintf(stderr, "signal_drag_leave\n");
    NOOP ("rodent_mouse: DND>> rodent_signal_drag_leave\n");

}

void
signal_drag_delete (GtkWidget * widget, GdkDragContext * context, gpointer data) {
    fprintf(stderr, "signal_drag_delete\n");
    NOOP ("rodent_mouse: DND>> rodent_signal_drag_delete\n");
}
gboolean
signal_drag_motion (GtkWidget * widget, 
	GdkDragContext * dc, gint x, gint y, guint t, gpointer data) {
    fprintf(stderr, "signal_drag_motion\n");
}

void
signal_drag_data_get (GtkWidget * widget,
                      GdkDragContext * context, GtkSelectionData * selection_data, guint info, guint time, gpointer data) {
    fprintf(stderr, "signal_drag_data_get\n");
}

void
signal_drag_begin (GtkWidget * widget, GdkDragContext * drag_context, gpointer data) {
    fprintf(stderr, "signal_drag_begin\n");
}

void
signal_drag_end (GtkWidget * widget, GdkDragContext * context, gpointer data) {
    fprintf(stderr, "signal_drag_end\n");
}



////////////////////////////////////////
// class methods
////////////////////////////////////////

view_c::view_c(data_c *data0, void *window_data, GtkNotebook *notebook) :  widgets_c(data0, window_data, notebook), thread_control_c((void *)this) {
    xfdir_p = NULL;
    data_p = data0;
    init();
}


view_c::view_c(data_c *data0, void *window_data, GtkNotebook *notebook, const gchar *path) : widgets_c(data0, window_data, notebook), thread_control_c((void *)this) {
    NOOP( "view_c::view_c.....\n");
    xfdir_p = NULL;
    xfdir_c *new_xfdir_p;
    data_p = data0;
    init();
    if (g_file_test(path, G_FILE_TEST_IS_DIR) ){
	new_xfdir_p = (xfdir_c *)new xfdir_local_c(data_p, path, shows_hidden());
    } else {
	// load specific class xfdir here
    }
    set_treemodel(new_xfdir_p);
    set_spinner(FALSE);
}


view_c::~view_c(void){
    DBG("view_c::~view_c\n");
    if (xfdir_p) delete xfdir_p;
    if (lpterm_p) delete lpterm_p;
}

gboolean
view_c::shows_hidden(void){
    gboolean state = gtk_toggle_button_get_active(get_hidden_button());
    return state;
}

void
view_c::toggle_show_hidden(void){
    if (!xfdir_p) return;
    xfdir_p->set_show_hidden(shows_hidden());
    reload(xfdir_p->get_path());
}

void 
view_c::root(void){
    DBG("root treemodel\n");
    xfdir_c *data = (xfdir_c *)new xfdir_root_c(data_p, "xffm:root");
    set_treemodel(data);
    
}

gboolean
view_c::window_keyboard_event(GdkEventKey *event, void *data){
    return lpterm_p->window_keyboard_event(event, data);
}
void
view_c::set_page_label(void){
    gchar *tab_label = g_path_get_basename(xfdir_p->get_label());
    gtk_label_set_markup(GTK_LABEL(get_page_label()), tab_label);
    g_free(tab_label);
}

gint 
view_c::get_dir_count(void){ 
    if (!xfdir_p) return 0;
    return xfdir_p->get_dir_count();
}

void
view_c::reload(const gchar *data){
    NOOP( "view_c::reload.....\n");
    set_spinner(TRUE);
    // clear highlight hash
    xfdir_p->clear_highlights();
    if (g_file_test(data, G_FILE_TEST_IS_DIR) &&
	    !g_file_test(get_path(), G_FILE_TEST_IS_DIR)){
        // This is to switch from a module to a local xfdir
	while (gtk_events_pending()) gtk_main_iteration();
	// switch back to local mode
        NOOP( "hidden toggle=%d\n", shows_hidden());
	xfdir_c *xfdir_local_p = (xfdir_c *)new xfdir_local_c(data_p, data, shows_hidden());
	set_treemodel(xfdir_local_p);
        delete xfdir_p;
	set_spinner(FALSE);
	return;
    }
    xfdir_p->reload(data);
    set_view_details();
    while (gtk_events_pending()) gtk_main_iteration();
    if (get_dir_count() <= 500) highlight();
    set_spinner(FALSE);
}

void
view_c::remove_page(void){
    gint page_num = gtk_notebook_page_num (get_notebook(), get_page_child());
    gint current_page = gtk_notebook_get_current_page (get_notebook());

    gtk_notebook_remove_page (get_notebook(), page_num);
    if (current_page == page_num) {
        gtk_notebook_set_current_page (get_notebook(), page_num-1);
    }    
}

xfdir_c *
view_c::get_xfdir_p(void) {return xfdir_p;}

const gchar *
view_c::get_path(void) {return xfdir_p->get_path();}


lpterm_c *
view_c::get_lpterm_p(void){return lpterm_p;}

GtkWindow *
view_c::get_window(void){
    window_c *window_p = (window_c *)get_window_v();
    return window_p->get_window();
}


void
view_c::set_treemodel(xfdir_c *data){
    xfdir_c *old_xfdir_p = xfdir_p;
    xfdir_p = data;
    GtkTreeModel *tree_model = xfdir_p->get_tree_model();
    NOOP( "new treemodel= %p (old_xfdir=%p new_xfdir=%p)\n", tree_model, old_xfdir_p, xfdir_p);
    //if (tree_model) gtk_widget_hide(GTK_WIDGET(get_iconview()));
    gtk_icon_view_set_model(GTK_ICON_VIEW(get_iconview()), tree_model);
    gtk_icon_view_set_text_column (GTK_ICON_VIEW(get_iconview()), xfdir_p->get_text_column());
    gtk_icon_view_set_pixbuf_column (GTK_ICON_VIEW(get_iconview()),  xfdir_p->get_icon_column());
    gtk_icon_view_set_selection_mode (GTK_ICON_VIEW(get_iconview()), GTK_SELECTION_MULTIPLE);
    //gtk_icon_view_set_tooltip_column (GTK_ICON_VIEW(get_iconview()),3);
    set_view_details();
    //gtk_widget_show(GTK_WIDGET(get_iconview()));
    NOOP( "set_treemodel done, now deleting %p\n", old_xfdir_p);
    if (old_xfdir_p) delete old_xfdir_p;
}
///////////////////////////// Private:
void
view_c::set_view_details(void){
    set_page_label();
    set_window_title();
    set_application_icon();
    update_tab_label_icon();
    while (gtk_events_pending()) gtk_main_iteration();
    update_pathbar(xfdir_p->get_path());
}

void
view_c::init(void){
    // Set objects in parent widget_c class with data pointing to child class
    g_object_set_data(G_OBJECT(get_pathbar()), "view_p", (void *)this);
    g_object_set_data(G_OBJECT(get_page_child()), "view_p", (void *)this);
    g_object_set_data(G_OBJECT(get_page_button()), "view_p", (void *)this);

    create_target_list();
    //signals_p = new signals_c();
    signals();
    pack();
    // lp_term object creation
    lpterm_p = new lpterm_c(data_p, (void *)this);
    
    lpterm_p->print_status(g_strdup(""));
    lpterm_p->show_text();
    lpterm_p->print(g_strdup_printf("%s\n", "Hello world."));
    lpterm_p->print_tag(NULL, g_strdup_printf("%s\n", "No tag."));
    lpterm_p->print_tag("tag/green",g_strdup_printf( "%s", "Green tag."));
    lpterm_p->print_tag("tag/bold",g_strdup_printf( "%s\n", "bold tag."));
    lpterm_p->print_error(g_strdup_printf("%s\n", "This is an error."));
    lpterm_p->print_debug(g_strdup_printf("%s\n", "This is a debug message."));
    lpterm_p->print_icon("face-monkey",g_strdup_printf("%s\n", "This is face-monkey."));
    lpterm_p->print_icon_tag("face-angry","tag/red",g_strdup_printf("%s\n", "This is face-angry in red."));
#if 10
    // FIXME
    /* drag and drop events */
    create_target_list ();
#endif

}

void
view_c::clear_diagnostics(void){
    clear_text_f(NULL, (void *)this);
    hide_text_f(NULL, (void *)this);
}

void
view_c::clear_status(void){
    clear_status_f(NULL, (void *)this);
}

void
view_c::show_diagnostics(void){
    show_text_f(NULL, (void *)this);
}

void 
view_c::highlight(void){
    highlight(highlight_x, highlight_y);
}

void 
view_c::highlight(gdouble X, gdouble Y){
    if (!xfdir_p) return; // avoid race condition here.
    highlight_x = X;
    highlight_y = Y;
    GtkTreeIter iter;
    GtkIconView *iconview = GTK_ICON_VIEW(get_iconview());
    GtkTreeModel *model = gtk_icon_view_get_model(iconview);
    
    GtkTreePath *tpath = gtk_icon_view_get_path_at_pos (iconview, X, Y); 
    if (tpath) {
        xfdir_p->highlight(tpath);
        //xfdir_p->tooltip(iconview, gtk_tree_path_copy(tpath));
    }
    else xfdir_p->clear_highlights();
}

void
view_c::signals(void){
    // iconview tooltips
    g_signal_connect (get_iconview(), "query-tooltip", 
            G_CALLBACK (query_tooltip_f), (void *)this);

    g_signal_connect (get_iconview(), "button-press-event",
	    G_CALLBACK(button_press_f), (void *)this);
    g_signal_connect (get_hidden_button(), "clicked", 
            G_CALLBACK (toggle_hidden_f), (void *)this);
    // clear button:
    g_signal_connect (get_clear_button(), "clicked", 
            G_CALLBACK (clear_text_f), (void *)this);
    g_signal_connect (get_clear_button(), "clicked", 
            G_CALLBACK (hide_text_f), (void *)this);
    g_signal_connect (get_page_button(), "clicked", 
            G_CALLBACK (on_remove_page_button), (void *)this);
    // iconview specific signal bindings:
    g_signal_connect (get_iconview(), "item-activated", 
            G_CALLBACK (item_activated), (void *)this);
    g_signal_connect (get_iconview(), "motion-notify-event", 
            G_CALLBACK (motion_notify_event), (void *)this);
    g_signal_connect (get_iconview(), "leave-notify-event", 
            G_CALLBACK (leave_notify_event), (void *)this);



    // notebook specific signal bindings:
    g_signal_connect (get_notebook(), "change-current-page", 
            G_CALLBACK (change_current_page), (void *)this);
    g_signal_connect (get_notebook(), "create-window", 
            G_CALLBACK (create_window), (void *)this);
    g_signal_connect (get_notebook(), "focus-tab", 
            G_CALLBACK (focus_tab), (void *)this);
    g_signal_connect (get_notebook(), "move-focus-out", 
            G_CALLBACK (move_focus_out), (void *)this);
    g_signal_connect (get_notebook(), "page-added", 
            G_CALLBACK (page_added), (void *)this);
    g_signal_connect (get_notebook(), "page-removed", 
            G_CALLBACK (page_removed), (void *)this);
    g_signal_connect (get_notebook(), "page-reordered", 
            G_CALLBACK (page_reordered), (void *)this);
    g_signal_connect (get_notebook(), "reorder-tab", 
            G_CALLBACK (reorder_tab), (void *)this);
    g_signal_connect (get_notebook(), "select-page", 
            G_CALLBACK (select_page), (void *)this);
    g_signal_connect (get_notebook(), "switch-page", 
            G_CALLBACK (switch_page), (void *)this);

// DnD ... testing
    g_signal_connect (G_OBJECT (get_iconview()), 
	    "drag-data-received", G_CALLBACK (signal_drag_data), (void *)this);
    g_signal_connect (G_OBJECT (get_iconview()), 
	    "drag-data-get", G_CALLBACK (signal_drag_data_get), (void *)this);
    g_signal_connect (G_OBJECT (get_iconview()), 
	    "drag-motion", G_CALLBACK (signal_drag_motion), (void *)this);
    g_signal_connect (G_OBJECT (get_iconview()), 
	    "drag-end", G_CALLBACK (signal_drag_end), (void *)this);
    g_signal_connect (G_OBJECT (get_iconview()), 
	    "drag-begin", G_CALLBACK (signal_drag_begin), (void *)this);
    g_signal_connect (G_OBJECT (get_iconview()), 
	    "drag-leave", G_CALLBACK (signal_drag_leave), (void *)this);
    g_signal_connect (G_OBJECT (get_iconview()), 
	    "drag-data-delete", G_CALLBACK (signal_drag_delete), (void *)this);

    /*  FIXME: Check which callbacks are necessary and which are not.

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

void
view_c::update_tab_label_icon(void){
    GList *children = 
	gtk_container_get_children (GTK_CONTAINER(get_page_label_icon_box()));
    GList *l = children;
    for (;l && l->data; l=l->next){
	 gtk_widget_destroy(GTK_WIDGET(l->data));
    }
    g_list_free(children);
    const gchar *icon_name = xfdir_p->get_xfdir_iconname();
    GdkPixbuf *pixbuf = 
            get_pixbuf(icon_name, GTK_ICON_SIZE_BUTTON);
    if (pixbuf){
	GtkWidget *image = gtk_image_new_from_pixbuf (pixbuf);
	gtk_container_add (GTK_CONTAINER (get_page_label_icon_box()), image);
	gtk_widget_show(image);
    }
}

GtkTreeModel *
view_c::get_tree_model(void){return xfdir_p->get_tree_model();}

void
view_c::set_application_icon (void) {
    const gchar *iconname = xfdir_p->get_xfdir_iconname();
    GdkPixbuf *icon_pixbuf = get_pixbuf (iconname, GTK_ICON_SIZE_DIALOG);
    if(icon_pixbuf) {
	GtkWindow *window = ((window_c *)get_window_v())->get_window();
        gtk_window_set_icon (window, icon_pixbuf);
    }
    // FIXME add to tab label (not here...)
}

void
view_c::set_application_icon (gint page_num) {
    GtkWidget *child_box = gtk_notebook_get_nth_page(get_notebook(), page_num);
    view_c *view_p = (view_c *)g_object_get_data(G_OBJECT(child_box), "view_p");
    if (!view_p->get_xfdir_p()) return;
    
    const gchar *iconname = view_p->get_xfdir_p()->get_xfdir_iconname();
    GdkPixbuf *icon_pixbuf = get_pixbuf (iconname, GTK_ICON_SIZE_DIALOG);
    if(icon_pixbuf) {
	GtkWindow *window = ((window_c *)get_window_v())->get_window();
        gtk_window_set_icon (window, icon_pixbuf);
    }
    // FIXME add to tab label (not here...)
}

void
view_c::set_window_title(void){
    gchar *window_title = xfdir_p->get_window_name();
    GtkWindow *window = GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET(get_notebook())));
    gtk_window_set_title (window, window_title);
    g_free (window_title);

}

void
view_c::set_window_title(gint page_num){
    GtkWidget *child_box = gtk_notebook_get_nth_page(get_notebook(), page_num);
    view_c *view_p = (view_c *)g_object_get_data(G_OBJECT(child_box), "view_p");
    if (!view_p->get_xfdir_p()) return;

    gchar *window_title = view_p->get_xfdir_p()->get_window_name();
    GtkWindow *window = GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET(get_notebook())));
    gtk_window_set_title (window, window_title);
    g_free (window_title);

}

/////////////////////////////////////////
// simple callbacks and thread functions:
/////////////////////////////////////////




static gboolean
motion_notify_event (GtkWidget *widget,
               GdkEvent  *event,
               gpointer   data){
    view_c * view_p = (view_c *)data;
    if (view_p->get_dir_count() > 500) return FALSE;
    if (!data) g_error("motion_notify_event: data cannot be NULL\n");
    GdkEventMotion *e = (GdkEventMotion *)event;
    view_p->highlight(e->x, e->y);
    return FALSE;
}
static gboolean
leave_notify_event (GtkWidget *widget,
               GdkEvent  *event,
               gpointer   data){
    view_c * view_p = (view_c *)data;
    if (!data) g_error("leave_notify_event: data cannot be NULL\n");
    //fprintf(TRACE("leave_notify_event\n");
    view_p->get_xfdir_p()->clear_highlights();
    window_c *window_p = (window_c *)view_p->get_window_v();
    window_p->set_tt_window(NULL, NULL);
}


static void 
on_remove_page_button(GtkWidget *b, gpointer data){
    view_c *view_p = (view_c *)data;
    view_p->remove_page();       
    // delete object: (remove from view_list)
    window_c *window_p = (window_c *)view_p->get_window_v();
    window_p->remove_view_from_list(data); // this calls view_c destructor
}

static void *
show_text_f (GtkWidget *w, gpointer data) {
    view_c *view_p =(view_c *)data;
    GtkAllocation allocation;
    gtk_widget_get_allocation (GTK_WIDGET(view_p->get_window()), &allocation);
    
    GtkWidget *vpane = view_p->get_vpane();
    if(!vpane) {
        fprintf(stderr, "vpane is NULL\n");
        return NULL;
    }
    if (allocation.height > 50)
    {
	gdouble position = 
	    gtk_paned_get_position (GTK_PANED(vpane));
	if(position > allocation.height * 0.90) {
	    gtk_paned_set_position (GTK_PANED (vpane), allocation.height * 0.60);
	}
    }
    return NULL;
}

static void
clear_status_f	(GtkWidget *w, gpointer data){
    view_c *view_p =(view_c *)data;
    GtkTextView *status = GTK_TEXT_VIEW(view_p->get_status());
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (status);
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds (buffer, &start, &end);
    gtk_text_buffer_delete (buffer, &start, &end);
}

static void
toggle_hidden_f	(GtkWidget *w, gpointer data){
    view_c *view_p =(view_c *)data;
    view_p->toggle_show_hidden();
}


static void
clear_text_f	(GtkWidget *w, gpointer data){
    view_c *view_p =(view_c *)data;
    GtkTextView *diagnostics = GTK_TEXT_VIEW(view_p->get_diagnostics());
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (diagnostics);
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds (buffer, &start, &end);
    gtk_text_buffer_delete (buffer, &start, &end);
}

static void
hide_text_f(GtkWidget *w, gpointer data){
    view_c *view_p =(view_c *)data;
    GtkWidget *vpane = GTK_WIDGET(view_p->get_vpane());
    gtk_paned_set_position (GTK_PANED (vpane), 10000);
    return;
}

////////////// iconview specific signal bindings: /////////////////////////
static void
item_activated (GtkIconView *iconview,
                GtkTreePath *tpath,
                gpointer     data)
{
    view_c *view_p = (view_c *)data;
    xfdir_c *xfdir_p = view_p->get_xfdir_p();
    xfdir_p->item_activated(iconview, tpath, data);
}


/////////////  notebook specific signal bindings: /////////////////////////
    
static gboolean
change_current_page (GtkNotebook *notebook,
               gint         arg1,
               gpointer     data)
{
    TRACE("change_current_page\n");
    return FALSE;
}

static GtkNotebook*
create_window (GtkNotebook *notebook,
               GtkWidget   *page,
               gint         x,
               gint         y,
               gpointer     data)
{
    TRACE("create_window\n");
    return NULL;
}

static gboolean
focus_tab (GtkNotebook   *notebook,
               GtkNotebookTab arg1,
               gpointer       data)
{
    TRACE("focus_tab\n");
    return FALSE;
}

static void
move_focus_out (GtkNotebook     *notebook,
               GtkDirectionType arg1,
               gpointer         data)
{
    TRACE("move_focus_out\n");
}

static void
page_added (GtkNotebook *notebook,
               GtkWidget   *child,
               guint        page_num,
               gpointer     data)
{
    TRACE("page_added\n");
}

static void
page_removed (GtkNotebook *notebook,
               GtkWidget   *child,
               guint        page_num,
               gpointer     data)
{
    TRACE("page_removed\n");
}

static void
page_reordered (GtkNotebook *notebook,
               GtkWidget   *child,
               guint        page_num,
               gpointer     data)
{
    TRACE("page_reordered\n");
}

static gboolean
reorder_tab (GtkNotebook     *notebook,
               GtkDirectionType arg1,
               gboolean         arg2,
               gpointer         data)
{
    TRACE("reorder_tab\n");
    return FALSE;
}

static gboolean
select_page (GtkNotebook *notebook,
               gboolean     arg1,
               gpointer     data)
{
    TRACE("select_page\n");
    return FALSE;
}

static void
switch_page (GtkNotebook *notebook,
               GtkWidget   *page,
               guint        new_page,
               gpointer     data)
{
    // This callback may occur after view has been destroyed.
    window_c *window_p = (window_c *)g_object_get_data(G_OBJECT(notebook), "window_p");
    if (!window_p->is_view_in_list(data)) {
	DBG("switch_page:: view_p %p no longer exists.\n", data);
	return;
    }

    view_c *view_p = (view_c *)data;
    gint current_page = gtk_notebook_get_current_page (notebook);
    TRACE("switch_page, page_num=%d current_page=%d xfdir_p=%p\n" ,new_page, current_page, view_p->get_xfdir_p());
    if (!view_p->get_xfdir_p()) return;
    view_p->set_window_title(new_page);
    view_p->set_application_icon(new_page);
}


static gboolean
query_tooltip_f (GtkWidget  *widget,
               gint        x,
               gint        y,
               gboolean    keyboard_mode,
               GtkTooltip *tooltip,
               gpointer    data){
    view_c *view_p = (view_c *)data;
    GtkIconView *icon_view = GTK_ICON_VIEW(view_p->get_iconview());

    if (!gtk_icon_view_get_tooltip_context(icon_view, &x, &y, FALSE, 
                NULL, NULL, NULL)) {
        NOOP( "tooltip context %d,%d\n", -1,-1);
        view_p->setup_tooltip(-1, -1);
    } else {
        NOOP( "tooltip context %d,%d\n", x,y);
        view_p->setup_tooltip(x, y);
    }
    return FALSE;
}

void
view_c::setup_tooltip(gint x, gint y){
    
    window_c *window_p = (window_c *)get_window_v();
    if (x < 0 || y < 0) {
        window_p->set_tt_window(NULL, NULL);
        return;
    }

    if (xfdir_p->is_large()) return;
    GtkTreePath *tpath = 
        gtk_icon_view_get_path_at_pos (GTK_ICON_VIEW(get_iconview()), x, y); 
    if (!tpath) {
        window_p->set_tt_window(NULL, NULL);
        return;
    }

    gchar *path_string = gtk_tree_path_to_string(tpath);
    // do we need to remake tt_window?
    const gchar *last_path_string = window_p->get_tooltip_path_string();
    if (last_path_string && strcmp(last_path_string, path_string)==0){
        g_free(path_string);
        return;
    }
    window_p->set_tooltip_path_string(path_string);


    gchar *text = xfdir_p->get_tooltip_text(tpath);
    if (!text) text = xfdir_p->make_tooltip_text(tpath);

    
    gchar *vname = xfdir_p->get_verbatim_name(tpath);
    gchar *u = wrap_utf_string(vname, 30);
    g_free(vname);

    gchar *markup = g_strdup_printf("<b>%s</b>", u);
    g_free(u);

    GdkPixbuf *pixbuf = xfdir_p->get_tooltip_pixbuf(tpath);
    if (!pixbuf) pixbuf = xfdir_p->get_normal_pixbuf(tpath); 
    gtk_tree_path_free(tpath);
    
    GtkWidget *tt_window = get_tt_window(
                pixbuf,     
                text,
                markup);
    g_free(text);
    g_free(markup);
    window_p->set_tt_window(tt_window, path_string);
    g_free(path_string);
    


    return;
}


static gboolean
button_press_f (GtkWidget *widget,
               GdkEvent  *event,
               gpointer   data){
    GdkEventButton *event_button = (GdkEventButton *)event;
    // long press or button 3 should do popup menu...
    if (event_button->button != 3) return FALSE;
    GtkTreePath *path;

    if (!gtk_icon_view_get_item_at_pos (GTK_ICON_VIEW(widget),
                               event_button->x,
                               event_button->y,
                               &path, NULL)) return FALSE;

    NOOP( "view_p: button_press_event...\n");
    
    gboolean retval = ((view_c *)data)->get_xfdir_p()->popup(path);
    gtk_tree_path_free(path);
    return retval;
}

///////////////////////////////////////////////////////////////////////
enum {
    TARGET_URI_LIST,
    TARGET_PLAIN,
    TARGET_UTF8,
    TARGET_STRING,
    TARGET_ROOTWIN,
    TARGET_MOZ_URL,
    TARGET_XDS,
    TARGET_RAW,
    TARGETS
};

static GtkTargetEntry target_table[] = {
    {(gchar *)"text/uri-list", 0, TARGET_URI_LIST},
    {(gchar *)"text/x-moz-url", 0, TARGET_MOZ_URL},
    {(gchar *)"text/plain", 0, TARGET_PLAIN},
    {(gchar *)"UTF8_STRING", 0, TARGET_UTF8},
    {(gchar *)"STRING", 0, TARGET_STRING}
};

#define NUM_TARGETS (sizeof(target_table)/sizeof(GtkTargetEntry))
void
view_c::create_target_list (void) {
    //if(target_list) return;
    target_list = gtk_target_list_new (target_table, NUM_TARGETS);
/*    this does not seem to be necessary */
    gtk_drag_source_set ((GtkWidget *) get_iconview(),
                         (GdkModifierType)(GDK_BUTTON1_MASK | GDK_BUTTON2_MASK), target_table,
                         NUM_TARGETS, GDK_ACTION_COPY);
                      //   NUM_TARGETS, (GdkModifierType)(GDK_ACTION_MOVE | GDK_ACTION_COPY | GDK_ACTION_LINK));
    gtk_drag_dest_set ((GtkWidget *) get_iconview(),
                       (GTK_DEST_DEFAULT_DROP), target_table, NUM_TARGETS,
                        GDK_ACTION_COPY);
//                       (GdkModifierType)(GDK_ACTION_MOVE | GDK_ACTION_COPY | GDK_ACTION_LINK));
    return;
}

#if 0
/**  drag events **********************************************/
// This signal is received by the receiving end of the drag/drop action
void
signal_drag_data (GtkWidget * widget,
                  GdkDragContext * context,
                  gint x, gint y, 
		  GtkSelectionData * selection_data, 
		  guint info, 
		  guint time, 
		  gpointer data) {
    NOOP ("rodent_mouse: DND>> rodent_signal_drag_data\n");

    const population_t *population_p;
    view_t *view_p = (view_t *) data;

    record_entry_t *target_en;
    NOOP ("rodent_mouse: drag_data: drag_data...\n");
    /*view_p = (view_t *)g_object_get_data(G_OBJECT(widget),"view_p"); */
    if(!view_p) {
        DBG ("rodent_signal_drag_data() view_p==NULL\n");
        gtk_drag_finish (context, FALSE, FALSE, time);
        return;
    }
    target_en = view_p->en;

    if(!target_en || !target_en->path) {
        NOOP ("rodent_mouse: drag_data: !target_en || !target_en->path\n");
        NOOP ("rodent_mouse: --DND>>rodent_signal_drag_data !target_en || !target_en->path\n");
        gtk_drag_finish (context, FALSE, FALSE, time);
        return;
    }

    // drop will proceed...
    if (!rfm_population_try_read_lock (view_p, "rodent_signal_drag_data")){
        gtk_drag_finish (context, FALSE, FALSE, time);
    }
    rfm_global_t *rfm_global_p = rfm_global();
    rfm_cursor_wait (rfm_global_p->window);

    NOOP ("rodent_mouse: population_sem: rodent_signal_drag_data() obtained...\n");
    population_p = rodent_find_in_population (view_p, x, y);
    if(!population_p) {
        population_p = rodent_find_in_labels (view_p, x, y);
    }

    if(population_p && population_p->en && population_p->en->path) {
        if(IS_SDIR(population_p->en->type)){
            target_en = population_p->en;
	} else if (population_p->en->mimetype &&
		strcmp(population_p->en->mimetype, "application/x-desktop")==0){
            target_en = population_p->en;
	}
    }

    gchar *source_path = NULL;
    read_drag_info(&source_path, NULL);
    
    NOOP ("rodent_mouse: DND>>rodent_signal_drag_data: target entry is %s, source is %s\n", (target_en)?target_en->path:NULL, source_path);
    // Here we check if source and target are the same, in which case dnd should 
    // be ignored.
    if(!target_en || !source_path || !target_en->path ||
	    strcmp (target_en->path, source_path) == 0){
	NOOP("rodent_mouse: ignoring drop command! source_path=%s target=%s\n", source_path ,(target_en)?target_en->path:NULL);
        rfm_cursor_reset (rfm_global_p->window);
        drag_view_p = NULL;
        gtk_drag_finish (context, FALSE, FALSE, time);
        rfm_population_read_unlock (view_p, "rodent_signal_drag_data");
        return;
    }
    g_free(source_path);

    NOOP ("rodent_mouse: drag_data...gui_drag_data\n");
    if(gui_drag_data (&(view_p->widgets), target_en, context, x, y, selection_data, info, time)) {
        NOOP ("rodent_mouse: drag_data...reload 0x%lx->0x%lx\n", (unsigned long)view_p, (unsigned long)view_p->en);
    }

    NOOP ("rodent_mouse: drag_data...all done\n");
    rfm_cursor_reset (rfm_global_p->window);
    rfm_population_read_unlock (view_p, "rodent_signal_drag_data");
    NOOP ("population_sem: rodent_signal_drag_data() released!\n");
    return;
}
//#define NOOP NOOP

void
signal_drag_leave (GtkWidget * widget, GdkDragContext * drag_context, guint time, gpointer data) {
    NOOP ("rodent_mouse: DND>> rodent_signal_drag_leave\n");

}

void
signal_drag_delete (GtkWidget * widget, GdkDragContext * context, gpointer data) {
    NOOP ("rodent_mouse: DND>> rodent_signal_drag_delete\n");
}


gboolean
signal_drag_motion (GtkWidget * widget, 
	GdkDragContext * dc, gint x, gint y, guint t, gpointer data) {
    NOOP ("rodent_mouse: DND>> rodent_signal_drag_motion\n");

    gboolean target_ok = FALSE;
    view_t *view_p = (view_t *) data;
    if (!rfm_population_try_read_lock (view_p, "rodent_signal_drag_motion")) return TRUE;
        
    
    NOOP ("rodent_mouse: population_sem: rodent_signal_drag_motion() obtained...\n");

    population_t *population_p = (population_t *) rodent_find_in_population (view_p, x, y);

    NOOP ("rodent_mouse: on_drag_motion...x=%d, y= %d, population_p=0x%lx\n", x, y, (unsigned long)population_p);
    rodent_hide_tip ();

    gboolean local_target = TRUE;
    gboolean local_source = TRUE;
    gint type=0;
    read_drag_info(NULL, &type);
    if (!IS_LOCAL_TYPE(type))local_source = FALSE;
    if (view_p->en && !IS_LOCAL_TYPE(view_p->en->type))local_target = FALSE;
    if(population_p) {
        /* if not valid drop target, return */

        if(POPULATION_MODULE(population_p)) {
            if(rfm_natural (PLUGIN_DIR, POPULATION_MODULE(population_p),
			population_p->en, "valid_drop_site"))
                target_ok = TRUE;
        } else {                /* local */
	    if (population_p->en && 
		population_p->en->path) {
		if (IS_SDIR(population_p->en->type)) {
		    target_ok = TRUE;
		    if (!IS_LOCAL_TYPE(population_p->en->type))local_target = FALSE;
		}

		if (population_p->en->mimetype && 
			strcmp(population_p->en->mimetype,
			    "application/x-desktop")==0) {
		    target_ok = TRUE;
		}
	    }
	}
    }
    
    if(view_p->mouse_event.saturated_p != population_p) {
        NOOP( "condition 3, unsaturate icon\n");
	unsaturate_icon (view_p);
    }
    if (target_ok) {
	saturate_icon (view_p, population_p);
    }

    if(view_p->mouse_event.doing_drag_p) {
        NOOP ("rodent_mouse: widget ok\n");
    }
    NOOP ("rodent_mouse: DND>> rodent_signal_drag_motion source=%s target=%s\n",
	    (local_source)?"local":"remote",
	    (local_target)?"local":"remote");
    

    if(getenv ("RFM_DRAG_DOES_MOVE") && strlen (getenv ("RFM_DRAG_DOES_MOVE")))
        view_p->mouse_event.drag_action = GDK_ACTION_MOVE;
    else
        view_p->mouse_event.drag_action = GDK_ACTION_COPY;

    // Override remote dnd with copy
    // when target or source is remote.
    if (!local_target || !local_source) {
        view_p->mouse_event.drag_action = GDK_ACTION_COPY;
    } 

#if GTK_MAJOR_VERSION==2
    gint actions = dc->actions;
#else
    gint actions = gdk_drag_context_get_actions(dc);
#endif
    if(actions == GDK_ACTION_MOVE)
        gdk_drag_status (dc, GDK_ACTION_MOVE, t);
    else if(actions == GDK_ACTION_COPY)
        gdk_drag_status (dc, GDK_ACTION_COPY, t);
    else if(actions == GDK_ACTION_LINK)
        gdk_drag_status (dc, GDK_ACTION_LINK, t);
    else if(actions & view_p->mouse_event.drag_action)
        gdk_drag_status (dc, view_p->mouse_event.drag_action, t);
    else
        gdk_drag_status (dc, 0, t);
    rfm_population_read_unlock (view_p, "rodent_signal_drag_motion");
    NOOP ("rodent_mouse: population_sem: rodent_signal_drag_motion() released!\n");
    return (TRUE);
}

// This signal is received by the sending end of the drag/drop event
void
signal_drag_data_get (GtkWidget * widget,
                      GdkDragContext * context, GtkSelectionData * selection_data, guint info, guint time, gpointer data) {
    NOOP ("rodent_mouse: DND>> rodent_signal_drag_data_get\n");
    view_t *view_p = (view_t *) data;
    rodent_hide_tip ();
    gui_drag_data_get (&(view_p->widgets), view_p->selection_list, context, selection_data, info, time);
    NOOP ("rodent_mouse: drag_data_get: all done\n");
}

void
signal_drag_begin (GtkWidget * widget, GdkDragContext * drag_context, gpointer data) {
    view_t *view_p = (view_t *) data;
    drag_view_p = view_p;
    rodent_hide_tip ();
    if (!view_p->en || !view_p->en->path) return; 
    write_drag_info(view_p->en->path, view_p->en->type);
    view_p->mouse_event.drag_event.context = drag_context;
}

void
signal_drag_end (GtkWidget * widget, GdkDragContext * context, gpointer data) {
    view_t *view_p = (view_t *) data;
    widgets_t *widgets_p = &(view_p->widgets);
    //rfm_diagnostics(widgets_p, "xffm_tag/red","rodent_mouse: DND>> rodent_signal_drag_end\n", NULL); 
    NOOP ("rodent_mouse: DND>> rodent_signal_drag_end\n" );
    view_p->mouse_event.doing_drag_p = NULL;
    rfm_global_t *rfm_global_p = rfm_global();
    rfm_cursor_reset(rfm_global_p->window);
    
    // Immediate test for a reload condition by the thread monitor.
    if (!xfdir_monitor_control_greenlight(widgets_p)){
	rodent_trigger_reload(view_p);
    }



    if(dnd_data) {
        g_free (dnd_data);
        dnd_data = NULL;
    }
    drag_view_p = NULL;
    NOOP ("rodent_mouse: drag_end... alldone\n");
    // Remove old MIT-shm  dnd info.
    shm_unlink (DND_SHM_NAME);
}

/* end of drag signals ********************************/

#endif

