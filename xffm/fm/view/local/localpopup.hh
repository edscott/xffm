#ifndef XF_LOCALITEMPOPUP__HH
# define XF_LOCALITEMPOPUP__HH
#include "fm/model/base/basepopup.hh"
#include "response/comboresponse.hh"
#include "response/commandresponse.hh"
#include "fm/view/fstab/fstab.hh"
#include "localclipboard.hh"

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

static const gchar *generalItems[]={
    "title",
};

namespace xf
{

static GtkMenu *localPopUp=NULL;
static GtkMenu *localItemPopUp=NULL;
template <class Type> class BaseView;
template <class Type> class BaseViewSignals;
template <class Type> class Dialog;
template <class Type> class LocalRm;
template <class Type>
class LocalPopUp {
    
    using gtk_c = Gtk<Type>;
    using pixbuf_c = Pixbuf<Type>;
    using util_c = Util<Type>;
    using pixbuf_icons_c = Icons<Type>;
    using page_c = Page<Type>;

public:

    static GtkMenu *createLocalPopUp(void){
         menuItem_t item[]={
	    {N_("New"), (void *)newItem, NULL, NULL},
            // deprecated {N_("Open in New Tab"), NULL, NULL, NULL},
            // deprecated {N_("Open in New Window"), NULL, NULL, NULL},
            
	    {N_("Copy"), (void *)LocalClipBoard<Type>::copy, NULL, NULL},
	    {N_("Cut"), (void *)LocalClipBoard<Type>::cut, NULL, NULL},
	    {N_("Paste"), (void *)LocalClipBoard<Type>::paste, NULL, NULL},
	    {N_("Delete"), (void *)LocalRm<Type>::rm, NULL, NULL},
            //{N_("About"), NULL, (void *) menu},
            //
            //only for listview: {N_("Sort by size"), NULL, (void *) menu},
            //only for listview: {N_("Sort by date"), NULL, (void *) menu},
            
            {N_("Select All"), (void *)selectAll, NULL, NULL},
            {N_("Select Items Matching..."), (void *)selectMatch, NULL, NULL},
            {N_("View as list"), (void *)toggleView, 
		(void *)"TreeView", "window"},
            {N_("Show hidden files"), (void *)toggleItem, 
                (void *) "ShowHidden", "LocalView"},
            {N_("Show Backup Files"), (void *)toggleItem, 
                (void *) "ShowBackups", "LocalView"},
            {N_("Sort data in descending order"), (void *)toggleItem, 
                (void *) "Descending", "LocalView"},
            {NULL,NULL,NULL, NULL}};
	localPopUp = BasePopUp<Type>::createPopup(item); 
        decorateEditItems(localPopUp);
        return localPopUp;        
    }  

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
	    {N_("Copy"), (void *)LocalClipBoard<Type>::copy, NULL, NULL},
	    {N_("Cut"), (void *)LocalClipBoard<Type>::cut, NULL, NULL},
	    {N_("Paste"), (void *)LocalClipBoard<Type>::paste, NULL, NULL},
	    {N_("Delete"), (void *)LocalRm<Type>::rm, NULL, NULL},
	    {N_("Rename"), (void *)rename, NULL, NULL},
	    {N_("Duplicate"), (void *)duplicate, NULL, NULL}, 
	    {N_("Link"), (void *)symlink, NULL, NULL},
	    {N_("Properties"), NULL, NULL, NULL},
	     {NULL,NULL,NULL,NULL}
        };
	localItemPopUp = BasePopUp<Type>::createPopup(item); 
        // Customize most important menu items
        GtkMenuItem *mItem;
        gchar *markup;
        mItem = (GtkMenuItem *)g_object_get_data(G_OBJECT(localItemPopUp), "Open with");
        markup = g_strdup_printf("<b>%s</b>", _("Open with"));
        gtk_c::menu_item_content(mItem, "system-run", markup, -24);
        g_free(markup);

        const gchar *smallKey[]={
            "Cut",
            "Copy",
            "Paste",
            "Delete",

            "Rename",
            "Duplicate",
            "Link",
            "Properties",
            NULL
        };
        const gchar *smallIcon[]={
            "edit-cut",
            "edit-copy",
            "edit-paste",
            "edit-delete",

	    "view-refresh-symbolic",
            "edit-copy",
            "emblem-symbolic-link",
            "view-refresh-symbolic",
            NULL
        };
        gint i=0;
        for (auto k=smallKey; k && *k; k++, i++){
            mItem = (GtkMenuItem *)g_object_get_data(G_OBJECT(localItemPopUp), *k);
            markup = g_strdup_printf("<span size=\"small\">%s</span>", _(*k));
	    gtk_c::menu_item_content(mItem, smallIcon[i], markup, -16);
	    g_free(markup);
        }


