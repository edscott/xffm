/* 
 *
 *  Copyright (C) 2012 Edscott Wilson Garcia <edscott@users.sf.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; .
*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define XMLTREE_C
#include "rodent.h"
/* this should be first 2 lines after headers: */
G_MODULE_EXPORT LIBRFM_MODULE 

#define XMLTREE_C
#include "tagfile.h"
#include "xmltree.h"

#define XMLTREE_key_type 0x01
#define XMLTREE_string_type 0x02
#define xmltree_item Tag_item_t
#define xmltree_attribute Attribute_item_t

typedef struct xmltree_t{
    gchar *window_title;
    gchar *xml_path;
    gchar *schema_path;
    gchar *namespace;
    gchar **editable_attributes;
    gchar *repeat_names;
    GHashTable *echo_hash;
    GHashTable *attribute_hash;
    GHashTable *editable_elements;
    gint text_activates_top_attribute;

    gboolean validated;

    void (*set_defaults)(struct xmltree_t *xmltree_p);
    void *set_defaults_data;
    xmltree_item *parent_tag;
    Tag_t *Tag_p;
} xmltree_t;


// XXX  XSD specific
static gboolean choice_callback(Tag_item_t *src_tag, 
        GtkTreeView *treeview, GtkTreeModel *treemodel, 
        GtkTreeIter *sibling, gint flag);
static void update_iconsXSD(GtkTreeModel *model, GtkTreeIter iter);
static GdkPixbuf *get_attribute_pixbuf(Attribute_item_t *row_attribute);
static Tag_item_t *set_attribute_colorXSD(GtkTreeModel *treemodel, GtkTreeIter *iter);
#include "xmltree.i"
#include "xsdtree.i"

G_MODULE_EXPORT
void *text_activates_top_attribute(xmltree_t *xmltree_p, void *q){
    if (!xmltree_p) return NULL;
    gint state = GPOINTER_TO_INT(q);
    NOOP("action set to %d\n", state);
    xmltree_p->text_activates_top_attribute = state;
    return GINT_TO_POINTER(1);
}


G_MODULE_EXPORT
void *xmltree_new(void){
    xmltree_t *xmltree_p = (xmltree_t *) malloc(sizeof(xmltree_t));
    if (!xmltree_p) {
	DBG("xmltree_new(): malloc failed: %s\n", strerror(errno));
	return NULL;
    }
    memset(xmltree_p, 0, sizeof(xmltree_t));
    xmltree_p->attribute_hash = g_hash_table_new(g_str_hash, g_str_equal);
    xmltree_p->echo_hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    xmltree_p->editable_elements = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    return (void *)xmltree_p; 
}

G_MODULE_EXPORT
void *xmltree_free(xmltree_t *xmltree_p){
    if (!xmltree_p) return NULL;
    g_free(xmltree_p->window_title);
    g_free(xmltree_p->xml_path);
    g_free(xmltree_p->schema_path);
    g_free(xmltree_p->namespace);
    g_free(xmltree_p->editable_attributes);
    g_free(xmltree_p->repeat_names);
    g_hash_table_destroy(xmltree_p->attribute_hash);
    g_hash_table_destroy(xmltree_p->echo_hash);
    g_hash_table_destroy(xmltree_p->editable_elements);
    g_free(xmltree_p);
    return NULL;
}

G_MODULE_EXPORT
void *xmltree_set_title(xmltree_t *xmltree_p, const gchar *title){
    if (!xmltree_p) return NULL;
    g_free(xmltree_p->window_title);
    xmltree_p->window_title = g_strdup(title);
    return (void *)(xmltree_p->window_title);
}

G_MODULE_EXPORT
void *xmltree_set_echo(xmltree_t *xmltree_p, const gchar *element, const gchar *attribute){
    if (!xmltree_p) return NULL;
    if (!xmltree_p->echo_hash){
        xmltree_p->echo_hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    }
    g_hash_table_replace(xmltree_p->echo_hash, g_strdup(element), g_strdup(attribute));
    return GINT_TO_POINTER(1);
}

