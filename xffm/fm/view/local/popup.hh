#ifndef XF_LOCALITEMPOPUP__HH
# define XF_LOCALITEMPOPUP__HH

#include "rm.hh"
#include "properties.hh"

static const gchar *directoryItems[] ={
    "Open in New Tab",
    "Create a compressed archive with the selected objects",
    "Mount the volume associated with this folder",
    "Unmount the volume associated with this folder",
    "Add bookmark",
    "Remove bookmark",
NULL};

static const gchar *regularItems[]={
    "mimetypeOpen",
    "Open with",
    "Run Executable...",
    "Extract files from the archive",
};

static const gchar *commonItems[]={
    "Copy",
    "Cut",
    "Paste",
    "Rename",
    "Duplicate",
    "Link",
    "Properties",
    "Delete",
    NULL,
};

static const gchar *singleSelectItems[]={
    "Rename",
    "Duplicate",
    "Link",
    "Paste into",
    NULL,
};

static const gchar *generalItems[]={
    "title",
};

namespace xf
{



template <class Type> class FstabView;
template <class Type> class View;
template <class Type> class BaseViewSignals;
template <class Type> class Dialog;
template <class Type> class LocalRm;
template <class Type> class LocalView;
template <class Type>
class LocalPopUp {
    
    using pixbuf_c = Pixbuf<Type>;
    using util_c = Util<Type>;
    using page_c = Page<Type>;

public:

    static GtkMenu *createLocalPopUp(void){
        auto popup = new(Popup<Type>)(menuItems(), menuKeys(), menuIcons());
        localPopUp = popup->menu();
	//localPopUp = BasePopUp<Type>::createPopup(item); 
        //decorateEditItems(localPopUp);
        return localPopUp;        
    }  

    static GtkMenu *createLocalItemPopUp(void){
	TRACE("createLocalItemPopUp\n" );
        auto popup = new(Popup<Type>)(localMenuItems(), localMenuItemsKeys(), localMenuItemsIcons(), TRUE);
        localItemPopUp = popup->menu();

        // Customize most important menu items
        GtkMenuItem *mItem;
        gchar *markup;
        mItem = (GtkMenuItem *)g_object_get_data(G_OBJECT(localItemPopUp), "Open with");
        markup = g_strdup_printf("<b>%s</b>", _("Open with"));
        Gtk<Type>::menu_item_content(mItem, "system-run", markup, -24);
	TRACE("createLocalItemPopUp: %p\n" , localItemPopUp);

        return localItemPopUp;
    }

    static menuItem_t *menuItems(void){
         static menuItem_t item[]={
	    {N_("New"), (void *)newItem, NULL, NULL},
	    {N_("Open in New Tab"), (void *)newTab, localPopUp, NULL},
            // deprecated {N_("Open in New Tab"), NULL, NULL, NULL},
            // deprecated {N_("Open in New Window"), NULL, NULL, NULL},
            
	    {N_("Copy"), (void *)ClipBoard<Type>::copy, NULL, NULL},
	    {N_("Cut"), (void *)ClipBoard<Type>::cut, NULL, NULL},
	    {N_("Paste into"), (void *)ClipBoard<Type>::pasteInto, NULL, NULL},
	    {N_("There is nothing on the clipboard to paste."), NULL, NULL, NULL},
	    {N_("Delete"), (void *)LocalRm<Type>::rm, NULL, NULL},
            //{N_("About"), NULL, (void *) menu},
            //
            {N_("Select All"), (void *)selectAll, NULL, NULL},
            {N_("Match regular expression"), (void *)selectMatch, NULL, NULL},
            {NULL,NULL,NULL, NULL}};
	 return item;
    }
    static const gchar **menuKeys(void){
        static const gchar *key[]={
            "New",// this menuitem is only for nonitem popup
            "Open in New Tab",
            "Cut",
            "Copy",
            "Paste into",
            "There is nothing on the clipboard to paste.",
            "Delete",
            NULL
        };
	return key;
    }
    static const gchar **menuIcons(void){
        static const gchar *keyIcon[]={
            "document-new",
            "tab-new-symbolic",
            "edit-cut",
            "edit-copy",
            "edit-paste",
            "edit-paste",
            "edit-delete",
            NULL
        };
	return keyIcon;
    }

    static menuItem_t *localMenuItems(void){
	static menuItem_t item[]=
        {
	    {("mimetypeOpen"), (void *)command, NULL, NULL},
	    {N_("Open with"), (void *)openWith, NULL, NULL},
	    {N_("Run Executable..."), (void *)runWith, NULL, NULL},
	    {N_("Extract files from the archive"), (void *)untar, NULL, NULL},
	    {N_("Open in New Tab"), (void *)newTab, localItemPopUp, NULL},
	    {N_("Create a compressed archive with the selected objects"), (void *)tarball, NULL, NULL},
#ifdef ENABLE_FSTAB_MODULE
	    {N_("Mount the volume associated with this folder"), (void *)mount, NULL, NULL},
	    {N_("Unmount the volume associated with this folder"), (void *)mount, NULL, NULL},
#endif
            {N_("Add bookmark"), (void *)addBookmark, NULL, NULL},
            {N_("Remove bookmark"), (void *)removeBookmark, NULL, NULL},
	    
	    //common buttons /(also an iconsize +/- button)
	    {N_("Copy"), (void *)ClipBoard<Type>::copy, NULL, NULL},
	    {N_("Cut"), (void *)ClipBoard<Type>::cut, NULL, NULL},
	    {N_("Paste"), (void *)ClipBoard<Type>::paste, NULL, NULL},
	    {N_("There is nothing on the clipboard to paste."), NULL, NULL, NULL},
	    {N_("Paste into"), (void *)ClipBoard<Type>::pasteInto, NULL, NULL},
	    {N_("Delete"), (void *)LocalRm<Type>::rm, NULL, NULL},
	    {N_("Rename"), (void *)rename, NULL, NULL},
	    {N_("Duplicate"), (void *)duplicate, NULL, NULL}, 
	    {N_("Link"), (void *)symlink, NULL, NULL},
	    {N_("Properties"), (void *)properties, NULL, NULL},
	     {NULL,NULL,NULL,NULL}
        };
	return item;
    }

