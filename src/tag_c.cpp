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
 * along with this program; .
 */


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <locale.h>
#include <glib.h>
#include <gtk/gtk.h>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlschemas.h>
#include <libxml/xmlmemory.h>
#include <libxml/xinclude.h>
#include <libxml/xmlwriter.h>
#include <libxml/encoding.h>

# include <libintl.h>
# define _(String) dgettext(GETTEXT_PACKAGE,String)
# define N_(String)  String

#ifndef NOOP
 #define NOOP(...)   G_STMT_START{ (void)0; }G_STMT_END
#endif


#include "tagfile.h"
#include "tagfile.i"
// Public
Tag_t * 
tag_new (void){
    tag_struct_t *tag_struct_p = mk_tag_struct(NULL);
    return (void *)tag_struct_p;
}

gboolean
tag_item_set_string(Tag_item_t *item, const gchar *string){
    if (!item) return FALSE;
    if (!string) return FALSE;
    tag_item_t *tag_p = (tag_item_t *)item;
    g_free(tag_p->string);
    tag_p->string = g_strdup(string);
    return TRUE;
}

const gchar *
tag_item_get_string(Tag_item_t *item){
    if (!item) return FALSE;
    tag_item_t *tag_p = (tag_item_t *)item;
    return (tag_p->string);
}

gboolean
tag_load_schema(Tag_t * Tag_p, gchar *filename, GError **in_error){
    if (!filename) return FALSE;
    tag_struct_t *tag_struct_p = (tag_struct_t *) Tag_p;
    gchar *schema_file=NULL;
    if (!g_file_test(filename, G_FILE_TEST_EXISTS)
            && !g_path_is_absolute(filename)){
	gchar *dirname = g_strdup(filename);
	if (strchr(dirname, G_DIR_SEPARATOR)) *strrchr(dirname, G_DIR_SEPARATOR) = 0;
	gchar *g = g_strdup_printf("%s%c%s", dirname, 
		G_DIR_SEPARATOR, filename);
	g_free(dirname);
	schema_file = g;
    } else {
	schema_file = g_strdup(filename);
    }

    if(tag_struct_p->schema_doc) {
	// free previous schema, if any
        xmlFreeDoc(tag_struct_p->schema_doc);  
    }
    tag_struct_p->schema_doc = xmlReadFile(schema_file, NULL, XML_PARSE_NONET);
    if (tag_struct_p->schema_doc == NULL) return FALSE;
    return TRUE;
}

Tag_t * 
tag_new_from_file (const gchar *filename, GError **in_error)
{
    GError *error=NULL;
    tag_struct_t *tag_struct_p = build_tag_struct(filename, &error);

    if (error) {
	if (in_error) *in_error = error;
	else g_error_free(error);
	return NULL;
    }

    return (void *)tag_struct_p;
}

Tag_t * 
tag_new_from_schema_file (const gchar *filename, GError **in_error)
{
    Tag_t *Tag_p = tag_new_from_file(filename, in_error);
    if (!Tag_p) return Tag_p;
    tag_struct_t *tag_struct_p = (tag_struct_t *)Tag_p;
    tag_struct_p->is_schema = TRUE;
    return Tag_p;
}


void 
tag_free (Tag_t *Tag_p)
{
    if (!Tag_p) return;
    free_tag_struct((tag_struct_t *) Tag_p);
    return;
}
#if 0
static gboolean
t_w_f(Tag_t *Tag_p, const gchar *output_file, gboolean validate_only){
    tag_struct_t *tag_struct_p = (tag_struct_t *)Tag_p;
    if (!tag_struct_p) {
	NOOP(stderr, "!Tag_p\n");
	return FALSE;
    }
    if (!tag_struct_p->tag_list){
	NOOP("tag_write_file 0x%x\n", 
		GPOINTER_TO_INT(tag_struct_p->tag_list));
	NOOP(stderr, "!tag_struct_p->tag_list\n");
	return FALSE;
    }
    xmlTextWriterPtr writer;
    // Create doc in memory
    // If the document has an associated schema, write it out first.
    // This, of course, if the schema path is not an absolute path.
    gchar *output_schema = NULL;
    if (tag_struct_p->schema_tag_struct_p &&
	    !g_path_is_absolute(tag_struct_p->schema)) {
	output_schema = g_strconcat(output_file, ".xsd", NULL);
	writer = writedoc(tag_struct_p->schema_tag_struct_p, output_schema, NULL);
	NOOP("writing local schema: %s\n", output_schema);
	xmlSaveFileEnc((const gchar *)(tag_struct_p->schema_tag_struct_p->doc->URL),
		tag_struct_p->schema_tag_struct_p->doc, "UTF-8");
	xmlFreeTextWriter(writer);
    }

    // This creates the actual xml doc and file
    // We just need the doc to validate, and we don't
    // need the doc if we write file and forget.
    if (output_schema){
	gchar *g= g_path_get_basename(output_schema);
	g_free(output_schema);
	output_schema = g;
    }
    writer = writedoc(tag_struct_p, output_file, output_schema);
    if (writer){
	gboolean retval = FALSE;
	if (!validate_only) {
	    xmlSaveFileEnc((const gchar *)(tag_struct_p->doc->URL), tag_struct_p->doc, "UTF-8");
	}
	else {
	    NOOP("Validating %s\n", output_file);
	    gchar *original_file = tag_struct_p->file;
	    gchar *original_schema = tag_struct_p->schema;
	    if (output_schema){
		tag_struct_p->file = g_strdup(output_file);
		tag_struct_p->schema = g_strdup(output_schema);
		retval = validate_xml(tag_struct_p);
		g_free(tag_struct_p->schema);
		g_free(tag_struct_p->file);
		tag_struct_p->file=original_file;
		tag_struct_p->schema = original_schema;
	    } else {
		retval = validate_xml(tag_struct_p);
	    }
	    if (retval) NOOP("%s validates\n", output_file);
	    else NOOP("%s does not validate\n", output_file);
	}

	xmlFreeTextWriter(writer);

	if (validate_only) return retval;
	else return TRUE;
    }
	NOOP(stderr, "write failed\n");
  
    return FALSE;
    // XXX pending:
    // 1. create backup files
    // 2. writeout files
    // 3. writeout schema OK
}
#endif

