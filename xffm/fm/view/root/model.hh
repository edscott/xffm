#ifndef XF_ROOTMODEL__HH
# define XF_ROOTMODEL__HH
#include "bookmarks.hh"


namespace xf
{

template <class Type> class Emerge;
template <class Type> class Pacman;
template <class Type> class Pkg;
template <class Type>
class RootModel {

    using util_c = Util<double>;
public:


    static gboolean
    loadModel (GtkTreeModel *treeModel)
    {
        TRACE("XXX: %s\n", "this is a test");
        TRACE("XXX: %s\n", "this is a test");
        TRACE("XXX: %s\n", "this is a test");
        Bookmarks<Type>::initBookmarks();
        TRACE("mk_tree_model:: model = %p\n", treeModel);
        while (gtk_events_pending()) gtk_main_iteration();
         GtkTreeIter iter;
        if (gtk_tree_model_get_iter_first (treeModel, &iter)){
            while (gtk_list_store_remove (GTK_LIST_STORE(treeModel),&iter));
        }
        // This is always in pathbar:  addRootItem(treeModel);
        addHomeItem(treeModel);
#ifdef ENABLE_FSTAB_MODULE
        addFstabItem(treeModel);
#ifdef ENABLE_EFS_MODULE
        addEfsItem(treeModel);
#endif
#endif
 #ifdef ENABLE_PKG_MODULE
        PkgModel<Type>::addPkgItem(treeModel);
#endif
        addTrashItem(treeModel);
        addLocalBookmarks(treeModel);

        return TRUE;
    }


    static gboolean enableDragSource(void){ return FALSE;}
    static gboolean enableDragDest(void){ return FALSE;}

    static const gchar *
    get_xfdir_iconname(void){
        return FILE_MANAGER;
    }


    static void
    addEfsItem(GtkTreeModel *treeModel){
         GtkTreeIter iter;
        auto items = EFS<Type>::getSavedItems();
        for (auto p=items; p && *p; p++){
            auto path = g_strconcat("efs:/", *p, NULL);
            auto basename = g_path_get_basename(*p);
            auto utf_name = Util<Type>::utf_string(basename);
            g_free(basename);
            const gchar *ball = NULL;
            if (!g_file_test(*p, G_FILE_TEST_IS_DIR))ball = "/NE/dialog-error/3.0/220";
            else {
                TRACE("model.hh::addEfsItem()...\n"); 
                if (FstabView<Type>::isMounted(*p))ball = "/NE/greenball/3.0/220";
                else ball = "/NE/grayball/3.0/220";
            }
            auto icon_name = g_strconcat(EFS_ICON, ball, NULL);

            auto highlight_name = g_strconcat(icon_name, "/", HIGHLIGHT_JUMP, NULL);
            auto treeViewPixbuf = Pixbuf<Type>::getPixbuf(icon_name,  -24);
            auto normal_pixbuf = Pixbuf<Type>::getPixbuf(icon_name,  -48);
            auto highlight_pixbuf = Pixbuf<Type>::getPixbuf(highlight_name,  -48);   

            gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
            gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
                    DISPLAY_NAME, utf_name,
                    PATH, path,
                    ICON_NAME, icon_name,
                    TREEVIEW_PIXBUF, treeViewPixbuf, 
                    DISPLAY_PIXBUF, normal_pixbuf,
                    NORMAL_PIXBUF, normal_pixbuf,
                    HIGHLIGHT_PIXBUF, highlight_pixbuf,
                    TOOLTIP_TEXT,g_get_home_dir(),
                    -1);
            g_free(path);
            g_free(utf_name);
            g_free(icon_name);
            g_free(highlight_name);
        }
        g_strfreev(items);
    }
    
