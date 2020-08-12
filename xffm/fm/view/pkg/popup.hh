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

#define PLAIN_OPTION    0x01
#define ARGUMENT_OPTION  0x02
#define ARGUMENT_EXCLUSIVE_OPTION  0x04

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


namespace PKG{
    GList *parentList=NULL;
    GList *nodeList=NULL;
    GtkMenu *pkgPopUp=NULL;
    GtkMenu *pkgItemPopUp=NULL;
    menuItem2_t *mainItems=NULL;
    menuItem2_t *specificItems=NULL;
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
        // populateStructures(xmlStructure, tag);
        GList *optionsList = xmlStructure->getList(tag, "option");
        GList *actionsList = xmlStructure->getList(tag, "action");
        GList *actionOptionsList = xmlStructure->getList("action", "option");
        // create popup menus.
        createMenu(xmlStructure, actionsList, &(PKG::pkgPopUp));
        createMenu(xmlStructure, actionsList, &(PKG::pkgItemPopUp), TRUE);
        /*for (auto l=actionsList; l && l->data; l=l->next){
            auto node = (XML::XmlNode *)l->data;
            auto cmd = xmlStructure->getAttribute(node, "cmd");
            DBG("-> %s: %s\n", cmd, node->text);
        }*/
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

    
    static void createMenu(XML::XmlStructure *xmlStructure, 
            GList *list, GtkMenu **menu_p, gboolean specific=FALSE){

        TRACE("createSpecificMenu...\n");
        const gchar *tag[2]={"local", "remote"};
        // Only nodes with "icon" attribute defined will be shown.
        GList *activeList = getActiveList(xmlStructure, list, 
                "icon", specific?tag:NULL);

        auto items = (menuItem2_t *)calloc(g_list_length(activeList)+1, sizeof(menuItem2_t));
        if (!items){
            DBG("createSpecificMenu(items): calloc: %s\n", strerror(errno));
            throw(1);
        }
        
        auto t = items;
        const gchar *titleIcon = NULL;
        const gchar *titleText = NULL;
        const gchar *titleTooltip = NULL;

        for(auto l = activeList; l && l->data; l = l->next, t++){
            auto node = (XML::XmlNode *)l->data;
            auto cmd = xmlStructure->getAttribute(node, "cmd");
            auto pkg = xmlStructure->getAttribute(node, "pkg");
            auto icon = xmlStructure->getAttribute(node, "icon"); 
            auto tooltip = xmlStructure->getAttribute(node, "tooltip");
            auto protect = xmlStructure->getAttribute(node, "protected");
            if (!cmd){
                titleIcon = icon;
                titleText = pkg;
                titleTooltip = tooltip;
                t--;
                continue;
            }
            t->icon = icon;
            t->label= cmd;
            t->tooltip = tooltip;
            t->callback=(void *)processCmd;
            t->callbackData=node;
            t->protect=protect?TRUE:FALSE;
        }
        g_list_free(activeList);
        TRACE("creating Popup<Type>...\n");
        auto popup = new(Popup<Type>)(items);
        g_free(items);
        //Title would be selected item label:
        popup->changeTitle( titleText, titleIcon);
        *menu_p = popup->menu();
        gtk_widget_show_all(GTK_WIDGET(*menu_p));
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
   
    static GtkDialog *createDialog(XML::XmlNode *node)
    {
        auto xmlStructure = (XML::XmlStructure *)XML::xml.structure();
        auto dialog = GTK_DIALOG(gtk_dialog_new());
        gtk_window_set_modal (GTK_WINDOW(dialog), TRUE);
        gtk_window_set_resizable(GTK_WINDOW(dialog), TRUE);
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(mainWindow));
        //gtk_window_set_keep_above (GTK_WINDOW(dialog), TRUE);
        gtk_window_set_title(GTK_WINDOW(dialog), _("Package Manager"));
        gtk_window_set_type_hint(GTK_WINDOW(dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
        
        /*auto button = Gtk<Type>::dialog_button("help-contents", _("Help"));
        gtk_dialog_add_action_widget (dialog, GTK_WIDGET(button), 1);*/
        auto button = Gtk<Type>::dialog_button("redball", _("Cancel"));
        gtk_dialog_add_action_widget (dialog, GTK_WIDGET(button), 2);
        button = Gtk<Type>::dialog_button("greenball", _("Accept"));
        gtk_dialog_add_action_widget (dialog, GTK_WIDGET(button), 3);

        auto content_box = GTK_BOX(gtk_dialog_get_content_area(dialog));
        g_object_set_data(G_OBJECT(content_box),"dialog", dialog);
        auto top = GTK_LABEL(gtk_label_new(""));
        g_object_set_data(G_OBJECT(dialog), "top", top);

        auto cmd = xmlStructure->getAttribute(node, "cmd");
        auto pkg = xmlStructure->getAttribute(node, "pkg");
        gchar *t = 
            g_strdup_printf("<big><b>%s <i>%s</i></b></big>",
                    pkg, (cmd)?cmd:"");
        gtk_label_set_markup(top, t);
        g_free(t);
        gtk_widget_show(GTK_WIDGET(top));
        gtk_box_pack_start(content_box, GTK_WIDGET(top), FALSE, FALSE, 1);

        // XXX here goes the use_custom_envar() conditional...
 /*
        if (use_custom_envar){
            GtkBox *env_box = GTK_BOX(rfm_hbox_new(FALSE, 1));
            gtk_box_pack_start(content_box, GTK_WIDGET(env_box), FALSE, FALSE, 1);
    // FIXME: find available translation for "Environment options:"
            GtkWidget *env=gtk_label_new(_("Environment options:"));
            gtk_box_pack_start(env_box, GTK_WIDGET(env), FALSE, FALSE, 1);
            GtkWidget *env_entry = gtk_entry_new();
            if (envvar && strlen(envvar)) gtk_entry_set_text(GTK_ENTRY(env_entry), envvar);
            else {
                gtk_entry_set_placeholder_text (GTK_ENTRY(env_entry), 
                        (use_custom_envar_string)?(use_custom_envar_string):"ENVAR=\"\"");
            }
            gtk_box_pack_start(env_box, GTK_WIDGET(env_entry), FALSE, FALSE, 1);
            g_object_set_data(G_OBJECT(dialog), "env_entry", env_entry);
            gtk_widget_show_all(GTK_WIDGET(env_box));
        }
  */           
        // Add pkg options...
        // node is pointing to first pkg item, which should have cmd==NULL


         
        g_object_set_data(G_OBJECT(dialog), "actionNode", node);
        if (node->child) {
            auto markup =g_strdup_printf("<b>%s %s</b> %s", pkg, cmd, _("options:"));
            contentAddOptions(node->child, content_box, markup);
            g_free(markup);  
        }
        auto pkgNode = getPkgNode();
    

        if (pkgNode){
            auto markup = g_strdup_printf("<b>%s</b> %s", pkg, _("options:"));
            contentAddOptions(pkgNode->child, content_box, markup);
            g_free(markup);
        }

        gtk_widget_show(GTK_WIDGET(content_box));

        gtk_widget_grab_focus(GTK_WIDGET(button)); 
        gtk_widget_show(GTK_WIDGET(dialog));
        return dialog;
    }
    
    static void contentAddOptions(XML::XmlNode *node,
            GtkBox *content_box, const gchar *markup)
    {
        auto xmlStructure = (XML::XmlStructure *)XML::xml.structure();
        
        auto dialog = GTK_DIALOG(g_object_get_data(G_OBJECT(content_box), "dialog"));
        auto top = g_object_get_data(G_OBJECT(dialog), "top");
        auto pkgButton = gtk_toggle_button_new_with_label("");
        g_object_set_data(G_OBJECT(pkgButton), "dialog", dialog);
        
        auto label_widget = gtk_bin_get_child(GTK_BIN(pkgButton));
        gtk_label_set_markup(GTK_LABEL(label_widget), markup);
        auto pkgBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1));
        gtk_box_pack_start(GTK_BOX(pkgBox), pkgButton, FALSE, FALSE, 1);
        gtk_widget_show(GTK_WIDGET(pkgBox));
        gtk_widget_show(pkgButton);
        gtk_box_pack_start(GTK_BOX(content_box), GTK_WIDGET(pkgBox), FALSE, FALSE, 1);

        // Pkg options box.
        auto scrolled_window = gtk_scrolled_window_new(NULL, NULL);
        gtk_box_pack_start(GTK_BOX(content_box), scrolled_window, FALSE, FALSE, 1);

        auto pkgOptionsBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1));
        gtk_container_add(GTK_CONTAINER(scrolled_window), GTK_WIDGET(pkgOptionsBox));
        gtk_widget_show(GTK_WIDGET(pkgOptionsBox));//
        //gtk_widget_realize(GTK_WIDGET(pkgOptionsBox));

        auto vbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 1));
        gtk_box_pack_start(GTK_BOX(pkgOptionsBox), GTK_WIDGET(vbox), FALSE, FALSE, 1);
        g_signal_connect(G_OBJECT(pkgButton), "clicked", G_CALLBACK(hideShow), scrolled_window); 

        

        gint width = -1;
        GtkWidget *check;
        for (auto optionNode = node; optionNode; optionNode = optionNode->next){
            auto optionBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1));
            gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(optionBox), FALSE, FALSE, 1);
            auto loption = xmlStructure->getAttribute(optionNode, "loption");
            gchar *label = g_strdup_printf ("--%s", loption);
            check = gtk_check_button_new_with_label(label);
            g_object_set_data(G_OBJECT(dialog), label, check);
            g_object_set_data(G_OBJECT(check), "top", top);
            g_object_set_data(G_OBJECT(check), "dialog", dialog);
            auto hlp = xmlStructure->getText(optionNode);
            gtk_widget_set_tooltip_text(GTK_WIDGET(check), hlp);
            g_object_set_data(G_OBJECT(check), "dialog", dialog);
            gtk_box_pack_start(GTK_BOX(optionBox), check, FALSE, FALSE, 1);
            
            GtkRequisition minimum;
            auto parameter = xmlStructure->getAttribute(optionNode, "parameter");
            auto active = xmlStructure->getAttribute(optionNode, "active");
            TRACE("adding %s -> %p active=%s\n", label, check, active);
            if (parameter) {
                GtkWidget *entry = createOptionEntry(optionBox, dialog, check, parameter, label);
                gtk_widget_get_preferred_size(GTK_WIDGET(optionBox), &minimum, NULL);
                if (minimum.width > width) width = minimum.width;
                g_signal_connect(G_OBJECT (entry), "key-release-event", G_CALLBACK (update_option_entry), optionNode);//XXX
                g_signal_connect(G_OBJECT (entry), "button-press-event", G_CALLBACK (update_option_entry), optionNode);//XXX
                if (active) {
                    gtk_entry_set_text(GTK_ENTRY(entry), active);
                }

            } else {
                g_object_set_data(G_OBJECT(check), "type", GINT_TO_POINTER(PLAIN_OPTION));
            }
            if (active) {
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), TRUE);
            }
            g_signal_connect(G_OBJECT (check), "clicked", G_CALLBACK (updateOption), node);//XXX
            g_signal_connect (G_OBJECT (check), "clicked", G_CALLBACK (sensitivizeChecks), NULL);//XXX
            g_free(label);
        
            gtk_widget_show(GTK_WIDGET(check));
            gtk_widget_show(GTK_WIDGET(optionBox));

        
        }
        // update option text after last button is created, not before.
        //updateOption(GTK_BUTTON(check), g_object_get_data(G_OBJECT(dialog), "top"));
        //auto markup = g_strdup_printf("<b>%s</b>
        //updateOption(GTK_LABEL(g_object_get_data(G_OBJECT(dialog), "top")), g_object_get_data(G_OBJECT(dialog), "top"));

        if (width > 0) width +=30; // hack
        gtk_widget_set_size_request(scrolled_window, width, 100);
        //gtk_widget_realize(GTK_WIDGET(vbox));
        gtk_widget_show(GTK_WIDGET(vbox));
    }

    static void 
    hideShow(GtkButton *button, gpointer *data) {

        if (gtk_widget_get_visible(GTK_WIDGET(data))){
            gtk_widget_hide(GTK_WIDGET(data));
            gtk_widget_unrealize(GTK_WIDGET(data));

            auto dialog = GTK_DIALOG(g_object_get_data(G_OBJECT(button), "dialog"));
            GtkRequisition requisition;
            GtkRequisition natural;
            gtk_widget_get_preferred_size(GTK_WIDGET(dialog), &requisition, &natural);
            TRACE("w= %d h= %d\n", requisition.width, requisition.height);

            gtk_window_resize(GTK_WINDOW(dialog), requisition.width, requisition.height);
            gtk_window_set_resizable(GTK_WINDOW(dialog), TRUE);
            // XXX use this resize algorithm for local/properties dialog...
        } else {
            gtk_widget_show_all(GTK_WIDGET(data));
        }
    }
    

    static GtkWidget *
    createOptionEntry(GtkBox *obox, GtkDialog *dialog, GtkWidget *check, 
            const gchar *parameter, const gchar *label){
        GtkWidget *entry = gtk_entry_new();
                
        g_object_set_data(G_OBJECT(entry), "check", check);
        gtk_entry_set_placeholder_text (GTK_ENTRY(entry),  parameter);
        g_object_set_data(G_OBJECT(entry), "dialog", dialog);
        
        g_object_set_data(G_OBJECT(check), "entry", entry);
        gchar *elabel = g_strconcat(label, "-entry", NULL);
        g_object_set_data(G_OBJECT(dialog), elabel, entry);
        g_free(elabel);
        GtkWidget *lab = gtk_label_new("=");
        gtk_box_pack_start(obox, GTK_WIDGET(lab), FALSE, FALSE, 1);
        gtk_box_pack_end(obox, GTK_WIDGET(entry), TRUE, TRUE, 1);
        gtk_widget_show_all(GTK_WIDGET(obox));
        gtk_widget_set_sensitive(entry, FALSE);
        g_object_set_data(G_OBJECT(check), "type", GINT_TO_POINTER(ARGUMENT_OPTION)); 
        return entry;
    }
   
    static gchar *commandLine(GtkDialog *dialog, gboolean withMarkup){
        auto xmlStructure = (XML::XmlStructure *)XML::xml.structure();
        //auto optionNode = (XML::XmlNode *)data;
        auto pkgNode =  getPkgNode();
        auto actionNode =  (XML::XmlNode *)g_object_get_data(G_OBJECT(dialog), "actionNode");
        auto pkgText = contentGetOptions(pkgNode->child, dialog);
        auto actionText = contentGetOptions(actionNode->child, dialog);

        auto pkg = xmlStructure->getAttribute(actionNode, "pkg");
        auto cmd = xmlStructure->getAttribute(actionNode, "cmd");
        gchar *markup;
        if (withMarkup){
            markup = g_strdup_printf("<big><b>%s</b></big> <span color=\"red\">%s</span>\n<big><b><i>%s</i></b></big> <span color=\"red\">%s</span>",
                pkg, pkgText, cmd, actionText);
        } else {
            markup = g_strdup_printf("%s %s %s %s", pkg, pkgText, cmd, actionText);
        }
        g_free(pkgText);
        g_free(actionText);
        return markup;
    }

    static void
    updateOption(GtkButton *button, gpointer data){
        auto dialog = GTK_DIALOG(g_object_get_data(G_OBJECT(button), "dialog"));
        GtkLabel *top = GTK_LABEL(g_object_get_data(G_OBJECT(button), "top"));
        auto markup = commandLine(dialog, TRUE);
        auto xmlStructure = (XML::XmlStructure *)XML::xml.structure();
        gtk_label_set_markup(top, markup);
        g_free(markup);
    }

    static gboolean
    update_option_entry (GtkWidget *widget,
                   GdkEvent  *event,
                   gpointer   data){
        TRACE("update_option_entry...\n");
        auto button = GTK_BUTTON(g_object_get_data(G_OBJECT(widget), "check"));

        updateOption(button, data);
        return FALSE;
    }

        


    static gchar *
    pkgConfirm(void *data){
        auto *node = (XML::XmlNode *)data;
        auto dialog = createDialog(node);

        gint response;
        response = gtk_dialog_run(dialog);
        gtk_widget_hide(GTK_WIDGET(dialog));
        DBG("Response = %d\n", response);
        if (response == 3){
            auto command = commandLine(dialog, FALSE);
            DBG("commandLine = %s\n", command);
        }


        gtk_widget_destroy(GTK_WIDGET(dialog));
        return NULL;
    }

    static void sensitivizeChecks(GtkButton *check, gpointer data){
        if (!GTK_IS_TOGGLE_BUTTON(check)) return;
        auto entry = GTK_WIDGET(g_object_get_data(G_OBJECT(check), "entry"));
        gboolean state = FALSE;
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check))){
            state = TRUE;
        } 
        if (entry) gtk_widget_set_sensitive(entry, state);
    }

    static XML::XmlNode *
    getPkgNode(void){
        auto xmlStructure = (XML::XmlStructure *)XML::xml.structure();
        XML::XmlNode *pkgNode = NULL;
        // FIXME: "pkg" is specific, use input tag instead...
        GList *actionsList = xmlStructure->getList("pkg", "action");
        {
            const gchar *tag[2]={"local", "remote"};
            GList *activeList = getActiveList(xmlStructure, actionsList, 
                "icon", tag);
            {
                for (auto l=activeList; l && l->data; l=l->next){
                    auto n = (XML::XmlNode *)l->data;
                    auto c = xmlStructure->getAttribute(n, "cmd");
                    if (c == NULL){
                        pkgNode = n;
                        break;
                    }
                }
            }
            g_list_free(activeList);


            
        }
        g_list_free(actionsList);
        return pkgNode;
    }

    static gchar *
    contentGetPkgOptions(XML::XmlNode *optionNode, GtkDialog *dialog){
        auto pkgNode = getPkgNode();
        return contentGetOptions(pkgNode, dialog);
    }

    static gchar *
    contentGetOptions(XML::XmlNode *optionNode, GtkDialog *dialog) {
            //GtkDialog *dialog, gchar *in_string, pkg_option_t *options){
        if (!optionNode) return NULL;
        auto xmlStructure = (XML::XmlStructure *)XML::xml.structure();
     

        gchar *response_string = g_strdup("");
        
        for (auto n=optionNode; n; n=n->next){
            auto loption = xmlStructure->getAttribute(n, "loption");

            gchar *label = g_strdup_printf ("--%s", loption);
            auto check = GTK_TOGGLE_BUTTON(g_object_get_data(G_OBJECT(dialog), label));
       
            TRACE("check %s: %p\n", label, check);
            if (!check || !GTK_IS_TOGGLE_BUTTON(check)) continue;

            if (gtk_toggle_button_get_active(check)){
                gchar *g = g_strconcat(response_string, " ", label, NULL);
                g_free(response_string);
                response_string = g;
                gint type = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(check), "type"));
                if (type & ARGUMENT_OPTION){
                    gchar *elabel = g_strconcat(label, "-entry", NULL);
                    auto entry =GTK_ENTRY(g_object_get_data(G_OBJECT(dialog), elabel));
                    if (entry) {
                        const gchar *etext = gtk_entry_get_text(entry);
                        if (etext && *etext && gtk_toggle_button_get_active(check)){
                            gchar *g = g_strconcat(response_string, "=", etext, NULL);
                            g_free(response_string);
                            response_string = g;
                        }
                    } 
                    g_free(elabel);
                }
            }

            g_free(label);
        }
        return response_string;

















