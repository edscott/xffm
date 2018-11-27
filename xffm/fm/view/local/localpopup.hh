#ifndef XF_LOCALITEMPOPUP__HH
# define XF_LOCALITEMPOPUP__HH
#include "fm/view/base/basepopup.hh"
#include "response/comboresponse.hh"
#include "response/commandresponse.hh"
#include "fm/view/fstab/fstab.hh"

namespace xf
{

static GtkMenu *localPopUp=NULL;
static GtkMenu *localItemPopUp=NULL;
template <class Type> class BaseView;
template <class Type> class BaseViewSignals;
template <class Type> class Dialog;
template <class Type>
class LocalPopUp {
    
    using gtk_c = Gtk<Type>;
    using pixbuf_c = Pixbuf<Type>;
    using util_c = Util<Type>;
    using pixbuf_icons_c = Icons<Type>;
    using page_c = Page<Type>;



public:


    static GtkMenu *createLocalItemPopUp(void){
	menuItem_t item[]=
        {
	    {("mimetypeOpen"), (void *)command, NULL, NULL},
	    {N_("Open with"), (void *)openWith, NULL, NULL},
	    {N_("Run Executable..."), (void *)runWith, NULL, NULL},
	    {N_("Extract files from the archive"), NULL, NULL, NULL},
	    {N_("Open in New Tab"), (void *)newTab, NULL, NULL},
	    {N_("Create a compressed archive with the selected objects"), (void *)tarball, NULL, NULL},
	    {N_("Mount the volume associated with this folder"), (void *)mount, NULL, NULL},
	    {N_("Unmount the volume associated with this folder"), (void *)mount, NULL, NULL},
            {N_("Add bookmark"), (void *)addBookmark, NULL, NULL},
            {N_("Remove bookmark"), (void *)removeBookmark, NULL, NULL},
	    
	    //common buttons /(also an iconsize +/- button)
	    {N_("Copy"), (void *)copy, NULL, NULL},
	    {N_("Cut"), (void *)cut, NULL, NULL},
	    {N_("Paste"), (void *)paste, NULL, NULL},
	    {N_("bcrypt"), NULL, NULL, NULL},
	    {N_("Rename"), NULL, NULL, NULL},
	    {N_("Duplicate"), NULL, NULL, NULL}, 
	    {N_("Link"), NULL, NULL, NULL},
	    {N_("Touch"), NULL, NULL, NULL},
	    {N_("File Information..."), NULL, NULL, NULL},
	    {N_("Properties"), NULL, NULL, NULL},
	    {N_("Delete"), NULL, NULL, NULL},
	    //{N_("Mimetype command"), NULL, NULL, NULL},
	    {N_("autotype_Prun"), NULL, NULL, NULL},
	     {NULL,NULL,NULL,NULL}
        };
	localItemPopUp = BasePopUp<Type>::createPopup(item); 
        return localItemPopUp;
    }

