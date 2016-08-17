#ifndef XFDIR_C_HPP
#define XFDIR_C_HPP

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <gtk/gtk.h>
#include "gtk_c.hpp"
#include "utility_c.hpp"



//FIXME: by configure script
#define HAVE_STRUCT_DIRENT_D_TYPE
enum
{
  COL_PIXBUF,
  COL_DISPLAY_NAME,
  COL_ACTUAL_NAME,
  COL_ICON_NAME,
  NUM_COLS
};

#define SET_DIR(x) x|=0x01
#define IS_DIR (x&0x01)

typedef struct xd_t{
    gchar *d_name;
    unsigned char d_type;
}xd_t;

class xfdir_c: public gtk_c, public utility_c {
    public:
        xfdir_c(const gchar *);
        ~xfdir_c(void);
        GtkTreeModel *get_tree_model(void);
    private:
        GList *read_items (gint *heartbeat); 
        gint heartbeat;
	const gchar *get_type_pixbuf(xd_t *xd_p);
        gchar *path;
        GtkTreeModel *mk_tree_model(void);
        GtkTreeModel *treemodel;
};

#endif
