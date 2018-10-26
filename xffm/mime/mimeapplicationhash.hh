#ifndef MIME_APPLICATION_HASH_HH
#define MIME_APPLICATION_HASH_HH
#include "mimehash.hh"

namespace xf {
template <class Type>
class MimeApplicationHash: public  MimeHash<Type>{
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

#ifdef PENDING
    // now load any previous user defined applications:
    //
    gchar *file=g_build_filename(USER_APPLICATIONS, NULL);
    DBG("mime-module, loading user defined applications from %s\n",file);
    FILE *config=fopen(file, "r");
    if (config) {
	gchar type[4096];
	while (fgets(type, 4096, config) && !feof(config)) {
	    char *s=strchr(type, '\n');
	    *s=0;
	    s=strchr(type, ':');
	    if (!s) continue;
	    *s=0;
	    const gchar *command=s+1;
	    add_type_to_hashtable(type, command, TRUE);
	}
	fclose(config);
    }
    g_free(file);
    
    
    NOOP("mime-module, hash table build is now complete.\n");
#endif
        }
};
}
#endif
