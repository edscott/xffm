#ifdef COPYRIGHT_INFORMATION
#include "gplv3.h"
#endif
/*
 *
 * Edscott Wilson Garcia 2001-2012 
 * <edscott@users.sf.net>
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

#ifndef S_ISSOCK
# ifdef S_IFSOCK
#  define S_ISSOCK(x) ((x&S_IFMT)==S_IFSOCK)
# else
#  define S_ISSOCK(x) FALSE
# endif
#else
#endif

#ifndef S_ISLNK
# ifdef S_IFLNK
#  define S_ISLNK(x) ((x&S_IFMT)==S_IFLNK)
# else
#  define S_ISLNK(x) FALSE
# endif
#endif

typedef struct mime_t {
    char *key;
    char *mimetype;
    char **apps;
} mime_t;

static GMutex *cache_mutex = NULL;


static GMutex *mimetype_hash_mutex = NULL;
static GHashTable *mimetype_hash= NULL;

static GMutex *alias_hash_mutex = NULL;
static GHashTable *alias_hash= NULL;

static GMutex *application_hash_mutex = NULL;
static GHashTable *application_hash_type = NULL;

// Read only hashes
static GHashTable *application_hash_sfx = NULL;
static GHashTable *application_hash_icon = NULL;
static GHashTable *application_hash_text = NULL;
static GHashTable *application_hash_text2 = NULL;
static GHashTable *application_hash_output = NULL;
static GHashTable *application_hash_output_ext = NULL;

static const gchar *locate_mime_t (const gchar * file);
static void destroy_application_hash_sfx (void);
static void destroy_application_hash_type (void);
static void mime_build_hashes (void);
static gpointer gencache (gpointer data);
/****************************************************************************/

static int
check_dir (char *path) {
    struct stat st;
    if(stat (path, &st) < 0) {
        if(mkdir (path, 0770) < 0)
            return FALSE;
        return TRUE;
    }
    if(S_ISDIR (st.st_mode)) {
        if(access (path, W_OK) < 0)
            return FALSE;
        return TRUE;
    }
    return FALSE;
}
#if 0
GSList *cleanup_list = NULL;
static void
clear_apps (gpointer data) {
    cleanup_list = g_slist_prepend(cleanup_list, data); 
    //TRACE("leak: should free apps 0x%x\n", GPOINTER_TO_INT(data)); return;
    //here we have a linux memory quirk with threads. we could hack a single pointer to 
    //a single memory block, but the cleanup_list hack is easier.
    //or without a hack, just free the array... after all, it only grows....
    //yeah, let's do that.
    //
    //gchar **apps = data;
    //g_strfreev(apps) //undetermined race...
   //for(; apps && *apps; apps++) g_free (*apps); g_free (data);
}
#endif

#if 0

static void
clear_sfx (gpointer key, gpointer value, gpointer user_data) {
    gchar *type = (gchar *) value;
    g_free (type);
    type = NULL;
}
#endif


static void
destroy_application_hash_sfx (void
    ) {
    if(!application_hash_sfx) return;
    g_hash_table_destroy (application_hash_sfx);
}

static void
destroy_application_hash_type (void
    ) {
    if(!application_hash_type) return;
    g_hash_table_destroy (application_hash_type);
}

static gchar *
get_hash_key (const gchar * pre_key) {
    GString *gs = g_string_new (pre_key);
    gchar *key;
    key = g_strdup_printf ("%10u", g_string_hash (gs));
    g_string_free (gs, TRUE);
    return key;
}

static void
add_type_to_hashtable(const gchar *type, const gchar *command, gboolean prepend){
    // Always use basic mimetype: avoid hashing alias mimetypes...
    gchar *hash_key=get_hash_key(type);
    g_mutex_lock(alias_hash_mutex);
    const gchar *basic_type = g_hash_table_lookup(alias_hash, hash_key);
    g_mutex_unlock(alias_hash_mutex);
    if (basic_type) type = basic_type;
    g_free(hash_key);

    gchar *key = get_hash_key (type);
    g_mutex_lock(application_hash_mutex);
    gchar **apps = g_hash_table_lookup (application_hash_type, key);

    if(!apps) {
        apps = (gchar **) malloc (2 * sizeof (gchar *));
	if (!apps) g_error("malloc: %s", strerror(errno));
	memset(apps, 0, 2 * sizeof (gchar *));
        *apps = g_strdup(command);
	g_hash_table_insert (application_hash_type, g_strdup(key), apps);
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
	g_hash_table_replace (application_hash_type, g_strdup(key), apps);
    }
    g_free(key);
    g_mutex_unlock(application_hash_mutex);
    /*
    gint i;
    NOOP("(%d) %s: %s\n", prepend, type, command); fflush(NULL);
    for(i = 0; apps[i]; i++)
        NOOP(" %s", apps[i]);
    NOOP("\n");
    */

    NOOP("OPEN APPS: mime_write(%s)\n", type);

}


