#ifndef XF_ROOTPOPUP_HH
# define XF_ROOTPOPUP_HH
#include "model.hh"

#ifndef BSD_FOUND
# ifdef ENABLE_EFS_MODULE
#  define DO_MOUNT_ITEMS
# endif
#else
# undef DO_MOUNT_ITEMS
#endif

namespace xf
{

template <class Type> class Popup;
template <class Type> class View;
template <class Type> class FstabView;
template <class Type> class LocalPopUp;
template <class Type> class BasePopUp;
template <class Type>
class RootPopUp  {
    public:


    static GtkMenu *popUpItem(void){
        if (!rootItemPopUp) rootItemPopUp = createItemPopUp();   
        return rootItemPopUp;
    }
    static GtkMenu *popUp(void){
        if (!rootPopUp) rootPopUp = createPopUp();   
        return rootPopUp;
    }
    
    static void
    resetMenuItems(void) {
        auto view = (View<Type> *)g_object_get_data(G_OBJECT(rootItemPopUp), "view");
        setPath(view);
        auto itemPath =Popup<Type>::getWidgetData(rootItemPopUp, "itemPath");
        TRACE("reset root menu items, itemPath=%s\n", itemPath);
	gboolean isBookMark = RootView<Type>::isBookmarked(itemPath);
#ifdef DO_MOUNT_ITEMS
        if (EFS<Type>::isEFS(itemPath)) isBookMark = TRUE;
#endif
	auto menuitem = GTK_WIDGET(g_object_get_data(G_OBJECT(rootItemPopUp), "Remove bookmark"));
	gtk_widget_set_sensitive(menuitem, isBookMark);
        if (isBookMark) gtk_widget_show(menuitem);
        else gtk_widget_hide(menuitem);
	menuitem = GTK_WIDGET(g_object_get_data(G_OBJECT(rootItemPopUp), "Empty trash"));
        auto trashFiles = g_build_filename(g_get_home_dir(), ".local/share/Trash", NULL);
        if (strstr(itemPath, trashFiles)) gtk_widget_show(menuitem);
        else gtk_widget_hide(menuitem);
        gtk_widget_set_sensitive(menuitem, g_file_test(trashFiles, G_FILE_TEST_IS_DIR));
        g_free(trashFiles); 
#ifdef DO_MOUNT_ITEMS
        auto umountW = GTK_WIDGET(g_object_get_data(G_OBJECT(rootItemPopUp), "Unmount the volume associated with this folder"));
        auto mountW = GTK_WIDGET(g_object_get_data(G_OBJECT(rootItemPopUp), "Mount the volume associated with this folder"));

        if (EFS<Type>::isEFS(itemPath)){
            itemPath += strlen("efs:/");
            GtkWidget *show, *hide;
	    TRACE("Root<>::resetMenuItems()...\n");
            if (FstabView<Type>::isMounted(itemPath)){
                show = umountW;
                hide = mountW;
            } else {
                hide = umountW;
                show = mountW;
            }
            gtk_widget_show(show);
            gtk_widget_hide(hide);
            gtk_widget_set_sensitive(show, TRUE);
        }
        else {
            gtk_widget_hide(umountW);
            gtk_widget_hide(mountW);
        }
#endif


    }

    static void
    resetPopup(void) {
        TRACE("reset root popup, is TreeView=%d\n", isTreeView);
        //auto w = GTK_WIDGET(g_object_get_data(G_OBJECT(rootPopUp), "View as list"));
        //gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w), isTreeView);

    }
     
private:

