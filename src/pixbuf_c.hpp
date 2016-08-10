#ifndef PIXBUF_C_HPP
#define PIXBUF_C_HPP
#include <gtk/gtk.h>
#include "pixbuf_hash_c.hpp"

class pixbuf_c: protected pixbuf_hash_c {
    public:
	GdkPixbuf *pixbuf_from_icon_name(const gchar *, gint);


};

#endif
