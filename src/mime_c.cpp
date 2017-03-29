#include "mime_c.hpp"
// gcc -std=c++11 mime_c.cpp `pkg-config gtk+-3.0 --cflags` `pkg-config dbh2 --cflags` `pkg-config --cflags libxml-2.0`
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>
#include <errno.h>
#include <dbh.h>

#ifndef PACKAGE_DATA_DIR
#warning "PACKAGE_DATA_DIR not defined"
#define PACKAGE_DATA_DIR ""
#endif

typedef struct mime_t {
    char *key;
    char *mimetype;
    char **apps;
} mime_t;

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



static void free_apps(void *);
static void add2type_hash (DBHashTable *, void *);
static void add2sfx_hash (DBHashTable *, void *);
static void add2cache_text (gpointer, gpointer, gpointer);
static void add2cache_type (gpointer, gpointer, gpointer);
static void add2cache_sfx (gpointer, gpointer, gpointer);
static void *gencache (void *);
static void write_cache_sum (long long);
static long long get_cache_sum (void);
static gchar *get_cache_path (const gchar *);
static gint check_dir (char *);

mime_c::mime_c (data_c *data0) {
    data_p = data0;

    if(!load_hashes_from_cache()) {
        DBG("mime_c:: now building hashes from scratch\n");
        mime_build_hashes ();
        generate_caches();
        write_cache_sum (get_cache_sum ());
    } else {
        DBG("mime_c:: hashes loaded from disk cache\n");
    }
    return;
}




mime_c::~mime_c (void){
}



gchar *
mime_c::mime_magic(const gchar *file){
    gchar *unalias = mime_magic_unalias(file);
    gchar *alias = mime_get_alias_type(unalias);
    g_free(unalias);
    return alias;
}



 
const gchar *
mime_c::find_mimetype_in_hash(const gchar *file){
    const gchar *type=NULL; 
#ifndef NO_MIMETYPE_HASH
    if (!data_p->mimetype_hash) return type;
    gchar *key = get_hash_key (file);
    pthread_mutex_lock(&data_p->mimetype_hash_mutex);
    type = (const gchar *)g_hash_table_lookup (data_p->mimetype_hash, key);
    pthread_mutex_unlock(&data_p->mimetype_hash_mutex);
    g_free (key);
#endif
    return type;
}

// This function will return a basic mimetype, never an alias.
 
gchar *
mime_c::mime_type (const gchar *file){
    if (!file) return NULL;
#ifndef NO_MIMETYPE_HASH
    const gchar *old_mimetype = find_mimetype_in_hash(file);
    if (old_mimetype) {
	// already tabulated. Just return previous value.
	return g_strdup(old_mimetype);
    }
#endif
    if(file[strlen (file) - 1] == '~' || file[strlen (file) - 1] == '%') {
	gchar *r_file = g_strdup(file);
	r_file[strlen (r_file) - 1] = 0;
	gchar *retval = mime_type(r_file);
	g_free(r_file);
	return retval;
    }
    gchar *retval = mimetype1(file);
    if (retval) return retval;  
    return mimetype2(file);
}
    
 
gchar *
mime_c::mime_type (const gchar *file, struct stat *st_p) {
    if (!file) return NULL;
#ifndef NO_MIMETYPE_HASH
    const gchar *old_mimetype = find_mimetype_in_hash(file);
    if (old_mimetype) {
	// already tabulated. Just return previous value.
	return g_strdup(old_mimetype);
    }
#endif
    if(file[strlen (file) - 1] == '~' || file[strlen (file) - 1] == '%') {
	gchar *r_file = g_strdup(file);
	r_file[strlen (r_file) - 1] = 0;
	gchar *retval = mime_type(r_file, st_p);
	g_free(r_file);
	return retval;
    }
    gchar *retval = mimetype1(file);
    if (retval) return retval;  
    
    // Get a mimetype from the stat information, if this applies.
    //


    gboolean try_extension = TRUE;
    // If stat information is not provided, then stat the item.
    struct stat st_v;
    memset(&st_v, 0, sizeof(struct stat));
    if(!st_p) {
        st_p = &st_v;
        if (stat (file, st_p) < 0) {
            try_extension = FALSE;
	}
    }

    if (try_extension){
        const gchar *type = mimeable_file (st_p);
        if(type) {
            put_mimetype_in_hash(file, type);
            NOOP ("MIME: stat mime_type(%s) -> %s\n", file, type);
            return g_strdup(type);
        }

        // Empty files (st_ino is there to make sure we do not have an empty stat):
        if (st_p->st_size == 0 && st_p->st_ino != 0) {
            return g_strdup("text/plain");
        }
    }

    retval = mimetype2(file);
    if (retval) return retval;  

    // 
    // Empty files (st_ino is there to make sure we do not have an empty stat):
    if (st_p->st_size == 0 && st_p->st_ino != 0) {
	return g_strdup("text/plain");
    }

    return mime_magic(file);

}