gboolean
tag_validate(Tag_t *Tag_p){
    return validate_xml((tag_struct_t *)Tag_p);
}

gboolean
tag_write_file(Tag_t *Tag_p, const gchar *output_file, gboolean split_includes){
    // XXX split_includes is not used yet...
    return write_file((tag_struct_t *)Tag_p, output_file);
}

gboolean
set_tag_item_user_data(Tag_item_t *item, gpointer user_data){
    if (!item) return FALSE;
    tag_item_t *tag_p = (tag_item_t *)item;
    tag_p->user_data = user_data;
    return TRUE;
}

gpointer
get_tag_item_user_data(Tag_item_t *item){
    if (!item) return NULL;
    tag_item_t *tag_p = (tag_item_t *)item;
    return tag_p->user_data;
}

Attribute_item_t *attribute_item_add(Tag_t *Tag_p, Tag_item_t *parent_tag, 
	const gchar *name,
	const gchar *value,
	const gchar *namespace
	){
    const xmlNs *ns = (const xmlNs *) namespace;
    
    if (!name || !strlen(name)){
	DBG("attribute_item_add(): !name || !strlen(name)  (%s, %s)\n", name, value);
	return NULL;
    }
    if (!value) value = "";
    tag_item_t *tag_p = (tag_item_t *)parent_tag;
    attribute_t * attribute_p = mk_attribute(name, value, ns, tag_p);

   
    //NOOP(stderr, "replacing attribute: %s with %s\n",
	//    attribute_p->name, value);
    g_hash_table_replace(tag_p->attribute_hash,
		    attribute_p->name, attribute_p);
    return (Attribute_item_t *)attribute_p;
}
/* no need...
void attribute_set_pattern(Attribute_item_t *item, gchar *pattern){
    if (!item) return ;
    attribute_t *a_p = (attribute_t *)item;
    g_free(a_p->pattern);
    if (pattern) a_p->pattern = g_strdup(pattern);
    else  a_p->pattern = NULL;
    return;
}

*/
void attribute_set_hidden(Attribute_item_t *item, gboolean state){
    if (!item) return ;
    attribute_t *a_p = (attribute_t *)item;
    a_p->hidden = state;
}

gboolean attribute_get_hidden(Attribute_item_t *item){
    if (!item) return TRUE;
    attribute_t *a_p = (attribute_t *)item;
    return a_p->hidden;
}

void tag_set_hidden(Tag_item_t *item, gboolean state){
    if (!item) return ;
    tag_item_t *t_p = (tag_item_t *)item;
    t_p->hidden = state;
}

gboolean tag_get_hidden(Tag_item_t *item){
    if (!item) return TRUE;
    tag_item_t *t_p = (tag_item_t *)item;
    return t_p->hidden;
}

Tag_item_t *get_tag_item(Tag_t *Tag_p, Tag_item_t *parent_tag, const gchar *tag_name){
    if (!Tag_p || !tag_name) return NULL;
    GSList *list = get_tag_item_list(Tag_p, parent_tag, tag_name); 
    if (list){
	Tag_item_t *item = list->data;
	g_slist_free(list);
	return item;
    }
    return NULL;
}

Tag_item_t *get_parent_tag(Tag_item_t *tag){
    if (!tag) return NULL;
    tag_item_t *tag_p = (tag_item_t *)tag;
    return (Tag_item_t *) tag_p->parent_tag;
}


Tag_item_t *tag_item_add(Tag_t *Tag_p, Tag_item_t *parent_tag, const gchar *tag_name){
    if (!Tag_p) {DBG("Tag_p is NULL\n");	return NULL; }
    tag_struct_t *tag_struct_p = (tag_struct_t *)Tag_p;
    GSList **list;
    tag_item_t *tag_p = (tag_item_t *)parent_tag;
    if (parent_tag == NULL) {list = &(tag_struct_p->tag_list);}
    else { list = &(tag_p->tag_list);}
    tag_item_t *new_tag_p = mk_tag(tag_name);
    *list = g_slist_append(*list, new_tag_p);
    new_tag_p->parent_tag = (void *)parent_tag;
    //fprintf(stderr, "tag =%p (%s) --> parent=%p (%s)\n", new_tag_p, tag_name, parent_tag, get_tag_name(parent_tag));
    //NOOP(stderr, "list=0x%x): %s\n", GPOINTER_TO_INT(*list), tag_name);
    return (Tag_item_t *) new_tag_p;
}