G_MODULE_EXPORT
void *xmltree_set_xml(xmltree_t *xmltree_p, const gchar *xml_file){
    if (!xmltree_p) return NULL;
    g_free(xmltree_p->xml_path);
    xmltree_p->xml_path = g_strdup(xml_file);
    return (void *)(xmltree_p->xml_path);
}

G_MODULE_EXPORT
void *xmltree_set_schema(xmltree_t *xmltree_p, const gchar *schema_file){
    if (!xmltree_p) return NULL;
    g_free(xmltree_p->schema_path);
    xmltree_p->schema_path = g_strdup(schema_file);
    return (void *)(xmltree_p->schema_path);
}

G_MODULE_EXPORT
void *xmltree_set_editable_attribute(xmltree_t *xmltree_p, const gchar *attribute, void *type){
    if (!xmltree_p) return NULL;
    if (!xmltree_p->editable_attributes){
       xmltree_p->editable_attributes = (gchar **) malloc(2 * sizeof(gchar *));
       if (!xmltree_p->editable_attributes){
	  DBG("xmltree_set_editable_attribute(): malloc failed: %s\n", strerror(errno));
	  return NULL;
       }
       memset(xmltree_p->editable_attributes, 0, 2 * sizeof(gchar *));
       xmltree_p->editable_attributes[0] = g_strdup(attribute);
       g_hash_table_replace(xmltree_p->attribute_hash, xmltree_p->editable_attributes[0], type); 
       return (void *)(xmltree_p->editable_attributes);
    } 
    // count items
    gchar **q = xmltree_p->editable_attributes;
    gint count = 0;
    for (;q && *q; q++) count++;
    q = xmltree_p->editable_attributes;
    xmltree_p->editable_attributes = (gchar **) malloc((count+1) * sizeof(gchar *));
    if (!xmltree_p->editable_attributes){
      DBG("xmltree_set_editable_attribute(): malloc failed: %s\n", strerror(errno));
      return NULL;
    }
    memset(xmltree_p->editable_attributes, 0, (count+1) * sizeof(gchar *));
    gint i;
    for (i=0; i<count-1; i++){
	xmltree_p->editable_attributes[i] = g_strdup(q[i]);
    }
    g_strfreev(q);
    xmltree_p->editable_attributes[count-1] = g_strdup(attribute);
    g_hash_table_replace(xmltree_p->attribute_hash, xmltree_p->editable_attributes[count-1], type); 
    return (void *)(xmltree_p->editable_attributes);
}



G_MODULE_EXPORT
void *
xmltree (GtkButton * button, gpointer data) {
    fprintf(stderr, "xmltree() is deprecated. Use xmltree_run instead.\n");
    return NULL;
}

