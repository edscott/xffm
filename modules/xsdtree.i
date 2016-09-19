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


