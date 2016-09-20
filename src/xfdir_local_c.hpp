#ifndef XFDIR_LOCAL_C_HPP
#define XFDIR_LOCAL_C_HPP
#include "xfdir_c.hpp"
#include "mime_c.hpp"

class xfdir_local_c: public xfdir_c, virtual utility_c, protected mime_c  {
    public:
	xfdir_local_c(const gchar *, gtk_c *);
        void reload(const gchar *);
	const gchar *get_xfdir_iconname(void);
    private:
        GtkTreeModel *mk_tree_model(void);
        gint heartbeat;
        GList *read_items (gint *heartbeat); 
        void insert_list_into_model(GList *, GtkListStore *);
	const gchar *get_home_iconname(const gchar *data);
	const gchar *get_type_pixbuf(xd_t *xd_p);
        const gchar *get_stat_pixbuf(xd_t *, gboolean);
        

};


#endif
