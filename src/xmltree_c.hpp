#ifndef XMLTREE_C_HPP
#define XMLTREE_C_HPP
#include "tag_c.hpp"
#define XMLTREE_key_type 0x01
#define XMLTREE_string_type 0x02
#define xmltree_item tag_item_t
#define xmltree_attribute attribute_t

class xmltree_c: protected tag_c{
    public:
        xmltree_c(const gchar *, gboolean);
        ~xmltree_c(void);
        void text_activates_top_attribute(gint);
        void xmltree_set_title(const gchar *);
        void xmltree_set_echo(const gchar *, const gchar *);
        void xmltree_set_xml(const gchar *);
        void xmltree_set_schema(const gchar *);
        void xmltree_set_editable_attribute(const gchar *attribute, void *type);
        gboolean xmltree_run(void);
void * xsdtree_run(void){
void *xsdtree_show_hidden(void *state){
    
    private:
        gchar *window_title;
        gchar *xml_path;
        gchar *schema_path;
        gchar *ns;
        gchar **editable_attributes;
        gchar *repeat_names;
        GHashTable *echo_hash;
        GHashTable *attribute_hash;
        GHashTable *editable_elements;
        gint text_activates_top_attribute;

        gboolean validated;

        void (*set_defaults)(void);
        void *set_defaults_data;
        xmltree_item *parent_tag;

        gboolean xmltree_c::validate_with_schema(void);
        

Attribute_item_t *
xmltree_c::xmltree_attribute_item_add


        // XXX  XSD specific
static gboolean choice_callback(Tag_item_t *src_tag, 
        GtkTreeView *treeview, GtkTreeModel *treemodel, 
        GtkTreeIter *sibling, gint flag);
static void update_iconsXSD(GtkTreeModel *model, GtkTreeIter iter);
static GdkPixbuf *get_attribute_pixbuf(Attribute_item_t *row_attribute);
static Tag_item_t *set_attribute_colorXSD(GtkTreeModel *treemodel, GtkTreeIter *iter);
};

#endif
