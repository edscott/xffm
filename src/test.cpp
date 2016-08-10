#include <gtk/gtk.h>
#include "xffm_c.hpp"

int main (int argc, char *argv[])
{
  gtk_init (&argc, &argv);
  xffm_c *xffm_p = new xffm_c();
  gtk_main ();
  return 0;
}