    static const gchar **localMenuItemsKeys(void){
        static const gchar *key[]={
            "Add bookmark",
            "Remove bookmark",
            "Open in New Tab",
            "Create a compressed archive with the selected objects",
            "Extract files from the archive",
#ifdef ENABLE_FSTAB_MODULE            
            "Mount the volume associated with this folder",
            "Unmount the volume associated with this folder",
#endif
            "Cut",
            "Copy",
            "Paste",
            "There is nothing on the clipboard to paste.",
            "Paste into",
            "Delete",

            "Rename",
            "Duplicate",
            "Link",
            "Properties",
            NULL
        };
	return key;
    }
    
    static const gchar **localMenuItemsIcons(void){
        static const gchar *keyIcon[]={
            "bookmark-new",
            "edit-clear-all",
            "tab-new-symbolic",
            "package-x-generic",
            "insert-object",
#ifdef ENABLE_FSTAB_MODULE            
            "greenball",
            "redball",
#endif
            "edit-cut",
            "edit-copy",
            "edit-paste",
            "edit-paste",
            "edit-paste",
            "edit-delete",

	    "document-revert",
            "document-save",
            "emblem-symbolic-link",
            "document-properties",
            NULL
        };
	return keyIcon;
    }

private:
    static void
    customPasteInto(GtkMenu *menu, const gchar *path, gint size){
	    // Make menuitem text specific...
	    gchar *specific;
	    if (strlen(path)>30){
		const gchar *p = strchr(path + (strlen(path)-30),'/');

		specific = g_strdup_printf("%s <span color=\"blue\">[...] %s</span>", _("Paste into"), p?p+1:path);
	    } else
		specific = g_strdup_printf("%s <span color=\"blue\">%s</span>", _("Paste into"), path);
	    auto menuItem = (GtkMenuItem *)g_object_get_data(G_OBJECT(menu),"Paste into");
            Gtk<Type>::menu_item_content(menuItem, "edit-paste", specific, size);	    
	    g_free(specific);
    }

public:
    static void
    resetPopup(void) {
        auto view = (View<Type> *)g_object_get_data(G_OBJECT(localPopUp), "view");
	gint listLength = view?g_list_length(view->selectionList()):0;

        auto path =Popup<Type>::getWidgetData(localPopUp, "path");
	TRACE("local resetPopup path=%s\n", path);
        if (!path){
	    ERROR("local/popup.hh::resetPopup: path is NULL\n");
	    return;
	}

	auto validView = g_object_get_data(G_OBJECT(localPopUp),"view")!=NULL;

	Popup<Type>::configureMenuItem(localPopUp, "New", validView, path);
	Popup<Type>::configureMenuItem(localPopUp, "Open in New Tab", TRUE, path);
	Popup<Type>::configureMenuItem(localPopUp, "Select All", validView, path);
	Popup<Type>::configureMenuItem(localPopUp, "Match regular expression",validView, path);

        // unsensitivize "Paste" only if valid pasteboard...
	//Popup<Type>::configureMenuItem(localPopUp, "Paste", validView && ClipBoard<Type>::clipBoardIsValid(), path);
	Popup<Type>::configureMenuItem(localPopUp, "There is nothing on the clipboard to paste.",!ClipBoard<Type>::clipBoardIsValid(), NULL);

	Popup<Type>::configureMenuItem(localPopUp, "Paste into", ClipBoard<Type>::clipBoardIsValid(), path);
	customPasteInto(localPopUp, path, -24);

	Popup<Type>::configureMenuItem(localPopUp, "Copy",listLength > 0, path);
	Popup<Type>::configureMenuItem(localPopUp, "Cut",listLength > 0, path);
	Popup<Type>::configureMenuItem(localPopUp, "Delete",listLength > 0, path);

	gchar *fileInfo = util_c::fileInfo(path);
	gchar *displayName = util_c::valid_utf_pathstring(path);
	gchar *mimetype = g_strdup("inode/directory");
	TRACE("local resetPopup mimetype=%s\n",mimetype);
	gchar *statLine;
	if (g_file_test(path, G_FILE_TEST_EXISTS)) {
            struct stat st;
            stat(path, &st);
            statLine  = Util<Type>::statInfo(&st);
        }
	else statLine = g_strdup(strerror(ENOENT));

	    Popup<Type>::setWidgetData(localPopUp, "fileInfo", fileInfo);
	    Popup<Type>::setWidgetData(localPopUp, "iconName", "folder");
	    Popup<Type>::setWidgetData(localPopUp, "displayName", displayName);
	    Popup<Type>::setWidgetData(localPopUp, "mimetype", mimetype);
	    Popup<Type>::setWidgetData(localPopUp, "statLine", statLine);


        //auto iconName = (gchar *)g_object_get_data(G_OBJECT(localPopUp), "iconName");
        //auto path = (gchar *)g_object_get_data(G_OBJECT(localPopUp), "path");


	if (fileInfo) {
	    while (strchr(fileInfo, '&')){
		*(strchr(fileInfo, '&')) = '+';
	    }
	}
        
        gchar *markup = g_strdup_printf("<span color=\"red\"><b><i>%s</i></b></span><span color=\"#aa0000\">%s%s</span>\n<span color=\"blue\">%s</span>\n<span color=\"green\">%s</span>", 
		displayName, 
		mimetype?": ":"", mimetype?mimetype:"",
		fileInfo?fileInfo:"", 
		statLine?statLine:"");


        Popup<Type>::changeTitle(localPopUp, markup, "folder");
	g_free(markup);
	g_free(fileInfo);
	g_free(displayName);
	g_free(mimetype);
	g_free(statLine);
    }