static void
mime_build_hashes (void) {
    xmlChar *value;
    xmlNodePtr node;
    xmlNodePtr subnode;
    xmlDocPtr doc;
    gchar *mimefile = NULL;
    gchar **apps;


    //build hashes from system files
    mimefile = g_build_filename (APPLICATION_MIME_FILE, NULL);

    NOOP("mime-module, reading mime specification file=%s\n", mimefile);
    if(access (mimefile, R_OK) != 0) {
        g_free (mimefile);
        DBG ("access(%s, R_OK)!=0 (%s)\n", mimefile, strerror(errno));
        return;
    }
    xmlKeepBlanksDefault (0);

    if((doc = xmlParseFile (mimefile)) == NULL) {
        gchar *g = g_strconcat (mimefile, ".bak", NULL);
        DBG ("mime-module, invalid xml file %s.bak\n", mimefile);
        if (rename (mimefile, g)<0){
            fprintf(stderr, "mime_build_hashes(): rename %s->%s (%s)\n",mimefile, g,strerror(errno));
        }
        g_free (g);
        g_free (mimefile);
        return;
    }

    node = xmlDocGetRootElement (doc);
    if(!xmlStrEqual (node->name, (const xmlChar *)"mime-info")) {
        gchar *g = g_strconcat (mimefile, ".bak", NULL);
        DBG ("mime-module, invalid xml file %s.bak\n", mimefile);
        if (rename (mimefile, g)<0){
            fprintf(stderr, "rename(): %s --> %s (%s)\n", mimefile, g, strerror(errno));
        }
        g_free (g);
        g_free (mimefile);
        xmlFreeDoc (doc);
        return;
    }
    /* Now parse the xml tree */
    NOOP("mime-module, parsing %s\n", mimefile);
    for(node = node->children; node; node = node->next) {
        if(xmlStrEqual (node->name, (const xmlChar *)"mime-key")) {
            gchar *type_key = NULL;
            gchar *type;

            //  type has to be defined. 
            type = (gchar *)xmlGetProp (node, (const xmlChar *)"type");
            if(!type) {
		NOOP("mime-module, return on type==NULL\n");
                 return;
            }

            apps = NULL;
            // apps may be null
            for(subnode = node->children; subnode; subnode = subnode->next) {
                if(xmlStrEqual (subnode->name, (const xmlChar *)"key")) {
                    value = xmlGetProp (subnode, (const xmlChar *)"value");
		    gchar *sfx = g_utf8_strdown ((gchar *)value, -1);
		    g_free (value);
		    gchar *sfx_key = get_hash_key (sfx);
		    if(sfx_key) {
			NOOP("mime-module,replacing hash element \"%s\" with key %s --> %s\n", 
				sfx, sfx_key, type);
			g_hash_table_replace (application_hash_sfx, g_strdup(sfx_key), g_strdup(type));
		    }
		    g_free (sfx);
		    g_free (sfx_key);
		    continue;
		}
                if(xmlStrEqual (subnode->name, (const xmlChar *)"alias")) {
                    value = xmlGetProp (subnode, (const xmlChar *)"type");
		    gchar *alias_type = g_utf8_strdown ((gchar *)value, -1);
		    g_free (value);
		    gchar *alias_key = get_hash_key (alias_type);
		    if(alias_key) {
			NOOP("mime-module, inserting alias hash element %s with key %s --> %s\n", 
				alias_type, alias_key, type);
			g_hash_table_replace (alias_hash, g_strdup(alias_key), g_strdup(type));
		    }
		    g_free (alias_type);
		    g_free (alias_key);
		    continue;
		}
                if(xmlStrEqual (subnode->name, (const xmlChar *)"application")) {
                    int i;
                    value = xmlGetProp (subnode, (const xmlChar *)"command");
                    if(value) {
                        if(!apps) {
                            i = 0;
                            apps = (gchar **)malloc (2 * sizeof (gchar *));
			    if (!apps) g_error("malloc: %s", strerror(errno));
                            memset (apps, 0, 2 * sizeof (gchar *));
                        } else {
                            gchar **tmp = apps;
                            for(i = 0; apps[i]; i++) ;
                            apps = (gchar **)malloc ((i + 2) * sizeof (gchar *));
			    if (!apps) g_error("malloc: %s", strerror(errno));
                            memset (apps, 0, (i + 2) * sizeof (gchar *));
                            for(i = 0; tmp[i]; i++)
                                apps[i] = tmp[i];
                            g_free (tmp);
                        }
                        apps[i] = (gchar *)value;
                        xmlChar *extra_value;
                        extra_value = 
                            xmlGetProp (subnode, (const xmlChar *)"icon");
                        if(extra_value) {
                            gchar *k=get_hash_key ((gchar *)value);
                            NOOP("mime-module, adding- %s : %s\n", value, extra_value);
                            g_hash_table_replace (application_hash_icon, k, extra_value);
                        }
                        extra_value = 
                            xmlGetProp (subnode, (const xmlChar *)"text");
                        if(extra_value) {
                            gchar *k=get_hash_key ((gchar *)value);
                            NOOP("mime-module a, adding- %s : %s\n", value, extra_value);
			    g_hash_table_replace (application_hash_text, k, extra_value);
                        }
                        extra_value = 
                            xmlGetProp (subnode, (const xmlChar *)"text2");
                        if(extra_value) {
                            gchar *k=get_hash_key ((gchar *)value);
                            NOOP("mime-module b, adding- %s : %s\n", value, extra_value);
                            g_hash_table_replace (application_hash_text2, k, extra_value);
                        }
                        extra_value = 
                            xmlGetProp (subnode, (const xmlChar *)"output");
                        if(extra_value) {
                            gchar *k=get_hash_key ((gchar *)value);
                            NOOP("mime-module c, adding- %s : %s\n", value, extra_value);
                            g_hash_table_replace (application_hash_output, k, extra_value);
                  
                        }
                        extra_value = 
                            xmlGetProp (subnode, (const xmlChar *)"output_ext");
                        if(extra_value) {
                            gchar *k=get_hash_key ((gchar *)value);
                            NOOP("mime-module d, adding- %s : %s\n", value, extra_value);
                            g_hash_table_replace (application_hash_output_ext, k, extra_value);
                        }
                    }
                }
            }
            if(apps) {
		type_key = get_hash_key (type);
                NOOP("mime-module, adding-%d : %s for %s (%s)\n", i, value, type, type_key);
                g_hash_table_replace (application_hash_type, type_key, apps);
            } 
	    g_free(type);
        }
    }
    xmlFreeDoc (doc);
    g_free (mimefile);
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
}

