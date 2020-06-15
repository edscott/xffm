#ifndef PKGPOPUP_HH
# define PKGPOPUP_HH
#include "response/comboresponse.hh"
#include "response/commandresponse.hh"

#ifdef HAVE_EMERGE 
# include "emerge.hh"
#else
# ifdef HAVE_PACMAN
#  include "pacman.hh"
# else
#  include "pkg.hh"
# endif
#endif
namespace xf
{

template <class Type> class PopUp;
template <class Type> class View;
template <class Type> class PkgView;
template <class Type>
class PkgPopUp {
    
    using gtk_c = Gtk<double>;
    using pixbuf_c = Pixbuf<double>;
    using util_c = Util<double>;
    using page_c = Page<Type>;
    static GtkMenu *createPopUp(void){
         menuItem_t item[]={
	    //{N_("Open in New Tab"), (void *)LocalPopUp<Type>::newTab, NULL, NULL},
            {NULL,NULL,NULL, FALSE}};
        auto popup = new(Popup<Type>)(item);
        pkgPopUp = popup->menu();
        popup->changeTitle( _("Software Updater"), PKG_ICON);

        return pkgPopUp;        
    }  

    static GtkMenu *createItemPopUp(void){
	menuItem_t item[]=
        {

	    {N_("Update Database"), (void *)sync, NULL, NULL},
	    {N_("Fetch"), (void *)fetch, NULL, NULL},
	    {N_("Install --dry-run"), (void *)installDry, NULL, NULL},
	    {N_("Install"), (void *)install, NULL, NULL},
	    {N_("Uninstall --dry-run"), (void *)uninstallDry, NULL, NULL},
	    {N_("Uninstall"), (void *)uninstall, NULL, NULL},
	    {N_("Information"), (void *)info, NULL, NULL},
	     {NULL,NULL,NULL,NULL}
        };
        const gchar *key[]={
            "Update Database",
            "Fetch",
            "Install --dry-run",
            "Install",
            "Uninstall --dry-run",
            "Uninstall",
            "Information",
            NULL
        };
        const gchar *keyIcon[]={
            "view-refresh",
            "emblem-downloads",
            "list-add",
            "list-add",
            "list-remove",
            "list-remove",
            "help-faq",
            NULL
        };
        auto popup = new(Popup<Type>)(item, key, keyIcon, TRUE);
        pkgItemPopUp = popup->menu();
        
        return pkgItemPopUp;
    }

    static gchar *
    getStringFull(const gchar *local, const gchar *remote, const gchar *name, gboolean full){
	gchar *command=NULL;
        gchar *comment=NULL;
        if (local){
	    command = g_strdup_printf("%s %s",local, name);
	    if (full) comment = Util<Type>::pipeCommandFull(command);
	    else  comment = Util<Type>::pipeCommand(command);
	    g_free(command);
	}
	TRACE("comment=\"%s\"\n", comment);
	if (!comment || strlen(comment) <2){
	    g_free(comment);
	    comment=NULL;
	    if (remote){
		command = g_strdup_printf("%s %s", remote, name);
		TRACE("remote command: %s\n", command);
		if (full) comment = Util<Type>::pipeCommandFull(command);
		else  comment = Util<Type>::pipeCommand(command);
		g_free(command);
	    }
	    TRACE("remote comment=\"%s\"\n", comment);
	}
	return comment;
    }

    static gchar *
    getString(const gchar *local, const gchar *remote, const gchar *name){
	return getStringFull(local, remote, name, FALSE);
    }

