#ifndef XFDIR_C_HPP
#define XFDIR_C_HPP

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <gtk/gtk.h>
#include "gtk_c.hpp"

//FIXME: by configure script
#define HAVE_STRUCT_DIRENT_D_TYPE
enum
{
  COL_DISPLAY_NAME,
  COL_PIXBUF,
  NUM_COLS
};
typedef struct xd_t{
    gchar *d_name;
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
    unsigned char d_type;
#endif
}xd_t;

class xfdir_c: gtk_c {
    public:
        GtkTreeModel *get_tree_model(const gchar *path);
    private:
        GSList *read_items (const gchar *path, gint *heartbeat); 
        gint heartbeat;
	GdkPixbuf *get_type_pixbuf(xd_t *xd_p);
};

#endif
