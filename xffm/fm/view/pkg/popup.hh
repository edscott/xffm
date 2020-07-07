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


namespace pkg {
    class XmlProperties {
    private:
        gint count;
        const gchar *tag[2];
        gboolean start[2];
        void incCount(void){ count++;}
        GSList *stack;
        
    public:
        gboolean atLevel(void){
            gboolean retval = current() && tag[0] && strcmp(current(),tag[0])==0;
            return (retval);
        }

        void push(const gchar *elementName){stack = g_slist_insert(stack, g_strdup(elementName), 0);}
        void pop(void){
            auto last = g_slist_nth_data(stack, 0);
            stack = g_slist_remove(stack, last);
            g_free(last);
        }
        const gchar *current(void){
            if (!stack) return NULL;
            return (const gchar *)stack->data;
        }
                 
        XmlProperties(void):stack(NULL){}
        
        gchar *currentTag;
        FILE *input;
        const gchar *getTag(gint id){return tag[id];}
        void setTag(gint id, const gchar *t){
            tag[id] = t;
            unsetStart(0);
            unsetStart(1);
        }
        void resetCount(void){ count=0;}
        gint index(void){return count;}

        void setStart(gint id){
            start[id] = TRUE;
            if (id == 1) incCount();
        }
        void unsetStart(gint id) {start[id] = FALSE;}
        gboolean getStart(gint id){return start[id];}
        
        pkg_option_t *xmlOptions; 
        pkg_command_t *xmlCmds;
        RodentMenuDefinition *xmlMenuDefinitions;
    };
    XmlProperties properties;
}


template <class Type>
class PkgPopUp {
    using Data=pkg::XmlProperties;

public:
    GtkMenu *popUp(Type *type);
    GtkMenu *popUpItem(Type *type);
    void resetPopup(Type *type);
    void resetMenuItems(Type *type);
    //void parseXML(Type *type);

    static void parseXMLfile(const gchar *xmlFile, const gchar *tag){
        Data& data = pkg::properties;
        if (!initXml(xmlFile, tag)){
            DBG("parseXMLfile() initProperties failed.\n");
            return;
        }
        populateGlobalOptions();
        populateMenu();
        fclose (data.input);

        
    }
private:
    static void 
    populateGlobalOptions(){
        // Count number of options.
        Data& data = pkg::properties;
        initSubTag("option");
        parse(startCount, endCount, NULL, NULL);
        DBG("\"%s\" count = %d \n", data.getTag(1), data.index());

        // Allocate structure (extra item for NULL terminator).
        data.xmlOptions = (pkg_option_t *) calloc((data.index()+1),sizeof(pkg_option_t));
        if (!data.xmlOptions){
            DBG("populateGlobalOptions() calloc: %s\n", strerror(errno));
            return;
        }
        initSubTag("option");
        parse(startOptions, endCount, textOptions, NULL);

    }

    static void
    populateMenu(void){
        Data& data = pkg::properties;
        initSubTag("action");
        parse(startCount, endCount, NULL, NULL);
        DBG("\"%s\" count = %d \n", data.getTag(1), data.index());
        // Allocate structures (extra item for NULL terminator).
        data.xmlCmds = (pkg_command_t *) calloc(data.index()+1,sizeof(pkg_command_t));
        data.xmlMenuDefinitions = 
            (RodentMenuDefinition *)calloc(data.index()+2,sizeof(RodentMenuDefinition));
        if (!data.xmlOptions || !data.xmlMenuDefinitions){
            DBG("populateGlobalOptions() calloc: %s\n", strerror(errno));
            return;
        }
        initSubTag("action");
        parse(startActions, endCount, textActions, NULL);
        // For each action, count options.
        auto count = data.index();
        data.setTag(0, "action");
        for (auto i=0; i<count; i++){
            initSubTag("option");
            parse(startCount, endCount, NULL, NULL);
            DBG("action %d: options=%d\n", i, data.index());
        }

/*
        // Allocate structures
        xml_cmds = 
            (pkg_command_t *)malloc((action_count+1)*sizeof(pkg_command_t));
        xmlMenuDefinitions = 
            (RodentMenuDefinition *)malloc((action_count+1)*sizeof(RodentMenuDefinition));

        if (!xml_cmds || !xmlMenuDefinitions){
            DBG("populateXML_menu() malloc failed: %s\n", strerror(errno));
            return;
        }
        memset(xml_cmds, 0, (action_count+1)*sizeof(pkg_command_t));
        memset(xmlMenuDefinitions, 0, (action_count+1)*sizeof(RodentMenuDefinition));


        xmlNodePtr node = xmlDocGetRootElement (doc);   
        for(node = node->children; node; node = node->next) {
            if (strcasecmp(command, (gchar *)(node->name))) continue;
            // Here we have our particular command section
            RodentMenuDefinition *p = xmlMenuDefinitions;
            pkg_command_t *q = xml_cmds;
            xmlNodePtr node1;
            for(node1 = node->children; node1; node1 = node1->next) {
                gint retval = parseXML_action(node1, p, q);
                if (retval){
                    q->cmd_options = parse_cmd_options(node1); 
                    NOOP("%s/%s --> %p\n", q->pkg, q->cmd, q->cmd_options);
                                   
                    if (retval & 0x01) q++;
                    if (retval & 0x02) p++; 
                }
            }
        }
        */
    }

