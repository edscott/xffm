#ifndef PIXBUF_HASH_C_HPP
#define PIXBUF_HASH_C_HPP
#include "xffm+.h"

#include "pixbuf_icons_c.hpp"

#define USER_XFFM_CACHE_DIR      g_get_user_cache_dir(),"xffm+"
#define XFFM_THUMBNAIL_DIR 	USER_XFFM_CACHE_DIR,"thumbnails"

class pixbuf_hash_c: public pixbuf_icons_c, virtual utility_c {
    public:
	pixbuf_hash_c(void);
	~pixbuf_hash_c(void);
        void zap_thumbnail_file(const gchar *, gint);

        void add_color_pixbuf(cairo_t *, GdkPixbuf *, const gchar *);
        void insert_pixbuf_tag (cairo_t *, const gchar *, GdkPixbuf *, 
                const gchar *, const gchar *, const gchar *);
        void add_label_pixbuf(cairo_t *, GdkPixbuf *, const gchar *);
        

    protected:
	GdkPixbuf *find_in_pixbuf_hash(const gchar *, gint);
	GdkPixbuf *find_in_pixbuf_hash(const gchar *, gint, gboolean);
    private:
	void rm_from_pixbuf_hash (const gchar *, gint);
	void put_in_pixbuf_hash(const gchar *, gint, const GdkPixbuf *);
        gchar *get_thumbnail_path (const gchar * file, gint size);
        GHashTable *pixbuf_hash;
        gchar *get_hash_key (const gchar *, gint);
        GdkPixbuf *lookup_icon(const gchar *, gint);
        GdkPixbuf *composite_icon(const gchar *, gint);
};

#endif
