
#include "window_c.hpp"

window_c::window_c(void) {
  utility_p = new utility_c();
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
  gtk_window_set_title (GTK_WINDOW (window), "Xffm+");
  gtk_container_set_border_width (GTK_CONTAINER (window), 0);
  gtk_widget_set_size_request (window, 800, 600);
  
  notebook = gtk_notebook_new();
  gtk_container_add (GTK_CONTAINER (window), notebook);
  gtk_widget_show (notebook);
  gtk_widget_show (window);
}
window_c::~window_c(void) {
    delete utility_p;
}

GtkWidget *
window_c::get_notebook(void) {return notebook;}