static gboolean
validate_with_schema(xmltree_t *xmltree_p){
    if (!xmltree_p->schema_path) {
        DBG("!xmltree_p->schema_path\n");
        gchar *text=g_strdup_printf("%s:  \n\n%s\n",
                _("No XSchema file specified"), 
                xmltree_p->schema_path);
        rfm_confirm(NULL, GTK_MESSAGE_ERROR, text, NULL,NULL);
        g_free(text);
        return FALSE; 
    }
    if (!g_file_test(xmltree_p->schema_path, G_FILE_TEST_EXISTS)) {
        DBG("!G_FILE_TEST_EXISTS\n");
        gchar *text=g_strdup_printf("%s:  \n\n%s\n",
                _("XSchema file does not exist"), 
                xmltree_p->schema_path);
        rfm_confirm(NULL, GTK_MESSAGE_ERROR, text, NULL,NULL);
        g_free(text);
        return FALSE;
    }
    if (g_file_test(xmltree_p->schema_path, G_FILE_TEST_IS_DIR)) {
        DBG("G_FILE_TEST_IS_DIR\n");
        gchar *text=g_strdup_printf("%s:  \n\n%s\n",
                _("XSchema file is not a regular file"), 
                xmltree_p->schema_path);
        rfm_confirm(NULL, GTK_MESSAGE_ERROR, text, NULL,NULL);
        g_free(text);
        return FALSE;
    }
    GError *error=NULL;
    DBG("attempting validation with %s\n",xmltree_p->schema_path);
    // Clean out old schema, if already loaded, and load new one
    // (if we keep tabs on stat info, this could be skipped if 
    //  not necessary, but WTH).
    tag_load_schema(xmltree_p->Tag_p, xmltree_p->schema_path, &error);
    
    if (!tag_validate(xmltree_p->Tag_p)){
        gchar *text=g_strdup_printf("%s: %s\n\n%s\n\n(%s)\n",
                _("Validate document"), xmltree_p->xml_path,
                _("The Document is not valid!"),
                xmltree_p->schema_path);
        rfm_confirm(NULL, GTK_MESSAGE_ERROR, text, NULL,NULL);
        g_free(text);
        return FALSE;
    }
#ifdef DEBUG
    gchar *text=g_strdup_printf("<b>%s (%s):</b>  \n\n <i>%s</i>\n",
            _("Schema validation passed"), 
            xmltree_p->schema_path,
            xmltree_p->xml_path);
    rfm_confirm(NULL, GTK_MESSAGE_INFO, text, NULL,NULL);
    g_free(text);
#endif
    return TRUE;
}

static void
validate_callback(GtkButton *b, void *data){
    xmltree_t *xmltree_p = data;
    if (validate_with_schema(xmltree_p)){
        // greenball somewhere
        rfm_set_bin_image(GTK_WIDGET(b), "xffm/emblem_greenball", SIZE_BUTTON);
    } else {
        // redball somewhere
        rfm_set_bin_image(GTK_WIDGET(b), "xffm/emblem_redball", SIZE_BUTTON);
    }
}


