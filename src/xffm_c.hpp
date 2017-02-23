#ifndef XFFM_C_HPP
#define XFFM_C_HPP
#include "xffm+.h"
#include "data_c.hpp"
#include "window_c.hpp"

class xffm_c{
    public:
        xffm_c(data_c *, gint, gchar **);
	~xffm_c(void);
	window_c *add_window_p(void);
	window_c *add_window_p(const gchar *);
	gint run(void);
    protected:

    private:
	gint argc;
	gchar **argv;
        window_c *xffm_init(const gchar *);
	data_c *data_p;


};

#endif
