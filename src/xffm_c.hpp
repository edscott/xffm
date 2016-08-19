#ifndef XFFM_C_HPP
#define XFFM_C_HPP
#include "xffm+.h"
#include "window_c.hpp"

class xffm_c{
    public:
        xffm_c(void);
        ~xffm_c(void);
    private:
        window_c *window_p;
};

#endif