static gchar *inode[]={
    "inode/socket",
    "inode/blockdevice",
    "inode/chardevice",
    "inode/fifo",
    "inode/directory",
    "unknown"
};

enum {
    INODE_SOCKET,
    INODE_BLOCKDEVICE,
    INODE_CHARDEVICE,
    INODE_FIFO,
    INODE_DIRECTORY,
    INODE_UNKNOWN
};

static const gchar *
mimeable_file (struct stat *st_p) {
    const gchar *result = NULL;
#ifdef S_IFWHT
    if(st_p->st_mode == S_IFWHT) {
        NOOP("mime-module, S_IFWHT file!\n");
        return NULL;
    }
#endif
    if(S_ISSOCK (st_p->st_mode))
        result = inode[INODE_SOCKET];
    else if(S_ISBLK (st_p->st_mode))
        result = inode[INODE_BLOCKDEVICE];
    else if(S_ISCHR (st_p->st_mode))
        result = inode[INODE_CHARDEVICE];
    else if(S_ISFIFO (st_p->st_mode))
        result = inode[INODE_FIFO];
    //else if (S_ISLNK(st_p->st_mode)) result= "inode/symlink";
    else if(S_ISDIR (st_p->st_mode))
        result = inode[INODE_DIRECTORY];
    else
        return NULL;
    return result;
}

