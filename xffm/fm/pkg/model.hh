#ifndef XF_PKGMODEL__HH
# define XF_PKGMODEL__HH
//#ifdef HAVE_LIBXML2

namespace xf
{

template <class Type>
class PkgModel  {

    using pixbuf_c = Pixbuf<double>;
    using util_c = Util<double>;
public:


    static gboolean
    loadModel (GtkTreeModel *treeModel)
    {
	TRACE("mk_tree_model:: model = %p\n", treeModel);
        while (gtk_events_pending()) gtk_main_iteration();
 	GtkTreeIter iter;
	if (gtk_tree_model_get_iter_first (treeModel, &iter)){
	    while (gtk_list_store_remove (GTK_LIST_STORE(treeModel),&iter));
	}
	addXffmItem(treeModel);
#ifdef HAVE_PKG
        addPortsItem(treeModel);      
        addCacheItem(treeModel);      
#endif

	return TRUE;
    }


    static gboolean enableDragSource(void){ return FALSE;}
    static gboolean enableDragDest(void){ return FALSE;}

    static const gchar *
    get_xfdir_iconname(void){
        return "emblem-downloads/SE/" PKG_EMBLEM "/2.0/225";
    }
    static void
    addXffmItem(GtkTreeModel *treeModel){
 	GtkTreeIter iter;
	// Root
	auto name = "xffm:root";
	auto utf_name = util_c::utf_string(".");
	auto icon_name = "go-up";
	auto highlight_name = "go-up/NW/go-up-symbolic/2.0/225";
        auto treeViewPixbuf = Pixbuf<Type>::get_pixbuf(icon_name,  -24);
	auto normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  -48);
	auto highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  -48);   
	gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
	gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
		DISPLAY_NAME, utf_name,
		ICON_NAME, icon_name,
                PATH, name,
                TREEVIEW_PIXBUF, treeViewPixbuf, 
		DISPLAY_PIXBUF, normal_pixbuf,
		NORMAL_PIXBUF, normal_pixbuf,
		HIGHLIGHT_PIXBUF, highlight_pixbuf,
		TOOLTIP_TEXT,"xffm:root",

		-1);
	g_free(utf_name);
    }

    static void
    addPortsItem(GtkTreeModel *treeModel){
 	GtkTreeIter iter;
	// Home
	auto name = g_get_home_dir();
	auto icon_name = "folder/SE/" PKG_EMBLEM;
	auto highlight_name = "folder/NE/document-open/2.0/225";
        auto treeViewPixbuf = Pixbuf<Type>::get_pixbuf(icon_name,  -24);
	auto normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  -48);
	auto highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  -48);   

	gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
	gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
		DISPLAY_NAME, "ports",
                PATH, "/usr/ports",
                ICON_NAME, icon_name,
                TREEVIEW_PIXBUF, treeViewPixbuf, 
		DISPLAY_PIXBUF, normal_pixbuf,
		NORMAL_PIXBUF, normal_pixbuf,
		HIGHLIGHT_PIXBUF, highlight_pixbuf,
		TOOLTIP_TEXT,"/usr/ports",
		-1);
    }


    static void
    addCacheItem(GtkTreeModel *treeModel){
 	GtkTreeIter iter;
	// Home
	auto name = g_get_home_dir();
	auto icon_name = "folder/SE/" PKG_EMBLEM;
	auto highlight_name = "folder/NE/document-open/2.0/225";
        auto treeViewPixbuf = Pixbuf<Type>::get_pixbuf(icon_name,  -24);
	auto normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  -48);
	auto highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  -48);   

	gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
	gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
		DISPLAY_NAME, "cache",
                PATH, "/var/cache/pkg",
		ICON_NAME, icon_name,
                TREEVIEW_PIXBUF, treeViewPixbuf, 
		DISPLAY_PIXBUF, normal_pixbuf,
		NORMAL_PIXBUF, normal_pixbuf,
		HIGHLIGHT_PIXBUF, highlight_pixbuf,
		TOOLTIP_TEXT,"/var/cache/pkg",
		-1);
    }

//	auto name = "xffm:pkg";
//	auto utf_name = util_c::utf_string(_("Software Updater"));

 
  
};
}
#endif
