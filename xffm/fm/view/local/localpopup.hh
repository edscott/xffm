#ifndef XF_LOCALITEMPOPUP__HH
# define XF_LOCALITEMPOPUP__HH
#include "response/comboresponse.hh"
#include "response/commandresponse.hh"
#include "fm/view/fstab.hh"

namespace xf
{

static GtkMenu *localPopUp=NULL;
static GtkMenu *localItemPopUp=NULL;
template <class Type> class BaseView;
template <class Type>
class LocalPopUp {
    
    using gtk_c = Gtk<Type>;
    using pixbuf_c = Pixbuf<Type>;
    using util_c = Util<Type>;
    using pixbuf_icons_c = Icons<Type>;
    using page_c = Page<Type>;



public:
     static void changeTitle(const gchar *iconName, 
	    const gchar *name, const gchar *path, 
            const gchar *mimetype, const gchar *fileInfo)
    {
	// change title
	auto title = GTK_MENU_ITEM(g_object_get_data(G_OBJECT(localItemPopUp), "title"));
	DBG("changeTitle, title = %p\n",(void *)title);
	gchar *statLine=util_c::statInfo(path);
	gchar *markup = g_strdup_printf("<span size=\"larger\" color=\"red\"><b><i>%s</i></b></span><span color=\"#aa0000\">%s%s</span>\n<span color=\"blue\">%s</span>\n<span color=\"green\">%s</span>", 
		name, 
		mimetype?": ":"",
		mimetype?mimetype:"",
		fileInfo?fileInfo:"no file info", 
		statLine?statLine:"no stat info");
	TRACE("iconName=%s, markup=%s\n", iconName, markup);
	gtk_c::menu_item_content(title, iconName, markup, -48);
	g_free(statLine);
	g_free(markup);
    }

    static GtkMenu *createLocalItemPopUp(void){
	localItemPopUp = GTK_MENU(gtk_menu_new());
	menuItem_t item[]={
	    {("mimetypeOpen"), (void *)command, (void *) localItemPopUp},
	    {N_("Open with"), (void *)openWith, (void *) localItemPopUp},
	    {N_("Run Executable..."), (void *)runWith, (void *) localItemPopUp},
	    {N_("Extract files from the archive"), (void *)noop, (void *) localItemPopUp},
	    {N_("Open in New Tab"), (void *)newTab, (void *) localItemPopUp},
	    {N_("Create a compressed archive with the selected objects"), (void *)tarball, (void *) localItemPopUp},
	    {N_("Mount the volume associated with this folder"), (void *)mount, (void *) localItemPopUp},
	    {N_("Unmount the volume associated with this folder"), (void *)mount, (void *) localItemPopUp},
            {N_("Add bookmark"), (void *)addBookmark, (void *) localItemPopUp},
            {N_("Remove bookmark"), (void *)removeBookmark, (void *) localItemPopUp},

	    
	    
	    //common buttons /(also an iconsize +/- button)
	    {N_("Copy"), (void *)noop, (void *) localItemPopUp},
	    {N_("Cut"), (void *)noop, (void *) localItemPopUp},
	    {N_("Paste"), (void *)noop, (void *) localItemPopUp},
	    {N_("bcrypt"), (void *)noop, (void *) localItemPopUp},
	    {N_("Rename"), (void *)noop, (void *) localItemPopUp},
	    {N_("Duplicate"), (void *)noop, (void *) localItemPopUp},
	    {N_("Link"), (void *)noop, (void *) localItemPopUp},
	    {N_("Touch"), (void *)noop, (void *) localItemPopUp},
	    {N_("File Information..."), (void *)noop, (void *) localItemPopUp},
	    {N_("Properties"), (void *)noop, (void *) localItemPopUp},
	    {N_("Delete"), (void *)noop, (void *) localItemPopUp},
	    //{N_("Mimetype command"), (void *)noop, (void *) localItemPopUp},
	    {N_("autotype_Prun"), (void *)noop, (void *) localItemPopUp},
	     {NULL,NULL,NULL}};
	// Create title element
	GtkWidget *title = gtk_c::menu_item_new(NULL, ""); 
	gtk_widget_set_sensitive(title, FALSE);
	gtk_widget_show (title);
	g_object_set_data(G_OBJECT(localItemPopUp), "title", title);
	gtk_container_add (GTK_CONTAINER (localItemPopUp), title);
	auto p = item;
	for (gint i=0;p && p->label; p++,i++){
	    GtkWidget *v = gtk_c::menu_item_new(NULL, _(p->label));
	    g_object_set_data(G_OBJECT(localItemPopUp), p->label, v);
	    gtk_widget_set_sensitive(v, FALSE);
	    gtk_container_add (GTK_CONTAINER (localItemPopUp), v);
	    g_signal_connect ((gpointer) v, "activate", MENUITEM_CALLBACK (p->callback), p->callbackData);
	    gtk_widget_show (v);
	}
	gtk_widget_show (GTK_WIDGET(localItemPopUp));
	return localItemPopUp;
    }