G_MODULE_EXPORT
void * xmltree_run(xmltree_t *xmltree_p){
    if (!xmltree_p) return NULL;

    GError *error=NULL;
    // FIXME: enter with dialog if null

    gboolean schema_processed = FALSE;
    //gboolean validated = FALSE;
    if (g_file_test(xmltree_p->xml_path, G_FILE_TEST_EXISTS)){

	xmltree_p->Tag_p =tag_new_from_file(xmltree_p->xml_path, &error);
	// validate with schema and dump if not valid
        if (xmltree_p->schema_path 
                && g_file_test(xmltree_p->schema_path, G_FILE_TEST_EXISTS) 
                && !g_file_test(xmltree_p->schema_path, G_FILE_TEST_IS_DIR)) {
            if (!validate_with_schema(xmltree_p)){
                tag_free(xmltree_p->Tag_p);
                xmltree_p->Tag_p = tag_new();  
            } //else validated = TRUE;
            schema_processed = TRUE;
        } 
    } else {
	xmltree_p->Tag_p = tag_new();
    }

    if (!schema_processed) {
      GSList *list = get_tag_item_list(xmltree_p->Tag_p, NULL, NULL);
      GSList *p=list;
      for (; p && p->data; p=p->next){
        Tag_item_t *item = p->data;
        Attribute_item_t *attribute =
            get_attribute(item, "noNamespaceSchemaLocation");
        if (attribute) {
            const gchar *schema = get_attribute_value(attribute);
            DBG("noNamespaceSchemaLocation=%s\n", schema);
            if (!g_file_test(schema, G_FILE_TEST_EXISTS)
                    && xmltree_p->schema_path 
                    && g_file_test(xmltree_p->schema_path, G_FILE_TEST_IS_DIR)){
                gchar *g = g_path_get_basename(schema);
                gchar *h = g_build_filename(xmltree_p->schema_path, g, NULL);
                g_free(g);
                g_free(xmltree_p->schema_path); 
                xmltree_p->schema_path = h;
            } else {
                g_free(xmltree_p->schema_path); 
                xmltree_p->schema_path = g_strdup(schema);
            }
            break;
        }
      }
      if (validate_with_schema(xmltree_p)){
          //validated = TRUE;
      } 
    }


    // FIXME: this is settings specific
    //        Add anything missing (may be everything)...
    //keybindings (xmltree_p->Tag_p);
    if (xmltree_p->set_defaults) (*(xmltree_p->set_defaults))(xmltree_p->set_defaults_data);
    // FIXME: enter with dialog if null
    //gchar *schema_file = g_build_filename(KEYBINDINGS_SCHEMA, NULL);
    gchar *schema_file = g_build_filename(xmltree_p->schema_path, NULL);
    Tag_t *schema_Tag_p = tag_new_from_schema_file(schema_file, &error);
    g_free(schema_file);

    GtkTreeModel *schemamodel= GTK_TREE_MODEL(gtk_tree_store_new (H_TREE_COLUMNS,
	    G_TYPE_POINTER,   // tag_item_p or parent's tag_item_p
	    G_TYPE_POINTER,   // attribute_item_p if applicable.
	    GDK_TYPE_PIXBUF, // pretty stuff
	    G_TYPE_STRING,   // tag
	    G_TYPE_STRING,   // attribute (string)
	    G_TYPE_STRING,   // value (string)
	    G_TYPE_STRING,    // prefix (string)
	    GDK_TYPE_PIXBUF, // button column
	    G_TYPE_STRING,    // color format
            G_TYPE_INT         // flags

	));
	    // G_TYPE_BOXED, // this might be useful later on
	    // G_TYPE_OBJECT,// and this one too...
    g_object_set_data(G_OBJECT(schemamodel), "xmltree_p", xmltree_p);

    // Create treemodels
    GtkTreeModel *model= GTK_TREE_MODEL(gtk_tree_store_new (H_TREE_COLUMNS,
	    G_TYPE_POINTER,   // tag_item_p or parent's tag_item_p
	    G_TYPE_POINTER,   // attribute_item_p if applicable.
	    GDK_TYPE_PIXBUF, // pretty stuff
	    G_TYPE_STRING,   // tag
	    G_TYPE_STRING,   // attribute (string)
	    G_TYPE_STRING,   // value (string)
	    G_TYPE_STRING,    // prefix (string)
	    GDK_TYPE_PIXBUF, // button column
	    G_TYPE_STRING,    // color format
            G_TYPE_INT         // flags

	));

    if (!GTK_IS_TREE_MODEL(model)){
	g_error("build_treemodel(): cannot create tree model!");
    }
    g_object_set_data(G_OBJECT(model), "xmltree_p", xmltree_p);
    // Create tag
    populate_tree_model_from_tag(xmltree_p->Tag_p, model, &error);
    // Build a graphic treeview.
    // Xml to the right
    GtkTreeView *treeview = build_treeview(model);
    g_object_set_data(G_OBJECT(treeview), "xmltree_p", xmltree_p);
    GtkTreePath *ipath = gtk_tree_path_new_from_string("0");
    gtk_tree_view_expand_row(treeview, ipath, FALSE);
    gtk_tree_path_free(ipath);

    g_object_set_data(G_OBJECT(model), "Tag_p", xmltree_p->Tag_p);

    gtk_tree_model_foreach(model, switch_colors, NULL);



    // Container for the graphic treeview.
    GtkWidget *treebox = build_treeview_box(treeview);


    GtkWidget *schemabox = NULL;
    // Build schema treeview
    if (schema_Tag_p) {
	//Create schema tag
	populate_tree_model_from_tag(schema_Tag_p, schemamodel, &error);
	// Schema on the left.
	GtkTreeView *schemaview = build_treeview(schemamodel);
	g_object_set_data(G_OBJECT(schemaview), "xmltree_p", xmltree_p);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(schemaview), FALSE);
	// Container for the graphic schemaview.
	schemabox = build_treeview_box(schemaview);
    }

    // Create a window for the treeview container.
    GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_object_set_data(G_OBJECT(window), "xmltree_p", xmltree_p);
    g_object_set_data(G_OBJECT(window), "Tag_p", xmltree_p->Tag_p);
    g_object_set_data(G_OBJECT(window), "model", model);
    // Create a shortcut reference.
    g_object_set_data(G_OBJECT(treeview), "parent_window", window);
    g_signal_connect(treeview, "key-press-event", G_CALLBACK(treeview_key), NULL);    
    
    // FIXME: enter as parameter
