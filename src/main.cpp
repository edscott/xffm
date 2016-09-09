#include <gtk/gtk.h>
#include "xffm_c.hpp"
int
main (int argc, char **argv){
    xffm_c xffm(argc, argv);
    return xffm.run();
}