    static void
    resetMenuItems(void) {
	TRACE("resetMenuItems\n" );
        auto view = (View<Type> *)g_object_get_data(G_OBJECT(localItemPopUp), "view");

        //  single or multiple item selected?
        setPath(view);
 
        // Hide all...
        GList *children = gtk_container_get_children (GTK_CONTAINER(localItemPopUp));
        for (GList *child = children; child && child->data; child=child->next){
            gtk_widget_hide(GTK_WIDGET(child->data));
        }
 
        auto itemPath =Popup<Type>::getWidgetData(localItemPopUp, "itemPath");

        auto display_name = Popup<Type>::getWidgetData(localItemPopUp,  "displayName");
        auto mimetype =Popup<Type>::getWidgetData(localItemPopUp,  "mimetype");
        auto fileInfo =Popup<Type>::getWidgetData(localItemPopUp,  "fileInfo");
        auto iconName = Popup<Type>::getWidgetData(localItemPopUp,  "iconName");
        auto statLine = Popup<Type>::getWidgetData(localItemPopUp,  "statLine");

	gchar *specificMimeType = NULL;
	gchar *plainMimeType = NULL;
        
	if (g_list_length(view->selectionList())==1){
	    specificMimeType = Mime<Type>::mimeType(itemPath);
	    if (mimetype && specificMimeType && strcmp(mimetype, specificMimeType)){
	        plainMimeType = g_strdup_printf("(%s)",mimetype);
            }
	}
	
        gchar *markup = g_strdup_printf("<span color=\"red\"><b><i>%s</i></b></span><span color=\"#aa0000\">: %s %s</span>\n<span color=\"blue\">%s</span>\n<span color=\"green\">%s</span>", 
		display_name, 
		specificMimeType?specificMimeType:"",
		plainMimeType?plainMimeType:"",
		fileInfo?fileInfo:"", 
		statLine?statLine:"");
	g_free(plainMimeType);

        Popup<Type>::changeTitle(localItemPopUp, markup, iconName);
	g_free(markup);
        auto v2 = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "title"));
        gtk_widget_show(v2);
	
	gint listLength = g_list_length(view->selectionList());
	for (auto k=commonItems; k && *k; k++){
	    Popup<Type>::configureMenuItem(localItemPopUp, *k, listLength > 0, itemPath);
	}
	Popup<Type>::configureMenuItem(localItemPopUp, "Paste", ClipBoard<Type>::clipBoardIsValid(), itemPath);
	Popup<Type>::configureMenuItem(localItemPopUp, "There is nothing on the clipboard to paste.", !ClipBoard<Type>::clipBoardIsValid(), NULL);
	Popup<Type>::configureMenuItem(localItemPopUp, "Paste into", FALSE, itemPath);

	for (auto k=singleSelectItems; k && *k; k++){
	    Popup<Type>::configureMenuItem(localItemPopUp, *k, listLength == 1, itemPath);
	}
	if (listLength==1){
	    Popup<Type>::configureMenuItem(localItemPopUp, "Extract files from the archive", strstr(mimetype, "compressed-tar") != NULL, itemPath);

	    Popup<Type>::configureMenuItem(localItemPopUp, "Paste into", 
			strstr(mimetype, "inode/directory") != NULL &&
			ClipBoard<Type>::clipBoardIsValid(), itemPath);
	    customPasteInto(localItemPopUp, itemPath, -16);
	}

	if (listLength == 1){
	    runWithDialog(itemPath); // ask for arguments for an executable path.
	    // open with mimetype application
	    setUpMimeTypeApp(specificMimeType?specificMimeType:mimetype, itemPath, fileInfo);
	    if (g_file_test(itemPath, G_FILE_TEST_IS_DIR)) {
		showDirectoryItems(itemPath);
	    }
	    else openWithDialog(itemPath, specificMimeType?specificMimeType:mimetype, fileInfo);
	} else {
	    openWithDialog();
	}

	g_free(specificMimeType);


    }

    static GtkMenu *popUpItem(void){
	TRACE("popUpItem\n" );
        if (!localItemPopUp) localItemPopUp = createLocalItemPopUp();   
        return localItemPopUp;
    }
    static GtkMenu *popUp(void){
        if (!localPopUp) localPopUp = createLocalPopUp();   
        return localPopUp;
    }

private:
    static void
    runWithDialog(const gchar *path){
	// Run with dialog
	auto v1 = GTK_MENU_ITEM(g_object_get_data(G_OBJECT(localItemPopUp), "Run Executable..."));

        gboolean state = (g_file_test(path, G_FILE_TEST_IS_EXECUTABLE) &&
                g_file_test(path, G_FILE_TEST_IS_REGULAR));
	gtk_widget_set_sensitive(GTK_WIDGET(v1), state);
	if (state) gtk_widget_show(GTK_WIDGET(v1));
	else gtk_widget_hide(GTK_WIDGET(v1));
    }

    static void
    openWithDialog(void){
        TRACE("openWithDialog(void)\n");
	auto v2 = GTK_MENU_ITEM(g_object_get_data(G_OBJECT(localItemPopUp), "Open with"));
        gtk_widget_show(GTK_WIDGET(v2));
	gtk_widget_set_sensitive(GTK_WIDGET(v2), TRUE);
    }

    static void
    openWithDialog(const gchar *path, const gchar *mimetype, const gchar *fileInfo){
        TRACE("openWithDialog(): path=%s\n", path);
        gboolean state;
        if (strchr(path, '\'')){
            gchar **f = g_strsplit(path,"\'", 2);
            state = g_file_test(f[0], G_FILE_TEST_IS_REGULAR);
            TRACE("%s is regular: %d\n", f[0], state);
            g_strfreev(f);
        } else {
            state = g_file_test(path, G_FILE_TEST_IS_REGULAR);
        }

	// Open with dialog
	auto v2 = GTK_MENU_ITEM(g_object_get_data(G_OBJECT(localItemPopUp), "Open with"));
        if (state) {
            gtk_widget_show(GTK_WIDGET(v2));
        } else {
            gtk_widget_hide(GTK_WIDGET(v2));
        }
	gtk_widget_set_sensitive(GTK_WIDGET(v2), state);
    }

    static void
    showDirectoryItems(const gchar *path){
	TRACE("showDirectoryItems: %s\n", path);
        // Directory items...
        const gchar *directoryItems[] ={
            "title",
            "Open in New Tab",
            "Create a compressed archive with the selected objects",
#ifdef ENABLE_FSTAB_MODULE
            "Mount the volume associated with this folder",
            "Unmount the volume associated with this folder",
#endif
	    "Add bookmark",
	    "Remove bookmark",
	    "Paste into",
        NULL};

        GtkWidget *w;
        // unhide 
        const gchar **p;
        for (p=directoryItems; p &&*p; p++){
	    Popup<Type>::configureMenuItem(localItemPopUp, *p, FALSE, path);
        }
	Popup<Type>::configureMenuItem(localItemPopUp, "title", TRUE, path);

        // unsensitivize "Paste" only if valid pasteboard...
	Popup<Type>::configureMenuItem(localItemPopUp, "Paste", ClipBoard<Type>::clipBoardIsValid(), path);
	Popup<Type>::configureMenuItem(localItemPopUp, "Paste into", ClipBoard<Type>::clipBoardIsValid(), path);

        //////  Directory options

        // open in new tab
	Popup<Type>::configureMenuItem(localItemPopUp, "Open in New Tab", TRUE, path);
        w = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "Open in New Tab"));
        gtk_widget_set_sensitive(w, TRUE);
        // Create compressed tarball
	Popup<Type>::configureMenuItem(localItemPopUp, "Create a compressed archive with the selected objects", TRUE, path);

	// bookmark options
        if (!RootView<Type>::isBookmarked(path)) {
	    Popup<Type>::configureMenuItem(localItemPopUp, "Add bookmark", TRUE, path);
	    Popup<Type>::configureMenuItem(localItemPopUp, "Remove bookmark", FALSE, path);
        } else {
	    Popup<Type>::configureMenuItem(localItemPopUp, "Add bookmark", FALSE, path);
	    Popup<Type>::configureMenuItem(localItemPopUp, "Remove bookmark", TRUE, path);
        }
	/*
	// bookmark options
        if (!RootView<Type>::isBookmarked(path)) {
            w = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "Add bookmark"));
            gtk_widget_set_sensitive(w, TRUE);
            w = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "Remove bookmark"));
            gtk_widget_hide(w);
        } else {
            w = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "Remove bookmark"));
            gtk_widget_set_sensitive(w, TRUE);
            w = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "Add bookmark"));
            gtk_widget_hide(w);
        }*/
	

