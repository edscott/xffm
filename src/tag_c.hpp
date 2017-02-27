#ifndef tag_c_hpp
#define tag_c_hpp
typedef struct tag_item_t {
    GtkTreePath *path;
    gchar *name;
    gchar *string;
    gboolean hidden;
    GHashTable *attribute_hash;
    GSList *tag_list;
    gpointer user_data;
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



class tag_c {
    public:
        ~tag_c(void);
        tag_c(const gchar *, gboolean); // TRUE for schema
    protected:
        gboolean tag_item_set_string(tag_item_t *, const gchar *);
        const gchar * tag_item_get_string(tag_item_t *);
        gboolean tag_load_schema(gchar *, GError **);

        gboolean tag_validate(void);
        gboolean tag_write_file(const gchar *, gboolean );
        gboolean set_tag_item_user_data(tag_item_t *, gpointer );
        gpointer get_tag_item_user_data(tag_item_t *);
        attribute_t *attribute_item_add(tag_item_t *, 
                const gchar *,
                const gchar *,
                const gchar *
                );
        void attribute_set_hidden(attribute_t *, gboolean );
        gboolean attribute_get_hidden(attribute_t *);
        void tag_set_hidden(tag_item_t *, gboolean );
        gboolean tag_get_hidden(tag_item_t *);
        tag_item_t *get_tag_item(tag_item_t *, const gchar *);
        tag_item_t *get_parent_tag(tag_item_t *);
        tag_item_t *tag_item_add(tag_item_t *, const gchar *);
        void tag_item_remove(tag_item_t *);
        GSList *get_tag_item_list(tag_item_t *, const gchar *);
        const gchar * get_tag_name(tag_item_t *);
        gboolean tag_item_has_children(tag_item_t *);
        attribute_t *get_attribute(tag_item_t *, const gchar *);
        GSList *get_attribute_item_list(tag_item_t *);
        GSList *get_full_attribute_list(tag_item_t *, const gchar *);
        tag_item_t * get_attribute_parent(attribute_t *);
        const gchar * get_attribute_name(attribute_t *);
        const gchar * get_attribute_prefix(attribute_t *);
        const gchar * get_attribute_value(attribute_t *);
        gboolean set_attribute_value(attribute_t *, const gchar *);
        gchar ** get_attribute_value_list(attribute_t *, gsize *);
        gint get_validation_status(void);
        gchar *xml_path;


    private:
        void init(const gchar *, GError **);
        xmlDocPtr doc;
        gboolean validated; // Three state: {yes, no, unknown}/{1,0,-1}
        gchar *schema_file;
        xmlDocPtr schema_doc;
        xmlTextWriterPtr writer;    
        GSList *tag_list;
        gboolean is_schema;

        void build_tag_struct (GError **);
        GSList *get_tag_list(const xmlNodePtr ,  tag_item_t *, gint );
        gchar *get_value(const gchar *);

        attribute_t *mk_attribute(const gchar *, const gchar *, 
                const xmlNs *, tag_item_t *);
        tag_item_t * mk_tag(const gchar *);
        void free_tag(tag_item_t *);
        gboolean create_doc(const gchar *);
        gboolean write_file(const gchar *);
        gboolean validate_xml(void);
        GSList * get_link_list(GSList *, GSList *);
        xmlTextWriterPtr writedoc(const gchar *);
        void get_att_list(GSList **, GSList *, const gchar *);

};

#endif
