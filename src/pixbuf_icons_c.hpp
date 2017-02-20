#ifndef PIXBUF_ICONS_C_HPP
#define PIXBUF_ICONS_C_HPP
#include "xffm+.h"

#include "pixbuf_hash_c.hpp"
#include "pixbuf_cairo_c.hpp"
#include "utility_c.hpp"


class pixbuf_icons_c: public pixbuf_cairo_c, public pixbuf_hash_c, virtual utility_c {
    public:
        pixbuf_icons_c(data_c *);
        ~pixbuf_icons_c(void);
        GdkPixbuf *absolute_path_icon(const gchar *, gint);
        GdkPixbuf *get_theme_pixbuf(const gchar *, gint);
        gboolean is_composite_icon_name(const gchar *);
        

    protected:
        GThread *self;
        void threadwait (void);
        GdkPixbuf *composite_icon(const gchar *, gint);
    private:
        void init_lite_hash (void);
        GdkPixbuf *pixbuf_new_from_file (const gchar *, gint, gint);
        GtkIconTheme *icon_theme;
        pthread_mutex_t pixbuf_mutex;

};


#endif
