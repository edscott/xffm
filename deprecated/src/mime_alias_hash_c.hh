#ifndef MIME_ALIASHASH_C_HH
#define MIME_ALIASHASH_C_HH
#include "mime_hash_c.hh"

template <class Type>
class mime_aliashash_c: public  mime_hash_c<Type>{
    public:
        static gchar *
        get_alias_type(const gchar *type, Type T){
            if(type) {
                gchar *hash_key=mime_aliashash_c::get_hash_key(type, T);
                const gchar *basic_type = (const gchar *)g_hash_table_lookup(T.hash, hash_key);
                g_free(hash_key);
                if (basic_type) return g_strdup(basic_type);
                return g_strdup(type);
            } 
            return g_strdup(inode[INODE_UNKNOWN]);
        }
        void build_hash (Type T, const gchar *mimefile) {
            xmlChar *value;
            xmlNodePtr node;
            xmlNodePtr subnode;
            //build hashes from system files

            node = xmlDocGetRootElement (T.doc);
            if(!xmlStrEqual (node->name, (const xmlChar *)"mime-info")) {
                fprintf(stderr, "mime_hash_t::Invalid XML file: %s (no mime-info). Replace this file.\n", mimefile);
                return;
            }
            /* Now parse the xml tree */
            NOOP("mime_hash_t:: parsing %s\n", mimefile);
            for(node = node->children; node; node = node->next) {
                if(xmlStrEqual (node->name, (const xmlChar *)"mime-key")) {
                    gchar *type;
                    //  type has to be defined. 
                    type = (gchar *)xmlGetProp (node, (const xmlChar *)"type");
                    if(!type) {
                        DBG("mime_hash_t:: return on type==NULL\n");
                         return;
                    }

                    for(subnode = node->children; subnode; subnode = subnode->next) {
                        if(xmlStrEqual (subnode->name, (const xmlChar *)T.xmlkey)) {
                            // xmlkey --> xmldata
                            value = xmlGetProp (subnode, (const xmlChar *)T.xmldata);
                            if(value && strlen((const gchar *)value)) {
                                g_hash_table_replace (T.hash, 
                                        g_strdup(type), g_strdup((const gchar *)value));
                                //DBG("hashing %s --> %s\n", type, (const gchar *)value);
                            }
                            g_free (value);
                            continue;
                        }
                    }
                    g_free(type);
                }
            }
            fprintf(stderr, "mime_aliashash_c::hash table build (%p) with \"%s\"-->\"%s\" is now complete.\n", 
                    T.hash, T.xmlkey, T.xmldata);
        }


};

#endif
