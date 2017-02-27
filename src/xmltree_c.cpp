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

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

#ifdef COPYRIGHT_INFORMATION
#include "gplv3.h"
#endif
/*
 * Copyright (C) 2002-2012 Edscott Wilson Garcia
 * EMail: edscott@users.sf.net
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; 
 */
//////////////////////////////////////////////////////////////////////////
#include "tagfile.h"
#include "rodent_gridview.h"
#include "rodent_popup_callbacks.h"
//////////////////////////////////////////////////////////////////////////
#define IS_SEQUENCE_CTL 0x01
#define IS_SEQUENCE_CHILD 0x02
#define IS_SEQUENCE_NESTED 0x03

#define IS_CHOICE_CTL 0x010
#define IS_CHOICE_CHILD 0x020
#define IS_CHOICE_NESTED 0x030

#define IS_ALL_CTL 0x0100
#define IS_ALL_CHILD 0x0200
#define IS_ALL_NESTED 0x0300

#define IS_HIDDEN_ROW 0x01000

#define HIDDEN_COLOR  "#990000"
#define EDIT_COLOR "blue"
#define READONLY_COLOR "gray"
#define REQUIRED_COLOR "red"

static GHashTable *noneditable_hash=NULL;

enum {
    H_TAG_ITEM_COLUMN,
    H_ATTRIBUTE_ITEM_COLUMN,
    H_PIXBUF_COLUMN,
    H_NS_COLUMN,
    H_TAG_COLUMN,
    H_ATTRIBUTE_COLUMN,
    H_VALUE_COLUMN, 
    H_BUTTON_COLUMN,  
    H_COLOR_COLUMN,
    H_FLAG_COLUMN,
    H_TREE_COLUMNS
};

enum {
    UNDEFINED_DATA,
    TABLE_DATA,
    TAG_DATA,
    ATTRIBUTE_DATA
};

/////////////////////////////////////////////////////////////////////////
//  FUNCTIONS
/////////////////////////////////////////////////////////////////////////
/*static GdkColor black={0,0,0,0};
static GdkColor blue={0,0x0,0x0,0x7777};
static GdkColor grey={0,0x6666,0x6666,0x6666};
static GdkColor red={0,0xdddd,0x0,0x0};*/

    static GdkPixbuf *broken=NULL;
    static GdkPixbuf *OK=NULL;
    static GdkPixbuf *KO=NULL;
    static GdkPixbuf *index_pix=NULL;
    static GdkPixbuf *index_pix2=NULL;
    static GdkPixbuf *list_add=NULL;
    static GdkPixbuf *question=NULL;
    static GdkPixbuf *list_remove=NULL;
    static GdkPixbuf *bold=NULL;
    static GdkPixbuf *redball=NULL;
    static GdkPixbuf *greenball=NULL;
    static GdkPixbuf *red=NULL;
    static GdkPixbuf *green=NULL;
    static GdkPixbuf *blue=NULL;
    static GdkPixbuf *folder_red=NULL;
    static GdkPixbuf *folder_green=NULL;
    static GdkPixbuf *keyboard=NULL;
    static GdkPixbuf *strikethrough=NULL;
    static GdkPixbuf *include_on=NULL;
    static GdkPixbuf *include_off=NULL;
    static GdkPixbuf *repeat_value=NULL;
static GHashTable *dialog_hash = NULL;
static gchar *
mod_string(guint mask){
    if (!mask) return NULL;
	gchar *mod=g_strdup("");
	if (mask & GDK_SHIFT_MASK) {
	    gchar *g = g_strconcat (mod,_("Shift"), "+", NULL);
	    g_free(mod);
	    mod = g;
	}
	if (mask & GDK_CONTROL_MASK) {
	    gchar *g = g_strconcat (mod,_("Control"), "+", NULL);
	    g_free(mod);
	    mod = g;
	}
	if (mask & GDK_MOD1_MASK)  {
	     gchar *g = g_strconcat (mod,_("Alt"), "+", NULL);
	    g_free(mod);
	    mod = g;
	}
	if (strlen(mod)==0) {
	    g_free(mod); 
	    mod = g_strdup_printf ("0x%x+", mask);
	}
	return mod;
}
static gchar *key_string(guint keyval){
	gchar *key=NULL;
	if ((keyval > 0x40 && keyval < 0x5b) ||(keyval >0x02f  && keyval < 0x03a)) {
	    key = g_strdup_printf("%c", keyval);
	}
	else if (keyval > 0x60 && keyval < 0x7b) {
	    key = g_strdup_printf("%c", keyval);
	}
	else if (keyval > 0xffbd && keyval < 0xffca) { // function keys f1-f12
	    key = g_strdup_printf("F%d", keyval-0xffbd);
	}
	else { // other keys 
	    switch (keyval){
		case GDK_KEY_Home: key = g_strdup(_("Home")); break;
		case GDK_KEY_Left: key = g_strdup(_("Left")); break;
		case GDK_KEY_Up: key = g_strdup(_("Up")); break;
		case GDK_KEY_Right: key = g_strdup(_("Right")); break;
		case GDK_KEY_Down: key = g_strdup(_("Down")); break;
		case GDK_KEY_Page_Up: key = g_strdup(_("Page up")); break;
		case GDK_KEY_Page_Down: key = g_strdup(_("Page down")); break;
		case GDK_KEY_End: key = g_strdup(_("End")); break;
		case GDK_KEY_Begin: key = g_strdup(_("Begin")); break;
		case GDK_KEY_Delete: key = g_strdup(_("Delete")); break;
		case GDK_KEY_Insert: key = g_strdup(_("Insert")); break;
		case GDK_KEY_equal: key = g_strdup(_("Equal")); break;
		case GDK_KEY_plus: key = g_strdup(_("Plus")); break;
		case GDK_KEY_minus: key = g_strdup(_("Minus")); break;
		case GDK_KEY_KP_Add: key = g_strdup(_("Add")); break;
		case GDK_KEY_KP_Subtract: key = g_strdup(_("Subtract")); break;
	    } 
	}
	if (!key) key = g_strdup_printf("0x%x", keyval);
	return key;
}


static void 
set_editable_element(xmltree_t *xmltree_p, const gchar *element, gboolean state){
    if (!xmltree_p || !element) return ;
    g_hash_table_replace(xmltree_p->editable_elements, g_strdup(element), GINT_TO_POINTER(state));
    return;
}

static gboolean
get_editable_element(xmltree_t *xmltree_p, const gchar *element){
    if (!xmltree_p || !element) return FALSE;
    void *r = g_hash_table_lookup(xmltree_p->editable_elements, element);
    return GPOINTER_TO_INT(r);
}

static gboolean
get_editable_attribute(xmltree_t *xmltree_p, const gchar *attribute){
    gboolean retval = FALSE;
    if (!attribute) return FALSE;
    gchar **p = xmltree_p->editable_attributes;
    for (;p && *p; p++){
	if (strcasecmp(*p, attribute) == 0){
            NOOP( "editatble set for %s\n", *p);
	    retval = TRUE;
	    break;
	}
    }
    NOOP( "get_editable_attribute(%s) = %d\n", attribute, retval);
    return retval;
}

#if 0
static Tag_item_t *
set_attribute_color(GtkTreeModel *treemodel, GtkTreeIter *iter){
	gchar *attribute=NULL;
	Tag_item_t *tag;
	Tag_item_t *parent_tag;
        Attribute_item_t *item = NULL;
        xmltree_t *xmltree_p = g_object_get_data(G_OBJECT(treemodel), "xmltree_p");
	gtk_tree_model_get(treemodel, iter, 
		H_TAG_ITEM_COLUMN, &tag,
                H_ATTRIBUTE_COLUMN, &attribute, 
                H_ATTRIBUTE_ITEM_COLUMN, &item, -1);
        if (!item) parent_tag = get_parent_tag(tag);
        else parent_tag=get_attribute_parent(item);
        
	if (!attribute) {
	    gtk_tree_store_set((GtkTreeStore *) treemodel, iter, H_COLOR_COLUMN, "black", -1);
	} else {
            if (noneditable_hash && g_hash_table_lookup(noneditable_hash, item)){
	        gtk_tree_store_set((GtkTreeStore *) treemodel, iter, H_COLOR_COLUMN, READONLY_COLOR, -1);
            } else if (get_editable_attribute(xmltree_p, attribute)){
	        gtk_tree_store_set((GtkTreeStore *) treemodel, iter, H_COLOR_COLUMN, EDIT_COLOR, -1);
	    } else if (get_editable_element(xmltree_p, get_tag_name(parent_tag))){
	        gtk_tree_store_set((GtkTreeStore *) treemodel, iter, H_COLOR_COLUMN, EDIT_COLOR, -1);
            } else {
	        gtk_tree_store_set((GtkTreeStore *) treemodel, iter, H_COLOR_COLUMN, READONLY_COLOR, -1);
            }
	}

        return tag;

}
#endif


static
gboolean 
switch_colors(		GtkTreeModel * treemodel, 
			GtkTreePath * treepath, 
			GtkTreeIter * iter, 
			gpointer data)
{
    //Tag_item_t *tag = set_attribute_color(treemodel, iter);
    return FALSE;
}


static void
set_button_serial(GtkTreeView *treeview, GdkEventButton * event){
    GtkTreePath *treepath;
    GtkTreeViewColumn *column;
    if(!gtk_tree_view_get_path_at_pos(treeview, event->x, event->y, 
		    &treepath, &column, NULL, NULL)) {
	return;
    }
    gchar *path = gtk_tree_path_to_string (treepath);
    gchar *oldpath = g_object_get_data(G_OBJECT(treeview), "button_path");
    g_free(oldpath);
    g_object_set_data(G_OBJECT(treeview), "button_path", path);
    g_object_set_data(G_OBJECT(treeview), "button_column", column);
    gtk_tree_path_free(treepath);
    return;
}

static void
unset_button_serial(GtkTreeView *treeview, GdkEventButton * event){
    gchar *oldpath = g_object_get_data(G_OBJECT(treeview), "button_path");
    g_free(oldpath);
    g_object_set_data(G_OBJECT(treeview), "button_path", NULL);
    g_object_set_data(G_OBJECT(treeview), "button_column", NULL);
    
}

 
static gboolean
check_button_serial(GtkTreeView *treeview, GdkEventButton * event){
    gint status=FALSE;
    GtkTreePath *treepath;
    GtkTreeViewColumn *column;
    if(gtk_tree_view_get_path_at_pos(treeview, event->x, event->y, 
		    &treepath, &column, NULL, NULL)) {
	gchar *path = gtk_tree_path_to_string (treepath);
	gchar *oldpath = 
	    g_object_get_data(G_OBJECT(treeview), "button_path");
	GtkTreeViewColumn *oldcolumn =
	    g_object_get_data(G_OBJECT(treeview), "button_column");
	status = (oldpath && path && 
		strcmp(path, oldpath)==0 && 
		column == oldcolumn);
	NOOP("path=%s oldpath=%s column=0x%x oldcolumn=0x%x status=%d\n",
		path, oldpath, 
		GPOINTER_TO_INT(column), GPOINTER_TO_INT(oldcolumn),
		status);
	g_free(path);
	gtk_tree_path_free(treepath);
    }
    return status;
}


static
gboolean 
find_keybinding(	GtkTreeModel * model, 
			GtkTreePath * treepath, 
			GtkTreeIter * iter, 
			gpointer data)
{
    Attribute_item_t *Attribute_item_p;

    gtk_tree_model_get (model, iter,  
		H_ATTRIBUTE_ITEM_COLUMN, &Attribute_item_p,
		-1);
    if (Attribute_item_p) return FALSE; // Only affect tags, not attributes.
    Tag_item_t *Tag_item_p;
    gtk_tree_model_get (model, iter,  
		H_TAG_ITEM_COLUMN, &Tag_item_p,
		-1);
    gint flag = GPOINTER_TO_INT(get_tag_item_user_data(Tag_item_p)); 

    if (flag & 0x01){
	    // change color to red.
	    gtk_tree_store_set(GTK_TREE_STORE(model), iter, 
		H_PIXBUF_COLUMN, repeat_value,
		H_COLOR_COLUMN, "red", 
		-1);
    } else {
	    // change color to black.
	    GdkPixbuf *pixbuf = keyboard;
	    Attribute_item_t *att_p = get_attribute(Tag_item_p, "icon_id");
	    if (att_p) {
		const gchar *icon_id = get_attribute_value(att_p);
		if (icon_id && strlen(icon_id)){
		    pixbuf = rfm_get_pixbuf(icon_id, TINY_ICON_SIZE);
		}
	    }
	    gtk_tree_store_set(GTK_TREE_STORE(model), iter, 
		H_PIXBUF_COLUMN, pixbuf,
		H_COLOR_COLUMN, "black", 
		-1);	    
	    g_object_unref(pixbuf);
	    
    }
    return FALSE;
}


static
gboolean 
check_clean(	GtkTreeModel * model, 
			GtkTreePath * treepath, 
			GtkTreeIter * iter, 
			gpointer data)
{
    gchar *color;
    gboolean *clean = data;
    gtk_tree_model_get (model, iter, 
		H_COLOR_COLUMN, &color,
		-1);
    if (!color) return FALSE;
    if (strcmp(color, "red")==0){
	*clean = FALSE;
	rfm_confirm(NULL, GTK_MESSAGE_ERROR, _("You may not specify duplicate patterns"),NULL, NULL);
	return TRUE;
    }
    return FALSE;
}


/////////////////////////////////////////////////////////////////////////
//  DIALOG CALLBACKS
/////////////////////////////////////////////////////////////////////////

static void
ak_destroy (GtkButton * button, gpointer data) {
    GtkEntry *entry = data;
    GtkWidget *window = g_object_get_data(G_OBJECT(entry), "attribute_window");
    gchar *key = g_object_get_data(G_OBJECT(window), "path_string");
    
    g_hash_table_steal(dialog_hash, key);
    g_free(key);
    gtk_widget_destroy(window);

}
static gboolean
on_destroy_child  (GtkWidget *window, GdkEvent  *event, gpointer data)  {
    gchar *path_string = g_object_get_data(G_OBJECT(window), "path_string");
    if(path_string) {
	g_hash_table_steal (dialog_hash, path_string);
	g_object_set_data(G_OBJECT(window), "path_string", NULL);
        g_free(path_string);
    }
    return FALSE;
}


static gboolean
signal_keyboard_event (
    GtkWidget *entry,
    GdkEventKey * event,
    gpointer data
) {
    /* asian Input methods */
    if(event->keyval == GDK_KEY_space && (event->state & (GDK_MOD1_MASK | GDK_SHIFT_MASK))) {
        return FALSE;
    }
    gchar *key=NULL;
    gchar *key_s=key_string(event->keyval);
    guint mod=0;
    gchar *mod_s=NULL;
    if (event->keyval == GDK_KEY_Tab 
	    || event->keyval == GDK_KEY_Return
	    || event->keyval == GDK_KEY_KP_Enter) return FALSE;
    if (event->state ) {
	if (event->state & GDK_SHIFT_MASK) mod |= GDK_SHIFT_MASK;
	if (event->state & GDK_CONTROL_MASK) mod |= GDK_CONTROL_MASK;
	if (event->state & (GDK_MOD1_MASK|GDK_MOD5_MASK))  mod |= (GDK_MOD1_MASK|GDK_MOD5_MASK) ;
	mod_s=mod_string(mod);	
    }
    if (mod_s) key=g_strdup_printf("%s%s", mod_s, key_s);
    else key=g_strdup_printf("%s", key_s);
    gtk_entry_set_text (GTK_ENTRY(entry), key);
    g_free(key);
    g_free(key_s);
    g_free(mod_s);
    g_object_set_data(G_OBJECT(entry), "mask", GUINT_TO_POINTER(mod));
    g_object_set_data(G_OBJECT(entry), "key", GUINT_TO_POINTER(event->keyval));


    return TRUE;

}
static void
ak_apply (GtkButton * button, gpointer data) {
    GtkWidget *entry = data;
    GtkWidget *window = g_object_get_data(G_OBJECT(entry), "attribute_window");
    GtkTreeModel *model = g_object_get_data(G_OBJECT(window), "model");
    const gchar *path_string = g_object_get_data(G_OBJECT(window), "path_string");
    struct xmltree_t *xmltree_p = g_object_get_data(G_OBJECT(window), "xmltree_p");
    GtkTreeIter iter;
    const gchar *value = NULL;
    
    if (GTK_IS_ENTRY(entry)) value = gtk_entry_get_text(GTK_ENTRY(entry));
#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
    else if (GTK_IS_COMBO_BOX(entry)) value = gtk_combo_box_get_active_text(GTK_COMBO_BOX(entry));
#else
    else if (GTK_IS_COMBO_BOX_TEXT(entry)) value = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(entry));
