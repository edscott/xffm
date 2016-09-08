#ifndef XFFM_C_HPP
#define XFFM_C_HPP
#include "xffm+.h"
#include "window_c.hpp"

class xffm_c{
    public:
        xffm_c(GtkApplication *);
        xffm_c(GtkApplication *, const gchar *);
        ~xffm_c(void);
        GtkWindow *get_window(void);
        void create_new_page(const gchar *);
    private:
        window_c *window_p;
        GtkApplication *app;

};

#endif