static gchar *
get_cache_path (char *which) {
    gchar *cache_path = NULL;
    gchar *cache_dir;

    cache_dir = g_build_filename (USER_DBH_CACHE_DIR, NULL);

    if(!check_dir (cache_dir)) {
        g_free (cache_dir);
        return NULL;
    }
    cache_path = g_strdup_printf ("%s%cmime.%s.cache64.dbh", cache_dir, G_DIR_SEPARATOR, which);
    g_free (cache_dir);
    NOOP("mime-module, using cache: %s\n", cache_path);
    return cache_path;
}

static void
add2cache_type (gpointer key, gpointer value, gpointer user_data) {
    gchar **apps = (gchar **)value;
    gint length;

    DBHashTable *cache = (DBHashTable *) user_data;
    if(!value || !cache) return;

    NOOP("mime-module, adding %s cache\n", (gchar *)key);
    memset (DBH_KEY (cache), 0, DBH_KEYLENGTH (cache));
    sprintf ((gchar *)DBH_KEY (cache), "%s", (gchar *)key);

    gchar *string = NULL;
    /* figure out record length */
    gchar **pp = apps;
    for (;pp && *pp; pp++){
        if(!string) {
            string = g_strconcat (*pp, "@", NULL);
        } else {
	    gchar *old_string = string;
            string = g_strconcat (old_string, *pp, "@", NULL);
            g_free (old_string);
        }
    }


    length = strlen (string) + 1;

    NOOP("mime-module, cache adding %s --> %s\n", (gchar *)key, string);

    dbh_set_recordsize (cache, length);
    memcpy (cache->data, string, length);
    g_free(string);
    dbh_update (cache);
    return;
}

static void
add2cache_sfx (gpointer key, gpointer value, gpointer user_data) {
    int length;

    DBHashTable *cache = (DBHashTable *) user_data;
    if(!value || !cache)
        return;

    NOOP("mime-module, adding %s (%s) to sfx cache\n", (char *)key, (char *)value);
    memset (DBH_KEY (cache), 0, DBH_KEYLENGTH (cache));
    sprintf ((char *)DBH_KEY (cache), "%s", (char *)key);

    /* figure out record length */
    length = strlen (value) + 1;

    dbh_set_recordsize (cache, length);

    memcpy (cache->data, value, length);
    dbh_update (cache);
    return;
}

static void
add2cache_text (gpointer key, gpointer value, gpointer user_data) {
        FILE *cache=user_data;
        fprintf(cache,"%s:%s\n", (gchar *)key, (gchar *)value);
    return;
}

static void 
save_text_cache(GHashTable *hash_table, const gchar *filename) {
    gchar *file=g_build_filename(USER_RFM_CACHE_DIR, filename, NULL);
    FILE *cache=fopen(file, "w");
    if (!cache) {
        DBG("unable to create %s\n",file);
        g_free(file);
        return;
    }
    g_free(file);
    g_hash_table_foreach (hash_table, add2cache_text, (gpointer) cache);
    fclose(cache);
}

static void 
load_text_hash(GHashTable *hash_table, const gchar *filename){
    gchar *file=g_build_filename(USER_RFM_CACHE_DIR, filename, NULL);
    FILE *cache=fopen(file, "r");
    if (!cache) {
        DBG("unable to open %s for read\n",file);
        g_free(file);
        return;
    }
    g_free(file);
    gchar buffer[4096];
    while (fgets(buffer, 4096, cache) && !feof(cache)) {
        char *s=strchr(buffer, '\n');
        *s=0;
        s=strchr(buffer, ':');
        if (!s) continue;
        *s=0;
        gchar *key=g_strdup(buffer);
        gchar *value=g_strdup(s+1);
        g_hash_table_replace (hash_table, (gpointer) key, (gpointer) value);
    }
    fclose(cache);
}