//    const gchar *window_title = _("Configuration of keybindings");
    const gchar *window_title = xmltree_p->window_title;



    if (window_title) gtk_window_set_title (GTK_WINDOW (window), window_title);
    gtk_window_set_default_size (GTK_WINDOW (window), 640, 480);
    GtkWidget *vbox = rfm_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (window), vbox);

    // Add a hpane
    //GtkWidget *hpane = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    GtkWidget *hpane = rfm_hpaned_new();
    gtk_box_pack_start (GTK_BOX (vbox), hpane, TRUE, TRUE, 3);
    //gtk_container_add (GTK_CONTAINER (vbox), hpane);
    if (schemabox) {
	gtk_paned_add1 (GTK_PANED(hpane), schemabox);
	gtk_paned_add2 (GTK_PANED(hpane), treebox);
    } else {
	gtk_paned_add1 (GTK_PANED(hpane), treebox);
    }

    GtkWidget * hbox = rfm_hbox_new (TRUE, 0);
    GtkWidget *b;

    b=rfm_dialog_button("xffm/emblem_blueball", _("Validate"));
    g_signal_connect(G_OBJECT(b), "clicked", G_CALLBACK (validate_callback), xmltree_p);
    gtk_box_pack_start (GTK_BOX (hbox), b, TRUE, FALSE, 3);


    if (xmltree_p->editable_attributes && *(xmltree_p->editable_attributes)) {
        b=rfm_dialog_button("xffm/stock_save", _("Save"));
        g_object_set_data(G_OBJECT(b), "callback", xml_edit_save);
        g_object_set_data(G_OBJECT(b), "window", window);
        g_signal_connect(G_OBJECT(b), "clicked", G_CALLBACK (xml_edit_save), window);
        gtk_box_pack_start (GTK_BOX (hbox), b, TRUE, FALSE, 3);
    }

    b=rfm_dialog_button("xffm/stock_window-close", _("Close"));


    g_object_set_data(G_OBJECT(b), "callback", xml_edit_destroy);
    g_object_set_data(G_OBJECT(b), "window", window);
    g_signal_connect(G_OBJECT(b), "clicked", G_CALLBACK (xml_edit_destroy), window);
    gtk_box_pack_start (GTK_BOX (hbox), b, TRUE, FALSE, 3);


    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);
  

    g_signal_connect(G_OBJECT(window), "destroy-event", G_CALLBACK (on_destroy_k), NULL);
    g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK (on_destroy_k), NULL);

 
    gtk_widget_show_all(window);
    gtk_paned_set_position(GTK_PANED(hpane), 0); 

    // Event loop.
    gtk_main();
    return NULL;
} 

#if 0
// FIXME: this does nothing but test, need not be exported...
void *load_custom_keybindings(void *p){
    // FIXME: enter as parameter
    gchar *keybindings_file = g_build_filename(KEYBINDINGS_FILE, NULL);
    if (g_file_test(keybindings_file, G_FILE_TEST_EXISTS)){
	NOOP(stderr, "load_custom_keybindings now\n");
    }
    g_free(keybindings_file);
    return NULL;
}
#endif

/////////////////////////////////////////////////////////////////////////
//  tag wrapper functions.
/////////////////////////////////////////////////////////////////////////

