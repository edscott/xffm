#ifndef XF_ROOTVIEW__HH
# define XF_ROOTVIEW__HH

namespace xf
{

template <class Type>
class RootView  {

    using pixbuf_c = Pixbuf<Type>;
    using util_c = Util<Type>;
public:

    static gboolean enableDragSource(void){ return FALSE;}
    static gboolean enableDragDest(void){ return FALSE;}

    static const gchar *
    get_xfdir_iconname(void){
	return "system-file-manager";
    }

 /*   static gchar *
    item_activated (GtkIconView *iconview, GtkTreePath *tpath, void *data)
    {
	// FIXME: should have a path column to 
	//        load localview items directly
	    DBG("RootView::item activated\n");
	GtkTreeModel *treeModel = gtk_icon_view_get_model (iconview);
	GtkTreeIter iter;
	if (!gtk_tree_model_get_iter (treeModel, &iter, tpath)) return NULL;
	gchar *name;
	gtk_tree_model_get (treeModel, &iter, ACTUAL_NAME, &name,-1);
	WARN("FIXME: load item iconview \"%s\"\n", name);
	//view_p->reload(name);
	g_free(name);
	return NULL;
    }*/


    static gboolean
    loadModel (GtkIconView *iconView)
    {
		
        g_object_set_data(G_OBJECT(iconView), "iconViewType", (void *)"RootView");
        auto treeModel = gtk_icon_view_get_model (iconView);
	DBG("mk_tree_model:: model = %p\n", treeModel);
        while (gtk_events_pending()) gtk_main_iteration();
 	GtkTreeIter iter;
	if (gtk_tree_model_get_iter_first (treeModel, &iter)){
	    while (gtk_list_store_remove (GTK_LIST_STORE(treeModel),&iter));
	}
        // Disable DnD
        //gtk_icon_view_unset_model_drag_source (iconView);
        //gtk_icon_view_unset_model_drag_dest (iconView);
        gtk_icon_view_set_selection_mode (iconView,GTK_SELECTION_SINGLE);      

	// Root
	const gchar *name = "/";
	gchar *utf_name = util_c::utf_string(_("Root Directory"));
	const gchar *icon_name = "folder/SE/system-file-manager-symbolic/3.0/180";
	const gchar *highlight_name = "document-open/SE/system-file-manager-symbolic/3.0/180";
	GdkPixbuf *normal_pixbuf;
	GdkPixbuf *highlight_pixbuf;
	normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  GTK_ICON_SIZE_DIALOG);
	highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  GTK_ICON_SIZE_DIALOG);   
	gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
	gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
		DISPLAY_NAME, utf_name,
		ACTUAL_NAME, name,
		ICON_NAME, icon_name,
                PATH, name,
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

	gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
	gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
		DISPLAY_NAME, utf_name,
		ACTUAL_NAME, name,
                PATH, name,
		ICON_NAME, icon_name,
		DISPLAY_PIXBUF, normal_pixbuf,
		NORMAL_PIXBUF, normal_pixbuf,
		HIGHLIGHT_PIXBUF, highlight_pixbuf,
		TOOLTIP_TEXT,_("This is the user's home directory"),
		-1);
	g_free(utf_name);

	return TRUE;
    }

};
}
#endif