static gboolean
generate_caches (void) {
    DBHashTable *cache;
    if(!application_hash_sfx || !application_hash_type) {
        DBG ("cannot build cache without application_hashes\n");
        return FALSE;
    }

    save_text_cache(application_hash_icon,"application_hash_icon");
    save_text_cache(application_hash_text,"application_hash_text");
    save_text_cache(application_hash_text2,"application_hash_text2");
    save_text_cache(application_hash_output,"application_hash_output");
    save_text_cache(application_hash_output_ext,"application_hash_output_ext");
    save_text_cache(alias_hash,"alias_hash");

    gchar *cache_path = get_cache_path ("sfx");
    if(!cache_path) {
        DBG ("!cache_path sfx\n");
        return FALSE;
    }
    gchar *tmp_cache_path = g_strdup_printf("%s-%d", 
	    cache_path, (gint)getpid());

    NOOP("mime-module, creating cache file: %s", cache_path);
    unsigned char keylength=11;
    gchar *directory = g_path_get_dirname(tmp_cache_path);
    if (!g_file_test(directory, G_FILE_TEST_IS_DIR)){
        g_mkdir_with_parents(directory, 0700);
    }
    g_free(directory);
    TRACE("opening %s...\n",tmp_cache_path); 
    cache = dbh_new (tmp_cache_path, &keylength, DBH_CREATE|DBH_PARALLEL_SAFE);
    TRACE("opened %s.\n",tmp_cache_path); 

    if(cache == NULL) {
	DBG ("Could not create cache table: %s\n", tmp_cache_path);
	g_free (tmp_cache_path);
	g_free (cache_path);
	return FALSE;
    }
    dbh_set_parallel_lock_timeout(cache, 3);
 
    g_hash_table_foreach (application_hash_sfx, add2cache_sfx, (gpointer) cache);
    NOOP("mime-module, generated cache %s with %d records\n", cache_path, g_hash_table_size (application_hash_sfx));
    dbh_regen_sweep (&cache);
    dbh_close (cache);
    if (rename(tmp_cache_path, cache_path) < 0){
	DBG("rename(%s, %s) failed: %s\n",
		tmp_cache_path, cache_path, strerror(errno));
    }
    g_free (cache_path);
    g_free (tmp_cache_path);

    cache_path = get_cache_path ("type");
    if(!cache_path) {
        DBG ("!cache_path type\n");
        return FALSE;
    }
    tmp_cache_path = g_strdup_printf("%s-%d", 
	    cache_path, (gint)getpid());

    NOOP("mime-module, Creating cache file: %s", cache_path);
    directory = g_path_get_dirname(tmp_cache_path);
    if (!g_file_test(directory, G_FILE_TEST_IS_DIR)){
        g_mkdir_with_parents(directory, 0700);
    }
    g_free(directory);
    TRACE("opening %s...\n",tmp_cache_path); 
    cache = dbh_new (tmp_cache_path, &keylength, DBH_CREATE|DBH_PARALLEL_SAFE);
    TRACE("opened %s.\n",tmp_cache_path); 
    if(cache == NULL) {
	DBG ("Could not create cache table: %s\n", tmp_cache_path);
	g_free (tmp_cache_path);
	g_free (cache_path);
	return FALSE;
    }
    dbh_set_parallel_lock_timeout(cache, 3);

    g_mutex_lock(application_hash_mutex);
    g_hash_table_foreach (application_hash_type, add2cache_type, (gpointer) cache);
    g_mutex_unlock(application_hash_mutex);

    NOOP("mime-module, generated cache %s with %d records\n", cache_path, g_hash_table_size (application_hash_type));
    dbh_regen_sweep (&cache);
    dbh_close (cache);
    if (rename(tmp_cache_path, cache_path) < 0){
	DBG("rename(%s, %s) failed: %s\n",
		tmp_cache_path, cache_path, strerror(errno));
    }
    g_free (tmp_cache_path);
    g_free (cache_path);

    return TRUE;
}


void
add2sfx_hash (DBHashTable * cache) {
    gchar *sfx_key = (gchar *) malloc (DBH_KEYLENGTH (cache));
    if (!sfx_key) g_error("malloc: %s", strerror(errno));
    memset(sfx_key, 0, DBH_KEYLENGTH (cache));
    memcpy (sfx_key, DBH_KEY (cache), DBH_KEYLENGTH (cache));
    gchar *type = (gchar *) malloc (DBH_RECORD_SIZE (cache));
    if (!type) g_error("malloc: %s", strerror(errno));
    memcpy (type, DBH_DATA (cache), DBH_RECORD_SIZE (cache));
    NOOP("mime-module, loading cache element %s -> %s\n", sfx_key, type);
    g_hash_table_replace (application_hash_sfx, sfx_key, type);
}