    static void 
    setPath(View<Type> * view){
	if (!view) {
	    ERROR("local/popup: setPath() view is %p\n", view);
	    return;
	}

	gint listLength = g_list_length(view->selectionList());
        GtkTreeIter iter;
        if (listLength > 1) return;

        auto tpath = (GtkTreePath *)view->selectionList()->data;
        gchar *itemPath;
        gchar *iconName;
        gchar *mimetype;
        gchar *displayName;
	if (!gtk_tree_model_get_iter (view->treeModel(), &iter, tpath)) {
	    return ;
	}
	gtk_tree_model_get (view->treeModel(), &iter, 
		//ACTUAL_NAME, &aname,
		DISPLAY_NAME, &displayName,
                MIMETYPE, &mimetype, 
		ICON_NAME, &iconName,
                PATH, &itemPath, 
                -1);
        if (!mimetype){
            mimetype = Mime<Type>::mimeType(itemPath); 
            gtk_list_store_set(GTK_LIST_STORE(view->treeModel()), &iter, 
                MIMETYPE, mimetype, -1);
        }
        // Set title element
        gchar *statLine;
        if (g_list_length(view->selectionList()) > 1) statLine = g_strdup("");
        else {
            struct stat st;
	    if (stat(itemPath, &st)<0){
		statLine = g_strdup_printf("stat(%s): %s", itemPath, strerror(errno));
		errno=0;
	    } else statLine  = Util<Type>::statInfo(&st);
        }

	gchar *fileInfo = Util<Type>::fileInfo(itemPath);

	auto title = GTK_MENU_ITEM(g_object_get_data(G_OBJECT(rootItemPopUp), "title"));
	gchar *markup = g_strdup_printf("<span color=\"blue\" size=\"large\">%s</span>", displayName);
	Gtk<Type>::menu_item_content(title, iconName, markup, -48);
	g_free(markup);
        
	
	    Popup<Type>::setWidgetData(rootItemPopUp, "fileInfo", fileInfo);
	    Popup<Type>::setWidgetData(rootItemPopUp, "iconName", iconName);
	    Popup<Type>::setWidgetData(rootItemPopUp, "displayName", displayName);
	    Popup<Type>::setWidgetData(rootItemPopUp, "itemPath", itemPath);
	    Popup<Type>::setWidgetData(rootItemPopUp, "mimetype", mimetype);
	    Popup<Type>::setWidgetData(rootItemPopUp, "statLine", statLine);
	    g_free(fileInfo);
	    g_free(iconName);
	    g_free(displayName);
	    g_free(itemPath);
	    g_free(mimetype);
	    g_free(statLine);
       return;
    }

    static GtkMenu *createPopUp(void){
         menuItem_t item[]={
            {N_("Add bookmark"), (void *)menuAddBookmark, NULL, NULL},
#ifdef DO_MOUNT_ITEMS
            {N_("Ecryptfs (EFS)"), (void *)menuAddEFS, NULL, NULL},
#endif
            {N_("Clear cache"), (void *)clearCache, NULL, NULL},
            {NULL,NULL,NULL, NULL}
         };
        const gchar *key[]={
            "Add bookmark",
            "Ecryptfs (EFS)",
            "Clear cache",
            NULL
        };
        const gchar *keyIcon[]={
            "list-add",
            "list-add",
            "list-remove",
            NULL
        };

        auto popup = new(Popup<Type>)(item, key, keyIcon, TRUE);
        rootPopUp = popup->menu();

        auto text = g_strdup_printf("Xffm+-%s", VERSION);
        popup->changeTitle(text, XFFM_ICON);
        g_free(text);
        return rootPopUp;        
    }  


    static GtkMenu *createItemPopUp(void){
	menuItem_t item[]=
        {
	    {N_("Open in New Tab"), (void *)LocalPopUp<Type>::newTab, rootItemPopUp, NULL},
            {N_("Remove bookmark"), (void *)removeBookmarkItem, NULL, NULL},
#ifdef DO_MOUNT_ITEMS
            {N_("Mount the volume associated with this folder"), (void *)LocalPopUp<Type>::mount, NULL, NULL},
            {N_("Unmount the volume associated with this folder"), (void *)LocalPopUp<Type>::mount, NULL, NULL},
#endif
            {N_("Empty trash"), (void *)emptyTrash, NULL, NULL},
	     {NULL,NULL,NULL,NULL}
        };
        const gchar *key[]={
            "Open in New Tab",
            "Remove bookmark",
#ifdef DO_MOUNT_ITEMS
            "Mount the volume associated with this folder",
            "Unmount the volume associated with this folder",
#endif
            "Empty trash",
            NULL
        };
        const gchar *keyIcon[]={
            "tab-new-symbolic",
            "edit-clear-all",
#ifdef DO_MOUNT_ITEMS
            "greenball",
            "redball",
#endif
            "user-trash-full",
            NULL
        };
        auto popup = new(Popup<Type>)(item, key, keyIcon, TRUE);
        rootItemPopUp = popup->menu();

        auto text = g_strdup_printf("Xffm+-%s", VERSION);
        popup->changeTitle(text, XFFM_ICON);
        g_free(text);
        
        return rootItemPopUp;
    }
    static void
    clearCache(GtkMenuItem *menuItem, gpointer data)
    {
        TRACE("clearCache\n");
	auto cache_dir = g_build_filename (XFTHUMBNAIL_DIR, NULL);
	Gio<Type>::clearDirectory(cache_dir);
	g_free(cache_dir);
    }

#ifdef ENABLE_EFS_MODULE
    static void
    menuAddEFS(GtkMenuItem *menuItem, gpointer data)
    {
        TRACE("menuAddEFS\n");
        EFS<Type>::doDialog(NULL, data);
    }
#endif
    static void
    menuAddBookmark(GtkMenuItem *menuItem, gpointer data)
    {
        menuMarker(menuItem, data, 0);
    }

