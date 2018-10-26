#ifndef MIME_COMMAND_HASH_C_HH
#define MIME_COMMAND_HASH_C_HH
#include "mimehash.hh"
namespace xf{
template <class Type>
class MimeCommandHash: public  MimeHash<Type>{
    public:
        static const gchar *lookup(const gchar *p, Type T){
            if (!p) return NULL;
            gchar *key=get_hash_key_strstrip (p, T);
            const gchar *value=(const gchar *)g_hash_table_lookup (T.hash, key);
            g_free(key);
            return value;
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
            TRACE("mime_hash_t:: parsing %s\n", mimefile);
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
                            int i;
                                    // application --> command --> 
                            value = xmlGetProp (subnode, (const xmlChar *)T.xmldata);
                            if(value) {
                                xmlChar *extra_value;
                                        // application --> command --> icon
                                extra_value = 
                                    xmlGetProp (subnode, (const xmlChar *)T.xmlsubdata);
                                if(extra_value) {
                                    gchar *k=get_hash_key ((gchar *)value, T);
                                    TRACE(stderr,"mime_command_hash_c::build_hash: adding %s: %s--> %s\n",
                                            type, value, extra_value);
                                    g_hash_table_replace (T.hash, k, extra_value);
                                }	
                            }
                       }
                    }
                    g_free(type);
                }
            }
            WARN( "mime_command_hash_c::hash table build (%p) with \"%s\"-->\"%s\"  -> \"%s\" is now complete.\n", 
                    T.hash, T.xmlkey, T.xmldata, T.xmlsubdata);
        }
   
};
}
#endif
