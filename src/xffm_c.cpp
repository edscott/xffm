#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "xffm_c.hpp"
#include "window_c.hpp"
GtkApplication *xffm_c::app = NULL;

static void activate(GtkApplication *, void *);
static void startup(GtkApplication *, void *);  
static void shutdown(GtkApplication *, void *);
static void open (GtkApplication *, gpointer, gint, gchar *, gpointer);
//////////////////////////////////////////////////////////////////
xffm_c::xffm_c(gint in_argc, gchar **in_argv){

    argc = in_argc;
    argv = in_argv;
    app = gtk_application_new (NULL, G_APPLICATION_HANDLES_OPEN);
    GError *error=NULL;
    g_application_register (G_APPLICATION(app), NULL, &error);
    if (error){
        fprintf(stderr, "g_application_register: %s\n", error->message);
        throw 1;
    }


    
    g_signal_connect (app, "activate", G_CALLBACK (activate), (void *)this);
    g_signal_connect (app, "open", G_CALLBACK (open), (void *)this);
    g_signal_connect (app, "shutdown", G_CALLBACK (shutdown), (void *)this);
    g_signal_connect (app, "startup", G_CALLBACK (startup), (void *)this);
}
    
xffm_c::~xffm_c(void){
    DBG("xffm_c::~xffm_c\n");
    GList *l = gtk_application_get_windows(app);
    for (;l && l->data; l=l->next){
        GtkWindow *w = (GtkWindow *)l->data;
	window_c *window_p = (window_c *)g_object_get_data(G_OBJECT(l->data), "window_p");
        gtk_application_remove_window(app, w);
	delete window_p;
    }
}  

gint 
xffm_c::run(void){
    return g_application_run (G_APPLICATION (app), argc, argv);
};

window_c * 
xffm_c::add_window_p(void){
    return xffm_init(g_get_home_dir());
}

window_c * 
xffm_c::add_window_p(const gchar *data){
    return xffm_init(data);
}

window_c * 
xffm_c::xffm_init(const gchar *data){
    window_c *window_p = new window_c(app);
    // initial view...
    window_p->create_new_page(data);
    gtk_application_add_window (app, window_p->get_window());
    return window_p;
}

/////////////////////////////////////////////////////////////

static void
activate(GtkApplication *app, void *data){
    NOOP ("activate(GtkApplication *app, void *data)\n");
    xffm_c *xffm_p = (xffm_c *)data;
    xffm_p->add_window_p();
}


static void
startup(GtkApplication *app, void *data){
  xffm_c *xffm_p = (xffm_c *)data;  
  DBG("startup(GtkApplication *app, void *data)\n");
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
    NOOP ("open(GtkApplication *app, void *data)\n");
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