gchar *
mime_c::mime_function(const gchar *path, const gchar *function) {
    if (!path || !function) return NULL;

    if (strcmp(function, "mime_file")==0) {
	return g_strdup(mime_file(path));
    }
    if (strcmp(function, "mime_encoding")==0) {
	return mime_encoding(path);
    }
    if (strcmp(function, "mime_magic")==0) {
	return mime_magic(path);
    }
    return NULL;
}





 
gboolean mime_c::mime_is_valid_command (const char *cmd_fmt) {
    //return GINT_TO_POINTER(TRUE);
    NOOP ("MIME: mime_is_valid_command(%s)\n", cmd_fmt);
    GError *error = NULL;
    int argc;
    gchar *path;
    gchar **argv;
    if(!cmd_fmt)
        return  (FALSE);
    if(!g_shell_parse_argv (cmd_fmt, &argc, &argv, &error)) {
        gchar *msg = g_strcompress (error->message);
        DBG ("%s: %s\n", msg, cmd_fmt);
        g_error_free (error);
        g_free (msg);
        return  (FALSE);
    }
    gchar **ap = argv;
    if (*ap==NULL) {
        errno = ENOENT;
        return  (FALSE);
    }

    // assume command is correct if environment is being set
    if (strchr(*ap, '=')){
        g_strfreev (argv);
        return  (TRUE);
    }

    path = g_find_program_in_path (*ap);
    if(!path) {
        gboolean direct_path = g_file_test (argv[0], G_FILE_TEST_EXISTS) ||
            strncmp (argv[0], "./", strlen ("./")) == 0 || strncmp (argv[0], "../", strlen ("../")) == 0;
        //DBG("argv[0]=%s\n",argv[0]);
        if(direct_path) {
            path = g_strdup (argv[0]);
        }
    }
    NOOP ("mime_is_valid_command(): g_find_program_in_path(%s)=%s\n", argv[0], path);

    //if (!path || access(path, X_OK) != 0) {
    if(!path) {
        g_strfreev (argv);
        errno = ENOENT;
        return  (FALSE);
    }
    // here we test for execution within sudo
    // XXX we could also check for commands executed in a terminal, but not today...
    gboolean retval=(TRUE);
    if (strcmp(argv[0],"sudo")==0) {
        int i=1;
        if (strcmp(argv[i],"-A")==0) i++;
        retval=mime_is_valid_command(argv[i]);
    }

    g_strfreev (argv);
    g_free (path);
    return retval;
}

 
const gchar *
mime_c::mime_command_text (gchar *p) {
    NOOP("mime_command_text()...\n");
    if (!p) return NULL;
    gchar *key=get_hash_key_strstrip (p);
    const gchar *value=(const gchar *)g_hash_table_lookup (data_p->application_hash_text, key);
    g_free(key);
    return value;
}

 
const gchar *
mime_c::mime_command_text2 (gchar *p) {
    NOOP("mime_command_text2()...\n");
    if (!p) return NULL;
    gchar *key=get_hash_key_strstrip (p);
    const gchar *value=(const gchar *)g_hash_table_lookup (data_p->application_hash_text2, key);
    g_free(key);
    return value;
}

 
const gchar *
mime_c::mime_command_icon (gchar *p) {
    NOOP("mime_command_icon()...\n");
    if (!p) return NULL;
    gchar *key=get_hash_key_strstrip (p);
    const gchar *value=(const gchar *)g_hash_table_lookup (data_p->application_hash_icon, key);
    g_free(key);
    return value;
}


const gchar *
mime_c::mime_command_output (gchar *p) {
    NOOP("mime_command_output()...\n");
    if (!p) return NULL;
    gchar *key=get_hash_key_strstrip (p);
    const gchar *value=(const gchar *)g_hash_table_lookup (data_p->application_hash_output, key);
    g_free(key);
    return value;
}
 
