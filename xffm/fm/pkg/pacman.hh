#ifndef PACMAN_HH
#define PACMAN_HH
#define PKG_EMBLEM "emblem-archlinux"
#define PKG_EXEC "pacman"
#define PKG_SEARCH_LOCAL "pacman -Qs"
#define PKG_SEARCH "pacman -Ss"
#define PKG_LIST "pacman -Q"

namespace xf {
template <class Type>
class Pacman {

public:
    static void
    addDirectories(GtkTreeModel *treeModel){

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
        *strchr(line,'\n')=0;
        gchar **a = g_strsplit(line, " ", -1);
        if (!a[1]) {
            TRACE("add_pacman_item(): no vector...\n");
            g_strfreev(a);
            return pkg_list;
        }
        auto path = g_strdup(a[0]);
        TRACE("add_pacman_item(): a0=%s, version=%s\n",a[0], a[1]);
        
        pkg_list=g_list_prepend(pkg_list,path);
        g_strfreev(a);
        return pkg_list;
    }
    

};
}
#endif

