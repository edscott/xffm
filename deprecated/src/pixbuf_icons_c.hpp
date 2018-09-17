#ifndef PIXBUF_ICONS_C_HPP
#define PIXBUF_ICONS_C_HPP
#include "xffm+.h"

#include "pixbuf_hash_c.hpp"
#include "pixbuf_cairo_c.hpp"


class pixbuf_icons_c {
    public:
        static void init(void);
        static GdkPixbuf *absolute_path_icon(const gchar *, gint);
        static GdkPixbuf *get_theme_pixbuf(const gchar *, gint);
        static gboolean is_composite_icon_name(const gchar *);
        

        static GdkPixbuf *composite_icon(const gchar *, gint);
        static GdkPixbuf *pixbuf_new_from_file (const gchar *, gint, gint);
        
        static GThread *self;
        static GtkIconTheme *icon_theme;
        static pthread_mutex_t pixbuf_mutex;

};


#endif
