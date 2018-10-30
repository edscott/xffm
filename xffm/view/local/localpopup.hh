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
		{N_("Mimetype command"), (void *)noop, (void *) localItemPopUp},
		{N_("autotype_Prun"), (void *)noop, (void *) localItemPopUp},
		{N_("Open with"), (void *)noop, (void *) localItemPopUp},
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

	// get mimetype app from hashtable (FIXME)
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
	    //gtk_c::menu_item_content(v, p?icon:"accessories-text-editor-symbolic", markup, -24);
	    auto command = (gchar *)g_object_get_data(G_OBJECT(v), "command");
	    g_free(command);

	    
	    command = g_strdup_printf("%s \"%s\"", editor, path);
	    g_object_set_data(G_OBJECT(v), "command", command);
                   
	    gtk_widget_show(GTK_WIDGET(v));
	    gtk_widget_set_sensitive(GTK_WIDGET(v), TRUE);
	    
	    g_free(displayName);
	    g_free(icon);
	    g_free(markup);

	} else {
	    auto v = GTK_WIDGET(g_object_get_data(G_OBJECT(localItemPopUp), "mimetypeOpen"));
	    gtk_widget_hide(v);
	}

	changeTitle(iconName, name, path, mimetype, fileInfo);
        g_free (fileInfo);
	g_free(name);
	g_free(iconName);
	g_free(path);
         
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
        
        Dialog<Type>::saveSettings("LocalView", item, value);
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
  
};
}
#endif