#if 0
        if (use_custom_envar) {
            GtkEntry *entry = g_object_get_data(G_OBJECT(dialog), "env_entry");
            const gchar *g = gtk_entry_get_text(entry);
            g_free(envvar);
            if (g && strlen(g)) envvar = g_strdup(g);
            else envvar = NULL;
        }

        if (response == 1){ // Help button
            // construct specific help command 
            widgets_t *widgets_p = rfm_get_widget("widgets_p");
            // default, freebsd pkg
            gchar *arg[]={pkg_command, "help", c->cmd, NULL};
            if (emerge){ // This will probably change if emerge implements action specific help
                arg[0] = "man";
                arg[1] = "emerge";
                arg[2] = NULL;
            } else if (apt){
                arg[0] = "man";
                arg[1] = "apt-get";
                arg[2] = NULL;
            } 
            // FIXME: zypper and yum case, and apt
            //        need to consider translations, or execute in locale=C

            rfm_clear_text (widgets_p); 
            rfm_show_text (widgets_p); 
            if (emerge) {
                // dump stderr on man output...
                rfm_thread_run_argv_full (widgets_p, arg, FALSE, NULL, rfm_operate_stdout, dump_stderr, scroll_to_top);
            } else {
                rfm_thread_run_argv_full (widgets_p, arg, FALSE, NULL, rfm_operate_stdout, NULL, scroll_to_top);
            }
        }

        gtk_widget_hide(GTK_WIDGET(dialog));


        // get response string
        gchar *response_string = NULL;
        if (response == 3){
            widgets_t *widgets_p = rfm_get_widget("widgets_p");
            response_string = content_get_options(dialog, g_strdup(""), xml_options);
            gchar *pcr=g_object_get_data(G_OBJECT(widgets_p->paper), "pkg_confirm_response");
            g_free(pcr);
            pcr = NULL;
            g_object_set_data(G_OBJECT(widgets_p->paper), "pkg_confirm_response", NULL);
            // In zypper/yum, assume yes is a global option
            if (zypper) {
                pcr = g_strdup_printf("%s --non-interactive", response_string);
            }
            else if (yum){
                pcr = g_strdup_printf("%s --assumeyes", response_string);
            } else if (apt) {
                pcr = g_strdup_printf("%s --assume-yes", response_string);
            }

            const gchar *action = NULL;
            if (c->cmd == NULL) action = content_get_action(GTK_WIDGET(dialog));
            else action = c->cmd;
            if (action) {
                gchar *g = g_strconcat(response_string, " ", action, NULL);
                g_free(response_string);
                response_string = g; 
                if (pcr) {
                    g = g_strconcat(pcr, " ", action, NULL);
                    g_free(pcr);
                    pcr = g;   
                }   
            }

            // In pkg, assume yes is an action option
            if (pkg) {
                pcr = g_strdup_printf("%s -y", response_string);
            }

            // If action has any other options, then get them too.
            pkg_option_t *cmd_options = g_object_get_data(G_OBJECT(dialog), "cmd_options");
            
            if (cmd_options || c->cmd_options) {
                response_string = content_get_options(dialog, response_string, 
                        (c->cmd_options)?c->cmd_options:cmd_options);
            }
        
            

            // FIXME: get and append action options
            
            if (argument) {
                const gchar *a = gtk_entry_get_text(GTK_ENTRY(argument));
                gchar *g = g_strconcat(response_string, " ", a, NULL);
                g_free(response_string);
                response_string = g;
                if (pcr) {
                    g = g_strconcat(pcr, " ", a, NULL);
                    g_free(pcr);
                    pcr = g;   
                }   
            }
            g_object_set_data(G_OBJECT(widgets_p->paper), "pkg_confirm_response", pcr);
        }

        gtk_widget_destroy(GTK_WIDGET(dialog));
        return (void *) response_string;
