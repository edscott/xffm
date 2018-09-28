#ifndef XF_ROOTVIEW__HH
# define XF_ROOTVIEW__HH

#include "baseview.hh"

namespace xf
{

template <class Type>
class RootView: public BaseView<Type> {
    using pixbuf_c = Pixbuf<Type>;
    using util_c = Util<Type>;
public:

RootView(const gchar *path): 
    BaseView<Type>("xffm:root")
{
    this->treemodel_ = mk_tree_model();
    g_object_set_data(G_OBJECT(this->treemodel_), "iconview", this->iconView_);
    gtk_icon_view_set_model(this->iconView_, this->treemodel_);
    gtk_icon_view_set_text_column (this->iconView_, this->get_text_column());
    gtk_icon_view_set_pixbuf_column (this->iconView_,  this->get_icon_column());
    gtk_icon_view_set_selection_mode (this->iconView_, GTK_SELECTION_SINGLE);   
}



void 
reload(const gchar *data){
    return;
}

const gchar *
get_xfdir_iconname(void){
    return "system-file-manager";
}

void
item_activated (GtkIconView *iconview, GtkTreePath *tpath, void *data)
{
    //view_c *view_p = (view_c *)data;
    GtkTreeModel *tree_model = gtk_icon_view_get_model (iconview);
    GtkTreeIter iter;
    if (!gtk_tree_model_get_iter (tree_model, &iter, tpath)) return;
    gchar *name;
    gtk_tree_model_get (tree_model, &iter, ACTUAL_NAME, &name,-1);
    WARN("FIXME: load item iconview \"%s\"\n", name);
    //view_p->reload(name);
    g_free(name);
}

protected:


private:

GtkTreeModel *
mk_tree_model (void)
{
    GtkTreeIter iter;
   // path = g_strdup("xffm:root");  
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
    const gchar *icon_name = "folder/SE/system-file-manager-symbolic/3.0/180";
    const gchar *highlight_name = "document-open/SE/system-file-manager-symbolic/3.0/180";
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
    icon_name = "folder/SE/user-home-symbolic/3.0/180";
    highlight_name = "document-open/SE/user-home-symbolic/3.0/180";
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

};
}
#endif