    static void
    resetLocalPopup(const gchar *path) {
        BasePopUp<Type>::clearKeys(localPopUp);

        // Path is set on buttonpress signal...
        //auto path = (const gchar *)g_object_get_data(G_OBJECT(localPopUp), "path");
	DBG("resetLocalPopup path=%s\n", path);
        if (!path){
	    ERROR("resetLocalPopup: path is NULL\n");
	    return;
	}
	g_object_set_data(G_OBJECT(localPopUp), "iconName", g_strdup("folder"));
	g_object_set_data(G_OBJECT(localPopUp), "displayName", util_c::valid_utf_pathstring(path));
	g_object_set_data(G_OBJECT(localPopUp), "path", g_strdup(path));
	g_object_set_data(G_OBJECT(localPopUp), "mimetype", g_strdup(Mime<Type>::mimeType(path)));
	g_object_set_data(G_OBJECT(localPopUp), "fileInfo", util_c::fileInfo(path));
	BasePopUp<Type>::changeTitle(localPopUp);
    }

/*
    static void
    resetLocalItemPopup(BaseView<Type> *baseView, const GtkTreePath *tpath) {
        // baseView data is set in BaseViewSignals on button press (button=3)
	DBG("resetLocalItemPopup\n");
	gchar *aname=NULL;
	gchar *iconName=NULL;
	gchar *path;
	gchar *mimetype;
	gchar *displayName;
        GtkTreeIter iter;
	GtkTreePath *ttpath = gtk_tree_path_copy(tpath);
	if (!gtk_tree_model_get_iter (baseView->treeModel(), &iter, ttpath)) {
	    gtk_tree_path_free(ttpath);
	    return;
	}
	gtk_tree_path_free(ttpath);
	gtk_tree_model_get (baseView->treeModel(), &iter, 
		ACTUAL_NAME, &aname,
		DISPLAY_NAME, &displayName,
		ICON_NAME, &iconName,
		MIMETYPE, &mimetype,
		PATH, &path,
		-1);
	if (!path){
	    ERROR("resetLocalItemPopup: path is NULL\n");
	    return;
	}
	gchar *fileInfo = util_c::fileInfo(path);
	if (!mimetype){
	    auto m = Mime<Type>::mimeType(path); 
	    mimetype = g_strdup(m); 
	    gtk_list_store_set(GTK_LIST_STORE(baseView->treeModel()), &iter, 
		MIMETYPE, mimetype, -1);
	}

	const gchar *keys[] = {"DISPLAY_NAME",  "PATH", "MIMETYPE", NULL};
	const gchar **q;
	for (q=keys; q && *q; q++){
	    auto cleanup = (gchar *)g_object_get_data(G_OBJECT(localItemPopUp), *q);
	    g_free(cleanup);
	}
	g_object_set_data(G_OBJECT(localItemPopUp), "DISPLAY_NAME", displayName);
	g_object_set_data(G_OBJECT(localItemPopUp), "PATH", path);
	g_object_set_data(G_OBJECT(localItemPopUp), "MIMETYPE", (void *)mimetype);
        gchar *name = util_c::valid_utf_pathstring(aname);
	// Set title element
        BasePopUp<Type>::changeTitle(localItemPopUp, iconName, name, path, mimetype, fileInfo);
	g_free(name);
	g_free(aname);
	g_free(iconName);
	g_free(fileInfo);
	// this now belongs to data of localItemPopup: g_free(path);
	// this now belongs to localItemPopup: g_free(displayName);
	// this now belongs to localItemPopup: g_free(mimetype);
    }
*/
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
    openWithDialog(const gchar *path, const gchar *mimetype, const gchar *fileInfo){
        WARN("openWithDialog(): path=%s\n", path);
        gboolean state;
        if (strchr(path, '\'')){
            gchar **f = g_strsplit(path,"\'", 2);
            state = g_file_test(f[0], G_FILE_TEST_IS_REGULAR);
            WARN("%s is regular: %d\n", f[0], state);
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
	WARN("showDirectoryItems: %s\n", path);
        // Directory items...
        const gchar *directoryItems[] ={
            "title",
            "Open in New Tab",
            "Create a compressed archive with the selected objects",
            "Mount the volume associated with this folder",
            "Unmount the volume associated with this folder",
	    "Add bookmark",
	    "Remove bookmark",
        NULL};
        const gchar *commonItems[]={
            "Copy",
            "Cut",
            "bcrypt",
            "Rename",
            "Duplicate",
            "Link",
            "Touch",
            "Properties",
            "Delete",
            NULL,
        };
        GtkWidget *w;
        // unhide 
        const gchar **p;
        for (p=directoryItems; p &&*p; p++){
            w = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), *p));
            if (w) {
                gtk_widget_show(w);
                gtk_widget_set_sensitive(w, FALSE);
            }
        }
        for (p=commonItems; p &&*p; p++){
            w = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), *p));
            if (w) {
                gtk_widget_show(w);
                gtk_widget_set_sensitive(w, FALSE); // WIP
            }
        }

        //////  Directory options

        // open in new tab
        w = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "Open in New Tab"));
        gtk_widget_set_sensitive(w, TRUE);
        // Create compressed tarball
         w = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "Create a compressed archive with the selected objects"));
        gtk_widget_set_sensitive(w, TRUE);

	// bookmark options
        if (!RootView<Type>::isBookmarked(path)) {
            w = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "Add bookmark"));
            gtk_widget_set_sensitive(w, TRUE);
        } else {
            w = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "Remove bookmark"));
            gtk_widget_set_sensitive(w, TRUE);
        }
	

        // mount options
        if (Fstab<Type>::isMounted(path)){
            w = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "Unmount the volume associated with this folder"));
            gtk_widget_set_sensitive(w, TRUE);
        } else {
            w = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "Unmount the volume associated with this folder"));
            gtk_widget_set_sensitive(w, FALSE);
            w = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "Mount the volume associated with this folder"));
            if (Fstab<Type>::isInFstab(path)){
                gtk_widget_set_sensitive(w, TRUE);
            } else {
                gtk_widget_set_sensitive(w, FALSE);
            }
        }

    }

    static void 
    setPath(BaseView<Type> * baseView, GList *selection_list, const GtkTreePath *tpath){
        BasePopUp<Type>::clearKeys(localItemPopUp);

        GtkTreeIter iter;
        if (g_list_length(selection_list) > 1) {
            gchar *paths = g_strdup("");
            for (GList *l = selection_list; l && l->data; l=l->next){
                if (!gtk_tree_model_get_iter (baseView->treeModel(), &iter, (GtkTreePath *)l->data)){
                    continue;
                }
                gchar *path;
        	gtk_tree_model_get (baseView->treeModel(), &iter, PATH, &path, -1);
                gchar *g = g_strconcat(paths, path, "\'", NULL);
                g_free(paths);
                g_free(path);
                paths = g;
            }
            gchar *fileInfo = g_strdup_printf("%s %d", _("Files:"), g_list_length(selection_list));
            g_object_set_data(G_OBJECT(localItemPopUp), "fileInfo", fileInfo);
            g_object_set_data(G_OBJECT(localItemPopUp), "iconName", g_strdup("edit-copy"));
            g_object_set_data(G_OBJECT(localItemPopUp), "displayName", g_strdup(_("Multiple selections")));
            g_object_set_data(G_OBJECT(localItemPopUp), "path", paths);
            g_object_set_data(G_OBJECT(localItemPopUp), "mimetype", g_strdup(""));
            return;
        }
	struct stat st;
        gchar *path;
        gchar *iconName;
        gchar *mimetype;
        gchar *displayName;
	GtkTreePath *ttpath = gtk_tree_path_copy(tpath);
	if (!gtk_tree_model_get_iter (baseView->treeModel(), &iter, ttpath)) {
	    gtk_tree_path_free(ttpath);
	    return ;
	}
	gtk_tree_path_free(ttpath);
	gtk_tree_model_get (baseView->treeModel(), &iter, 
		//ACTUAL_NAME, &aname,
		DISPLAY_NAME, &displayName,
                MIMETYPE, &mimetype, 
		ICON_NAME, &iconName,
                PATH, &path, 
                -1);
        if (!mimetype){
            auto m = Mime<Type>::mimeType(path); 
            mimetype = g_strdup(m); 
            gtk_list_store_set(GTK_LIST_STORE(baseView->treeModel()), &iter, 
                MIMETYPE, mimetype, -1);
        }

	gchar *fileInfo = util_c::fileInfo(path);
        g_object_set_data(G_OBJECT(localItemPopUp), "fileInfo", fileInfo);
        g_object_set_data(G_OBJECT(localItemPopUp), "iconName", iconName);
        g_object_set_data(G_OBJECT(localItemPopUp), "displayName", displayName);
        g_object_set_data(G_OBJECT(localItemPopUp), "path", path);
        g_object_set_data(G_OBJECT(localItemPopUp), "mimetype", mimetype);
        if (stat(path, &st)<0){
            ERROR("resetMenuItems(): cannot stat %s (expect problems)\n", path);
        }
       return;
    }
