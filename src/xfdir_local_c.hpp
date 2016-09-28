#ifndef XFDIR_LOCAL_C_HPP
#define XFDIR_LOCAL_C_HPP
#include "xfdir_c.hpp"

typedef struct xd_t{
    gchar *d_name;
    unsigned char d_type;
    struct stat st;
    gchar *mimetype;
}xd_t;

class xfdir_local_c: public xfdir_c, virtual utility_c  {
    public:
	xfdir_local_c(const gchar *, gtk_c *);
        void reload(const gchar *);
	const gchar *get_xfdir_iconname(void);
        gchar *get_tip_text (const gchar *, GtkTreePath *);
    private:
        gchar *path_info(const gchar *, struct stat *, const gchar *);
        gint count_files (const gchar *);
        gint count_hidden_files (const gchar *);
        
        GtkTreeModel *mk_tree_model(void);
        gint heartbeat;
        GList *read_items (gint *); 
        void insert_list_into_model(GList *, GtkListStore *);

        gchar *get_iconname(xd_t *);
        gchar *get_iconname(xd_t *, gboolean);
        gchar *get_basic_iconname(xd_t *);
        const gchar *get_mime_iconname(xd_t *);
	gchar *get_emblem_string(xd_t *);
	gchar *get_emblem_string(xd_t *, gboolean);

	gchar *get_home_iconname(const gchar *);
        GList *sort_directory_list(GList *);
        
        gchar *user_string (struct stat *);
        gchar *group_string (struct stat *);
        gchar *mode_string (mode_t mode);
        gchar *date_string (time_t);
        gchar *sizetag (off_t, gint);

        pthread_mutex_t user_string_mutex;
        pthread_mutex_t group_string_mutex;
        pthread_mutex_t date_string_mutex;
};


#endif