        return localItemPopUp;
    }
    static void
    decorateEditItems(GtkMenu *menu){
        const gchar *key[]={
            "New",// this menuitem is only for nonitem popup
            "Cut",
            "Copy",
            "Paste",
            "Delete",
            NULL
        };
        const gchar *keyIcon[]={
            "document-new",
            "edit-cut",
            "edit-copy",
            "edit-paste",
            "edit-delete",
            NULL
        };
        gint i=0;
        for (auto k=key; k && *k; k++, i++){
            auto mItem = (GtkMenuItem *)g_object_get_data(G_OBJECT(menu), *k);
            auto markup = g_strdup_printf("<span color=\"blue\">%s</span>", _(*k));
	    gtk_c::menu_item_content(mItem, keyIcon[i], markup, -24);
	    g_free(markup);
        }
    }
    static void
    resetLocalPopup(void) {
        auto baseView = (BaseView<Type> *)g_object_get_data(G_OBJECT(localPopUp), "baseView");
        BasePopUp<Type>::clearKeys(localPopUp);

        // Path is set on buttonpress signal...
        //auto path = (const gchar *)g_object_get_data(G_OBJECT(localPopUp), "path");
	DBG("resetLocalPopup path=%s\n", baseView->path());
        if (!baseView->path()){
	    ERROR("resetLocalPopup: path is NULL\n");
	    return;
	}
        // unsensitivize "Paste" only if valid pasteboard...
        auto w = GTK_WIDGET(g_object_get_data(G_OBJECT(localPopUp), "Paste"));
        if (w) gtk_widget_set_sensitive(w, LocalClipBoard<Type>::clipBoardIsValid());
        
        w = GTK_WIDGET(g_object_get_data(G_OBJECT(localPopUp), "Copy"));
        if (w) gtk_widget_set_sensitive(w, g_list_length(baseView->selectionList()) > 0);
        w = GTK_WIDGET(g_object_get_data(G_OBJECT(localPopUp), "Cut"));
        if (w) gtk_widget_set_sensitive(w, g_list_length(baseView->selectionList()) > 0);
        w = GTK_WIDGET(g_object_get_data(G_OBJECT(localPopUp), "Delete"));
        if (w) {
	    if (g_list_length(baseView->selectionList()) > 0) gtk_widget_show(w);
	    else gtk_widget_hide(w);
	    gtk_widget_set_sensitive(w, g_list_length(baseView->selectionList()) > 0);
	} else ERROR(" no widget for Delete\n");

        gboolean isTreeView = (Settings<Type>::getSettingInteger("window", "TreeView") > 0);
        w = GTK_WIDGET(g_object_get_data(G_OBJECT(localPopUp), "View as list"));
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w), isTreeView);


	g_object_set_data(G_OBJECT(localPopUp), "iconName", g_strdup("folder"));
	g_object_set_data(G_OBJECT(localPopUp), "displayName", util_c::valid_utf_pathstring(baseView->path()));
	g_object_set_data(G_OBJECT(localPopUp), "path", g_strdup(baseView->path()));
	g_object_set_data(G_OBJECT(localPopUp), "mimetype", g_strdup(Mime<Type>::mimeType(baseView->path())));
	g_object_set_data(G_OBJECT(localPopUp), "fileInfo", util_c::fileInfo(baseView->path()));
	BasePopUp<Type>::changeTitle(localPopUp);
    }

    static void
    resetMenuItems(void) {
        auto baseView = (BaseView<Type> *)g_object_get_data(G_OBJECT(localItemPopUp), "baseView");

        //  single or multiple item selected?
        setPath(baseView);
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
        if (g_list_length(baseView->selectionList()) > 1) {
        } else {
	    fileInfo = util_c::fileInfo(path);
            runWithDialog(path);
            // open with mimetype application
            setUpMimeTypeApp(mimetype, path, fileInfo);
            if (g_file_test(path, G_FILE_TEST_IS_DIR)) showDirectoryItems(path);
        }
        openWithDialog(path, mimetype, fileInfo);
 
	for (auto k=commonItems; k && *k; k++){
	    auto w = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), *k));
	    gtk_widget_show(w);
            gtk_widget_set_sensitive(w, g_list_length(baseView->selectionList()) > 0);
	}
    }

    static GtkMenu *popUpItem(void){
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
            "Paste",
            "Rename",
            "Duplicate",
            "Link",
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
                auto oldPath = (gchar *)g_object_get_data(G_OBJECT(w), "path");
                g_free(oldPath);
                g_object_set_data(G_OBJECT(w), "path", g_strdup(path));

                gtk_widget_show(w);
                gtk_widget_set_sensitive(w, FALSE);
            }
        }
        for (p=commonItems; p &&*p; p++){
            w = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), *p));
            if (w) {
                auto oldPath = (gchar *)g_object_get_data(G_OBJECT(w), "path");
                g_free(oldPath);
                g_object_set_data(G_OBJECT(w), "path", g_strdup(path));
                gtk_widget_show(w);
                gtk_widget_set_sensitive(w, FALSE); // WIP
            }
        }
        // unsensitivize "Paste" only if valid pasteboard...
        {
            auto w = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "Paste"));
            if (w) gtk_widget_set_sensitive(w, LocalClipBoard<Type>::clipBoardIsValid());
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
    setPath(BaseView<Type> * baseView){
        BasePopUp<Type>::clearKeys(localItemPopUp);

        GtkTreeIter iter;
        if (g_list_length(baseView->selectionList()) > 1) {
            gchar *paths = g_strdup("");
            for (GList *l = baseView->selectionList(); l && l->data; l=l->next){
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
            gchar *fileInfo = g_strdup_printf("%s %d", _("Files:"), g_list_length(baseView->selectionList()));
            g_object_set_data(G_OBJECT(localItemPopUp), "fileInfo", fileInfo);
            g_object_set_data(G_OBJECT(localItemPopUp), "iconName", g_strdup("edit-copy"));
            g_object_set_data(G_OBJECT(localItemPopUp), "displayName", g_strdup(_("Multiple selections")));
            g_object_set_data(G_OBJECT(localItemPopUp), "path", paths);
            g_object_set_data(G_OBJECT(localItemPopUp), "mimetype", g_strdup(""));
            return;
        }

        auto tpath = (GtkTreePath *)baseView->selectionList()->data;
	struct stat st;
        gchar *path;
        gchar *iconName;
        gchar *mimetype;
        gchar *displayName;
	if (!gtk_tree_model_get_iter (baseView->treeModel(), &iter, tpath)) {
	    return ;
	}
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

private:
    static gboolean
    selectAll_(GtkTreeModel *treeModel, GtkTreePath *tpath, GtkTreeIter *iter, gpointer data){
        auto localView = (LocalView<Type> *)data;
        auto baseView = (BaseView<Type> *)data;
        if (localView->isSelectable(treeModel, iter)){
            gtk_icon_view_select_path(baseView->iconView(), tpath);
        }
        return FALSE;        
    }

    static void 
    selectAll(GtkMenuItem *menuItem, gpointer data)
    {
	auto baseView =  (BaseView<Type> *)g_object_get_data(G_OBJECT(data), "baseView");
        gtk_tree_model_foreach (baseView->treeModel(), selectAll_, (void *)baseView);
    }

    static gboolean
    selectMatch_(GtkTreeModel *treeModel, GtkTreePath *tpath, GtkTreeIter *iter, gpointer data){
        auto arg = (void **)data;
        auto baseView = (BaseView<Type> *)(arg[0]);
        auto regex = (const GRegex *)arg[1];
        gchar *path;
        gtk_tree_model_get(treeModel, iter, PATH, &path, -1);
        auto basename = g_path_get_basename(path);
        g_free(path);
        gboolean match = g_regex_match (regex, basename, (GRegexMatchFlags)0, NULL);
        if (match){
            TRACE("match: %s\n", basename);
            gtk_icon_view_select_path(baseView->iconView(), tpath);
        }
        g_free(basename);
        return FALSE;
    }

    static void 
    selectMatch(GtkMenuItem *menuItem, gpointer data)
    {
	auto baseView =  (BaseView<Type> *)g_object_get_data(G_OBJECT(data), "baseView");
        auto entryResponse = new(EntryResponse<Type>)(GTK_WINDOW(mainWindow), _("Select items"), NULL);
	auto markup = 
	    g_strdup_printf("<span color=\"blue\" size=\"larger\"><b>%s</b></span>", _("Select Items Matching"));  
  	
        entryResponse->setResponseLabel(markup);
        g_free(markup);
        entryResponse->setEntryLabel(_("Regular expression"));
        auto response = entryResponse->runResponse();
        delete entryResponse;
	WARN("response=%s\n", response);
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
                Gtk<Type>::quickHelp(GTK_WINDOW(mainWindow), markup, "dialog-error");
                g_free(markup);
                g_error_free(error);
                return;
            }
            void *arg[]={
                (void *)baseView, 
                (void *)regex
            };
            // unselect all
            gtk_icon_view_unselect_all(baseView->iconView());
            gtk_tree_model_foreach (baseView->treeModel(), selectMatch_, (void *)arg);
            GList *selection_list = gtk_icon_view_get_selected_items (baseView->iconView());
            baseView->setSelectionList(selection_list);
            if (!selection_list) {
                gchar *markup = g_strdup_printf("<span size=\"larger\" color=\"blue\">%s\n<span color=\"red\">%s</span></span>\n", _("Nothing selected"),_("No matches."));
                Gtk<Type>::quickHelp(GTK_WINDOW(mainWindow),markup, "dialog-error"); 
                g_free(markup);
            }

            g_free(response);
            g_regex_unref(regex);
        }
    }


