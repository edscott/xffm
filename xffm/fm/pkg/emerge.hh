#ifndef EMERGE_HH
#define EMERGE_HH
#define PKG_EMBLEM "emblem-gentoo"
#define PKG_EXEC "emerge"
#define PKG_LIST "xffm --fgr -R -i -a -t dir /var/db/pkg"

#define PKG_SEARCH "emerge --search"
#define PKG_SEARCH_LOCAL NULL
#define PKG_COMMENT NULL
#define PKG_REMOTE_COMMENT NULL
#define PKG_STATLINE NULL
#define PKG_REMOTE_STATLINE NULL
#define PKG_VERSION NULL
#define PKG_REMOTE_VERSION NULL
#define PKG_TOOLTIPTEXT "emerge --info"
#define PKG_REMOTE_TOOLTIPTEXT NULL
#define PKG_WEB NULL
#define PKG_REMOTE_WEB NULL
#define PKG_GROUP NULL
#define PKG_REMOTE_GROUP NULL

#define PKG_REFRESH "sudo -A emerge-webrsync"
#define PKG_INSTALL "sudo -A emerge --ask n --color y"
#define PKG_INSTALL_DRYRUN "sudo -A emerge --ask n --color y --pretend"
#define PKG_UNINSTALL "sudo -A emerge --unmerge --ask n --color y"
#define PKG_UNINSTALL_DRYRUN "sudo -A emerge --unmerge --ask n --color y --pretend"
#define PKG_FETCH "sudo -A emerge --ask n --fetchonly"

namespace xf {

template <class Type>
class Emerge {

    static gchar *chop_version(const gchar *package){
	if (!package) return NULL;
	gchar *q = g_strdup(package);
	gchar *p = strchr(q, '-');
	if (!p) return q;
	while (p && !isdigit(*(p+1))) p = strchr(p+1, '-');
	if (!p) return q;
	*p = 0;
	return q;
    }


    static void
    addPortageItem(GtkTreeModel *treeModel){
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
		DISPLAY_NAME, "portage",
                PATH, "/usr/portage",
		TOOLTIP_TEXT,"/usr/portage",
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
	addPortageItem(treeModel);
    }

    static GList *addPackage(GList *pkg_list, const gchar *line){
	if (!strchr(line,'\n')) return pkg_list;
	 TRACE("add_emerge_item:%s", line);

	gchar **a = g_strsplit(line, G_DIR_SEPARATOR_S, -1);
	if (!a[5]) {
	    g_strfreev(a);
	    return pkg_list;
	}
	//if (strchr(a[5],'\n'))*strchr(a[5],'\n')=0;
	TRACE("line=%s\na1=%s, a2=%s a3=%s a4=%s a5=%s\n ",line,a[1], a[2], a[3], a[4], a[5]?a[5]:"nil");

	auto g = chop_version(a[5]);
	auto path = g_strconcat(a[4], G_DIR_SEPARATOR_S, g, NULL);
	g_free(g);
	//auto path = g_strconcat(a[4], G_DIR_SEPARATOR_S, a[5], NULL);

	if (strchr(path,'\n')) *strchr(path,'\n')=0;
	TRACE("a5=%s, chopped=%s line=%s",a[5], path, line);
	
	pkg_list=g_list_prepend(pkg_list, path);
	g_strfreev(a);
	return pkg_list;
    }

    static GList *addSearchItems(GList *pkg_list, const gchar *line){
	gchar *tip = NULL;
	if (!strchr(line,'\n')) return pkg_list;
	if (*line =='*'){
	    const gchar *q = line+1;
	    while (*q==' ') q++;
	    auto path = g_strdup(q);
	    if (strchr(path,'\n')) *strchr(path,'\n')=0;
	    if (strstr(path, " [ Masked ]")) *(strstr(path, " [ Masked ]")) = 0;
	    pkg_list=g_list_prepend(pkg_list,path);

	}
	return pkg_list;
    }
};

}
#endif