    static void 
    resetItemPopup(View<Type> *view, const gchar *path) {
	TRACE("resetItemPopup pkg\n");
        GtkTreeIter iter;
	if (!gtk_tree_model_get_iter (view->treeModel(), &iter, 
                    (GtkTreePath *)view->selectionList()->data)) 
        {
	    ERROR("pkg/popup.hh::pkgItemPopup: tpath is invalid\n");
	    return;
	}
	gchar *iconName;
	gchar *displayName;
        gtk_tree_model_get (view->treeModel(), &iter, 
		DISPLAY_NAME, &displayName,
		ICON_NAME, &iconName,
		-1);
	if (!path){
	    ERROR("pkg/popup.hh::pkgItemPopup: path is NULL\n");
	    return;
	}

	gchar *version = NULL;
	gchar *comment = NULL;
	gchar *statLine = NULL;

	if (strcmp(path, "xffm:root") && strcmp(path, "xffm:pkg:search")){
	    comment = getString(PKG_COMMENT, PKG_REMOTE_COMMENT,  displayName);
	    if (comment) Util<Type>::lineBreaker(comment, 20);
	    version = getString(PKG_VERSION, PKG_REMOTE_VERSION,  displayName);
	    if (version){
		gchar *g = g_strconcat("\n",_("Version"), " ", version, NULL);
		g_free(version);
		version = g;
	    }
	    statLine = getString(PKG_STATLINE, PKG_REMOTE_STATLINE,  displayName);
	} else {
	    g_free(displayName);
	    if (strcmp(path, "xffm:root")==0) {
		displayName = g_strdup("Xffm+");
		comment = g_strdup(_("Go up"));
	    } else {
		displayName = g_strdup(_("Search"));
		comment = g_strdup(_("Search packages"));
	    }
	}
	
	Util<Type>::resetObjectData(G_OBJECT(pkgItemPopUp), "displayName", displayName);
	Util<Type>::resetObjectData(G_OBJECT(pkgItemPopUp), "mimetype", version);
	Util<Type>::resetObjectData(G_OBJECT(pkgItemPopUp), "iconName", iconName);
	Util<Type>::resetObjectData(G_OBJECT(pkgItemPopUp), "fileInfo", comment);
	Util<Type>::resetObjectData(G_OBJECT(pkgItemPopUp), "statLine", statLine);
	if (comment && strchr(comment, '&')) *(strchr(comment, '&')) = '+';

	
 	// Set title element
	TRACE("Set title element for pkgItemPopUp\n");
	TRACE("displayName=%s\n",displayName);
	TRACE("mimetype=%s\n",version);
	TRACE("iconName=%s\n",iconName);
	TRACE("fileInfo=%s\n",comment);
	TRACE("statLine=%s\n",statLine);
        gchar *markup = g_strdup_printf("<span color=\"red\"><b><i>%s</i></b></span><span color=\"#aa0000\">%s%s</span>\n<span color=\"blue\">%s</span>\n<span color=\"green\">%s</span>", 
                displayName, 
                version?": ":"",
                version?version:"",
                comment?comment:"", 
                statLine?statLine:"");
        Popup<Type>::changeTitle(pkgItemPopUp, markup, iconName);
        g_free(markup);
    }
    static void
    info(GtkMenuItem *menuItem, gpointer data)
    {
        auto view = (View<Type> *)g_object_get_data(G_OBJECT(data), "view");
        auto wait = g_strdup_printf(_("Loading %s...%s"), PKG_TOOLTIPTEXT, _("Please Wait..."));
	view->page()->updateStatusLabel(wait);
	g_free(wait);
	gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), FALSE);
	while (gtk_events_pending())gtk_main_iteration();
	auto displayName =(const gchar *)g_object_get_data(G_OBJECT(pkgItemPopUp), "displayName");
	auto text = getStringFull(PKG_TOOLTIPTEXT, PKG_REMOTE_TOOLTIPTEXT,  displayName, TRUE);
	Print<Type>::clear_text(view->page()->output());
	Print<Type>::showText(view->page()->output());
	Print<Type>::print(view->page()->output(), text);
        while(gtk_events_pending())gtk_main_iteration();
    	Print<Type>::scroll_to_top(view->page()->output());
	gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), TRUE);
	view->page()->updateStatusLabel(NULL);
	
	// This is freed by print...g_free(text);
    }
    static void
    runSimple(GtkMenuItem *menuItem, gpointer data, const gchar *cmd)
    {
	auto displayName =(const gchar *)g_object_get_data(G_OBJECT(pkgItemPopUp), "displayName");
        auto view = (View<Type> *)g_object_get_data(G_OBJECT(data), "view");
	Print<Type>::show_text(view->page()->output());
	view->page()->command(cmd);
    }

    static void
    run(GtkMenuItem *menuItem, gpointer data, const gchar *cmd)
    {
	auto displayName =(const gchar *)g_object_get_data(G_OBJECT(pkgItemPopUp), "displayName");
        auto view = (View<Type> *)g_object_get_data(G_OBJECT(data), "view");
	Print<Type>::show_text(view->page()->output());
	auto command = g_strdup_printf("%s %s", cmd, displayName);
	view->page()->command(command);
	g_free(command);
    }

    static void
    sync(GtkMenuItem *menuItem, gpointer data)
    {
	runSimple(menuItem, data,PKG_REFRESH);	
    }
    


    static void
    fetch(GtkMenuItem *menuItem, gpointer data)
    {
	run(menuItem, data,PKG_FETCH);	
    }

    static void
    install(GtkMenuItem *menuItem, gpointer data)
    {
	run(menuItem, data,PKG_INSTALL);	
    }
    
    static void
    uninstall(GtkMenuItem *menuItem, gpointer data)
    {
	run(menuItem, data,PKG_UNINSTALL);	
    }

    static void
    installDry(GtkMenuItem *menuItem, gpointer data)
    {
	run(menuItem, data,PKG_INSTALL_DRYRUN);	
    }
    
    static void
    uninstallDry(GtkMenuItem *menuItem, gpointer data)
    {
	run(menuItem, data,PKG_UNINSTALL_DRYRUN);	
    }
   