    static void
    startActions (GMarkupParseContext * context,
                    const gchar * elementName,
                    const gchar ** attributeNames, 
                    const gchar ** attributeValues, 
                    gpointer functionData, 
                    GError ** error) 
    {
        TRACE ("startActions -> %s\n",elementName); 
        Data& data = pkg::properties;
        if (data.getTag(0) && strcmp(data.getTag(0), elementName)==0){
            TRACE("Gotcha: getTag(0)=%s\n", elementName);
            data.setStart(0);
            data.push(elementName);
            return;
        }  
        if (data.atLevel() && data.getStart(0) &&  strcasecmp(data.getTag(1), elementName)==0){
            data.setStart(1);
            const gchar **p;
            const gchar **q;
            for (p=attributeNames, q=attributeValues;
                    p && *p; p++, q++){
                DBG("(%s: %d) %s = %s \n", data.getTag(1), data.index(), *p, *q);
                // Strings.
                if (strcasecmp(*p, "pkg")==0)
                    data.xmlCmds[data.index()].pkg=g_strdup(*q);
                else if (strcasecmp(*p, "parameter")==0)
                    data.xmlCmds[data.index()].parameter=g_strdup(*q);
                else if (strcasecmp(*p, "cmd")==0)
                    data.xmlCmds[data.index()].cmd=g_strdup(*q);
                else if (strcasecmp(*p, "argument")==0)
                    data.xmlCmds[data.index()].argument=g_strdup(*q);
                else if (strcasecmp(*p, "string")==0)
                    data.xmlCmds[data.index()].string=g_strdup(*q);
                else if (strcasecmp(*p, "icon")==0)
                    data.xmlCmds[data.index()].icon=g_strdup(*q);
                // Flags.
                else if (strcasecmp(*p, "protected")==0) 
                    data.xmlCmds[data.index()].flags |= PKG_ACCESS_READ;
                else if (strcasecmp(*p, "local")==0)
                    data.xmlCmds[data.index()].flags |= PKG_LOCAL_SELECTION;
                else if (strcasecmp(*p, "remote")==0)
                    data.xmlCmds[data.index()].flags |= PKG_REMOTE_SELECTION;
                else if (strcasecmp(*p, "no_selection")==0)
                    data.xmlCmds[data.index()].flags |= PKG_NO_SELECTION;
                else if (strcasecmp(*p, "no_version")==0)
                    data.xmlCmds[data.index()].flags |= PKG_NO_VERSION;
                else if (strcasecmp(*p, "scroll_up")==0)
                    data.xmlCmds[data.index()].flags |= PKG_SCROLL_UP;
          }

          // Process inner "options"
          gchar buffer[2048];
          memset(buffer, 0, 2048);
          while (fgets(buffer, 255, data.input) && !feof(data.input)){
            if (strstr(buffer,"/action")) break;
            DBG("%s", buffer);
            // here we need another processor to handle the following...
            // process options
            // process text
          }
          // text field: data.[data.index()].hlp = g_strdup_printf("<b>%s %s</b>\n%s",q->pkg, (q->cmd)? q->cmd:"", value); 
            data.xmlCmds[data.index()].pkg_options = data.xmlOptions;
            // We are done here. No need to push.
            data.setStart(0);
            return;
        }
        data.push(elementName);
        return;
    }

   
    static void textActions (GMarkupParseContext *context,
                              const gchar         *text,
                              gsize                text_len,
                              gpointer             functionData,
                              GError             **error){
        Data& data = pkg::properties;
        if (data.getStart(1) && text_len > 0){
            gchar buffer[text_len+1];
            memset(buffer, 0, text_len+1);
            memcpy(buffer, text, text_len);
            g_strstrip(buffer);
            if (!strlen(buffer)) return;
            TRACE ("mainActionsText -> \"%s\"\n",buffer); 
            data.xmlCmds[data.index()].hlp=g_strdup_printf("<b>%s %s</b>\n%s", 
//                        "foo", "bar" , buffer);
              data.xmlCmds[data.index()].pkg, 
              (data.xmlCmds[data.index()].cmd)? data.xmlCmds[data.index()].cmd:"",
              buffer);
        }

    }
    
