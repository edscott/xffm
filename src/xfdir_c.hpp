#ifndef XFDIR_C_HPP
#define XFDIR_C_HPP

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <gtk/gtk.h>
#include "gtk_c.hpp"

class xfdir_c: gtk_c {
    public:
        GtkTreeModel *get_tree_model(const gchar *path);
    private:
        GSList *read_items (const gchar *path, gint *heartbeat); 
        gint heartbeat;

};

#endif
