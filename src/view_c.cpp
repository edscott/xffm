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


////////////////////////////////////////
// class methods
////////////////////////////////////////


view_c::view_c(void *window_data, GtkNotebook *notebook) : widgets_c(window_data, notebook), thread_control_c((void *)this) {
    xfdir_p = NULL;
    dirty_hash = FALSE;

    // Set objects in parent widget_c class with data pointing to child class
    g_object_set_data(G_OBJECT(pathbar_p->pathbar), "view_p", (void *)this);
    g_object_set_data(G_OBJECT(page_child_box), "view_p", (void *)this);
    g_object_set_data(G_OBJECT(page_label_button), "view_p", (void *)this);

    highlight_hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    //signals_p = new signals_c();
    init();
    icon_view = GTK_ICON_VIEW(gtk_icon_view_new());
    gtk_icon_view_set_item_width (icon_view, 60);
    gtk_icon_view_set_activate_on_single_click(icon_view, TRUE);
    signals();
    pack();
    // lp_term object creation
    lpterm_p = new lpterm_c((void *)this);
    
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
#if 0
    // FIXME
    /* drag and drop events */
    rodent_create_target_list (view_p);
#endif
    
}


view_c::~view_c(void){
    DBG("view_c::~view_c\n");
    if (xfdir_p) delete xfdir_p;
    if (lpterm_p) delete lpterm_p;
}

void 
view_c::root(void){
    DBG("root treemodel\n");
    
}

gboolean
view_c::window_keyboard_event(GdkEventKey *event, void *data){
    return lpterm_p->window_keyboard_event(event, data);
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

const gchar *
view_c::get_path(void) {return xfdir_p->get_path();}


lpterm_c *
view_c::get_lpterm_p(void){return lpterm_p;}

GtkWindow *
view_c::get_window(void){
    window_c *window_p = (window_c *)window_v;
    return window_p->get_window();
}


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
    update_tab_label_icon();
    while (gtk_events_pending()) gtk_main_iteration();
    pathbar_p->update_pathbar(xfdir_p->get_path());
}