    static void
    menuMarker(GtkMenuItem *menuItem, gpointer data, gint which)
    {
        // File chooser
        const gchar *text = _("Add bookmark");
        const gchar *defDir = NULL;
        switch (which) {
            case 1: // FUSE Ecrypt filesystem
                text = _("Ecrypt Volume");
                defDir = _("private");
                break;
            default:
                break;
        }
        auto icon = "list-add";
        auto entryResponse = new(EntryFolderResponse<Type>)(GTK_WINDOW(mainWindow), text, icon);        

        entryResponse->setEntryLabel(_("Specify Output Directory..."));
        

        gchar *dirname = g_build_filename(g_get_home_dir(), defDir, NULL);
        entryResponse->setEntryDefault(dirname);
        entryResponse->setEntryBashFileCompletion(g_get_home_dir());
        entryResponse->setInLineCompletion(1);
        g_free(dirname);
        
        auto response = entryResponse->runResponse();
        if (response){
            response = LocalPopUp<Type>::ckDir(response);
            if (!response) return;
 	    g_strstrip(response);
            if (!strlen(response))return;
            switch (which) {
                case 1:
                    break;
                default:
                    RootModel<Type>::addBookmark(response);
                    break;
            }
            g_free(response);
            auto view =  (View<Type> *)g_object_get_data(G_OBJECT(data), "view");
            view->reloadModel();
       }
       return;
        
    }

    static void
    removeBookmarkItem(GtkMenuItem *menuItem, gpointer data)
    {
        TRACE("Remove bookmark\n");
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "itemPath");
        if (EFS<Type>::isEFS(path)){
            if (!EFS<Type>::removeItem(path+strlen("efs:/"))) return;
        } else {
            if (!RootView<Type>::removeBookmark(path)) return;
        }
	auto view =  (View<Type> *)g_object_get_data(G_OBJECT(data), "view");
	view->reloadModel();
    }

    static gboolean
    updateTrashIcon(GtkTreeModel *treeModel, GtkTreePath *tpath, GtkTreeIter *iter, void *data){
	auto path = (const gchar *)data;
	if (!path) return FALSE;
	gchar *thisPath;
        gtk_tree_model_get(treeModel, iter, PATH, &thisPath, -1);
	TRACE( "%s <__> %s \n", thisPath, path);
	if (!thisPath) return FALSE;
	if (!strstr(thisPath, path)){
	    g_free(thisPath);
            return FALSE;
	}
        g_free(thisPath);	
	auto icon_name = "user-trash";
	auto highlight_name = g_strconcat(icon_name, "/", HIGHLIGHT_EMBLEM, NULL);
	auto treeViewPixbuf = Pixbuf<Type>::getPixbuf(icon_name,  -24);
	auto normal_pixbuf = Pixbuf<Type>::getPixbuf(icon_name,  -48);
	auto highlight_pixbuf = Pixbuf<Type>::getPixbuf(highlight_name,  -48);   
	g_free(highlight_name);
	
	gtk_list_store_set (GTK_LIST_STORE(treeModel), iter, 
		ICON_NAME, icon_name,
                TREEVIEW_PIXBUF, treeViewPixbuf, 
		DISPLAY_PIXBUF, normal_pixbuf,
		NORMAL_PIXBUF, normal_pixbuf,
		HIGHLIGHT_PIXBUF, highlight_pixbuf,
		-1);
	return TRUE;
    }

    static void
    emptyTrash(GtkMenuItem *menuItem, gpointer data) {
	auto trash = g_build_filename(g_get_home_dir(), ".local/share/Trash", NULL);
	Gio<Type>::execute((GtkDialog *)NULL, trash, MODE_RM);
	auto view = (View<Type> *)g_object_get_data(G_OBJECT(data), "view");
	gtk_tree_model_foreach(view->treeModel(), updateTrashIcon, (void *)trash);
	g_free(trash);
    }
	


};
}
#endif