public:
    static GtkMenu *popUp(void){
        if (!pkgPopUp) pkgPopUp = createPopUp();   
        return pkgPopUp;
    }
    static GtkMenu *popUpItem(void){
        if (!pkgItemPopUp) pkgItemPopUp = createItemPopUp();   
	TRACE("pkgItemPopUp=%p\n", pkgItemPopUp);
        return pkgItemPopUp;
    }

    static void
    resetPopup(void){
        auto path = g_object_get_data(G_OBJECT(pkgPopUp), "path");
        if (!path) g_object_set_data(G_OBJECT(pkgPopUp), "path", (void *)g_strdup("xffm:pkg"));
    }

    static void
    resetMenuItems(void) {
        auto view = (View<Type> *)g_object_get_data(G_OBJECT(pkgItemPopUp), "view");
        auto itemPath = (const gchar *)g_object_get_data(G_OBJECT(pkgItemPopUp), "itemPath");
        resetItemPopup(view, itemPath);
        auto displayName = (const gchar *)g_object_get_data(G_OBJECT(pkgItemPopUp), "displayName");

        //Hide all...
        GList *children = gtk_container_get_children (GTK_CONTAINER(pkgItemPopUp));
        for (GList *child = children; child && child->data; child=child->next){
            gtk_widget_hide(GTK_WIDGET(child->data));
        }
        auto v = GTK_WIDGET(g_object_get_data(G_OBJECT(pkgItemPopUp), "title"));
	gtk_widget_show(v);
     
	if (strcmp(itemPath, "xffm:root")==0) {
            auto w = GTK_WIDGET(g_object_get_data(G_OBJECT(pkgItemPopUp), "Go up"));
	    gtk_widget_show(w);	
	    return;	    
	}	
	if (strcmp(itemPath, "xffm:pkg:search")==0) {
            auto w = GTK_WIDGET(g_object_get_data(G_OBJECT(pkgItemPopUp), "Search packages"));
	    gtk_widget_show(w);		
	    return;	    
	}		
	auto version = getString(PKG_VERSION, NULL,  displayName);
	TRACE("menuitems.. %s -> version=\"%s\"\n", displayName, version);
#if defined HAVE_EMERGE ||defined HAVE_PACMAN

	const gchar *items[]={"Install","Install --dry-run","Uninstall","Uninstall --dry-run","Fetch",  NULL};
	for (auto p=items; p && *p; p++){
            auto w = GTK_WIDGET(g_object_get_data(G_OBJECT(pkgItemPopUp), *p));
	    gtk_widget_show(w);
	}
#else
	if (version && strlen(version)>1){
            auto w = GTK_WIDGET(g_object_get_data(G_OBJECT(pkgItemPopUp), "Uninstall"));
	    gtk_widget_show(w);
            w = GTK_WIDGET(g_object_get_data(G_OBJECT(pkgItemPopUp), "Uninstall --dry-run"));
	    gtk_widget_show(w);
	} else {
            auto w = GTK_WIDGET(g_object_get_data(G_OBJECT(pkgItemPopUp), "Install"));
	    gtk_widget_show(w);
            w = GTK_WIDGET(g_object_get_data(G_OBJECT(pkgItemPopUp), "Install --dry-run"));
	    gtk_widget_show(w);
            w = GTK_WIDGET(g_object_get_data(G_OBJECT(pkgItemPopUp), "Fetch"));
	    gtk_widget_show(w);
	}
#endif
        v = GTK_WIDGET(g_object_get_data(G_OBJECT(pkgItemPopUp), "Update Database"));
	gtk_widget_show(v);
        v = GTK_WIDGET(g_object_get_data(G_OBJECT(pkgItemPopUp), "Information"));
	gtk_widget_show(v);
    }
    


};
}
#endif
