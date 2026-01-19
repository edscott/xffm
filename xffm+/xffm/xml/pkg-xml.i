#ifdef COPYRIGHT_INFORMATION
#include "gplv3.h"
#endif

static gboolean use_custom_envar = FALSE;
static gchar *use_custom_envar_string = NULL;
gchar *strings[]={
    N_("Add"),
    N_("audit"),
    N_("autoremove"),
    N_("Backup"),
    N_("Check"),
    N_("Clean"),
    N_("Convert"),
    N_("Uninstall"),
    N_("Download"),
    N_("Information"),
    N_("Install"),
    N_("Lock"),
    N_("Plugins"),
    N_("Search"),
    N_("Shared libraries"),
    N_("Statistics"),
    N_("Unlock"),
    N_("Update Database"),
    N_("Updating"),
    N_("Upgrade"),
    N_("Version"),
    N_("which"),
    N_("Help"),
};
#if 0
#include <tagfile.h>



/////////////////////////////////////////////////


gboolean validate_XML_schema(const gchar *xml_file){
    Tag_t *Tag_p;
    GError *error=NULL;
    gchar *dirname = g_path_get_dirname(xml_file);
    gchar *schema_file = g_build_filename(dirname, "pkg.xsd", NULL);
    g_free(dirname);
    DBG("Loading schema: %s\n",  schema_file);
    Tag_p =tag_new_from_file(xml_file, &error);
    tag_load_schema(Tag_p, schema_file, &error);
    if (!tag_validate(Tag_p)){
        gchar *text=g_strdup_printf("%s: %s\n\n%s",
                _("Validate document"), xml_file,
                _("The Document is not valid!"));
        rfm_confirm(NULL, GTK_MESSAGE_ERROR, text, NULL,NULL);
        g_free(text);
        tag_free(Tag_p);
        return FALSE;
    } 
    g_free(schema_file);
    tag_free(Tag_p);
    return TRUE;
}

////////////////////////////////////////////////

#else 
gboolean validate_XML_schema(const gchar *xml_file){
    return TRUE;
}
#endif


static pkg_command_t *xml_cmds = NULL;
static pkg_option_t *xml_options = NULL;
//??static pkg_option_t *xml_cmd_options = NULL;
static RodentMenuDefinition *xml_menu_definitions = NULL;

static gint
count_tags(xmlNodePtr node, const gchar *tag){
    gint count = 0;
    xmlNodePtr node1;
        for(node1 = node->children; node1; node1 = node1->next) {
            if (strcasecmp(tag, (gchar *)(node1->name)) == 0) count++;
        }
    return count;
}

static gint
count_xml_tags(xmlDocPtr doc, const gchar *parent_tag, const gchar *tag){
    gint count = 0;
    xmlNodePtr node = xmlDocGetRootElement (doc); 
    for(node = node->children; node; node = node->next) {
        if (strcasecmp(parent_tag, (gchar *)(node->name))) continue;
        // Here we have our particular parent_tag section
        count = count_tags(node, tag);
    }
    return count;
}


static gchar *get_content_string(xmlNodePtr node1){
    xmlNodePtr child_node=node1->children;
    for (; child_node; child_node=child_node->next){
        if (strcmp((gchar *)(child_node->name), "text") == 0){
	    return g_strdup((const gchar *)(child_node->content));
        }
    }
    return NULL;
}

