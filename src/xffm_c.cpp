#include "xffm_c.hpp"
#include "window_c.hpp"
static void activate(GtkApplication *, void *);
static void startup(GtkApplication *, void *);  
static void shutdown(GtkApplication *, void *);
static void open (GtkApplication *, gpointer, gint, gchar *, gpointer);

xffm_c::xffm_c(gint in_argc, gchar **in_argv){
    gtk_p = NULL;
    argc = in_argc;
    argv = in_argv;
    app = gtk_application_new (NULL, G_APPLICATION_HANDLES_OPEN);

    window_p_list = NULL;
    g_signal_connect (app, "activate", G_CALLBACK (activate), (void *)this);
    g_signal_connect (app, "open", G_CALLBACK (open), (void *)this);
    g_signal_connect (app, "shutdown", G_CALLBACK (shutdown), (void *)this);
    g_signal_connect (app, "startup", G_CALLBACK (startup), (void *)this);
}

void
xffm_c::set_signal_menu_model(GMenuModel *data){
    signal_menu_model = data;
}

GMenuModel *
xffm_c::get_signal_menu_model(void){ return signal_menu_model;}

gint 
xffm_c::run(void){
    return g_application_run (G_APPLICATION (app), argc, argv);
};

    
xffm_c::~xffm_c(void){
    DBG("xffm_c::~xffm_c\n");
    GList *l = window_p_list;
    for (;l && l->data; l=l->next){
	window_c *window_p = (window_c *)l->data;
	delete window_p;
    }
    if (gtk_p) delete gtk_p;
}  

window_c * 
xffm_c::add_window_p(void){
    window_c *window_p = new window_c(app, gtk_p);
    if (!gtk_p) gtk_p = window_p->get_gtk_p();
    
    // initial view...
    window_p->create_new_page(g_get_home_dir());
    gtk_application_add_window (app, window_p->get_window());
    window_p_list = g_list_append(window_p_list, (void *)window_p);
    g_object_set_data(G_OBJECT(window_p->get_window()), "signal_menu_model", (void *)signal_menu_model);
    return window_p;
}

window_c * 
xffm_c::add_window_p(const gchar *data){
    window_c *window_p = new window_c(app, gtk_p);
    if (!gtk_p) gtk_p = window_p->get_gtk_p();
    // initial view...
    window_p->create_new_page(data);
    gtk_application_add_window (app, window_p->get_window());
    window_p_list = g_list_append(window_p_list, (void *)window_p);
    g_object_set_data(G_OBJECT(window_p->get_window()), "signal_menu_model", (void *)signal_menu_model);
    return window_p;
}

static void
activate(GtkApplication *app, void *data){
    DBG("activate(GtkApplication *app, void *data)\n");
    xffm_c *xffm_p = (xffm_c *)data;
    xffm_p->add_window_p();
}



static void
send_signal (GSimpleAction *action,
                       GVariant      *parameter,
                       gpointer       app)
{
    DBG("send_signal\n");
}

static void
process_info (GSimpleAction *action,
                       GVariant      *parameter,
                       gpointer       app)
{
    DBG("process_info\n");
}

static GActionEntry app_entries[] =
{
  { "send_signal", send_signal, NULL, NULL, NULL },
  { "process_info", process_info, NULL, NULL, NULL }
};

static void
startup(GtkApplication *app, void *data){
  xffm_c *xffm_p = (xffm_c *)data;  
  DBG("startup(GtkApplication *app, void *data)\n");
  GtkBuilder *builder;

  //G_APPLICATION_CLASS (menu_button_parent_class)->startup (application);

  g_action_map_add_action_entries (G_ACTION_MAP (app), app_entries, G_N_ELEMENTS (app_entries), app);
 
  builder = gtk_builder_new ();
  gtk_builder_add_from_string (builder,
"<interface>"
"  <menu id='signal-menu'>"
"    <section>"
"      <item>"
"        <attribute name='label' translatable='yes'>Signal</attribute>"
"        <attribute name='action'>app.send_signal</attribute>"
"      </item>"
"      <item>"
"        <attribute name='label' translatable='yes'>Information</attribute>"
"        <attribute name='action'>app.process_info</attribute>"
"      </item>"
"    </section>"
"  </menu>"
"</interface>", -1, NULL);
  GMenuModel *signal_menu_model = G_MENU_MODEL (gtk_builder_get_object (builder, "signal-menu"));
    xffm_p->set_signal_menu_model(signal_menu_model);

  gtk_application_set_app_menu (GTK_APPLICATION (app), G_MENU_MODEL (gtk_builder_get_object (builder, "signal-menu")));
  g_object_unref (builder);
}


static void
shutdown(GtkApplication *app, void *data){
  DBG("shutdown(GtkApplication *app, void *data)\n");
}

static gchar *
get_dirname(GFile *file){
    gchar *path = g_file_get_path(file);
    if (!g_file_test(path, G_FILE_TEST_IS_DIR)){
        gchar *g = g_path_get_dirname(path);
        g_free(path);
        path = g;
    }
    return path;
}

static void
open (GtkApplication *app,
               gpointer      files,
               gint          n_files,
               gchar        *hint,
               gpointer      data){
//    open(GtkApplication *app, void *data){
    DBG("open(GtkApplication *app, void *data)\n");
    xffm_c *xffm_p = (xffm_c *)data;
    if (!n_files){
	xffm_p->add_window_p();	
        return;
    }    
    GFile **array =(GFile **)files;
    gchar *path = get_dirname(array[0]);
    window_c *window_p = xffm_p->add_window_p(path);	
    g_free(path);
    gint i;  
    for (i=1; i<n_files; i++){
        // add extra notebook page
        path = get_dirname(array[i]);
        window_p->create_new_page(path);
        g_free(path);
    }
}

void
xffm_c::remove_window_p_from_list(void *data){
    window_p_list = g_list_remove(window_p_list, data);
}


