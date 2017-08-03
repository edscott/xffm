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


class string_hash_c {
    public:
	string_hash_c(gchar *data0, gchar *data1, gchar *data2){
	    xmlkey = (data0)?g_strdup(data0):NULL;
	    xmldata = (data1)?g_strdup(data1):NULL;
	    xmlsubdata = (data2)?g_strdup(data2):NULL;
	}
	~string_hash_c(void){
	    g_free(xmlkey);
	    g_free(xmldata);
	    g_free(xmlsubdata);
	}
/*	void ((*get_data_free_f)(void *)){
	    return (g_free);
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
	gchar *xmlkey;
	gchar *xmldata;
	gchar *xmlsubdata;
};

template <class T>
class mime_hash_t {

    public:
	mime_hash_t(gchar *name){
	    hashname = g_strdup(name);
	    init(g_free);
	}
	~mime_hash_t(void){
	    g_free(hashname);
	    g_hash_table_destroy(hash);
	    pthread_mutex_destroy(&mutex);
	}
	    

    private:
	gboolean load_hash_from_cache(void){return FALSE;}
	
	void init(void (*f)){
	    pthread_mutex_init(&mutex, NULL);
	    hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
	    if(!load_hash_from_cache()) {
		std::cerr<<"mime_hash_t:: now building hash from scratch:"<<hashname<<"\n";
		build_hash();
		//generate_cache();
		//write_cache_sum (get_cache_sum ());
	    } else {
		std::cerr<<"mime_hash_t:: hash loaded from disk cache:"<<hashname<<"\n";
	    }
	    
	}
	pthread_mutex_t mutex;
	GHashTable *hash;
	gchar  *hashname;

	gchar *get_hash_key (const gchar * pre_key) {
	    GString *gs = g_string_new (pre_key);
	    gchar *key;
	    key = g_strdup_printf ("%10u", g_string_hash (gs));
	    g_string_free (gs, TRUE);
	    return key;
	}
	
	void build_hash (void) {
	    xmlChar *value;
	    xmlNodePtr node;
	    xmlNodePtr subnode;
	    xmlDocPtr doc;

	    //build hashes from system files
	    gchar *mimefile = g_build_filename (APPLICATION_MIME_FILE, NULL);

	    std::cerr<<"mime_hash_t::build_hash(): reading mime specification file="<<mimefile<<"\n";
	    if(access (mimefile, R_OK) != 0) {
		g_free (mimefile);
		DBG ("access(%s, R_OK)!=0 (%s)\n", mimefile, strerror(errno));
		return;
	    }
	    xmlKeepBlanksDefault (0);

	    if((doc = xmlParseFile (mimefile)) == NULL) {
		fprintf(stderr, "mime_hash_t:: Cannot parse XML file: %s. Replace this file.\n", mimefile);
		g_free (mimefile);
		return;
	    }

	    node = xmlDocGetRootElement (doc);
	    if(!xmlStrEqual (node->name, (const xmlChar *)"mime-info")) {
		fprintf(stderr, "mime_hash_t::Invalid XML file: %s (no mime-info). Replace this file.\n", mimefile);
		g_free (mimefile);
		xmlFreeDoc (doc);
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
			if(xmlStrEqual (subnode->name, (const xmlChar *)T->get_xmlkey())) {
			    // xmlkey --> xmldata
			    value = xmlGetProp (subnode, (const xmlChar *)T->get_xmldata());
			    gchar *key_string = g_utf8_strdown ((gchar *)value, -1);
			    g_free (value);
			    gchar *hash_key = get_hash_key (key_string);
			    if(key_string) {
					NOOP("mime_hash_t::replacing hash element \"%s\" with key %s --> %s\n", 
					    key_string, hash_key, type);
					g_hash_table_replace (hash, hash_key, g_strdup(type));
			    }
			    g_free (key_string);
			    continue;
			}
		    }
		    g_free(type);
		}
	    }
	    xmlFreeDoc (doc);
	    g_free (mimefile);	    
	    NOOP("mime_hash_t::hash table build is now complete.\n");
	}


};

#endif