#ifdef ENABLE_FSTAB_MODULE
        // mount options
#ifdef ENABLE_EFS_MODULE
        if (EFS<Type>::isEFS(path)) path += strlen("efs:/");
#endif
        GtkWidget *show, *hide;
	TRACE("local/showDirectoryItems()...isInFstab(%s)=%d\n",
		path, FstabView<Type>::isInFstab(path));

        if (FstabView<Type>::isMounted(path)){
            show = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "Unmount the volume associated with this folder"));
            hide = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "Mount the volume associated with this folder"));
	    gtk_widget_show(show);
	    gtk_widget_hide(hide);
        } else {
	    // only if in fstab...

            hide = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "Unmount the volume associated with this folder"));
            show = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "Mount the volume associated with this folder"));
	    gtk_widget_hide(hide);
	    if (FstabView<Type>::isInFstab(path)) gtk_widget_show(show);
	    else gtk_widget_hide(show);
        }
        gtk_widget_set_sensitive(show, TRUE);
#endif
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
        if (listLength > 1) {
            gchar *paths = g_strdup("");
            for (GList *l = view->selectionList(); l && l->data; l=l->next){
                if (!gtk_tree_model_get_iter (view->treeModel(), &iter, (GtkTreePath *)l->data)){
                    continue;
                }
                gchar *path;
        	gtk_tree_model_get (view->treeModel(), &iter, PATH, &path, -1);
                gchar *g = g_strconcat(paths, path, "\'", NULL);
                g_free(paths);
                g_free(path);
                paths = g;
            }
            gchar *fileInfo = g_strdup_printf("%s %d", _("Files:"), g_list_length(view->selectionList()));
	    Popup<Type>::setWidgetData(localItemPopUp, "fileInfo", fileInfo);
	    Popup<Type>::setWidgetData(localItemPopUp, "iconName", "edit-copy");
	    Popup<Type>::setWidgetData(localItemPopUp, "displayName", _("Multiple selections"));
	    Popup<Type>::setWidgetData(localItemPopUp, "path", paths);
	    Popup<Type>::setWidgetData(localItemPopUp, "mimetype", "");
	    Popup<Type>::setWidgetData(localItemPopUp, "statLine", "");
	    g_free(fileInfo);
	    g_free(paths);

            return;
        }

        auto tpath = (GtkTreePath *)view->selectionList()->data;
        gchar *path;
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
                PATH, &path, 
                -1);
        if (!mimetype){
            mimetype = Mime<Type>::mimeType(path); 
            gtk_list_store_set(GTK_LIST_STORE(view->treeModel()), &iter, 
                MIMETYPE, mimetype, -1);
        }
        // Set title element
        gchar *statLine;
        {
            struct stat st;
	    if (stat(path, &st)<0){
		statLine = g_strdup_printf("stat(%s): %s", path, strerror(errno));
		errno=0;
	    } else statLine  = Util<Type>::statInfo(&st);
        }

	gchar *fileInfo = util_c::fileInfo(path);
	    Popup<Type>::setWidgetData(localItemPopUp, "fileInfo", fileInfo);
	    Popup<Type>::setWidgetData(localItemPopUp, "iconName", iconName);
	    Popup<Type>::setWidgetData(localItemPopUp, "displayName", displayName);
	    Popup<Type>::setWidgetData(localItemPopUp, "path", path);
	    Popup<Type>::setWidgetData(localItemPopUp, "mimetype", mimetype);
	    Popup<Type>::setWidgetData(localItemPopUp, "statLine", statLine);
	    g_free(fileInfo);
	    g_free(iconName);
	    g_free(displayName);
	    g_free(path);
	    g_free(mimetype);
	    g_free(statLine);
       return;
    }

    static void 
    setUpMimeTypeApp(const gchar *mimetype, const gchar *path, const gchar *fileInfo)
    {
        gchar *defaultApp = Run<Type>::defaultExtApp(path);
        if (!defaultApp) defaultApp = Run<Type>::defaultMimeTypeApp(mimetype);
        if (!defaultApp) defaultApp = Run<Type>::defaultTextApp(fileInfo);

	auto v = GTK_MENU_ITEM(g_object_get_data(G_OBJECT(localItemPopUp), "mimetypeOpen"));
	if (defaultApp)  {
	    auto v = GTK_MENU_ITEM(g_object_get_data(G_OBJECT(localItemPopUp), "mimetypeOpen"));

	    auto command = (gchar *)g_object_get_data(G_OBJECT(v), "command");
	    g_free(command);
	    if (Run<Type>::runInTerminal(defaultApp)){
		command = Run<Type>::mkTerminalLine(defaultApp, path);
	    } else {
		command = Run<Type>::mkCommandLine(defaultApp, path);
	    }
	    auto displayCommand = Run<Type>::mkCommandLine(defaultApp, path);
	    auto markup = g_strdup_printf("<b>%s</b>", displayCommand);
	    g_free(displayCommand);

	    auto icon = Run<Type>::baseIcon(defaultApp);
	    //auto p = pixbuf_c::getPixbuf(icon, -24); 
	    auto iconOK = Pixbuf<Type>::iconThemeHasIcon(icon);
	    Gtk<Type>::menu_item_content(v, iconOK?icon:"system-run-symbolic", markup, -24);
	    g_free(icon);
	    g_free(markup);
	    
	    g_object_set_data(G_OBJECT(v), "command", command);
	    gtk_widget_show(GTK_WIDGET(v));
	    gtk_widget_set_sensitive(GTK_WIDGET(v), TRUE);
	    return;
	} else {
	    auto v = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "mimetypeOpen"));
	    gtk_widget_hide(v);
	}
	return;
    }