Tag_item_t *
xmltree_get_tag_item(xmltree_t *xmltree_p, xmltree_item *parent_tag, const gchar *tag_name){
    return (get_tag_item(xmltree_p->Tag_p, (Tag_item_t *)parent_tag, tag_name));
}
Tag_item_t *
xmltree_tag_item_add(xmltree_t *xmltree_p, xmltree_item *parent_tag, const gchar *tag_name){
    return (tag_item_add(xmltree_p->Tag_p, (Tag_item_t *)parent_tag, tag_name));
}
GSList *
xmltree_get_tag_item_list(xmltree_t *xmltree_p, xmltree_item *parent_tag, const gchar *tag_name){
    return (get_tag_item_list(xmltree_p->Tag_p, (Tag_item_t *)parent_tag, tag_name));
}

Attribute_item_t *
xmltree_get_attribute(xmltree_item *parent_tag, const gchar *attribute_name){
    return (get_attribute((Tag_item_t *)parent_tag, attribute_name));
}
const gchar *
xmltree_get_attribute_value(xmltree_attribute *item){
    return (get_attribute_value((Attribute_item_t *)item));
}

void *
xmltree_set_namespace(xmltree_t *xmltree_p, const gchar *namespace){
    g_free(xmltree_p->namespace);
    xmltree_p->namespace = g_strdup(namespace);
    return (xmltree_p->namespace);
}

void *
xmltree_set_defaults_function(xmltree_t *xmltree_p, void (*set_defaults)(xmltree_t *xmltree_p), void *set_defaults_data){
    xmltree_p->set_defaults = set_defaults;
    xmltree_p->set_defaults_data = set_defaults_data;
    return (set_defaults);
}

void *
xmltree_set_attribute_parent(xmltree_t *xmltree_p, xmltree_item *parent_tag){
    xmltree_p->parent_tag = parent_tag;
    return (parent_tag);
}

