#ifndef XF_ROOTMODEL__HH
# define XF_ROOTMODEL__HH

namespace xf
{

template <class Type>
class RootModel  {

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
	addRootItem(treeModel);
	addHomeItem(treeModel);
	addFstabItem(treeModel);
#ifdef HAVE_LIBXML2
	addPkgItem(treeModel);
#endif
	addTrashItem(treeModel);
        addLocalBookmarks(treeModel);

	return TRUE;
    }


    static gboolean enableDragSource(void){ return FALSE;}
    static gboolean enableDragDest(void){ return FALSE;}

    static const gchar *
    get_xfdir_iconname(void){
	return "system-file-manager";
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
    addRootItem(GtkTreeModel *treeModel){
 	GtkTreeIter iter;
	// Root
	auto name = "/";
	auto utf_name = util_c::utf_string(_("Root Directory"));
	auto icon_name = "system-file-manager";
	auto highlight_name = "system-file-manager/SE/document-open/2.0/225";
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
		TOOLTIP_TEXT,_("This is the root of the filesystem"),

		-1);
	g_free(utf_name);
    }

    static void
    addHomeItem(GtkTreeModel *treeModel){
 	GtkTreeIter iter;
	// Home
	auto name = g_get_home_dir();
	auto utf_name = util_c::utf_string(_("Home Directory"));
	auto icon_name = "user-home";
	auto highlight_name = "user-home/NE/document-open/2.0/225";
        auto treeViewPixbuf = Pixbuf<Type>::get_pixbuf(icon_name,  -24);
	auto normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  -48);
	auto highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  -48);   

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
    }

    static void
    addTrashItem(GtkTreeModel *treeModel){
 	GtkTreeIter iter;
	// Home
	auto name = g_strdup_printf("%s/.local/share/Trash/files", g_get_home_dir());
	auto utf_name = util_c::utf_string(_("Trash bin"));
	auto icon_name = "user-trash";
	auto trash = g_build_filename(g_get_home_dir(), ".local/share/Trash", NULL);
	if (g_file_test(trash, G_FILE_TEST_EXISTS))icon_name = "user-trash-full";
	    
	auto highlight_name = "user-trash/NE/document-open/2.0/225";
        auto treeViewPixbuf = Pixbuf<Type>::get_pixbuf(icon_name,  -24);
	auto normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  -48);
	auto highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  -48);   

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
    }

    static void
    addFstabItem(GtkTreeModel *treeModel){	
 	GtkTreeIter iter;
	auto name = "xffm:fstab";
	auto utf_name = util_c::utf_string(_("Disk Image Mounter"));
	auto icon_name = "media-eject";
	auto highlight_name = "media-eject/NE/document-open/2.0/225";
        auto treeViewPixbuf = Pixbuf<Type>::get_pixbuf(icon_name,  -24);
	auto normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  GTK_ICON_SIZE_DIALOG);
	auto highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  GTK_ICON_SIZE_DIALOG); 
	auto tooltipText = g_strdup_printf("%s\n%s\n%s\n%s\n%s",
		_("Mount local disks and devices"),
		_("eCryptfs Volume"),
		_("SSHFS Remote Synchronization Folder"),
		_("NFS Network Volume"),
		_("CIFS Volume"));

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
    }

#if defined HAVE_PACMAN || defined HAVE_EMERGE || defined HAVE_PKG
    static void
    addPkgItem(GtkTreeModel *treeModel){	
 	GtkTreeIter iter;
	auto name = "xffm:pkg";
	auto utf_name = util_c::utf_string(_("Software Updater"));
#ifdef HAVE_PACMAN
	auto icon_name = "package-x-generic/SE/emblem-archlinux/2.0/225";
	auto highlight_name = "package-x-generic/NE/document-open/2.0/225/SE/emblem-archlinux/2.0/225";
	auto pkg = "pacman";

#else 
#  ifdef HAVE_EMERGE
	auto icon_name = "package-x-generic/SE/emblem-gentoo/2.0/225";
	auto highlight_name = "package-x-generic/NE/document-open/2.0/225/SE/emblem-gentoo/2.0/225";
	auto pkg = "emerge";

#  else 
	auto icon_name = "package-x-generic/SE/x-package-repository/2.0/225";
	auto highlight_name = "package-x-generic/NE/document-open/2.0/225/SE/x-package-repository/2.0/225";
	auto pkg = "pkg";

#  endif
#endif
        auto treeViewPixbuf = Pixbuf<Type>::get_pixbuf(icon_name,  -24);
	auto normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  -48);
	auto highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  -48); 
	auto tooltipText = g_strdup_printf("%s",
		_("Add or remove software installed on the system"));

	gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
	gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
		DISPLAY_NAME, pkg, //utf_name,
                PATH, name,
		ICON_NAME, icon_name,
                TREEVIEW_PIXBUF, treeViewPixbuf, 
		DISPLAY_PIXBUF, normal_pixbuf,
		NORMAL_PIXBUF, normal_pixbuf,
		HIGHLIGHT_PIXBUF, highlight_pixbuf,
		TOOLTIP_TEXT,_("Add or remove software installed on the system"),
		-1);
	g_free(utf_name);
    }
