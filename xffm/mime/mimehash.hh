#ifndef XF_MIMEHASH_HH
#define XF_MIMEHASH_HH

#define USER_RFM_CACHE_DIR      g_get_user_cache_dir(),"xffm+"
#define APPLICATION_MIME_FILE 	SYSTEM_MODULE_DIR,"mime-module.xml"
#define USER_APPLICATIONS 	USER_RFM_DIR,"user-applications.2"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>
#include <errno.h>



typedef struct txt_hash_t {
	    GHashTable *hash;
	    xmlDocPtr doc;
            gchar const *xmlkey;
            gchar const *xmldata;
            gchar const *xmlsubdata;
            pthread_mutex_t mutex;
}txt_hash_t;

enum {
    INODE_SOCKET,
    INODE_BLOCKDEVICE,
    INODE_CHARDEVICE,
    INODE_FIFO,
    INODE_DIRECTORY,
    INODE_UNKNOWN
};

static const gchar *inode[]={
    "inode/socket",
    "inode/blockdevice",
    "inode/chardevice",
    "inode/fifo",
    "inode/directory",
    "unknown"
};

namespace xf {

template <class Type>
class MimeHash {
    public:
	~MimeHash(void){
	    //g_free(hashname);
	    //g_hash_table_destroy(hash);
	    //pthread_mutex_destroy(&mutex);
	}
	    

        static gchar *
        get_hash_key_strstrip (const gchar *p, Type T){
            gchar *pp=g_strdup((char *)p);
            g_strstrip(pp);
            gchar *key=get_hash_key ((gchar *)pp, T);
            g_free(pp);
            return key;
        }
	static gchar *get_hash_key (const gchar * pre_key, Type T) {
	    GString *gs = g_string_new (pre_key);
	    gchar *key;
	    key = g_strdup_printf ("%10u", g_string_hash (gs));
	    g_string_free (gs, TRUE);
	    return key;
	}
        static const gchar *lookup(const gchar *key, Type T){
               return (const gchar *)
                    g_hash_table_lookup(T.hash, key);
        }

        static xmlDocPtr openXML(const gchar *mimefile){
            xmlDocPtr doc;
            xmlKeepBlanksDefault (0);
            if(access (mimefile, R_OK) != 0) {
                fprintf(stderr, "access(%s, R_OK)!=0 (%s)\n", mimefile, strerror(errno));
                return NULL;
            } 
            if((doc = xmlParseFile (mimefile)) == NULL) {
                fprintf(stderr, "mime_hash_t:: Cannot parse XML file: %s. Replace this file.\n", mimefile);
                return NULL;
            }
            return doc;
        }


        static void build_hash (Type T, const gchar *mimefile) {
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
                            // xmlkey --> xmldata
                            value = xmlGetProp (subnode, (const xmlChar *)T.xmldata);
                            gchar *key_string = g_utf8_strdown ((gchar *)value, -1);
                            g_free (value);
                            gchar *hash_key = get_hash_key (key_string,T);
                            if(key_string) {
                                        TRACE("mime_hash_t::replacing hash element \"%s\" with key %s --> %s\n", 
                                            key_string, hash_key, type);
                                        g_hash_table_replace (T.hash, hash_key, g_strdup(type));
                            }
                            g_free (key_string);
                            continue;
                        }
                    }
                    g_free(type);
                }
            }
            WARN( "MimeHash::hash table build (%p) with \"%s\"-->\"%s\" is now complete.\n", 
                    T.hash, T.xmlkey, T.xmldata);
        }


};
}
#include "mimealiashash.hh"
#include "mimesfxhash.hh"
#include "mimecommandhash.hh"
#include "mimeapplicationhash.hh"

#endif