#endif
    if (!value) value ="gtk_entry_get_text returns NULL";

    const gchar *variable_type = g_object_get_data(G_OBJECT(entry), "variable_type");
        NOOP( "variable type = %s\n", variable_type);
    if (variable_type){
        if (strstr(variable_type, "double")){
            char *r;
            errno=0;
            double d = strtod(value, &r);
            if (errno==ERANGE || (!d && value==r)){
                gchar *c = g_strdup_printf("<b>%s</b> cannot be converted to a valid double", value);
                rfm_confirm(NULL, GTK_MESSAGE_ERROR, c, NULL, NULL);
                g_free(c);
                return;
            }
        }
    }

    // find attribute with value repeated.
    GtkTreeStore *store;
    GtkTreeModelFilter *filter_model = g_object_get_data(G_OBJECT(window), "filter_model");
    if (filter_model) store = GTK_TREE_STORE(gtk_tree_model_filter_get_model(filter_model));
    else store = GTK_TREE_STORE(model);
    NOOP( "pathstring=%s\n", path_string);
    GtkTreePath *treepath = gtk_tree_path_new_from_string (path_string);
    if (filter_model){
        GtkTreePath *tp = gtk_tree_model_filter_convert_path_to_child_path(filter_model, treepath);
        if (!tp){
            fprintf(stderr, "cannot convert filter path to child path\n");
            return;
        }
        gtk_tree_path_free(treepath);
        treepath=tp;
    }

    if (gtk_tree_model_get_iter (GTK_TREE_MODEL(store), &iter, treepath)){

	// Get associated attribute_item_p and parent tag_item_p
	Tag_item_t *Tag_item_p;
	Attribute_item_t *Attribute_item_p;
        gchar *attribute;
	gtk_tree_model_get (GTK_TREE_MODEL(store), &iter,  
		H_TAG_ITEM_COLUMN, &Tag_item_p, 
		H_ATTRIBUTE_COLUMN, &attribute, 
		H_ATTRIBUTE_ITEM_COLUMN, &Attribute_item_p, 
		-1);
	set_attribute_value(Attribute_item_p, value);

        /// XXX   XSD stuff ....
	// update the viewable treeview and model
	gtk_tree_store_set (store, &iter, 
		H_VALUE_COLUMN, value,
                H_PIXBUF_COLUMN, get_attribute_pixbuf(Attribute_item_p), 
		-1);
        set_attribute_colorXSD(GTK_TREE_MODEL(store), &iter);
        update_iconsXSD(GTK_TREE_MODEL(store), iter);
            
        //////////////////////

        if (get_editable_attribute(xmltree_p, attribute)) {
            NOOP( "Gotcha: update parent too...\n");
            GtkTreeIter parent;
            if (gtk_tree_model_iter_parent(GTK_TREE_MODEL(store), &parent, &iter)){
                gtk_tree_store_set (store, &parent, 
                        H_VALUE_COLUMN, value,
                        -1);
            }
        }

        // if not key selection, we are done.
                                ;
        if (!g_object_get_data(G_OBJECT(window), "XMLTREE_key_type")){
            gchar *key = g_object_get_data(G_OBJECT(window), "path_string");
            g_hash_table_steal(dialog_hash, key);
            g_free(key);
            gtk_widget_destroy(window);
            return;
        }

    // XXX from here down are specifics to keybindings dialog...
    // Must update "mask" and "key" attributes as well.
	Attribute_item_t *a_p;
	a_p = get_attribute(Tag_item_p, "key");
	if (a_p){
	    guint key = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(entry), "key"));
	    gchar *key_s = g_strdup_printf("%d", key);
	    set_attribute_value(a_p, key_s);
	    g_free(key_s);	
	}
	a_p = get_attribute(Tag_item_p, "mask");
	if (a_p){
	    guint mask = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(entry), "mask"));
	    gchar *mask_s = g_strdup_printf("%d", mask);
	    set_attribute_value(a_p, mask_s);
	    g_free(mask_s);	
	}
    }
    gtk_tree_path_free(treepath);

    // XXX: the repeat stuff is specific to keybindings dialog...
    // Duplicate test
    Tag_t *Tag_p =  g_object_get_data(G_OBJECT(model), "Tag_p");
    GSList *list = get_full_attribute_list(Tag_p, NULL, "Keybinding");
    GSList *tmp = list;
    for (;tmp && tmp->data; tmp = tmp->next){
	Attribute_item_t *Attribute_item_p = tmp->data;
	Tag_item_t *parent = get_attribute_parent(Attribute_item_p);
	set_tag_item_user_data(parent, GINT_TO_POINTER(0x0));
    }
    for (tmp = list;tmp && tmp->data; tmp = tmp->next){
	Attribute_item_t *Attribute_item_p = tmp->data;
	Tag_item_t *parent = get_attribute_parent(Attribute_item_p);
	const gchar *value = get_attribute_value(Attribute_item_p);
	GSList *tmp2 = tmp->next;
	//NOOP(stderr, "duptest: %s\n", value); continue;
	gboolean dupped = FALSE;
	for (;tmp2 && tmp2->data; tmp2 = tmp2->next){
	    const gchar *value2 = get_attribute_value((Attribute_item_t *)tmp2->data);
	    Tag_item_t *parent2 = get_attribute_parent((Attribute_item_t *)(tmp2->data));
	    if (!parent || !parent2) g_error("terribly wrong\n");
	    if (value && value2 && strcmp(value, value2) == 0){
		dupped = TRUE;
		set_tag_item_user_data(parent2, GINT_TO_POINTER(0x01));
		// NOOP(stderr, "redlisted: %s, %s\n", get_tag_name(parent), get_tag_name(parent2));
	    }
	}
	if (dupped) {
	    set_tag_item_user_data(parent, GINT_TO_POINTER(0x01));
	} 
    }
    g_slist_free(list);
    gtk_tree_model_foreach(GTK_TREE_MODEL(store), find_keybinding, NULL);


    gchar *key = g_object_get_data(G_OBJECT(window), "path_string");
    
    g_hash_table_steal(dialog_hash, key);
    g_free(key);

    gtk_widget_destroy(window);


}

static void
ak_erase (GtkButton * button, gpointer data) {
    GtkEntry *entry = data;
    gtk_entry_set_text(entry, "");
}

/////////////////////////////////////////////////////////////////////////
//    DIALOG CREATION FUNCTIONALITY
/////////////////////////////////////////////////////////////////////////

static void *
tag_box(const gchar *head, const gchar *tag, const gchar *value, gint flag, Tag_item_t *parent_tag){
    GtkWidget *vbox = rfm_vbox_new (FALSE, 0);
    
    GtkWidget *hbox = rfm_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 3);
    GtkWidget *label = gtk_label_new("");
    gchar *text = g_strdup_printf("<b>%s</b> ", head);
    gtk_label_set_markup(GTK_LABEL(label), text);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 3);

    hbox = rfm_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 3);
    text = g_strdup_printf("<i>%s:</i> ", strcasecmp(tag,"text")?tag:get_tag_name(parent_tag));
    label = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(label), text);
    g_free(text);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 3);

    GtkEntryBuffer *entry_buffer =
	gtk_entry_buffer_new (value, -1);
        
    //if (value) g_strstrip(value);
    GtkWidget *entry = NULL;

    gchar *att_name = g_strdup_printf("%s:type", tag);
    Attribute_item_t *type_item = get_attribute(parent_tag, att_name);
    g_free(att_name);
    const gchar *variable_type = NULL;
    const gchar *variable_subtype = NULL;
    if (type_item){
        variable_type = get_attribute_value(type_item);
        Attribute_item_t *subtype_item = get_attribute(parent_tag, "list:itemType");
        if (subtype_item) variable_subtype = get_attribute_value(subtype_item);
        if (variable_type && strstr(variable_type, "integer")){
            gint min = 1;
            gint max = 99999999;
            gint step = 1;
            entry = gtk_spin_button_new_with_range((double)min, (double)max, (double)step);
        }
    } 
        
    // do we have a pattern restriction?    
    
    att_name = g_strdup_printf("%s:pattern", tag);
    Attribute_item_t *pattern_item = get_attribute(parent_tag, att_name);
    g_free(att_name);
    if (pattern_item){
        const gchar *pattern = get_attribute_value(pattern_item);
        gchar **pattern_v = g_strsplit(pattern, "|", -1);
        gchar **p = pattern_v;
#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
         // use a text combo
        entry = gtk_combo_box_new_text();
        if (value && strlen(value)) gtk_combo_box_prepend_text(GTK_COMBO_BOX(entry), value);
        for (;p && *p; p++){
            g_strstrip(*p);
            if (strcmp(value, *p)==0) continue;
            gtk_combo_box_append_text(GTK_COMBO_BOX(entry), *p);
        }
#else
       // use a text combo
        entry = gtk_combo_box_text_new();
        if (value && strlen(value)) gtk_combo_box_text_prepend_text(GTK_COMBO_BOX_TEXT(entry), value);
        for (;p && *p; p++){
            g_strstrip(*p);
            if (value && strcmp(value, *p)==0) continue;
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(entry), *p);
        }
#endif
        g_strfreev(pattern_v);
        gtk_combo_box_set_active(GTK_COMBO_BOX(entry), 0);
    } else if (!entry){
        entry = gtk_entry_new_with_buffer(entry_buffer);
    }


    //gchar *pattern;
    gtk_box_pack_start (GTK_BOX (hbox), entry, FALSE, FALSE, 3);

    // single key input
    if (flag==1) {
        g_signal_connect (G_OBJECT (entry), "key-press-event", G_CALLBACK (signal_keyboard_event), NULL);
    }
    //gtk_container_add (GTK_CONTAINER (vbox), hbox);
    if (variable_type){
        g_object_set_data(G_OBJECT(entry), "variable_type",(void *) variable_type);
        g_object_set_data(G_OBJECT(entry), "variable_subtype",(void *) variable_subtype);
        hbox = rfm_hbox_new (FALSE, 0);
        label = gtk_label_new("");
        gchar *g = g_strdup_printf("<i>%s%s%s</i>", strchr(variable_type, ':')?
                strchr(variable_type, ':')+1: variable_type,
                (variable_subtype==NULL)?"":":",
                (variable_subtype==NULL)?"":
                strchr(variable_subtype, ':')?
                strchr(variable_subtype, ':')+1: variable_subtype);
        gtk_label_set_markup(GTK_LABEL(label), g);
        g_free(g);
        gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 3);
        gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 3);
    }
    hbox = rfm_hbox_new (FALSE, 0);
    GtkWidget *button;
    button = rfm_mk_little_button ("xffm/stock_ok", 
		      (gpointer)ak_apply, entry, _("Apply"));
    gtk_widget_set_can_focus (button, TRUE);
    gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 3);

    if (pattern_item == NULL){
        button = rfm_mk_little_button ("xffm/stock_clear", 
                          (gpointer)ak_erase, entry, _("Clear"));
        gtk_widget_set_can_focus (button, TRUE);
        gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 3);
    }

    button = rfm_mk_little_button ("xffm/stock_cancel", 
		      (gpointer)ak_destroy, entry, _("Cancel"));
    gtk_widget_set_can_focus (button, TRUE);
    gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 3);

    gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 3);
    g_object_set_data(G_OBJECT(vbox), "entry", entry);

     return vbox;
}

#if 0
static void *
string_box(const gchar *head, const gchar *tag, const gchar *value){
    GtkWidget *vbox = rfm_vbox_new (FALSE, 0);
    
    GtkWidget *hbox = rfm_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 3);
    GtkWidget *label = gtk_label_new("");
    gchar *text = g_strdup_printf("<b>%s</b> ", head);
    gtk_label_set_markup(GTK_LABEL(label), text);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 3);

    hbox = rfm_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 3);
    text = g_strdup_printf("<i>%s:</i> ", tag);
    label = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(label), text);
    g_free(text);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 3);

    GtkEntryBuffer *entry_buffer =
	gtk_entry_buffer_new (value, -1);
    GtkWidget *entry =
	gtk_entry_new_with_buffer(entry_buffer);
    gtk_box_pack_start (GTK_BOX (hbox), entry, FALSE, FALSE, 3);

    //g_signal_connect (G_OBJECT (entry), "key-press-event", G_CALLBACK (signal_keyboard_event), NULL);
    //gtk_container_add (GTK_CONTAINER (vbox), hbox);

    hbox = rfm_hbox_new (FALSE, 0);
    GtkWidget *button;
    button = rfm_mk_little_button ("xffm/stock_ok", 
		      (gpointer)ak_apply, entry, _("Apply"));
    gtk_widget_set_can_focus (button, TRUE);
    gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 3);

    button = rfm_mk_little_button ("xffm/stock_edit-clear", 
		      (gpointer)ak_erase, entry, _("Clear"));
    gtk_widget_set_can_focus (button, TRUE);
    gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 3);

    button = rfm_mk_little_button ("xffm/emblem_cancel", 
		      (gpointer)ak_destroy, entry, _("Cancel"));
    gtk_widget_set_can_focus (button, TRUE);
    gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 3);

    gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 3);
    g_object_set_data(G_OBJECT(vbox), "entry", entry);

     return vbox;
}
#endif

static void *
new_dialog (GtkWidget *vbox, const gchar *title){
    GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DIALOG);
    gtk_window_set_title (GTK_WINDOW (window), title);
    gtk_container_add (GTK_CONTAINER (window), vbox);
    g_object_set_data(G_OBJECT(window), "vbox", vbox);
    GtkWidget *entry = g_object_get_data(G_OBJECT(vbox), "entry");
    g_object_set_data(G_OBJECT(entry), "attribute_window", window);
    return (void *)window;
}

/////////////////////////////////////////////////////////////////////////
//  WINDOW CALLBACKS
/////////////////////////////////////////////////////////////////////////
static void
on_destroy_k (GtkButton * button, gpointer user_data) {
    gtk_main_quit ();
}


static void 
button_cell_pressed(GtkTreeView *treeview, GtkTreePath *treepath,  const gchar *column_title){
    NOOP( "activate_f --> \"%s\"\n", column_title);
    //copy content to one row above.
    // Content:
    GtkTreeIter sibling;

    // This is the filter model
    GtkTreeModelFilter *filter_model = g_object_get_data(G_OBJECT(treeview), "filter_model");
    // Currently only set up for xsd treeview of associated xml.
    if (!filter_model) return;
    // This is the actual treestore
    GtkTreeModel *treemodel = gtk_tree_model_filter_get_model(filter_model); 

    GtkTreePath *storepath = gtk_tree_model_filter_convert_path_to_child_path(filter_model, treepath);
    if (!gtk_tree_model_get_iter(treemodel, &sibling, storepath)) {
        fprintf(stderr, "*** button_cell_pressed(): Cannot get iter from treepath.\n");
        return ;
    }
    gtk_tree_path_free(storepath);

    static gboolean lock=FALSE;
    if (lock) return;
    lock = TRUE;
    Tag_item_t *src_tag;
    gint flag;
    gtk_tree_model_get(treemodel, &sibling, 
            H_TAG_ITEM_COLUMN, &src_tag,
            H_FLAG_COLUMN, &flag,
            -1);

    choice_callback(src_tag, treeview, treemodel, &sibling, flag);
    
    lock = FALSE;

    return;
}

