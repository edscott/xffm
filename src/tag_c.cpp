/*
 * Copyright (C) 2002-2017 Edscott Wilson Garcia
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


#include "tag_c.hpp"


tag_c::~tag_c(void){
    // This goes down with the writer (that's screwy)
    // if (doc) xmlFreeDoc (doc);
    if (schema_doc) xmlFreeDoc (schema_doc);
    GSList *list=tag_list;
    for (;list && list->data; list=list->next){
	tag_item_t *list_tag_p = (tag_item_t *)list->data;
	free_tag(list_tag_p);
    }
    if (tag_list) g_slist_free(tag_list);
    g_free(file);
    g_free(schema_file);
}


void 
tag_c::init(const gchar *filename, GError **in_error){
    validated = -1; // unknown status.
    file = filename?g_strdup(filename):NULL;
    GError *error=NULL;
    is_schema=FALSE;
    tag_list = NULL;
    schema_file = NULL;
    is_schema=FALSE;
    if(file) build_tag_struct(&error);

    if (error) {
	if (in_error) *in_error = error;
	else g_error_free(error);
        fprintf(stderr, "tag_c: %s\n", error->message);
        throw 1;
    }
}

tag_c::tag_c(const gchar *filename){
    init(filename, NULL);
}


tag_c::tag_c(const gchar *filename, gboolean data){
    init(filename, NULL);
    is_schema = data; // TRUE for schema.
}

tag_c::tag_c (const gchar *filename, GError **in_error, gboolean data)
{
    init(filename, in_error);
    is_schema = data; // TRUE for schema.
}

//*//*//*//*//*//*//*//*//*//*//*//*//*//*//*//*//*//*//*//*//*//*//

void
tag_c::build_tag_struct (GError **error){
    if (!file){
	g_error("build_tag_p(): data cannot be NULL!");
    }
    GQuark  quark = g_quark_from_string ("TagT");
    // Check XML file
    if(access (file, R_OK) != 0) {
        fprintf(stderr, "build_tag_struct() access(%s, R_OK)!=0 (%s)\n", file, strerror(errno));
	if (error) {
	    *error = g_error_new(quark, 0x01, "access(%s, R_OK)!=0 (%s)\n", file, strerror(errno));
	}
        throw 2;
    }

    xmlKeepBlanksDefault (0);
    // Parse XML file
    if((doc = xmlParseFile (file)) == NULL) {
        fprintf(stderr, "xmlParseFile(): unable to parse %s \n", file);
	if (error) {
	    *error = g_error_new(quark, 0x02,"xmlParseFile(): unable to parse %s \n", file);
	}
        throw 3;
    } 

    /* Now parse the xml tree */
    xmlNodePtr node = xmlDocGetRootElement (doc);
    if (node == NULL) {
        fprintf(stderr, "xmlDocGetRootElement (): empty document\n");
	if (error) {
	    *error = g_error_new(quark, 0x03, "xmlDocGetRootElement (): empty document");
	}
	throw 4;
    }


    if (xmlXIncludeProcess (doc) < 0) {
	fprintf(stderr, "XInclude processing failed\n");
    }
    validated = -1;
  //////////////////////////

    /* Now parse the xml tree, adding elements to the tag_p. */
    tag_list = get_tag_list((const xmlNodePtr)node, NULL, 0);
    // Clean up XML allocated memory
    xmlFreeDoc (doc);
    doc = NULL;
    
    return ;
}

GSList * 
tag_c::get_tag_list(const xmlNodePtr in_node,  tag_item_t *top_tag_p, gint level){
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
	//for (i=0; i<level; i++) NOOP(" "); NOOP("%s\n", (gchar *)(node->name));
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
		get_tag_list((const xmlNodePtr)node->children, tag_p, level+1);

	}
	if (strcasecmp((gchar *)(node->name), "schema")==0){
	    NOOP("node->schema_tag\n");
	}
	// Name space specifics
	if (node->nsDef){
	    xmlNs *nsDef = node->nsDef;
	    for (;nsDef && nsDef->href; nsDef = nsDef->next){
		xmlNs ns;
		ns.prefix = (const xmlChar *)"xmlns";
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
		    fprintf(stderr, "// This should never happen: attribute without a name.\n");
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
		    if (schema_file){
			fprintf(stderr, "More than one \"noNamespaceSchemaLocation\"\n");
		    } else {
			schema_file = g_strdup(attribute_p->value);
		    }
		}
	    }
	}
    }
    return list;
}



