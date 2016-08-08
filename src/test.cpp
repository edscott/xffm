#include <gtk/gtk.h>
#include "view_c.hpp"

int main (int argc, char *argv[])
{
  GtkWidget *window;
   
  gtk_init (&argc, &argv);
   
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
  gtk_window_set_title (GTK_WINDOW (window), "Simple class Example");
  gtk_container_set_border_width (GTK_CONTAINER (window), 0);
  gtk_widget_set_size_request (window, 800, 600);
  
  GtkWidget *notebook = gtk_notebook_new();
  gtk_container_add (GTK_CONTAINER (window), notebook);
  gtk_widget_show (notebook);

  view_c *view_p = new view_c(notebook);

  gtk_widget_show (window);
   
  gtk_main ();
   
  return 0;
}
