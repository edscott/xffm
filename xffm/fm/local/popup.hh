#ifndef XF_LOCALITEMPOPUP__HH
# define XF_LOCALITEMPOPUP__HH

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
            
	    {N_("Copy"), (void *)ClipBoard<Type>::copy, NULL, NULL},
	    {N_("Cut"), (void *)ClipBoard<Type>::cut, NULL, NULL},
	    {N_("Paste"), (void *)ClipBoard<Type>::paste, NULL, NULL},
	    {N_("Delete"), (void *)LocalRm<Type>::rm, NULL, NULL},
            //{N_("About"), NULL, (void *) menu},
            //
            //only for listview: {N_("Sort by size"), NULL, (void *) menu},
            //only for listview: {N_("Sort by date"), NULL, (void *) menu},
            
            {N_("Select All"), (void *)selectAll, NULL, NULL},
            {N_("Match regular expression"), (void *)selectMatch, NULL, NULL},
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
	    {N_("Extract files from the archive"), (void *)untar, NULL, NULL},
	    {N_("Open in New Tab"), (void *)newTab, NULL, NULL},
	    {N_("Create a compressed archive with the selected objects"), (void *)tarball, NULL, NULL},
	    {N_("Mount the volume associated with this folder"), (void *)mount, NULL, NULL},
	    {N_("Unmount the volume associated with this folder"), (void *)mount, NULL, NULL},
            {N_("Add bookmark"), (void *)addBookmark, NULL, NULL},
            {N_("Remove bookmark"), (void *)removeBookmark, NULL, NULL},
	    
	    //common buttons /(also an iconsize +/- button)
	    {N_("Copy"), (void *)ClipBoard<Type>::copy, NULL, NULL},
	    {N_("Cut"), (void *)ClipBoard<Type>::cut, NULL, NULL},
	    {N_("Paste"), (void *)ClipBoard<Type>::paste, NULL, NULL},
	    {N_("Delete"), (void *)LocalRm<Type>::rm, NULL, NULL},
	    {N_("Rename"), (void *)rename, NULL, NULL},
	    {N_("Duplicate"), (void *)duplicate, NULL, NULL}, 
	    {N_("Link"), (void *)symlink, NULL, NULL},
	    {N_("Properties"), (void *)properties, NULL, NULL},
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
            "Add bookmark",
            "Remove bookmark",
            "Open in New Tab",
            "Create a compressed archive with the selected objects",
            "Extract files from the archive",
            
            "Mount the volume associated with this folder",
            "Unmount the volume associated with this folder",
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
            "bookmark-new",
            "edit-clear-all",
            "tab-new-symbolic",
            "package-x-generic",
            "insert-object",

            "greenball",
            "redball",

            "edit-cut",
            "edit-copy",
            "edit-paste",
            "edit-delete",

	    "document-revert",
            "document-save",
            "emblem-symbolic-link",
            "document-properties",
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
        auto view = (View<Type> *)g_object_get_data(G_OBJECT(localPopUp), "view");

        // Path is set on buttonpress signal...
        //auto path = (const gchar *)g_object_get_data(G_OBJECT(localPopUp), "path");
	TRACE("resetLocalPopup path=%s\n", view->path());
        if (!view->path()){
	    ERROR("resetLocalPopup: path is NULL\n");
	    return;
	}
        // unsensitivize "Paste" only if valid pasteboard...
        auto w = GTK_WIDGET(g_object_get_data(G_OBJECT(localPopUp), "Paste"));
        if (w) gtk_widget_set_sensitive(w, ClipBoard<Type>::clipBoardIsValid());
        
        w = GTK_WIDGET(g_object_get_data(G_OBJECT(localPopUp), "Copy"));
        if (w) gtk_widget_set_sensitive(w, g_list_length(view->selectionList()) > 0);
        w = GTK_WIDGET(g_object_get_data(G_OBJECT(localPopUp), "Cut"));
        if (w) gtk_widget_set_sensitive(w, g_list_length(view->selectionList()) > 0);
        w = GTK_WIDGET(g_object_get_data(G_OBJECT(localPopUp), "Delete"));
        if (w) {
	    if (g_list_length(view->selectionList()) > 0) gtk_widget_show(w);
	    else gtk_widget_hide(w);
	    gtk_widget_set_sensitive(w, g_list_length(view->selectionList()) > 0);
	} else ERROR(" no widget for Delete\n");

        //w = GTK_WIDGET(g_object_get_data(G_OBJECT(localPopUp), "View as list"));
        //gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w), isTreeView);

	Util<Type>::resetObjectData(G_OBJECT(localPopUp), "iconName", g_strdup("folder"));
	Util<Type>::resetObjectData(G_OBJECT(localPopUp), "displayName", util_c::valid_utf_pathstring(view->path()));
	Util<Type>::resetObjectData(G_OBJECT(localPopUp), "path", g_strdup(view->path()));
	Util<Type>::resetObjectData(G_OBJECT(localPopUp), "mimetype", Mime<Type>::mimeType(view->path()));
	Util<Type>::resetObjectData(G_OBJECT(localPopUp), "fileInfo", util_c::fileInfo(view->path()));
	gchar *statLine;
	if (g_file_test(view->path(), G_FILE_TEST_EXISTS)) statLine  = util_c::statInfo(view->path());
	else statLine = g_strdup(strerror(ENOENT));
	Util<Type>::resetObjectData(G_OBJECT(localPopUp), "statLine", statLine);

	BasePopUp<Type>::changeTitle(localPopUp);
    }

    static void
    resetMenuItems(void) {
        auto view = (View<Type> *)g_object_get_data(G_OBJECT(localItemPopUp), "view");

        //  single or multiple item selected?
        setPath(view);
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
	for (auto k=commonItems; k && *k; k++){
	    auto w = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), *k));
	    if (g_list_length(view->selectionList()) > 0) gtk_widget_show(w);
            else gtk_widget_hide(w);
	    gtk_widget_set_sensitive(w, g_list_length(view->selectionList()) > 0);
	    if (strcmp(*k, "Paste")==0) gtk_widget_set_sensitive(w, FALSE);
	}
	for (auto k=singleSelectItems; k && *k; k++){
	    auto w = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), *k));
	    if (g_list_length(view->selectionList()) == 1) gtk_widget_show(w);
            else gtk_widget_hide(w);
	    gtk_widget_set_sensitive(w, g_list_length(view->selectionList()) == 1);
            
	}
	if (g_list_length(view->selectionList())==1){
	    auto w = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "Extract files from the archive"));
	    if (strstr(mimetype, "compressed-tar")) gtk_widget_show(w);
	    else gtk_widget_hide(w);
	}

	if (g_list_length(view->selectionList()) == 1){
	    runWithDialog(path); // ask for arguments for an executable path.
	    // open with mimetype application
	    setUpMimeTypeApp(mimetype, path, fileInfo);
	    if (g_file_test(path, G_FILE_TEST_IS_DIR)) {
		showDirectoryItems(path);
	    }
	    else openWithDialog(path, mimetype, fileInfo);
	} else {
	    openWithDialog();
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
    openWithDialog(void){
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
            "Mount the volume associated with this folder",
            "Unmount the volume associated with this folder",
	    "Add bookmark",
	    "Remove bookmark",
        NULL};

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

        // unsensitivize "Paste" only if valid pasteboard...
        {
            auto w = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "Paste"));
            if (w) gtk_widget_set_sensitive(w, ClipBoard<Type>::clipBoardIsValid());
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
            w = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "Remove bookmark"));
            gtk_widget_hide(w);
        } else {
            w = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "Remove bookmark"));
            gtk_widget_set_sensitive(w, TRUE);
            w = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "Add bookmark"));
            gtk_widget_hide(w);
        }
	

        // mount options
        if (FstabView<Type>::isMounted(path)){
            w = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "Unmount the volume associated with this folder"));
            gtk_widget_show(w);
            gtk_widget_set_sensitive(w, TRUE);
            w = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "Mount the volume associated with this folder"));
            gtk_widget_hide(w);

        } else {
            w = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "Unmount the volume associated with this folder"));
            gtk_widget_set_sensitive(w, FALSE);
            gtk_widget_hide(w);
            w = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "Mount the volume associated with this folder"));
            gtk_widget_show(w);
            if (FstabView<Type>::isInFstab(path)){
                gtk_widget_show(w);
                gtk_widget_set_sensitive(w, TRUE);
            } else {
                gtk_widget_hide(w);
                gtk_widget_set_sensitive(w, FALSE);
            }
        }

    }

    static void 
    setPath(View<Type> * view){

        GtkTreeIter iter;
        if (g_list_length(view->selectionList()) > 1) {
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
	    Util<Type>::resetObjectData(G_OBJECT(localItemPopUp), "fileInfo", fileInfo);
	    Util<Type>::resetObjectData(G_OBJECT(localItemPopUp), "iconName", g_strdup("edit-copy"));
	    Util<Type>::resetObjectData(G_OBJECT(localItemPopUp), "displayName", g_strdup(_("Multiple selections")));
	    Util<Type>::resetObjectData(G_OBJECT(localItemPopUp), "path", paths);
	    Util<Type>::resetObjectData(G_OBJECT(localItemPopUp), "mimetype", g_strdup(""));
	    Util<Type>::resetObjectData(G_OBJECT(localItemPopUp), "statLine", g_strdup(""));
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
            auto m = Mime<Type>::mimeType(path); 
            mimetype = g_strdup(m); 
            gtk_list_store_set(GTK_LIST_STORE(view->treeModel()), &iter, 
                MIMETYPE, mimetype, -1);
        }

	gchar *fileInfo = util_c::fileInfo(path);
        g_object_set_data(G_OBJECT(localItemPopUp), "fileInfo", fileInfo);
        g_object_set_data(G_OBJECT(localItemPopUp), "iconName", iconName);
        g_object_set_data(G_OBJECT(localItemPopUp), "displayName", displayName);
        g_object_set_data(G_OBJECT(localItemPopUp), "path", path);
        g_object_set_data(G_OBJECT(localItemPopUp), "mimetype", mimetype);
	struct stat st;
	errno=0;
        if (stat(path, &st)<0){
            ERROR("local/popup.hh::resetMenuItems(): cannot stat %s (expect problems) %s\n", path, strerror(errno));
	    errno=0;
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
        auto view = (View<Type> *)data;
        if (localView->isSelectable(treeModel, iter)){
            gtk_icon_view_select_path(view->iconView(), tpath);
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
            gtk_icon_view_select_path(view->iconView(), tpath);
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
        delete entryResponse;
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
                Gtk<Type>::quickHelp(GTK_WINDOW(mainWindow), markup, "dialog-error");
                g_free(markup);
                g_error_free(error);
                return;
            }
            void *arg[]={
                (void *)view, 
                (void *)regex
            };
            // unselect all
            gtk_icon_view_unselect_all(view->iconView());
            gtk_tree_model_foreach (view->treeModel(), selectMatch_, (void *)arg);
            GList *selection_list = gtk_icon_view_get_selected_items (view->iconView());
            view->setSelectionList(selection_list);
            if (!selection_list) {
                gchar *markup = g_strdup_printf("<span size=\"larger\" color=\"blue\">%s\n<span color=\"red\">%s</span></span>\n", _("No selection"),_("No matches."));
                Gtk<Type>::quickHelp(GTK_WINDOW(mainWindow),markup, "dialog-error"); 
                g_free(markup);
            }

            g_free(response);
            g_regex_unref(regex);
        }
    }


