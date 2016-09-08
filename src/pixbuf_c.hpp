#ifndef PIXBUF_C_HPP
#define PIXBUF_C_HPP
#include "xffm+.h"

#include "pixbuf_hash_c.hpp"

class pixbuf_c : protected pixbuf_hash_c {
    public:
	GdkPixbuf *get_pixbuf(const gchar *, gint); // This will not return NULL
	GdkPixbuf *find_pixbuf(const gchar *, gint);// This will return NULL on failure
//	GdkPixbuf *pixbuf_new_from_icon_name(const gchar *, gint);
    protected:
    private:
        gint get_pixel_size(gint);
};

#endif
