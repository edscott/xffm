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

#define PKG_NO_SELECTION            0x01
#define PKG_SINGLE_SELECTION        0x02
#define PKG_REMOTE_SELECTION        0x04
#define PKG_LOCAL_SELECTION         0x08
#define PKG_ACCESS_WRITE            0x10
#define PKG_ACCESS_READ             0x20
#define PKG_INACTIVE                0x40
#define PKG_LOCAL_REPO              0x80
#define PKG_REMOTE_REPO             0x100
#define PKG_OPTIONS                 0x200
#define PKG_IN_TERM                 0x400
#define PKG_LOCK_DATABASE           0x800
#define PKG_NO_VERSION              0x1000
#define PKG_SCROLL_UP               0x2000
/*
#define NO_SELECTION            0x01
#define SINGLE_SELECTION        0x02
#define REMOTE_SELECTION        0x04
#define LOCAL_SELECTION                0x08
#define ACCESS_WRITE            0x10
#define ACCESS_READ             0x20
#define INACTIVE                0x40
#define LOCAL_REPO              0x80
#define REMOTE_REPO             0x100
#define OPTIONS                 0x200
#define IN_TERM                 0x400
#define LOCK_DATABASE           0x800
#define NO_VERSION                0x1000
#define SCROLL_UP                0x2000
*/
static void
mainStart (GMarkupParseContext * context,
               const gchar * element_name,
               const gchar ** attribute_names, 
           const gchar ** attribute_values, 
           gpointer data, 
           GError ** error) 
{
    DBG ("mainStart -> %s\n",element_name); 

     return;
}
static void mainEnd(GMarkupParseContext *context,
                          const gchar         *element_name,
                          gpointer             user_data,
                          GError             **error){
    DBG ("mainEnd -> %s\n",element_name); 
     ;
 }
  /* text is not nul-terminated */
static void mainText (GMarkupParseContext *context,
                          const gchar         *text,
                          gsize                text_len,
                          gpointer             user_data,
                          GError             **error){
    DBG ("mainText -> %s\n",text); 

}
  /* text is not nul-terminated. */
static void mainPassthrough (GMarkupParseContext *context,
                          const gchar         *passthrough_text,
                          gsize                text_len,
                          gpointer             user_data,
                          GError             **error)
{
    DBG ("mainPassthrough -> %s\n",passthrough_text); 
    
}
static void mainError (GMarkupParseContext *context,
                          GError              *error,
                          gpointer             user_data)
{
    DBG ("mainError -> %s\n",error->message); 
}
GMarkupParser mainParser = {
    mainStart,
    mainEnd, // mainEnd,
    mainText,                   /*text_fun, */
    mainPassthrough,
    mainError
};

static void parseXMLfile(const gchar *xmlFile){
    auto resourcePath = g_build_filename(PREFIX, "share", "xml", "xffm+", xmlFile, NULL);
    if (!g_file_test(resourcePath, G_FILE_TEST_EXISTS)){
        DBG("parseXMLfile(): %s %s\n", resourcePath, strerror(ENOENT));
        g_free(resourcePath);
        resourcePath = g_build_filename(buildXml, xmlFile, NULL);
        if (!g_file_test(resourcePath, G_FILE_TEST_EXISTS)){
            DBG("parseXMLfile(): %s %s\n", resourcePath, strerror(ENOENT));
            g_free(resourcePath);
            return;
        }
    }
    DBG("parseXMLfile(): %s\n", resourcePath);
    auto mainContext = g_markup_parse_context_new (&mainParser, (GMarkupParseFlags)0, NULL, NULL);
    auto input = fopen (resourcePath, "r");
    if(!input) {
        TRACE ("cannot open %s\n", resourcePath);
        return;
    }
    gchar line[2048];
    while(!feof (input) && fgets (line, 2048, input)) {
        memset(line, 0, 2048);
        //fprintf(stderr, "%s", line);
        GError *error = NULL;
        if (!g_markup_parse_context_parse (mainContext, line, strlen(line), &error) )
        {
            DBG("parseXMLfile(): %s\n", error->message);
            g_error_free(error);
        }
    }
    fclose (input);
    
    g_markup_parse_context_free (mainContext);

    g_free(resourcePath);
    
}


namespace xf
{

typedef struct pkg_option_t {
    gchar *loption; 
    gchar *parameter;
    gchar *hlp;
    gchar *active;
}pkg_option_t;

typedef struct pkg_command_t {
    gint flags;     // command flags
    gchar *pkg;     // pkg command 
    gchar *cmd;     // pkg action
    pkg_option_t *pkg_options;
    pkg_option_t *cmd_options;
    gchar *parameter;
    gchar *hlp;     // tooltip help
    gchar *argument;
    gchar *string;
    gchar *icon;

} pkg_command_t;


template <class Type>
class PkgBSD {
    gchar *pkg_;
    gchar *com_;
    gchar *pkg_dbdir_;
    gchar *pkg_cachedir_;  
    gchar *portsdir_;
          
public:
    PkgBSD(void) {
        getDefaults();
        //printDefaults();
    }