public:
    static void
    resetMenuItems(BaseView<Type> *baseView, const GtkTreePath *tpath) {
	GtkTreeIter iter;
	GtkTreePath *ttpath = gtk_tree_path_copy(tpath);
	if (!gtk_tree_model_get_iter (baseView->treeModel(), &iter, ttpath)) {
	    gtk_tree_path_free(ttpath);
	    return;
	}

	gtk_tree_path_free(ttpath);

        //  single or multiple item selected?
        GList *selection_list = gtk_icon_view_get_selected_items (baseView->iconView());
        baseView->setSelectionList(selection_list);
        setPath(baseView, selection_list, tpath);
 	// Set title element
        BasePopUp<Type>::changeTitle(localItemPopUp);

 
        // Hide all...
        GList *children = gtk_container_get_children (GTK_CONTAINER(localItemPopUp));
        for (GList *child = children; child && child->data; child=child->next){
            gtk_widget_hide(GTK_WIDGET(child->data));
        }
            
	auto v2 = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "title"));
        gtk_widget_show(v2);
	
        auto fileInfo =(const gchar *)g_object_get_data(G_OBJECT(localItemPopUp), "fileInfo");
        auto path =(const gchar *)g_object_get_data(G_OBJECT(localItemPopUp), "path");
        auto mimetype =(const gchar *)g_object_get_data(G_OBJECT(localItemPopUp), "mimetype");
        if (g_list_length(selection_list) > 1) {
        } else {
	    fileInfo = util_c::fileInfo(path);
            runWithDialog(path);
            // open with mimetype application
            setUpMimeTypeApp(mimetype, path, fileInfo);
            if (g_file_test(path, G_FILE_TEST_IS_DIR)) showDirectoryItems(path);
        }
        openWithDialog(path, mimetype, fileInfo);
    //  Delete/trash/shred
        const gchar *key[]={
	    "Delete",
	    "Cut",
	    "Copy",
	    NULL
	};
	for (auto k=key; k && *k; k++){
	    auto w = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), *k));
	    gtk_widget_show(w);
	    gtk_widget_set_sensitive(w, TRUE);
	}
    }

    static GtkMenu *popUp(BaseView<Type> *baseView, const GtkTreePath *tpath){
        if (!localItemPopUp) localItemPopUp = createLocalItemPopUp();   
	resetMenuItems(baseView, tpath);
        return localItemPopUp;
    }
    static GtkMenu *popUp(BaseView<Type> *baseView){
        if (!localPopUp) localPopUp = createLocalPopUp();   
        resetLocalPopup(baseView->path());
        return localPopUp;
    }
    static GtkMenu *createLocalPopUp(void){
         menuItem_t item[]={
            {N_("Show hidden files"), (void *)toggleItem, 
                (void *) "ShowHidden", "ShowHidden"},
            {N_("Show Backup Files"), (void *)toggleItem, 
                (void *) "ShowBackups", "ShowBackups"},
	    {N_("New"), (void *)newItem, NULL, NULL},
            {N_("Open in New Tab"), NULL, NULL, NULL},
            {N_("Open in New Window"), NULL, NULL, NULL},
            
	    {N_("Copy"), (void *)copy, NULL, NULL},
	    {N_("Cut"), (void *)cut, NULL, NULL},
	    {N_("Paste"), (void *)paste, NULL, NULL},
             // main menu items
            //{N_("Home"), NULL, (void *) menu},
            //{N_("Open terminal"), NULL, (void *) menu},
            //{N_("About"), NULL, (void *) menu},
            //
            //common buttons /(also an iconsize +/- button)
            //{N_("Paste"), NULL, (void *) menu},
            //{N_("Sort data in ascending order"), NULL, (void *) menu},
            //{N_("Sort data in descending order"), NULL, (void *) menu},
            //{N_("Sort case insensitive"), NULL, (void *) menu},
            
            //{N_("Select All"), NULL, (void *) menu},
            //{N_("Invert Selection"), NULL, (void *) menu},
            //{N_("Unselect"), NULL, (void *) menu},
            //{N_("Select Items Matching..."), NULL, (void *) menu},
            //{N_("Unselect Items Matching..."), NULL, (void *) menu},
            //{N_("Sort by name"), NULL, (void *) menu},
            //{N_("Default sort order"), NULL, (void *) menu},
            //{N_("Sort by date"), NULL, (void *) menu},
            //{N_("Sort by size"), NULL, (void *) menu},
            //{N_("View as list""), NULL, (void *) menu},
            {NULL,NULL,NULL, NULL}};
	localPopUp = BasePopUp<Type>::createPopup(item); 
        return localPopUp;        
    }  

    static gchar *
    defaultMimeTypeApp(const gchar *mimetype, const gchar *fileInfo){
	gchar *defaultApp = Settings<Type>::getSettingString("MimeTypeApplications", mimetype);
	if (!defaultApp) {
	    const gchar **apps = Mime<Type>::locate_apps(mimetype);
	    if (apps && *apps) defaultApp = g_strdup(*apps);
	}

	if (!defaultApp)  {
	    gboolean textMimetype = (mimetype && strncmp(mimetype, "text/", strlen("text/")) == 0);
	    gboolean textFiletype =(fileInfo && 
		    (strstr(fileInfo, "text")||strstr(fileInfo,"empty")));
	    if (textMimetype || textFiletype) {
		gchar *editor = util_c::get_text_editor();
		defaultApp =g_strdup_printf("%s %%s", editor);
		g_free(editor);
	    }
	}
	return defaultApp;
    }

    static void 
    setUpMimeTypeApp(const gchar *mimetype, const gchar *path, const gchar *fileInfo)
    {
	gchar *defaultApp = defaultMimeTypeApp(mimetype, fileInfo);

	auto v = GTK_MENU_ITEM(g_object_get_data(G_OBJECT(localItemPopUp), "mimetypeOpen"));
	if (defaultApp)  {
	    auto v = GTK_MENU_ITEM(g_object_get_data(G_OBJECT(localItemPopUp), "mimetypeOpen"));

	    auto command = (gchar *)g_object_get_data(G_OBJECT(v), "command");
	    g_free(command);
	    if (Mime<Type>::runInTerminal(defaultApp)){
		command = Mime<Type>::mkTerminalLine(defaultApp, path);
	    } else {
		command = Mime<Type>::mkCommandLine(defaultApp, path);
	    }
	    auto displayCommand = Mime<Type>::mkCommandLine(defaultApp, path);
	    auto markup = g_strdup_printf("<b>%s</b>", displayCommand);
	    g_free(displayCommand);

	    auto icon = Mime<Type>::baseIcon(defaultApp);
	    //auto p = pixbuf_c::get_pixbuf(icon, -24); 
	    auto iconOK = pixbuf_icons_c::iconThemeHasIcon(icon);
	    gtk_c::menu_item_content(v, iconOK?icon:"system-run-symbolic", markup, -24);
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

    /*
     	//fallback if above mimetype application fails...
	gboolean textMimetype = (mimetype && strncmp(mimetype, "text/", strlen("text/")) == 0);
	gchar *fileInfo = util_c::fileInfo(path);
        gboolean textFiletype =(fileInfo && 
                (strstr(fileInfo, "text")||strstr(fileInfo,"empty")));
	if (textMimetype || textFiletype) {
	    // environ EDITOR
	    gchar *editor = util_c::get_text_editor();

	    auto v = GTK_MENU_ITEM(g_object_get_data(G_OBJECT(localItemPopUp), "mimetypeOpen"));
	    //gtk_c::menu_item_content(v, "gvim", markup, -24);
	    gchar *icon = g_strdup(editor);
	    g_strstrip(icon);
	    if (strchr(icon, ' ')) *strchr(icon, ' ') = 0;
	    gchar *g = g_path_get_basename(icon);
	    g_free(icon);
	    icon = g;
	    GdkPixbuf *p = pixbuf_c::get_pixbuf(icon, -24);
	    gchar *markup = g_strdup_printf("<b>%s %s</b>", editor, displayName);
	    
	    gboolean iconOK = pixbuf_icons_c::iconThemeHasIcon(icon);
	    gtk_c::menu_item_content(v, iconOK?icon:"accessories-text-editor-symbolic", markup, -24);
	    
	    auto command = (gchar *)g_object_get_data(G_OBJECT(v), "command");
	    g_free(command);

	    command = g_strdup_printf("%s \"%s\"", editor, path);
	    g_object_set_data(G_OBJECT(v), "command", command);
                   
	    gtk_widget_show(GTK_WIDGET(v));
	    gtk_widget_set_sensitive(GTK_WIDGET(v), TRUE);
	    
	    g_free(icon);
	    g_free(markup);
	    g_free(fileInfo);
	    return;
	} 
*/
public:
    static void
    toggleItem(GtkCheckMenuItem *menuItem, gpointer data)
    {
        auto item = (const gchar *)data;
        gint value; 
        if (Settings<Type>::getSettingInteger("LocalView", item) > 0){
            value = 0;
            gtk_check_menu_item_set_active(menuItem, FALSE);
        } else {
            value = 1;
            gtk_check_menu_item_set_active(menuItem, TRUE);
        }
        auto baseView = (BaseView<Type> *)g_object_get_data(G_OBJECT(localPopUp), "baseView");
        auto path = (const gchar *)g_object_get_data(G_OBJECT(localPopUp), "path");
        
        Settings<Type>::setSettingInteger("LocalView", item, value);
        baseView->loadModel(path);
    }

    static void
    mount(GtkMenuItem *menuItem, gpointer data)
    {
	auto baseView =  (BaseView<Type> *)g_object_get_data(G_OBJECT(data), "baseView");
        auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "path");
        if (!Fstab<Type>::mountPath(baseView, path, NULL)){
            DBG("localpopup.hh:: mount command failed\n");
        } 
    }

    static void
    reloadIcons(BaseView<Type> *baseView){
	if (!BaseView<Type>::validBaseView(baseView)) return;
        auto page = baseView->page();
        auto viewPath = page->workDir();            
        baseView->loadModel(viewPath);
    }

    static void
    addBookmark(GtkMenuItem *menuItem, gpointer data)
    {
        TRACE("Add bookmark\n");
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "path");
        if (!RootView<Type>::addBookmark(path)) return;
	auto baseView =  (BaseView<Type> *)g_object_get_data(G_OBJECT(data), "baseView");
        reloadIcons(baseView);
    }

    static void
    removeBookmark(GtkMenuItem *menuItem, gpointer data)
    {
        DBG("Remove bookmark\n");
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "path");
        if (!RootView<Type>::removeBookmark(path)) return;
	auto baseView =  (BaseView<Type> *)g_object_get_data(G_OBJECT(data), "baseView");
        reloadIcons(baseView);
    }



    static void
    newTab(GtkMenuItem *menuItem, gpointer data)
    {
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "path");
	auto baseView =  (BaseView<Type> *)g_object_get_data(G_OBJECT(data), "baseView");
	auto page = baseView->page();
        auto dialog = (Dialog<Type> *)page->parent();
        dialog->addPage(path);
    }


    static void
    tarball(GtkMenuItem *menuItem, gpointer data)
    {
        // File chooser
        auto entryResponse = new(EntryFolderResponse<Type>)(GTK_WINDOW(mainWindow), _("Create a compressed archive with the selected objects"), NULL);

        
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "PATH");
	auto displayPath = util_c::valid_utf_pathstring(path);
	auto markup = 
	    g_strdup_printf("<span color=\"blue\" size=\"larger\"><b>%s</b></span>", displayPath);  
	g_free(displayPath);
	
        entryResponse->setResponseLabel(markup);
        g_free(markup);

        //entryResponse->setCheckButton(_("Run in Terminal"));
        //entryResponse->setCheckButton(Mime<Type>::runInTerminal(path));

        entryResponse->setEntryLabel(_("Specify Output Directory..."));
        // get last used arguments...
        gchar *dirname = NULL;
	if (Settings<Type>::keyFileHasGroupKey("Tarballs", "Default")){
	    dirname = Settings<Type>::getSettingString("Tarballs", "Default");
	} 
	if (!dirname || !g_file_test(dirname, G_FILE_TEST_IS_DIR) ) {
	    g_free(dirname);
	    dirname = g_path_get_dirname(path);
	}
        entryResponse->setEntryDefault(dirname);
        g_free(dirname);
        
        auto response = entryResponse->runResponse();
        delete entryResponse;
	WARN("response=%s\n", response);
	if (response){
	    g_strstrip(response);
	    Settings<Type>::setSettingString("Tarballs", "Default", response);
	    if (!g_file_test(response, G_FILE_TEST_IS_DIR)){
		// FIXME dialog 
	    } else {
		gchar *basename = g_path_get_basename(path);
		gchar *fmt = g_strdup_printf("tar -cjf \"%s/%s.tar.bz2\"", response, basename);
		gchar *command = Mime<Type>::mkCommandLine(fmt, basename);
		    
                // execute command...
                // get baseView
                auto baseView =  (BaseView<Type> *)g_object_get_data(G_OBJECT(data), "baseView");
                auto page = baseView->page();
                pid_t pid = page->command(command);

                // open follow dialog for long commands...
		WARN("command= %s\n", command);
                CommandResponse<Type>::dialog(command,"system-run", pid );
		g_free(basename);
		g_free(fmt);
		g_free(command);
		//FIXME chdir basename and run command in shell
	    }
	    g_free(response);
	}

    }
   static void
    command(GtkMenuItem *menuItem, gpointer data)
    {
	auto command = (gchar *)g_object_get_data(G_OBJECT(menuItem), "command");
	// execute command...
        DBG("command %s\n", command);
	// get baseView
	auto baseView =  (BaseView<Type> *)g_object_get_data(G_OBJECT(data), "baseView");
	// get page
	auto page = baseView->page();
	page->command(command);
	
    }

    static void
    newItem(GtkMenuItem *menuItem, gpointer data){
	auto baseView =  (BaseView<Type> *)g_object_get_data(G_OBJECT(data), "baseView");
	auto page = baseView->page();
	auto path = page->workDir();
	auto displayPath = util_c::valid_utf_pathstring(path);
	auto markup = 
	    g_strdup_printf("<span color=\"blue\" size=\"larger\"><b>%s</b></span>", displayPath);  
	g_free(displayPath);
        auto entryResponse = new(EntryResponse<Type>)(GTK_WINDOW(mainWindow), _("Create new..."), NULL);
        entryResponse->setResponseLabel(markup);
        g_free(markup);

        entryResponse->setCheckButton(_("Directory"));
        entryResponse->setEntryLabel(_("New Name:"));
        // get last used arguments...
        entryResponse->setEntryDefault("");
        auto response = entryResponse->runResponse();
        gboolean isDirectory = 
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(entryResponse->checkButton()));
        delete entryResponse;

	if (!response || !strlen(response)){
            gtk_c::quick_help(NULL, _("No name specified!")); 
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
            gtk_c::quick_help(NULL, message); 
            g_free(message);
            return;
        }
        g_free(response);
    }

    static void
    runWith(GtkMenuItem *menuItem, gpointer data){
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "path");
	auto displayPath = util_c::valid_utf_pathstring(path);
	auto markup = 
	    g_strdup_printf("<span color=\"blue\" size=\"larger\"><b>%s</b></span>\n<span color=\"red\">(%s)</span>", displayPath, 
		_("Item is executable by the user"));  
	g_free(displayPath);
	
        auto entryResponse = new(EntryResponse<Type>)(GTK_WINDOW(mainWindow), _("Run Executable..."), "system-run");
        entryResponse->setResponseLabel(markup);
        g_free(markup);

        entryResponse->setCheckButton(_("Run in Terminal"));
        entryResponse->setCheckButton(Mime<Type>::runInTerminal(path));

        entryResponse->setEntryLabel(_("Custom arguments:"));
        // get last used arguments...
        entryResponse->setEntryDefault("");
        
        entryResponse->setCheckButtonEntryCallback((void *)toggleTerminalRun, (void *)path); 
        auto response = entryResponse->runResponse();
        delete entryResponse;


	if (!response) return;
	// Is the terminal flag set?
	gchar *command ;
	if (Mime<Type>::runInTerminal(path)){
	    command = Mime<Type>::mkTerminalLine(path, response);
	} else {
	    command = Mime<Type>::mkCommandLine(path, response);
	}
        g_free(response);
	// get baseView
	auto baseView =  (BaseView<Type> *)g_object_get_data(G_OBJECT(data), "baseView");
	auto page = baseView->page();
	page->command(command);
	TRACE("2)command = %s\n", command);
	g_free(command);
    }

    static void
    openWith(GtkMenuItem *menuItem, gpointer data)
    {	
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "path");
	auto mimetype = (const gchar *)g_object_get_data(G_OBJECT(data), "mimetype");
        if (!mimetype){
            mimetype = Mime<Type>::mimeType(path);
        }
        gboolean multiple = FALSE;
        gchar *mpath = g_strdup(path);
        for (char *p = mpath; p && *p; p++){
            if (*p=='\'') {
                multiple = TRUE;
                *p = '\n';
            }
        }
	gchar *responseLabel = g_strdup_printf("<b><span size=\"larger\" color=\"blue\">%s</span></b>\n<span color=\"#880000\">(%s)</span>", 
		mpath, mimetype);
        for (char *p = mpath; p && *p; p++){
            if (*p=='\n') *p = ' ';
        }

	const gchar **apps = Mime<Type>::locate_apps(mimetype);

	gchar *fileInfo;
        if (multiple) fileInfo = g_strdup("FIXMEfileinfo");
        else fileInfo = util_c::fileInfo(path);	
	gchar *defaultApp = defaultMimeTypeApp(mimetype, fileInfo);
	g_free(fileInfo);
        
	auto baseView =  (BaseView<Type> *)g_object_get_data(G_OBJECT(localItemPopUp), "baseView");
	// get page
	auto page = baseView->page();
	const gchar *wd = page->workDir();
	if (!wd) wd = g_get_home_dir();
        gchar *response = NULL;
        if (!apps || apps[0] == NULL || apps[1] == NULL) {
            auto entryResponse = new(EntryResponse<Type>)(GTK_WINDOW(mainWindow), _("Open with"), NULL);
            entryResponse->setResponseLabel(responseLabel);
            g_free(responseLabel);

            entryResponse->setCheckButton(_("Run in Terminal"));
            entryResponse->setCheckButton(defaultApp && Mime<Type>::runInTerminal(defaultApp));

            entryResponse->setEntryLabel(_("Open with"));
            entryResponse->setEntryDefault(defaultApp);
            entryResponse->setEntryBashCompletion(wd);
            
            entryResponse->setCheckButtonEntryCallback((void *)toggleTerminal); 
            entryResponse->setEntryCallback((void *)entryKeyRelease); 
            response = entryResponse->runResponse();
            delete entryResponse;
        } else {
            auto comboResponse = new(ComboResponse<Type>)(GTK_WINDOW(mainWindow), _("Open with"), NULL);
            comboResponse->setResponseLabel(responseLabel);
            g_free(responseLabel);

            comboResponse->setCheckButton(_("Run in Terminal"));
            comboResponse->setCheckButton(defaultApp && Mime<Type>::runInTerminal(defaultApp));

            comboResponse->setComboLabel(_("Open with"));
            comboResponse->setComboOptions(apps);
            comboResponse->setComboDefault(defaultApp);
            comboResponse->setComboBashCompletion(wd);

            comboResponse->setCheckButtonComboCallback((void *)toggleTerminal); 
            comboResponse->setComboCallback((void *)comboChanged); 
        
            response = comboResponse->runResponse();
            delete comboResponse;
        }

	g_free(defaultApp);
	if (!response) return;

	// Check whether applicacion is valid.
	gboolean valid = Mime<Type>::isValidCommand(response);
	if (!valid){
	    gchar *message = g_strdup_printf("\n<span color=\"#990000\"><b>%s</b></span>:\n <b>%s</b>\n", _("Invalid entry"), response); 
	    gtk_c::quick_help (GTK_WINDOW(mainWindow), message);
	    g_free(message);
	    return;
	}
	// save value as default for mimetype
	if (strrchr(response,'\n')) *(strrchr(response,'\n')) = 0;
	Settings<Type>::setSettingString("MimeTypeApplications", mimetype, response);
	gchar *command;
        if (!multiple) {
	// Is the terminal flag set?
	    if (Mime<Type>::runInTerminal(response)){
                command = Mime<Type>::mkTerminalLine(response, mpath);
            } else {
                command = Mime<Type>::mkCommandLine(response, mpath);
            }
        } else { // hack
   	    if (Mime<Type>::runInTerminal(response)){
                command = Mime<Type>::mkTerminalLine(response, "");
            } else {
                command = Mime<Type>::mkCommandLine(response, "");
            }
            gchar **f = g_strsplit(path, "\'", -1);
            for (gchar **p=f; p&& *p; p++){
                gchar *g = g_strconcat(command, " ", *p, NULL);
                g_free(command);
                command = g;
            }
        }
        TRACE("command line= %s\n", command);
	page->command(command);
	g_free(command);

    }