    static void
    addXffmItem(GtkTreeModel *treeModel){
         GtkTreeIter iter;
        // Root
        auto name = "xffm:root";
        auto utf_name = util_c::utf_string(".");
        auto icon_name = GO_UP;
        auto highlight_name = GO_UP "/NW/" GO_UP_SYMBOLIC "/2.0/225";

        auto treeViewPixbuf = Pixbuf<Type>::getPixbuf(icon_name,  -24);
        auto normal_pixbuf = Pixbuf<Type>::getPixbuf(icon_name,  -48);
        auto highlight_pixbuf = Pixbuf<Type>::getPixbuf(highlight_name,  -48);   
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
    addRootItem(GtkTreeModel *treeModel){
         GtkTreeIter iter;
        // Root
        auto name = "/";
        auto utf_name = util_c::utf_string(_("Root Directory"));
        auto icon_name = FILE_MANAGER;
        auto highlight_name = g_strconcat(icon_name, "/", HIGHLIGHT_JUMP, NULL);

        auto treeViewPixbuf = Pixbuf<Type>::getPixbuf(icon_name,  -24);
        auto normal_pixbuf = Pixbuf<Type>::getPixbuf(icon_name,  -48);
        auto highlight_pixbuf = Pixbuf<Type>::getPixbuf(highlight_name,  -48);   
        gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
        gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
                DISPLAY_NAME, utf_name,
                ICON_NAME, icon_name,
                PATH, name,
                TREEVIEW_PIXBUF, treeViewPixbuf, 
                DISPLAY_PIXBUF, normal_pixbuf,
                NORMAL_PIXBUF, normal_pixbuf,
                HIGHLIGHT_PIXBUF, highlight_pixbuf,
                TOOLTIP_TEXT,_("This is the root of the filesystem"),

                -1);
        g_free(utf_name);
        g_free(highlight_name);
    }