const gchar *
mime_c::mime_command_output_ext (gchar *p) {
    NOOP("mime_command_output_ext()...\n");
    if (!p) return NULL;
    gchar *key=get_hash_key_strstrip (p);
    const gchar *value=(const gchar *)g_hash_table_lookup (data_p->application_hash_output_ext, key);
    g_free(key);
    return value;
}

    

gchar *
mime_c::mime_command (const char *type) {
    NOOP ("APPS: mime_command(%s)\n", type);
    gchar **apps;
    int i;
    gchar *cmd_fmt = NULL;
    apps = locate_apps (type);
    if(!apps) {
        NOOP ("APPS: --> NULL\n");
        return NULL;
    }
    if(!apps[0]) {
        NOOP ("APPS: --> NULL\n");
        g_free (apps);
        return NULL;
    }

    for(i = 0; apps[i]; i++) {
        g_free (cmd_fmt);
        cmd_fmt = g_strcompress (apps[i]);
        if(mime_is_valid_command (cmd_fmt)) {
            g_strfreev (apps);
            NOOP ("APPS: --> %s\n", cmd_fmt);
            return cmd_fmt;
        }
    }
    g_free (cmd_fmt);
    g_strfreev (apps);
    NOOP ("APPS: --> NULL\n");
    return NULL;
}


gchar **
mime_c::mime_apps (const char *type) {
    NOOP("mime_apps()...\n");
    NOOP ("MIME: mime_apps(%s)\n", type);
    gchar **apps;
    apps = locate_apps (type);
    if(!apps)
        return NULL;
    if(!apps[0]) {
        g_free (apps);
        return NULL;
    }
    return apps;
}


// Insert a command to a mimetype. This will regenerate the disk
// cache.
 
void *
mime_c::mime_add (gchar *type, gchar *q) {
    NOOP("mime_add()...\n");
    gchar *command = g_strdup(q);
    g_strstrip(command);
    if(!command || !strlen (command)){
	g_free(command);
        return NULL;
    }

    NOOP ("OPEN APPS: adding type %s->%s\n", type, command);
    add_type_to_hashtable(type, command, TRUE);
  

    // thread will dispose of config_command:
    gchar *config_command=g_strdup_printf("%s:%s", type, command);
    g_free(command);
    pthread_t thread;
    pthread_create(&thread, NULL, gencache, (void *)config_command); 
    pthread_detach(thread);
    return NULL;
}

// Append a command to a mimetype. This will not regenerate the disk
// cache, (see dotdesktop module for the reason why not)
 
void *
mime_c::mime_append (gchar *type, gchar *q) {
    gchar *command = g_strdup(q);
    g_strstrip(command);
    if(!command || !strlen (command)){
	g_free(command);
        return NULL;
    }
    NOOP ("OPEN APPS: appending type %s->%s\n", type, command);
    pthread_mutex_lock (&data_p->cache_mutex);
    add_type_to_hashtable(type, command, FALSE);
    pthread_mutex_unlock (&data_p->cache_mutex);
    g_free(command);
    return NULL;
}


gchar *
mime_c::mime_mk_command_line (const gchar *command_fmt, const gchar *path) {
    NOOP("mime_mk_command_line()...\n");

    NOOP ("MIME: mime_mk_command_line(%s)\n", path);
    gchar *command_line = NULL;
    gchar *fmt = NULL;

    if(!command_fmt)
        return NULL;
    if(!path)
        path = "";

    NOOP ("MIME: command_fmt=%s\n", command_fmt);

    /* this is to send path as an argument */

    if(strstr (command_fmt, "%s")) {
        fmt = g_strdup (command_fmt);
    } else {
        fmt = g_strconcat (command_fmt, " %s", NULL);
    }
    NOOP ("MIME: command_fmt fmt=%s\n", fmt);

    NOOP ("MIME: path=%s\n", path);
    gchar *esc_path = esc_string (path);
    command_line = g_strdup_printf (fmt, esc_path);
    g_free (esc_path);
    NOOP ("MIME2: command_line=%s\n", command_line);

    g_free (fmt);
    return command_line;
}

 
gchar *
mime_c::mime_mk_terminal_line (const gchar *command) {
    NOOP("mime_mk_terminal_line()...\n");
    NOOP ("MIME: mime_mk_command_line(%s)\n", command);
    gchar *command_line = NULL;

    if(!command)
        return NULL;

    const gchar *term = what_term ();
    const gchar *exec_flag = term_exec_option(term);
    /*
    // Validation is already done by rfm_what_term
    if(!mime_is_valid_command ((void *)term)) {
        DBG ("%s == NULL\n", term);
        return NULL;
    }*/
    command_line = g_strdup_printf ("%s %s %s", term, exec_flag, command);

    return command_line;
}


