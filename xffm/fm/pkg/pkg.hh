#ifndef PKG_HH
#define PKG_HH
#define PKG_EMBLEM "emblem-bsd"
#define PKG_EXEC "pkg"
#define PKG_LIST "pkg query %n"

#define PKG_SEARCH "pkg rquery -x %n"
#define PKG_SEARCH_LOCAL "pkg query -x %n"
#define PKG_COMMENT "pkg query %c"
#define PKG_REMOTE_COMMENT "pkg rquery %c"
#define PKG_STATLINE "pkg query %sh"
#define PKG_REMOTE_STATLINE "pkg rquery %sh"
#define PKG_VERSION "pkg query %v"
#define PKG_REMOTE_VERSION "pkg rquery %v"
#define PKG_TOOLTIPTEXT "pkg query %e"
#define PKG_REMOTE_TOOLTIPTEXT "pkg rquery %e"
#define PKG_WEB "pkg query %w"
#define PKG_REMOTE_WEB "pkg query %w"
#define PKG_GROUP "pkg query %o"
#define PKG_REMOTE_GROUP "pkg query %o"

#define PKG_INSTALL "sudo -A pkg install --yes"
#define PKG_INSTALL_DRYRUN "sudo -A pkg install --yes --dry-run"
#define PKG_UNINSTALL "sudo -A pkg delete --yes"
#define PKG_UNINSTALL_DRYRUN "sudo -A pkg delete --yes --dry-run"
#define PKG_FETCH "sudo -A pkg install --yes --fetch-only"

namespace xf {
template <class Type>
class Pkg {
    static void
    addCacheItem(GtkTreeModel *treeModel){
 	GtkTreeIter iter;
	// Home
	auto name = g_get_home_dir();
	auto icon_name = "folder/SE/" PKG_EMBLEM "/2.0/225";
	auto highlight_name = "folder/NE/document-open/2.0/225";
        auto treeViewPixbuf = Pixbuf<Type>::get_pixbuf(icon_name,  -24);
	auto normal_pixbuf = Pixbuf<Type>::get_pixbuf(icon_name,  -48);
	auto highlight_pixbuf = Pixbuf<Type>::get_pixbuf(highlight_name,  -48);   

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

    static void
    addPortsItem(GtkTreeModel *treeModel){
 	GtkTreeIter iter;
	// Home
	auto name = g_get_home_dir();
	auto icon_name = "folder/SE/" PKG_EMBLEM "/2.0/225";
	auto highlight_name = "folder/NE/document-open/2.0/225";
        auto treeViewPixbuf = Pixbuf<Type>::get_pixbuf(icon_name,  -24);
	auto normal_pixbuf = Pixbuf<Type>::get_pixbuf(icon_name,  -48);
	auto highlight_pixbuf = Pixbuf<Type>::get_pixbuf(highlight_name,  -48);   

	gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
	gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
		DISPLAY_NAME, "ports",
                PATH, "/usr/ports",
		TOOLTIP_TEXT,"/usr/ports",
                ICON_NAME, icon_name,
                TREEVIEW_PIXBUF, treeViewPixbuf, 
		DISPLAY_PIXBUF, normal_pixbuf,
		NORMAL_PIXBUF, normal_pixbuf,
		HIGHLIGHT_PIXBUF, highlight_pixbuf,
		-1);
    }

public:

    static void
    addDirectories(GtkTreeModel *treeModel){
        addPortsItem(treeModel);      
        addCacheItem(treeModel);   
    }

    static GList *
    addSearchItems(GList *pkg_list, const gchar *line){  
	if (!strchr(line,'\n')) return pkg_list;
	auto path = g_strdup(line);
	*(strchr(path,'\n'))=0;
	g_strstrip(path);
	pkg_list=g_list_prepend(pkg_list,path);
	return pkg_list;
    }

    static GList *
    addPackage(GList *pkg_list, const gchar *line)
    {
	if (!strchr(line,'\n')) return pkg_list;
	auto path = g_strdup(line);
	*(strchr(path,'\n'))=0;
	g_strstrip(path);
	pkg_list=g_list_prepend(pkg_list,path);
	return pkg_list;
    }

    static gchar *
    getShortInfo(const gchar *line)
    {
	TRACE("getShortInfo: %s\n", line);
	if (!strchr(line,'\n')) return NULL;
	auto path = g_strdup(line);
	*(strchr(path,'\n'))=0;
	g_strstrip(path);
	return path;
    }

};
}
#endif

