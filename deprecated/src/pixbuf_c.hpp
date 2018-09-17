#ifndef PIXBUF_C_HPP
#define PIXBUF_C_HPP
#include "xffm+.h"

#include "pixbuf_icons_c.hpp"

class pixbuf_c {
    public:
//	GdkPixbuf *pixbuf_new_from_icon_name(const gchar *, gint);
	static GdkPixbuf *get_pixbuf(const gchar *, gint); // This will not return NULL
	static GdkPixbuf *find_pixbuf(const gchar *, gint);// This will return NULL on failure
        static gint get_pixel_size(gint);
};

#endif
