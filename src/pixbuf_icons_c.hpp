#ifndef PIXBUF_ICONS_C_HPP
#define PIXBUF_ICONS_C_HPP
#include "xffm+.h"

#include "pixbuf_cairo_c.hpp"
#include "utility_c.hpp"


class pixbuf_icons_c: public pixbuf_cairo_c, virtual utility_c {
    public:
        pixbuf_icons_c(void);
        ~pixbuf_icons_c(void);
        GdkPixbuf *absolute_path_icon(const gchar *, gint);
        GdkPixbuf *get_theme_pixbuf(const gchar *, gint);
        gboolean is_composite_icon_name(const gchar *);
        

    protected:
        GThread *self;
        void threadwait (void);
	gboolean insert_pixbuf_tag (GdkPixbuf *, GdkPixbuf *, const gchar *, const gchar *, const gchar *);
    private:
        void init_lite_hash (void);
        GtkIconTheme *icon_theme;
        GdkPixbuf *pixbuf_new_from_file (const gchar *, gint, gint);
        pthread_mutex_t pixbuf_mutex;

};


#endif