void
add2type_hash (DBHashTable * cache) {
    gchar *type_key = (gchar *) malloc (DBH_KEYLENGTH (cache));
    if (!type_key) g_error("malloc: %s", strerror(errno));
    memset(type_key, 0, DBH_KEYLENGTH (cache));
    memcpy (type_key, DBH_KEY (cache), DBH_KEYLENGTH (cache));
    //gchar *string = g_strdup ((gchar *) DBH_DATA (cache));
    const gchar *string = DBH_DATA (cache);

    gint count = 0;
    gint i;
    for(i = 0; i < strlen (string); i++){
        if(string[i] == '@'){
            count++;
	}
    }
    if(!count) {
        DBG ("Apparent cache corruption. Please delete \"history\" in user preferences dialog (sysmsg: add2type_hash() count==0)\n");
        g_free(type_key);
        return;
    }

    gchar **apps = g_strsplit(string, "@", -1);
#if 0
    gchar **apps = (gchar **) malloc ((count + 1) * sizeof (gchar *));
    if (!apps) g_error("malloc: %s", strerror(errno));
    memset (apps, 0, (count + 1) * sizeof (gchar *));

    NOOP("mime-module, %d loading cache type element %s -> %s\n", count, type_key, string);

    for(i = 0; i < count; i++) {

        apps[i] = strtok ((i == 0) ? string : NULL, "@");
    }
    apps[count] = NULL;
#endif

#if 0
    TRACE("mime-module, hash table replacing apps:");
    for(i = 0; i < count; i++) TRACE("\"%s\"", apps[i]); TRACE("\n");
#endif
    g_hash_table_replace (application_hash_type, (gpointer) type_key, (gpointer) apps);
}

static long long
get_cache_sum (void) {
    long long sum = 0;
    gchar *mimefile;
    mimefile = g_build_filename (APPLICATION_MIME_FILE, NULL);
    struct stat st;
    if (stat (mimefile, &st)==0){
        sum += (st.st_dev + st.st_ino + st.st_mode + st.st_nlink
                + st.st_uid + st.st_gid + st.st_rdev + st.st_size + st.st_blksize + st.st_blocks + st.st_mtime + st.st_ctime);
    }
    g_free(mimefile);

    return sum;
}

static void
write_cache_sum (long long sum) {
    FILE *file;
    gchar *infofile = get_cache_path ("info");
    file = fopen (infofile, "w");
    if(!file) {
        NOOP("mime-module, cannot open file %s\n", infofile);
        g_free (infofile);
        return;
    }

    if(fwrite (&sum, sizeof (long long), 1, file) != 1) {
        DBG ("cannot write to file %s\n", infofile);
    }
    fclose (file);
    NOOP("mime-module, wrote %lld to %s\n", sum, infofile);
    g_free (infofile);
    return;
}

static long long
read_cache_sum (void
    ) {
    FILE *file;
    long long sum = 0;
    gchar *infofile = get_cache_path ("info");
    file = fopen (infofile, "r");
    if(!file) {
        NOOP("mime-module, cannot open file %s\n", infofile);
        g_free (infofile);
        return sum;
    }

    if(fread (&sum, sizeof (long long), 1, file) != 1) {
        DBG ("cannot read from file %s\n", infofile);
    }
    fclose (file);
    g_free (infofile);
    return sum;
}

