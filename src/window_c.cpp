#include "intl.h"
#include "window_c.hpp"
#include "view_c.hpp"
#include "xfdir_c.hpp"

static void on_new_page(GtkWidget *, gpointer);


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

    notebook = GTK_NOTEBOOK(gtk_notebook_new());
    gtk_notebook_set_scrollable (notebook, TRUE);
    gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET(notebook));

    new_tab_button = gtk_button_new ();
    gtk_p->setup_image_button(new_tab_button, "list-add", _("Open a new tab (Ctrl+T)"));
    gtk_widget_show(new_tab_button);

    GtkWidget *button = gtk_button_new ();
    gtk_p->setup_image_button(button, "go-home", _("Home"));
    gtk_widget_show(button);



    gtk_notebook_set_action_widget (notebook, new_tab_button, GTK_PACK_END);
    gtk_notebook_set_action_widget (notebook, button, GTK_PACK_START);

    

    g_signal_connect(G_OBJECT(new_tab_button), "clicked", 
            G_CALLBACK(on_new_page), (void *)this); 

    gtk_widget_show (GTK_WIDGET(notebook));
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
    // XXX crash:
    delete ((view_c *)view_p);
    if (g_list_length(view_list) == 0) gtk_main_quit();
}

void 
window_c::set_up_view_signals(void *view){
//    view_c *view_p = (view_c *)view;
//    signals_p->setup_callback((void *)this, widget, "clicked", (void *)xxx, data); 
    
    // Delete button...

}

void 
window_c::create_new_page(const gchar *path){
    view_c *view_p = new view_c((void *)this, get_notebook());
    xfdir_c *xfdir_p = new xfdir_c(path);
    view_p->set_treemodel(xfdir_p);
    add_view_to_list((void *)view_p);
}

GtkWindow *
window_c::get_window(void){return GTK_WINDOW(window);}

GtkNotebook *window_c::get_notebook(void) {return GTK_NOTEBOOK(notebook);}

////////////////////////////////////////////////////////////////////////////

static void
on_new_page(GtkWidget *widget, gpointer data){
    // get current page
    // get path
    window_c *window_p = (window_c *)data;
    window_p->create_new_page(g_get_home_dir());
}

