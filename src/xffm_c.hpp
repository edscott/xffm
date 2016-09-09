#ifndef XFFM_C_HPP
#define XFFM_C_HPP
#include "xffm+.h"
#include "window_c.hpp"

class xffm_c{
    public:
        xffm_c(gint, gchar **);
	~xffm_c(void);
	window_c *add_window_p(void);
	window_c *add_window_p(const gchar *);
	gint run(void);
    private:
	GList *window_p_list;
        GtkApplication *app;
	gint argc;
	gchar **argv;

};

#endif