    static void
    addHomeItem(GtkTreeModel *treeModel){
         GtkTreeIter iter;
        // Home
        auto name = g_get_home_dir();
        auto utf_name = util_c::utf_string(_("Home Directory"));
        auto icon_name = HOME_ICON;
        auto highlight_name = g_strconcat(icon_name, "/", HIGHLIGHT_JUMP, NULL);
        auto treeViewPixbuf = Pixbuf<Type>::getPixbuf(icon_name,  -24);
        auto normal_pixbuf = Pixbuf<Type>::getPixbuf(icon_name,  -48);
        auto highlight_pixbuf = Pixbuf<Type>::getPixbuf(highlight_name,  -48);   

        gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
        gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
                DISPLAY_NAME, utf_name,
                PATH, name,
                ICON_NAME, icon_name,
                TREEVIEW_PIXBUF, treeViewPixbuf, 
                DISPLAY_PIXBUF, normal_pixbuf,
                NORMAL_PIXBUF, normal_pixbuf,
                HIGHLIGHT_PIXBUF, highlight_pixbuf,
                TOOLTIP_TEXT,g_get_home_dir(),
                -1);
        g_free(utf_name);
        g_free(highlight_name);
    }

    static void
    addTrashItem(GtkTreeModel *treeModel){
         GtkTreeIter iter;
        // Home
        auto name = g_strdup_printf("%s/.local/share/Trash/files", g_get_home_dir());
        auto utf_name = util_c::utf_string(_("Trash bin"));
        auto icon_name = TRASH_ICON;
        auto trash = g_build_filename(g_get_home_dir(), ".local/share/Trash", NULL);
        if (Thread<Type>::fileTest(trash, G_FILE_TEST_EXISTS)){
	    icon_name =TRASH_FULL_ICON;
	}
            
        auto highlight_name = g_strconcat(icon_name, "/", HIGHLIGHT_JUMP, NULL);

        auto treeViewPixbuf = Pixbuf<Type>::getPixbuf(icon_name,  -24);
        auto normal_pixbuf = Pixbuf<Type>::getPixbuf(icon_name,  -48);
        auto highlight_pixbuf = Pixbuf<Type>::getPixbuf(highlight_name,  -48);   

        gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
        gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
                DISPLAY_NAME, utf_name,
                PATH, name,
                ICON_NAME, icon_name,
                TREEVIEW_PIXBUF, treeViewPixbuf, 
                DISPLAY_PIXBUF, normal_pixbuf,
                NORMAL_PIXBUF, normal_pixbuf,
                HIGHLIGHT_PIXBUF, highlight_pixbuf,
                TOOLTIP_TEXT,g_get_home_dir(),
                -1);
        g_free(name);
        g_free(utf_name);
        g_free(highlight_name);
    }

    static void
    addFstabItem(GtkTreeModel *treeModel){        
         GtkTreeIter iter;
        auto name = "xffm:fstab";
        auto utf_name = util_c::utf_string(_("Disk Image Mounter"));
        auto icon_name = FSTAB_ICON;
        auto highlight_name = g_strconcat(icon_name, "/", HIGHLIGHT_JUMP, NULL);

        auto treeViewPixbuf = Pixbuf<Type>::getPixbuf(icon_name,  -24);
        auto normal_pixbuf = Pixbuf<Type>::getPixbuf(icon_name,  GTK_ICON_SIZE_DIALOG);
        auto highlight_pixbuf = Pixbuf<Type>::getPixbuf(highlight_name,  GTK_ICON_SIZE_DIALOG); 

        gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
        gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
                DISPLAY_NAME, utf_name,
                PATH, name,
                ICON_NAME, icon_name,
                TREEVIEW_PIXBUF, treeViewPixbuf, 
                DISPLAY_PIXBUF, normal_pixbuf,
                NORMAL_PIXBUF, normal_pixbuf,
                HIGHLIGHT_PIXBUF, highlight_pixbuf,
                TOOLTIP_TEXT,_("Mount local disks and devices"),
                -1);
        g_free(utf_name);
        g_free(highlight_name);
    }

    static void
    addLocalBookmarks(GtkTreeModel *treeModel){
         GtkTreeIter iter;
        auto list = Bookmarks<Type>::bookmarksList();
        for (auto l=list; l && l->data; l=l->next){
            // Bookmarks in settings.ini
            // local bookmarks:
            auto p = (bookmarkItem_t *)l->data;
            if (!p->path) continue;
            TRACE("adding bookmark %p -> %s\n", p, p->path);
             if (g_path_is_absolute(p->path)) {
                const gchar *icon_name = EMBLEM_BOOKMARK;
                if (!Thread<Type>::fileTest(p->path, G_FILE_TEST_EXISTS)) {
                    DBG_("Bookmark %s does not exist\n", p->path);
                    icon_name = EMBLEM_BOOKMARK;
                }
                auto basename = g_path_get_basename(p->path);
                auto utf_name = util_c::utf_string(basename);
             
                auto highlight_name = g_strconcat(icon_name, "/", HIGHLIGHT_JUMP, NULL);

                auto treeViewPixbuf = Pixbuf<Type>::getPixbuf(icon_name,  -24);
                GdkPixbuf *normal_pixbuf = Pixbuf<Type>::getPixbuf(icon_name,  -48);
                GdkPixbuf *highlight_pixbuf = Pixbuf<Type>::getPixbuf(highlight_name,-48);   
                
                gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
                gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
                        DISPLAY_NAME, utf_name,
                        PATH, p->path,
                        ICON_NAME, icon_name,
                        TREEVIEW_PIXBUF, treeViewPixbuf, 
                        DISPLAY_PIXBUF, normal_pixbuf,
                        NORMAL_PIXBUF, normal_pixbuf,
                        HIGHLIGHT_PIXBUF, highlight_pixbuf,
                        TOOLTIP_TEXT,_("Go to bookmarked location"),
                        -1);
                g_free(basename);
                g_free(utf_name);
                g_free(highlight_name);
             }
        }

    }
    static gboolean
    isBookmarked(const gchar *path){
       return Bookmarks<Type>::isBookmarked(path);
    }

    static gboolean
    addBookmark(const gchar *path){
        return Bookmarks<Type>::addBookmark(path);
    }

    static gboolean
    removeBookmark(const gchar *path)
    {
        return Bookmarks<Type>::removeBookmark(path);
    }

 /*   static void
    addEFS(GtkTreeModel *treeModel){        
         GtkTreeIter iter;
        auto name = "xffm:efs";
        auto utf_name = util_c::utf_string(_("Ecryptfs (EFS)"));
        auto icon_name = "emblem-readonly/SE/list-add/2.0/225";
        auto highlight_name = g_strconcat(icon_name, "/", HIGHLIGHT_JUMP, NULL);

        auto treeViewPixbuf = Pixbuf<Type>::getPixbuf(icon_name,  -24);
        auto normal_pixbuf = Pixbuf<Type>::getPixbuf(icon_name,  GTK_ICON_SIZE_DIALOG);
        auto highlight_pixbuf = Pixbuf<Type>::getPixbuf(highlight_name,  GTK_ICON_SIZE_DIALOG); 

        gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
        gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
                DISPLAY_NAME, utf_name,
                PATH, name,
                ICON_NAME, icon_name,
                TREEVIEW_PIXBUF, treeViewPixbuf, 
                DISPLAY_PIXBUF, normal_pixbuf,
                NORMAL_PIXBUF, normal_pixbuf,
                HIGHLIGHT_PIXBUF, highlight_pixbuf,
                TOOLTIP_TEXT,_("Encrypted filesystem"),
                -1);
        g_free(utf_name);
        g_free(highlight_name);
    }*/

};
}
#endif
