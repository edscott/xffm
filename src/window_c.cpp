#include "intl.h"
#include "window_c.hpp"
#include "view_c.hpp"
static void on_new_page(GtkWidget *, gpointer);
static void on_remove_page(GtkWidget *, gpointer);


window_c::window_c(void) {
    view_list_mutex = PTHREAD_MUTEX_INITIALIZER;
    signals_p = new signals_c();
    utility_p = new utility_c();
    gtk_p = new gtk_c();
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
    gtk_window_set_title (GTK_WINDOW (window), "Xffm+");
    gtk_container_set_border_width (GTK_CONTAINER (window), 0);
    gtk_widget_set_size_request (window, 800, 600);

    notebook = gtk_notebook_new();
    gtk_notebook_set_scrollable (GTK_NOTEBOOK(notebook), TRUE);
    gtk_container_add (GTK_CONTAINER (window), notebook);

    new_tab_child = gtk_p->new_add_page_tab(notebook, &new_tab_button);
    signals_p->setup_callback((void *)this, new_tab_button, 
          "clicked", (void *)on_new_page,NULL); 

    gtk_widget_show (notebook);
    gtk_widget_show (window);
}

window_c::~window_c(void) {
    GList *l;
    pthread_mutex_lock(&view_list_mutex);
    for (l=view_list; l && l->data; l=l->next){
        view_c *view_p = (view_c *)l->data;
        delete view_p;
    }
    pthread_mutex_unlock(&view_list_mutex);
    delete utility_p;
    delete gtk_p;
    delete signals_p;
}

gtk_c *
window_c::get_gtk_p(void){return gtk_p;}

void
window_c::add_view_to_list(void *view_p) {
    set_up_view_signals(view_p);
    pthread_mutex_lock(&view_list_mutex);
    view_list = g_list_prepend(view_list, view_p);
    pthread_mutex_unlock(&view_list_mutex);
}


void 
window_c::remove_view_from_list(void *view_p){
    // unset signals?
    pthread_mutex_lock(&view_list_mutex);
    view_list = g_list_remove(view_list, view_p);
    pthread_mutex_unlock(&view_list_mutex);
    // FIXME: crashes...
    // delete ((view_c *)view_p);
    if (g_list_length(view_list) == 0) gtk_main_quit();
}

void 
window_c::set_up_view_signals(void *view){
    view_c *view_p = (view_c *)view;
    GtkWidget *widget = view_p->get_page_label_button();
    void *data = (void *)view_p->get_page_child_box();
    signals_p->setup_callback((void *)this, widget, "clicked", (void *)on_remove_page, data); 
    
    // Delete button...

}


GtkWidget *window_c::get_notebook(void) {return notebook;}

GtkWidget *window_c::get_new_tab_child(void) {return new_tab_child;}

GtkWidget *window_c::get_new_tab_button(void) {return new_tab_button;}


////////////////////////////////////////////////////////////////////////////

static void
on_new_page(GtkWidget *widget, gpointer data){
    window_c *window_p = (window_c *)g_object_get_data(G_OBJECT(widget), "object");
    view_c *view_p = new view_c((void *)window_p,
            window_p->get_notebook(), window_p->get_new_tab_child());
    window_p->add_view_to_list((void *)view_p);
}

static void 
on_remove_page(GtkWidget *page_label_button, gpointer data){
    window_c *window_p = (window_c *)g_object_get_data(G_OBJECT(page_label_button), "object");
    view_c *view_p = (view_c *)g_object_get_data(G_OBJECT(page_label_button), "view_p");
    GtkWidget *page_child_box = (GtkWidget *)data;
    GtkWidget *notebook = window_p->get_notebook();
    gint page_num = gtk_notebook_page_num (GTK_NOTEBOOK(notebook), page_child_box);
    gtk_notebook_remove_page (GTK_NOTEBOOK(notebook), page_num);
    window_p->remove_view_from_list((void *)view_p);
}

