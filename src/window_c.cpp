//#define DEBUG_TRACE 1
#include "intl.h"
#include "window_c.hpp"
#include "view_c.hpp"
#include "xfdir_local_c.hpp"

static void on_new_page(GtkWidget *, gpointer);
static void on_go_home(GtkWidget *, gpointer);
static gboolean window_keyboard_event (GtkWidget *, GdkEventKey *, gpointer);
static void destroy(GtkWidget *, gpointer);
static gboolean window_tooltip_f(GtkWidget  *, gint, gint, gboolean, GtkTooltip *, gpointer);


window_c::window_c(data_c *data0):menu_c(data0) {
    data_p = data0;
    view_list=NULL;
    tt_window=NULL;
    tooltip_path_string = NULL;
    create_menu_model(data_p->get_app());
    add_actions(data_p->get_app());

    view_list_mutex = PTHREAD_MUTEX_INITIALIZER;
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    // keep tabs on the window_p from gtk_application...
    g_object_set_data(G_OBJECT(window), "window_p", (void *) this);
    
    gtk_widget_set_has_tooltip (window, TRUE);
    g_signal_connect (G_OBJECT (window), "query-tooltip", G_CALLBACK (window_tooltip_f), (void *)this);
    
    g_signal_connect (window, "key-press-event", G_CALLBACK (window_keyboard_event), (void *)this);
    gtk_window_set_title (GTK_WINDOW (window), "Xffm+");
    gtk_container_set_border_width (GTK_CONTAINER (window), 0);
    gtk_widget_set_size_request (window, 800, 600);

    notebook = GTK_NOTEBOOK(gtk_notebook_new());
    g_object_set_data(G_OBJECT(notebook), "window_p", (void *)this);
    
    gtk_notebook_set_scrollable (notebook, TRUE);

    gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET(notebook));
    
 /*
    GtkWidget *window_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *view_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET(window_box));
    gtk_box_pack_start(GTK_BOX(window_box), view_box, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(window_box), button_box, FALSE, FALSE, 0); 
    gtk_box_pack_end(GTK_BOX(view_box), GTK_WIDGET(notebook), TRUE, TRUE, 0);
    gtk_widget_show_all(window_box);
    */



    new_tab_button = gtk_button_new ();
    setup_image_button(new_tab_button, "list-add", _("Open a new tab (Ctrl+T)"));
    gtk_widget_show(new_tab_button);
/*
    GtkWidget *button = gtk_button_new ();
    setup_image_button(button, "go-home", _("Home"));
    gtk_widget_show(button);
*/
    GtkWidget *menu_button = gtk_menu_button_new ();
    GtkWidget *popover = gtk_popover_new_from_model (menu_button, signal_menu_model);
    gtk_menu_button_set_popover (GTK_MENU_BUTTON(menu_button), popover);
    gtk_widget_show(menu_button);

    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(button_box), new_tab_button,  FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), menu_button,  FALSE, FALSE, 0);
    gtk_widget_show(button_box);
    gtk_notebook_set_action_widget (notebook, button_box, GTK_PACK_END);

    /*
    gtk_notebook_set_action_widget (notebook, menu_button, GTK_PACK_END);
    gtk_notebook_set_action_widget (notebook, new_tab_button, GTK_PACK_END);
    */
    

    g_signal_connect(G_OBJECT(new_tab_button), "clicked", 
            G_CALLBACK(on_new_page), (void *)this); 
    //g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(on_go_home), (void *)this); 
    g_signal_connect (G_OBJECT(window), "destroy", G_CALLBACK (destroy), 
            (void *)this);


    gtk_widget_show (GTK_WIDGET(notebook));
    gtk_window_present (GTK_WINDOW (window));
    //gtk_widget_show (window);
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
}

data_c *
window_c::get_data_p(void){
    return data_p;
}

const gchar *
window_c::get_tooltip_path_string(void){
    return (const gchar *)tooltip_path_string;
}

void
window_c::set_tooltip_path_string(const gchar *data){
    g_free(tooltip_path_string);
    if (data){
        tooltip_path_string = g_strdup(data);
    } else {
        tooltip_path_string = NULL;
    }
}

void
window_c::set_tt_window(GtkWidget *data, const gchar *data2){
    if (data2) tooltip_path_string = g_strdup(data2);
    set_tooltip_path_string(data2);
    tt_window = data;
    gtk_widget_set_tooltip_window (window, GTK_WINDOW(tt_window));
    if (tt_window && G_IS_OBJECT (tt_window)) {
        g_object_set_data(G_OBJECT(tt_window), "tooltip_target", (void *)window);
    }
    
    return;
}