void tag_item_remove(Tag_t *Tag_p, Tag_item_t *tag){
    if (!Tag_p || !tag) return;
    tag_struct_t *tag_struct_p = (tag_struct_t *)Tag_p;
    Tag_item_t *parent_tag = get_parent_tag(tag); 
    tag_item_t *ptag = (tag_item_t *)parent_tag;
    GSList **list;
    if (parent_tag == NULL) {list = &(tag_struct_p->tag_list);}
    else { list = &(ptag->tag_list);}
    *list = g_slist_remove(*list, tag);
    free_tag((tag_item_t *)tag);
}

GSList *get_tag_item_list(Tag_t *Tag_p, Tag_item_t *parent_tag, const gchar *tag_name){
    if (!Tag_p) {
	DBG("Tag_p is NULL\n");
	return NULL;
    }
    //NOOP(stderr, "getting list for %s\n", tag_name);
    tag_item_t *tag_p = (tag_item_t *)parent_tag;
    tag_struct_t *tag_struct_p = (tag_struct_t *)Tag_p;
    GSList *list;
    if (parent_tag == NULL) {list = tag_struct_p->tag_list;}
    else { list = tag_p->tag_list;}
    if (!list) return NULL;
    GSList *retlist = NULL;
    for (;list && list->data; list = list->next){
	tag_item_t *tag_p = list->data;
	if (tag_name == NULL) {
	    retlist = g_slist_append(retlist, tag_p);
	} else if (strcasecmp(tag_name, tag_p->name)==0){
	    retlist = g_slist_append(retlist, tag_p);
	}
    }
    return retlist;
}

static
void filter_f (gpointer key, gpointer value, gpointer data){
    void **arg = data;
    GSList **outlist = arg[0];   
    const gchar *attribute_name = arg[1];  
    if (strcmp((gchar *)key, attribute_name)==0){
	//attribute_t *attribute_p = value;
	*outlist =g_slist_prepend(*outlist, value);
    }
}

static
void
get_att_list(GSList **outlist, GSList *inlist, const gchar *attribute_name){
    GSList *list = inlist;
    for(; list && list->data; list = list->next){
	tag_item_t *tag_p = list->data;
	if (tag_p->attribute_hash){
	    void *arg[]={(void *)outlist, (void *)attribute_name};
	    g_hash_table_foreach(tag_p->attribute_hash, filter_f, arg);
	}
	if (tag_p->tag_list) {
	    get_att_list(outlist, tag_p->tag_list, attribute_name);
	}
    }
    return;
}

const gchar *
get_tag_name(Tag_item_t *item){
    if (!item) return NULL;
    //NOOP(stderr, "name.... %s\n",  ((tag_item_t *)(item))->name);
    return ((tag_item_t *)(item))->name;
}
    
  
gboolean
tag_item_has_children(Tag_item_t *item){
    if (!item) return FALSE;
    tag_item_t *tag_p = (tag_item_t *) item;
    if (tag_p->tag_list) return TRUE;
    else return FALSE;
}

/////

Attribute_item_t *get_attribute(Tag_item_t *parent_tag, const gchar *attribute_name){
    if (!parent_tag) return NULL;
    tag_item_t *tag_p = (tag_item_t *)parent_tag;
    GHashTable *hash = tag_p->attribute_hash;
    if (!hash) return NULL;
    Attribute_item_t *Attribute_item_p = g_hash_table_lookup(hash, attribute_name);
    return Attribute_item_p;
}

GSList *get_attribute_item_list(Tag_item_t *parent_tag){
    if (!parent_tag) return NULL;
    tag_item_t *tag_p = (tag_item_t *)parent_tag;
    GHashTable *hash = tag_p->attribute_hash;
    if (!hash) return NULL;
    GSList *retlist = NULL;
    void *arg[2]={(void *)&retlist, NULL};
    g_hash_table_foreach(hash, hash2list, arg);
    return retlist;
}

GSList *get_full_attribute_list(Tag_t *Tag_p, Tag_item_t *parent_tag, const gchar *attribute_name){
    //NOOP(stderr, "get_full_attribute_list for %s\n", attribute_name);
    if (!attribute_name || strlen(attribute_name)==0) return NULL;
    if (!parent_tag && !Tag_p) return NULL;
    GSList *outlist=NULL;
    tag_item_t *tag_p = (tag_item_t *)parent_tag;
    tag_struct_t *tag_struct_p =(tag_struct_t *)Tag_p;

    GSList *in_list;
    if (tag_p) {
	in_list = tag_p->tag_list;
    } else {
	in_list = tag_struct_p->tag_list;
    }
    get_att_list(&outlist, in_list, attribute_name);
    outlist = g_slist_reverse(outlist);
    return (outlist);
}

Tag_item_t *
get_attribute_parent(Attribute_item_t *item){
    if (!item) return NULL;
    attribute_t *a_p = (attribute_t *)item;
    return (Tag_item_t *)a_p->parent_tag;
}

const gchar *
get_attribute_name(Attribute_item_t *item){
    if (!item) return NULL;
    attribute_t *a_p = (attribute_t *)item;
    return a_p->name;
}

const gchar *
get_attribute_prefix(Attribute_item_t *item){
    if (!item) return NULL;
    attribute_t *a_p = (attribute_t *)item;
    return a_p->prefix;
}
    
const gchar *
get_attribute_value(Attribute_item_t *item){
    if (!item) return NULL;
    attribute_t *a_p = (attribute_t *)item;
    return a_p->value;
}
     
gboolean
set_attribute_value(Attribute_item_t *item, const gchar *value){
    if (!item || !value) return FALSE;
    attribute_t *a_p = (attribute_t *)item;
    g_free(a_p->value);
    a_p->value = g_strdup(value);
    return TRUE;
}
   
