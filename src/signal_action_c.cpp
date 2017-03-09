//#define DEBUG_TRACE 1
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include "signal_action_c.hpp"
#include "view_c.hpp"
#include "run_button_c.hpp"


signal_action_c::signal_action_c(data_c *data){
    create_menu();
    //add_actions();
    fprintf(stderr, "signal_action_c::menu is %p\n", menu);
}