    static void
    startOptions (GMarkupParseContext * context,
                    const gchar * elementName,
                    const gchar ** attributeNames, 
                    const gchar ** attributeValues, 
                    gpointer functionData, 
                    GError ** error) 
    {
        TRACE ("mainStart -> %s\n",elementName); 
        Data& data = pkg::properties;
        if (data.getTag(0) && strcmp(data.getTag(0), elementName)==0){
            TRACE("Gotcha: getTag(0)=%s\n", elementName);
            data.setStart(0);
            data.push(elementName);
            return;
        }  
        if (data.atLevel() && data.getStart(0) &&  strcasecmp(data.getTag(1), elementName)==0){
            data.setStart(1);
            const gchar **p;
            const gchar **q;
            for (p=attributeNames, q=attributeValues;
                    p && *p; p++, q++){
                DBG("(%s: %d) %s = %s \n", data.getTag(1), data.index(), *p, *q);
                if (strcasecmp(*p, "loption")==0)
                    data.xmlOptions[data.index()].loption=g_strdup(*p);
                if (strcasecmp(*p, "parameter")==0)
                    data.xmlOptions[data.index()].parameter=g_strdup(*p);
                if (strcasecmp(*p, "active")==0)
                    data.xmlOptions[data.index()].loption=g_strdup(*p);
          }
        }
        data.push(elementName);
        return;
    }
    
    static void textOptions (GMarkupParseContext *context,
                              const gchar         *text,
                              gsize                text_len,
                              gpointer             functionData,
                              GError             **error){
        Data& data = pkg::properties;
        if (data.getStart(1) && text_len > 0){
            gchar buffer[text_len+1];
            memset(buffer, 0, text_len+1);
            memcpy(buffer, text, text_len);
            g_strstrip(buffer);
            if (!strlen(buffer)) return;
            DBG ("Text -> \"%s\"\n",buffer); 
            data.xmlOptions[data.index()].hlp=g_strdup_printf("<b>%s</b>\n%s", 
//                        "foo", buffer);
                        data.xmlOptions[data.index()].loption, buffer);
        }

    }

 /*   static void endOptions(GMarkupParseContext *context,
                              const gchar         *elementName,
                              gpointer             functionData,
                              GError             **error){
        Data& data = pkg::properties;
        data.pop();
        if (data.getTag(0) && strcmp(data.getTag(0), elementName)==0){
            DBG ("mainEnd -> %s\n",elementName); 
            data.unsetStart(0);
        }
        auto currentElement = data.current();
        if (currentElement && strcmp(currentElement,data.getTag(0)) == 0){
            data.unsetStart(1);     
        }
         
     }*/
    
    static void
    startCount (GMarkupParseContext * context,
                    const gchar * elementName,
                    const gchar ** attributeNames, 
                    const gchar ** attributeValues, 
                    gpointer functionData, 
                    GError ** error) 
    {
        TRACE ("mainStart -> %s\n",elementName); 
        Data& data = pkg::properties;
        if (data.getTag(0) && strcmp(data.getTag(0), elementName)==0){
            data.setStart(0);
            data.push(elementName);
            return;
        }

        if (data.atLevel() && data.getTag(1) && strcmp(data.getTag(1), elementName)==0){
            TRACE("Gotcha(%d): current=%s, elementName=%s\n", data.index(), data.current(), elementName);
            data.setStart(1);
        }  
        data.push(elementName);
        return;
    }