gchar **
get_attribute_value_list(Attribute_item_t *item, gsize *length){
    if (!item) return NULL;
    attribute_t *a_p = (attribute_t *)item;
    gchar *full_string = a_p->value;
    if (!full_string || !strlen(full_string)) return NULL;

    gchar *p = g_strdup(full_string);
    // Change tabs, LF and CR to the delimiter (whitespace now).
    for (;p && *p; p++) {if (*p == 9 || *p == 10 || *p == 13) {*p = ' ';}}
    gchar **strings = g_strsplit(full_string, " ", -1);
    gint i = 0;
    for (; strings[i]; i++);
    *length = i;
    g_free(p);
    return strings;
}

gint 
get_validation_status(Tag_t *Tag_p){
    tag_struct_t *tag_struct_p = (tag_struct_t *)Tag_p;
    return tag_struct_p->validated;
}

#if 0
Tag_t *
tag_get_schema(Tag_t *Tag_p){
    tag_struct_t *tag_struct_p = (tag_struct_t *)Tag_p;
    return (Tag_t *) (tag_struct_p->schema_tag_struct_p);
}
#endif
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

#ifdef COPYRIGHT_INFORMATION
#include "gplv3.h"
#endif
#include "debug.h"
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

#if 0
//xmllint --schema
#include <libxml/parser.h>
#include <libxml/xmlschemas.h>

int is_valid(const xmlDocPtr doc, const char *schema_filename)
{
    xmlDocPtr schema_doc = xmlReadFile(schema_filename, NULL, XML_PARSE_NONET);
    if (schema_doc == NULL) {
        /* the schema cannot be loaded or is not well-formed */
        return -1;
    }
    xmlSchemaParserCtxtPtr parser_ctxt = xmlSchemaNewDocParserCtxt(schema_doc);
    if (parser_ctxt == NULL) {
        /* unable to create a parser context for the schema */
        xmlFreeDoc(schema_doc);
        return -2;
    }
    xmlSchemaPtr schema = xmlSchemaParse(parser_ctxt);
    if (schema == NULL) {
        /* the schema itself is not valid */
        xmlSchemaFreeParserCtxt(parser_ctxt);
        xmlFreeDoc(schema_doc);
        return -3;
    }
    xmlSchemaValidCtxtPtr valid_ctxt = xmlSchemaNewValidCtxt(schema);
    if (valid_ctxt == NULL) {
        /* unable to create a validation context for the schema */
        xmlSchemaFree(schema);
        xmlSchemaFreeParserCtxt(parser_ctxt);
        xmlFreeDoc(schema_doc);
        return -4; 
    }
    int is_valid = (xmlSchemaValidateDoc(valid_ctxt, doc) == 0);
    xmlSchemaFreeValidCtxt(valid_ctxt);
    xmlSchemaFree(schema);
    xmlSchemaFreeParserCtxt(parser_ctxt);
    xmlFreeDoc(schema_doc);
    /* force the return value to be non-negative on success */
    return is_valid ? 1 : 0;
}
#endif

// PRIVATE

typedef struct tag_item_t {
    GtkTreePath *path;
    gchar *name;
    gchar *string;
    gboolean hidden;
    GHashTable *attribute_hash;
    GSList *tag_list;
    gpointer *user_data;
    void *parent_tag;
} tag_item_t;

typedef struct attribute_t {
    gchar *name;
    gchar *value;
    gchar *prefix;
    gboolean hidden;
    gchar *pattern;
    tag_item_t *parent_tag;
} attribute_t;

typedef struct tag_struct_t {
    gchar *file;
    xmlDocPtr doc;
    gboolean validated; // Three state: {yes, no, unknown}/{1,0,-1}
    gchar *schema_file;
    xmlDocPtr schema_doc;
    xmlTextWriterPtr writer;    
    GSList *tag_list;
    gboolean is_schema;
} tag_struct_t;


static gboolean validate_xml(tag_struct_t *tag_struct_p);

static xmlTextWriterPtr 
writedoc(tag_struct_t *tag_struct_p, const gchar *url);



static attribute_t *
mk_attribute(const gchar *name, const gchar *value, const xmlNs *ns, tag_item_t *parent_tag){
    attribute_t *attribute_p = (attribute_t *)malloc(sizeof(attribute_t)); 
    if (!attribute_p) g_error("malloc:%s", strerror(errno));
    memset(attribute_p, 0, sizeof(attribute_t));
    attribute_p->name = g_strdup(name);
    attribute_p->value = g_strdup(value);
    if (ns && ns->prefix) attribute_p->prefix = g_strdup((gchar *)(ns->prefix));
    attribute_p->parent_tag = parent_tag;
    return attribute_p;
}

// may crash (on hash key collisions?)
static void free_attribute(void *data){
    attribute_t *attribute_p = data;
    if (!attribute_p) return;
    // attribute_p->name is also the key!!
    g_free(attribute_p->name);
    g_free(attribute_p->value);
    g_free(attribute_p->prefix);
    g_free(attribute_p);
}

static tag_item_t *
mk_tag(const gchar *name){
    if (!name) return NULL;
    tag_item_t *tag_p = (tag_item_t *)malloc(sizeof(tag_item_t)); 
    if (!tag_p) g_error("malloc:%s", strerror(errno));
    memset(tag_p, 0, sizeof(tag_item_t));
    tag_p->name = g_strdup(name);
    // Do *not* g_free the key. The key is contained within the attribute structure!
    tag_p->attribute_hash = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, free_attribute);
    return tag_p;
}