static gint
parseXML_action(xmlNodePtr node1, RodentMenuDefinition *p, pkg_command_t *q){
    if (strcasecmp("action", (gchar *)(node1->name)) != 0) return 0;
    // **********   application cmd structure   ****************
    // 1. protected (action requires sudo)
    // 2. application (local selected, remote selected, nothing selected)
    // 3. associated argument, if any
    // 4. associated help text for tooltip, if any
    // 5. command options, if they apply to action
    //
 
    
    gchar *value = (gchar *)xmlGetProp (node1, (const xmlChar *)"pkg");
    if (value) q->pkg = g_strdup(value);
    g_free(value);
    value = (gchar *)xmlGetProp (node1, (const xmlChar *)"parameter");
    if (value) q->parameter = g_strdup(value); 
    g_free(value);
    value = (gchar *)xmlGetProp (node1, (const xmlChar *)"cmd");
    if (value) q->cmd = g_strdup(value);
    g_free(value);
    // flags
    value = (gchar *)xmlGetProp (node1, (const xmlChar *)"protected");
    if (value) q->flags |= ACCESS_WRITE; else q->flags |= ACCESS_READ;
    g_free(value);
    value = (gchar *)xmlGetProp (node1, (const xmlChar *)"local");
    if (value) q->flags |= LOCAL_SELECTION;  
    g_free(value);
    value = (gchar *)xmlGetProp (node1, (const xmlChar *)"remote");
    if (value) q->flags |= REMOTE_SELECTION;  
    g_free(value);
    value = (gchar *)xmlGetProp (node1, (const xmlChar *)"no_selection");
    if (value) q->flags |= NO_SELECTION;  
    g_free(value);
    value = (gchar *)xmlGetProp (node1, (const xmlChar *)"no_version");
    if (value) {
        DBG("%s is set to no_version\n", q->cmd);
        q->flags |= NO_VERSION;  
    }
    g_free(value);
    value = (gchar *)xmlGetProp (node1, (const xmlChar *)"scroll_up");
    if (value) q->flags |= SCROLL_UP;  
    g_free(value);
    //
    value = (gchar *)xmlGetProp (node1, (const xmlChar *)"argument");
    if (value) q->argument = g_strdup(value);  
    g_free(value);
    value = get_content_string(node1);
    if (value) {
        q->hlp = g_strdup_printf("<b>%s %s</b>\n%s",q->pkg, (q->cmd)? q->cmd:"", value);  
    }
    g_free(value);
    value = (gchar *)xmlGetProp (node1, (const xmlChar *)"command_options");
    if (value) q->pkg_options = xml_options;  g_free(value);
    value = (gchar *)xmlGetProp (node1, (const xmlChar *)"string");
    if (value) q->string = g_strdup(_(value));
    else if (!q->cmd) {
        q->string = g_strdup_printf("%s%s%s",q->pkg, (q->cmd)?" ":"",(q->cmd)?q->cmd:"");
    }
    //else q->string = g_strdup_printf("%s%s%s",q->pkg, (q->cmd)?" ":"",(q->cmd)?q->cmd:"");
    g_free(value);
    value = (gchar *)xmlGetProp (node1, (const xmlChar *)"icon");
    q->icon = value?g_strdup(value):NULL;
    g_free(value);

    // XXX action options (emerge has none)

    // Only add menu items with string defined, except for the one with
    // empty action (string defined above).
    if (!q->string) return 0x01;
    p->type = MENUITEM_TYPE;
    p->parent_id = "pkg_menu_menu";
    p->id = g_strdup_printf("pkg_%s", q->cmd?q->cmd:"");
    NOOP("menu item id: %s\n", p->id);
    p->callback.data = q;

    p->callback.string = q->string;
    NOOP("menu string = %s \n", p->callback.string);
    p->callback.icon = q->icon;

    p->callback.function = process_cmd; 		
    // unused items:
    p->callback.function_id = 0x2001;
    // these are already initialized above
    //p->callback->key = 0;
    //p->callback->mask = 0;
    //p->callback->type = 0;

    
    
    return (0x01|0x02);
}

static void 
option_parse(xmlNodePtr node1, pkg_option_t *s){
    gchar *value = (gchar *)xmlGetProp (node1, (const xmlChar *)"loption");
    if (value && !strlen(value)) {g_free(value); value=NULL;}
    if (value) s->loption = g_strdup(value); g_free(value);
    value = (gchar *)xmlGetProp (node1, (const xmlChar *)"parameter");
    if (value) s->parameter = g_strdup(value); g_free(value);
    value = get_content_string(node1);
    if (value) {
        s->hlp = g_strdup_printf("<b>%s</b>\n%s", s->loption, value);  
    }
    g_free(value);
    value = (gchar *)xmlGetProp (node1, (const xmlChar *)"active");
    if (value) s->active = g_strdup(value);
    g_free(value);
    return;
}


static pkg_option_t *
parse_cmd_options(xmlNodePtr node){
    gint count = count_tags(node, "option");
    if (!count) return NULL;
    count++;
    pkg_option_t *cmd_options =
        (pkg_option_t *)malloc(count*sizeof (pkg_option_t));
    if (!cmd_options) return NULL;
    memset(cmd_options, 0, count*sizeof (pkg_option_t));


    pkg_option_t *s = cmd_options;
    xmlNodePtr node1;
    for(node1 = node->children; node1; node1 = node1->next) {
        if (strcasecmp("option", (gchar *)(node1->name)) != 0) continue;
        // parse it
        option_parse(node1, s);
        NOOP("option: %s\n", s->loption);
        s++;
    }
    return cmd_options;
}

