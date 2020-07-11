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
    gchar *parameter;
    gchar *cmd;     // pkg action
    gchar *argument;
    gchar *string;
    gchar *icon;
    gchar *hlp;     // tooltip help
    pkg_option_t *pkg_options;
    pkg_option_t *cmd_options;

} pkg_command_t;

typedef struct RodentCallback {
    gint function_id;
    const gchar *string;
    const gchar *icon;
    gpointer function;
    gpointer data;
    guint key; 
    guint mask; 
    gint type;
}RodentCallback;


typedef struct RodentMenuDefinition{
    gint type;
    const gchar *parent_id;
    const gchar *id;
    RodentCallback callback;
} RodentMenuDefinition;


namespace PKG{
    GList *parentList=NULL;
    GList *nodeList=NULL;
    GtkMenu *pkgPopUp=NULL;
    GtkMenu *pkgItemPopUp=NULL;
    menuItem2_t *mainItems=NULL;
    menuItem2_t *specificItems=NULL;
    RodentMenuDefinition *xml_menu_definitions = NULL;
}


template <class Type>
class PkgPopUp {
    using Xml=XML::Xml;

public:
    GtkMenu *popUp(Type *type);
    GtkMenu *popUpItem(Type *type);
    void resetPopup(Type *type);
    void resetMenuItems(Type *type);
    //void parseXML(Type *type);

    static void parseXMLfile(const gchar *xmlFile, const gchar *tag){
        auto xmlStructure = new(XML::XmlStructure)(xmlFile);
        // PopulateStructures will create PKG::nodeList 
        //populateStructures(xmlStructure, tag);
        GList *optionsList = xmlStructure->getList(tag, "option");
        GList *actionsList = xmlStructure->getList(tag, "action");
        GList *actionOptionsList = xmlStructure->getList("action", "option");
        // create popup menus.
        createMainMenu(xmlStructure, actionsList, actionOptionsList);
        createSpecificMenu(xmlStructure, actionsList);
        for (auto l=actionsList; l && l->data; l=l->next){
            auto node = (XML::XmlNode *)l->data;
            auto cmd = xmlStructure->getAttribute(node, "cmd");
            DBG("-> %s: %s\n", cmd, node->text);
        }
            

        g_list_free(optionsList);
        g_list_free(actionsList);
        g_list_free(actionOptionsList);


    }
private:

    static void 
    populateStructures(XML::XmlStructure *xmlStructure, const gchar *tag){
        gint count;
        
        // We need this to create the menus (based on actions)
        const gchar *data2[]={tag, "action", NULL}; 
        //auto countActions = countSweepItems(xmlStructure,(void *)data2);
/*      
        // We need this later on to get the options for each action.
        const gchar *data[]={tag, "option", NULL};
        count = countChildlessItems(xmlStructure, (void *)data);
        //auto pkgOptions = globalOptions(PKG::nodeList);

        const gchar *data2[]={tag, "action", NULL}; 
        auto countActions = countSweepItems(xmlStructure,(void *)data2);
*/
        /*const gchar *data3[]={"action", "option", NULL}; 
        count = countSweepItems(xmlStructure, (void *)data3);
        auto countActionParents = g_list_length(PKG::parentList);
        */


    }
       
    //We need GList for options (currently have), and GList for actions 
    //in simultaneous mode.
    //Update window icon
    
    static void createSpecificMenu(XML::XmlStructure *xmlStructure, 
            GList *list){

        TRACE("createSpecificMenu...\n");
        const gchar *tag[2]={"local", "remote"};
        GList *specificList = getActiveList(xmlStructure, list, "icon", tag);

        PKG::specificItems =(menuItem2_t *)calloc(g_list_length(specificList)+1, sizeof(menuItem2_t));
        if (!PKG::specificItems){
            DBG("createSpecificMenu(): calloc: %s\n", strerror(errno));
            throw(1);
        }
        
        auto t = PKG::specificItems;
        for(auto l = specificList; l && l->data; l = l->next, t++){
            auto node = (XML::XmlNode *)l->data;
            t->icon = xmlStructure->getAttribute(node, "icon");
            auto cmd = xmlStructure->getAttribute(node, "cmd");
            auto pkg = xmlStructure->getAttribute(node, "pkg");
            // Not for actions:
            // auto text = xmlStructure->getText(node);
            auto tooltip = xmlStructure->getAttribute(node, "tooltip");
            if (cmd) {
                t->label=g_strdup_printf("  <i>%s %s</i>", pkg, cmd);
            } else {
                t->label=g_strdup_printf("<b>%s</b>", pkg);
            }
            TRACE("label = %s\n", t->label);
            // t->tooltip = text;
            t->tooltip = (tooltip)?tooltip:NULL;
            t->callback=(void *)MenuPopoverSignals<Type>::noop;
            t->callbackData=node;
        }
        g_list_free(specificList);
        DBG("creating Popup<Type>...\n");
        auto popup = new(Popup<Type>)(PKG::specificItems);
        //Title would be selected item label:
        //popup->changeTitle( _("Software Updater"), PKG_ICON);
        PKG::pkgItemPopUp = popup->menu();
        gtk_widget_show_all(GTK_WIDGET(PKG::pkgItemPopUp));
        return;
    }