static void free_tag(tag_item_t *tag_p){
    if (!tag_p) return;

    g_hash_table_destroy(tag_p->attribute_hash);

    GSList *list=tag_p->tag_list;
    for (;list && list->data; list=list->next){
	tag_item_t *list_tag_p = list->data;
	free_tag(list_tag_p);
    }
    g_slist_free(tag_p->tag_list);

    g_free(tag_p->string);
    g_free(tag_p->name);
    g_free(tag_p);
}

static tag_struct_t *
mk_tag_struct(const gchar *filename){
    /*if (!filename){
	DBG("mk_tag_struct_p(): filename is NULL.\n");
	return NULL;
    }*/
    tag_struct_t *tag_struct_p = (tag_struct_t *)malloc(sizeof(tag_struct_t)); 
    if (!tag_struct_p) g_error("malloc:%s", strerror(errno));
    memset(tag_struct_p, 0, sizeof(tag_struct_t));
    if (filename) tag_struct_p->file = g_strdup(filename);
    tag_struct_p->validated = -1; // unknown status.
    return tag_struct_p;
}

static void free_tag_struct(tag_struct_t *tag_struct_p){
    if (!tag_struct_p) return;
    // This goes down with the writer (that's screwy)
    // if (tag_struct_p->doc) xmlFreeDoc (tag_struct_p->doc);
    if (tag_struct_p->schema_doc) xmlFreeDoc (tag_struct_p->schema_doc);
    // recurse:
    GSList *list=tag_struct_p->tag_list;
    for (;list && list->data; list=list->next){
	tag_item_t *list_tag_p = list->data;
	free_tag(list_tag_p);
    }
    g_slist_free(tag_struct_p->tag_list);
    g_free(tag_struct_p->file);
    g_free(tag_struct_p->schema_file);
    g_free(tag_struct_p);
}


static gchar *
get_value(const gchar *xmlvalue){
    if (!xmlvalue) return NULL;
    gchar *value=g_strdup(xmlvalue);
    g_strstrip(value);
    if (strlen(value)==0 || strcmp(value, "\n")==0){
	g_free(value);
	return NULL;
    }
    return value;
}


static GSList * 
get_tag_list(tag_struct_t *tag_struct_p, const xmlNodePtr in_node,  tag_item_t *top_tag_p, gint level){
    gint i;
    if (!in_node) return NULL;
    

    xmlNodePtr node;
    GSList *list=NULL;

    tag_item_t *tag_p = NULL;


    for(node = in_node; node && node->name; node = node->next) {
	if (strcasecmp((gchar *)(node->name), "include")==0){
	    continue;
	}
	if (strcasecmp((gchar *)(node->name), "comment")==0) continue;
	

	// Any marked up text?.
	if (strcmp((gchar *)(node->name), "text")==0) {
	    // We have a text attribute here...
	    gchar *text = get_value((const gchar *)(node->content));
	    if (text) {
		NOOP( "yes text attribute\n");
		if (!top_tag_p) g_error("!top_tag_p");
		attribute_t *attribute_p = mk_attribute("text", text, NULL, top_tag_p);
		g_hash_table_replace(top_tag_p->attribute_hash,
		    attribute_p->name, attribute_p);
		for (i=0; i<level+1; i++) NOOP(" ");
		    NOOP("text--> %s\n", text);
		
	    } else {
		NOOP("no text attribute\n");
	    }
	    continue;
	}
	for (i=0; i<level; i++) NOOP(" "); NOOP("%s\n", (gchar *)(node->name));
	tag_p = mk_tag((gchar *)node->name);
        if (!tag_p) return NULL;
        tag_p->parent_tag = top_tag_p;
	list = g_slist_append(list, tag_p);
	//NOOP( "%d) 0x%x %s\n", 
	//	level, GPOINTER_TO_INT(tag_p), node->name);
	// Does the node have children? 
	if (node->children) {
	    // recurse
	    tag_p->tag_list = 
		get_tag_list(tag_struct_p, (const xmlNodePtr)node->children, tag_p, level+1);

	}
	if (strcasecmp((gchar *)(node->name), "schema")==0){
	    NOOP("node->schema_tag\n");
	}
	// Name space specifics
	if (node->nsDef){
	    xmlNs *nsDef = node->nsDef;
	    for (;nsDef && nsDef->href; nsDef = nsDef->next){
		xmlNs ns={.prefix = (const xmlChar *)"xmlns"};
		 attribute_t *attribute_p;
		if (nsDef->prefix) {
		  attribute_p =
		    mk_attribute(((gchar *)(nsDef->prefix)), ((gchar *)(nsDef->href)), &ns, tag_p);
		} else {
		  attribute_p =
		    mk_attribute("xmlns", ((gchar *)(nsDef->href)), NULL, tag_p);
		}
		g_hash_table_replace(tag_p->attribute_hash,
		    attribute_p->name, attribute_p);

	    }
	}

	// Does the node have properties? (or attributes)
	// This is likewise content...
	if (node->properties) {
	    xmlAttrPtr properties = node->properties;
	    for (; properties; properties = properties->next){
		if (!properties->name) {
		    DBG("// This should never happen: attribute without a name.\n");
		    continue;
		}
		// This is nerdy, the value is within a node called "children"...
		xmlNodePtr children = properties->children;
		gchar *value = NULL;
		if (children) {
		    value = get_value((const gchar *)(children->content));
		} else {
		    value = g_strdup("");
		}
		attribute_t *attribute_p =
		    mk_attribute(((gchar *)(properties->name)), value, properties->ns, tag_p);
		g_hash_table_replace(tag_p->attribute_hash,
		    attribute_p->name, attribute_p);
		if (strcasecmp(attribute_p->name, "noNamespaceSchemaLocation")==0){
		    if (tag_struct_p->schema_file){
			DBG("More than one \"noNamespaceSchemaLocation\"\n");
		    } else {
			tag_struct_p->schema_file = g_strdup(attribute_p->value);
		    }
		}
	    }
	}
    }
    return list;
}


