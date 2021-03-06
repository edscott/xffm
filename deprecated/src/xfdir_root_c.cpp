#include "util_c.hpp"
#include "view_c.hpp"
#include "xfdir_root_c.hpp"



xfdir_root_c::xfdir_root_c(void): 
    xfdir_c("xffm:root")
{
    treemodel = mk_tree_model();
}




GtkTreeModel *
xfdir_root_c::mk_tree_model (void)
{
    GtkTreeIter iter;
    path = g_strdup("xffm:root");  
    GtkListStore *list_store = gtk_list_store_new (BASIC_COLS, 
	    GDK_TYPE_PIXBUF, // icon in display
	    GDK_TYPE_PIXBUF, // normal icon reference
	    GDK_TYPE_PIXBUF, // highlight icon reference
	    GDK_TYPE_PIXBUF, // preview, tooltip image (cache)
	    G_TYPE_STRING,   // name in display (UTF-8)
	    G_TYPE_STRING,   // name from filesystem (verbatim)
	    G_TYPE_STRING,   // tooltip text (cache)
	    G_TYPE_STRING   // icon identifier (name or composite key)
	    ); // Preview pixbuf
            
    DBG("mk_tree_model:: model = %p\n", list_store);
    // Root
    const gchar *name = "/";
    gchar *utf_name = util_c::utf_string(_("Root Directory"));
    const gchar *icon_name = "folder/SE/emblem-root/3.0/180";
    const gchar *highlight_name = "document-open/SE/emblem-root/3.0/180";
    GdkPixbuf *normal_pixbuf;
    GdkPixbuf *highlight_pixbuf;
    normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  GTK_ICON_SIZE_DIALOG);
    highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  GTK_ICON_SIZE_DIALOG);   
    gtk_list_store_append (list_store, &iter);
    gtk_list_store_set (list_store, &iter, 
	    DISPLAY_NAME, utf_name,
	    ACTUAL_NAME, name,
	    ICON_NAME, icon_name,
	    DISPLAY_PIXBUF, normal_pixbuf,
	    NORMAL_PIXBUF, normal_pixbuf,
            HIGHLIGHT_PIXBUF, highlight_pixbuf,
            TOOLTIP_TEXT,_("This is the root of the filesystem"),

	    -1);
    g_free(utf_name);

    // Home
    name = g_get_home_dir();
    utf_name = util_c::utf_string(_("Home Directory"));
    icon_name = "folder/SE/emblem-home/3.0/180";
    highlight_name = "document-open/SE/emblem-home/3.0/180";
    normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  GTK_ICON_SIZE_DIALOG);
    highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  GTK_ICON_SIZE_DIALOG);   

    gtk_list_store_append (list_store, &iter);
    gtk_list_store_set (list_store, &iter, 
	    DISPLAY_NAME, utf_name,
	    ACTUAL_NAME, name,
	    ICON_NAME, icon_name,
	    DISPLAY_PIXBUF, normal_pixbuf,
	    NORMAL_PIXBUF, normal_pixbuf,
            HIGHLIGHT_PIXBUF, highlight_pixbuf,
            TOOLTIP_TEXT,_("This is the user's home directory"),
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

void
xfdir_root_c::item_activated (GtkIconView *iconview, GtkTreePath *tpath, void *data)
{
    view_c *view_p = (view_c *)data;
    GtkTreeModel *tree_model = gtk_icon_view_get_model (iconview);
    GtkTreeIter iter;
    if (!gtk_tree_model_get_iter (tree_model, &iter, tpath)) return;
    gchar *name;
    gtk_tree_model_get (tree_model, &iter, ACTUAL_NAME, &name,-1);
    view_p->reload(name);
    g_free(name);
}

