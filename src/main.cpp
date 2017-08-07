#include <gtk/gtk.h>
#include "xffm_c.hpp"
#include "mime_c.hpp"

int
main (int argc, char **argv){
    try {
        mime_c::init_hashes();
        xffm_c xffm(argc, argv);
        return xffm.run();
    }
    catch (int i) {
        std::cerr << "xffm+ reported error: " << i << std::endl;
        return 0;
    }
}

