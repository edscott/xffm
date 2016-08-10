#ifndef PIXBUF_HASH_C_HPP
#define PIXBUF_HASH_C_HPP
#include <gtk/gtk.h>
#include "window_c.hpp"

class pixbuf_hash_c {
    public:
	pixbuf_hash_c(window_c *);
	~pixbuf_hash_c(void);

    protected:
	GdkPixbuf *find_in_pixbuf_hash(const gchar *, gint);
	void rm_from_pixbuf_hash (const gchar *, gint);
	void put_in_pixbuf_hash(const gchar *, gint, const GdkPixbuf *);
	void zap_thumbnail_file(const gchar *, gint);
    private:
       GHashTable *pixbuf_hash;
       window_c *window_p;

};

#endif