    static void
    resetLocalItemPopup(GtkTreeModel *treeModel, const GtkTreePath *tpath) {
        // baseView data is set in BaseViewSignals on button press (button=3)
	DBG("resetLocalItemPopup\n");
	gchar *aname=NULL;
	gchar *iconName=NULL;
	gchar *path;
	gchar *mimetype;
	gchar *displayName;
        GtkTreeIter iter;
	GtkTreePath *ttpath = gtk_tree_path_copy(tpath);
	if (!gtk_tree_model_get_iter (treeModel, &iter, ttpath)) {
	    gtk_tree_path_free(ttpath);
	    return;
	}
	gtk_tree_path_free(ttpath);
	gtk_tree_model_get (treeModel, &iter, 
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
	    gtk_list_store_set(GTK_LIST_STORE(treeModel), &iter, 
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
	changeTitle(iconName, name, path, mimetype, fileInfo);
	g_free(name);
	g_free(aname);
	g_free(iconName);
	g_free(fileInfo);
	// this now belongs to data of localItemPopup: g_free(path);
	// this now belongs to localItemPopup: g_free(displayName);
	// this now belongs to localItemPopup: g_free(mimetype);
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
    openWithDialog(const gchar *path, const gchar *mimetype, const gchar *fileInfo){
	// Open with dialog
	auto v2 = GTK_MENU_ITEM(g_object_get_data(G_OBJECT(localItemPopUp), "Open with"));
        gboolean state = g_file_test(path, G_FILE_TEST_IS_REGULAR);
        if (state) {
            gtk_widget_show(GTK_WIDGET(v2));
        } else {
            gtk_widget_hide(GTK_WIDGET(v2));
        }
	gtk_widget_set_sensitive(GTK_WIDGET(v2), state);
	// open with mimetype application
	setUpMimeTypeApp(mimetype, path, fileInfo);
    }

    static void
    showDirectoryItems(const gchar *path){
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


public:
    static void
    resetMenuItems(GtkTreeModel *treeModel, const GtkTreePath *tpath) {
	GtkTreeIter iter;
	GtkTreePath *ttpath = gtk_tree_path_copy(tpath);
	if (!gtk_tree_model_get_iter (treeModel, &iter, ttpath)) {
	    gtk_tree_path_free(ttpath);
	    return;
	}

	gtk_tree_path_free(ttpath);
	gchar *path;
        path = (gchar *)g_object_get_data(G_OBJECT(localItemPopUp), "path");
        g_free(path);
	gchar *mimetype = (gchar *)g_object_get_data(G_OBJECT(localItemPopUp), "mimetype");;
        g_free(mimetype);
	gtk_tree_model_get (treeModel, &iter, 
		MIMETYPE, &mimetype,
		PATH, &path,
	    -1);
        g_object_set_data(G_OBJECT(localItemPopUp), "path", path);
        g_object_set_data(G_OBJECT(localItemPopUp), "mimetype", mimetype);
	struct stat st;
        // Hide all...
        GList *children = gtk_container_get_children (GTK_CONTAINER(localItemPopUp));
        for (GList *child = children; child && child->data; child=child->next){
            gtk_widget_hide(GTK_WIDGET(child->data));
        }
        if (stat(path, &st)<0){
            ERROR("resetMenuItems(): cannot stat %s\n", path);
            return;
        }
            
	auto v2 = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "title"));
        gtk_widget_show(v2);
	
        runWithDialog(path);
        gchar *fileInfo = util_c::fileInfo(path);
        openWithDialog(path, mimetype, fileInfo);
        if ((st.st_mode & S_IFMT) == S_IFDIR) showDirectoryItems(path);
	g_free(fileInfo);
        
    }

    static GtkMenu *popUp(GtkTreeModel *treeModel, const GtkTreePath *tpath){
        if (!localItemPopUp) localItemPopUp = createLocalItemPopUp();   
	resetLocalItemPopup(treeModel, tpath);
	resetMenuItems(treeModel, tpath);
        return localItemPopUp;
    }

    static GtkMenu *popUp(void){
        if (localPopUp) return localPopUp;
        localPopUp = GTK_MENU(gtk_menu_new());
         menuCheckItem_t item[]={
            {N_("Show hidden files"), (void *)toggleItem, 
                (void *) "ShowHidden", "ShowHidden"},
            {N_("Show Backup Files"), (void *)toggleItem, 
                (void *) "ShowBackups", "ShowBackups"},
	    {N_("New"), (void *)newItem, (void *) localPopUp},
            {N_("Open in New Tab"), (void *)noop, (void *) localPopUp},
            {N_("Open in New Window"), (void *)noop, (void *) localPopUp, FALSE},
            
	    {N_("Paste"), (void *)noop, (void *) localPopUp},
             // main menu items
            //{N_("Home"), (void *)noop, (void *) menu},
            //{N_("Open terminal"), (void *)noop, (void *) menu},
            //{N_("About"), (void *)noop, (void *) menu},
            //
            //common buttons /(also an iconsize +/- button)
            //{N_("Paste"), (void *)noop, (void *) menu},
            //{N_("Sort data in ascending order"), (void *)noop, (void *) menu},
            //{N_("Sort data in descending order"), (void *)noop, (void *) menu},
            //{N_("Sort case insensitive"), (void *)noop, (void *) menu},
            
            //{N_("Select All"), (void *)noop, (void *) menu},
            //{N_("Invert Selection"), (void *)noop, (void *) menu},
            //{N_("Unselect"), (void *)noop, (void *) menu},
            //{N_("Select Items Matching..."), (void *)noop, (void *) menu},
            //{N_("Unselect Items Matching..."), (void *)noop, (void *) menu},
            //{N_("Sort by name"), (void *)noop, (void *) menu},
            //{N_("Default sort order"), (void *)noop, (void *) menu},
            //{N_("Sort by date"), (void *)noop, (void *) menu},
            //{N_("Sort by size"), (void *)noop, (void *) menu},
            //{N_("View as list""), (void *)noop, (void *) menu},
            {NULL,NULL,NULL, FALSE}};
        
        auto p = item;
        gint i;
        for (i=0;p && p->label; p++,i++){
            GtkWidget *v;
            if (p->toggleID){
                v = gtk_check_menu_item_new_with_label(_(p->label));
                if (Settings<Type>::getSettingInteger("LocalView", p->toggleID) > 0){
                   gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(v), TRUE);
                } 
            }
            else v = gtk_menu_item_new_with_label (_(p->label));
            gtk_container_add (GTK_CONTAINER (localPopUp), v);
            g_signal_connect ((gpointer) v, "activate", MENUITEM_CALLBACK (p->callback), p->callbackData);
            gtk_widget_show (v);
        }
        gtk_widget_show (GTK_WIDGET(localPopUp));
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
        if (!Fstab<Type>::mount(baseView, path)){
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
        auto dialog = (fmDialog<Type> *)page->parent();
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
        
        //entryResponse->setCheckButtonEntryCallback((void *)toggleTerminalRun, (void *)path); 
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
    noop(GtkMenuItem *menuItem, gpointer data)
    {
        DBG("noop\n");
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
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "PATH");
	auto displayPath = util_c::valid_utf_pathstring(path);
	auto markup = 
	    g_strdup_printf("<span color=\"blue\" size=\"larger\"><b>%s</b></span>", displayPath);  
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
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "PATH");
	auto mimetype = (const gchar *)g_object_get_data(G_OBJECT(data), "MIMETYPE");
        if (!mimetype){
            mimetype = Mime<Type>::mimeType(path);
        }
	gchar *responseLabel = g_strdup_printf("<b><span size=\"larger\" color=\"blue\">%s</span></b>\n<span color=\"#880000\">(%s)</span>", 
		path, mimetype);
	const gchar **apps = Mime<Type>::locate_apps(mimetype);

	gchar *fileInfo = util_c::fileInfo(path);	
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

/*
        auto response = getResponse (_("Open with"),
		responseLabel,_("Open with"),
		_("Run in Terminal"), apps,
		defaultApp, NULL);
                */
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
	// Is the terminal flag set?
	if (Mime<Type>::runInTerminal(response)){
	    command = Mime<Type>::mkTerminalLine(response, path);
	} else {
	    command = Mime<Type>::mkCommandLine(response, path);
	}
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
    activate_entry (GtkEntry * entry, gpointer data) {
	auto dialog = GTK_WIDGET(g_object_get_data(G_OBJECT(entry), "dialog"));
	gtk_dialog_response (GTK_DIALOG(dialog),GTK_RESPONSE_YES);
    }

    static void
    cancel_entry (GtkEntry * entry, gpointer data) {
	GtkWidget *dialog = g_object_get_data(G_OBJECT(entry), "dialog");
	gtk_dialog_response (GTK_DIALOG(dialog),GTK_RESPONSE_CANCEL);
    }

    static gboolean 
    response_delete(GtkWidget *dialog, GdkEvent *event, gpointer data){
	gtk_dialog_response (GTK_DIALOG(dialog),GTK_RESPONSE_CANCEL);
	return TRUE;
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


    static void add_cancel_ok(GtkDialog *dialog){
	// button no
	auto button =
	    gtk_c::dialog_button ("window-close-symbolic", _("Cancel"));
	gtk_widget_show (GTK_WIDGET(button));
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), GTK_WIDGET(button), GTK_RESPONSE_NO);
	g_object_set_data (G_OBJECT (dialog), "action_false_button", button);
	// button yes
	button = gtk_c::dialog_button ("system-run-symbolic", _("Ok"));
	gtk_widget_show (GTK_WIDGET(button));
	g_object_set_data (G_OBJECT (dialog), "action_true_button", button);
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), GTK_WIDGET(button), GTK_RESPONSE_YES);
    }

