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

