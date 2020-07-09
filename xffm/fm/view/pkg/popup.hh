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
    pkg_option_t *pkg_options;
    pkg_option_t *cmd_options;
    gchar *hlp;     // tooltip help
    gchar *argument;
    gchar *string;
    gchar *icon;

} pkg_command_t;

namespace PKG{
    GList *parentList=NULL;
    GList *nodeList=NULL;
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
        populateStructures(xmlStructure, tag);

       /* auto topNode = xmlStructure->topNode();
        const gchar *data[]={tag, "option", NULL}; 
        xmlStructure->sweep(topNode, processOptions, (void *)data);
        const gchar *data2[]={"action", "option", NULL}; 
        xmlStructure->sweep(topNode, processActions, (void *)data2);*/
    }
private:

    static void 
    initLists(XML::XmlStructure *xmlStructure){
        if (PKG::parentList) {
            g_list_free(PKG::parentList);
            PKG::parentList = NULL;
        }
        if (PKG::nodeList) {
            g_list_free(PKG::nodeList);
            PKG::nodeList = NULL;
        }
        xmlStructure->initSweepCount();
    }

    static gint 
    countSweepItems(XML::XmlStructure *xmlStructure, void *data){
        initLists(xmlStructure);
        auto topNode = xmlStructure->topNode();
        xmlStructure->sweep(topNode, countItems, data);

        auto tags = (const gchar **)data;
        DBG("countSweepItems(): %s/%s count = %d/%d parents = %d nodelist=%d\n",
                tags[0], tags[1],
                xmlStructure->sweepCount(),
                XML::xml.items,
                g_list_length(PKG::parentList),
                g_list_length(PKG::nodeList));
        return xmlStructure->sweepCount();
    }

    static gint 
    countChildlessItems(XML::XmlStructure *xmlStructure, void *data){
        initLists(xmlStructure);
        auto topNode = xmlStructure->topNode();
        xmlStructure->sweep(topNode, countChildless, data);

        auto tags = (const gchar **)data;
        DBG("countChildlessItems(): %s/%s count = %d/%d parents = %d nodelist=%d\n",
                tags[0], tags[1],
                xmlStructure->sweepCount(),
                XML::xml.items,
                g_list_length(PKG::parentList),
                g_list_length(PKG::nodeList));
        return xmlStructure->sweepCount();
    }

    static void 
    populateStructures(XML::XmlStructure *xmlStructure, const gchar *tag){
        //auto topNode = xmlStructure->topNode();
        gint count;
        
        const gchar *data[]={tag, "option", NULL};
        //count = countSweepItems(xmlStructure, (void *)data);
        count = countChildlessItems(xmlStructure, (void *)data);

        auto pkgOptions = globalOptions(PKG::nodeList);

        const gchar *data2[]={tag, "action", NULL}; 
        auto countActions = countSweepItems(xmlStructure, (void *)data2);
        //count = countChildlessItems(xmlStructure, (void *)data2);

        const gchar *data3[]={"action", "option", NULL}; 
        count = countSweepItems(xmlStructure, (void *)data3);
        auto countActionParents = g_list_length(PKG::parentList);


    }

    static pkg_option_t * 
    globalOptions(GList *list){
        // Allocate structure
        auto count = g_list_length(list);
        auto xmlOptions = (pkg_option_t *) calloc((count+1), sizeof(pkg_option_t));
        if (!xmlOptions){
            DBG("globalOptions() calloc failed: %s\n", strerror(errno));
            throw 1;
        }
        auto s = xmlOptions;
        for (auto l=list; l && l->data; l=l->next, s++){
            DBG("list data=%p\n", l->data);
            auto node = (XML::XmlNode *)l->data;
            auto p = node->attNames;
            auto q = node->attValues;
            for (;p && *p; p++, q++){
                if (strcmp(*p, "loption")==0) s->loption = g_strdup(*q);
                else if (strcmp(*p, "parameter")==0) s->parameter = g_strdup(*q);
                else if (strcmp(*p, "active")==0) s->active = g_strdup(*q);
            }
            if (node->text){
                s->hlp = g_strdup_printf("<b>%s</b>\n", *q);
                DBG("text(%s): %s\n", s->loption, node->text);
           }
        }
        return xmlOptions;
    }



    // check for childless nodes
    static gboolean
    checkLevel1(XML::XmlNode *node, const gchar **tags){
        if (!checkLevel2(node, tags)) return FALSE;
        // Childless:
        if (node->child) return FALSE;

        TRACE("parent: %s (%p) childless %s %p\n",
                node->parent->name,  node->parent,
                node->name,  node);
        return TRUE;
    }

    static gboolean
    checkLevel2(XML::XmlNode *node, const gchar **tags){
        if (!node || !tags) return FALSE;
        if (!node->parent || !node->parent->name) return FALSE;
        if (strcmp(node->parent->name, tags[0])) return FALSE;
        if (strcmp(node->name, tags[1])) return FALSE;
        TRACE("parent: %s (%p)\n",node->parent->name,  node->parent);
        if (!PKG::parentList || !g_list_find (PKG::parentList, node->parent)){
            PKG::parentList = g_list_append(PKG::parentList, node->parent);
        }
        return TRUE;
    }

    static gboolean countItems(XML::XmlNode *node, void *data){
        auto tags = (const gchar **)data;
        if (!checkLevel2(node, tags)) return FALSE;
        //printNode(node);
        PKG::nodeList = g_list_append(PKG::nodeList, node);

        return TRUE;        
    }

    static gboolean countChildless(XML::XmlNode *node, void *data){
        auto tags = (const gchar **)data;
        if (!checkLevel1(node, tags)) return FALSE;
        //printNode(node);
        PKG::nodeList = g_list_append(PKG::nodeList, node);
        DBG("appended %p to nodelist\n", node);

        return TRUE;        
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

    static gboolean processOptions(XML::XmlNode *node, void *data){
        auto tags = (const gchar **)data;
        if (!checkLevel2(node, tags)) return FALSE;

        TRACE("processOptions: %s (%d)\n", node->name, node->level);
        auto p = node->attNames;
        auto q = node->attValues;
        fprintf(stderr, "***%d) %s/%s ( ", 
                node->level, node->parent->name, node->name); 
        
        for (;p && *p; p++, q++){
            fprintf(stderr, "%s=%s ", *p, *q);
        }
        fprintf(stderr, ")\n");
        return TRUE;
    }

    static gboolean processActions(XML::XmlNode *node, void *data){
        TRACE("process3: %s (%d)\n", node->name, node->level);
        auto tags = (const gchar **)data;
        if (!checkLevel2(node, tags)) return FALSE;

        fprintf(stderr, "***%d) %s/%s/%s ( ", 
                node->level, 
                node->parent->parent->name, 
                node->parent->name, node->name); 
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
        return TRUE;
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
