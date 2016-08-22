#define DEBUG_TRACE 1
#include "intl.h"
#include "window_c.hpp"
#include "view_c.hpp"
#include "xfdir_c.hpp"

static void on_new_page(GtkWidget *, gpointer);
static void on_go_home(GtkWidget *, gpointer);
static gboolean signal_keyboard_event (GtkWidget *, GdkEventKey *, gpointer);

window_c::window_c(void) {
    view_list_mutex = PTHREAD_MUTEX_INITIALIZER;
    signals_p = new signals_c();
    utility_p = new utility_c();
    gtk_p = new gtk_c();
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
    g_signal_connect (window, "key-press-event", G_CALLBACK (signal_keyboard_event), (void *)this);
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
    g_signal_connect(G_OBJECT(button), "clicked", 
            G_CALLBACK(on_go_home), (void *)this); 


    gtk_widget_show (GTK_WIDGET(notebook));
    gtk_widget_show (window);
    while (gtk_events_pending()) gtk_main_iteration();
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
    delete ((view_c *)view_p);
    if (g_list_length(view_list) == 0) gtk_main_quit();
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
    // get current page
    // get path
    window_c *window_p = (window_c *)data;
    window_p->create_new_page(g_get_home_dir());
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
signal_keyboard_event (
    GtkWidget * window,
    GdkEventKey * event,
    gpointer data
) {
    window_c *window_p = (window_c *)data;

    /* asian Input methods */
    if(event->keyval == GDK_KEY_space && (event->state & (GDK_MOD1_MASK | GDK_SHIFT_MASK))) {
        return FALSE;
    }


    gint ignore[]={
        GDK_KEY_Control_L,
        GDK_KEY_Control_R,
        GDK_KEY_Shift_L,
        GDK_KEY_Shift_R,
        GDK_KEY_Shift_Lock,
        GDK_KEY_Caps_Lock,
        GDK_KEY_Meta_L,
        GDK_KEY_Meta_R,
        GDK_KEY_Alt_L,
        GDK_KEY_Alt_R,
        GDK_KEY_Super_L,
        GDK_KEY_Super_R,
        GDK_KEY_Hyper_L,
        GDK_KEY_Hyper_R,
	GDK_KEY_ISO_Lock,
	GDK_KEY_ISO_Level2_Latch,
	GDK_KEY_ISO_Level3_Shift,
	GDK_KEY_ISO_Level3_Latch,
	GDK_KEY_ISO_Level3_Lock,
	GDK_KEY_ISO_Level5_Shift,
	GDK_KEY_ISO_Level5_Latch,
	GDK_KEY_ISO_Level5_Lock,
        0
    };

    gint i;
    for (i=0; ignore[i]; i++) {
        if(event->keyval ==  ignore[i]) {
	    TRACE("key ignored\n");
            return TRUE;
        }
    }


    view_c *view_p = (view_c *)(window_p->get_active_view_p());
    gboolean active = view_p->lp_get_active();
    TRACE("signal_keyboard_event(0x%x): view_p->get_active_lp = %d\n", event->keyval, active);

    if (!active && view_p->is_iconview_key(event)) return FALSE;


    if (!active) {
	if (event->keyval == GDK_KEY_Tab){
	    event->keyval = GDK_KEY_Escape;
	}
	if (event->keyval == GDK_KEY_Escape){
            view_p->lp_set_active(TRUE); 

        } 
        return TRUE;
    }
    else if (event->keyval == GDK_KEY_Escape){
        view_p->lp_set_active(FALSE);
        return TRUE;
    }
    // By now we have a lp key
    if (!active) view_p->lp_set_active(TRUE); 
    TRACE("send key to status dialog for lpterm command\n");
    return TRUE;



    /* FIXME: callbacks...
    if (rodent_do_callback(event->keyval, event->state)) {
        TRACE("signal_keyboard_event(): Tried callback with keyval!\n");
        return TRUE;
    } */
    
    /*
    if (!active){
	if (iconview_key(event)) {
            TRACE("signal_keyboard_event(): This may be a callback key!\n");
            return TRUE;
        }
    }

    if ((event->state & GDK_CONTROL_MASK) && !view_p->is_lpterm_key(event)) return TRUE;
*/
    /* FIXME
    if (view_p->selection_list) {
	// selection list must be redefined...
	update_reselect_list(widgets_p);
	TRACE( "Selection---> %p\n", view_p->selection_list);	
    } */
    
    //False defaults to status line keybinding signal callback
    return FALSE;
}
    