    static void endCount(GMarkupParseContext *context,
                              const gchar         *elementName,
                              gpointer             functionData,
                              GError             **error){
        Data& data = pkg::properties;
        data.pop();

        if (data.getTag(0) && strcmp(data.getTag(0), elementName)==0){
            TRACE ("endCount -> %s\n",elementName); 
            data.unsetStart(0);
            return;
        }
        if (!data.atLevel()) return;
        if (data.getTag(1) && strcmp(data.getTag(1), elementName)==0){
            data.unsetStart(1);     
        }         
     }
    
   /* static void
    mainStart (GMarkupParseContext * context,
                    const gchar * element_name,
                    const gchar ** attribute_names, 
                    const gchar ** attribute_values, 
                    gpointer functionData, 
                    GError ** error) 
    {
        DBG ("mainStart -> %s\n",element_name); 

         return;
    }
    static void mainEnd(GMarkupParseContext *context,
                              const gchar         *element_name,
                              gpointer             functionData,
                              GError             **error){
        DBG ("mainEnd -> %s\n",element_name); 
         ;
     }*/
      /* text is not nul-terminated */
    static void mainText (GMarkupParseContext *context,
                              const gchar         *text,
                              gsize                text_len,
                              gpointer             functionData,
                              GError             **error){
        DBG ("mainText -> %s\n",text); 

    }
      /* text is not nul-terminated. */
    static void mainPassthrough (GMarkupParseContext *context,
                              const gchar         *passthrough_text,
                              gsize                text_len,
                              gpointer             functionData,
                              GError             **error)
    {
        DBG ("mainPassthrough -> %s\n",passthrough_text); 
        
    }
    static void mainError (GMarkupParseContext *context,
                              GError              *error,
                              gpointer             functionData)
    {
        DBG ("mainError -> %s\n",error->message); 
    }
    


    void addMenuTitle(GtkMenu *menu, const gchar *text){
        GtkWidget *title = Gtk<Type>::menu_item_new(NULL, text); 
        gtk_widget_set_sensitive(title, TRUE);
        gtk_widget_show (title);
        g_object_set_data(G_OBJECT(menu), "title", title);
        gtk_container_add (GTK_CONTAINER (menu), title);

    }

    static void
    parse(
            void (*start)(GMarkupParseContext *, const gchar *, 
                  const gchar **, const gchar **, gpointer, GError **), 
            void (*end)(GMarkupParseContext *, const gchar *, 
                  gpointer, GError **), 
            void (*text)(GMarkupParseContext *, 
                  const gchar *, gsize, gpointer, GError **)=NULL, 
            void (*passthrough)(GMarkupParseContext *,
                  const gchar *, gsize, gpointer, GError **)=NULL)
    {
        Data& data = pkg::properties;
        rewind(data.input);
        GMarkupParser mainParser = { start, end, text, passthrough, NULL};
        auto mainContext = g_markup_parse_context_new (&mainParser, (GMarkupParseFlags)0, NULL, NULL);
        
        gchar line[2048];
        memset(line, 0, 2048);
        while(!feof (data.input) && fgets (line, 2048, data.input)) {
            //fprintf(stderr, "%s", line);
            GError *error = NULL;
            if (!g_markup_parse_context_parse (mainContext, line, strlen(line), &error) )
            {
                DBG("parse(): %s\n", error->message);
                g_error_free(error);
            }
        }

        g_markup_parse_context_free (mainContext);

    }

    static void initSubTag(const gchar *subTag){
        Data& data = pkg::properties;
        data.resetCount();
        data.setTag(1,subTag);
        data.unsetStart(0);
        data.unsetStart(0);
    }

    static gboolean initXml(const gchar *xmlFile, const gchar *tag){
        Data& data = pkg::properties;
        data.input = NULL;
        data.setTag(0,tag);
        auto resourcePath = g_build_filename(PREFIX, "share", "xml", "xffm+", xmlFile, NULL);
        if (!g_file_test(resourcePath, G_FILE_TEST_EXISTS)){
            DBG("parseXMLfile(): %s %s\n", resourcePath, strerror(ENOENT));
            g_free(resourcePath);
            resourcePath = g_build_filename(buildXml, xmlFile, NULL);
            if (!g_file_test(resourcePath, G_FILE_TEST_EXISTS)){
                DBG("parseXMLfile(): %s %s\n", resourcePath, strerror(ENOENT));
                g_free(resourcePath);
                return FALSE;
            }
        }
        DBG("parseXMLfile(): %s\n", resourcePath);
        data.input = fopen (resourcePath, "r");
        g_free(resourcePath);
        if (!data.input) return FALSE;
        return TRUE;
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
