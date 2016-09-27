#ifndef PIXBUF_CAIRO_C_HPP
#define PIXBUF_CAIRO_C_HPP
#include "xffm+.h"
#include <cairo.h>


class pixbuf_cairo_c {
    public:
        cairo_t *pixbuf_cairo_create( GdkPixbuf *); 
        GdkPixbuf *pixbuf_cairo_destroy( cairo_t  *, GdkPixbuf *);
        GdkPixbuf *create_pixbuf_mask(GdkPixbuf *, guchar , guchar , guchar );
        void add_color_pixbuf(cairo_t *, GdkPixbuf *, const gchar *);
        void insert_pixbuf_tag (cairo_t *, GdkPixbuf *, GdkPixbuf *, 
                const gchar *, const gchar *, const gchar *);
        void add_label_pixbuf(cairo_t *, GdkPixbuf *, const gchar *);
        
};

#endif