#else
#warning "Package manager only with pkg, emerge or pacman"
    static void
    addPkgItem(GtkTreeModel *treeModel){}
#endif

    static void
    addLocalBookmarks(GtkTreeModel *treeModel){
 	GtkTreeIter iter;
        gchar **bookMarks = getBookmarks();
        if (!bookMarks) return;
        gchar **p;
        for (p=bookMarks; p && *p; p++){
            // Bookmarks in settings.ini
            // local bookmarks:
            TRACE("adding bookmark %p -> %s\n", p, *p);
             if (g_path_is_absolute(*p)) {
                if (!g_file_test(*p, G_FILE_TEST_EXISTS)) continue;
                auto basename = g_path_get_basename(*p);
                auto utf_name = util_c::utf_string(basename);
             
                const gchar *icon_name = "emblem-documents/SE/bookmark-new/2.0/220";
                const gchar *highlight_name = "emblem-documents/SE/bookmark-new/2.0/220/NE/document-open/2.0/220";
                auto treeViewPixbuf = Pixbuf<Type>::get_pixbuf(icon_name,  -24);
                GdkPixbuf *normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  -48);
                GdkPixbuf *highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,-48);   
                
                gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
                gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
                        DISPLAY_NAME, utf_name,
                        PATH, *p,
                        ICON_NAME, icon_name,
                        TREEVIEW_PIXBUF, treeViewPixbuf, 
                        DISPLAY_PIXBUF, normal_pixbuf,
                        NORMAL_PIXBUF, normal_pixbuf,
                        HIGHLIGHT_PIXBUF, highlight_pixbuf,
                        TOOLTIP_TEXT,_("Go to bookmarked location"),
                        -1);
                g_free(basename);
                g_free(utf_name);
             }
        }
        g_strfreev(bookMarks);

    }

    static gchar **
    getBookmarks(void){
        gsize size;
        gchar **keys = Settings<Type>::getKeys("Bookmarks", &size);
        if (!keys) return NULL;
        auto bookMarks = (gchar **)calloc(size+1, sizeof(gchar *));
        if (!bookMarks){
            ERROR("calloc: %s\n", strerror(errno));
            exit(1);
        }
        gchar **p;
        gint i=0;
        for (p=keys; p && *p; p++){
            bookMarks[i++] = Settings<Type>::getSettingString("Bookmarks", *p);
        }
        g_strfreev(keys);
        return bookMarks;
    }

    static gboolean
    addBookmark(const gchar *path)
    {
        gchar *item = findBookmarkKey(path);
	if (item){
            TRACE("%s is already bookmarked (%s)\n", path, item);
            g_free(item);
            return FALSE;
        }
        
        gint i=0;
	do {
	    item = g_strdup_printf("item-%0d", i++);
	    if (Settings<Type>::keyFileHasGroupKey("Bookmarks", item)) {
                g_free(item);
                continue;
            }
            break;
        } while (TRUE);
	Settings<Type>::setSettingString("Bookmarks", item, path);
	g_free(item);
        return TRUE;
    }

    static gboolean
    removeBookmark(const gchar *path)
    {
	gchar *item = findBookmarkKey(path);
	if (!item) return FALSE;
        Settings<Type>::removeKey("Bookmarks", item);
        g_free(item);
        return TRUE;
    }

    static gboolean
    isBookmarked(const gchar *path){
        gchar *key = findBookmarkKey(path);
        if (!key) return FALSE;
        g_free(key);
        return TRUE;
    }

private:
    static gchar *
    findBookmarkKey(const gchar *path){
        auto keys = Settings<Type>::getKeys("Bookmarks");
        if (!keys) return NULL;
        gchar **p;
        for (p=keys; p && *p; p++){
            gchar *g = Settings<Type>::getSettingString("Bookmarks", *p);
            if (strcmp(g, path) ==0){
                gchar *key = g_strdup(*p);
                g_free(g);
                g_strfreev(keys);
                return key;
            }
            g_free(g);
        }
        g_strfreev(keys);
        return NULL;
    }

/*	gint i=0;
	gchar *item;
	do {
	    item = g_strdup_printf("item-%0d", i++);
	    if (Settings<Type>::keyFileHasGroupKey("Bookmarks", item)){
		gchar *g = Settings<Type>::getSettingString("Bookmarks", item);
		if (strcmp(g, path) ==0){
		    g_free(g);
		    return item;
		}
	    } else break;
	} while(TRUE);
        return NULL;
    }
*/

};
}
#endif
