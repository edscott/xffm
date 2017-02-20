#ifndef XFDIR_ROOT_C_HPP
#define XFDIR_ROOT_C_HPP
#include "xfdir_c.hpp"

class xfdir_root_c: public xfdir_c, virtual utility_c {
    public:
	xfdir_root_c(data_c *, const gchar *);
        void reload(const gchar *);
	const gchar *get_xfdir_iconname(void);

    private:
        GtkTreeModel *mk_tree_model(void);
};


#endif