static tag_struct_t *
build_tag_struct (const gchar *file, GError **error){
    if (!file){
	g_error("build_tag_p(): data cannot be NULL!");
    }
    GQuark  quark = g_quark_from_string ("TagT");
    // Check XML file
    if(access (file, R_OK) != 0) {
        DBG ("build_tag_struct() access(%s, R_OK)!=0 (%s)\n", file, strerror(errno));
	if (error) {
	    *error = g_error_new(quark, 0x01, "access(%s, R_OK)!=0 (%s)\n", file, strerror(errno));
	}
        return NULL;
    }
    tag_struct_t *tag_struct_p = mk_tag_struct(file);

    xmlKeepBlanksDefault (0);
    // Parse XML file
    if((tag_struct_p->doc = xmlParseFile (tag_struct_p->file)) == NULL) {
        DBG ("xmlParseFile(): unable to parse %s \n", tag_struct_p->file);
	if (error) {
	    *error = g_error_new(quark, 0x02,"xmlParseFile(): unable to parse %s \n", file);
	}
	free_tag_struct(tag_struct_p);
        return NULL;
    } 

    /* Now parse the xml tree */
    xmlNodePtr node = xmlDocGetRootElement (tag_struct_p->doc);
    if (node == NULL) {
        DBG ("xmlDocGetRootElement (): empty document\n");
	if (error) {
	    *error = g_error_new(quark, 0x03, "xmlDocGetRootElement (): empty document");
	}
	free_tag_struct(tag_struct_p);
	return NULL;
    }


    if (xmlXIncludeProcess (tag_struct_p->doc) < 0) {
	DBG("XInclude processing failed\n");
    }
    tag_struct_p->validated = -1;
    //tag_struct_p->validated = validate_xml(tag_struct_p);
  //////////////////////////

    /* Now parse the xml tree, adding elements to the tag_p. */
    tag_struct_p->tag_list = get_tag_list(tag_struct_p, (const xmlNodePtr)node, NULL, 0);
    // Clean up XML allocated memory
    xmlFreeDoc (tag_struct_p->doc);
    tag_struct_p->doc = NULL;
    
    return tag_struct_p;
}

#if 0
static
void dump_a(gpointer key, gpointer value, gpointer user_data){
    attribute_t *a_p = value;
    TRACE( "hash: %s = %s\n", a_p->name, a_p->value);
}
#endif

static
void hash2list(gpointer key, gpointer value, gpointer data){
    void **arg = data;
    GSList **list = arg[0];
    const gchar *attribute_name = arg[1];
    attribute_t *a_p = value;
    if (!attribute_name){
	*list = g_slist_append(*list, a_p);
    } else if (strcasecmp(attribute_name, a_p->name)==0){
	*list = g_slist_append(*list, a_p);
    }
}
static gboolean
create_doc(tag_struct_t *tag_struct_p, const gchar *output_file){
    if (!tag_struct_p) return FALSE;
    if (!output_file) output_file = "memory";
    xmlTextWriterPtr writer = writedoc(tag_struct_p, output_file);
    xmlFreeTextWriter(writer);
    return TRUE;
}


    // Create doc in memory
    //
#if 0
    // If the document has an associated schema, write it out first.
    // This, of course, if the schema path is not an absolute path.
    gchar *output_schema = NULL;
    if (tag_struct_p->schema_tag_struct_p &&
	    !g_path_is_absolute(tag_struct_p->schema)) {
	output_schema = g_strconcat(output_file, ".xsd", NULL);
	writer = writedoc(tag_struct_p->schema_tag_struct_p, output_schema, NULL);
	NOOP("writing local schema: %s\n", output_schema);
	xmlSaveFileEnc((const gchar *)(tag_struct_p->schema_tag_struct_p->doc->URL),
		tag_struct_p->schema_tag_struct_p->doc, "UTF-8");
	xmlFreeTextWriter(writer);
    }

    // This creates the actual xml doc and file
    // We just need the doc to validate, and we don't
    // need the doc if we write file and forget.
    if (output_schema){
	gchar *g= g_path_get_basename(output_schema);
	g_free(output_schema);
	output_schema = g;
    }

    if (writer){
	gboolean retval = FALSE;
	if (!validate_only) {
	    xmlSaveFileEnc((const gchar *)(tag_struct_p->doc->URL), tag_struct_p->doc, "UTF-8");
	}
	else {
	    NOOP("Validating %s\n", output_file);
	    gchar *original_file = tag_struct_p->file;
	    gchar *original_schema = tag_struct_p->schema;
	    if (output_schema){
		tag_struct_p->file = g_strdup(output_file);
		tag_struct_p->schema = g_strdup(output_schema);
		retval = validate_xml(tag_struct_p);
		g_free(tag_struct_p->schema);
		g_free(tag_struct_p->file);
		tag_struct_p->file=original_file;
		tag_struct_p->schema = original_schema;
	    } else {
		retval = validate_xml(tag_struct_p);
	    }
	    if (retval) NOOP("%s validates\n", output_file);
	    else NOOP("%s does not validate\n", output_file);
	}

	xmlFreeTextWriter(writer);

	if (validate_only) return retval;
	else return TRUE;
    }
	NOOP(stderr, "write failed\n");
  
}
#endif