gchar *
mime_c::mime_get_alias_type(const gchar *type){
    if(type) {
	gchar *hash_key=get_hash_key(type);
	pthread_mutex_lock (&data_p->alias_hash_mutex);
	const gchar *basic_type = (const gchar *)g_hash_table_lookup(data_p->alias_hash, hash_key);
	pthread_mutex_unlock (&data_p->alias_hash_mutex);
	g_free(hash_key);
	if (basic_type) return g_strdup(basic_type);
	return g_strdup(type);
    } 
    return g_strdup(inode[INODE_UNKNOWN]);
}
 

///////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////


gboolean
mime_c::load_hashes_from_cache (void) {
    // FIXME: currently broken.
    return FALSE;

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
    DBG ("mime-module,NO_MIME_CACHE\n");
    return FALSE;
#endif
    if(actual_sum != sum) {
        DBG ("mime-module,OPEN-CACHE: regenerating mime-module caches %lld != %lld\n", sum, actual_sum);
        return FALSE;
    } else {
        NOOP ("mime-module,OPEN-CACHE: mime-module caches are up to date\n");
    }


    gchar *cache_path = get_cache_path ("sfx");
    DBHashTable *cache;
    cache = dbh_new (cache_path, NULL, DBH_PARALLEL_SAFE);
    DBG("mime_c::load_hashes_from_cache: opened %s-->%p\n",cache_path, cache); 
    g_free (cache_path);
    if(!cache) {
        goto failed;
    }
    dbh_set_parallel_lock_timeout(cache, 3);

    dbh_foreach (cache, add2sfx_hash, (void *)data_p->application_hash_sfx);
    dbh_close (cache);

    cache_path = get_cache_path ("type");
    cache = dbh_new (cache_path, NULL, DBH_PARALLEL_SAFE);
    DBG("mime_c::load_hashes_from_cache: opened %s-->%p\n",cache_path, cache); 
    g_free (cache_path);
    if(!cache) {
        goto failed;
    }
    dbh_set_parallel_lock_timeout(cache, 3);

    dbh_foreach (cache, add2type_hash, (void *)data_p->application_hash_type);
    dbh_close (cache);

    load_text_hash(data_p->application_hash_icon, "application_hash_icon");
    load_text_hash(data_p->application_hash_text, "application_hash_text");
    load_text_hash(data_p->application_hash_text2, "application_hash_text2");
    load_text_hash(data_p->application_hash_output, "application_hash_output");
    load_text_hash(data_p->application_hash_output_ext, "application_hash_output_ext");
    load_text_hash(data_p->alias_hash, "alias_hash");
    return cache_ok;

  failed:
    destroy_application_hash_sfx ();
    destroy_application_hash_type ();
    data_p->application_hash_sfx = NULL;
    data_p->application_hash_type = NULL;
    return FALSE;
}