gchar *
tag_c::get_value(const gchar *xmlvalue){
    if (!xmlvalue) return NULL;
    gchar *value=g_strdup(xmlvalue);
    g_strstrip(value);
    if (strlen(value)==0 || strcmp(value, "\n")==0){
	g_free(value);
	return NULL;
    }
    return value;
}


//*//*//*//*//*//*//*//*//*//*//*//*//*//*//*//*//*//*//*//*//*//*//

gboolean
tag_c::tag_item_set_string(tag_item_t *item, const gchar *string){
    if (!item) return FALSE;
    if (!string) return FALSE;
    tag_item_t *tag_p = (tag_item_t *)item;
    g_free(tag_p->string);
    tag_p->string = g_strdup(string);
    return TRUE;
}

const gchar *
tag_c::tag_item_get_string(tag_item_t *item){
    if (!item) return FALSE;
    tag_item_t *tag_p = (tag_item_t *)item;
    return (tag_p->string);
}

gboolean
tag_c::tag_load_schema(gchar *filename, GError **in_error){
    if (!filename) return FALSE;
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

    if(schema_doc) {
	// free previous schema, if any
        xmlFreeDoc(schema_doc);  
    }
    schema_doc = xmlReadFile(schema_file, NULL, XML_PARSE_NONET);
    if (schema_doc == NULL) return FALSE;
    return TRUE;
}

gboolean
tag_c::tag_validate(void){
    return validate_xml();
}

gboolean
tag_c::tag_write_file(const gchar *output_file, gboolean split_includes){
    // XXX split_includes is not used yet...
    return write_file(output_file);
}

gboolean
tag_c::set_tag_item_user_data(tag_item_t *item, gpointer user_data){
    if (!item) return FALSE;

    item->user_data = user_data;
    return TRUE;
}

gpointer
tag_c::get_tag_item_user_data(tag_item_t *item){
    if (!item) return NULL;
    tag_item_t *tag_p = (tag_item_t *)item;
    return tag_p->user_data;
}

attribute_t *
tag_c::attribute_item_add(tag_item_t *parent_tag, 
	const gchar *name,
	const gchar *value,
	const gchar *in_ns
	){
    const xmlNs *ns = (const xmlNs *) in_ns;
    
    if (!name || !strlen(name)){
	fprintf(stderr, "attribute_item_add(): !name || !strlen(name)  (%s, %s)\n", name, value);
	return NULL;
    }
    if (!value) value = "";
    tag_item_t *tag_p = (tag_item_t *)parent_tag;
    attribute_t * attribute_p = mk_attribute(name, value, ns, tag_p);

   
    //NOOP(stderr, "replacing attribute: %s with %s\n",
	//    attribute_p->name, value);
    g_hash_table_replace(tag_p->attribute_hash,
		    attribute_p->name, attribute_p);
    return (attribute_t *)attribute_p;
}

void tag_c::attribute_set_hidden(attribute_t *item, gboolean state){
    if (!item) return ;
    attribute_t *a_p = (attribute_t *)item;
    a_p->hidden = state;
}

gboolean tag_c::attribute_get_hidden(attribute_t *item){
    if (!item) return TRUE;
    attribute_t *a_p = (attribute_t *)item;
    return a_p->hidden;
}

void tag_c::tag_set_hidden(tag_item_t *item, gboolean state){
    if (!item) return ;
    tag_item_t *t_p = (tag_item_t *)item;
    t_p->hidden = state;
}

gboolean tag_c::tag_get_hidden(tag_item_t *item){
    if (!item) return TRUE;
    tag_item_t *t_p = (tag_item_t *)item;
    return t_p->hidden;
}

tag_item_t *
tag_c::get_tag_item(tag_item_t *parent_tag, const gchar *tag_name){
    if (!tag_name) return NULL;
    GSList *list = get_tag_item_list(parent_tag, tag_name); 
    if (list){
	tag_item_t *item = (tag_item_t *)list->data;
	g_slist_free(list);
	return item;
    }
    return NULL;
}

tag_item_t *
tag_c::get_parent_tag(tag_item_t *tag){
    if (!tag) return NULL;
    tag_item_t *tag_p = (tag_item_t *)tag;
    return (tag_item_t *) tag_p->parent_tag;
}