GtkWidget * 
window_c::get_tt_window(void){
    return tt_window;
}

    

void
window_c::add_view_to_list(void *view_p) {
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
        gtk_application_remove_window (data_p->get_app(), GTK_WINDOW(window));
    }
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
    // get current page and reload homedir
    view_c *view_p =(view_c *)get_active_view_p();
    view_p->reload(g_get_home_dir());
}

void 
window_c::create_new_page(const gchar *path){
    if (!path){
	g_warning("window_c::create_new_page path is NULL\n");
	return;
    }
    view_c *view_p = new view_c(data_p, (void *)this, get_notebook(), path);
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
/*
static void
on_go_home(GtkWidget *widget, gpointer data){
    window_c *window_p = (window_c *)data;
    window_p->go_home();
}
*/

// mod2 is numlock
// mod5 is alt-gr


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
    data_c *data_p = window_p->get_data_p();
    gtk_application_remove_window (data_p->get_app(), GTK_WINDOW(window_p->get_window()));
}

static gboolean
window_tooltip_f (GtkWidget  *widget,
               gint        x,
               gint        y,
               gboolean    keyboard_mode,
               GtkTooltip *tooltip,
               gpointer    data){
    window_c *window_p = (window_c *)data;
    GtkWidget *tt_window = window_p->get_tt_window();

    gtk_widget_set_tooltip_window (GTK_WIDGET(window_p->get_window()), GTK_WINDOW(tt_window));
    if (!tt_window) return FALSE;
    return TRUE;
}


void
window_c::create_menu_model(GtkApplication *app){
    GtkBuilder *builder;
    builder = gtk_builder_new ();
    const gchar *items[]={        
        N_("Open terminal"),
        N_("Execute Shell Command"),
        N_("Search"),
        N_("Exit"),
        NULL};
    gtk_builder_add_from_string (builder,
"<interface>"
"  <menu id='signal-menu'>"
"    <section>"
"      <item>"
"        <attribute name='label' translatable='yes'>Home</attribute>"
"        <attribute name='action'>app.home</attribute>"
"      </item>"
"      <item>"
"        <attribute name='label' translatable='yes'>Open terminal</attribute>"
"        <attribute name='action'>app.terminal</attribute>"
"        <attribute name='verb-icon'>terminal</attribute>"
"      </item>"
"      <item>"
"        <attribute name='label' translatable='yes'>Execute Shell Command</attribute>"
"        <attribute name='action'>app.shell</attribute>"
"      </item>"
"      <item>"
"        <attribute name='label' translatable='yes'>Search</attribute>"
"        <attribute name='action'>app.search</attribute>"
"      </item>"
"      <item>"
"        <attribute name='label' translatable='yes'>Exit</attribute>"
"        <attribute name='action'>app.finish</attribute>"
"      </item>"
"    </section>"
"  </menu>"
"</interface>", -1, NULL);
    signal_menu_model = G_MENU_MODEL (gtk_builder_get_object (builder, "signal-menu"));
    
    gtk_application_set_app_menu (GTK_APPLICATION (app), G_MENU_MODEL (gtk_builder_get_object (builder, "signal-menu")));
    g_object_unref (builder);
}

static void
 home(GSimpleAction *action,
                       GVariant      *parameter,
                       gpointer       data)
{
    DBG("home\n");
    window_c *window_p = (window_c *)data;
    window_p->go_home();

}

static void
 terminal(GSimpleAction *action,
                       GVariant      *parameter,
                       gpointer       app)
{
    DBG("terminal\n");
}

static void
 shell(GSimpleAction *action,
                       GVariant      *parameter,
                       gpointer       app)
{
    DBG("shell\n");
}

static void
 search(GSimpleAction *action,
                       GVariant      *parameter,
                       gpointer       app)
{
    DBG("Search\n");
}

static void
 finish(GSimpleAction *action,
                       GVariant      *parameter,
                       gpointer       app)
{
    DBG("finish\n");
    _exit(123);
}



void
window_c::add_actions(GtkApplication *app){
    GActionEntry app_entries[] =
    {
      { "home", home, NULL, NULL, NULL },
      { "terminal", terminal, NULL, NULL, NULL },
      { "shell", shell, NULL, NULL, NULL },
      { "search", search, NULL, NULL, NULL },
      { "finish",  finish, NULL, NULL, NULL }
    };
    g_action_map_add_action_entries (G_ACTION_MAP (app), app_entries, 
            G_N_ELEMENTS (app_entries), (gpointer) this);

}