long long
mime_c::read_cache_sum (void) {
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



void 
mime_c::load_text_hash(GHashTable *hash_table, const gchar *filename){
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


void
mime_c::mime_build_hashes (void) {
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
        fprintf(stderr, "mime_build_hashes(): Cannot parse XML file: %s. Replace this file.\n", mimefile);
        g_free (mimefile);
        return;
    }

    node = xmlDocGetRootElement (doc);
    if(!xmlStrEqual (node->name, (const xmlChar *)"mime-info")) {
        fprintf(stderr, "Invalid XML file: %s (no mime-info). Replace this file.\n", mimefile);
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
		DBG("mime-module, return on type==NULL\n");
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
			g_hash_table_replace (data_p->application_hash_sfx, g_strdup(sfx_key), g_strdup(type));
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
			g_hash_table_replace (data_p->alias_hash, g_strdup(alias_key), g_strdup(type));
		    }
		    g_free (alias_type);
		    g_free (alias_key);
		    continue;
		}
                if(xmlStrEqual (subnode->name, (const xmlChar *)"generic-icon")) {
                    value = xmlGetProp (subnode, (const xmlChar *)"name");
		    if(value && strlen((const gchar *)value)) {
			g_hash_table_replace (data_p->generic_icon_hash, 
                                g_strdup(type), g_strdup((const gchar *)value));
                        //DBG("hashing %s --> %s\n", type, (const gchar *)value);
		    }
		    g_free (value);
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
                            g_hash_table_replace (data_p->application_hash_icon, k, extra_value);
                        }
                        extra_value = 
                            xmlGetProp (subnode, (const xmlChar *)"text");
                        if(extra_value) {
                            gchar *k=get_hash_key ((gchar *)value);
                            NOOP("mime-module a, adding- %s : %s\n", value, extra_value);
			    g_hash_table_replace (data_p->application_hash_text, k, extra_value);
                        }
                        extra_value = 
                            xmlGetProp (subnode, (const xmlChar *)"text2");
                        if(extra_value) {
                            gchar *k=get_hash_key ((gchar *)value);
                            NOOP("mime-module b, adding- %s : %s\n", value, extra_value);
                            g_hash_table_replace (data_p->application_hash_text2, k, extra_value);
                        }
                        extra_value = 
                            xmlGetProp (subnode, (const xmlChar *)"output");
                        if(extra_value) {
                            gchar *k=get_hash_key ((gchar *)value);
                            NOOP("mime-module c, adding- %s : %s\n", value, extra_value);
                            g_hash_table_replace (data_p->application_hash_output, k, extra_value);
                  
                        }
                        extra_value = 
                            xmlGetProp (subnode, (const xmlChar *)"output_ext");
                        if(extra_value) {
                            gchar *k=get_hash_key ((gchar *)value);
                            NOOP("mime-module d, adding- %s : %s\n", value, extra_value);
                            g_hash_table_replace (data_p->application_hash_output_ext, k, extra_value);
                        }
                    }
                }
            }
            if(apps) {
		type_key = get_hash_key (type);
                NOOP("mime-module, adding-%d : %s for %s (%s)\n", i, value, type, type_key);
                g_hash_table_replace (data_p->application_hash_type, type_key, apps);
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

void
mime_c::destroy_application_hash_sfx (void) {
    if(!data_p->application_hash_sfx) return;
    g_hash_table_destroy (data_p->application_hash_sfx);
}

void
mime_c::destroy_application_hash_type (void) {
    if(!data_p->application_hash_type) return;
    g_hash_table_destroy (data_p->application_hash_type);
}

gchar *
mime_c::get_hash_key (const gchar * pre_key) {
    GString *gs = g_string_new (pre_key);
    gchar *key;
    key = g_strdup_printf ("%10u", g_string_hash (gs));
    g_string_free (gs, TRUE);
    return key;
}

void
mime_c::add_type_to_hashtable(const gchar *type, const gchar *command, gboolean prepend){
    // Always use basic mimetype: avoid hashing alias mimetypes...
    gchar *hash_key=get_hash_key(type);
    pthread_mutex_lock(&data_p->alias_hash_mutex);
    const gchar *basic_type = (const gchar *)g_hash_table_lookup(data_p->alias_hash, hash_key);
    pthread_mutex_unlock(&data_p->alias_hash_mutex);
    if (basic_type) type = basic_type;
    g_free(hash_key);

    gchar *key = get_hash_key (type);
    pthread_mutex_lock(&data_p->application_hash_mutex);
    gchar **apps = (gchar **)g_hash_table_lookup (data_p->application_hash_type, key);

    if(!apps) {
        apps = (gchar **) malloc (2 * sizeof (gchar *));
	if (!apps) g_error("malloc: %s", strerror(errno));
	memset(apps, 0, 2 * sizeof (gchar *));
        *apps = g_strdup(command);
	g_hash_table_insert (data_p->application_hash_type, g_strdup(key), apps);
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
	g_hash_table_replace (data_p->application_hash_type, g_strdup(key), apps);
    }
    g_free(key);
    pthread_mutex_unlock(&data_p->application_hash_mutex);
    /*
    gint i;
    NOOP("(%d) %s: %s\n", prepend, type, command); fflush(NULL);
    for(i = 0; apps[i]; i++)
        NOOP(" %s", apps[i]);
    NOOP("\n");
    */

    NOOP("OPEN APPS: mime_write(%s)\n", type);

}

const gchar *
mime_c::mimeable_file (struct stat *st_p) {
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


void 
mime_c::save_text_cache(GHashTable *hash_table, const gchar *filename) {
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

gboolean
mime_c::generate_caches (void) {
    DBHashTable *cache;
    if(!data_p->application_hash_sfx || !data_p->application_hash_type) {
        DBG ("cannot build cache without application_hashes\n");
        return FALSE;
    }

    save_text_cache(data_p->application_hash_icon,"application_hash_icon");
    save_text_cache(data_p->application_hash_text,"application_hash_text");
    save_text_cache(data_p->application_hash_text2,"application_hash_text2");
    save_text_cache(data_p->application_hash_output,"application_hash_output");
    save_text_cache(data_p->application_hash_output_ext,"application_hash_output_ext");
    save_text_cache(data_p->generic_icon_hash,"generic_icon_hash");
    save_text_cache(data_p->alias_hash,"alias_hash");

    gchar *cache_path = get_cache_path ("sfx");
    if(!cache_path) {
        DBG ("!cache_path sfx\n");
        return FALSE;
    }
    gchar *tmp_cache_path = g_strdup_printf("%s-%d", 
	    cache_path, (gint)getpid());

    DBG("mime-module, creating cache file: %s\n", cache_path);
    unsigned char keylength=11;
    gchar *directory = g_path_get_dirname(tmp_cache_path);
    if (!g_file_test(directory, G_FILE_TEST_IS_DIR)){
        g_mkdir_with_parents(directory, 0700);
    }
    g_free(directory);
    cache = dbh_new (tmp_cache_path, &keylength, DBH_CREATE|DBH_PARALLEL_SAFE);
    TRACE("opened %s->%p\n",tmp_cache_path, cache); 

    if(cache == NULL) {
	DBG ("Could not create cache\n", tmp_cache_path);
	g_free (tmp_cache_path);
	g_free (cache_path);
	return FALSE;
    }
    dbh_set_parallel_lock_timeout(cache, 3);
 
    g_hash_table_foreach (data_p->application_hash_sfx, add2cache_sfx, (gpointer) cache);
    NOOP("mime-module, generated cache %s with %d records\n", cache_path, g_hash_table_size (data_p->application_hash_sfx));
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

    NOOP("mime-module, Creating cache file: %s\n", cache_path);
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

    pthread_mutex_lock(&data_p->application_hash_mutex);
    g_hash_table_foreach (data_p->application_hash_type, add2cache_type, (gpointer) cache);
    pthread_mutex_unlock(&data_p->application_hash_mutex);

    DBG("mime-module, generated cache %s with %d records\n", cache_path, g_hash_table_size (data_p->application_hash_type));
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

const gchar *
mime_c::locate_mime_t (const gchar * file) {
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

        type = (const gchar *)g_hash_table_lookup (data_p->application_hash_sfx, key);
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
	type = (const gchar *)g_hash_table_lookup (data_p->application_hash_sfx, key);
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

gchar **
mime_c::locate_apps (const gchar * type) {
    gchar **apps;

    //load_hashes ();
    ///  now look in hash...

    gchar *key = get_hash_key (type);
    pthread_mutex_lock(&data_p->application_hash_mutex);
    apps = (gchar **)g_hash_table_lookup (data_p->application_hash_type, key);
    pthread_mutex_unlock(&data_p->application_hash_mutex);
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


 
void *
mime_c::put_mimetype_in_hash(const gchar *file, const gchar *mimetype){
#ifndef NO_MIMETYPE_HASH
    if (!data_p->mimetype_hash) return NULL;
    gchar *key = get_hash_key (file);
    pthread_mutex_lock(&data_p->mimetype_hash_mutex);
    g_hash_table_replace (data_p->mimetype_hash, g_strdup(key), g_strdup(mimetype));
    pthread_mutex_unlock(&data_p->mimetype_hash_mutex);
    g_free (key);
#endif
    return NULL;
}

gchar *
mime_c::mimetype1(const gchar *file){
    if (!strchr(file, '.')){
	if (strstr(file, "README")) {
	    return g_strdup("text/x-readme");
	}
	if (strstr(file, "core")){
	    return g_strdup("application/x-core");
	}
	if (strstr(file, "INSTALL")){
	    return g_strdup("text/x-install");
	}
	if (strstr(file, "COPYING")) {
	    return g_strdup("text/x-credits");
	}
	if (strstr(file, "AUTHORS")) {
	    return g_strdup("text/x-authors");
	}
	if (strstr(file, "TODO")) {
	    return g_strdup("text/x-info");
	}
    }
    return NULL;
}

gchar *
mime_c::mimetype2(const gchar *file){
    const gchar *type = locate_mime_t (file);
    if(type && strlen(type)) {
        NOOP ("MIME:locate_mime_t(%s) -> %s\n", file, type);
	put_mimetype_in_hash(file, type);
        return g_strdup(type);
    }
    NOOP ("mime_type(): Could not locate mimetype for %s\n", file);
    return NULL;
}

const gchar *
mime_c::get_mimetype_iconname(const gchar *mimetype){
    return (const gchar *)
        g_hash_table_lookup(data_p->generic_icon_hash, mimetype);
}


gchar *
mime_c::get_hash_key_strstrip (void *p){
    gchar *pp=g_strdup((char *)p);
    g_strstrip(pp);
    gchar *key=get_hash_key ((gchar *)pp);
    g_free(pp);
    return key;
}


void *
mime_c::mime_gencache (gchar *data) {
    TRACE("gencache (%s)\n", data);
    pthread_mutex_lock (&data_p->cache_mutex);
    if (data && strchr(data,':')) {
	gchar *file=g_build_filename(USER_APPLICATIONS, NULL);
	gchar *newfile=g_build_filename(USER_APPLICATIONS".new", NULL);
        gchar *location = g_path_get_dirname(newfile);
        if (!g_file_test(location, G_FILE_TEST_IS_DIR) ){
            if (!g_mkdir_with_parents(location, 0600)){
                g_warning("Cannot create config directory: %s\n", location);
                g_free(location);
                pthread_mutex_unlock (&data_p->cache_mutex);
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
    pthread_mutex_unlock (&data_p->cache_mutex);
    return NULL;
}

long long
mime_c::get_cache_sum (void) {
    long long sum = 0;
    gchar *mimefile;
    mimefile = g_build_filename (APPLICATION_MIME_FILE, NULL);
    struct stat st;
    DBG("mime_c::get_cache_sum for %s\n", mimefile);
    if (stat (mimefile, &st)==0){
        sum += (st.st_dev + st.st_ino + st.st_mode + st.st_nlink
                + st.st_uid + st.st_gid + st.st_rdev + st.st_size + st.st_blksize + st.st_blocks + st.st_mtime + st.st_ctime);
    }
    g_free(mimefile);

    return sum;
}

void
mime_c::write_cache_sum (long long sum) {
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
    DBG("mime-module, wrote %lld to %s\n", sum, infofile);
    g_free (infofile);
    return;
}


gchar *
mime_c::get_cache_path (const gchar *which) {
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


gint
mime_c::check_dir (char *path) {
    struct stat st;
    if(!g_file_test(path, G_FILE_TEST_EXISTS)) {
        if(mkdir (path, 0770) < 0) return FALSE;
        return TRUE;
    }
    if(g_file_test(path, G_FILE_TEST_IS_DIR)) {
        if(access (path, W_OK) < 0) return FALSE;
        return TRUE;
    }
    return FALSE;
}


//**************************************************************


static void
add2sfx_hash (DBHashTable * cache, void *data) {
    GHashTable *application_hash_sfx = (GHashTable *)data;
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

static void
add2type_hash (DBHashTable * cache, void *data) {
    GHashTable *application_hash_type = (GHashTable *)data;
    gchar *type_key = (gchar *) malloc (DBH_KEYLENGTH (cache));
    if (!type_key) g_error("malloc: %s", strerror(errno));
    memset(type_key, 0, DBH_KEYLENGTH (cache));
    memcpy (type_key, DBH_KEY (cache), DBH_KEYLENGTH (cache));
    //gchar *string = g_strdup ((gchar *) DBH_DATA (cache));
    const gchar *string = (const gchar *)DBH_DATA (cache);

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
    length = strlen ((gchar *)value) + 1;

    dbh_set_recordsize (cache, length);

    memcpy (cache->data, value, length);
    dbh_update (cache);
    return;
}

static void
add2cache_text (gpointer key, gpointer value, gpointer user_data) {
        FILE *cache = (FILE *)user_data;
        fprintf(cache,"%s:%s\n", (gchar *)key, (gchar *)value);
    return;
}



static void *
gencache (void *data) {
    void **arg = (void **)data;
    mime_c *mime_p = (mime_c *)arg[0];
    return mime_p->mime_gencache((gchar *)arg[1]);
}


