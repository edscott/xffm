#ifndef PACMAN_HH
#define PACMAN_HH
#define PKG_EMBLEM "emblem-archlinux"
#define PKG_EXEC "pacman"
#define PKG_LIST "pacman -Q"

#define PKG_SEARCH "pacman -Ss"
#define PKG_SEARCH_LOCAL "pacman -Qs"
#define PKG_COMMENT NULL
#define PKG_REMOTE_COMMENT "pacman -Ss"
#define PKG_STATLINE NULL
#define PKG_REMOTE_STATLINE NULL
#define PKG_VERSION NULL
#define PKG_REMOTE_VERSION NULL
#define PKG_TOOLTIPTEXT "man pacman"
#define PKG_REMOTE_TOOLTIPTEXT NULL
#define PKG_WEB NULL
#define PKG_REMOTE_WEB NULL
#define PKG_GROUP NULL
#define PKG_REMOTE_GROUP NULL

#define PKG_REFRESH "sudo -A pacman -Syy && sudo -A pacman -S --noconfirm archlinux-keyring"
#define PKG_INSTALL "sudo -A pacman --noconfirm --color always --noprogressbar -S"
#define PKG_INSTALL_DRYRUN "sudo -A pacman  --print -S"
#define PKG_UNINSTALL "sudo -A pacman --noconfirm --color always --noprogressbar -R"
#define PKG_UNINSTALL_DRYRUN "sudo -A pacman --print -R"
#define PKG_FETCH "sudo -A pacman -Sw"

namespace xf {
template <class Type>
class Pacman {
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
                PATH, "/var/cache/pacman/pkg",
		ICON_NAME, icon_name,
                TREEVIEW_PIXBUF, treeViewPixbuf, 
		DISPLAY_PIXBUF, normal_pixbuf,
		NORMAL_PIXBUF, normal_pixbuf,
		HIGHLIGHT_PIXBUF, highlight_pixbuf,
		TOOLTIP_TEXT,"/var/cache/pkg",
		-1);
    }

public:
    static void
    addDirectories(GtkTreeModel *treeModel){
        addCacheItem(treeModel);   

    }

    static GList *addSearchItems(GList *pkg_list, const gchar *line){
        if (!strchr(line,'\n')) return pkg_list;
        if (*line != ' '){
            gchar **a = g_strsplit(line, " ", -1);
            // check a
            gchar *p = strchr(a[0], '/');
            // check p
            p++;
            auto path = g_strdup(p);
            pkg_list=g_list_prepend(pkg_list,path);
            g_strfreev(a);
        } else {
            //the rest is tooltip material   
         /*   record_entry_t *en = pkg_list->data;
            gchar *tip = g_hash_table_lookup(installed_hash, en->path);
            gchar *new_tip = g_strconcat ((tip)?tip:"", line, NULL);
            g_hash_table_replace(installed_hash, g_strdup(en->path), new_tip);*/
        }
        return pkg_list;
    }
    
    static GList *addPackage(GList *pkg_list, const gchar *line){
        if (!strchr(line,'\n')) return pkg_list;
        TRACE("add_pacman_item(): %s", line);
        gchar **a = g_strsplit(line, " ", -1);
        if (!a[1]) {
            TRACE("add_pacman_item(): no vector...\n");
            g_strfreev(a);
            return pkg_list;
        }
        auto path = g_strdup(a[0]);
        if (strchr(path,'\n')) *strchr(path,'\n')=0;
        
        TRACE("add_pacman_item(): a0=%s, version=%s\n",path, a[1]);
        
        pkg_list=g_list_prepend(pkg_list,path);
        g_strfreev(a);
        return pkg_list;
    }
    

};
}
#endif