#endif

    }

    static 
    void
    processCmd(GtkMenuItem *m, gpointer data){
        gchar *response = pkgConfirm(data);
        
#if 0
        pkg_command_t *c = data;
        if (!c) return;
        widgets_t *widgets_p = rfm_get_widget ("widgets_p");
        gint flags = 0;
        DBG("pkg:\"%s\" cmd:\"%s\"\n", c->pkg, c->cmd);
        gchar *cmd=NULL;
        gchar *response = rfm_context_function(pkg_confirm_f, c);
        if (response){
                g_object_set_data(G_OBJECT(widgets_p->paper), "flags", NULL);
            DBG("response=%s\n", response);
            if (c->cmd && 
            (strcmp(c->cmd, "search")==0 ||
             strcmp(c->cmd, "--search")==0 ||
                 strcmp(c->cmd, "-Ss")==0)){
                view_t *view_p = widgets_p->view_p;
                record_entry_t *en = rfm_copy_entry(view_p->en);
                g_free (en->path);
                g_strstrip(response);
                en->path = g_strdup_printf("%s", response);
                DBG("command will be \"emerge %s\" \n", en->path);
                rodent_refresh (widgets_p, en);
                // This would open a new window, which annoys me:
                // gchar * cmd = g_strdup_printf("rodent-plug pkg %s", response);
                // g_free(cmd);
                flags = c->flags;
                g_free(response);
                return;
            }
            
            const gchar *sudo;
            if (geteuid() == 0 || (c->flags & ACCESS_READ)) sudo="";
            else sudo = "sudo -A ";
            cmd = g_strdup_printf("%s%s %s %s", sudo, (envvar)?envvar:"", c->pkg, response);

            if (sudo && strlen(sudo)) {
                // sudoize pcr if necessary
                gchar *pcr=g_object_get_data(G_OBJECT(widgets_p->paper), "pkg_confirm_response");
                gchar *g = g_strdup_printf("%s%s %s", sudo, c->pkg, pcr);
                g_free(pcr);
                pcr=g;
                g_object_set_data(G_OBJECT(widgets_p->paper), "pkg_confirm_response", pcr);
            }

            g_object_set_data(G_OBJECT(widgets_p->paper), "flags", GINT_TO_POINTER(c->flags));


            flags = c->flags;
            rfm_diagnostics(widgets_p, "xffm_tag/blue", cmd, "\n", NULL);
            DBG("do it --> %s\n", cmd);
            g_free(response);
        }
            
        do_it(widgets_p, cmd, flags);
        g_free(cmd);
#endif
        return;
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
    TRACE("pkgItemPopUp=%p\n", PKG::pkgPopUp);
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
    TRACE("pkgItemPopUp=%p\n", PKG::pkgItemPopUp);
    return PKG::pkgItemPopUp;
}

template <class Type>
void PkgPopUp<Type>::resetPopup(Type *type){
    TRACE("resetPopup=%p\n", PKG::pkgItemPopUp);  
}

template <class Type>
void PkgPopUp<Type>::resetMenuItems(Type *type) {
    TRACE("resetMenuItems...\n");
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
