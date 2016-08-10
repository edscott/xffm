#ifndef XFFM_C_HPP
#define XFFM_C_HPP
#include "window_c.hpp"
#include "view_c.hpp"
#include "signals_c.hpp"

class xffm_c{
    public:
        xffm_c(void);
        ~xffm_c(void);
        window_c *window_p;
        void add_view_to_list(view_c *view_p);
    private:
        GList *view_list;
        signals_c *signals_p;
};

#endif