static gboolean
activate_f(GtkTreeView *treeview, GtkTreePath *treepath,  const gchar *column_title, GdkEventButton * event){
    gint Xsize=45;
    gint Ysize=45;
    gchar *path = gtk_tree_path_to_string (treepath);
    xmltree_t *xmltree_p = g_object_get_data(G_OBJECT(treeview), "xmltree_p");
    if (column_title && strcmp(column_title, "*")==0) {
        button_cell_pressed(treeview, treepath,  column_title);
        g_free(path);
        return TRUE;
    }
    if (column_title && strcmp(column_title, _("Value"))==0) {
	if (!dialog_hash){
	    dialog_hash  = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
	}
	TRACE( "hash lookup for window: %s\n", path);
	GtkWidget *window = g_hash_table_lookup(dialog_hash, path);
	if (!window) {
	    gchar *key=g_strdup(path);
	    gint tag_type = UNDEFINED_DATA;
	    // Retrieve value in _("XML tag") to see if we have a table.
	    // 1. Get associated treemodel
	    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
	    // 2. Get the iter which corresponds to the treepath
	    GtkTreeIter iter;
	    gtk_tree_model_get_iter (model, &iter, treepath);
	    // Then we retrieve the values in the columns we will need.
	    gchar *tag;
	    gchar *attribute;
	    gchar *value;
	    Tag_item_t *parent_tag;
            Attribute_item_t *attribute_item;
            gint flag;
	    gtk_tree_model_get (model, &iter, 
		H_TAG_COLUMN, &tag, 
		H_ATTRIBUTE_COLUMN, &attribute, 
		H_TAG_ITEM_COLUMN, &parent_tag,
		H_ATTRIBUTE_ITEM_COLUMN, &attribute_item,
		H_VALUE_COLUMN, &value,
                H_FLAG_COLUMN, &flag, 
		-1);
            if (noneditable_hash && g_hash_table_lookup(noneditable_hash, attribute_item)) goto done;
            if (flag & IS_ALL_CTL) goto done;
            if (flag & IS_SEQUENCE_CTL) goto done;
            // FIXME: also, get all parent tags, and if sequence ctl, return
            GtkTreeIter c_iter = iter;
            GtkTreeIter p_iter;
            while (gtk_tree_model_iter_parent(model, &p_iter, &c_iter)){
	        gtk_tree_model_get (model, &p_iter,
                       H_FLAG_COLUMN, &flag,
                      -1);
                if (flag & IS_ALL_CTL) goto done;
                if (flag & IS_SEQUENCE_CTL) goto done;
                c_iter= p_iter;
            } 

	    NOOP("tag=\"%s\", attribute=\"%s\"\n", tag, attribute); 
	    if (tag && !attribute){
		if (strcmp(tag, "TABLE")==0){
		    tag_type = TABLE_DATA;
		} else {
		    tag_type = TAG_DATA;
		}
	    }
	    else if (!tag && attribute){
		tag_type = ATTRIBUTE_DATA;
	    }
	    switch (tag_type){
		case TABLE_DATA:
		    NOOP("table data: %d!\n", tag_type);
		    break;
		case ATTRIBUTE_DATA:;
                    const gchar *element_name = get_tag_name(parent_tag);
                    
		    fprintf(stderr, "attribute data: %d!\n", tag_type);
                    if (attribute_item && 
                            !attribute_get_hidden(attribute_item) &&
                            get_editable_element(xmltree_p, element_name)){
                        const gchar *text = get_attribute_value(attribute_item);
                        if (!text) text = _("Modify");
                        gchar *attribute_title = g_strdup_printf("%s", _("<Modify value>")); 
                        GtkWidget *box = tag_box(text, attribute, value, 2, parent_tag);
                        if (box) {
                            window = new_dialog(box, attribute_title);
                            g_object_set_data(G_OBJECT(window), "filter_model", g_object_get_data(G_OBJECT(treeview), "filter_model"));
                            g_object_set_data(G_OBJECT(window), "model", gtk_tree_view_get_model(treeview));
                            g_object_set_data(G_OBJECT(window), "xmltree_p", xmltree_p);
                        }
                        g_free(attribute_title);
                   } else if (!get_editable_attribute(xmltree_p, attribute)){
			g_free(path); 
			return FALSE;
		    } else {
                        gint type = GPOINTER_TO_INT(g_hash_table_lookup(xmltree_p->attribute_hash, attribute));
                        Attribute_item_t *att_text = get_attribute(parent_tag, "text");
                        const gchar *text = get_attribute_value(att_text);
                        if (!text) text = _("Modify");
                        gchar *attribute_title = NULL; 
                        GtkWidget *box = NULL;
                        if (type == XMLTREE_key_type){
                            attribute_title = g_strdup_printf("%s", _("<choose a key>")); 
                            box = tag_box(text, attribute, value, 1, parent_tag);
                        } else if (type == XMLTREE_string_type){
                            attribute_title = g_strdup_printf("%s", _("<Modify value>")); 
                            box = tag_box(text, attribute, value, 2, parent_tag);
                        }
                        if (box) {
                            window = new_dialog(box, attribute_title);
                            if (type == XMLTREE_key_type){
                                g_object_set_data(G_OBJECT(window), "XMLTREE_key_type", GINT_TO_POINTER(1));
                            }
                            g_object_set_data(G_OBJECT(window), "filter_model", g_object_get_data(G_OBJECT(treeview), "filter_model"));
                            g_object_set_data(G_OBJECT(window), "model", gtk_tree_view_get_model(treeview));
                            g_object_set_data(G_OBJECT(window), "xmltree_p", xmltree_p);
                        }
                        g_free(attribute_title);
                    }


		    

		    break;
		case TAG_DATA:
    //        click on text will fire up modification of key attribute. OK
		    fprintf(stderr,"tag data: %d!\n", tag_type);
		    gint action = xmltree_p->text_activates_top_attribute;
		    fprintf(stderr, "action=%d\n", action);
		    if (action == -1) {
			// FIXME: here we open dialog with textview to modify text
			//        if text is modifyable... (action == -1)
		    } else if (action==1) {
		      if (gtk_tree_model_iter_has_child(model, &iter)){
			GtkTreePath *ipath=NULL;
			GtkTreeIter child;	
			gtk_tree_model_iter_children (model, &child, &iter);
                        do {
                            gchar * attribute;
                            gtk_tree_model_get (model, &child, 
                                    H_ATTRIBUTE_COLUMN, &attribute, 
                                    -1);
                            NOOP(stderr, "got attribute \"%s\"\n", attribute);
                            if (get_editable_attribute(xmltree_p, attribute)){
                                NOOP(stderr, "gotcha \"%s\"\n", attribute);
                                ipath = gtk_tree_model_get_path(model, &child);
                                GdkEventButton newevent;
                                newevent.x = event->x;
                                newevent.y = event->y;
                                activate_f(treeview, ipath,  _("Value"), &newevent);
                                gtk_tree_path_free(ipath);
                                break;
                            } 
                        } while (gtk_tree_model_iter_next(model, &child));
                      
		      }
		    }
		    break;

		default:
		    g_free(path); 
		    return FALSE;
	    }
	    if (key) {
		gchar *path_string = g_strdup(key);
		g_object_set_data(G_OBJECT(window), "path_string", path_string);
	    }

	    g_hash_table_replace(dialog_hash, key, window);
	    TRACE( "inserting %s -- 0x%x\n", key, GPOINTER_TO_INT(window));
	    g_signal_connect(G_OBJECT(window), "delete-event", 
		    G_CALLBACK (on_destroy_child), NULL);
	    g_signal_connect(G_OBJECT(window), "destroy-event", 
		    G_CALLBACK (on_destroy_child), NULL);
	    // freed with g_hash_table_destroy ... g_free(key);

	    NOOP("size--> %d, %d\n", Xsize, Ysize);
	    gtk_window_set_default_size (GTK_WINDOW (window), Xsize, Ysize);
	    gtk_widget_show_all(window);
	}
	// Move window... 
	if (event){
	    gint root_x;
	    gint root_y;
	    gint x=event->x;
	    gint y=event->y;
	    GtkWidget *parent_window = g_object_get_data(G_OBJECT(treeview), "parent_window");
	    gtk_window_get_position (GTK_WINDOW(parent_window), &root_x, &root_y);
	    NOOP("moving %s to %d, %d\n", path, x, y);
	    gtk_window_move (GTK_WINDOW(window), root_x+x, root_y+y);
	}
	// raise as well...
	gdk_window_raise (gtk_widget_get_window(window));
    }
done:
    g_free(path); 
    return TRUE;
}

static gboolean
on_button_press (GtkWidget * widget, GdkEventButton * event, gpointer data) {
    set_button_serial (GTK_TREE_VIEW(widget), event);
    return FALSE;
}    

static gboolean
on_button_release (GtkWidget * widget, GdkEventButton * event, gpointer data) 
{  
    if (!check_button_serial(GTK_TREE_VIEW(widget), event)) return FALSE;
    unset_button_serial (GTK_TREE_VIEW(widget), event);


    GtkTreeView *treeview = GTK_TREE_VIEW(widget);
    GtkTreePath *treepath=NULL;
    GtkTreeViewColumn *column;
    gtk_tree_view_get_path_at_pos(treeview, event->x, event->y, 
		    &treepath, &column, NULL, NULL);

    const gchar *column_title="none";
    if (column){
	column_title = gtk_tree_view_column_get_title (column);
    }

    NOOP ("treepath=\"%s\" column=\"%s\"\n", path, column_title);
    gboolean retval = activate_f(treeview, treepath,  column_title, event);
    if (strcmp(column_title, _("Value"))) retval = FALSE;

    gtk_tree_path_free(treepath);
    return retval;
}

// To enter edit mode via keyboard:
static
gboolean treeview_key(GtkWidget *w, GdkEventKey *event,gpointer data){
    //NOOP(stderr, "0x%x\n",event->keyval); 
    if (event->keyval != GDK_KEY_Return 
	    && event->keyval != GDK_KEY_KP_Enter
	    && event->keyval != GDK_KEY_ISO_Enter
	    && event->keyval != GDK_KEY_3270_Enter){
	return FALSE;
    }
    GtkTreeView *treeview=(GtkTreeView *)w;
    xmltree_t *xmltree_p = g_object_get_data(G_OBJECT(treeview), "xmltree_p");
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(treeview);
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_BROWSE);
    GtkTreePath *ipath=NULL;
    GtkTreeIter iter;
	

    if(gtk_tree_selection_get_selected(selection, &model, &iter)){
	gchar *tag;
	gtk_tree_model_get(model, &iter, H_TAG_COLUMN, &tag, -1);
	if (tag && strcasecmp(tag, "keys")==0) {
	    ipath = gtk_tree_model_get_path(model, &iter);
	    if (gtk_tree_view_row_expanded (treeview,ipath)){
		 gtk_tree_view_collapse_row(treeview, ipath);
	    } else {
		 gtk_tree_view_expand_row(treeview, ipath, FALSE);
	    }
	    if (ipath) gtk_tree_path_free(ipath);
	    return FALSE;
	}
    }
    GtkTreeIter *att_iter;
    GtkTreeIter child;
    if (gtk_tree_model_iter_has_child(model, &iter)){
	gtk_tree_model_iter_children (model, &child, &iter);
	att_iter = &child;
    } else {
	att_iter = &iter;
    }
    gchar * attribute;
    gtk_tree_model_get (model, att_iter, 
		H_ATTRIBUTE_COLUMN, &attribute, 
		-1);
    if (!attribute) goto done;
    // FIXME: any editable attribute.
    if (get_editable_attribute(xmltree_p, attribute)){
    //if (strcasecmp(attribute, "Keybinding")==0) {
	ipath = gtk_tree_model_get_path(model, att_iter);
	GdkEventButton newevent;
	gint x;
	gint y;
	gint wx;
	gint wy;
	GtkWindow *window = g_object_get_data(G_OBJECT(treeview), "parent_window");
	gtk_window_get_position (window, &wx, &wy);
#if GTK_MAJOR_VERSION==3
        rfm_global_t *rfm_global_p = rfm_global();
	gdk_device_get_position (rfm_global_p->pointer, NULL, &(x), &(y));
#else 
	gdk_display_get_pointer (gdk_display_get_default(), NULL, &(x), &(y), NULL);
#endif
	x -= wx;
	y -= wy;
	newevent.x = x;
	newevent.y = y;
	activate_f(treeview, ipath,  _("Value"), &newevent);
    }
done:
    if (ipath) gtk_tree_path_free(ipath);
    return FALSE;
}

static void
xml_edit_destroy (GtkButton * button, gpointer data) {
    GtkWidget *window = data;
    Tag_t *Tag_p = g_object_get_data(G_OBJECT(window), "Tag_p");
    tag_free (Tag_p);
    gtk_widget_destroy(window);
    gtk_main_quit();

}
static void
xml_edit_save (GtkButton * button, gpointer data) {
    GtkWidget *window = data;
    gboolean clean = TRUE;
    GtkTreeModel *model = g_object_get_data(G_OBJECT(window), "model");
    gtk_tree_model_foreach(model, check_clean, &clean);
    if (!clean){
	return;
    } 
    // save xml.
    Tag_t *Tag_p = g_object_get_data(G_OBJECT(window), "Tag_p");
    xmltree_t *xmltree_p = g_object_get_data(G_OBJECT(window), "xmltree_p");
    // FIXME: enter as parameter OK
    //gchar *keybindings_file = g_build_filename(KEYBINDINGS_FILE, NULL);
    gchar *file = g_build_filename(xmltree_p->xml_path, NULL);
    tag_write_file(Tag_p, file, FALSE);
    rfm_confirm(NULL, GTK_MESSAGE_INFO, file, NULL, NULL);
    g_free(file);
    gtk_widget_destroy(window);

}

/////////////////////////////////////////////////////////////////////////
//    TREEVIEW CREATION FUNCTIONALITY
/////////////////////////////////////////////////////////////////////////