static gboolean
write_file(tag_struct_t *tag_struct_p, const gchar *file){
    if (!tag_struct_p){
	DBG("tag_struct_p==NULL\n");
	return FALSE;
    }
    if (!create_doc(tag_struct_p, file)){
	DBG("cannot create xmldoc\n");
	return FALSE;
    }
    xmlSaveFileEnc((const gchar *)(tag_struct_p->doc->URL), tag_struct_p->doc, "UTF-8");
    return TRUE;
}

static gboolean validate_xml(tag_struct_t *tag_struct_p){
    if (!tag_struct_p){
	DBG("tag_struct_p==NULL\n");
	return FALSE;
    }
    if (tag_struct_p->is_schema) return TRUE; // Schemas should not have a schema...

    if (!tag_struct_p->schema_doc){
	DBG("tag_struct_p->schema_doc==NULL\n");
	return FALSE;
    }
    NOOP("validating xml %s ...\n", tag_struct_p->file);

    if (!create_doc(tag_struct_p, NULL)){
	DBG("cannot create xmldoc\n");
	return FALSE;
    }
    
    xmlSchemaParserCtxtPtr validationCtxt = xmlSchemaNewDocParserCtxt(tag_struct_p->schema_doc);
    //xmlSchemaValidCtxtPtr validationCtxt = xmlSchemaNewValidCtxt (NULL);
    if (validationCtxt == NULL) {
	// Cannot create a validation context for the schema
	DBG("Failed to create the validation context.\n");
	return FALSE;
    }

    xmlSchemaPtr schema = xmlSchemaParse(validationCtxt);
    if (schema == NULL) {
        /* the schema itself is not valid */
	DBG("schema itself is not valid\n");
        xmlSchemaFreeParserCtxt(validationCtxt);
        return FALSE;
    }

    xmlSchemaValidCtxtPtr valid_ctxt = xmlSchemaNewValidCtxt(schema);
    if (valid_ctxt == NULL) {
        DBG("unable to create a validation context for the schema\n");
        xmlSchemaFree(schema);
        xmlSchemaFreeParserCtxt(validationCtxt);
        return FALSE;
    }

    gboolean result = (xmlSchemaValidateDoc(valid_ctxt, tag_struct_p->doc) == 0);
    xmlSchemaFreeValidCtxt(valid_ctxt);
    xmlSchemaFree(schema);
    xmlSchemaFreeParserCtxt(validationCtxt);

    NOOP("XML document (%s) is %s valid according to xsd specification %s\n",
		tag_struct_p->file,
		(result)?"":"NOT",
		(tag_struct_p->schema_file)?tag_struct_p->schema_file: "");

    return result;
}

#if 0
typedef struct link_list_t{
    const gchar *name;
    GSList *attribute_list;
} link_list_t;

static link_list_t *
mk_link_list_p(const gchar *name, const gchar *value){
	link_list_t *link_list_p = (link_list_t *)malloc(sizeof(link_list_t));
	if (!link_list_p) g_error("malloc: %s\n", strerror(errno));
	memset(link_list_p, 0, sizeof(link_list_t));
	link_list_p->name = name;

	return link_list_p;
}
#endif

static GSList *
get_link_list(tag_struct_t *tag_struct_p, GSList *tag_list, GSList *link_list){
    
    if (!tag_struct_p) return FALSE;
    
    GSList *l_list = link_list;
    GSList *list;
    if (!tag_list) list = tag_struct_p->tag_list;
    else list = tag_list;

    for (;list; list=list->next){
	if (!list->data) continue;
	tag_item_t *tag_p = list->data;
        // skip hidden tags
        if (tag_p->hidden) continue;
	//if (strcasecmp(tag_p->name, "include")==0 ) continue;
    	//NOOP( "Start element: %s\n", tag_p->name);
	l_list = g_slist_append(l_list, tag_p);

	// recurse:
	if (tag_p->tag_list){
	    l_list = get_link_list(tag_struct_p, tag_p->tag_list, l_list);
	}

	l_list = g_slist_append(l_list, GINT_TO_POINTER(-1));
    }

    return l_list;
}


