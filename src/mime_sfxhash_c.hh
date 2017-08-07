#ifndef MIME_SFXHASH_C_HH
#define MIME_SFXHASH_C_HH

#include "mime_hash_c.hh"

typedef struct txt_hash_t {
	    GHashTable *hash;
	    xmlDocPtr doc;
            gchar const *xmlkey;
            gchar const *xmldata;
            gchar const *xmlsubdata;
            pthread_mutex_t mutex;
}txt_hash_t;

template <class Type>
class mime_application_hash_c: public  mime_hash_c<Type>{
    public:
static gchar **lookup(const gchar *type, Type T){
    gchar **apps;

    //load_hashes ();
    ///  now look in hash...

    gchar *key = mime_hash_c<Type>::get_hash_key (type, T);
    pthread_mutex_lock(&T.mutex);
    apps = (gchar **)g_hash_table_lookup (T.hash, key);
    pthread_mutex_unlock(&T.mutex);
    g_free (key);
    if(apps) {
        gint i;
        for(i = 0; apps[i]; i++) ;
        gchar **a_apps = (gchar **) calloc ((i + 1), sizeof (gchar *));
        if (!a_apps) g_error("malloc: %s", strerror(errno));
        for(i = 0; apps[i]; i++) a_apps[i] = g_strdup (apps[i]);
        return a_apps;
    }
    return NULL;

}

static void
add(const gchar *key, const gchar *command, gboolean prepend, Type T){

    pthread_mutex_lock(&T.mutex);
    gchar **apps = (gchar **)g_hash_table_lookup (T.hash, key);

    if(!apps) {
        apps = (gchar **) malloc (2 * sizeof (gchar *));
	if (!apps) g_error("malloc: %s", strerror(errno));
	memset(apps, 0, 2 * sizeof (gchar *));
        *apps = g_strdup(command);
	g_hash_table_insert (T.hash, g_strdup(key), apps);
    } else {
        gint old_apps_count;
        gchar **old_apps = apps;
        for(old_apps_count = 0; old_apps[old_apps_count]; old_apps_count++) ;
        apps = (gchar **)malloc ((old_apps_count + 2) * sizeof (gchar *));
	if (!apps) g_error("malloc: %s", strerror(errno));
	memset(apps, 0, (old_apps_count + 2) * sizeof (gchar *));
	gint k = 0;
        if (prepend) {
	    *(apps) = g_strdup(command);
	    k = 1;
	}
	gint j;
	gboolean duplicate=FALSE;
        for(j = 0; j < old_apps_count; j++){
	    if (strcmp(command, old_apps[j]) == 0){ //
		duplicate=TRUE;
		NOOP ("mime-module,mime-module: duplicate command \"%s\"\n", command);
		if (prepend) continue;
	    }
	    apps[k] = g_strdup(old_apps[j]);
	    k++;
	}
        if (!prepend && !duplicate) {
	    apps[k++] = g_strdup(command);
	}   
	*(apps + k) = NULL;
	g_hash_table_replace (T.hash, g_strdup(key), apps);
    }
    pthread_mutex_unlock(&T.mutex);
    /*
    gint i;
    NOOP("(%d) %s: %s\n", prepend, type, command); fflush(NULL);
    for(i = 0; apps[i]; i++)
        NOOP(" %s", apps[i]);
    NOOP("\n");
    */

    NOOP("OPEN APPS: mime_write(%s)\n", type);

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

            gchar **apps = NULL;// apps may be null
            for(subnode = node->children; subnode; subnode = subnode->next) {
               if(xmlStrEqual (subnode->name, (const xmlChar *)T.xmlkey)) {
                    int i;
			    // application --> command --> 
                    value = xmlGetProp (subnode, (const xmlChar *)T.xmlsubdata);
                    if(value) {
                        if(!apps) {
                            i = 0;
                            apps = (gchar **)calloc (2, sizeof (gchar *));
			    if (!apps) g_error("calloc: %s", strerror(errno));
                        } else {
                            gchar **tmp = apps;
                            for(i = 0; apps[i]; i++) ;
                            apps = (gchar **)calloc ((i + 2), sizeof (gchar *));
			    if (!apps) g_error("calloc: %s", strerror(errno));
                            for(i = 0; tmp[i]; i++) apps[i] = tmp[i];
                            g_free (tmp);
                        }
                        apps[i] = (gchar *)value;
                    }
               }
            }
            if(apps) {
		gchar *type_key = mime_application_hash_c::get_hash_key (type, T);
                NOOP("mime-module, adding-%d : %s for %s (%s)\n", i, value, type, type_key);
                g_hash_table_replace (T.hash, type_key, apps);
            } 
            g_free(type);
        }
    }
    fprintf(stderr, "mime_application_hash_c::hash table build (%p) with \"%s\"-->\"%s\" is now complete.\n", 
            T.hash, T.xmlkey, T.xmlsubdata);
}
   
};