private:
    static gboolean
    selectAll_(GtkTreeModel *treeModel, GtkTreePath *tpath, GtkTreeIter *iter, gpointer data){
        auto localView = (LocalView<Type> *)data;
        auto view = (View<Type> *)data;
        if (localView->isSelectable(treeModel, iter)){
	    if (isTreeView){
		auto selection = gtk_tree_view_get_selection (view->treeView());
                gtk_tree_selection_select_path (selection, tpath);
	    } else {
		gtk_icon_view_select_path(view->iconView(), tpath);
	    }
        }
        return FALSE;        
    }

    static void 
    selectAll(GtkMenuItem *menuItem, gpointer data)
    {
	auto view =  (View<Type> *)g_object_get_data(G_OBJECT(data), "view");
        gtk_tree_model_foreach (view->treeModel(), selectAll_, (void *)view);
    }

    static gboolean
    selectMatch_(GtkTreeModel *treeModel, GtkTreePath *tpath, GtkTreeIter *iter, gpointer data){
        auto arg = (void **)data;
        auto view = (View<Type> *)(arg[0]);
        auto regex = (const GRegex *)arg[1];
        gchar *path;
        gtk_tree_model_get(treeModel, iter, PATH, &path, -1);
        auto basename = g_path_get_basename(path);
        g_free(path);
        gboolean match = g_regex_match (regex, basename, (GRegexMatchFlags)0, NULL);
        if (match){
            TRACE("match: %s\n", basename);
	    if (isTreeView){
		auto selection = gtk_tree_view_get_selection (view->treeView());
                gtk_tree_selection_select_path (selection, tpath);
	    } else {
		gtk_icon_view_select_path(view->iconView(), tpath);
	    }
        }
        g_free(basename);
        return FALSE;
    }

    static void 
    selectMatch(GtkMenuItem *menuItem, gpointer data)
    {
	auto view =  (View<Type> *)g_object_get_data(G_OBJECT(data), "view");
        auto entryResponse = new(EntryResponse<Type>)(GTK_WINDOW(mainWindow), _("Select items"), NULL);
	auto markup = 
	    g_strdup_printf("<span color=\"blue\" size=\"larger\"><b>%s</b></span>", _("Match regular expression"));  
  	
        entryResponse->setResponseLabel(markup);
        g_free(markup);
        entryResponse->setEntryLabel(_("Regular expression"));
        auto response = entryResponse->runResponse();
	TRACE("response=%s\n", response);
        if (!response) return;
        g_strstrip(response);
	if (strlen(response)){
            auto cflags = (GRegexCompileFlags)((guint)G_REGEX_CASELESS | (guint)G_REGEX_OPTIMIZE);
            GError *error=NULL;
            GRegex *regex = g_regex_new (response, cflags,(GRegexMatchFlags) 0, &error);
            if (!regex) {
                gchar *markup = g_strdup_printf("<span size=\"larger\" color=\"blue\">%s\n<span color=\"red\">%s</span></span>\n%s\n",
                        _("Regular Expression syntax is incorrect"), error->message,
                        FindDialog<Type>::grep_text_help);
                Dialogs<Type>::quickHelp(GTK_WINDOW(mainWindow), markup, "dialog-error");
                g_free(markup);
                g_error_free(error);
                return;
            }
            void *arg[]={
                (void *)view, 
                (void *)regex
            };
            // unselect all
	    // FIXME: need to separate treeview instructions here!
            if (isTreeView){
		auto selection = gtk_tree_view_get_selection (view->treeView());
		gtk_tree_selection_unselect_all (selection);
	    } else {
		gtk_icon_view_unselect_all(view->iconView());
	    }
            gtk_tree_model_foreach (view->treeModel(), selectMatch_, (void *)arg);
            GList *selection_list;
            if (isTreeView){
		auto selection = gtk_tree_view_get_selection (view->treeView());
		auto treeModel = view->treeModel();
		selection_list = gtk_tree_selection_get_selected_rows (selection, &treeModel);
	    } else {
		selection_list = gtk_icon_view_get_selected_items (view->iconView());
	    }
            view->setSelectionList(selection_list);
            if (!selection_list) {
                auto markup = g_strdup_printf("<span size=\"larger\" color=\"blue\">%s\n<span color=\"red\">%s</span></span>\n", _("No selection"),_("No matches."));
                Dialogs<Type>::quickHelp(GTK_WINDOW(mainWindow),markup, "dialog-error"); 
                g_free(markup);
            }

            g_free(response);
            g_regex_unref(regex);
        }
    }


