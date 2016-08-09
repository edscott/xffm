#include <gtk/gtk.h>
#include "window_c.hpp"
#include "view_c.hpp"

int main (int argc, char *argv[])
{
  GtkWidget *window;
   
  gtk_init (&argc, &argv);

  window_c *window_p = new window_c();
  view_c *view_p = new view_c(window_p);
   
  gtk_main ();
   
  return 0;
}
