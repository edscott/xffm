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
	TRACE("mk_tree_model:: model = %p\n", treeModel);
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
	const gchar *icon_name = "system-file-manager";
	const gchar *highlight_name = "system-file-manager/SE/document-open/2.0/225";
	//const gchar *highlight_name = "document-open/SE/system-file-manager-symbolic/3.0/180";
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
	icon_name = "user-home";
	highlight_name = "user-home/SE/document-open/2.0/225";
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

        addLocalBookmarks(treeModel);

	return TRUE;
    }

    static void
    addLocalBookmarks(GtkTreeModel *treeModel){
 	GtkTreeIter iter;
        gchar **bookMarks = getBookmarks();
        if (!bookMarks) return;
        gchar **p;
        for (p=bookMarks; p && *p; p++){
            // Bookmarks in settings.ini
            // local bookmarks:
            DBG("adding bookmark %p -> %s\n", p, *p);
             if (g_path_is_absolute(*p)) {
                if (!g_file_test(*p, G_FILE_TEST_EXISTS)) continue;
                auto basename = g_path_get_basename(*p);
                auto utf_name = util_c::utf_string(basename);
             
                const gchar *icon_name = "emblem-documents/NE/bookmark-new/2.0/220";
                const gchar *highlight_name = "emblem-documents/NE/document-open/2.0/220";

                /*if (g_file_test(*p, G_FILE_TEST_IS_DIR)){
                    icon_name = "folder/NE/bookmark-new/2.0/220";
                    highlight_name = "document-open/NE/bookmark-new/2.0/220";
                } else {
                    icon_name = "emblem-documents/NE/bookmark-new/2.0/220";
                    highlight_name = "emblem-documents/NE/document-open/2.0/220";
                }*/
                GdkPixbuf *normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  GTK_ICON_SIZE_DIALOG);
                GdkPixbuf *highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  GTK_ICON_SIZE_DIALOG);   
                
                gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
                gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
                        DISPLAY_NAME, utf_name,
                        ACTUAL_NAME, basename,
                        PATH, *p,
                        ICON_NAME, icon_name,
                        DISPLAY_PIXBUF, normal_pixbuf,
                        NORMAL_PIXBUF, normal_pixbuf,
                        HIGHLIGHT_PIXBUF, highlight_pixbuf,
                        TOOLTIP_TEXT,_("Open Bookmark"),
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
            DBG("%s is already bookmarked (%s)\n", path, item);
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