public:
#ifdef ENABLE_FSTAB_MODULE

    static void
    mount(GtkMenuItem *menuItem, gpointer data)
    {
	auto view =  (View<Type> *)g_object_get_data(G_OBJECT(data), "view");
        auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "itemPath");
#ifdef ENABLE_EFS_MODULE
        if (EFS<Type>::isEFS(path)){
            path += strlen("efs:/");
	    TRACE("local/popup.hh::mount()..\n");
            if (!FstabView<Type>::isMounted(path)){ 
                EFS<Type>::doDialog(path, data);
            } else {
                if (!FstabView<Type>::mountPath(view, path, NULL)){
                    ERROR("localpopup.hh:: mount command failed\n");
                } else {
		    // update icon emblem:
		    g_timeout_add(1000, EFS<Type>::changeEfsItem, (void*)view);
		}
            }
            return;
        }
#endif
        if (!FstabView<Type>::mountPath(view, path, NULL)){
            ERROR("localpopup.hh:: mount command failed\n");
        } 
    }
#endif

    static void
    reloadIcons(View<Type> *view){
	if (!BaseSignals<Type>::validBaseView(view)) return;
        auto page = view->page();
        auto viewPath = g_strdup(page->workDir());    
	DBG("local/popup.hh::reloadIcons()\"%s\"\n",viewPath);	
        view->loadModel(viewPath);
	g_free(viewPath);
    }

    static void
    addBookmark(GtkMenuItem *menuItem, gpointer data)
    {
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "path");
        DBG("local/popup.hh::addBookmark():\"%s\"\n",path);
        if (!RootView<Type>::addBookmark(path)) return;
	auto view =  (View<Type> *)g_object_get_data(G_OBJECT(data), "view");
        reloadIcons(view);
    }

    static void
    removeBookmark(GtkMenuItem *menuItem, gpointer data)
    {
        TRACE("Remove bookmark\n");
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "path");
        if (!RootView<Type>::removeBookmark(path)) return;
	auto view =  (View<Type> *)g_object_get_data(G_OBJECT(data), "view");
        reloadIcons(view);
    }



    static void
    newTab(GtkMenu *menu, gpointer data)
    {
        auto dialog = (Dialog<Type> *)g_object_get_data(G_OBJECT(mainWindow), "dialog");
	auto view =  (View<Type> *)g_object_get_data(G_OBJECT(menu), "view");
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data?data:menu), "path");
#ifdef ENABLE_EFS_MODULE
        if (EFS<Type>::isEFS(path)) path += strlen("efs:/");
#endif
	TRACE("localview::newTab path= %s\n", path);
        dialog->addPage(path);