    static void createMainMenu(XML::XmlStructure *xmlStructure, 
            GList *actionsList, GList *actionOptionsList){
        TRACE("createMainMenu...\n");
        GList *activeList = getActiveList(xmlStructure, actionsList, "icon");

        PKG::mainItems =(menuItem2_t *)calloc(g_list_length(activeList)+1, sizeof(menuItem2_t));
        if (!PKG::mainItems){
            DBG("createMainMenu(): calloc: %s\n", strerror(errno));
            throw(1);
        }
        
        auto t = PKG::mainItems;
        for(auto l = activeList; l && l->data; l = l->next, t++){
            auto node = (XML::XmlNode *)l->data;
            t->icon = xmlStructure->getAttribute(node, "icon");
            auto cmd = xmlStructure->getAttribute(node, "cmd");
            auto pkg = xmlStructure->getAttribute(node, "pkg");
            // Not for actions:
            // auto text = xmlStructure->getText(node);
            auto tooltip = xmlStructure->getAttribute(node, "tooltip");
            if (cmd) {
                t->label=g_strdup_printf("  <i>%s %s</i>", pkg, cmd);
            } else {
                t->label=g_strdup_printf("<b>%s</b>", pkg);
            }
            t->tooltip = (tooltip)?tooltip:NULL;
//            t->tooltip = text;
            TRACE("label = %s\n", t->label);
            t->callback=(void *)MenuPopoverSignals<Type>::noop;
            t->callbackData=node;
        }
        
        g_list_free(activeList);
        DBG("creating Popup<Type>...\n");
        auto popup = new(Popup<Type>)(PKG::mainItems);
        popup->changeTitle( _("Software Updater"), PKG_ICON);
        PKG::pkgPopUp = popup->menu();
        gtk_widget_show_all(GTK_WIDGET(PKG::pkgPopUp));
        return;
     }

private:
        // First, lets count how many items have the 
        // "icon" attribute set. Only items with
        // "icon" attribute will appear in popup menu.

    static GList *
    getActiveList(XML::XmlStructure *xmlStructure, 
            GList *list, const gchar *required, const gchar *tag[2]=NULL){
        GList *activeList = NULL;
        for (auto l=list; l && l->data; l=l->next){
            auto node =(XML::XmlNode *)l->data;
            auto attribute = xmlStructure->getAttribute(node, required);
            if (attribute) {
                if (!tag) activeList = g_list_prepend(activeList, node);
                else { 
                    auto tag1 = xmlStructure->getAttribute(node, tag[0]);
                    auto tag2 = xmlStructure->getAttribute(node, tag[1]);
                    if (tag1 || tag2){
                        activeList = g_list_prepend(activeList, node);
                    }
                }
            }
        }
        activeList=g_list_reverse(activeList);
        return activeList;
    }

    static void
    printNode(XML::XmlNode *node){
        fprintf(stderr, "***%d) %s/%s/%s %p( ", 
                node->level, 
                node->parent->parent->name, 
                node->parent->name, node->name, node); 
        auto p = node->parent->attNames;
        auto q = node->parent->attValues;
        for (;p && *p; p++, q++){
            fprintf(stderr, "%s=%s ", *p, *q);
        }
        fprintf(stderr, ")\n( ");
       
        p = node->attNames;
        q = node->attValues;
        for (;p && *p; p++, q++){
            fprintf(stderr, "%s=%s ", *p, *q);
        }
        fprintf(stderr, ")\n");
    }

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
    if (!PKG::pkgPopUp) {
        PKG::pkgPopUp = GTK_MENU(gtk_menu_new()); 
        addMenuTitle(PKG::pkgPopUp, type->title());
        type->parseXML(); 
    }
    auto view = Fm<Type>::getCurrentView();
    g_object_set_data(G_OBJECT(PKG::pkgPopUp), "baseModel", (void *)view);
    g_object_set_data(G_OBJECT(PKG::pkgPopUp), "view", (void *)view);  
    DBG("pkgItemPopUp=%p\n", PKG::pkgPopUp);
    return PKG::pkgPopUp;
}

template <class Type>
GtkMenu *PkgPopUp<Type>::popUpItem(Type *type){
    if (!PKG::pkgItemPopUp) {
        PKG::pkgItemPopUp = GTK_MENU(gtk_menu_new());
        addMenuTitle(PKG::pkgItemPopUp, type->itemTitle());
        type->parseXML();
    }

    auto view = Fm<Type>::getCurrentView();
    g_object_set_data(G_OBJECT(PKG::pkgItemPopUp), "baseModel", (void *)view);
    g_object_set_data(G_OBJECT(PKG::pkgItemPopUp), "view", (void *)view);    
    DBG("pkgItemPopUp=%p\n", PKG::pkgItemPopUp);
    return PKG::pkgItemPopUp;
}

template <class Type>
void PkgPopUp<Type>::resetPopup(Type *type){
    DBG("resetPopup=%p\n", PKG::pkgItemPopUp);  
}

template <class Type>
void PkgPopUp<Type>::resetMenuItems(Type *type) {
    DBG("resetMenuItems...\n");
}





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
        PkgPopUp<Type>::parseXMLfile("pkg_pkg.xml", com_);
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


}
#endif
