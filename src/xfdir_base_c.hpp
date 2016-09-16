#ifndef XFDIR_C_HPP
#define XFDIR_C_HPP

#include "xffm+.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "gtk_c.hpp"
#include "utility_c.hpp"

enum
{
  COL_PIXBUF,
  COL_MODE,
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
    struct stat st;
}xd_t;

class xfdir_base_c: virtual utility_c {
    public:
        xfdir_base_c(const gchar *, gtk_c *);
        ~xfdir_base_c(void);
        GtkTreeModel *get_tree_model(void);
        void reload(const gchar *);
        gint get_icon_size(const gchar *);
        gint get_icon_highlight_size(const gchar *);
	gint get_dir_count(void);
        const gchar *get_label();
	gchar *get_window_name (void);
	const gchar *get_xfdir_iconname(void);
        const gchar *get_path(void);
    private:
        pthread_mutex_t population_mutex;
        pthread_cond_t population_cond;
        gint population_condition;
        pthread_rwlock_t population_lock;

	const gchar *get_home_iconname(const gchar *data);
        void insert_list_into_model(GList *, GtkListStore *);
        GList *read_items (gint *heartbeat); 
        gint heartbeat;
	const gchar *get_type_pixbuf(xd_t *xd_p);
        const gchar *get_stat_pixbuf(xd_t *, gboolean);
        gchar *path;
        GtkTreeModel *mk_tree_model(void);
        GtkTreeModel *treemodel;
	gint dir_count;
	gtk_c *gtk_p;
};

#endif
