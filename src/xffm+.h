#ifndef XFFM_PLUS_X
# define XFFM_PLUS_X
# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif

# include <gtk/gtk.h>
# include "intl.h"
# include "debug.h"
#define USER_RFM_CACHE_DIR      g_get_user_cache_dir(),"rfm"

typedef struct pixbuf_t {
    time_t mtime; // stat mtime info for thumbnails
    gint   size;  // pixbuf icon size
    off_t  st_size; // stat st_size for thumbnails
    ino_t  st_ino; // stat st_ino for thumbnails
    GdkPixbuf *pixbuf;
    union {
        gchar *mime_id;
        gchar *path;
    };
} pixbuf_t;


#endif

