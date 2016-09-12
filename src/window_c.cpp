//#define DEBUG_TRACE 1
#include "intl.h"
#include "window_c.hpp"
#include "view_c.hpp"
#include "xfdir_c.hpp"

static void on_new_page(GtkWidget *, gpointer);
static void on_go_home(GtkWidget *, gpointer);
static gboolean window_keyboard_event (GtkWidget *, GdkEventKey *, gpointer);
static void destroy(GtkWidget *, gpointer);


window_c::window_c(gtk_c *data) {
    view_list=NULL;
    if (data) {gtk_p = data;}
    else {throw 1;}

    view_list_mutex = PTHREAD_MUTEX_INITIALIZER;
    utility_p = new utility_c();
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect (window, "key-press-event", G_CALLBACK (window_keyboard_event), (void *)this);
    gtk_window_set_title (GTK_WINDOW (window), "Xffm+");
    gtk_container_set_border_width (GTK_CONTAINER (window), 0);
    gtk_widget_set_size_request (window, 800, 600);

    notebook = GTK_NOTEBOOK(gtk_notebook_new());
    g_object_set_data(G_OBJECT(notebook), "window_p", (void *)this);
    
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
    g_signal_connect(G_OBJECT(button), "clicked", 
            G_CALLBACK(on_go_home), (void *)this); 
    g_signal_connect (G_OBJECT(window), "destroy", G_CALLBACK (destroy), 
            (void *)this);


    gtk_widget_show (GTK_WIDGET(notebook));
    gtk_widget_show (window);
    while (gtk_events_pending()) gtk_main_iteration();
}

window_c::~window_c(void) {
    DBG("window_c::~window_c\n");
    GList *l;
    pthread_mutex_lock(&view_list_mutex);
    for (l=view_list; l && l->data; l=l->next){
        view_c *view_p = (view_c *)l->data;
        delete view_p;
    }
    pthread_mutex_unlock(&view_list_mutex);
    delete utility_p;
    //delete gtk_p;
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


gboolean
window_c::is_view_in_list(void *view_p) {
    pthread_mutex_lock(&view_list_mutex);
    gboolean result = GPOINTER_TO_INT(g_list_find(view_list, view_p));
    pthread_mutex_unlock(&view_list_mutex);
    return result;
}

void 
window_c::remove_view_from_list(void *view_p){
    TRACE("window_c::remove_view_from_list(%p)\n", (void *)view_p);
    // unset signals?
    pthread_mutex_lock(&view_list_mutex);
    view_list = g_list_remove(view_list, view_p);
    pthread_mutex_unlock(&view_list_mutex);
    delete ((view_c *)view_p);
    if (g_list_length(view_list) == 0) {
        gtk_application_remove_window (gtk_p->get_app(), GTK_WINDOW(window));
    }
}

void 
window_c::set_up_view_signals(void *view){
//    view_c *view_p = (view_c *)view;
//    signals_p->setup_callback((void *)this, widget, "clicked", (void *)xxx, data); 
    
    // Delete button...

}

void *
window_c::get_active_view_p(void){
    gint current_page = gtk_notebook_get_current_page (notebook);
    GtkWidget *child = gtk_notebook_get_nth_page (notebook, current_page);
    // get view_p
    return (view_c *)g_object_get_data(G_OBJECT(child), "view_p");
}

void 
window_c::go_home(void){
    //XXX this would need tweaking when xfdir_p is not a directory xfdir_p
    // get current page
    // reload
    view_c *view_p =(view_c *)get_active_view_p();
    view_p->reload(g_get_home_dir());
}

void 
window_c::create_new_page(const gchar *path){
    view_c *view_p = new view_c((void *)this, get_notebook());
    xfdir_c *xfdir_p = new xfdir_c(path, gtk_p);
    view_p->set_treemodel(xfdir_p);
    add_view_to_list((void *)view_p);
}

GtkWindow *
window_c::get_window(void){return GTK_WINDOW(window);}


GtkNotebook *window_c::get_notebook(void) {return GTK_NOTEBOOK(notebook);}

////////////////////////////////////////////////////////////////////////////
static void
on_new_page(GtkWidget *widget, gpointer data){
    window_c *window_p = (window_c *)data;
    // get current page
    view_c *view_p =(view_c *)window_p->get_active_view_p();
    // get and set path
    window_p->create_new_page(view_p->get_path());
}

static void
on_go_home(GtkWidget *widget, gpointer data){
    window_c *window_p = (window_c *)data;
    window_p->go_home();
}


// mod2 is numlock
// mod5 is alt-gr
//

static gboolean iconview_key(GdkEventKey * event){
    TRACE("iconview_key\n");
    return TRUE;
}

static gboolean
window_keyboard_event (GtkWidget * window, GdkEventKey * event, gpointer data)
{
    TRACE("window_keyboard_event\n");
    window_c *window_p = (window_c *)data;
    view_c *view_p = (view_c *)(window_p->get_active_view_p());
    return view_p->window_keyboard_event(event, (void *)view_p);
}


static void 
destroy(GtkWidget *window, void *data){
    window_c *window_p = (window_c *)data;
    gtk_application_remove_window (window_p->get_gtk_p()->get_app(), GTK_WINDOW(window_p->get_window()));
}

