#ifndef VIEW_CALLBACKS_I
#define VIEW_CALLBACKS_I


///////////////////////////////////////////////////////////////////
//    simple gtk thread callbacks...
///////////////////////////////////////////////////////////////////

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

#endif
