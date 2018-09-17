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