public:
   
    static void
    comboChanged (GtkComboBox *combo, gpointer data){
        auto comboResponse = (ComboResponse<Type> *)data;
	auto checkButton = GTK_TOGGLE_BUTTON(comboResponse->checkButton());
        auto entry = comboResponse->comboEntry();
	const gchar *text = gtk_entry_get_text(entry);
	gtk_toggle_button_set_active(checkButton, Mime<Type>::runInTerminal(text));
    }

    static void
    entryKeyRelease (GtkWidget *widget, GdkEvent  *event, gpointer data){
        auto entryResponse = (EntryResponse<Type> *)data;
	auto checkButton = GTK_TOGGLE_BUTTON(entryResponse->checkButton());
        auto entry = GTK_ENTRY(widget);
	const gchar *text = gtk_entry_get_text(entry);
	gtk_toggle_button_set_active(checkButton, Mime<Type>::runInTerminal(text));
    }

   static gint
    on_completion (GtkWidget * widget, GdkEventKey * event, gpointer data) {
	auto store = (GtkListStore *)data;
	// get entry text
	auto entry = GTK_ENTRY(widget);
	const gchar *text = gtk_entry_get_text(entry);
	if (!text || strlen(text)<2) return FALSE;

	// Determine if Terminal check button should be depressed
	auto checkButton = GTK_TOGGLE_BUTTON(g_object_get_data(G_OBJECT(entry), "checkButton"));
	gtk_toggle_button_set_active(checkButton, Mime<Type>::runInTerminal(text));
	// Hard coded exceptions:
	// nano vi and others...
	if (Mime<Type>::fixedInTerminal(text)){
	    gchar *a = Mime<Type>::baseCommand(text);
	    gtk_toggle_button_set_active(checkButton, TRUE);
	    Settings<Type>::setSettingInteger("Terminal", a, 1);
	    g_free(a);
	}

	// Get GSlist of bash completion
	// get baseView
	auto baseView =  (BaseView<Type> *)g_object_get_data(G_OBJECT(localItemPopUp), "baseView");
	// get page
	auto page = baseView->page();
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
    static void
    pasteClip(GtkClipboard *clipBoard, const gchar *text, gpointer data){
        // FIXME: ensure path utfs are same as os paths
        WARN("pasteClip(target=%s):\n%s\n", (const gchar *)data, text);
    }

    static void
    paste(GtkMenuItem *menuItem, gpointer data) { 
        DBG("paste\n");
        gtk_clipboard_request_text (clipBoard, pasteClip, (void *) "fixme:target");
    }

    static void 
    putInClipBoard(BaseView<Type> *baseView, const gchar *instruction){
         if (!baseView || ! instruction){
            ERROR("baseView||instruction is null\n");
            exit(1);
        }
        DBG("%s\n", instruction); 
        // FIXME: ensure paths are valid utf.
        GList *selection_list = gtk_icon_view_get_selected_items (baseView->iconView());
        baseView->setSelectionList(selection_list);
        gchar *clipData = BaseViewSignals<Type>::getSelectionData(baseView,instruction );
        gtk_clipboard_set_text (clipBoard, clipData, strlen(clipData)+1);
    }

    static void
    copy(GtkMenuItem *menuItem, gpointer data) { 
	auto baseView = (BaseView<Type> *)g_object_get_data(G_OBJECT(data), "baseView");
        putInClipBoard(baseView, "copy");
    }

    static void
    cut(GtkMenuItem *menuItem, gpointer data) { 
	auto baseView = (BaseView<Type> *)g_object_get_data(G_OBJECT(data), "baseView");
        putInClipBoard(baseView, "cut");
    }

    static void 
    toggleTerminal (GtkToggleButton *togglebutton, gpointer data){
	if (!data) return;
	const gchar *app = gtk_entry_get_text(GTK_ENTRY(data));
	// Hard coded exceptions:
	if (Mime<Type>::fixedInTerminal(app)) {
	    gtk_toggle_button_set_active(togglebutton, TRUE);
	    return;
	}
	
	// if not valid command, do nothing 
	if (!Mime<Type>::isValidCommand(app)) return;
	// Valid command, continue. Get basename 
	gint value;
	if (gtk_toggle_button_get_active(togglebutton)) value = 1; else value = 0;
	gchar *a = Mime<Type>::baseCommand(app);
	Settings<Type>::setSettingInteger("Terminal", a, value);
	g_free(a);
    }
    
    static void 
    toggleTerminalRun (GtkToggleButton *togglebutton, gpointer data){
	if (!data) {
	    ERROR("toggleTerminalRun: data not set to path\n");
	    return;
	}
	auto path = (const gchar *)data;
	TRACE("runPath = %s\n", path);
	gint value;
	if (gtk_toggle_button_get_active(togglebutton)) value = 1; else value = 0;
	gchar *a = Mime<Type>::baseCommand(path);
	Settings<Type>::setSettingInteger("Terminal", a, value);
	g_free(a);
    }

private:



  
};
}
#endif