    static gchar *
    getResponse ( 
	    const gchar *windowTitle,
	    const gchar *title,  
	    const gchar *text,  
	    const gchar *checkboxText,
	    const gchar **comboOptions,
	    const gchar *defaultValue,
	    const gchar *runPath) {
	gchar *response_txt = NULL;
	gint response = GTK_RESPONSE_NONE;
	if(!text) text = "";
	auto dialog = gtk_dialog_new ();
	gtk_window_set_type_hint(GTK_WINDOW(dialog), GDK_WINDOW_TYPE_HINT_DIALOG);

	response_txt = NULL;
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (mainWindow));
	gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);
	gtk_container_set_border_width (GTK_CONTAINER (dialog), 6);

	GtkWidget *title_label=NULL;
	if (title) {
	    title_label = gtk_label_new ("");
	    gtk_label_set_markup(GTK_LABEL(title_label), title);
	}
	
	auto label = GTK_LABEL(gtk_label_new (""));

	if(text) gtk_label_set_markup(label, text);

	auto hbox = gtk_c::hboxNew (TRUE, 6);
	auto vbox = gtk_c::vboxNew (TRUE, 6);
	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area(GTK_DIALOG (dialog))), GTK_WIDGET(vbox), FALSE, FALSE, 0);
	GtkComboBoxText *combo;
	GtkEntry *entry;
	if (comboOptions){
	    combo = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new_with_entry());
	    const gchar **p;
	    for (p=comboOptions; p && *p; p++){
		gtk_combo_box_text_append_text (combo,*p);
		DBG("setting combo value: %s\n" , *p);
	    }
	    if (defaultValue) {
		gtk_combo_box_text_prepend_text (combo,defaultValue);
	    }
	    gtk_combo_box_set_active (GTK_COMBO_BOX(combo),0);
	    entry =  GTK_ENTRY(gtk_bin_get_child(GTK_BIN(combo)));

	} else {
	    entry = GTK_ENTRY(gtk_entry_new ());
	    //if (defaultValue) {
		//gtk_entry_set_text ((GtkEntry *) entry, defaultValue);
	    //}

	}

	// 
	// * for combobox, entry is child of combobox.
	// model must update on keyrelease
	//
	if (comboOptions){
	    auto completion = gtk_entry_completion_new();
	    gtk_entry_set_completion (entry, completion);
	    gtk_entry_completion_set_popup_completion(completion, TRUE);
	    gtk_entry_completion_set_text_column (completion, 0);
	    gtk_entry_completion_set_minimum_key_length (completion, 2);
	    auto completionStore = gtk_list_store_new(1, G_TYPE_STRING);
	    gtk_entry_completion_set_model (completion, GTK_TREE_MODEL(completionStore));
	    g_signal_connect (entry,
			      "key_release_event", 
			      //"key_press_event", 
			      KEY_EVENT_CALLBACK(LocalPopUp<Type>::on_completion), 
			      (void *)completionStore);
	}
			      


	if (title_label){
	    gtk_box_pack_start (GTK_BOX (vbox), title_label, TRUE, TRUE, 0);
	}
	gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET(hbox), FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET(label), TRUE, TRUE, 0);

	if (comboOptions){
	    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET(combo), TRUE, TRUE, 0);
	} else {
	    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET(entry), TRUE, TRUE, 0);
	    g_object_set_data(G_OBJECT(entry),"dialog", dialog);
	    g_signal_connect (G_OBJECT (entry), "activate", G_CALLBACK (activate_entry), (void *)dialog);
	}

	gtk_widget_show_all (GTK_WIDGET(hbox));
	GtkWidget *checkButton = NULL;
	if (checkboxText) { 
	    checkButton = gtk_check_button_new_with_label(checkboxText);
	    gtk_box_pack_start (GTK_BOX (vbox), checkButton, TRUE, TRUE, 0);
	    if (runPath) {
		g_signal_connect (G_OBJECT (checkButton), "clicked", 
		    BUTTON_CALLBACK(toggleTerminalRun), g_strdup(runPath));
	    } else {
		g_signal_connect (G_OBJECT (checkButton), "clicked", 
		    BUTTON_CALLBACK(toggleTerminal), entry);
	    }
	    g_object_set_data(G_OBJECT(entry), "checkButton", (void *)checkButton); 
	    if (defaultValue && Mime<Type>::runInTerminal(defaultValue)){
		    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkButton), TRUE);
	    }
	    if (runPath && Mime<Type>::runInTerminal(runPath)){
		    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkButton), TRUE);
	    }

	    if (comboOptions) g_signal_connect (combo,
			  "changed", 
			  COMBO_CALLBACK(LocalPopUp<Type>::comboChanged), 
			  (void *)checkButton);
	    // signal callback: in keyfile set Terminal.mimetype 0/1
	    // save settings
	    // On execution, check keyfile value
	}

	add_cancel_ok(GTK_DIALOG (dialog));

	gtk_widget_realize (dialog);
	if(windowTitle){
	    // This may or may not work, depending on the window manager.
	    // That is why we duplicate above with markup.
	    gtk_window_set_title (GTK_WINDOW (dialog), windowTitle);
	} else {
	    gdk_window_set_decorations (gtk_widget_get_window(dialog), GDK_DECOR_BORDER);
	}

	g_signal_connect (G_OBJECT (dialog), "delete-event", G_CALLBACK (response_delete), dialog);
	/* show dialog and return */
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
	gtk_widget_show_all (dialog);
	response  = gtk_dialog_run(GTK_DIALOG(dialog));
	if (checkboxText) g_free(g_object_get_data(G_OBJECT(checkButton), "app"));


	if(response == GTK_RESPONSE_YES) {
	    const gchar *et;
	    if (runPath) {
		response_txt = g_strdup_printf("%s %s", runPath, gtk_entry_get_text (entry));
	    }
	    else if (comboOptions){
		response_txt = g_strdup (gtk_combo_box_text_get_active_text (combo));
	    }  else {
		response_txt = g_strdup(gtk_entry_get_text (entry));
	    }
	    //Save option as mimetype default in settings, 
	    //and use it to set entry text.
	    //This is done when function returns, after
	    //checking if response is actually a valid 
	    //answer.
	}
	gtk_widget_hide (dialog);
	gtk_widget_destroy (dialog);
	if(response_txt != NULL){
	    g_strstrip (response_txt);
	}

	return response_txt;
    }
private:



  
};
}
#endif

