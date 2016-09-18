
#include "xfdir_root_c.hpp"


xfdir_root_c::xfdir_root_c(const gchar *data, gtk_c *data_gtk_c): 
    xfdir_c(data, data_gtk_c)
{
    treemodel = mk_tree_model();
}


GtkTreeModel *
xfdir_root_c::mk_tree_model (void)
{
    GtkTreeIter iter;
    path = g_strdup("xffm:root");  
    GtkListStore *list_store = gtk_list_store_new (NUM_COLS, 
	    GDK_TYPE_PIXBUF, // icon
	    G_TYPE_INT,      // mode
	    G_TYPE_STRING,   // name (UTF-8)
	    G_TYPE_STRING,   // name (verbatim)
	    G_TYPE_STRING);   // icon_name
    DBG("mk_tree_model:: model = %p\n", list_store);
    // Root
    const gchar *name = "/";
    gchar *utf_name = utf_string(_("Root Directory"));
    const gchar *icon_name = "folder/SE/emblem-root/3.0/180";
        
    gtk_list_store_append (list_store, &iter);
    gtk_list_store_set (list_store, &iter, 
	    COL_DISPLAY_NAME, utf_name,
	    COL_ACTUAL_NAME, name,
	    COL_ICON_NAME, icon_name,
	    COL_MODE,0, 
	    COL_PIXBUF, gtk_p->get_pixbuf(icon_name,  GTK_ICON_SIZE_DIALOG),
	    -1);
    g_free(utf_name);

    // Home
    name = g_get_home_dir();
    utf_name = utf_string(_("Home Directory"));
    icon_name = "folder/SE/emblem-home/3.0/180";
        
    gtk_list_store_append (list_store, &iter);
    gtk_list_store_set (list_store, &iter, 
	    COL_DISPLAY_NAME, utf_name,
	    COL_ACTUAL_NAME, name,
	    COL_ICON_NAME, icon_name,
	    COL_MODE,0, 
	    COL_PIXBUF, gtk_p->get_pixbuf(icon_name,  GTK_ICON_SIZE_DIALOG),
	    -1);
    g_free(utf_name);

    return GTK_TREE_MODEL (list_store);
}

void 
xfdir_root_c::reload(const gchar *data){
    return;
}

const gchar *
xfdir_root_c::get_xfdir_iconname(void){
    return "emblem-rodent";
}


