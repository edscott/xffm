#ifndef MIME_HASH_HPP
#define MIME_HASH_HPP
#include "xffm+.h"
#define USER_RFM_CACHE_DIR      g_get_user_cache_dir(),"rfm"
#define APPLICATION_MIME_FILE 	SYSTEM_MODULE_DIR,"mime-module.xml"
#define USER_APPLICATIONS 	USER_RFM_DIR,"user-applications.2"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>
#include <errno.h>
#include <dbh.h>
//template <class>
struct string4_hash_t {
	    xmlDocPtr doc;
            gchar const *keys[4];
            gchar *mimefile;
	    GHashTable *hash;
};


//template <class Type = string_hash_c >
template <class Type>
class mime_hash_t {
    public:
	mime_hash_t(void){
        }/*
	mime_hash_t(const gchar *data0, const gchar *data1, const gchar *data2){
            xmlkey = data0;
            xmldata = data1;
            xmlsubdata = data2;
            pthread_mutex_init(&mutex, NULL);
	    hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
            build_hash();
        }*/
/*	string_hash_c(const gchar *data0, const gchar *data1, const gchar *data2){
	    xmlkey = data0;
	    xmldata = data1;
	    xmlsubdata = data2;
	}
	void ((*get_data_free_f)(void *)){
	    return (g_free);
	}*/
/*	void set_xmlkey(const gchar *){
	    return ;
	}
	void set_xmldata(const gchar *){
	    return ;
	}
	void set_xmlsubdata(const gchar *){
	    return ;
	}*/
	const gchar *get_xmlkey(void){
	    return xmlkey;
	}
	const gchar *get_xmldata(void){
	    return xmldata;
	}
	const gchar *get_xmlsubdata(void){
	    return xmlsubdata;
	}
        
    private:
	const gchar *xmlkey;
	const gchar *xmldata;
	const gchar *xmlsubdata;

    public:
#if 0
	mime_hash_t(gchar *name){
	    hashname = g_strdup(name);
	    pthread_mutex_init(&mutex, NULL);
	    hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
/*	    if(!load_hash_from_cache()) {
		std::cerr<<"mime_hash_t:: now building hash from scratch:"<<hashname<<"\n";
		build_hash(Type);
		//generate_cache();
		//write_cache_sum (get_cache_sum ());
	    } else {
		std::cerr<<"mime_hash_t:: hash loaded from disk cache:"<<hashname<<"\n";
	    }
*/
	}
#endif
	~mime_hash_t(void){
	    //g_free(hashname);
	    //g_hash_table_destroy(hash);
	    //pthread_mutex_destroy(&mutex);
	}
	    

    private:
	gboolean load_hash_from_cache(void){return FALSE;}
	
	pthread_mutex_t mutex;
	//gchar  *hashname;

	gchar *get_hash_key (const gchar * pre_key) {
	    GString *gs = g_string_new (pre_key);
	    gchar *key;
	    key = g_strdup_printf ("%10u", g_string_hash (gs));
	    g_string_free (gs, TRUE);
	    return key;
	}
    public:
	void build_hash (Type T) {
            xmlkey = T.keys[0];
            xmldata = T.keys[1];
            xmlsubdata = T.keys[2];
	    gchar *mimefile = T.mimefile;
	    
	    xmlDocPtr doc = T.doc;

	    xmlChar *value;
	    xmlNodePtr node;
	    xmlNodePtr subnode;


	    //build hashes from system files

            const gchar *key = get_xmlkey();
            const gchar *xmldata = get_xmldata();
            printf("...\n");
	    if(mimefile==NULL || access (mimefile, R_OK) != 0) {
		DBG ("access(%s, R_OK)!=0 (%s)\n", mimefile, strerror(errno));
		return;
	    }


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
			if(xmlStrEqual (subnode->name, (const xmlChar *)get_xmlkey())) {
			    // xmlkey --> xmldata
			    value = xmlGetProp (subnode, (const xmlChar *)get_xmldata());
			    gchar *key_string = g_utf8_strdown ((gchar *)value, -1);
			    g_free (value);
			    gchar *hash_key = get_hash_key (key_string);
			    if(key_string) {
					NOOP("mime_hash_t::replacing hash element \"%s\" with key %s --> %s\n", 
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
	    fprintf(stderr, "mime_hash_t::hash table build (%p) for %s with \"%s\"-->\"%s\" is now complete.\n", 
		    T.hash, T.mimefile, key, xmldata);
	}


};

#endif