static gboolean
load_hashes_from_cache (void) {
    // test for cache regeneration
    // on regeneration we must:
    // 1- read cache
    // 2- update records to be updated from system installed association file
    //

    // step 1.

    long long sum = read_cache_sum ();
    long long actual_sum = get_cache_sum ();
    gboolean cache_ok = TRUE;
//#define NO_MIME_CACHE
#ifdef NO_MIME_CACHE
    NOOP ("mime-module,NO_MIME_CACHE\n");
    return FALSE;
#endif
    if(actual_sum != sum) {
        NOOP ("mime-module,OPEN-CACHE: regenerating mime-module caches %lld != %lld\n", sum, actual_sum);
        return FALSE;
    } else {
        NOOP ("mime-module,OPEN-CACHE: mime-module caches are up to date\n");
    }


    gchar *cache_path = get_cache_path ("sfx");
    DBHashTable *cache;
    TRACE("opening %s...\n",cache_path); 
    cache = dbh_new (cache_path, NULL, DBH_PARALLEL_SAFE);
    TRACE("opened %s.\n",cache_path); 
    g_free (cache_path);
    if(!cache)
        goto failed;
    dbh_set_parallel_lock_timeout(cache, 3);

    dbh_foreach_sweep (cache, add2sfx_hash);
    dbh_close (cache);

    cache_path = get_cache_path ("type");
    TRACE("opening %s...\n",cache_path); 
    cache = dbh_new (cache_path, NULL, DBH_PARALLEL_SAFE);
    TRACE("opened %s.\n",cache_path); 
    g_free (cache_path);
    if(!cache)
        goto failed;
    dbh_set_parallel_lock_timeout(cache, 3);

    dbh_foreach_sweep (cache, add2type_hash);
    dbh_close (cache);

    load_text_hash(application_hash_icon, "application_hash_icon");
    load_text_hash(application_hash_text, "application_hash_text");
    load_text_hash(application_hash_text2, "application_hash_text2");
    load_text_hash(application_hash_output, "application_hash_output");
    load_text_hash(application_hash_output_ext, "application_hash_output_ext");
    load_text_hash(alias_hash, "alias_hash");
    return cache_ok;

  failed:
    destroy_application_hash_sfx ();
    destroy_application_hash_type ();
    application_hash_sfx = NULL;
    application_hash_type = NULL;
    return FALSE;
}

static gpointer
gencache (gpointer data) {
    NOOP("mime-module, gencache (%s)\n", (char *)data);
    g_mutex_lock (cache_mutex);
    if (data && strchr((char *)data,':')) {
	gchar *file=g_build_filename(USER_APPLICATIONS, NULL);
	gchar *newfile=g_build_filename(USER_APPLICATIONS".new", NULL);
        gchar *location = g_path_get_dirname(newfile);
        if (!g_file_test(location, G_FILE_TEST_IS_DIR) ){
            if (!g_mkdir_with_parents(location, 0600)){
                g_warning("Cannot create config directory: %s\n", location);
                g_free(location);
                return NULL;
            }
        }
        g_free(location);
	gchar b[4096];
	FILE *config=fopen(file, "r");
        if (!config)
            DBG("mimemodule.i: gencache(). Cannot open %s for read\n", file);
	FILE *newconfig=fopen(newfile, "w");
        if (!newconfig)
            DBG("mimemodule.i: gencache(). Cannot open %s for write\n", newfile);
	if (config) {
	    while (fgets(b, 4096, config) && !feof(config)) {
	      if (strchr(b,'\n')) *strchr(b,'\n')=0;
	      g_strstrip(b);
	      if (strcmp(b, (char *)data)) {
		if (newconfig) fprintf(newconfig,"%s\n", b);
	      }
	    }
	    fclose(config);
	}
	if(newconfig) {
            fprintf(newconfig,"%s\n", (char *)data);
	    fclose(newconfig);
	    if (rename(newfile, file)<0){
                fprintf(stderr, "gencache(): rename %s->%s (%s)\n",newfile, file,strerror(errno));
            }
        }

	g_free(file);
	g_free(newfile);
	g_free(data); //disposible data
    }
    generate_caches ();
    write_cache_sum (get_cache_sum ());
    g_mutex_unlock (cache_mutex);
    return NULL;
}

static const gchar *
locate_mime_t (const gchar * file) {
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
        gchar *key = get_hash_key (sfx);
        TRACE("mime-module, lOOking for \"%s\" with key=%s\n", sfx, key);

        type = g_hash_table_lookup (application_hash_sfx, key);
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
	gchar *key = get_hash_key (sfx);
	type = g_hash_table_lookup (application_hash_sfx, key);
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

static gchar **
locate_apps (const gchar * type) {
    gchar **apps;

    //load_hashes ();
    ///  now look in hash...

    gchar *key = get_hash_key (type);
    g_mutex_lock(application_hash_mutex);
    apps = g_hash_table_lookup (application_hash_type, key);
    g_mutex_unlock(application_hash_mutex);
    g_free (key);
    if(apps) {
        gint i;
        for(i = 0; apps[i]; i++) ;
        gchar **a_apps = (gchar **) malloc ((i + 1) * sizeof (gchar *));
	if (!a_apps) g_error("malloc: %s", strerror(errno));
        memset (a_apps, 0, (i + 1) * sizeof (gchar *));
        for(i = 0; apps[i]; i++) {
            a_apps[i] = g_strdup (apps[i]);
        }
        return a_apps;
    }
    return NULL;
}


