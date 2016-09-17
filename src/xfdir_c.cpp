#include <iostream>

#include "xfdir_c.hpp"
using namespace std;



xfdir_c::xfdir_c(const gchar *data, gtk_c *data_gtk_c){
    gtk_p = data_gtk_c;
    path = g_strdup(data);

    gint result;
    population_mutex = PTHREAD_MUTEX_INITIALIZER;
    population_cond = PTHREAD_COND_INITIALIZER;
    result = pthread_rwlock_init(&population_lock, NULL);

    if (result){
        cerr << "view_c::init(): " << strerror(result) << "\n";
        throw 1;
    }
    population_condition = 0;

}

xfdir_c::~xfdir_c(void){
    g_free(path);
    g_object_unref(treemodel);
    pthread_mutex_destroy(&population_mutex);
    pthread_cond_destroy(&population_cond);
    pthread_rwlock_destroy(&population_lock);
}

const gchar *
xfdir_c::get_label(void){
    return get_path();
}

const gchar *
xfdir_c::get_path(void){return (const gchar *)path;}
GtkTreeModel *
xfdir_c::get_tree_model (void){return treemodel;}

gint
xfdir_c::get_dir_count(void){ return dir_count;}


gint 
xfdir_c::get_icon_size(const gchar *name){
    if (strcmp(name, "..")==0) return GTK_ICON_SIZE_DND;
    return GTK_ICON_SIZE_DIALOG;
}

gint 
xfdir_c::get_icon_highlight_size(const gchar *name){
    return GTK_ICON_SIZE_DIALOG;
}
gchar *
xfdir_c::get_window_name (void) {
    gchar *iconname;
    if(!path) {
        iconname = utf_string (g_get_host_name());
    } else if(g_path_is_absolute(path) &&
	    g_file_test (path, G_FILE_TEST_EXISTS)) {
        gchar *basename = g_path_get_basename (path);
        gchar *pathname = g_strdup (path);
        gchar *b = utf_string (basename);   // non chopped
        chop_excess (pathname);
        gchar *q = utf_string (pathname);   // non chopped

        g_free (basename);
        g_free (pathname);
	//iconname = g_strconcat (display_host, ":  ", b, " (", q, ")", NULL);
	iconname = g_strconcat (b, " (", q, ")", NULL);
        g_free (q);
        g_free (b);
    } else {
        iconname = utf_string (path);
        chop_excess (iconname);
    }

#ifdef DEBUG
    gchar *gg = g_strdup_printf("%s-%d-D", iconname, getpid());
    g_free(iconname);
    iconname = gg;
#else
#ifdef CORE
    gchar *gg = g_strdup_printf("%s-%d-C", iconname, getpid());
    g_free(iconname);
    iconname = gg;
#endif
#endif
    return (iconname);
}