static void
populateXML_menu(xmlDocPtr doc,const gchar *command){

    // count number of actions. This will equal the number of menu elements.
    gint action_count = count_xml_tags(doc, command, "action");

    // Allocate structures
    xml_cmds = 
        (pkg_command_t *)malloc((action_count+1)*sizeof(pkg_command_t));
    xml_menu_definitions = 
        (RodentMenuDefinition *)malloc((action_count+1)*sizeof(RodentMenuDefinition));

    if (!xml_cmds || !xml_menu_definitions){
        DBG("populateXML_menu() malloc failed: %s\n", strerror(errno));
        return;
    }
    memset(xml_cmds, 0, (action_count+1)*sizeof(pkg_command_t));
    memset(xml_menu_definitions, 0, (action_count+1)*sizeof(RodentMenuDefinition));


    xmlNodePtr node = xmlDocGetRootElement (doc);   
    for(node = node->children; node; node = node->next) {
        if (strcasecmp(command, (gchar *)(node->name))) continue;
        // Here we have our particular command section
        RodentMenuDefinition *p = xml_menu_definitions;
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
}


static void 
populate_global_options(xmlDocPtr doc, const gchar *command){
    // count number of options. 
    gint option_count = count_xml_tags(doc, command, "option");
    // Allocate structure
    xml_options = (pkg_option_t *) malloc((option_count+1)*sizeof(pkg_option_t));
    if (!xml_options){
        DBG("populate_global_options() malloc failed: %s\n", strerror(errno));
        return;
    }
    memset(xml_options, 0, (option_count+1)*sizeof(pkg_option_t));


    xmlNodePtr node = xmlDocGetRootElement (doc);
    for(node = node->children; node; node = node->next) {
        if (strcasecmp(command, (gchar *)(node->name))) continue;
        // Here we have our particular command section
        pkg_option_t *s = xml_options;
        xmlNodePtr node1;
        for(node1 = node->children; node1; node1 = node1->next) {
            if (strcasecmp("option", (gchar *)(node1->name)) == 0){
                option_parse(node1, s);
                s++;
            }
        }
    }
#ifdef DEBUG   
    pkg_option_t *pp;
    for (pp=xml_options; pp && pp->loption; pp++){
        NOOP("... %s\n", pp->loption);
    }
#endif
}

// Single variable: used for emerge, pacman 
static gboolean
custom_envars(xmlDocPtr doc){
    xmlNodePtr node = xmlDocGetRootElement (doc);
    for(node = node->children; node; node = node->next) {
        xmlNodePtr node1;
        for(node1 = node->children; node1; node1 = node1->next) {
            if (strcasecmp("environment", (gchar *)(node1->name))==0){
                // get value attribute
            gchar *value = (gchar *)xmlGetProp (node1, (const xmlChar *)"value");
            //fprintf(stderr, "node1->name=%s value=%s\n", (gchar *)(node1->name), value);
            if (!value) return FALSE;
            if (strcasecmp(value, "true")==0) return TRUE;
                if (strcmp(value, "1")==0) return TRUE;
                return FALSE;
            }
        }
    }
    return FALSE;
}

static gchar *
custom_envars_string(xmlDocPtr doc){
    xmlNodePtr node = xmlDocGetRootElement (doc);
    for(node = node->children; node; node = node->next) {
        xmlNodePtr node1;
        for(node1 = node->children; node1; node1 = node1->next) {
            if (strcasecmp("environment", (gchar *)(node1->name))==0){
                // get string attribute
                return (gchar *)xmlGetProp (node1, (const xmlChar *)"string");
            }
        }
    }
    return NULL;
}

static gboolean 
parse_xml(const gchar *xml_file, const gchar *command){
    DBG("parsing XML for %s\n", command);
    // 0. verify Xschema  FIXME
    validate_XML_schema(xml_file);
    if(access (xml_file, R_OK) != 0) {
        DBG ("access(%s, R_OK)!=0 (%s)\n", xml_file, strerror(errno));
        return FALSE;
    }
    xmlKeepBlanksDefault (0);
    xmlDocPtr doc;
    if((doc = xmlParseFile (xml_file)) == NULL) {
        gchar *g = g_strconcat (xml_file, ".bak", NULL);
        DBG ("invalid xml file %s.bak\n", xml_file);
        return FALSE;
    }
    /* Now parse the xml tree */
    // populate global options.
    use_custom_envar = custom_envars(doc);// emerge, pacman.
    g_free(use_custom_envar_string);
    use_custom_envar_string = custom_envars_string(doc);
    
    populate_global_options(doc, command);
    // populate menu and command structures
    populateXML_menu(doc, command);
    xmlFreeDoc (doc);
    return TRUE;
}