public:

   static gboolean
    toggleGroupItem(GtkCheckMenuItem *menuItem, const gchar *group, const gchar *item)
    {
        gboolean value; 
        if (Settings<Type>::getSettingInteger(group, item) > 0){
            value = FALSE;
        } else {
            value = TRUE;
        }
        gtk_check_menu_item_set_active(menuItem, value);
        Settings<Type>::setSettingInteger(group, item, value);
        auto notebook_p = (Notebook<Type> *)g_object_get_data(G_OBJECT(mainWindow), "xffm");
	gint pages = gtk_notebook_get_n_pages (notebook_p->notebook());
	for (int i=0; i<pages; i++){
            auto page = notebook_p->currentPageObject(i);
            auto baseView = page->baseView();
            baseView->reloadModel();
	}
        return value;
    }

   static void
    toggleItem(GtkCheckMenuItem *menuItem, gpointer data)
    {

        auto item = (const gchar *)data;
	toggleGroupItem(menuItem, "LocalView", item);

    }

   static void
    toggleView(GtkCheckMenuItem *menuItem, gpointer data)
    {

        auto item = (const gchar *)data;
	toggleGroupItem(menuItem, "window", item);
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

        
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "path");
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
                auto target = g_strdup_printf("%s/%s.tar.bz2", response, basename);
                const gchar *arg[] = {
                    "tar",
                    "-cjf",
                    (const gchar *)target,
                    (const gchar *)basename,
                    NULL
                };
                CommandResponse<Type>::dialog(command,"system-run", arg);
		g_free(basename);
		g_free(fmt);
		g_free(command);
                g_free(target);
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
        WARN("runWith: path = %s\n", path);
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
	//auto baseView =  (BaseView<Type> *)g_object_get_data(G_OBJECT(data), "baseView");
	auto baseModel =  (BaseModel<Type> *)g_object_get_data(G_OBJECT(data), "baseModel");
	auto page = baseModel->page();
	page->command(command);
	TRACE("2)command = %s\n", command);
	g_free(command);
    }

    static gchar *
    getNewPath(const gchar *path, const gchar *icon, const gchar *text){
        auto entryResponse = new(EntryResponse<Type>)(GTK_WINDOW(mainWindow), text, icon);
        auto basename = g_path_get_basename(path);
        entryResponse->setEntryDefault(basename);
        g_free(basename);
        entryResponse->setEntryLabel(_("New Name:"));
        auto response = entryResponse->runResponse();
        delete entryResponse;
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


    static void
    symlink(GtkMenuItem *menuItem, gpointer data)
    {	
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "path");
        auto newName = getNewPath(path, "emblem-symbolic-link", _("Link"));
        if (!newName) return;
         g_strstrip(newName);
        if (strlen(newName)){
            DBG("*** symlink %s to %s\n", path, newName);
            Gio<Type>::doIt(path, newName, MODE_LINK);
        }
        g_free(newName);
    }

    static void
    duplicate(GtkMenuItem *menuItem, gpointer data)
    {	
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "path");
        auto newName = getNewPath(path, "edit-copy", _("Duplicate"));
        if (!newName) return;
        g_strstrip(newName);
        if (strlen(newName)){
            DBG("*** duplicate %s to %s\n", path, newName);
            Gio<Type>::doIt(path, newName, MODE_COPY);
        }
        g_free(newName);
    }

    static void
    rename(GtkMenuItem *menuItem, gpointer data)
    {	
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "path");
        auto newName = getNewPath(path, "view-refresh-symbolic", _("Rename"));
        if (!newName) return;
        g_strstrip(newName);
        if (strlen(newName)){
            DBG("*** rename %s to %s\n", path, newName);
            Gio<Type>::doIt(path, newName, MODE_MOVE);
        }
        g_free(newName);
    }
    
    static void
    openWith(GtkMenuItem *menuItem, gpointer data)
    {	
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "path");
	auto mimetype = Mime<Type>::mimeType(path);
	// auto mimetype = (const gchar *)g_object_get_data(G_OBJECT(data), "mimetype");
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

