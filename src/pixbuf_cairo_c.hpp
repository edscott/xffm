#ifndef PIXBUF_CAIRO_C_HPP
#define PIXBUF_CAIRO_C_HPP
#include "xffm+.h"
#include <cairo.h>


class pixbuf_cairo_c {
    public:
        cairo_t *pixbuf_cairo_create( GdkPixbuf *pixbuf); 
        GdkPixbuf *pixbuf_cairo_destroy( cairo_t  *cr, GdkPixbuf *pixbuf);
        GdkPixbuf *create_pixbuf_mask(GdkPixbuf *in_pixbuf, guchar red, guchar green, guchar blue);
};

#endif