/*	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "path");
	auto view =  (View<Type> *)g_object_get_data(G_OBJECT(data), "view");
	auto page = view->page();
        auto dialog = (Dialog<Type> *)page->parent();
        dialog->addPage(path);*/
    }

    static void
    untar(GtkMenuItem *menuItem, gpointer data)
    {
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "path");
        auto response = getOutputDir(path, _("Extract files from the archive"), "Tarballs");
	TRACE("response=%s\n", response);
	if (response){
            //get currentdir (homedir)
            //change workdir
            errno=0;
            if (chdir(response)==0){
                TRACE("chdir to %s\n", response);   
                gchar * mimetype = Mime<Type>::mimeType(path);
                gchar *format; 
                if (strstr(mimetype, "bzip")) format=g_strdup("-xjf");
                else if (strstr(mimetype, "xz")) format=g_strdup("-xJf");
                else format=g_strdup("-xzf");
                g_free(mimetype);
                gchar *command = g_strdup_printf("tar %s \"%s\"", format, path);
                // execute command...
                // get view
                auto view =  (View<Type> *)g_object_get_data(G_OBJECT(data), "view");
                auto page = view->page();
                pid_t pid = page->command(command, response);

                // open follow dialog for long commands...
                TRACE("command= %s\n", command);
                const gchar *arg[] = {
                    "tar",
                    format,
                    (const gchar *)path,
                    "",
                    NULL
                };
                new (CommandResponse<Type>)(command,"system-run", arg);
                g_free(format);
                g_free(command);
                chdir(g_get_home_dir());

            } else {
                auto m=g_strdup_printf("\n%s: %s\n", response, strerror(errno?errno:ENOENT));
                errno=0;
                Dialogs<Type>::quickHelp(GTK_WINDOW(mainWindow), "dialog-error", m);
                g_free(m);
            }
	    g_free(response);
	}

    }


    static void
    tarball(GtkMenuItem *menuItem, gpointer data)
    {
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "path");
        auto response = getOutputDir(path, _("Create a compressed archive with the selected objects"), "Tarballs", "package-x-generic");
 
	TRACE("response=%s\n", response);
	if (response){
            gchar *basename = g_path_get_basename(path);
            gchar *fmt = g_strdup_printf("tar -cjf \"%s/%s.tar.bz2\"", response, basename);
            gchar *command = Run<Type>::mkCommandLine(fmt, basename);
                
            // execute command...
            // get view
            auto view =  (View<Type> *)g_object_get_data(G_OBJECT(data), "view");
            auto page = view->page();
            pid_t pid = page->command(command);

            // open follow dialog for long commands...
            TRACE("command= %s\n", command);
            auto target = g_strdup_printf("%s/%s.tar.bz2", response, basename);
            const gchar *arg[] = {
                "tar",
                "-cjf",
                (const gchar *)target,
                (const gchar *)basename,
                NULL
            };
            new (CommandResponse<Type>)(command,"system-run", arg);
            g_free(basename);
            g_free(fmt);
            g_free(command);
            g_free(target);
	    g_free(response);
	}

    }
   static void
    command(GtkMenuItem *menuItem, gpointer data)
    {
	auto command = (gchar *)g_object_get_data(G_OBJECT(menuItem), "command");
	// execute command...
        TRACE("command %s\n", command);
	// get view
	auto view =  (View<Type> *)g_object_get_data(G_OBJECT(data), "view");
	// get page
	auto page = view->page();
	page->command(command);
    }

    static void
    newItem(GtkMenuItem *menuItem, gpointer data){
	auto view =  (View<Type> *)g_object_get_data(G_OBJECT(data), "view");
	auto page = view->page();
	auto path = page->workDir();
	auto displayPath = util_c::valid_utf_pathstring(path);
	auto markup = 
	    g_strdup_printf("<span color=\"blue\" size=\"larger\"><b>%s</b></span>", displayPath);  
	g_free(displayPath);
        auto entryResponse = new(EntryResponse<Type>)(GTK_WINDOW(mainWindow), _("Create new..."), NULL);
        entryResponse->setResponseLabel(markup);
        g_free(markup);

        entryResponse->setCheckButton(_("Directory"));
        entryResponse->setCheckButton(TRUE);
        
        entryResponse->setEntryLabel(_("New Name:"));
        // get last used arguments...
        entryResponse->setEntryDefault("");
        auto response = entryResponse->runResponse();
        gboolean isDirectory = 
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(entryResponse->checkButton()));

	if (!response || !strlen(response)){
            Dialogs<Type>::quickHelp(NULL, _("No name")); 
            return;
        }
        // XXX: Will character code set of response match that of path?
        auto newPath = g_build_filename(path, response, NULL);
        if (!g_file_test(newPath, G_FILE_TEST_EXISTS)){
            if (isDirectory){
                mkdir(newPath, 0700);
            } else {
                FILE *f = fopen(newPath, "w");
                if (f) fclose(f);
            }
        } else {
            auto message = g_strdup_printf(_("Another file with the same name already exists in “%s”."), path);
            Dialogs<Type>::quickHelp(NULL, message); 
            g_free(message);
            return;
        }
        g_free(response);
    }


    static void
    runWith(GtkMenuItem *menuItem, gpointer data){
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "path");
        gchar *command = Run<Type>::getRunCommand(mainWindow, path);
        if (not command) return;

	// get view
	auto view =  (View<Type> *)g_object_get_data(G_OBJECT(data), "view");
	//auto baseModel =  (BaseModel<Type> *)g_object_get_data(G_OBJECT(data), "baseModel");
	//auto page = baseModel->page();
	auto page = view->page();
	page->command(command);
	TRACE("*** runWith command = %s\n", command);
	g_free(command);
    }

    static gchar *
    getPath(const gchar *path, const gchar *icon, const gchar *text, const gchar *label){
        auto entryResponse = new(EntryResponse<Type>)(GTK_WINDOW(mainWindow), text, icon);
        auto basename = g_path_get_basename(path);
        entryResponse->setEntryDefault(basename);
        g_free(basename);
        entryResponse->setEntryLabel(label);

	auto view =  (View<Type> *)g_object_get_data(G_OBJECT(localItemPopUp), "view");
	// get page
	auto page = view->page();
	const gchar *wd = page->workDir();
	if (!wd) wd = g_get_home_dir();
	
        entryResponse->setEntryBashFileCompletion(wd);
        entryResponse->setInLineCompletion(1);
        auto response = entryResponse->runResponse();
        return response;
   }

    static gchar *
    getNewPath(const gchar *path, const gchar *icon, const gchar *text, 
            const gchar *label=_("New Name:"))
    {
        auto response = getPath(path, icon, text, label);
        // entryResponse is deleted automatically.
	if (response){
            gchar *newName;
            if (g_path_is_absolute(response)){
                newName = g_strdup(response);
            } else {
                gchar *dirname = g_path_get_dirname(path);
                newName = g_strconcat(dirname, G_DIR_SEPARATOR_S, response, NULL);
                g_free(dirname);
            }
            g_free(response);
            return newName;
        }
        return NULL;
   }

    static gchar *
    getOutputDir (const gchar *path, const gchar *text, const gchar *iniGroup, const gchar *icon=NULL)
    {
        // File chooser
        auto entryResponse = new(EntryFolderResponse<Type>)(GTK_WINDOW(mainWindow), text, icon);        
	auto displayPath = util_c::valid_utf_pathstring(path);
	auto markup = 
	    g_strdup_printf("<span color=\"blue\" size=\"larger\"><b>%s</b></span>", displayPath);  
	g_free(displayPath);
	
        entryResponse->setResponseLabel(markup);
        g_free(markup);

        entryResponse->setEntryLabel(_("Specify Output Directory..."));
        // get last used arguments...
        gchar *dirname = NULL;
	if (Settings<Type>::keyFileHasGroupKey(iniGroup, "Default")){
	    dirname = Settings<Type>::getSettingString(iniGroup, "Default");
	} 
	if (!dirname || !g_file_test(dirname, G_FILE_TEST_IS_DIR) ) {
	    g_free(dirname);
	    dirname = g_path_get_dirname(path);
	}
        entryResponse->setEntryDefault(dirname);
        entryResponse->setEntryBashFileCompletion(dirname);
        entryResponse->setInLineCompletion(1);
        g_free(dirname);
        
        auto response = entryResponse->runResponse();
        if (response){
            response = ckDir(response);
 	    g_strstrip(response);
	    Settings<Type>::setSettingString(iniGroup, "Default", response);
        }
       return response;
    
    
    }

    static gchar *
    ckDir(gchar *response){
        TRACE("ckDir(%s)\n", response);
	if (g_file_test(response, G_FILE_TEST_IS_DIR)) return response;
        if (g_file_test(response, G_FILE_TEST_EXISTS)){
            Dialogs<Type>::quickHelp(mainWindow, _("Not a directory"), 
                    "dialog-error", _("error"));
            g_free(response);
            return NULL;
        } 
        // Create directory?
        auto entryResponse = new(EntryResponse<Type>)(GTK_WINDOW(mainWindow), strerror(ENOTDIR), NULL);
        auto markup = 
            g_strdup_printf("<span color=\"blue\" size=\"larger\"><b>%s</b></span>", _("Create new..."));  
        entryResponse->setResponseLabel(markup);
        g_free(markup);
        entryResponse->setEntryLabel(_("New Name:"));
        entryResponse->setEntryDefault(response);
        g_free(response);
        entryResponse->setEntryBashFileCompletion(g_get_home_dir());
        entryResponse->setInLineCompletion(1);
        response = entryResponse->runResponse();
        if (response && strlen(response)){
            if (mkdir(response, 0750) < 0){
                auto m=g_strdup_printf("\n%s: %s\n", response, strerror(errno));
                errno=0;
                Dialogs<Type>::quickHelp(GTK_WINDOW(mainWindow), "dialog-error", m);
                g_free(m);
                g_free(response);
                return NULL;
            }                     
        }
        return response;      
    } 


    static void 
    properties(GtkMenuItem *menuItem, gpointer data)
    {	
	auto view =  (View<Type> *)g_object_get_data(G_OBJECT(localItemPopUp), "view");
        auto treeModel = view->treeModel();
	GList *selectionList;
	// get selection list
        if (isTreeView){
            auto selection = gtk_tree_view_get_selection (view->treeView());
            selectionList = gtk_tree_selection_get_selected_rows (selection, &treeModel);
        } else {
            selectionList = gtk_icon_view_get_selected_items (view->iconView());
        }
	if (!selectionList) return;

	//
	//
	// fire up a properties dialog
	// selection list will be freed by object 
	// (avoid race)
	//
	new(Properties<Type>)(treeModel, selectionList);

    }

    static void
    symlink(GtkMenuItem *menuItem, gpointer data)
    {	
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "path");
        auto newName = getNewPath(path, "emblem-symbolic-link", _("Link"));
        if (!newName) {
		DBG("symlink() path is null\n");
		return;
	}
         g_strstrip(newName);
        if (strlen(newName)){
            TRACE("*** symlink %s to %s\n", path, newName);
            Gio<Type>::execute(path, newName, MODE_LINK);
        }
        g_free(newName);
    }

    static void
    duplicate(GtkMenuItem *menuItem, gpointer data)
    {	
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "path");
        auto newName = getNewPath(path, "edit-copy", _("Duplicate"));
        if (!newName) {
		DBG("duplicate() path is null\n");
		return;
	}
        g_strstrip(newName);
        if (strlen(newName)){
            TRACE("*** duplicate %s to %s\n", path, newName);
            Gio<Type>::execute(path, newName, MODE_COPY);
        }
        g_free(newName);
    }

    static void
    rename(GtkMenuItem *menuItem, gpointer data)
    {	
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "path");
        auto newName = getNewPath(path, "view-refresh-symbolic", _("Rename"));
        if (!newName)  {
		DBG("rename() path is null\n");
		return;
	}
        g_strstrip(newName);
        if (strlen(newName)){
            TRACE("*** rename %s to %s\n", path, newName);
            Gio<Type>::execute(path, newName, MODE_RENAME);
        }
        g_free(newName);
    }
    
    
    static void
    openWith(GtkMenuItem *menuItem, gpointer data)
    {	
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "path");
        TRACE("openWith menuItem: %s\n", path);

        // get current view
	auto view = (View<Type> *)g_object_get_data(G_OBJECT(data), "view");
        // get selection
        //  single or multiple item selected?
        GList *selectionList;
        if (isTreeView){
            auto treeModel = view->treeModel();
            auto selection = gtk_tree_view_get_selection (view->treeView());
            selectionList = gtk_tree_selection_get_selected_rows (selection, &treeModel);
        } else {
            selectionList = gtk_icon_view_get_selected_items (view->iconView());
        }
        view->setSelectionList(selectionList);
        
        GList *pathList = NULL;
        for(auto l = selectionList; l && l->data; l = l->next) {
            auto tpath = (GtkTreePath *)l->data;
            gchar *path;
            GtkTreeIter iter;
            gtk_tree_model_get_iter (view->treeModel(), &iter, tpath);
            gtk_tree_model_get (view->treeModel(), &iter, PATH, &path, -1);
            pathList = g_list_append(pathList, path);
        }
        
	// get page
	auto page = view->page();
	const gchar *wd = page->workDir();
	if (!wd) wd = g_get_home_dir();
        auto command = Run<Type>::getOpenWithCommand(mainWindow, pathList, wd);

        for(auto l = pathList; l && l->data; l = l->next) { g_free(l->data);}
        g_list_free(pathList);



	if (!command or strlen(command)==0) return;


        TRACE("command line= %s\n", command);
	page->command(command);
	g_free(command);

    }