tag_item_t *
tag_c::tag_item_add(tag_item_t *parent_tag, const gchar *tag_name){
    GSList **list;
    tag_item_t *tag_p = (tag_item_t *)parent_tag;
    if (parent_tag == NULL) {list = &(tag_list);}
    else { list = &(tag_p->tag_list);}
    tag_item_t *new_tag_p = mk_tag(tag_name);
    *list = g_slist_append(*list, new_tag_p);
    new_tag_p->parent_tag = (void *)parent_tag;
    //fprintf(stderr, "tag =%p (%s) --> parent=%p (%s)\n", new_tag_p, tag_name, parent_tag, get_tag_name(parent_tag));
    //NOOP(stderr, "list=0x%x): %s\n", GPOINTER_TO_INT(*list), tag_name);
    return (tag_item_t *) new_tag_p;
}

void 
tag_c::tag_item_remove(tag_item_t *tag){
    tag_item_t *parent_tag = get_parent_tag(tag); 
    tag_item_t *ptag = (tag_item_t *)parent_tag;
    GSList **list;
    if (parent_tag == NULL) {list = &(tag_list);}
    else { list = &(ptag->tag_list);}
    *list = g_slist_remove(*list, tag);
    free_tag((tag_item_t *)tag);
}

GSList *
tag_c::get_tag_item_list(tag_item_t *parent_tag, const gchar *tag_name){
    //NOOP(stderr, "getting list for %s\n", tag_name);
    tag_item_t *tag_p = (tag_item_t *)parent_tag;
    GSList *list;
    if (parent_tag == NULL) {list = tag_list;}
    else { list = tag_p->tag_list;}
    if (!list) return NULL;
    GSList *retlist = NULL;
    for (;list && list->data; list = list->next){
	tag_item_t *tag_p = (tag_item_t *)list->data;
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
    void **arg = (void **)data;
    GSList **outlist = (GSList **)arg[0];   
    const gchar *attribute_name = (const gchar *)arg[1];  
    if (strcmp((gchar *)key, attribute_name)==0){
	//attribute_t *attribute_p = value;
	*outlist =g_slist_prepend(*outlist, value);
    }
}


void
tag_c::get_att_list(GSList **outlist, GSList *inlist, const gchar *attribute_name){
    GSList *list = inlist;
    for(; list && list->data; list = list->next){
	tag_item_t *tag_p = (tag_item_t *)list->data;
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
tag_c::get_tag_name(tag_item_t *item){
    if (!item) return NULL;
    //NOOP(stderr, "name.... %s\n",  ((tag_item_t *)(item))->name);
    return ((tag_item_t *)(item))->name;
}
    
  
gboolean
tag_c::tag_item_has_children(tag_item_t *item){
    if (!item) return FALSE;
    tag_item_t *tag_p = (tag_item_t *) item;
    if (tag_p->tag_list) return TRUE;
    else return FALSE;
}

/////

attribute_t *
tag_c::get_attribute(tag_item_t *parent_tag, const gchar *attribute_name){
    if (!parent_tag) return NULL;
    tag_item_t *tag_p = (tag_item_t *)parent_tag;
    GHashTable *hash = tag_p->attribute_hash;
    if (!hash) return NULL;
    attribute_t *Attribute_item_p = (attribute_t *)g_hash_table_lookup(hash, attribute_name);
    return Attribute_item_p;
}


static void 
hash2list(gpointer key, gpointer value, gpointer data){
    void **arg = (void **)data;
    GSList **list = (GSList **)arg[0];
    const gchar *attribute_name = (const gchar *)arg[1];
    attribute_t *a_p = (attribute_t *)value;
    if (!attribute_name){
	*list = g_slist_append(*list, a_p);
    } else if (strcasecmp(attribute_name, a_p->name)==0){
	*list = g_slist_append(*list, a_p);
    }
}

GSList *
tag_c::get_attribute_item_list(tag_item_t *parent_tag){
    if (!parent_tag) return NULL;
    tag_item_t *tag_p = (tag_item_t *)parent_tag;
    GHashTable *hash = tag_p->attribute_hash;
    if (!hash) return NULL;
    GSList *retlist = NULL;
    void *arg[2]={(void *)&retlist, NULL};
    g_hash_table_foreach(hash, hash2list, (void *)arg);
    return retlist;
}

GSList *
tag_c::get_full_attribute_list(tag_item_t *parent_tag, const gchar *attribute_name){
    //NOOP(stderr, "get_full_attribute_list for %s\n", attribute_name);
    if (!attribute_name || strlen(attribute_name)==0) return NULL;
    if (!parent_tag) return NULL;
    GSList *outlist=NULL;
    tag_item_t *tag_p = (tag_item_t *)parent_tag;

    GSList *in_list;
    if (tag_p) {
	in_list = tag_p->tag_list;
    } else {
	in_list = tag_list;
    }
    get_att_list(&outlist, in_list, attribute_name);
    outlist = g_slist_reverse(outlist);
    return (outlist);
}

tag_item_t *
tag_c::get_attribute_parent(attribute_t *item){
    if (!item) return NULL;
    attribute_t *a_p = (attribute_t *)item;
    return (tag_item_t *)a_p->parent_tag;
}

const gchar *
tag_c::get_attribute_name(attribute_t *item){
    if (!item) return NULL;
    attribute_t *a_p = (attribute_t *)item;
    return a_p->name;
}

const gchar *
tag_c::get_attribute_prefix(attribute_t *item){
    if (!item) return NULL;
    attribute_t *a_p = (attribute_t *)item;
    return a_p->prefix;
}
    
const gchar *
tag_c::get_attribute_value(attribute_t *item){
    if (!item) return NULL;
    attribute_t *a_p = (attribute_t *)item;
    return a_p->value;
}
     
gboolean
tag_c::set_attribute_value(attribute_t *item, const gchar *value){
    if (!item || !value) return FALSE;
    attribute_t *a_p = (attribute_t *)item;
    g_free(a_p->value);
    a_p->value = g_strdup(value);
    return TRUE;
}
   
gchar **
tag_c::get_attribute_value_list(attribute_t *item, gsize *length){
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
tag_c::get_validation_status(void){
    return validated;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

// PRIVATE


attribute_t *
tag_c::mk_attribute(const gchar *name, const gchar *value, const xmlNs *ns, tag_item_t *parent_tag){
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
static void 
free_attribute(void *data){
    attribute_t *attribute_p = (attribute_t *)data;
    if (!attribute_p) return;
    // attribute_p->name is also the key!!
    g_free(attribute_p->name);
    g_free(attribute_p->value);
    g_free(attribute_p->prefix);
    g_free(attribute_p);
}

tag_item_t *
tag_c::mk_tag(const gchar *name){
    if (!name) return NULL;
    tag_item_t *tag_p = (tag_item_t *)malloc(sizeof(tag_item_t)); 
    if (!tag_p) g_error("malloc:%s", strerror(errno));
    memset(tag_p, 0, sizeof(tag_item_t));
    tag_p->name = g_strdup(name);
    // Do *not* g_free the key. The key is contained within the attribute structure!
    tag_p->attribute_hash = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, free_attribute);
    return tag_p;
}

void tag_c::free_tag(tag_item_t *tag_p){
    if (!tag_p) return;

    g_hash_table_destroy(tag_p->attribute_hash);

    GSList *list=tag_p->tag_list;
    for (;list && list->data; list=list->next){
	tag_item_t *list_tag_p = (tag_item_t *)list->data;
	free_tag(list_tag_p);
    }
    g_slist_free(tag_p->tag_list);

    g_free(tag_p->string);
    g_free(tag_p->name);
    g_free(tag_p);
}



gboolean
tag_c::create_doc(const gchar *output_file){
    if (!output_file) output_file = "memory";
    xmlTextWriterPtr writer = writedoc(output_file);
    xmlFreeTextWriter(writer);
    return TRUE;
}


gboolean
tag_c::write_file(const gchar *xfile){

    if (!create_doc(xfile)){
	fprintf(stderr, "cannot create xmldoc\n");
	return FALSE;
    }
    xmlSaveFileEnc((const gchar *)(doc->URL), doc, "UTF-8");
    return TRUE;
}

gboolean tag_c::validate_xml(void){
    if (is_schema) return TRUE; // Schemas should not have a schema...

    if (!schema_doc){
	fprintf(stderr, "schema_doc==NULL\n");
	return FALSE;
    }
    NOOP("validating xml %s ...\n", file);

    if (!create_doc(NULL)){
	fprintf(stderr, "cannot create xmldoc\n");
	return FALSE;
    }
    
    xmlSchemaParserCtxtPtr validationCtxt = xmlSchemaNewDocParserCtxt(schema_doc);
    //xmlSchemaValidCtxtPtr validationCtxt = xmlSchemaNewValidCtxt (NULL);
    if (validationCtxt == NULL) {
	// Cannot create a validation context for the schema
	fprintf(stderr, "Failed to create the validation context.\n");
	return FALSE;
    }

    xmlSchemaPtr schema = xmlSchemaParse(validationCtxt);
    if (schema == NULL) {
        /* the schema itself is not valid */
	fprintf(stderr, "schema itself is not valid\n");
        xmlSchemaFreeParserCtxt(validationCtxt);
        return FALSE;
    }

    xmlSchemaValidCtxtPtr valid_ctxt = xmlSchemaNewValidCtxt(schema);
    if (valid_ctxt == NULL) {
        fprintf(stderr, "unable to create a validation context for the schema\n");
        xmlSchemaFree(schema);
        xmlSchemaFreeParserCtxt(validationCtxt);
        return FALSE;
    }

    gboolean result = (xmlSchemaValidateDoc(valid_ctxt, doc) == 0);
    xmlSchemaFreeValidCtxt(valid_ctxt);
    xmlSchemaFree(schema);
    xmlSchemaFreeParserCtxt(validationCtxt);

    NOOP("XML document (%s) is %s valid according to xsd specification %s\n",
		file,
		(result)?"":"NOT",
		(schema_file)?schema_file: "");

    return result;
}


GSList *
tag_c::get_link_list(GSList *tag_list, GSList *link_list){
    
    
    GSList *l_list = link_list;
    GSList *list;
    if (!tag_list) list = tag_list;
    else list = tag_list;

    for (;list; list=list->next){
	if (!list->data) continue;
	tag_item_t *tag_p = (tag_item_t *)list->data;
        // skip hidden tags
        if (tag_p->hidden) continue;
	//if (strcasecmp(tag_p->name, "include")==0 ) continue;
    	//NOOP( "Start element: %s\n", tag_p->name);
	l_list = g_slist_append(l_list, tag_p);

	// recurse:
	if (tag_p->tag_list){
	    l_list = get_link_list(tag_p->tag_list, l_list);
	}

	l_list = g_slist_append(l_list, GINT_TO_POINTER(-1));
    }

    return l_list;
}


xmlTextWriterPtr 
tag_c::writedoc(const gchar *url){

    NOOP( "write 0x%x\n", GPOINTER_TO_INT(tag_list));
	
    xmlLineNumbersDefault(1);
    xmlThrDefIndentTreeOutput(1);
    xmlKeepBlanksDefault(0);
    xmlThrDefTreeIndentString("  ");

    doc = xmlNewDoc(BAD_CAST "1.0");
    xmlTextWriterPtr writer = xmlNewTextWriterDoc(&(doc), 0);
    doc->URL = (const xmlChar *)url;

    if (writer == NULL) {
        fprintf(stderr, "Error creating the xml writer\n");
        return NULL;
    }
    if (xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL) < 0){
        fprintf(stderr, "Error at xmlTextWriterStartDocument\n");
	xmlFreeTextWriter(writer);
        return NULL;
    }
	
    xmlTextWriterSetIndent(writer, 20);
    

    GSList *l_list = get_link_list(NULL, NULL);

    GSList *list = l_list;
  
    const gchar *text = NULL;
    for (list = l_list;list; list = list->next){
	if (!list->data) continue;
	tag_item_t *tag_p = (tag_item_t *)list->data;
	if (GPOINTER_TO_INT(list->data) == -1) {
	    if (xmlTextWriterEndElement(writer) < 0) {
		fprintf(stderr, "Error at xmlTextWriterEndElement\n");
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
	    attribute_t *attribute_p = (attribute_t *)tmp->data;
	    if (strcasecmp(attribute_p->name, "text")==0) {
		text = attribute_p->value;
	    }
	}
	//NOOP("Start element = %s level %d\n", tag_p->name, level++);
	
	if (xmlTextWriterStartElement(writer, BAD_CAST (tag_p->name)) < 0){
	    fprintf(stderr, "Error at xmlTextWriterStartElement: %s\n", tag_p->name);
	}
	
	tmp = attribute_list;
	for (;tmp && tmp->data; tmp=tmp->next){
	    attribute_t *attribute_p = (attribute_t *)tmp->data;
            // skip hidden attributes...
            if (attribute_p->hidden) continue;
	    if (strcasecmp(attribute_p->name, "text")==0) continue;
            if (!attribute_p->value || !strlen(attribute_p->value)) continue;
            if (attribute_p->prefix) {
                const gchar *value = attribute_p->value;
                if (schema_file && 
                        strcasecmp(attribute_p->name, 
                            "noNamespaceSchemalocation")==0)
                {
                    value = schema_file;
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



