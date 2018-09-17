#ifndef PIXBUF_HASH_C_HPP
#define PIXBUF_HASH_C_HPP
#include "xffm+.h"

#define USER_XFFM_CACHE_DIR      g_get_user_cache_dir(),"xffm+"
#define XFFM_THUMBNAIL_DIR 	USER_XFFM_CACHE_DIR,"thumbnails"

class pixbuf_hash_c {
    public:
	static void init(void);
	~pixbuf_hash_c(void);
        static void zap_thumbnail_file(const gchar *, gint);
	
	static GdkPixbuf *find_in_pixbuf_hash(const gchar *, gint);
	static void put_in_pixbuf_hash(const gchar *, gint, const GdkPixbuf *);
        static gchar *get_thumbnail_path (const gchar * file, gint size);
        static GHashTable *pixbuf_hash;

    protected:
    private:
	static void free_pixbuf_tt(void *);
	static void rm_from_pixbuf_hash (const gchar *, gint);
        static gchar *get_hash_key (const gchar *, gint);
        static GdkPixbuf *lookup_icon(const gchar *, gint);

};
#endif
