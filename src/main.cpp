#include <gtk/gtk.h>
#include "xffm_c.hpp"
int
main (int argc, char **argv){
    data_c *data_p = new data_c();
    try {
        xffm_c xffm(data_p, argc, argv);
        return xffm.run();
    }
    catch (int i) {
        std::cerr << "xffm+ reported error: " << i << std::endl;
        return 0;
    }
}