public:

   static gint
    on_completion (GtkWidget * widget, GdkEventKey * event, gpointer data) {
	auto store = (GtkListStore *)data;
	// get entry text
	auto entry = GTK_ENTRY(widget);
	const gchar *text = gtk_entry_get_text(entry);
	if (!text || strlen(text)<2) return FALSE;

	// Determine if Terminal check button should be depressed
	auto checkButton = GTK_TOGGLE_BUTTON(g_object_get_data(G_OBJECT(entry), "checkButton"));
	gtk_toggle_button_set_active(checkButton, Run<Type>::runInTerminal(text));
	// Hard coded exceptions:
	// nano vi and others...
	if (Run<Type>::fixedInTerminal(text)){
	    gchar *a = Run<Type>::baseCommand(text);
	    gtk_toggle_button_set_active(checkButton, TRUE);
	    Settings<Type>::setSettingInteger("Terminal", a, 1);
	    g_free(a);
	}

	// Get GSlist of bash completion
	// get view
	auto view =  (View<Type> *)g_object_get_data(G_OBJECT(localItemPopUp), "view");
	// get page
	auto page = view->page();
	const gchar *wd = page->workDir();
	if (!wd) wd = g_get_home_dir();
    
	auto slist = BaseCompletion<Type>::baseExecCompletionList(wd, text);
	// remove all old model entries
	gtk_list_store_clear(store);
	// add new entries from GSList
	GSList *p;
	GtkTreeIter iter;
	for (p=slist; p && p->data; p=p->next){
	    TRACE("completion list: %s\n", (const gchar *)p->data);
	    gtk_list_store_append (store, &iter);
	    gtk_list_store_set(store, &iter, 0, (const gchar *)p->data, -1);
	    g_free(p->data);
	}
	g_slist_free(slist);

        auto completion = gtk_entry_get_completion(GTK_ENTRY(widget));
        gtk_entry_completion_complete (completion);
        return FALSE;
    }
private:



  
};
}
#endif