static void *
build_treeview(GtkTreeModel *model){
    GtkWidget *treeview = gtk_tree_view_new_with_model (model);
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;

    // This is the nerdy stuff: 
    //   adding the columns with the appropriate cell renderer...
    //
    //   button column
    column = gtk_tree_view_column_new ();    
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_column_set_resizable (column, FALSE);
    gtk_tree_view_column_set_reorderable (column, FALSE);
    gtk_tree_view_column_set_spacing (column, 2);

    cell = gtk_cell_renderer_pixbuf_new ();
    gtk_tree_view_column_pack_start (column, cell, FALSE);
    gtk_tree_view_column_set_attributes (column, cell,
                                         "pixbuf", H_BUTTON_COLUMN,
					 NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
    gtk_tree_view_column_set_title (column, "*");

    // Start with the pixbuf column...
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_column_set_resizable (column, FALSE);
    gtk_tree_view_column_set_reorderable (column, FALSE);
    gtk_tree_view_column_set_spacing (column, 2);

    cell = gtk_cell_renderer_pixbuf_new ();
    gtk_tree_view_column_pack_start (column, cell, FALSE);
    gtk_tree_view_column_set_attributes (column, cell,
                                         "pixbuf", H_PIXBUF_COLUMN,
                                         "pixbuf_expander_closed", H_PIXBUF_COLUMN, 
					 "pixbuf_expander_open", H_PIXBUF_COLUMN, 
					 NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
    gtk_tree_view_set_expander_column (GTK_TREE_VIEW(treeview), column);


    // Start with the tag column...
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_column_set_resizable (column, FALSE);
    gtk_tree_view_column_set_reorderable (column, FALSE);
    gtk_tree_view_column_set_spacing (column, 2);
    cell = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (cell), "editable", FALSE, NULL);
    gtk_tree_view_column_set_clickable (column, TRUE);

    gtk_tree_view_column_pack_start (column, cell, FALSE);
    gtk_tree_view_column_set_attributes (column, cell, "text", H_TAG_COLUMN, 					 "foreground", H_COLOR_COLUMN,
					 NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
    gtk_tree_view_column_set_title (column, _("XML tag"));

    // Continue with the namespace (prefix) column...
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_column_set_resizable (column, FALSE);
    gtk_tree_view_column_set_reorderable (column, FALSE);
    gtk_tree_view_column_set_spacing (column, 2);
    cell = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (cell), "editable", FALSE, NULL);
    gtk_tree_view_column_set_clickable (column, TRUE);

    gtk_tree_view_column_pack_start (column, cell, FALSE);
    gtk_tree_view_column_set_attributes (column, cell, "text", H_NS_COLUMN, 					 "foreground", H_COLOR_COLUMN,
					 NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
    gtk_tree_view_column_set_title (column, _("prefix"));


    // Next is the attributes column...
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_column_set_resizable (column, FALSE);
    gtk_tree_view_column_set_reorderable (column, FALSE);
    gtk_tree_view_column_set_spacing (column, 2);
    cell = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (cell), "editable", FALSE, NULL);
    gtk_tree_view_column_set_clickable (column, TRUE);

    gtk_tree_view_column_pack_start (column, cell, FALSE);
    gtk_tree_view_column_set_attributes (column, cell, "text", H_ATTRIBUTE_COLUMN, 					 "foreground", H_COLOR_COLUMN,
					 NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
    gtk_tree_view_column_set_title (column, _("Attribute"));
   

    // Finally (for now) is the value (string) column...
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_column_set_resizable (column, FALSE);
    gtk_tree_view_column_set_reorderable (column, FALSE);
    gtk_tree_view_column_set_spacing (column, 2);
    cell = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (cell), "editable", FALSE, NULL);
    gtk_tree_view_column_set_clickable (column, TRUE);

    gtk_tree_view_column_pack_start (column, cell, FALSE);
    gtk_tree_view_column_set_attributes (column, cell, "text", H_VALUE_COLUMN,					 "foreground", H_COLOR_COLUMN,
					 NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
    gtk_tree_view_column_set_title (column, _("Value"));
    // Signal connections
    // "button-press" and "button-release" are combined to create a 
    // "clicked" signal for the widget.
    g_signal_connect(G_OBJECT(treeview), "button-press-event",
	    G_CALLBACK (on_button_press), NULL);
    g_signal_connect(G_OBJECT(treeview), "button-release-event", 
	    G_CALLBACK (on_button_release), NULL);
    return treeview;
}

static void *
build_treeview_box(GtkTreeView *treeview){
    GtkWidget *vbox = rfm_vbox_new (TRUE, 0);
    GtkWidget *scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_container_add (GTK_CONTAINER (vbox), scrolledwindow);
    gtk_container_add (GTK_CONTAINER (scrolledwindow), GTK_WIDGET(treeview));
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(treeview), TRUE);
    return vbox;
}


static void
recurse_tree(Tag_t *Tag_p, Tag_item_t *item, GtkTreeModel *tree_model, gint level, GtkTreeIter *parent_p){
    GtkTreeStore *model = GTK_TREE_STORE(tree_model);
    xmltree_t *xmltree_p = g_object_get_data(G_OBJECT(tree_model), "xmltree_p");

    if (!GDK_IS_PIXBUF(OK) || !GDK_IS_PIXBUF(KO)){
	broken = rfm_get_pixbuf("xffm/emblem_broken", TINY_ICON_SIZE);
	OK = rfm_get_pixbuf("xffm/stock_yes", TINY_ICON_SIZE);
	KO = rfm_get_pixbuf("xffm/stock_no", TINY_ICON_SIZE);
	keyboard = rfm_get_pixbuf("xffm/emblem_keyboard", TINY_ICON_SIZE);
	folder_red = rfm_get_pixbuf("xffm/stock_directory/compositeC/emblem_redball", TINY_ICON_SIZE);
	folder_green = rfm_get_pixbuf("xffm/stock_directory/compositeC/emblem_greenball", TINY_ICON_SIZE);
        redball = rfm_get_pixbuf("xffm/emblem_redball", 16);
        greenball = rfm_get_pixbuf("xffm/emblem_greenball", 16);
	green = rfm_get_pixbuf("xffm/emblem_edit/compositeNE/emblem_greenball", TINY_ICON_SIZE);
        red = rfm_get_pixbuf("xffm/emblem_edit/compositeNE/emblem_redball", TINY_ICON_SIZE);
	blue = rfm_get_pixbuf("xffm/emblem_edit", TINY_ICON_SIZE);
	bold = rfm_get_pixbuf("xffm/stock_bold", TINY_ICON_SIZE);
	list_remove = rfm_get_pixbuf("xffm/stock_list-remove", 16);
	list_add = rfm_get_pixbuf("xffm/stock_list-add", 16);
	question = rfm_get_pixbuf("xffm/stock_dialog-question", 16);
	index_pix = rfm_get_pixbuf("xffm/stock_index", 16);
	index_pix2 = rfm_get_pixbuf("xffm/emblem_file", 16);
	strikethrough = rfm_get_pixbuf("xffm/stock_strikethrough", TINY_ICON_SIZE);
	include_on = rfm_get_pixbuf("xffm/stock_go-bottom", TINY_ICON_SIZE);
	include_off = rfm_get_pixbuf("xffm/stock_go-top", TINY_ICON_SIZE);
	repeat_value = rfm_get_pixbuf("xffm/stock_stop", TINY_ICON_SIZE);
	// references are in hash table
	g_object_unref(broken);
	g_object_unref(OK);
	g_object_unref(KO);
	g_object_unref(keyboard);
	g_object_unref(folder_red);
	g_object_unref(folder_green);
	g_object_unref(bold);
	g_object_unref(redball);
	g_object_unref(greenball);
	g_object_unref(red);
	g_object_unref(green);
	g_object_unref(blue);
	g_object_unref(strikethrough);
	g_object_unref(include_on);
	g_object_unref(include_off);
	g_object_unref(repeat_value);
    }

    GSList *list;
    GtkTreeIter child;
    GtkTreeIter grandchild;
    NOOP("item=0x%x\n", GPOINTER_TO_INT(item));


    GSList *the_list = get_tag_item_list(Tag_p, item, NULL); //  all tags 
    for (list=the_list;list && list->data; list=list->next){
	Tag_item_t * item = list->data;
	const gchar *name = get_tag_name(item);
	const gchar *string = tag_item_get_string(item);
        if (!name)continue;
	if (strcasecmp(name, "schema")==0){
	    NOOP("schema_tag\n");
	}
	// Add name to model to create a new treeiter.
	// strike through icon for tags with no attributes (default)
	gtk_tree_store_append (model, &child, parent_p);
	GdkPixbuf *pixbuf = strikethrough;
	gtk_tree_store_set (model, &child, 
		H_PIXBUF_COLUMN, pixbuf,
		H_TAG_COLUMN, (string)?string:name, 
		H_TAG_ITEM_COLUMN, item, 
		H_ATTRIBUTE_ITEM_COLUMN, NULL, // tags are not attributes.
                H_VALUE_COLUMN, NULL,
		-1);
	// Does the node have properties? (or attributes)
	GSList *attribute_list = get_attribute_item_list(item);
	GSList *tmp = attribute_list;
	for(;tmp && tmp->data; tmp=tmp->next){
	    if (strcasecmp(name, "schema")==0){
		NOOP("schema_tag attribute...\n");
	    }
	    Attribute_item_t *at_item = tmp->data;
	    const gchar *value = get_attribute_value(at_item);
	    if (value) {
		Attribute_item_t * at_item = tmp->data;
                const gchar *at_name=get_attribute_name(at_item);
                if (!at_name){
                    DBG("recurse_tree(): at_name=NULL\n");
                    continue;
                }
		const gchar *name_field = g_hash_table_lookup(xmltree_p->echo_hash, name);
                if (!name_field) name_field = "text";
		if (strcasecmp(at_name, "Keybinding")==0) pixbuf = OK;
		else pixbuf = NULL;
		// Put the text field into the parent tag's value column
		if (strcasecmp(at_name, name_field)==0){
		    gtk_tree_store_set (model, &child, 
			H_VALUE_COLUMN, get_attribute_value(at_item), 
			-1);
		} 
                //else  
                {
                    if (attribute_get_hidden(at_item)) gtk_tree_store_append (model, &grandchild, &child);
                    else if (noneditable_hash && g_hash_table_lookup(noneditable_hash, at_item))
                        gtk_tree_store_append (model, &grandchild, &child);
                    else gtk_tree_store_prepend (model, &grandchild, &child);
                    gtk_tree_store_set (model, &grandchild, 
                        H_NS_COLUMN, get_attribute_prefix(at_item), 
                        H_ATTRIBUTE_COLUMN, get_attribute_name(at_item), 
                        H_PIXBUF_COLUMN, pixbuf,
                        H_VALUE_COLUMN, get_attribute_value(at_item), 
                        H_TAG_ITEM_COLUMN, item, // This will correspond to the parent tag
                        H_ATTRIBUTE_ITEM_COLUMN, at_item, 
                        -1);
                }

	    } else {
		gtk_tree_store_set (model, &grandchild, 
		    H_ATTRIBUTE_COLUMN, get_attribute_name(at_item), 
		    H_PIXBUF_COLUMN, KO,
		    H_TAG_ITEM_COLUMN, item, // This will correspond to the parent tag
		    H_ATTRIBUTE_ITEM_COLUMN, at_item, 
		    -1);
	    }
	}
	if (attribute_list) {
	    // Change tag icon to keyboard
	    g_slist_free(attribute_list);
	    pixbuf = keyboard;
	    if (keyboard) g_object_ref(keyboard);
	    Attribute_item_t *att = get_attribute(item, "icon_id");
	    if (att){
		const gchar *icon_id = get_attribute_value(att);
		if (icon_id){
		    if (strlen(icon_id)){
			if (pixbuf) g_object_unref(pixbuf);
			pixbuf = rfm_get_pixbuf(icon_id, TINY_ICON_SIZE);
		    } 
		} 
	    } 
	    gtk_tree_store_set (model, &child, 
		H_PIXBUF_COLUMN, pixbuf,
		-1);
	    if (pixbuf) g_object_unref(pixbuf);
	}
	// recurse:	
	// Does the node have children?
	if (tag_item_has_children(item)){
	    pixbuf = keyboard;
	    gtk_tree_store_set (model, &child, 
		    H_PIXBUF_COLUMN, pixbuf, 
		    -1);
	    recurse_tree(Tag_p, item, tree_model, level+1, &child);
	}
    }
    g_slist_free(the_list);
    return;
}

static GtkTreeModel *
populate_tree_model_from_tag (Tag_t *Tag_p, GtkTreeModel *model, GError **error){
    if (!Tag_p){
	g_error("build_treemodel(): Tag_p cannot be NULL!");
    }
    
    recurse_tree(Tag_p, NULL, model, 0, NULL);
   
    return model;
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

#ifdef COPYRIGHT_INFORMATION
#include "gplv3.h"
#endif
/*
 * Copyright (C) 2002-2012 Edscott Wilson Garcia
 * EMail: edscott@users.sf.net
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; 
 */
//////////////////////////////////////////////////////////////////////////
#include "tagfile.h"
//////////////////////////////////////////////////////////////////////////

static GHashTable *all_hash=NULL;
static GHashTable *sequence_hash=NULL;
static GHashTable *choice_hash=NULL;
static GHashTable *complexType_hash = NULL;
static GHashTable *simpleType_hash = NULL;
static GHashTable *unset_hash = NULL;

static gboolean is_ctl_full(Tag_t *Tag_p, Tag_item_t *src_tag, GHashTable *hash);
/*
static gboolean
is_nested_sequence(Tag_item_t *tag){
    Tag_item_t *pt = get_parent_tag(tag);
    while (pt){
        if (g_hash_table_lookup(sequence_hash, pt)){
            return TRUE;
        }
        pt = get_parent_tag(pt);
    }
    return FALSE;
}

static gboolean
is_nested_choice(Tag_item_t *tag){
    Tag_item_t *pt = get_parent_tag(tag);
    while (pt){
        if (g_hash_table_lookup(choice_hash, pt)){
            return TRUE;
        }
        pt = get_parent_tag(pt);
    }
    return FALSE;
}
*/

static gboolean
is_sequence_ctl(Tag_item_t *tag){
    if (sequence_hash && g_hash_table_lookup(sequence_hash, tag)) return TRUE;
    return FALSE;
}
static gboolean
is_all_ctl(Tag_item_t *tag){
    if (all_hash && g_hash_table_lookup(all_hash, tag)) return TRUE;
    return FALSE;
}
static gboolean
is_choice_ctl(Tag_item_t *tag){
    if (choice_hash && g_hash_table_lookup(choice_hash, tag)) return TRUE;
    return FALSE;
}

static gboolean is_ctl(Tag_item_t *pt){
    if (sequence_hash) {
        if (g_hash_table_lookup(sequence_hash, pt)){
            return TRUE;
        }
    }
    if (all_hash) {
        if (g_hash_table_lookup(all_hash, pt)){
            return TRUE;
        }
    }    
    if (choice_hash) {
        if (g_hash_table_lookup(choice_hash, pt)){
            return TRUE;
        }
    }    
    return FALSE;
}

static gboolean
is_nested_in_ctl(Tag_item_t *tag){
    Tag_item_t *pt = get_parent_tag(tag);
    while (pt){
        if (is_ctl(pt)) return TRUE;
        pt = get_parent_tag(pt);
    }
    return FALSE;
}
/*   
static gboolean
is_attribute_required(GtkTreeModel *treemodel, GtkTreeIter *iter){
    GtkTreeIter parent;
    if (gtk_tree_model_iter_parent(treemodel, &parent, iter)){
        GtkTreeIter child;
        if (gtk_tree_model_iter_children(treemodel, &child, &parent)){
            Attribute_item_t *at = NULL;
            gchar *t = g_strdup_printf("%s:use", attribute);
            do {
                gtk_tree_model_get(treemodel, &child, 
                    H_ATTRIBUTE_ITEM_COLUMN, &at, -1);
                if (at){
                  const gchar *g = get_attribute_name(at);
                  const gchar *v = get_attribute_value(at);
                  NOOP( "%s <--> %s>\n", g, t);
                  if (g && strcmp(g,t)==0 && strcmp(v,"required")==0){
                    g_free(t);
                    return TRUE;
                  }
                }
            } while (gtk_tree_model_iter_next(treemodel, &child));
            g_free(t);
        }
    }
    return FALSE;
}*/
   
static gboolean
is_attribute_required(Attribute_item_t *at_item){
    // "text" attribute is always required. 
    const gchar *n = get_attribute_name(at_item);
    if (n && strcasecmp(n, "text")==0) return TRUE;

    GSList *list = get_attribute_item_list(get_attribute_parent(at_item));
    GSList *l = list;

    for (;l && l->data; l = l->next){
          Attribute_item_t *at = l->data;
          gchar *t = g_strdup_printf("%s:use", get_attribute_name(at_item));
          const gchar *g = get_attribute_name(at);
          const gchar *v = get_attribute_value(at);
          //fprintf(stderr, "%s <--> %s>\n", g, t);
          if (g && strcasecmp(g,t)==0 && strcasecmp(v,"required")==0){
            g_free(t);
            g_slist_free(list);
            return TRUE;
          }
          g_free(t);
    }
    g_slist_free(list);
    return FALSE;
}

/*
static gboolean
is_attribute_row_clean(GtkTreeModel *treemodel, GtkTreeIter *iter){
    const gchar *attribute;
    Attribute_item_t *at;
    gtk_tree_model_get(treemodel, iter, 
                H_ATTRIBUTE_COLUMN, &attribute, 
                H_ATTRIBUTE_ITEM_COLUMN, &at, -1);
    const gchar *v = get_attribute_value(at);
    if (v && strlen(v)) return TRUE;
    if (is_attribute_required(treemodel, iter)) return FALSE;
    return TRUE;
}*/


static gboolean
is_attribute_row_clean(Tag_t *Tag_p, Tag_item_t *tag){
    GSList *list = get_attribute_item_list(tag);
    GSList *l = list;
    fprintf(stderr, "%s attribute list = %p\n", get_tag_name(tag), l);
    for (;l && l->data; l = l->next){
        Attribute_item_t *at_item = l->data;
        if (attribute_get_hidden(at_item)) continue;
        const gchar *n = get_attribute_name(at_item);
        fprintf(stderr, "   attribute %s\n", n);
        // FIXME, attribute with no name nor value. Should not happen, but does...
        if (!n || !strlen(n)) continue;
        const gchar *v = get_attribute_value(at_item);
            fprintf(stderr, "CHECK attribute %s ...\n", n);
        if (v && strlen(v)) {
            fprintf(stderr, "value \"%s\" is OK\n", v);
            continue;
        }
        if (is_attribute_required(at_item)){
            g_slist_free(list);
            fprintf(stderr, "attribute %s is required\n", n);
            return FALSE;
        } else fprintf(stderr, "attribute %s is not required\n", n);
    }
    g_slist_free(list);
    return TRUE;
}


static Tag_item_t *
set_attribute_colorXSD(GtkTreeModel *treemodel, GtkTreeIter *iter){
	gchar *attribute=NULL;
	Tag_item_t *tag;
	Tag_item_t *parent_tag;
        Attribute_item_t *item = NULL;
        xmltree_t *xmltree_p = g_object_get_data(G_OBJECT(treemodel), "xmltree_p");
	gtk_tree_model_get(treemodel, iter, 
		H_TAG_ITEM_COLUMN, &tag,
                H_ATTRIBUTE_COLUMN, &attribute, 
                H_ATTRIBUTE_ITEM_COLUMN, &item, -1);

        if (!item) parent_tag = get_parent_tag(tag);
        else parent_tag=get_attribute_parent(item);
        
        if (is_attribute_required(item)){
            gtk_tree_store_set((GtkTreeStore *) treemodel, iter, H_COLOR_COLUMN, REQUIRED_COLOR, -1);
        }

        if (item){
           if(attribute_get_hidden(item)){
	        gtk_tree_store_set((GtkTreeStore *) treemodel, iter, H_COLOR_COLUMN, HIDDEN_COLOR, -1);
           }
           else if (noneditable_hash && g_hash_table_lookup(noneditable_hash, item)){
	        gtk_tree_store_set((GtkTreeStore *) treemodel, iter, H_COLOR_COLUMN, READONLY_COLOR, -1);
	   } else if (get_editable_element(xmltree_p, get_tag_name(parent_tag))){
	        gtk_tree_store_set((GtkTreeStore *) treemodel, iter, H_COLOR_COLUMN, EDIT_COLOR, -1);
           } 
        } 
        if (tag && tag_get_hidden(tag)){
	        gtk_tree_store_set((GtkTreeStore *) treemodel, iter, H_COLOR_COLUMN, HIDDEN_COLOR, -1);
        }
        return tag;

}


static GdkPixbuf *
get_attribute_pixbuf(Attribute_item_t *row_attribute){
    const gchar *value = get_attribute_value(row_attribute);
    // hidden? (NULL)
    // optional? (blue)
    // unset? (red)
    // invalid? (broken)
    // all passed (green)
     
    // hidden:
    if (attribute_get_hidden(row_attribute)) return NULL;
    if (noneditable_hash && g_hash_table_lookup(noneditable_hash, row_attribute)) return NULL;
        
    // optional: (if required/fixed not specified, then assume optional)
    Tag_item_t *p_tag = get_attribute_parent(row_attribute);
    gchar *a = g_strdup_printf("%s:use", get_attribute_name(row_attribute));
    Attribute_item_t *use_at = get_attribute(p_tag, a);
    g_free(a);
    if (use_at){
        const gchar *ca = get_attribute_value(use_at);
        if (strcasecmp(ca, "optional")==0){
           return blue;
        }
    }

    if (!unset_hash) unset_hash = g_hash_table_new(g_direct_hash, g_direct_equal);
    // unset? 
    if (!value || !strlen(value)) {
        g_hash_table_replace(unset_hash, row_attribute, row_attribute);
        return red;
    }
    // invalid: FIXME
    //else if (invalid) return broken;
    NOOP( "%s-->\"%s\" OK\n", get_attribute_name(row_attribute), value);
    g_hash_table_replace(unset_hash, row_attribute, NULL);

    return green;
}

static gboolean
element_has_unset_attributes(Tag_t *Tag_p, Tag_item_t *tag){
    // Some tag types don't count
    // sequence/choice controllers.
    if (choice_hash && g_hash_table_lookup(choice_hash, tag)){
        // If already full, then it does not count
        if (is_ctl_full(Tag_p, tag, choice_hash)) return FALSE;
        // Otherwise, counts
        return TRUE;
    }

    // FIXME: must test each item in sequence/all
    if (sequence_hash && g_hash_table_lookup(sequence_hash, tag)){
        // If already full, then it does not count
        // Otherwise, does not count either.
        //return FALSE;
    }
    if (all_hash && g_hash_table_lookup(all_hash, tag)){
        // If already full, then it does not count
        // Otherwise, does not count either.
        //return FALSE;
    } 


    // Are all attributes correct?
    if (!is_attribute_row_clean(Tag_p, tag)) return TRUE;
        fprintf(stderr, "Attribute row clean for %s now checking elements...\n", get_tag_name(tag));

    GSList *list = get_tag_item_list(Tag_p, tag, NULL);
    GSList *l = list;
    for (;l && l->data; l = l->next){
        Tag_item_t *t = l->data;
        if (choice_hash && g_hash_table_lookup(choice_hash, t))continue;
        if (sequence_hash && g_hash_table_lookup(sequence_hash, t))continue;
        if (all_hash && g_hash_table_lookup(all_hash, t))continue;

        if (element_has_unset_attributes(Tag_p, t)){
            g_slist_free(list);
            return TRUE;
        }
    }
    g_slist_free(list);
    return FALSE;
/*
    // (check for red pixbuf)
    GSList *list = get_attribute_item_list(tag);
    //if (!list) return FALSE; // should not happen...
    GSList *l = list;
    for (;l && l->data; l = l->next){
        Attribute_item_t *a = l->data;
        if (unset_hash && g_hash_table_lookup(unset_hash, a)){
            NOOP( "unset attribute: %s --> %s\n", get_tag_name(tag), get_attribute_name(a)); 
            g_slist_free(list);
            return TRUE;
        }
    }
    g_slist_free(list);
    // recurse

    list = get_tag_item_list(Tag_p, tag, NULL);
    for (l=list; l && l->data; l=l->next){
        Tag_item_t *t = l->data;
        // recurse here
        gboolean r_value = element_has_unset_attributes(Tag_p, t);
        if (r_value){
            g_slist_free(list);
            return TRUE;
        }
    }
    g_slist_free(list);
    return FALSE;*/
}

static GdkPixbuf *
get_element_pixbuf(Tag_t *Tag_p, Tag_item_t *tag){
    // Mark "all" controllers
    if (all_hash && g_hash_table_lookup(all_hash, tag)){
        //fprintf(stderr, "%s --> index_pix2\n", get_tag_name(tag));
        return index_pix2;
    }
    // Mark "sequence" controllers
    if (sequence_hash && g_hash_table_lookup(sequence_hash, tag)){
        //fprintf(stderr, "%s --> index_pix\n", get_tag_name(tag));
        return index_pix;
    }
    // Mark "choice" controllers
    if (choice_hash && g_hash_table_lookup(choice_hash, tag)){
        return question;
    }
    fprintf(stderr, "getting pixbuf for %s\n", get_tag_name(tag));

    GdkPixbuf *red, *green;
    GSList *list = get_tag_item_list(Tag_p, tag, NULL);
    if (list){
        g_slist_free(list);
        red = folder_red;
        green = folder_green;
    } else {
        red = redball;
        green = greenball; 
    } 

    // Mark "element" containers
    if (element_has_unset_attributes(Tag_p, tag)){
        return red;
    }
    return green;

    // unset sequence/all/choice-->return folder_red
    

}


static
void
set_row_iconXSD(	GtkTreeModel * treemodel, 
			GtkTreeIter * iter, 
			Tag_item_t *tag)
{

    Attribute_item_t *row_attribute = NULL;
    gtk_tree_model_get(treemodel, iter,
            H_ATTRIBUTE_ITEM_COLUMN, &row_attribute,
            -1);
    if (row_attribute){
        GdkPixbuf *pixbuf = get_attribute_pixbuf(row_attribute);
        gtk_tree_store_set((GtkTreeStore *) treemodel, iter,
                    H_PIXBUF_COLUMN, pixbuf, 
                        -1);
        return ;
    }
    if (!tag) return;
    Tag_t *Tag_p = g_object_get_data(G_OBJECT(treemodel), "Tag_p");
    GdkPixbuf *pixbuf = get_element_pixbuf(Tag_p, tag);
    gtk_tree_store_set((GtkTreeStore *) treemodel, iter, 
                H_PIXBUF_COLUMN, pixbuf, 
                -1);
#if 0        
    // Mark all controllers
    if (all_hash && g_hash_table_lookup(all_hash, tag)){
        gint flag = IS_ALL_CTL;
        gtk_tree_store_set((GtkTreeStore *) treemodel, iter, 
                    H_BUTTON_COLUMN, list_add, 
                    H_FLAG_COLUMN, flag,
                    -1);
    }
        
    // Mark sequence controllers
    if (sequence_hash && g_hash_table_lookup(sequence_hash, tag)){
        gint flag = IS_SEQUENCE_CTL;
        gtk_tree_store_set((GtkTreeStore *) treemodel, iter, 
                    H_BUTTON_COLUMN, list_add, 
                    H_FLAG_COLUMN, flag,
                    -1);
    }
    // Mark choice controllers
    if (choice_hash && g_hash_table_lookup(choice_hash, tag)){
        gint flag = IS_CHOICE_CTL;

        gtk_tree_store_set((GtkTreeStore *) treemodel, iter, 
                    H_BUTTON_COLUMN, list_add, 
                    H_FLAG_COLUMN, flag,
                    -1);
    }
#endif

    return ;
}


static
gboolean 
switch_colorsXSD(		GtkTreeModel * treemodel, 
			GtkTreePath * treepath, 
			GtkTreeIter * iter, 
			gpointer data)
{
    Tag_item_t *tag = set_attribute_colorXSD(treemodel, iter);
    set_row_iconXSD(treemodel, iter, tag);
    // Mark all controllers
    if (all_hash && g_hash_table_lookup(all_hash, tag)){
        gint flag = IS_ALL_CTL;
        gtk_tree_store_set((GtkTreeStore *) treemodel, iter, 
                    H_BUTTON_COLUMN, list_add, 
                    H_FLAG_COLUMN, flag,
                    -1);
    }
        
    // Mark sequence controllers
    if (sequence_hash && g_hash_table_lookup(sequence_hash, tag)){
        gint flag = IS_SEQUENCE_CTL;
        gtk_tree_store_set((GtkTreeStore *) treemodel, iter, 
                    H_BUTTON_COLUMN, list_add, 
                    H_FLAG_COLUMN, flag,
                    -1);
    }
    // Mark choice controllers
    if (choice_hash && g_hash_table_lookup(choice_hash, tag)){
        gint flag = IS_CHOICE_CTL;

        gtk_tree_store_set((GtkTreeStore *) treemodel, iter, 
                    H_BUTTON_COLUMN, list_add, 
                    H_FLAG_COLUMN, flag,
                    -1);
    }
    return FALSE; 
}


Tag_item_t *
find_tag_with_name(Tag_t *Tag_p, Tag_item_t *parent_tag, 
        const gchar *tag_name,
        const gchar *name){ 
    Tag_item_t *retval=NULL;
    GSList *the_list = get_tag_item_list(Tag_p, parent_tag, NULL);
    GSList *l=the_list;
    for (;l && l->data; l= l->next){
        Tag_item_t * l_item = l->data;
        const gchar *nn = get_tag_name(l_item);
        if (nn && strcasecmp(nn, tag_name) == 0){ 
            Attribute_item_t *aa = get_attribute(l_item, "name");
            const gchar *n = get_attribute_value(aa);
            //if (n) NOOP( "looking at %s attribute (%s?)\n", n, name);
            if (n && strcasecmp(n, name)==0){
                NOOP( "gotcha: %s\n", name);
                retval = l_item;
                break;
            }
        }
	if (tag_item_has_children(l_item)){
            NOOP( "recursing...\n");
          retval=find_tag_with_name(Tag_p, l_item, tag_name, name);
          if (retval) break;
        }
    }
    g_slist_free(the_list);
    return retval;
}

Tag_item_t *
find_tag_with_attribute_and_value(Tag_t *Tag_p, Tag_item_t *parent_tag, const gchar *tag_name,
        const gchar *attribute_name, const gchar *attribute_value){ 
    Tag_item_t *retval=NULL;

    GSList *the_list = get_tag_item_list(Tag_p, parent_tag, tag_name);
    GSList *l=the_list;
    for (;l && l->data; l= l->next){
        Tag_item_t * l_item = l->data;
        Attribute_item_t *aa = get_attribute(l_item, attribute_name);
        if (aa) {
            const gchar *l_value = get_attribute_value(aa);
            NOOP( "%s==%s\n", l_value, attribute_value);
            if (l_value && strcasecmp(l_value, attribute_value)==0){
                retval = l_item;
                break;
            }
        }
    }
    g_slist_free(the_list);
    return retval;
}

static const gchar *
get_element_string(Tag_t *Tag_p, const gchar *name){
    const gchar *s = NULL;
    Tag_item_t *t = (complexType_hash)?g_hash_table_lookup(complexType_hash, name):NULL;
    if (!t) return NULL;
    NOOP( "found tag %p\n", t);
        // got the parent tag. Now get the first tag called "attribute" with name=="string"
        GSList *ta_list = get_tag_item_list(Tag_p, t, "attribute");
        GSList *ta = ta_list;
        for (;ta && ta->data; ta=ta->next){
            Tag_item_t *q = ta->data;
            NOOP( "tag child %p: \"%s\"\n", q, get_tag_name(q));
            if (strcasecmp(get_tag_name(q), "attribute")==0){
                NOOP( "gotcha again...\n");
                Attribute_item_t *b = get_attribute(q, "name");
                if (b) {
                    const gchar *c = get_attribute_value(b);
/*// DBG
GSList *xl = get_attribute_item_list(q);
GSList *xll = xl;
for (;xll && xll->data; xll = xll->next){
Attribute_item_t *a = xll->data;
fprintf(stderr, "attributes: \"%s\" --> \"%s\"\n", get_attribute_name(a), get_attribute_value(a));
}
g_slist_free(xl);
// DBG*/
                    if (c && strcasecmp(c, "string")==0){
                        b = get_attribute(q, "fixed");
                        if (b) s = get_attribute_value(b);
                        NOOP( "and gotcha again: \"%s\"\n", c);
                        break;
                    }
                }
            }
        }
        g_slist_free(ta_list);
    return s;
}
            
static void 
assign_default_value(Tag_item_t *src_item, Tag_t *newTag_p, Tag_item_t *tgt_item){
    if (!src_item || strcasecmp("element", get_tag_name(src_item))) return;
    
    Attribute_item_t *default_at = get_attribute(src_item, "default");
    if(default_at){
        const gchar *c = get_attribute_value(default_at); 
        if (c) attribute_item_add(newTag_p, tgt_item, "text", c, NULL);
    }
    return;
}

static void
add_schema_defined_attributes(Tag_item_t *src_item, Tag_t *newTag_p, Tag_item_t *tgt_item){
    if (!src_item || strcasecmp("element", get_tag_name(src_item))) return;
    
    GSList *attribute_list = get_attribute_item_list(src_item);
    GSList *tmp = attribute_list;
    for(;tmp && tmp->data; tmp=tmp->next){
        Attribute_item_t *at_item = tmp->data;
        const gchar *aname = get_attribute_name(at_item);
        const gchar *value = get_attribute_value(at_item);
        if (strcasecmp(aname, "type")==0){
            gboolean defined_type = FALSE;
            if (complexType_hash && g_hash_table_lookup(complexType_hash, value)) defined_type = TRUE;
            if (simpleType_hash && g_hash_table_lookup(simpleType_hash, value)) defined_type = TRUE;
            // if not a complex type, do this:
            if (!defined_type) aname = "text:type"; 
        }
        // This would add to tag structure, which we de not want
        Attribute_item_t *new_item = attribute_item_add(newTag_p, tgt_item, aname, value, NULL);
        // But we do want to add to the treeview, so mark it hidden:
        attribute_set_hidden(new_item, TRUE);
    }
}

static void
add_boolean_restriction(Tag_t *Tag_p, Tag_item_t *src_item, Tag_t *newTag_p, Tag_item_t *tgt_item){
    if (!src_item || strcasecmp("element", get_tag_name(src_item))) return;

    // boolean type is a simple restricted text
    Attribute_item_t *type_item = get_attribute(src_item, "type");
    const gchar *pattern = "text:pattern";
    if (type_item){
        const gchar *t = get_attribute_value(type_item);
        if (strstr(t, "boolean")){
            Attribute_item_t *new_item = 
                attribute_item_add(newTag_p, tgt_item, pattern, "0|1|true|false", NULL);
            // mark it hidden:
            attribute_set_hidden(new_item, TRUE);
            return;
        }
    }
    return;
}

static void
add_list_restrictions(Tag_t *Tag_p, Tag_item_t *simple_tag, Tag_t *newTag_p, Tag_item_t *tgt_item){
    // Get simpleType tag restrictions
    if (!simple_tag) return;
    Tag_item_t *list_tag = get_tag_item(Tag_p, simple_tag, "list");
    if (!list_tag) return;
    NOOP( "gotcha: list_tag\n");
    Attribute_item_t *itemType_at = get_attribute(list_tag, "itemType");
    if (!itemType_at) {
        fprintf(stderr, "no itemType attribute in list definition\n");
        return;
    }

    const gchar *list_type = get_attribute_value(itemType_at);
    //if (list_type && strchr(list_type, ':')) list_type = strchr(list_type, ':') + 1;
    // This would add to tag structure
    Attribute_item_t *new_item = 
        attribute_item_add(newTag_p, tgt_item, "text:type", "list", NULL);
    attribute_set_hidden(new_item, TRUE);
    new_item = 
        attribute_item_add(newTag_p, tgt_item, "list:itemType", list_type, NULL);
    // But we do want to consider it part of the treeview, so mark it hidden
    attribute_set_hidden(new_item, TRUE);
}

static void
add_text_restrictions(Tag_t *Tag_p, Tag_item_t *simple_tag, Tag_t *newTag_p, Tag_item_t *tgt_item){
    // Get simpleType tag restrictions
    if (!simple_tag) return;
    Tag_item_t *restriction_tag = get_tag_item(Tag_p, simple_tag, "restriction");
    if (!restriction_tag) return;
    NOOP( "gotcha: restriction_tag\n");
    // Here we could get base attribute, if it were actually useful...
    Tag_item_t *pattern_tag = get_tag_item(Tag_p, restriction_tag, "pattern");
    if (!pattern_tag) return;

    Attribute_item_t *value_item = get_attribute(pattern_tag, "value");
    if (!value_item) return;

    // This would add to tag structure
    Attribute_item_t *new_item = 
        attribute_item_add(newTag_p, tgt_item, "text:pattern", get_attribute_value(value_item), NULL);
    // But we do want to consider it part of the treeview, so mark it hidden
    attribute_set_hidden(new_item, TRUE);
}

static gchar *
get_simpletype_restrictions(Tag_t *Tag_p, Tag_item_t *attribute_item){
    NOOP( "get_simpletype_restrictions...\n");
       gchar *r = NULL;
       Tag_item_t *simple_tag = get_tag_item(Tag_p, attribute_item, "simpleType");
       if (!simple_tag) {
            NOOP( "no simpleTag...\n");
           return r;
       }
       Tag_item_t *restriction_tag = get_tag_item(Tag_p, simple_tag, "restriction");
       if (!restriction_tag) {
            NOOP( "no restriction_tag...\n");
           return r;
       }
       GSList *list = get_tag_item_list(Tag_p, restriction_tag, "enumeration");
       if (!list){
           NOOP( "no enumerations...\n");
           return r;
       }
       GSList *l = list;
       for (; l && l->data; l=l->next){
           Tag_item_t *t = l->data;
           Attribute_item_t *a = get_attribute(t, "value");
           if (a){
               const gchar *av =  get_attribute_value(a);
               if (av && strlen(av)){
                   gchar *g = g_strconcat(
                           r?r:"",
                           r?"|":"",
                           av, NULL);
                   g_free(r);
                   r=g;
               }
           }
       }
       g_slist_free(list);
       return r; 
}

static void
add_simpletype_restrictions(Tag_t *Tag_p, Tag_t *newTag_p, Tag_item_t *newparent_item, Tag_item_t *attribute_tag)
{ 
    NOOP( "add_simpletype_restrictions...\n");
    Attribute_item_t *a_item = get_attribute(attribute_tag, "name");
    const gchar *name = get_attribute_value(a_item);

    //  restrictions... 
    gchar *r = get_simpletype_restrictions(Tag_p, attribute_tag);
    NOOP( "restrictions=%s\n", r);
    if (r){
        gchar *pattern = g_strdup_printf("%s:pattern", name);
        Attribute_item_t *pattern_item =
            attribute_item_add(newTag_p, newparent_item, pattern, r, NULL);
        attribute_set_hidden(pattern_item, TRUE); 
        g_free(r);
        g_free(pattern);
    }
}

static const gchar *
add_subattribute(Tag_t *newTag_p, Tag_item_t *newparent_item, 
        Tag_item_t *attribute_tag, const gchar *name, const gchar *sub_name)
{
    Attribute_item_t *a_item = get_attribute(attribute_tag, sub_name);
    if (a_item){
      const gchar *value = get_attribute_value(a_item);
      gchar *g =g_strdup_printf("%s:%s", name, sub_name);
      Attribute_item_t *new_item = 
        attribute_item_add(newTag_p, newparent_item, g, value, NULL);
      g_free(g);
      // But we do want to add to the treeview, so mark it hidden:
      attribute_set_hidden(new_item, TRUE); 
      return get_attribute_value(new_item);
    }
    return NULL;
}

static void
add_simple_attributes(Tag_t *Tag_p, Tag_t *newTag_p, Tag_item_t *newparent_item, Tag_item_t *complex_tag){
    NOOP( "add_simple_attributes...\n");
   // get all attributes, which are setup as tags with "attribute" name here...
   GSList *list = get_tag_item_list(Tag_p, complex_tag, "attribute");
   GSList *l = list;
   for (;l && l->data; l=l->next){
        Tag_item_t *attribute_tag = l->data;
        Attribute_item_t *a_item = get_attribute(attribute_tag, "name");
        const gchar *name = get_attribute_value(a_item);

        add_subattribute(newTag_p, newparent_item, attribute_tag, name, "use");
        const gchar *default_value = 
            add_subattribute(newTag_p, newparent_item, attribute_tag, name, "default");
        const gchar *fixed  = 
            add_subattribute(newTag_p, newparent_item, attribute_tag, name, "fixed");
        add_subattribute(newTag_p, newparent_item, attribute_tag, name, "type");

        add_simpletype_restrictions(Tag_p, newTag_p, newparent_item, attribute_tag);

        if (fixed){
            Attribute_item_t *a = attribute_item_add(newTag_p, newparent_item, name, fixed, NULL);
            if (!noneditable_hash) noneditable_hash = 
                g_hash_table_new(g_direct_hash, g_direct_equal);
            g_hash_table_replace(noneditable_hash, a, GINT_TO_POINTER(1));
        } else {
            attribute_item_add(newTag_p, newparent_item, name, default_value, NULL);
        }
   }
   g_slist_free(list); 
} 

static gboolean
add_simple_content(Tag_t *Tag_p, Tag_t *newTag_p, Tag_item_t *tgt_item, Tag_item_t *complex_tag)
{
   Tag_item_t *simple_tag = get_tag_item(Tag_p, complex_tag, "simpleContent");
   if (!simple_tag) return FALSE;

   // add text attribute hmmm XXX
   attribute_item_add(newTag_p, tgt_item, "text", "", NULL);
  
   // look for "extension"
   Tag_item_t *extension_tag = get_tag_item(Tag_p, simple_tag, "extension");
   if (!extension_tag) {
       return FALSE;
   }
        // add attributes to tag.
   add_simple_attributes(Tag_p, newTag_p, tgt_item, extension_tag);

   // base 
    Attribute_item_t *a_item = get_attribute(extension_tag, "base");
    if (a_item) {
      const gchar *base = get_attribute_value(a_item);
      if (base){
        //  base can be standard type or a simple/complex type
        NOOP( "add_simple_content...extension base=%s\n", base);
        // Test for simple type
        Tag_item_t *simpletype_tag = (!simpleType_hash)?NULL:
            g_hash_table_lookup(simpleType_hash, base);
        if (complexType_hash && g_hash_table_lookup(complexType_hash, base)){
            NOOP( "* it is a defined complexType!\n");
            return FALSE;
        }
        else if (simpletype_tag){
            NOOP( "-- found simpleType definition for extension base = %s\n", base);
            // only list types supported today
            add_list_restrictions(Tag_p, simpletype_tag, newTag_p, tgt_item);
        } else {
//            const gchar *b = strchr(base,':')? strchr(base,':')+1:base;
            const gchar *b = base;
            // add text restriction
            Attribute_item_t *at = attribute_item_add(newTag_p, tgt_item, "text:type", b, NULL);
            attribute_set_hidden(at, TRUE);
        }
      }
    }     
    return TRUE;

}

static Tag_item_t *
process_element(xmltree_t *xmltree_p, Tag_t *newTag_p, Tag_item_t *src_item, Tag_item_t *tgt_item);

static void
add_complextype_items(xmltree_t *xmltree_p, Tag_item_t *complexType_item, Tag_t *newTag_p, Tag_item_t *tgt_item){
    if (!complexType_item || strcasecmp("complexType", get_tag_name(complexType_item))) return;
    Tag_t *Tag_p = xmltree_p->Tag_p;

 
    // Add just simple content?
    if (add_simple_content(Tag_p, newTag_p, tgt_item, complexType_item)) return;

    // Get simpleType tag restrictions
    Attribute_item_t *a_item = get_attribute(complexType_item, "mixed");
    if (a_item){
        const gchar *v = get_attribute_value(a_item);
        if (v && (strcasecmp(v, "true") || strcasecmp(v, "1"))){
          // add text attribute
          attribute_item_add(newTag_p, tgt_item, "text", "", NULL);
        }
    }
    // add simple attributes 
     add_simple_attributes(Tag_p, newTag_p, tgt_item, complexType_item);
    // add simple content (see above)
    //add_simple_content(Tag_p, newTag_p, tgt_item, complexType_item);
    // add all elements (beneath all, sequence or choice)
    Tag_item_t *container_item;
    gchar *containers[] = {"all", "sequence", "choice", NULL};
    gchar **p=containers;
    for (;p && *p; p++){
        container_item = get_tag_item(Tag_p, complexType_item, *p);
        if (container_item){
            // Replicate minOccurs/maxOccurs within element for choice container
            // If not set for all or sequence, set to "1"
            Attribute_item_t *min_at = get_attribute(container_item, "minOccurs");
            Attribute_item_t *max_at = get_attribute(container_item, "maxOccurs");
            GSList *list = get_tag_item_list(Tag_p, container_item, "element");
            GSList *l = list;
            for (;l && l->data; l=l->next){
                // add new element
                //Attribute_item_t *name_at = get_attribute((Tag_item_t *)l->data, "name");
                //Tag_item_t *new_element = tag_item_add(newTag_p, tgt_item, get_attribute_value(name_at));
                // recurse
                Tag_item_t *new_element = 
                    process_element(xmltree_p, newTag_p, (Tag_item_t *)l->data, tgt_item);
                if (strcasecmp(*p, "choice")==0){
                    if (min_at) {
                        attribute_set_hidden(attribute_item_add(newTag_p, new_element, 
                            "minOccurs", get_attribute_value(min_at), NULL), TRUE);
                    } else {
                         attribute_set_hidden(attribute_item_add(newTag_p, new_element, 
                            "minOccurs", "1", NULL), TRUE);
                   }
                    if (max_at) {
                        attribute_set_hidden(attribute_item_add(newTag_p, new_element, 
                            "maxOccurs", get_attribute_value(max_at), NULL), TRUE);
                    } else {
                        attribute_set_hidden(attribute_item_add(newTag_p, new_element, 
                            "maxOccurs", "1", NULL), TRUE);

                    }
                } else {
                    // set to one if not defined.

                }
                 if (strcasecmp(*p, "all")==0){
                    if (!all_hash) all_hash = g_hash_table_new(g_direct_hash, g_direct_equal);
                    g_hash_table_replace(all_hash, new_element, GINT_TO_POINTER(1));
                    //fprintf(stderr, "%s is all\n", get_tag_name(new_element));
                    tag_set_hidden(new_element, TRUE);
                }
           
                if (strcasecmp(*p, "sequence")==0){
                    if (!sequence_hash) sequence_hash = g_hash_table_new(g_direct_hash, g_direct_equal);
                    g_hash_table_replace(sequence_hash, new_element, GINT_TO_POINTER(1));
                    //fprintf(stderr, "%s is sequence\n", get_tag_name(new_element));
                    tag_set_hidden(new_element, TRUE);
                }
                if (strcasecmp(*p, "choice")==0){
                    if (!choice_hash) choice_hash = g_hash_table_new(g_direct_hash, g_direct_equal);
                    g_hash_table_replace(choice_hash, new_element, GINT_TO_POINTER(1));
                    tag_set_hidden(new_element, TRUE);
                }
            }
            g_slist_free(list);
            break;
        }
    }
}

static void
add_simpletype_items(Tag_t *Tag_p, Tag_item_t *simpleType_item, Tag_t *newTag_p, Tag_item_t *newtgt_item){
    // process simpleType definition
    // add specific patterns for text elements 
    add_text_restrictions(Tag_p, simpleType_item, newTag_p, newtgt_item);
    // process lists here, element list, attribute itemType 
    add_list_restrictions(Tag_p, simpleType_item, newTag_p, newtgt_item);
    // Verification:
    GSList *list = get_tag_item_list(Tag_p, simpleType_item, NULL);
    GSList *l = list;
    for (;l && l->data; l=l->next){
        const gchar *s = get_tag_name((Tag_item_t *)(l->data));
        if (strcasecmp(s,"restriction") && strcasecmp(s,"list")){
            fprintf(stderr, "*** simpleType child tag \"%s\" not considered (FIXME)\n", s);
        }
    }
    g_slist_free(list);
}

static Tag_item_t *
process_element(xmltree_t *xmltree_p, Tag_t *newTag_p, Tag_item_t *src_item, Tag_item_t *tgt_item){
    if (strcasecmp("element", get_tag_name(src_item))) return NULL;
    // "name" attribute is mandatory
    Attribute_item_t *name_item = get_attribute(src_item, "name");
    if (!name_item) return NULL;
    const gchar *name = get_attribute_value(name_item);
    if (!name || !strlen(name)) return NULL;

    // element may be simple or complex Type. 
    // type may be specified with "type" attribute or contained within element
    Tag_t *Tag_p = xmltree_p->Tag_p;
    Tag_item_t *newtgt_item = NULL;

    newtgt_item = tag_item_add(newTag_p, tgt_item, name);
    // XSD tree will be able to modify all elements 
    set_editable_element(xmltree_p, name, TRUE);

    /*HACK set cell name*/

    // Find an associated string element in xschema types
    const gchar *s = get_element_string(Tag_p, name);
    if (s){ 
        tag_item_set_string(newtgt_item, s);
    }
    
    // Test "element" for global defined type
    Attribute_item_t *type_at = get_attribute(src_item, "type");
    Tag_item_t *complexType_item = NULL;
    Tag_item_t *simpleType_item = NULL;
    if (type_at){
        complexType_item = (!complexType_hash)? NULL:
            g_hash_table_lookup(complexType_hash, get_attribute_value(type_at));
        simpleType_item = (!simpleType_hash)? NULL:
            g_hash_table_lookup(simpleType_hash, get_attribute_value(type_at));
    } else {
        // Test for local defined type
        complexType_item = get_tag_item(Tag_p, src_item, "complexType");
        simpleType_item = get_tag_item(Tag_p, src_item, "simpleType");
    }

    // Assume simple content for non-typed elements
    if (!complexType_item && !simpleType_item){
        NOOP( "\"%s\" is not defined as simple or complex Type\n", name);
        gchar *t = strchr(get_attribute_value(type_at), ':');
        if (t) t++; else t=(gchar *)get_attribute_value(type_at);
        // FIXME: consider integer, double, and other standard types, if any, besides "string"
        if (strcasecmp("string", t) && strcasecmp("boolean", t)
                && strcasecmp("integer", t) && strcasecmp("double", t)){
            fprintf(stderr, "element type=%s, I don't know what to do...\n", t);
        } 
        attribute_item_add(newTag_p, newtgt_item, "text", NULL, NULL);
        // assign default value to text attribute, if any.
        assign_default_value(src_item, newTag_p, newtgt_item);
        // internal types: string, boolean, integer, double,...
        // add a pattern for "boolean" text elements
        add_boolean_restriction(Tag_p, src_item, newTag_p, newtgt_item);
    }

    if (simpleType_item){
        attribute_item_add(newTag_p, newtgt_item, "text", NULL, NULL);
        // assign default value to text attribute, if any.
        assign_default_value(src_item, newTag_p, newtgt_item);
        add_simpletype_items(Tag_p, simpleType_item, newTag_p, newtgt_item);
    } else if (complexType_item){
        // process complexType definition
        // In order to have attributes, item must be complexType
        add_complextype_items(xmltree_p, complexType_item, newTag_p, newtgt_item);
    }

    // Process schema defined attributes
    add_schema_defined_attributes(src_item, newTag_p, newtgt_item);

    return newtgt_item;

}

/////////////////////////////////////////////////////////////////////////////////

static GtkTreeRowReference *
get_row_reference(GtkTreeModel *model, GtkTreeIter *iter){
    GtkTreePath *path = gtk_tree_model_get_path(model, iter);
    GtkTreeRowReference *row_ref = gtk_tree_row_reference_new(model, path);
    gtk_tree_path_free(path);
    return row_ref;
}
static gboolean
get_row_iter(GtkTreeModel *model, GtkTreeRowReference *ref, GtkTreeIter *iter){
    GtkTreePath *path = gtk_tree_row_reference_get_path(ref);
    gboolean retval = gtk_tree_model_get_iter(model, iter, path);
    gtk_tree_path_free(path);
    return retval;
}

static void copy_attributes(Tag_t *Tag_p, Tag_item_t *tgt_tag, Tag_item_t *src_tag, 
        GtkTreeModel *treemodel, GtkTreeRowReference *ref){
    GtkTreeIter iter;
    GSList *list = get_attribute_item_list(src_tag);
    GSList *l = list;
    NOOP( "attribute list = %p\n", l);
    for (;l && l->data; l = l->next){
        Attribute_item_t *at = l->data;
        const gchar *aname = get_attribute_name(at);
        const gchar *avalue = get_attribute_value(at);
        NOOP( "%s --> %s\n", aname, avalue);
        Attribute_item_t *new_at = attribute_item_add(Tag_p, tgt_tag, aname, avalue, NULL);
        // XXX fails: (FIXME: is original attribute not hidden?)
        //attribute_set_hidden(new_at, attribute_get_hidden(at));
        // and this does work... 
        if (is_ctl(get_attribute_parent(new_at))) attribute_set_hidden(new_at, TRUE);
        GtkTreeIter child;
        if (get_row_iter(treemodel, ref, &iter)) {
            gtk_tree_store_insert_after(GTK_TREE_STORE(treemodel), &child, &iter, NULL);
            gtk_tree_store_set(GTK_TREE_STORE(treemodel), &child, 
                H_TAG_ITEM_COLUMN, tgt_tag,
                H_VALUE_COLUMN, avalue,
                H_ATTRIBUTE_COLUMN, aname,
                H_ATTRIBUTE_ITEM_COLUMN, new_at,
                -1);
        }
        if (noneditable_hash && g_hash_table_lookup(noneditable_hash, at)){
            g_hash_table_replace(noneditable_hash, new_at, GINT_TO_POINTER(1));
        }
        
    }
    g_slist_free(list);
    // Attribute icon depends on all set attributes (use:optional, f.e)
 
    if (get_row_iter(treemodel, ref, &iter)) {
        GtkTreeIter c;
        if (gtk_tree_model_iter_children(treemodel, &c, &iter)){
            do {
                set_attribute_colorXSD(treemodel, &c);
                set_row_iconXSD(treemodel, &c, NULL);
            } while (gtk_tree_model_iter_next(treemodel, &c));
        }
    }
}
#if 0
gboolean get_tag_path_bug(GtkTreeModel *treemodel, GtkTreePath *path, GtkTreeIter *iter, void *data){
    void **arg = data;
    Tag_item_t *tag = arg[0];
    Tag_item_t *current_tag;
    gtk_tree_model_get (treemodel, iter, 
            H_TAG_ITEM_COLUMN, &current_tag,
            -1);
    if (current_tag == tag) {
        arg[1] = gtk_tree_path_to_string(path);
        return TRUE;
    }
    return FALSE;
}
static gboolean
find_iter_from_tag(GtkTreeIter *iter, GtkTreeModel *treemodel, Tag_item_t *tag){
    void *arg[]={tag, NULL};
    gtk_tree_model_foreach(treemodel, get_tag_path_bug, arg);
    if (!arg[1]) return FALSE;
        NOOP( "found it...%s\n", (gchar *)(arg[1]));
    gtk_tree_model_get_iter_from_string(treemodel, iter, (gchar *)(arg[1]));
    g_free(arg[1]);
    return TRUE;
}
#endif

gboolean get_tag_reference(GtkTreeModel *treemodel, GtkTreePath *path, GtkTreeIter *iter, void *data){
    void **arg = data;
    Tag_item_t *tag = arg[0];
    Tag_item_t *current_tag;
    gtk_tree_model_get (treemodel, iter, 
            H_TAG_ITEM_COLUMN, &current_tag,
            -1);
    if (current_tag == tag) {
        arg[1] = gtk_tree_row_reference_new(treemodel, path);
        return TRUE;
    }
    return FALSE;
}

static GtkTreeRowReference *
find_reference_from_tag(GtkTreeModel *treemodel, Tag_item_t *tag){
    void *arg[]={tag, NULL};
    gtk_tree_model_foreach(treemodel, get_tag_reference, arg);
    GtkTreeRowReference *ref = arg[1];
    return ref;
}

static Tag_item_t *
copy_tag(Tag_t *Tag_p, Tag_item_t *parent_tag, Tag_item_t *src_tag, 
        GtkTreeModel *treemodel, 
        GtkTreeRowReference *parent_ref, 
        GtkTreeRowReference *sibling_ref){
    GtkTreePath *treepath;
    GtkTreeIter iter;
    GtkTreeRowReference *child_ref;

    Tag_item_t *new_tag = tag_item_add(Tag_p, parent_tag, get_tag_name(src_tag));
    // new tags cannot be in hash tables (pointer may have been deleted...)
    if (all_hash) g_hash_table_replace(all_hash, new_tag, NULL);
    if (sequence_hash) g_hash_table_replace(sequence_hash, new_tag, NULL);
    if (choice_hash) g_hash_table_replace(choice_hash, new_tag, NULL);

    // a) Replicate all/sequence/choice status (for children)
    if (parent_ref){
        if (all_hash && g_hash_table_lookup(all_hash, src_tag)){
            NOOP( "inserting into all hash: %p\n", new_tag);
            g_hash_table_insert(all_hash, new_tag, new_tag);
            tag_set_hidden(new_tag, TRUE);
        }
        if (sequence_hash && g_hash_table_lookup(sequence_hash, src_tag)){
            NOOP( "inserting into sequence hash: %p\n", new_tag);
            g_hash_table_insert(sequence_hash, new_tag, new_tag);
            tag_set_hidden(new_tag, TRUE);
        }
        if (choice_hash && g_hash_table_lookup(choice_hash, src_tag)){
            NOOP( "inserting into choice hash: %p\n", new_tag);
            g_hash_table_insert(choice_hash, new_tag, new_tag);
            tag_set_hidden(new_tag, TRUE);
        } //else NOOP( "NOT inserting into choice hash: %p\n", new_tag);
    }

    // Copy treemodel data
    // a) insert new row into treemodel
    {
        GtkTreeIter child;
        if (sibling_ref){
            // If sibling_ref is a sequence controller, then append. Otherwise insert.
            GtkTreeIter sibling_iter;
            get_row_iter(treemodel, sibling_ref, &sibling_iter);
            if (sequence_hash && g_hash_table_lookup(sequence_hash, src_tag)){
                // append (order is conserved)
                gtk_tree_store_insert_after(GTK_TREE_STORE(treemodel), &child, NULL, &sibling_iter);
            } else {
                // insert (order is not important)
                GtkTreeIter parent_iter, first;
                gtk_tree_model_iter_parent(treemodel, &parent_iter, &sibling_iter);
                gtk_tree_model_iter_children(treemodel, &first, &parent_iter);
                do {
                    Attribute_item_t *at;
                    gtk_tree_model_get(treemodel, &first,
                      H_ATTRIBUTE_ITEM_COLUMN, &at, -1);
                    if (at == NULL) break;
                } while (gtk_tree_model_iter_next(treemodel, &first));
                // test for valid iter...
                gtk_tree_store_insert_before(GTK_TREE_STORE(treemodel), &child, NULL, &first);
            }
        } else {
            GtkTreeIter parent_iter;
            get_row_iter(treemodel, parent_ref, &parent_iter);
            gtk_tree_store_insert_after(GTK_TREE_STORE(treemodel), &child, 
                    &parent_iter, NULL);
        }
        gtk_tree_store_set(GTK_TREE_STORE(treemodel), &child, 
            H_TAG_ITEM_COLUMN, new_tag,
            H_TAG_COLUMN, get_tag_name(new_tag),
            -1);
            
        child_ref = get_row_reference(treemodel, &child);
        // copy attributes verbatim
        copy_attributes(Tag_p, new_tag, src_tag, treemodel, child_ref);
    }
    // b) Get source treemodel row data and copy to target treemodel row
    {
        GtkTreeIter src_iter, child;
        if (parent_ref){
            GtkTreeRowReference *ref = find_reference_from_tag(treemodel, src_tag);
            if (!ref){
                g_error("Cannot find treepath from tag: CRITICAL\n");
            }
            get_row_iter(treemodel, ref, &src_iter);
        } else {
            get_row_iter(treemodel, sibling_ref, &src_iter);
        }

        GdkPixbuf *pixbuf, *button;
        const gchar *color;
        gint flag;
        gtk_tree_model_get (treemodel, &src_iter, 
            H_PIXBUF_COLUMN, &pixbuf,
            H_BUTTON_COLUMN, &button,
            //H_NS_COLUMN, &ns,
           // tag_items do not have values H_VALUE_COLUMN, &value,
            H_COLOR_COLUMN, &color,
            H_FLAG_COLUMN, &flag, 
            -1);
         get_row_iter(treemodel, child_ref, &child);
         if(parent_ref){
             gtk_tree_store_set(GTK_TREE_STORE(treemodel), &child, 
                  H_PIXBUF_COLUMN, pixbuf,
                  H_BUTTON_COLUMN, button, 
                  H_FLAG_COLUMN, flag, 
                //  H_NS_COLUMN, g_strdup(ns),
                  H_COLOR_COLUMN, g_strdup(color),// must be freed on row zap
              -1);
                 //fprintf(stderr, "flag copied 0x%x\n", flag);
         } else {
             // parent not specified, then icon must be resolved and flag set to 0
             gint newflag = 0;
             if (flag & IS_ALL_CTL) newflag = IS_ALL_CHILD;
             else if (flag & IS_SEQUENCE_CTL) newflag = IS_SEQUENCE_CHILD;
             else if (flag & IS_CHOICE_CTL) newflag = IS_CHOICE_CHILD;
             GdkPixbuf *button = (flag)?list_remove:NULL;
             // invalidate hidden color status for sequence and controller children
             color=g_strdup("black");
             gtk_tree_store_set(GTK_TREE_STORE(treemodel), &child, 
                H_BUTTON_COLUMN, button,
              //  H_NS_COLUMN, g_strdup(ns),
              // tag_item type does not have value...  H_VALUE_COLUMN, g_strdup(value),
                H_COLOR_COLUMN, g_strdup(color),
                H_FLAG_COLUMN, newflag, 

               -1);             
                 //fprintf(stderr, "child from ctl, storing 0x%x --> 0x%x\n", flag, newflag);
         } 
         // until recurse returns:
          set_row_iconXSD(treemodel, &child, new_tag);

        while (gtk_events_pending()) gtk_main_iteration();
    }

    // Recurse
    GSList *list = get_tag_item_list(Tag_p, src_tag, NULL);
    GSList *l = list;
    //fprintf(stderr, "tag list = %p\n", l);
    for (;l && l->data; l = l->next){
      copy_tag(Tag_p, new_tag, (Tag_item_t *)(l->data), treemodel, child_ref, NULL);
        while (gtk_events_pending()) gtk_main_iteration();
    }
    g_slist_free(list);

    
    // Now set the icon for the element. (red/greenball)
    GtkTreeIter child;
    get_row_iter(treemodel, child_ref, &child);
    //fprintf(stderr, "setting icon for %s\n", get_tag_name(new_tag));
    set_row_iconXSD(treemodel, &child, new_tag);
    gtk_tree_row_reference_free(child_ref);
    

    return new_tag;
}


static gboolean 
is_ctl_full(Tag_t *Tag_p, Tag_item_t *src_tag, GHashTable *hash){
    Tag_item_t *parent_tag = get_parent_tag(src_tag);
    Attribute_item_t *at = get_attribute(src_tag, "maxOccurs");
    const gchar *v = "1";
    if (at) v = get_attribute_value(at);
    gint count = 0;
    long num;

    gboolean result = FALSE;
   // unbounded will spit an error...
   if (v && strcasecmp(v, "unbounded")){
       errno = 0;
       if (hash != choice_hash){

       }
       num = strtol(v,NULL,10);
       if (errno) return FALSE;
       GSList *list = get_tag_item_list(Tag_p, parent_tag, 
               (hash==choice_hash)?NULL:get_tag_name(src_tag));
       GSList *l = list;
       for (;l && l->data; l = l->next){
           if (!hash) continue;
           if (g_hash_table_lookup(hash, l->data)) continue;
           count++;
       }
       g_slist_free(list);
       if (num <= count){
           result = TRUE;
       }
   }
   //fprintf(stderr, "is_ctl_full, maxOccurs= %ld, count= %d\n", num, count);
   return result;
}

static GHashTable *
get_associated_hash(GtkTreeModel *treemodel, GtkTreeRowReference *sibling_ref){
    GtkTreeIter sibling;
    gint flag;
    // Add new element to "choice/all/sequence"
    get_row_iter(treemodel, sibling_ref, &sibling);
    gtk_tree_model_get(treemodel, &sibling,
            H_FLAG_COLUMN, &flag,
            -1);
    //fprintf(stderr, "got flag = 0x%x\n", flag);
    gboolean is_choice = flag & (IS_CHOICE_CTL | IS_CHOICE_CHILD);
    gboolean is_sequence = flag & (IS_SEQUENCE_CTL | IS_SEQUENCE_CHILD);
    gboolean is_all = flag & (IS_ALL_CTL | IS_ALL_CHILD);
    if (!is_choice && !is_sequence && !is_all){
       NOOP( "not choice nor sequence\n");
       return NULL;
    }
    GHashTable *hash = NULL;
    if (is_choice) hash = choice_hash;
    else if (is_sequence) hash = sequence_hash;
    else if (is_all) hash = all_hash;
    return hash;
}

// ctl_visuals: change visual status of controllers (choice/all/sequence) as
//              items are added or removed.
static void
ctl_visuals_remove(Tag_t *Tag_p, Tag_item_t *parent_tag, const gchar *name, GtkTreeModel *treemodel, GtkTreeRowReference *sibling_ref, gint flag){
    // Are we dealing with a controller or a controller child? (zap_it == controller child).
    // Default values for button and color are for controller child.
    GdkPixbuf *button = list_add;
    const gchar *color = HIDDEN_COLOR;
    GHashTable *hash = choice_hash;
    if (flag & (IS_SEQUENCE_CHILD | IS_SEQUENCE_CTL)) hash = sequence_hash;
    else if (flag & (IS_ALL_CHILD | IS_ALL_CTL)) hash = all_hash;
    GSList *list, *l;

    // sequence/all controllers are individually determined. 
    if (hash != choice_hash){
        GtkTreeIter iter;
        // Figure out the iter for the controller.
        list = get_tag_item_list(Tag_p, parent_tag, name);
        for (l=list; l && l->data; l= l->next){
            Tag_item_t *tag = l->data;
            if (!hash || !g_hash_table_lookup(hash, tag)) continue;
            if (strcasecmp(get_tag_name(tag), name)==0){
                GtkTreeRowReference *ref = find_reference_from_tag(treemodel, tag);
                if (ref == NULL) continue; // This should not happen...
                get_row_iter(treemodel, ref, &iter);
                gtk_tree_row_reference_free(ref);
                break;
            }
        }
        g_slist_free(list);
        gtk_tree_store_set(GTK_TREE_STORE(treemodel), &iter, 
            H_BUTTON_COLUMN, button,
            H_COLOR_COLUMN, color,
            -1);  
        return;
    }

    // Choice controller requires resetting all choice controller in the same block
    list = get_tag_item_list(Tag_p, parent_tag, NULL);
    GtkTreeIter iter;
    for (l = list;l && l->data; l = l->next){
        Tag_item_t *tag = l->data;
        // Skip all non controllers.
        if (!hash || !g_hash_table_lookup(hash, tag)) continue;
        // Get the respective tree iter
        GtkTreeRowReference *ref = find_reference_from_tag(treemodel, tag);
        if (ref == NULL) continue; // This should not happen...
        get_row_iter(treemodel, ref, &iter);
        gtk_tree_store_set(GTK_TREE_STORE(treemodel), &iter, 
            H_BUTTON_COLUMN, button,
            H_COLOR_COLUMN, color,
            -1);    
        gtk_tree_row_reference_free(ref);
    }
    g_slist_free(list);
    return;
}

static void
ctl_visuals_add(Tag_t *Tag_p, Tag_item_t *src_tag, GtkTreeModel *treemodel, GtkTreeRowReference *sibling_ref, gint flag){
    // Are we dealing with a controller or a controller child? (zap_it == controller child).
    // Default values for button and color are for controller child.
    GdkPixbuf *button = NULL;
    const gchar *color = READONLY_COLOR;
    GHashTable *hash = choice_hash;
    if (flag & (IS_SEQUENCE_CHILD | IS_SEQUENCE_CTL)) hash = sequence_hash;
    else if (flag & (IS_ALL_CHILD | IS_ALL_CTL)) hash = all_hash;

    // If not a controller child, figure out if controller is full (sequence/all)
    if (!is_ctl_full(Tag_p, src_tag, hash)) return;
    // sequence/all controllers are individually determined. 
    if (!is_choice_ctl(src_tag)){
        GtkTreeIter iter;
        // If zapping item, figure out the iter for the controller
        get_row_iter(treemodel, sibling_ref, &iter);
        gtk_tree_store_set(GTK_TREE_STORE(treemodel), &iter, 
            H_BUTTON_COLUMN, button,
            H_COLOR_COLUMN, color,
            -1); 
        return;
    }
    // Choice controller requires resetting all choice controller in the same block
    GSList *list, *l;
    list = get_tag_item_list(Tag_p, get_parent_tag(src_tag), NULL);
    GtkTreeIter iter;
    for (l = list;l && l->data; l = l->next){
        Tag_item_t *tag = l->data;
        // Skip all non controllers.
        if (!hash || !g_hash_table_lookup(hash, tag)) continue;
        // Get the respective tree iter
        GtkTreeRowReference *ref = find_reference_from_tag(treemodel, tag);
        if (ref == NULL) continue; // This should not happen...
        get_row_iter(treemodel, ref, &iter);
        gtk_tree_store_set(GTK_TREE_STORE(treemodel), &iter, 
            H_BUTTON_COLUMN, button,
            H_COLOR_COLUMN, color,
            -1);    
        gtk_tree_row_reference_free(ref);
    }
    g_slist_free(list);
    return;
}

static void
update_iconsXSD(GtkTreeModel *model, GtkTreeIter iter){
        GtkTreeIter parent;
        GtkTreeIter child;
        child = iter;
        // loop for all parents.
        while (gtk_tree_model_iter_parent(model, &parent, &child)){
            Tag_item_t *p_tag;
            gtk_tree_model_get(model, &parent,
		H_TAG_ITEM_COLUMN, &p_tag,
               -1); 
            set_row_iconXSD(model, &parent, p_tag);
            child = parent;
        }
}

static void
update_all_iconsXSD(GtkTreeModel *model, GtkTreeIter *parent){
    //fprintf(stderr, "update_all_iconsXSD...\n");
        GtkTreeIter iter;
        if (!parent) gtk_tree_model_get_iter_first(model, &iter);
        else iter = *parent;

        Tag_item_t *tag;
        gtk_tree_model_get(model, &iter,
            H_TAG_ITEM_COLUMN, &tag,
           -1); 
        // skip controllers
        if (choice_hash && g_hash_table_lookup(choice_hash, tag)) return;
        if (sequence_hash && g_hash_table_lookup(sequence_hash, tag)) return;
        if (all_hash && g_hash_table_lookup(all_hash, tag)) return;
        // proceed
        set_row_iconXSD(model, &iter, get_parent_tag(tag));
        GtkTreeIter child;
        if (gtk_tree_model_iter_children(model, &child, &iter)){
            do {
                update_all_iconsXSD(model, &child);
            } while (gtk_tree_model_iter_next(model, &child));
        }
}

static void
zap_row(Tag_t *Tag_p, Tag_item_t *src_tag, GtkTreeModel *treemodel, GtkTreeRowReference *sibling_ref){
    // zap it
    GtkTreeIter sibling, parent;
    Tag_item_t *parent_tag = get_parent_tag(src_tag);
    get_row_iter(treemodel, sibling_ref, &sibling);
    gtk_tree_model_iter_parent(treemodel, &parent, &sibling);
    GtkTreeRowReference *parent_ref = get_row_reference(treemodel, &parent);
    tag_item_remove(Tag_p, src_tag);
    gtk_tree_store_remove(GTK_TREE_STORE(treemodel), &sibling);
}

static gboolean 
add_row(Tag_t *Tag_p, Tag_item_t *src_tag, GtkTreeModel *treemodel, GtkTreeRowReference *sibling_ref){
    GHashTable *hash = get_associated_hash(treemodel, sibling_ref);
    if (is_ctl_full(Tag_p, src_tag, hash)) return FALSE;
    //Tag_item_t *new_tag = 
    Tag_item_t *parent_tag = get_parent_tag(src_tag);
    copy_tag(Tag_p, parent_tag, src_tag, treemodel, NULL, sibling_ref);
    return TRUE;
}


// XXX FIXME NEXT: Follow up with red/green ball functionality updates
static gboolean 
choice_callback(Tag_item_t *src_tag, 
        GtkTreeView *treeview, GtkTreeModel *treemodel, 
        GtkTreeIter *sibling, gint flag){
    if (!(flag & (IS_ALL_NESTED|IS_SEQUENCE_NESTED|IS_CHOICE_NESTED))) {
        NOOP( "IS_*_NESTED\n"); 
        return FALSE;
    }
    gboolean zap_it = flag & (IS_CHOICE_CHILD|IS_SEQUENCE_CHILD|IS_ALL_CHILD);

    GtkTreeRowReference *sibling_ref = get_row_reference(treemodel, sibling);

    Tag_t *Tag_p = g_object_get_data(G_OBJECT(treeview), "Tag_p");

    if (zap_it){
        gchar *name = g_strdup(get_tag_name(src_tag));
        Tag_item_t *tag = get_parent_tag(src_tag);
        zap_row(Tag_p, src_tag, treemodel, sibling_ref);
        ctl_visuals_remove(Tag_p, tag, name, treemodel, sibling_ref, flag);
        g_free(name);
    } else {
        add_row(Tag_p, src_tag, treemodel, sibling_ref);
        ctl_visuals_add(Tag_p, src_tag, treemodel, sibling_ref, flag);
    }

    update_all_iconsXSD(treemodel, NULL);

    gtk_tree_row_reference_free(sibling_ref);
    return TRUE;

}

//////////////////////////////////////////////////////////////////////////////////////////

static GHashTable *
create_Type_hash(Tag_t *Tag_p, const gchar *type_name){
    GHashTable *hash = NULL;
    // Find schema element or bust.
    Tag_item_t *schema_item = get_tag_item(Tag_p, NULL, "schema");
    if (!schema_item){
        fprintf(stderr, "tag_new_from_schema_tag(); No schema element\n");
        return NULL;
    }
    // Here we process all "$type_name" definitions
    // 1. Find all top level
    // 2. Hash types to access src tag for each one

    GSList *list = get_tag_item_list(Tag_p, schema_item, type_name);
    GSList *l = list;
    for (;l && l->data; l = l->next){
        Tag_item_t *src_item = l->data;
        Attribute_item_t *at = get_attribute(src_item, "name");
        if (!at){
            fprintf(stderr, "%s definition without a name (useless)\n", type_name);
            continue;
        }
        const gchar *name = get_attribute_value(at);
        if (!hash) hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
        g_hash_table_replace(hash, g_strdup(name), src_item);
        NOOP( "Definition: %s: \"%s\" hashed: %p --> %p\n", type_name,
                name, src_item, g_hash_table_lookup(hash, get_attribute_value(at)));
    }
    return hash;
}

static
Tag_t *tag_new_from_schema_tag(xmltree_t *xmltree_p, GError **error){

    if (!xmltree_p){
	g_error("build_treemodel(): xmltree_p cannot be NULL!");
    }
    Tag_t *Tag_p = xmltree_p->Tag_p;
    complexType_hash = create_Type_hash(Tag_p, "complexType");
    simpleType_hash = create_Type_hash(Tag_p, "simpleType");
    // We may proceed if schema does not define any complexType elements.
    Tag_item_t *schema_item = get_tag_item(Tag_p, NULL, "schema");
    if (!schema_item){
        fprintf(stderr, "tag_new_from_schema_tag(); No schema element\n");
        return NULL;
    }

    // Next, process schema elements.
    Tag_t *newTag_p =tag_new();

    // Here we populate defined elements.
    gboolean first=TRUE;
    GSList *list = get_tag_item_list(Tag_p, schema_item, "element");
    GSList *l = list;
    for (;l && l->data; l = l->next){
        Tag_item_t *tgt_item=NULL;
        Tag_item_t *src_item = l->data;

        tgt_item = process_element(xmltree_p, newTag_p, src_item, NULL);
    
        if (first){
            first = FALSE;
            if (!noneditable_hash) noneditable_hash = 
                g_hash_table_new(g_direct_hash, g_direct_equal);
            Attribute_item_t *a;

            a = attribute_item_add(newTag_p, tgt_item, 
                    "xmlns:xi", "http://www.w3.org/2001/XInclude", NULL);
            g_hash_table_replace(noneditable_hash, a, a);
            a = attribute_item_add(newTag_p, tgt_item, 
                    "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance", NULL);
            g_hash_table_replace(noneditable_hash, a, a);
            a = attribute_item_add(newTag_p, tgt_item, 
                    "xsi:noNamespaceSchemaLocation", xmltree_p->xml_path, NULL);
            g_hash_table_replace(noneditable_hash, a, a);
        }
    }

    

#if 0

#endif

    return newTag_p;
}

static void
xsd_edit_save (GtkButton * button, gpointer data) {
    NOOP( "xsd_edit_save...\n"); 
    GtkWidget *window = data;
    gboolean clean = TRUE;
    GtkTreeModel *model = g_object_get_data(G_OBJECT(window), "model");
    gtk_tree_model_foreach(model, check_clean, &clean);
    if (!clean){
	return;
    } 
    // save xml.
    Tag_t *Tag_p = g_object_get_data(G_OBJECT(window), "xml_Tag_p");
    if (!Tag_p) return;
    //xmltree_t *xmltree_p = g_object_get_data(G_OBJECT(window), "xmltree_p");
    // FIXME: enter as parameter OK
   // gchar *file = g_build_filename(xmltree_p->xml_path, NULL);
    gchar *file = g_build_filename("save_test.xml", NULL);
    tag_write_file(Tag_p, file, FALSE);
    rfm_confirm(NULL, GTK_MESSAGE_INFO, file, NULL, NULL);
    g_free(file);
    //gtk_widget_destroy(window);

}

static gboolean show_hidden=FALSE;
static gboolean
show_visible_row(GtkTreeModel *f_model, GtkTreeIter *f_iter, void *data){
    if (show_hidden) return TRUE;
    //GtkTreeIter iter;
    Tag_item_t *tag;
    Attribute_item_t *item = NULL;
    gtk_tree_model_get(f_model, f_iter, 
            H_ATTRIBUTE_ITEM_COLUMN, &item, 
            H_TAG_ITEM_COLUMN, &tag,
            -1);   
        //if (strstr(get_tag_name(tag), "Specify")) 
          //  fprintf(stderr, "show_visible_row: %s\n", get_tag_name(tag));
    if (item && attribute_get_hidden(item)) return FALSE; 

    // also hide all children in all/sequence/choice controllers
    if (is_nested_in_ctl(tag)) return FALSE;
    return TRUE;
}






