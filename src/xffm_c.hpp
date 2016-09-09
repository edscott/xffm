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
	void remove_window_p_from_list(void *);
	
	gint run(void);
    private:
        gtk_c *gtk_p;
        GtkApplication *app;
	GList *window_p_list;
	gint argc;
	gchar **argv;

};

#endif