void
view_c::init(void){

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

static gboolean
unhighlight (gpointer key, gpointer value, gpointer data){
    void **arg = (void **)data;
    view_c *view_p = (view_c *)arg[0];
    gchar *tree_path_string = (gchar *)arg[1];
    if (tree_path_string && strcmp(tree_path_string, (gchar *)key)==0) return FALSE;
    TRACE("unhighlight %s\n", (gchar *)key);
    GtkTreeModel *model = view_p->get_tree_model();
    GtkTreeIter iter;
    gchar *icon_name;
            
    GtkTreePath *tpath = gtk_tree_path_new_from_string ((gchar *)key);
    if (tpath) {
        gtk_tree_model_get_iter (model, &iter, tpath);
        gtk_tree_model_get (model, &iter, COL_ICON_NAME, &icon_name, -1);
        gchar *name;
        gtk_tree_model_get (model, &iter, COL_ACTUAL_NAME, &name, -1);
        gint icon_size = view_p->get_icon_size(name);
        g_free(name);
        gtk_list_store_set (GTK_LIST_STORE(model), &iter,
            COL_PIXBUF, view_p->get_gtk_p()->get_pixbuf(icon_name, icon_size ), 
            -1);
        g_free(icon_name);
        gtk_tree_path_free (tpath);
    }

    return TRUE;
}

void 
view_c::clear_highlights(const gchar *tree_path_string){
    void *arg[]={(void *)this, (void *)tree_path_string};
    if (!highlight_hash) DBG("view_c::clear_highlights: hash is null!\n");
    g_hash_table_foreach_remove (highlight_hash, unhighlight, (void *)arg);
    dirty_hash = (tree_path_string != NULL)? TRUE: FALSE;
}

void 
view_c::set_highlight(gdouble X, gdouble Y){
    //    TRACE("x=%lf y=%lf\n", X, Y);
    highlight_x = X + 0.5;
    highlight_y = Y + 0.5;
}

void 
view_c::highlight(void){
    //TRACE("highlight %d, %d\n", highlight_x, highlight_y);
    gchar *tree_path_string = NULL;
    
    GtkCellRenderer *cell;
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_icon_view_get_model(icon_view);
    
    GtkTreePath *tpath = gtk_icon_view_get_path_at_pos (icon_view, highlight_x, highlight_y); 

    if (tpath){
        tree_path_string = gtk_tree_path_to_string (tpath);
        if (!highlight_hash) DBG("view_c::highlight: hash is null!\n");
        if (g_hash_table_lookup(highlight_hash, tree_path_string)) {
            //TRACE("%s already in hash\n", tree_path_string);
            g_free (tree_path_string);
            gtk_tree_path_free (tpath);
            return;
        }
        // Not highlighted?
        TRACE("yes: %s\n", tree_path_string);
        
        g_hash_table_insert(highlight_hash, tree_path_string, GINT_TO_POINTER(1));
        // Do highlight.
        gtk_tree_model_get_iter (model, &iter, tpath);
        gchar *name;
        gtk_tree_model_get (model, &iter, COL_ACTUAL_NAME, &name, -1);
        gchar *icon_name=NULL;
        if (strcmp(name, "..")==0) icon_name = g_strdup("go-up");
	else {
	    gint mode;
	    gtk_tree_model_get (model, &iter, COL_MODE, &mode, -1);
	    if (S_ISDIR(mode)){
		icon_name = g_strdup("document-open");
	    } else {
		gchar *iname;
		gtk_tree_model_get (model, &iter, COL_ICON_NAME, &iname, -1);
		icon_name = g_strdup_printf("%s/NE/document-open/2.0/220", iname);
		g_free(iname);
	    }
	}
	g_free(name);

	gtk_list_store_set (GTK_LIST_STORE(model), &iter,
                COL_PIXBUF, gtk_p->get_pixbuf(icon_name, GTK_ICON_SIZE_DIALOG ), 
		-1);
	g_free(icon_name);

        gtk_tree_path_free (tpath);
        clear_highlights(tree_path_string);
    } else {
        // No item at position?
        // Do we need to clear hash table?
        if (dirty_hash){
            TRACE("no (%d, %d)\n", highlight_x,highlight_y);
            clear_highlights(NULL);
        }
    }
}

void
view_c::signals(void){
    // clear button:
    g_signal_connect (clear_button, "clicked", 
            G_CALLBACK (clear_text_f), (void *)this);
    g_signal_connect (clear_button, "clicked", 
            G_CALLBACK (hide_text_f), (void *)this);
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
    GList *children = gtk_container_get_children (GTK_CONTAINER(page_label_icon_box));
    GList *l = children;
    for (;l && l->data; l=l->next){
	 gtk_widget_destroy(GTK_WIDGET(l->data));
    }
    g_list_free(children);
    const gchar *icon_name = xfdir_p->get_xfdir_iconname();
    GdkPixbuf *pixbuf = 
            gtk_p->get_pixbuf(icon_name, GTK_ICON_SIZE_BUTTON);
    if (pixbuf){
	GtkWidget *image = gtk_image_new_from_pixbuf (pixbuf);
	gtk_container_add (GTK_CONTAINER (page_label_icon_box), image);
	gtk_widget_show(image);
    }
}

GtkTreeModel *
view_c::get_tree_model(void){return xfdir_p->get_tree_model();}
gint
view_c::get_icon_size(const gchar *name){ return xfdir_p->get_icon_size(name);}

void
view_c::set_application_icon (void) {
    const gchar *iconname = xfdir_p->get_xfdir_iconname();
    GdkPixbuf *icon_pixbuf = gtk_p->get_pixbuf (iconname, GTK_ICON_SIZE_DIALOG);
    if(icon_pixbuf) {
	GtkWindow *window = ((window_c *)window_v)->get_window();
        gtk_window_set_icon (window, icon_pixbuf);
    }
    // FIXME add to tab label (not here...)
}

void
view_c::set_application_icon (gint page_num) {
    GtkWidget *child_box = gtk_notebook_get_nth_page(notebook, page_num);
    view_c *view_p = (view_c *)g_object_get_data(G_OBJECT(child_box), "view_p");
    if (!view_p->get_xfdir_p()) return;
    
    const gchar *iconname = view_p->get_xfdir_p()->get_xfdir_iconname();
    GdkPixbuf *icon_pixbuf = gtk_p->get_pixbuf (iconname, GTK_ICON_SIZE_DIALOG);
    if(icon_pixbuf) {
	GtkWindow *window = ((window_c *)window_v)->get_window();
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

void
view_c::set_window_title(gint page_num){
    GtkWidget *child_box = gtk_notebook_get_nth_page(notebook, page_num);
    view_c *view_p = (view_c *)g_object_get_data(G_OBJECT(child_box), "view_p");
    if (!view_p->get_xfdir_p()) return;

    gchar *window_title = view_p->get_xfdir_p()->get_window_name();
    GtkWindow *window = GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET(notebook)));
    gtk_window_set_title (window, window_title);
    g_free (window_title);

}

void
view_c::harakiri(void){
    window_c *window_p = (window_c *)window_v;
    window_p->remove_view_from_list((void *)this);
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
    view_p->set_highlight(e->x, e->y);
    view_p->highlight();
    return FALSE;
}
static gboolean
leave_notify_event (GtkWidget *widget,
               GdkEvent  *event,
               gpointer   data){
    view_c * view_p = (view_c *)data;
    if (!data) g_error("leave_notify_event: data cannot be NULL\n");
    //fprintf(TRACE("leave_notify_event\n");
    view_p->clear_highlights(NULL);
}


static void 
on_remove_page_button(GtkWidget *page_label_button, gpointer data){
    view_c *view_p = (view_c *)data;
    view_p->remove_page();       
    // delete object: (remove from view_list)
    view_p->harakiri();
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
                GtkTreePath *path,
                gpointer     data)
{
    view_c *view_p = (view_c *)data;
    GtkTreeModel *tree_model = gtk_icon_view_get_model (iconview);
    GtkTreeIter iter;
    if (!gtk_tree_model_get_iter (tree_model, &iter, path)) return;
    
    gchar *ddname;
    gtk_tree_model_get (tree_model, &iter,
                          COL_ACTUAL_NAME, &ddname,-1);

    const gchar *dir = view_p->get_path();

    gchar *full_path = g_strconcat(dir, G_DIR_SEPARATOR_S, ddname, NULL);
    TRACE("dname = %s, path = %s\n", ddname, full_path);


    if (g_file_test(full_path, G_FILE_TEST_IS_DIR)){
        view_p->reload(full_path);
    } else {
        view_p->get_lpterm_p()->print_error(g_strdup_printf("%s: %s\n", full_path, strerror(ENOENT)));
    }
    g_free(ddname);
    g_free(full_path);
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
    gint current_page = gtk_notebook_get_current_page (notebook);
    view_c *view_p = (view_c *)data;
    TRACE("switch_page, page_num=%d current_page=%d xfdir_p=%p\n" ,new_page, current_page, view_p->get_xfdir_p());
    if (!view_p->get_xfdir_p()) return;
    view_p->set_window_title(new_page);
    view_p->set_application_icon(new_page);
}


