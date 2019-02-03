#ifndef PKG_HH
#define PKG_HH
#define PKG_EMBLEM "emblem-bsd"
#define PKG_EXEC "pkg"
#define PKG_SEARCH_LOCAL "pkg query -x %n"
#define PKG_SEARCH "pkg rquery -x %n"
#define PKG_LIST "pkg query %n"
#define PKG_SHORT_INFO "pkg rquery %c"

namespace xf {
template <class Type>
class Pkg {

public:

    static void
    addDirectories(GtkTreeModel *treeModel){
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

