// Copyright (C) Instituto Mexicano del Petróleo. All rights reserved.
// Edscott Wilson Garcia <edscott@imp.mx>
#ifndef TAGFILE_H
#define TAGFILE_H

typedef struct Tag_t
{
    void *data;
}Tag_t;

typedef struct Tag_item_t
{
    void *data;
}Tag_item_t;

typedef struct Attribute_item_t
{
    void *data;
}Attribute_item_t;

// 
// Create a new empty Tag_t. 
Tag_t * 
tag_new (void);

// Load a schema from a specified file into the Tag_p
// This will override any previous schema.
gboolean
tag_load_schema(Tag_t * Tag_p, gchar *filename, GError **in_error);
    
// Create a Tag_t from an existing xml file.
Tag_t * 
tag_new_from_file (const gchar *filename, GError **in_error);

// Create a Tag_t from an existing schema file.
Tag_t * 
tag_new_from_schema_file (const gchar *filename, GError **in_error);
// Liberate all associated memory with a Tag_t.
void 
tag_free (Tag_t *Tag_p);

// Write an XML file from a Tag_t.
gboolean
tag_write_file(Tag_t *Tag_p, const gchar *output_file, gboolean split_includes);

// Validate an XML structure within a Tag_t from an associated schema.
gboolean
tag_validate(Tag_t *Tag_p);

#if 0
// ?
Tag_t *
tag_get_schema(Tag_t *Tag_p);
#endif

// Add or replace a given attribute listed under the specified Tag_item_t.
//
Attribute_item_t *attribute_item_add(Tag_t *Tag_p, Tag_item_t *parent_tag, 
	const gchar *name,
	const gchar *value,
	const gchar *namespace
	);

#define attribute_item_replace attribute_item_add

// Add a Tag_item_t. If parent tag is null, then tag will be added to the top level.
Tag_item_t *tag_item_add(Tag_t *Tag_p, Tag_item_t *parent_tag, const gchar *tag_name);

void tag_item_remove(Tag_t *Tag_p, Tag_item_t *tag);

// Get first tag which is child of parent tag. Restrict to tag_name if not NULL. 
// If parent tag is NULL, get top level tag.
Tag_item_t *get_tag_item(Tag_t *Tag_p, Tag_item_t *parent_tag, const gchar *tag_name);

Tag_item_t *get_parent_tag(Tag_item_t *tag);

// Get tags which are children of parent tag. Restrict to tag_name if not NULL. 
// If parent tag is NULL, get top level tag list.
GSList *get_tag_item_list(Tag_t *Tag_p, Tag_item_t *parent_tag, const gchar *tag_name);

// Get all descendent attributes of parent tag with matching attribute name.
// Data belongs to Tag_t, but list must be freed with g_slist_free()
GSList *get_full_attribute_list(Tag_t *Tag_p, Tag_item_t *parent_tag, const gchar *attribute_name);

// Get all attribute items which are children to parent tag.
// Data belongs to Tag_t, but list must be freed with g_slist_free()
GSList *get_attribute_item_list(Tag_item_t *parent_tag);

const gchar *
get_tag_name(Tag_item_t *item);

const gchar *
tag_item_get_string(Tag_item_t *item);

gboolean
tag_item_has_children(Tag_item_t *item);

gboolean
set_tag_item_user_data(Tag_item_t *item, gpointer user_data);

gboolean
tag_item_set_string(Tag_item_t *item, const gchar *string);

gpointer
get_tag_item_user_data(Tag_item_t *item);

// Set attribute to hidden status. Hidden attributes will not be
// written and will be shown in tree in different color.
void attribute_set_hidden(Attribute_item_t *item, gboolean state);
gboolean attribute_get_hidden(Attribute_item_t *item);
void tag_set_hidden(Tag_item_t *item, gboolean state);
gboolean tag_get_hidden(Tag_item_t *item);


// Get first attribute with matching name, child of parent tag.
Attribute_item_t *get_attribute(Tag_item_t *parent_tag, const gchar *attribute_name);

const gchar *
get_attribute_prefix(Attribute_item_t *item);

const gchar *
get_attribute_name(Attribute_item_t *item);

Tag_item_t *
get_attribute_parent(Attribute_item_t *item);

const gchar *
get_attribute_value(Attribute_item_t *item);

gboolean
set_attribute_value(Attribute_item_t *item, const gchar *value);

gchar **
get_attribute_value_list(Attribute_item_t *item, gsize *length);

gint 
get_validation_status(Tag_t *Tag_p);
#endif
