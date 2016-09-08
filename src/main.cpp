#include <gtk/gtk.h>
#include "xffm_c.hpp"

static void
activate(GtkApplication *app, void *data){
  fprintf(stderr, "activate(GtkApplication *app, void *data)\n");
  //xffm_c *xffm_p = new xffm_c(argv[1]);
  xffm_c *xffm_p = new xffm_c(app);
  //GtkWindow *window = xffm_p->get_window();
  //gtk_application_add_window (app, window);
}
static void
startup(GtkApplication *app, void *data){
  fprintf(stderr, "startup(GtkApplication *app, void *data)\n");
}
static void
shutdown(GtkApplication *app, void *data){
  fprintf(stderr, "shutdown(GtkApplication *app, void *data)\n");
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
    fprintf(stderr, "open(GtkApplication *app, void *data)\n");
    if (!n_files){
        xffm_c *xffm_p = new xffm_c(app);
        return;
    }    
    GFile **array =(GFile **)files;
    gchar *path = get_dirname(array[0]);
    xffm_c *xffm_p = new xffm_c(app, path);
    g_free(path);
    gint i;  
    for (i=1; i<n_files; i++){
        // add extra notebook page
        path = get_dirname(array[i]);
        xffm_p->create_new_page(path);
        g_free(path);
    }
}

int
main (int argc, char **argv){
  GtkApplication *app;
  int status;
  app = gtk_application_new ("org.xffm", G_APPLICATION_HANDLES_OPEN);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  g_signal_connect (app, "open", G_CALLBACK (open), (void *)argv[1]);
  g_signal_connect (app, "shutdown", G_CALLBACK (shutdown), NULL);
  g_signal_connect (app, "startup", G_CALLBACK (startup), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);
  return status;
}

/*
int main (int argc, char *argv[])
{
  gtk_init (&argc, &argv);
  xffm_c *xffm_p = new xffm_c(argv[1]);
  gtk_main ();
  return 0;
}
*/
