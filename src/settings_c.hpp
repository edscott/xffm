#ifndef SETTINGS_C_HPP
#define SETTINGS_C_HPP

#include "xffm+.h"
#include "gtk_c.hpp"

class  settings_c: public gtk_c, virtual utility_c {
    public:
        settings_c(data_c *);
};

#endif