template <class Type>
class mime_command_hash_c: public  mime_hash_c<Type>{
    public:
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
                    int i;
			    // application --> command --> 
                    value = xmlGetProp (subnode, (const xmlChar *)T.xmldata);
                    if(value) {
                        xmlChar *extra_value;
			        // application --> command --> icon
                        extra_value = 
                            xmlGetProp (subnode, (const xmlChar *)T.xmlsubdata);
                        if(extra_value) {
                            gchar *k=mime_command_hash_c::get_hash_key ((gchar *)value, T);
                            NOOP("mime-module, adding- %s : %s\n", value, extra_value);
                            g_hash_table_replace (T.hash, k, extra_value);
                        }	
                    }
               }
            }
            g_free(type);
        }
    }
    fprintf(stderr, "mime_command_hash_c::hash table build (%p) with \"%s\"-->\"%s\"  -> \"%s\" is now complete.\n", 
            T.hash, T.xmlkey, T.xmldata, T.xmlsubdata);
}
   
};

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

template <class Type>
class mime_sfxhash_c: public  mime_hash_c<Type>{
    public:

static const gchar *
get_type_from_sfx(const gchar * file, Type T ) {
    const gchar *type;
    const gchar *p;
    //load_hashes ();
    TRACE("mime-module, locate_mime_t looking in sfx hash for \"%s\"\n", file);

    if (!strchr(file,'.')) return NULL;
    ///  now look in sfx hash...
    gchar *basename = g_path_get_basename (file);

    p = strchr (basename, '.');
    // Right to left:
    for (;p && *p; p = strchr (p, '.'))
    {
        while (*p=='.') p++;
        if (*p == 0) break;
        
        gchar *sfx;
        /* try all lower case (hash table keys are set this way) */
        sfx = g_utf8_strdown (p, -1);
        gchar *key = mime_sfxhash_c::get_hash_key (sfx,T);
        TRACE("mime-module, lOOking for \"%s\" with key=%s\n", sfx, key);

        type = (const gchar *)g_hash_table_lookup (T.hash, key);
        g_free (key);
        if(type) {
            NOOP(stderr,"mime-module, FOUND %s: %s\n", sfx, type);
            g_free (sfx);
            g_free (basename);
            return type;
        } 
        g_free (sfx);
    }
    // Left to right, test all extensions.
    gchar **q = g_strsplit(basename, ".", -1);
    gchar **q_p = q+1;
    
    for (;q_p && *q_p; q_p++){
	gchar *sfx;
	/* try all lower case (hash table keys are set this way) */
	sfx = g_utf8_strdown (*q_p, -1);
	gchar *key = mime_sfxhash_c::get_hash_key (sfx,T);
	type = (const gchar *)g_hash_table_lookup (T.hash, key);
	g_free (key);
	if(type) {
	    NOOP(stderr,"mime-module(2), FOUND %s: %s\n", sfx, type);
	    g_free (sfx);
	    g_free (basename);
	    g_strfreev(q);
	    return type;
	}
	g_free (sfx);
    }
    g_strfreev(q);
    g_free (basename);
    return NULL;
}


};

#endif
