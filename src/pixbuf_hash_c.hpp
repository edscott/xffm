#ifndef PIXBUF_HASH_C_HPP
#define PIXBUF_HASH_C_HPP
#include "xffm+.h"
#include "utility_c.hpp"


#define USER_XFFM_CACHE_DIR      g_get_user_cache_dir(),"xffm+"
#define XFFM_THUMBNAIL_DIR 	USER_XFFM_CACHE_DIR,"thumbnails"

class pixbuf_hash_c: virtual utility_c {
    public:
	pixbuf_hash_c(void);
	~pixbuf_hash_c(void);
        void zap_thumbnail_file(const gchar *, gint);
	
	GdkPixbuf *find_in_pixbuf_hash(const gchar *, gint);
	void put_in_pixbuf_hash(const gchar *, gint, const GdkPixbuf *);
        gchar *get_thumbnail_path (const gchar * file, gint size);

        

    protected:
    private:
	void rm_from_pixbuf_hash (const gchar *, gint);
        GHashTable *pixbuf_hash;
        gchar *get_hash_key (const gchar *, gint);
        GdkPixbuf *lookup_icon(const gchar *, gint);
};

#endif