public:
    static void
    mount(GtkMenuItem *menuItem, gpointer data)
    {
	auto view =  (View<Type> *)g_object_get_data(G_OBJECT(data), "view");
        auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "path");
        if (!FstabView<Type>::mountPath(view, path, NULL)){
            ERROR("localpopup.hh:: mount command failed\n");
        } 
    }

    static void
    reloadIcons(View<Type> *view){
	if (!BaseSignals<Type>::validBaseView(view)) return;
        auto page = view->page();
        auto viewPath = page->workDir();            
        view->loadModel(viewPath);
    }

    static void
    addBookmark(GtkMenuItem *menuItem, gpointer data)
    {
        TRACE("Add bookmark\n");
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "path");
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
    newTab(GtkMenuItem *menuItem, gpointer data)
    {
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "path");
	auto view =  (View<Type> *)g_object_get_data(G_OBJECT(data), "view");
	auto page = view->page();
        auto dialog = (Dialog<Type> *)page->parent();
        dialog->addPage(path);
    }

    static void
    untar(GtkMenuItem *menuItem, gpointer data)
    {
        // File chooser
        auto entryResponse = new(EntryFolderResponse<Type>)(GTK_WINDOW(mainWindow), _("Extract files from the archive"), NULL);

        
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "path");
	auto displayPath = util_c::valid_utf_pathstring(path);
	auto markup = 
	    g_strdup_printf("<span color=\"blue\" size=\"larger\"><b>%s</b></span>", displayPath);  
	g_free(displayPath);
	
        entryResponse->setResponseLabel(markup);
        g_free(markup);

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
	TRACE("response=%s\n", response);
	if (response){
	    g_strstrip(response);
	    Settings<Type>::setSettingString("Tarballs", "Default", response);
	    if (!g_file_test(response, G_FILE_TEST_IS_DIR)){
		// FIXME dialog 
	    } else {
                auto mimetype = Mime<Type>::mimeType(path);
		//get currentdir (homedir)
		//change workdir
		errno=0;
		if (g_file_test(response, G_FILE_TEST_IS_DIR) && chdir(response)==0){
		    DBG("chdir to %s\n", response);   
		    gchar *format; 
		    if (strstr(mimetype, "bzip")) format=g_strdup("-xjf");
		    else if (strstr(mimetype, "xz")) format=g_strdup("-xJf");
		    else format=g_strdup("-xzf");
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
		    CommandResponse<Type>::dialog(command,"system-run", arg);
		    g_free(format);
		    g_free(command);
		    chdir(g_get_home_dir());

		} else {
		    auto m=g_strdup_printf("\n%s: %s\n", response, strerror(errno?errno:ENOENT));
		    errno=0;
		    Gtk<Type>::quickHelp(GTK_WINDOW(mainWindow), "dialog-error", m);
		    g_free(m);
		}
		//execute
		//restore workdir (homedir)

		gchar *basename = g_path_get_basename(path);
		gchar *fmt = g_strdup_printf("tar -cjf \"%s/%s.tar.bz2\"", response, basename);
		gchar *command = Mime<Type>::mkCommandLine(fmt, basename);
		    
		//FIXME chdir basename and run command in shell
	    }
	    g_free(response);
	}

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
	TRACE("response=%s\n", response);
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
        delete entryResponse;

	if (!response || !strlen(response)){
            gtk_c::quick_help(NULL, _("No name")); 
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
        TRACE("runWith: path = %s\n", path);
	auto displayPath = util_c::valid_utf_pathstring(path);
	auto markup = 
	    g_strdup_printf("<span color=\"blue\" size=\"larger\"><b>%s</b></span>\n<span color=\"red\">(%s)</span>", displayPath, 
		_("Executable"));  
	g_free(displayPath);
	
        auto entryResponse = new(EntryResponse<Type>)(GTK_WINDOW(mainWindow), _("Run Executable..."), "system-run");
        entryResponse->setResponseLabel(markup);
        g_free(markup);

        entryResponse->setCheckButton(_("Run in Terminal"));
        entryResponse->setCheckButton(Mime<Type>::runInTerminal(path));

        entryResponse->setEntryLabel(_("Arguments for the Command"));
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
	// get view
	auto view =  (View<Type> *)g_object_get_data(G_OBJECT(data), "view");
	//auto baseModel =  (BaseModel<Type> *)g_object_get_data(G_OBJECT(data), "baseModel");
	//auto page = baseModel->page();
	auto page = view->page();
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
        if (!newName) return;
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
        if (!newName) return;
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
        if (!newName) return;
        g_strstrip(newName);
        if (strlen(newName)){
            TRACE("*** rename %s to %s\n", path, newName);
            Gio<Type>::execute(path, newName, MODE_MOVE);
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
		mpath, 
		multiple?_("You have selected multiple files or folders"):mimetype);
        for (char *p = mpath; p && *p; p++){
            if (*p=='\n') *p = ' ';
        }

	const gchar **apps = Mime<Type>::locate_apps(mimetype);

	gchar *fileInfo;
        if (multiple) fileInfo = g_strdup("FIXMEfileinfo");
        else fileInfo = util_c::fileInfo(path);	
	gchar *defaultApp = multiple?g_strdup(""):defaultMimeTypeApp(mimetype, fileInfo);
	g_free(fileInfo);
        
	auto view =  (View<Type> *)g_object_get_data(G_OBJECT(localItemPopUp), "view");
	// get page
	auto page = view->page();
	const gchar *wd = page->workDir();
	if (!wd) wd = g_get_home_dir();
        gchar *response = NULL;
        if (!apps || apps[0] == NULL || apps[1] == NULL) {
            auto entryResponse = new(EntryResponse<Type>)(GTK_WINDOW(mainWindow), _("Open with"), NULL);
            entryResponse->setResponseLabel(responseLabel);
            g_free(responseLabel);

            entryResponse->setCheckButton(_("Run in Terminal"));
            if (!multiple) entryResponse->setCheckButton(defaultApp && Mime<Type>::runInTerminal(defaultApp));

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
            if (!multiple) comboResponse->setCheckButton(defaultApp && Mime<Type>::runInTerminal(defaultApp));

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