static xmlTextWriterPtr 
writedoc(tag_struct_t *tag_struct_p, const gchar *url){

    if (!tag_struct_p) return NULL;
    NOOP( "write 0x%x\n", GPOINTER_TO_INT(tag_struct_p->tag_list));
	
    xmlLineNumbersDefault(1);
    xmlThrDefIndentTreeOutput(1);
    xmlKeepBlanksDefault(0);
    xmlThrDefTreeIndentString("  ");

    tag_struct_p->doc = xmlNewDoc(BAD_CAST "1.0");
    xmlTextWriterPtr writer = xmlNewTextWriterDoc(&(tag_struct_p->doc), 0);
    tag_struct_p->doc->URL = (const xmlChar *)url;

    if (writer == NULL) {
        DBG("Error creating the xml writer\n");
        return NULL;
    }
    if (xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL) < 0){
        DBG("Error at xmlTextWriterStartDocument\n");
	xmlFreeTextWriter(writer);
        return NULL;
    }
	
    xmlTextWriterSetIndent(writer, 20);
    

    GSList *l_list = get_link_list(tag_struct_p, NULL, NULL);

    GSList *list = l_list;
  
    const gchar *text = NULL;
    for (list = l_list;list; list = list->next){
	if (!list->data) continue;
	tag_item_t *tag_p = list->data;
	if (GPOINTER_TO_INT(list->data) == -1) {
	    if (xmlTextWriterEndElement(writer) < 0) {
		DBG("Error at xmlTextWriterEndElement\n");
	    }
	    //NOOP("End element: %d\n", --level);
	    continue;
	}
        const gchar *s = tag_p->string;
        //fprintf(stderr, "writing tag \"%s\" (%d)\n", tag_p->name, tag_p->hidden);

        if (s) {
            gchar *c = g_strdup_printf("====== BEGIN %s  ======", s);
	    xmlTextWriterWriteComment(writer,BAD_CAST c);
            g_free(c);
            //fprintf(stderr, "STRING: %s\n", tag_p->string);
        }
	GSList *attribute_list = NULL;
	void *arg[] = {(void *)&attribute_list, NULL};
	g_hash_table_foreach(tag_p->attribute_hash, hash2list, arg);
	GSList *tmp = attribute_list;
	for (;tmp && tmp->data; tmp=tmp->next){
	    attribute_t *attribute_p = tmp->data;
	    if (strcasecmp(attribute_p->name, "text")==0) {
		text = attribute_p->value;
	    }
	}
	//NOOP("Start element = %s level %d\n", tag_p->name, level++);
	
	if (xmlTextWriterStartElement(writer, BAD_CAST (tag_p->name)) < 0){
	    DBG("Error at xmlTextWriterStartElement: %s\n", tag_p->name);
	}
	
	tmp = attribute_list;
	for (;tmp && tmp->data; tmp=tmp->next){
	    attribute_t *attribute_p = tmp->data;
            // skip hidden attributes...
            if (attribute_p->hidden) continue;
	    if (strcasecmp(attribute_p->name, "text")==0) continue;
            if (!attribute_p->value || !strlen(attribute_p->value)) continue;
            if (attribute_p->prefix) {
                const gchar *value = attribute_p->value;
                if (tag_struct_p->schema_file && 
                        strcasecmp(attribute_p->name, 
                            "noNamespaceSchemalocation")==0)
                {
                    value = tag_struct_p->schema_file;
                }
                xmlTextWriterWriteAttributeNS (writer,
                    BAD_CAST (attribute_p->prefix), 
                    BAD_CAST (attribute_p->name), 
                    NULL,  
                    BAD_CAST value);
            } else
            {
                xmlTextWriterWriteAttribute(writer, 
                    BAD_CAST (attribute_p->name),
                    BAD_CAST (attribute_p->value));
            }
	}
	if (text && strlen(text)) {
	    xmlTextWriterWriteFormatString(writer, "%s",  BAD_CAST (text));
	    text = NULL;
	}
	g_slist_free(attribute_list);

    }
    g_slist_free(l_list);
    xmlTextWriterEndDocument(writer);
    return writer;
}

#if 0
    if (!in_list) in_list = tag_struct_p->tag_list;
 /*   if (!writer) {
	writer = xmlNewTextWriterDoc(&(tag_struct_p->doc), 0);
	if (writer == NULL) {
	    DBG("Error creating the xml writer\n");
	    return FALSE;
	}
    	if (xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL) < 0){
	    DBG("Error at xmlTextWriterStartDocument\n");
	    return FALSE;
	}
    }*/
    
    GSList *list =  in_list;

    gboolean included_file = FALSE;
    for (;list && list->data; list=list->next){
	tag_item_t *tag_p = list->data;
	GSList *attribute_list = NULL;
	void *arg[] = {(void *)&attribute_list, NULL};
	g_hash_table_foreach(tag_p->attribute_hash, hash2list, arg);
	if (tag_p->name) {
	  /*  if (xmlTextWriterStartElement(writer, BAD_CAST (tag_p->name)) < 0){
		DBG("Error at xmlTextWriterStartElement: %s\n", tag_p->name);
	    }*/
    	    NOOP( "Start element: %s\n", tag_p->name);
	}
	GSList *tmp = attribute_list;
	for (;tmp && tmp->data; tmp=tmp->next){
	    attribute_t *attribute_p = tmp->data;
	   /* if (xmlTextWriterWriteAttribute(writer,
			BAD_CAST (attribute_p->name),
                        BAD_CAST (attribute_p->value)) < 0)
	    {
		DBG("Error at xmlTextWriterWriteAttribute: %s\n", attribute_p->name);
	    }*/
	    NOOP( "attribute %s = %s\n", attribute_p->name, attribute_p->value);
	}
	g_slist_free(attribute_list);
	// recurse:
	if (tag_p->tag_list){
	    gboolean result = write2memory(tag_struct_p, writer, tag_p->tag_list);
	    if (!result) return FALSE;
	}
	/*if (xmlTextWriterEndElement(writer) < 0) {
	    DBG("Error at xmlTextWriterEndElement :%s\n", tag_p->name);
	}*/
    	NOOP( "End element: %s\n", tag_p->name);
    }
    // free writer
  /*  if (xmlTextWriterEndDocument(writer) < 0) {
        DBG("Error at xmlTextWriterEndDocument\n");
        return FALSE;
    }

    xmlFreeTextWriter(writer);

    xmlSaveFileEnc("/tmp/test.xml", tag_struct_p->doc, "UTF-8");*/

    //xmlFreeDoc(doc);
    return TRUE;

}

#endif