    const gchar *title(void){return "foo";}
    const gchar *itemTitle(void){return "bar";}
    void parseXML(void){
        parseXMLfile("pkg_pkg.xml");
    }

private:
    // Get basic pkg program defaults.
    void getDefaults(void){
        pkg_ = g_find_program_in_path("pkg");
        if (!pkg_) throw 1;
        com_ = g_path_get_basename(pkg_);
        pkg_cachedir_ = g_strdup("/var/cache/pkg");
        portsdir_ = g_strdup("/usr/ports");

        const gchar *e = getenv("PKG_DBDIR");
        if (e && strlen(e) && g_file_test(e, G_FILE_TEST_IS_DIR)){
           pkg_dbdir_ = g_strdup(e);
        }
        else pkg_dbdir_ = g_strdup("/var/db/pkg");
        readPkfConf();
    }

    // Overwrite defaults with values in pkg.conf.
    void readPkfConf() {
        FILE *f = fopen("/usr/local/etc/pkg.conf", "r");
            if (f) {
                gchar line[256];
                memset (line, 0, 256);
                while (fgets(line, 255, f) && !feof(f)){
                    g_strchug(line);
                    if (*line == '#') continue;
                    if (strchr(line, '\n'))*strchr(line, '\n') = 0;
                    if (getParam(&pkg_dbdir_, line, "PKG_DB_DIR")) continue;
                    if (getParam(&pkg_cachedir_, line, "PKG_CACHEDIR")) continue;
                    if (getParam(&portsdir_, line, "PORTSDIR")) continue;
                }
                fclose(f);
            }
        }
    
    // Parse pkg.conf line for variable.
    gboolean getParam(gchar **variable, const gchar *line, const gchar *identifier){
        if (strncmp (line, identifier, strlen(identifier))==0){
            g_free(*variable);
            gchar **gg = g_strsplit(line, ":", -1);
            *variable = g_strdup(gg[1]);
            g_strstrip(*variable);
            g_strfreev(gg);
            return TRUE;
        }
        return FALSE;
    }

    void printDefaults(void){
        DBG("pkg_ = %s\n", pkg_);
        DBG("com_ = %s\n", com_);
        DBG("pkg_dbdir_ = %s\n", pkg_dbdir_);
        DBG("pkg_cachedir_ = %s\n", pkg_cachedir_);
        DBG("portsdir_ = %s\n", portsdir_);
    }
};

template <class Type>
class PkgPopUp {

public:
    GtkMenu *popUp(Type *type);
    GtkMenu *popUpItem(Type *type);
    void resetPopup(Type *type);
    void resetMenuItems(Type *type);
    //void parseXML(Type *type);
    
    


private:
    void addMenuTitle(GtkMenu *menu, const gchar *text){
        GtkWidget *title = Gtk<Type>::menu_item_new(NULL, text); 
        gtk_widget_set_sensitive(title, TRUE);
        gtk_widget_show (title);
        g_object_set_data(G_OBJECT(menu), "title", title);
        gtk_container_add (GTK_CONTAINER (menu), title);

    }

};

template <class Type>
GtkMenu *PkgPopUp<Type>::popUp(Type *type){
    if (!pkgPopUp) {
        pkgPopUp = GTK_MENU(gtk_menu_new()); 
        addMenuTitle(pkgPopUp, type->title());
    }
    return pkgPopUp;
}

template <class Type>
GtkMenu *PkgPopUp<Type>::popUpItem(Type *type){
    if (!pkgItemPopUp) {
        pkgItemPopUp = GTK_MENU(gtk_menu_new());
        addMenuTitle(pkgItemPopUp, type->itemTitle());
        type->parseXML();
    }
    DBG("pkgItemPopUp=%p\n", pkgItemPopUp);
    return pkgItemPopUp;
}

template <class Type>
void PkgPopUp<Type>::resetPopup(Type *type){
    DBG("resetPopup=%p\n", pkgItemPopUp);  
}

template <class Type>
void PkgPopUp<Type>::resetMenuItems(Type *type) {
    DBG("resetMenuItems...\n");
}
/*
template <class Type>
void PkgPopUp<Type>::parseXML(Type *type) {
    DBG("parseXML...\n");
    type->parseXML();
}
*/

#if 0
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
#endif

}
#endif
