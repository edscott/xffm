#ifndef PIXBUF_CAIRO_C_HPP
#define PIXBUF_CAIRO_C_HPP
#include "xffm+.h"


class pixbuf_cairo_c {
    public:
        static cairo_t *pixbuf_cairo_create( GdkPixbuf *); 
        static GdkPixbuf *pixbuf_cairo_destroy( cairo_t  *, GdkPixbuf *);
        static GdkPixbuf *create_pixbuf_mask(GdkPixbuf *, guchar , guchar , guchar );
        static void add_color_pixbuf(cairo_t *, GdkPixbuf *, const gchar *);
        static void insert_pixbuf_tag (cairo_t *, GdkPixbuf *, GdkPixbuf *, 
                const gchar *, const gchar *, const gchar *);
        static void add_label_pixbuf(cairo_t *, GdkPixbuf *, const gchar *);
        
};

#endif
