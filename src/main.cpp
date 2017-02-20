#include <gtk/gtk.h>
#include "xffm_c.hpp"
int
main (int argc, char **argv){
    data_c *data_p = new data_c();
    xffm_c xffm(data_p, argc, argv);
    return xffm.run();
}