// attribute functions, requires parent tag to be set... 
// (reduced parameter list to fit in complex call)
Attribute_item_t *
xmltree_attribute_item_add(
        xmltree_t *xmltree_p, 
	const gchar *name,
	const gchar *value){
    if (!xmltree_p) return NULL;
    if (!xmltree_p->parent_tag){
        DBG("*** %s -> %s parent_tag not set. Please call XMLTREE_set_attribute_parent()\n",
                name, value);
        return NULL;
    }
    Attribute_item_t *retval;
    retval = attribute_item_add(xmltree_p->Tag_p, xmltree_p->parent_tag, 
	name, value, xmltree_p->namespace);
    NOOP("adding item to tag %p\n", retval);
    //xmltree_p->parent_tag = NULL;
    return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////////
#include <libxml/xmlschemas.h>

static const gchar *
validate_schema(gchar *schema_file){
    xmlDocPtr schema_doc = xmlReadFile(schema_file, NULL, XML_PARSE_NONET);
    if (schema_doc == NULL) return "Cannot read schema file";
    xmlSchemaParserCtxtPtr validationCtxt = xmlSchemaNewDocParserCtxt(schema_doc);
    //xmlSchemaValidCtxtPtr validationCtxt = xmlSchemaNewValidCtxt (NULL);
    if (validationCtxt == NULL) {
	// Cannot create a validation context for the schema
	return ("Failed to create the validation context.\n");
    }

    xmlSchemaPtr schema = xmlSchemaParse(validationCtxt);
    if (schema == NULL) {
        /* the schema itself is not valid */
        xmlSchemaFreeParserCtxt(validationCtxt);
	return ("schema itself is not valid\n");
    }

    xmlSchemaValidCtxtPtr valid_ctxt = xmlSchemaNewValidCtxt(schema);
    if (valid_ctxt == NULL) {
        xmlSchemaFree(schema);
        xmlSchemaFreeParserCtxt(validationCtxt);
        return ("unable to create a validation context for the schema\n");
    }
    return NULL;
}

static void
validate_callbackXSD(GtkButton *b, void *data){
    gchar *schema_file = data;
    if (validate_schema(schema_file)==NULL){
        // greenball somewhere
        rfm_set_bin_image(GTK_WIDGET(b), "xffm/emblem_greenball", SIZE_BUTTON);
    } else {
        // redball somewhere
        rfm_set_bin_image(GTK_WIDGET(b), "xffm/emblem_redball", SIZE_BUTTON);
    }
}


G_MODULE_EXPORT
void * xsdtree_run(xmltree_t *xmltree_p){
    if (!xmltree_p) return NULL;

    GError *error=NULL;
    // FIXME: enter with dialog if null

    if (g_file_test(xmltree_p->xml_path, G_FILE_TEST_EXISTS)){
	xmltree_p->Tag_p =tag_new_from_file(xmltree_p->xml_path, &error);
    } else {
	xmltree_p->Tag_p = tag_new();
    }


    // Create treemodels
    GtkTreeModel *schemamodel= GTK_TREE_MODEL(gtk_tree_store_new (H_TREE_COLUMNS,
	    G_TYPE_POINTER,   // tag_item_p or parent's tag_item_p
	    G_TYPE_POINTER,   // attribute_item_p if applicable.
	    GDK_TYPE_PIXBUF, // pretty stuff
	    G_TYPE_STRING,   // tag
	    G_TYPE_STRING,   // attribute (string)
	    G_TYPE_STRING,   // value (string)
	    G_TYPE_STRING,    // prefix (string)
	    GDK_TYPE_PIXBUF, // button column
	    G_TYPE_STRING,    // color format
            G_TYPE_INT         // flags
	));
	    // G_TYPE_BOXED, // this might be useful later on
	    // G_TYPE_OBJECT,// and this one too...
    g_object_set_data(G_OBJECT(schemamodel), "xmltree_p", xmltree_p);

    GtkTreeModel *model= GTK_TREE_MODEL(gtk_tree_store_new (H_TREE_COLUMNS,
	    G_TYPE_POINTER,   // tag_item_p or parent's tag_item_p
	    G_TYPE_POINTER,   // attribute_item_p if applicable.
	    GDK_TYPE_PIXBUF, // pretty stuff
	    G_TYPE_STRING,   // tag
	    G_TYPE_STRING,   // attribute (string)
	    G_TYPE_STRING,   // value (string)
	    G_TYPE_STRING,    // prefix (string)
	    GDK_TYPE_PIXBUF, // button column
	    G_TYPE_STRING,    // color format
            G_TYPE_INT         // flags
	));
    GtkTreeModel *filtermodel = gtk_tree_model_filter_new(model, NULL);

    if (!GTK_IS_TREE_MODEL(model)){
	g_error("build_treemodel(): cannot create tree model!");
    }
    g_object_set_data(G_OBJECT(model), "xmltree_p", xmltree_p);
    GtkWidget *treebox=NULL;
     

    GtkWidget *schemabox = NULL;
    // Build schema treeview
    if (xmltree_p->Tag_p) {
	//Create schema tag
	populate_tree_model_from_tag(xmltree_p->Tag_p, schemamodel, &error);
	// Schema on the left.
	GtkTreeView *schemaview = build_treeview(schemamodel);
	g_object_set_data(G_OBJECT(schemaview), "xmltree_p", xmltree_p);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(schemaview), FALSE);
	// Container for the graphic schemaview.
	schemabox = build_treeview_box(schemaview);
    }

    // Create a window for the treeview container.
    GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_object_set_data(G_OBJECT(window), "xmltree_p", xmltree_p);
    g_object_set_data(G_OBJECT(window), "Tag_p", xmltree_p->Tag_p);
    g_object_set_data(G_OBJECT(model), "Tag_p", xmltree_p->Tag_p);
    g_object_set_data(G_OBJECT(window), "model", model);
 
    GtkTreeView *treeview;
    // Create schema generated xml tag
    Tag_t *xml_Tag_p = tag_new_from_schema_tag(xmltree_p, &error);
    g_object_set_data(G_OBJECT(window), "xml_Tag_p", xml_Tag_p);
    g_object_set_data(G_OBJECT(model), "xml_Tag_p", xml_Tag_p);

    if (xml_Tag_p) {
        populate_tree_model_from_tag(xml_Tag_p, model, &error);
        // Build a graphic treeview.
        // Xml to the right
        treeview = build_treeview(filtermodel);
        g_object_set_data(G_OBJECT(treeview), "filter_model", filtermodel);
        gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(filtermodel), 
                show_visible_row, NULL, NULL);
        g_object_set_data(G_OBJECT(treeview), "xmltree_p", xmltree_p);
        GtkTreePath *ipath = gtk_tree_path_new_from_string("0");
        gtk_tree_view_expand_row(treeview, ipath, FALSE);
        gtk_tree_path_free(ipath);
        g_object_set_data(G_OBJECT(treeview), "Tag_p", xml_Tag_p);
        g_object_set_data(G_OBJECT(model), "Tag_p", xml_Tag_p);
//        g_object_set_data(G_OBJECT(model), "Tag_p", xmltree_p->Tag_p);

        //gtk_tree_model_foreach(model, switch_colors, NULL);
        gtk_tree_model_foreach(model, switch_colorsXSD, NULL);

        // Container for the graphic treeview.
        treebox = build_treeview_box(treeview);
        // Create a shortcut reference.
        g_object_set_data(G_OBJECT(treeview), "parent_window", window);
        g_signal_connect(treeview, "key-press-event", G_CALLBACK(treeview_key), NULL);   
    }

    
    const gchar *window_title = xmltree_p->window_title;



    if (window_title) gtk_window_set_title (GTK_WINDOW (window), window_title);
    gtk_window_set_default_size (GTK_WINDOW (window), 640, 480);
    GtkWidget *vbox = rfm_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (window), vbox);

    // Add a hpane
    //GtkWidget *hpane = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    GtkWidget *hpane = rfm_hpaned_new();
    gtk_box_pack_start (GTK_BOX (vbox), hpane, TRUE, TRUE, 3);
    //gtk_container_add (GTK_CONTAINER (vbox), hpane);
    if (schemabox) {
	gtk_paned_add1 (GTK_PANED(hpane), schemabox);
	if (treebox) gtk_paned_add2 (GTK_PANED(hpane), treebox);
    } else {
	if (treebox) gtk_paned_add1 (GTK_PANED(hpane), treebox);
    }

    GtkWidget * hbox = rfm_hbox_new (TRUE, 0);
    GtkWidget *b;

    b=rfm_dialog_button("xffm/emblem_blueball", _("Validate"));
    g_signal_connect(G_OBJECT(b), "clicked", G_CALLBACK (validate_callbackXSD), xmltree_p->xml_path);
    gtk_box_pack_start (GTK_BOX (hbox), b, TRUE, FALSE, 3);


    if (xmltree_p->editable_attributes && *(xmltree_p->editable_attributes)) {
        b=rfm_dialog_button("xffm/stock_save", _("Save"));
        g_object_set_data(G_OBJECT(b), "callback", xsd_edit_save);
        g_object_set_data(G_OBJECT(b), "window", window);
        g_signal_connect(G_OBJECT(b), "clicked", G_CALLBACK (xsd_edit_save), window);
        gtk_box_pack_start (GTK_BOX (hbox), b, TRUE, FALSE, 3);
    }

    b=rfm_dialog_button("xffm/stock_window-close", _("Close"));


    g_object_set_data(G_OBJECT(b), "callback", xml_edit_destroy);
    g_object_set_data(G_OBJECT(b), "window", window);
    g_signal_connect(G_OBJECT(b), "clicked", G_CALLBACK (xml_edit_destroy), window);
    gtk_box_pack_start (GTK_BOX (hbox), b, TRUE, FALSE, 3);


    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);
  

    g_signal_connect(G_OBJECT(window), "destroy-event", G_CALLBACK (on_destroy_k), NULL);
    g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK (on_destroy_k), NULL);

 
    gtk_widget_show_all(window);
    gtk_paned_set_position(GTK_PANED(hpane), 0); 

    // Event loop.
    gtk_main();
    return NULL;
} 

G_MODULE_EXPORT
void *
xsdtree_show_hidden(void *state){
    show_hidden = GPOINTER_TO_INT(state);
    return NULL;
}

