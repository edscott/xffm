#ifndef XF_LOCALITEMPOPUP__HH
# define XF_LOCALITEMPOPUP__HH
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
	gchar *statLine=util_c::statInfo(path);
	gchar *markup = g_strdup_printf("<span size=\"larger\" color=\"red\"><b><i>%s</i></b></span><span color=\"#aa0000\">%s%s</span>\n<span color=\"blue\">%s</span>\n<span color=\"green\">%s</span>", 
		name, 
		mimetype?": ":"",
		mimetype?mimetype:"",
		fileInfo?fileInfo:"no file info", 
		statLine?statLine:"no stat info");
	gtk_c::menu_item_content(title, iconName, markup, -48);
	g_free(statLine);
	g_free(markup);
    }

    static GtkMenu *popUp(GtkTreeModel *treeModel, GtkTreePath *tpath){
        GtkTreeIter iter;
	if (!gtk_tree_model_get_iter (treeModel, &iter, tpath)) return NULL;
	gchar *aname=NULL;
        gchar *iconName=NULL;
	gchar *path;
	const gchar *mimetype;
	gchar *displayName;
	gtk_tree_model_get (treeModel, &iter, 
		ACTUAL_NAME, &aname,
		DISPLAY_NAME, &displayName,
		ICON_NAME, &iconName,
		MIMETYPE, &mimetype,
		PATH, &path,
		-1);
	if (!mimetype){
	    mimetype = Mime<Type>::mimeType(path); 
	    gtk_list_store_set(GTK_LIST_STORE(treeModel), &iter, 
		MIMETYPE, mimetype, -1);
	}

	    
	/*if (!st){
	    st = (struct stat *)calloc(1, sizeof(struct stat));
	    stat(path, st);
	    //  FIXME
	    // STAT only performed if sort order is date or size
	    // 
	}*/
        gchar *name = util_c::valid_utf_pathstring(aname);
        g_free(aname);

        if (!localItemPopUp) {
	    localItemPopUp = GTK_MENU(gtk_menu_new());
	     menuItem_t item[]={
		{("mimetypeOpen"), (void *)command, (void *) localItemPopUp},
		{N_("Open with"), (void *)openWith, (void *) localItemPopUp},
		{N_("Create a new empty folder inside this folder"), (void *)noop, (void *) localItemPopUp},
		{N_("Open in New Tab"), (void *)noop, (void *) localItemPopUp},
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
		{N_("Mount the volume associated with this folder"), (void *)noop, (void *) localItemPopUp},
		{N_("Unmount the volume associated with this folder"), (void *)noop, (void *) localItemPopUp},
		 {NULL,NULL,NULL}};
	    
	    auto p = item;
	    gint i;
		

	    GtkWidget *title = gtk_c::menu_item_new(iconName, ""); 
	    gtk_widget_set_sensitive(title, FALSE);
	    gtk_widget_show (title);
	    g_object_set_data(G_OBJECT(localItemPopUp), "title", title);
	    gtk_container_add (GTK_CONTAINER (localItemPopUp), title);

	    for (i=0;p && p->label; p++,i++){
		//GtkWidget *v = gtk_menu_item_new_with_label (_(p->label));
		GtkWidget *v = gtk_c::menu_item_new(NULL, _(p->label));
		g_object_set_data(G_OBJECT(localItemPopUp), p->label, v);
		gtk_widget_set_sensitive(v, FALSE);
		gtk_container_add (GTK_CONTAINER (localItemPopUp), v);
		g_signal_connect ((gpointer) v, "activate", MENUITEM_CALLBACK (p->callback), p->callbackData);
		gtk_widget_show (v);
	    }
	    gtk_widget_show (GTK_WIDGET(localItemPopUp));

	}

	auto cleanup = (gchar *) g_object_get_data(G_OBJECT(localItemPopUp), "DISPLAY_NAME");
	g_free(cleanup);
	cleanup = (gchar *) g_object_get_data(G_OBJECT(localItemPopUp), "PATH");
	g_free(cleanup);
	g_object_set_data(G_OBJECT(localItemPopUp), "DISPLAY_NAME", displayName);
	g_object_set_data(G_OBJECT(localItemPopUp), "PATH", path);
	// mimetype is const gchar *
	g_object_set_data(G_OBJECT(localItemPopUp), "MIMETYPE", (void *)mimetype);

	// Open with dialog
	{
	    auto v = GTK_MENU_ITEM(g_object_get_data(G_OBJECT(localItemPopUp), "Open with"));
	    gtk_widget_show(GTK_WIDGET(v));
	    gtk_widget_set_sensitive(GTK_WIDGET(v), TRUE);
	}

	// open with mimetype application
	gchar *fileInfo = util_c::fileInfo(path);
	setUpMimeTypeApp(mimetype, path, fileInfo);


	changeTitle(iconName, name, path, mimetype, fileInfo);
	g_free(name);
	g_free(iconName);
	g_free(fileInfo);
         
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
            
            {N_("Add bookmark"), (void *)noop, (void *) localPopUp, FALSE},
            {N_("Remove bookmark"), (void *)noop, (void *) localPopUp, FALSE},
            {N_("Create a new empty folder inside this folder"), (void *)noop, (void *) localPopUp, FALSE},
            {N_("Open in New Window"), (void *)noop, (void *) localPopUp, FALSE},
            {N_("Reload"), (void *)noop, (void *) localPopUp, FALSE},
            {N_("Close"), (void *)noop, (void *) localPopUp, FALSE},
            // main menu items
            //{N_("Open in New Tab"), (void *)noop, (void *) menu},
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
                if (Dialog<Type>::getSettingInteger("LocalView", p->toggleID) > 0){
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
	gchar *defaultApp = Dialog<Type>::getSettingString("MimeTypeApplications", mimetype);
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
	    command = Mime<Type>::mkCommandLine(defaultApp, path);
	    gchar *markup = g_strdup_printf("<b>%s</b>", command);

	    gchar *icon = g_strdup(defaultApp);
	    g_strstrip(icon);
	    if (strchr(icon, ' ')) *strchr(icon, ' ') = 0;
	    gchar *g = g_path_get_basename(icon);
	    g_free(icon);
	    icon = g;
	    GdkPixbuf *p = pixbuf_c::get_pixbuf(icon, -24); 
	    gboolean iconOK = pixbuf_icons_c::iconThemeHasIcon(icon);
	    gtk_c::menu_item_content(v, iconOK?icon:"system-run-symbolic", markup, -24);
	    

	    //gtk_c::menu_item_content(v, "system-run-symbolic", markup, -24);
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
        if (Dialog<Type>::getSettingInteger("LocalView", item) > 0){
            value = 0;
            gtk_check_menu_item_set_active(menuItem, FALSE);
        } else {
            value = 1;
            gtk_check_menu_item_set_active(menuItem, TRUE);
        }
        auto baseView = (BaseView<Type> *)g_object_get_data(G_OBJECT(localPopUp), "baseView");
        auto path = (const gchar *)g_object_get_data(G_OBJECT(localPopUp), "path");
        
        Dialog<Type>::setSettingInteger("LocalView", item, value);
	Dialog<Type>::writeSettings();
        baseView->loadModel(path);
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
    openWith(GtkMenuItem *menuItem, gpointer data)
    {	
	//auto displayName = (const gchar *)g_object_get_data(G_OBJECT(data), "DISPLAY_NAME");
	//gchar *title = g_strdup_printf("<b>%s <span color=\"blue\">%s</span></b>",_("File"), displayName);
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "PATH");
	auto mimetype = (const gchar *)g_object_get_data(G_OBJECT(data), "MIMETYPE");
	gchar *title = g_strdup_printf("<b><span size=\"larger\" color=\"blue\">%s</span></b>\n<span color=\"#880000\">(%s)</span>", 
		path, mimetype);
	const gchar **apps = Mime<Type>::locate_apps(mimetype);

	gchar *fileInfo = util_c::fileInfo(path);	
	gchar *defaultApp = defaultMimeTypeApp(mimetype, fileInfo);
	g_free(fileInfo);
        auto response = getResponse (_("Open with"),
		title,_("Open with"),
		_("Run in Terminal"), apps,
		defaultApp);
	g_free(defaultApp);
	if (!response) return;

	// Check whether applicacion is valid.
	gboolean valid = Mime<Type>::isValidCommand(response);
	WARN("response = %s, valid=%d\n", response, valid);
	if (!valid){
	    gchar *message = g_strdup_printf("\n<span color=\"#990000\"><b>%s</b></span>:\n <b>%s</b>\n", _("Invalid entry"), response); 
	    gtk_c::quick_help (GTK_WINDOW(mainWindow), message);
	    g_free(message);
	    return;
	}
	// save value as default for mimetype
	Dialog<Type>::setSettingString("MimeTypeApplications", mimetype, response);
	Dialog<Type>::writeSettings();
	gchar *command = Mime<Type>::mkCommandLine(response, path);
	// get baseView
	auto baseView =  (BaseView<Type> *)g_object_get_data(G_OBJECT(data), "baseView");
	auto page = baseView->page();
	page->command(command);
	g_free(command);

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
    getResponse ( const gchar *windowTitle,
	    const gchar *title,  
	    const gchar *text,  
	    const gchar *checkboxText,
	    const gchar **completionOptions,
	    const gchar *defaultValue) {
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
	if (completionOptions){
	    combo = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new_with_entry());
	    const gchar **p;
	    for (p=completionOptions; p && *p; p++){
		gtk_combo_box_text_append_text (combo,*p);
		DBG("setting combo value: %s\n" , *p);
	    }
	    if (defaultValue) {
		gtk_combo_box_text_prepend_text (combo,defaultValue);
	    }
	    gtk_combo_box_set_active (GTK_COMBO_BOX(combo),0);
	} else {
	    entry = GTK_ENTRY(gtk_entry_new ());
	    if (defaultValue) {
		gtk_entry_set_text ((GtkEntry *) entry, defaultValue);
	    }
	}

	if (title_label){
	    gtk_box_pack_start (GTK_BOX (vbox), title_label, TRUE, TRUE, 0);
	}
	gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET(hbox), FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET(label), TRUE, TRUE, 0);

	if (completionOptions){
	    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET(combo), TRUE, TRUE, 0);
	} else {
	    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET(entry), TRUE, TRUE, 0);
	    g_object_set_data(G_OBJECT(entry),"dialog", dialog);
	    g_signal_connect (G_OBJECT (entry), "activate", G_CALLBACK (activate_entry), dialog);
	}

	gtk_widget_show_all (GTK_WIDGET(hbox));
	GtkWidget *checkbox = NULL;
	if (checkboxText) { //FIXME do something with this
	    checkbox = gtk_check_button_new_with_label(checkboxText);
	    gtk_box_pack_start (GTK_BOX (vbox), checkbox, TRUE, TRUE, 0);
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


	if(response == GTK_RESPONSE_YES) {
	    const gchar *et;
	    if (completionOptions){
		et = gtk_combo_box_text_get_active_text (combo);
	    } else {
		et = gtk_entry_get_text (entry);
	    }
	    if(et && strlen (et)) {
		response_txt = g_strdup (et);
	    }
	    //FIXME: save option as mimetype default in settings, and use it to
	    //       set entry text.
	}
	gtk_widget_hide (dialog);
	gtk_widget_destroy (dialog);
	if(response_txt != NULL){
	    g_strstrip (response_txt);
	}

	return response_txt;
    }

  
};
}
#endif

