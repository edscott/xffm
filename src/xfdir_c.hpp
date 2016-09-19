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

class xfdir_c: virtual utility_c {
    public:
        xfdir_c(const gchar *, gtk_c *);
        ~xfdir_c(void);
        virtual void reload(const gchar *)=0;
	virtual const gchar *get_xfdir_iconname(void)=0;
        virtual void item_activated (GtkIconView *, GtkTreePath *, void *);

	
	gint get_dir_count(void);
        GtkTreeModel *get_tree_model(void);
        gint get_icon_size(const gchar *);
        gint get_icon_highlight_size(const gchar *);
        const gchar *get_label();
	gchar *get_window_name (void);
        const gchar *get_path(void);
    protected:
        virtual GtkTreeModel *mk_tree_model(void) = 0;
        GtkTreeModel *treemodel;
        gchar *path;
	gtk_c *gtk_p;
	gint dir_count;        
    private:
        pthread_mutex_t population_mutex;
        pthread_cond_t population_cond;
        gint population_condition;
        pthread_rwlock_t population_lock;

};

#endif
