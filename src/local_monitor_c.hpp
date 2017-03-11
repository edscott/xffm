#ifndef LOCAL_MONITOR_C_HPP
#define LOCAL_MONITOR_C_HPP

typedef struct xd_t{
    gchar *d_name;
    unsigned char d_type;
    struct stat *st;
    gchar *mimetype;
    gchar *mimefile;
}xd_t;


#define MAX_AUTO_STAT 500

#define O_ALL(x) ((S_IROTH & x) && (S_IWOTH & x) &&  (S_IXOTH & x))
#define G_ALL(x) ((S_IRGRP & x) && (S_IWGRP & x) &&  (S_IXGRP & x))
#define U_ALL(x) ((S_IRUSR & x) && (S_IWUSR & x) &&  (S_IXUSR & x))
#define O_RX(x) ((S_IROTH & x) &&  (S_IXOTH & x))
#define G_RX(x) ((S_IRGRP & x) &&  (S_IXGRP & x))
#define U_RX(x) ((S_IRUSR & x) &&  (S_IXUSR & x))
#define O_RW(x) ((S_IROTH & x) && (S_IWOTH & x))
#define G_RW(x) ((S_IRGRP & x) && (S_IWGRP & x))
#define U_RW(x) ((S_IRUSR & x) && (S_IWUSR & x))
#define O_RX(x) ((S_IROTH & x) && (S_IXOTH & x))
#define G_RX(x) ((S_IRGRP & x) && (S_IXGRP & x))
#define U_RX(x) ((S_IRUSR & x) && (S_IXUSR & x))
#define O_R(x) (S_IROTH & x)
#define G_R(x) (S_IRGRP & x)
#define U_R(x) (S_IRUSR & x)
#define MY_FILE(x) (x == geteuid())
#define MY_GROUP(x) (x == getegid())



#include "xffm+.h"
#include "xfdir_c.hpp" // for treemodel column enum

class local_monitor_c: public xfdir_c, virtual utility_c {
    public:
        local_monitor_c(data_c *, const gchar *);
        ~local_monitor_c(void);
        GtkListStore *get_liststore(void);
        void add_local_item(GtkListStore *, xd_t *);
        void start_monitor(const gchar *, GtkTreeModel *);
	gchar *get_home_iconname(const gchar *);
        void free_xd_p(xd_t *);
        xd_t *get_xd_p(struct dirent *);
        GFile *get_gfile(void);
        gboolean add_new_item(GFile *);
        gboolean remove_item(GFile *);
        gboolean restat_item(GFile *);
        void destroy_tree_model(void);
    protected:
        void stop_monitor(void);
    private:
        xd_t *get_xd_p(GFile *);
        GCancellable *cancellable;
        GFile *gfile;
        GFileMonitor *monitor;
        GError *error;
        GtkListStore *store;
        gchar *get_iconname(xd_t *);
        gchar *get_iconname(xd_t *, gboolean);
        gchar *get_basic_iconname(xd_t *);
	gchar *get_emblem_string(xd_t *);
	gchar *get_emblem_string(xd_t *, gboolean);
        const gchar *get_mime_iconname(xd_t *);
        GHashTable *items_hash;

        
};

#endif
