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
        auto topNode = xmlStructure->topNode();
        const gchar *data[]={"pkg", "option", NULL}; 
        xmlStructure->sweep(topNode, process2, (void *)data);
        const gchar *data2[]={"action", "option", NULL}; 
        xmlStructure->sweep(topNode, process3, (void *)data2);
    }

    static void process2(XML::XmlNode *node, void *data){
        TRACE("process2: %s (%d)\n", node->name, node->level);
        auto p = node->attNames;
        auto q = node->attValues;
        
        auto parent = node->parent;
        if (!parent) return;
        if (!parent->name) return;
        auto tags = (gchar **)data;
        if (strcmp(parent->name, tags[0])) return;
        if (strcmp(node->name, tags[1])) return;
        fprintf(stderr, "***%d) %s/%s ( ", node->level, parent->name, node->name); 
        
        for (;p && *p; p++, q++){
            fprintf(stderr, "%s=%s ", *p, *q);
        }
        fprintf(stderr, ")\n");
        
    }

    static void process3(XML::XmlNode *node, void *data){
        TRACE("process3: %s (%d)\n", node->name, node->level);
        
        auto parent = node->parent;
        if (!parent) return;
        if (!parent->name) return;
        auto tags = (gchar **)data;
        if (strcmp(parent->name, tags[0])) return;
        if (strcmp(node->name, tags[1])) return;

        fprintf(stderr, "***%d) %s/%s/%s ( ", node->level, parent->parent->name, parent->name, node->name); 
        auto p = parent->attNames;
        auto q = parent->attValues;
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
