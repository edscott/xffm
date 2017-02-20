#ifndef PIXBUF_C_HPP
#define PIXBUF_C_HPP
#include "xffm+.h"

#include "pixbuf_icons_c.hpp"

class pixbuf_c : protected pixbuf_icons_c {
    public:
	pixbuf_c(data_c *);
//	GdkPixbuf *pixbuf_new_from_icon_name(const gchar *, gint);
	GdkPixbuf *get_pixbuf(const gchar *, gint); // This will not return NULL
	GdkPixbuf *find_pixbuf(const gchar *, gint);// This will return NULL on failure
    protected:
    private:
        gint get_pixel_size(gint);
};

#endif
