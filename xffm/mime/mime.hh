#ifndef XF_MIME_HH
#define XF_MIME_HH

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>
#include <errno.h>

// For starters, we need mime_type() and mime_file(), 
// then type_from_sfx and alias_type and apps and command

// We have now simplified, removing custom made mime determination
// with now mature shared-mime-info package.
// This follows the same principle used in replacing custom made
// iconview with now mature gtk iconview.
// Things get simpler and maintainance not so complicated (methinks).
// 
// Remake: simplify with now mature shared-mime-info package
namespace xf {

GHashTable *application_hash_sfx=NULL;
GHashTable *alias_hash=NULL;
GHashTable *application_hash_icon=NULL;
GHashTable *application_hash_text=NULL;
GHashTable *application_hash_text2=NULL;
GHashTable *application_hash_output=NULL;
GHashTable *application_hash_output_ext=NULL;
GHashTable *application_hash_type=NULL;

template <class Type>
class Mime {

private:
    static gchar *
    mime (const gchar *command){
	FILE *pipe = popen (command, "r");
	if(pipe == NULL) {
	    ERROR("Cannot pipe from %s\n", command);
	    return NULL;
	}
#define MIMETYPE_LINE 256
	gchar *retval = NULL;
        gchar line[MIMETYPE_LINE];
        line[MIMETYPE_LINE - 1] = 0;
	if (!fgets (line, MIMETYPE_LINE - 1, pipe)) {
	    ERROR("!fgets (line, MIMETYPE_LINE - 1, pipe)\n");
        } 
	else 
	{
	    retval = g_strdup(line);
	    if (strchr(retval, '\n')) *strchr(retval, '\n') = 0;
	}
        pclose (pipe);
	return retval;
    }

public:    

    static gchar *
    mimeMagic (const gchar *file){
	//only magic:
	gchar *command = g_strdup_printf("%s -L -M --output-format=\"%%m\" \"%s\"", MIMETYPE_PROGRAM, file);
	gchar *retval = mime(command);
	g_free(command);
	return retval;
    }

    static gchar *
    mimeType (const gchar *file){
	gchar *command = g_strdup_printf("%s -L --output-format=\"%%m\" \"%s\"", MIMETYPE_PROGRAM, file);
	TRACE("mimeType command: %s\n", command);
 	gchar *retval = mime(command);
	TRACE("mimeType: %s --> %s\n", file, retval);
	g_free(command);
	return retval;
   } 

        
    static gchar *
    mimeType (const gchar *file, struct stat *st_p) {
        if (!file) return NULL;
        if(file[strlen (file) - 1] == '~' || file[strlen (file) - 1] == '%') {
            gchar *r_file = g_strdup(file);
            r_file[strlen (r_file) - 1] = 0;
            gchar *retval = mimeType(r_file, st_p);
            g_free(r_file);
            return retval;
        }
        return mimeMagic(file);
    }

// FIXME: use language code -l code, --language=code 
    static gchar *
    mimeFile (const gchar *file){
	gchar *command = g_strdup_printf("%s -d -L --output-format=\"%%d\" \"%s\"", MIMETYPE_PROGRAM, file);
 	gchar *retval = mime(command);
	g_free(command);
	return retval;
   } 
   
    static const gchar *
    get_mimetype_iconname(const gchar *mimetype){
	//FIXME: pull in value built from hash:
	return NULL;
        //return MimeHash<txt_hash_t>::lookup(mimetype, hash_data[GENERIC_ICON]); 
    }

#define WORKINPROGRESS 1
#ifdef WORKINPROGRESS
    static void
    mimeBuildHashes (void) {
        xmlChar *value;
        xmlNodePtr node;
        xmlNodePtr subnode;
        xmlDocPtr doc;
        gchar **apps;


        //build hashes from system files
        const gchar *mimefile = FREEDESKTOP;

        WARN("mime-module, reading mime specification file=%s\n", mimefile);
        if(access (mimefile, R_OK) != 0) {
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
            xmlFreeDoc (doc);
            return;
        }
        /* Now parse the xml tree */
        WARN("mime-module, parsing %s\n", mimefile);
        for(node = node->children; node; node = node->next) {
            if(xmlStrEqual (node->name, (const xmlChar *)"mime-key")) {
                gchar *type_key = NULL;
                gchar *type;

                //  type has to be defined. 
                type = (gchar *)xmlGetProp (node, (const xmlChar *)"type");
                if(!type) {
                    TRACE("mime-module, return on type==NULL\n");
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
                            TRACE("mime-module,replacing hash element \"%s\" with key %s --> %s\n", 
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
                            TRACE("mime-module, inserting alias hash element %s with key %s --> %s\n", 
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
                                TRACE("mime-module, adding- %s : %s\n", value, extra_value);
                                g_hash_table_replace (application_hash_icon, k, extra_value);
                            }
                            extra_value = 
                                xmlGetProp (subnode, (const xmlChar *)"text");
                            if(extra_value) {
                                gchar *k=get_hash_key ((gchar *)value);
                                TRACE("mime-module a, adding- %s : %s\n", value, extra_value);
                                g_hash_table_replace (application_hash_text, k, extra_value);
                            }
                            extra_value = 
                                xmlGetProp (subnode, (const xmlChar *)"text2");
                            if(extra_value) {
                                gchar *k=get_hash_key ((gchar *)value);
                                TRACE("mime-module b, adding- %s : %s\n", value, extra_value);
                                g_hash_table_replace (application_hash_text2, k, extra_value);
                            }
                            extra_value = 
                                xmlGetProp (subnode, (const xmlChar *)"output");
                            if(extra_value) {
                                gchar *k=get_hash_key ((gchar *)value);
                                TRACE("mime-module c, adding- %s : %s\n", value, extra_value);
                                g_hash_table_replace (application_hash_output, k, extra_value);
                      
                            }
                            extra_value = 
                                xmlGetProp (subnode, (const xmlChar *)"output_ext");
                            if(extra_value) {
                                gchar *k=get_hash_key ((gchar *)value);
                                TRACE("mime-module d, adding- %s : %s\n", value, extra_value);
                                g_hash_table_replace (application_hash_output_ext, k, extra_value);
                            }
                        }
                    }
                }
                if(apps) {
                    type_key = get_hash_key (type);
                    TRACE("mime-module, adding-%d : %s for %s (%s)\n", i, value, type, type_key);
                    g_hash_table_replace (application_hash_type, type_key, apps);
                } 
                g_free(type);
            }
        }
        xmlFreeDoc (doc);
        /*
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
        */
        
        WARN("mime-module, hash table build is now complete.\n");
    }

private:

    static gchar *
    get_hash_key (const gchar * pre_key) {
        GString *gs = g_string_new (pre_key);
        gchar *key;
        key = g_strdup_printf ("%10u", g_string_hash (gs));
        g_string_free (gs, TRUE);
        return key;
    }

    static gchar *
    get_hash_key_strstrip (void *p){
        gchar *pp=g_strdup((char *)p);
        g_strstrip(pp);
        gchar *key=get_hash_key ((gchar *)pp);
        g_free(pp);
        return key;
    }

/*    static gchar *
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
    }*/

#endif
    

};
}
#if 0 //deprecated stuff

#include "mime/mimemagic.hh"
#include "mime/lite.hh"
#include "mime/mimehash.hh"

#define USER_RFM_DIR            g_get_user_config_dir(),"xffm+"

#include "mime/mimehash.hh"
#define USER_RFM_CACHE_DIR      g_get_user_cache_dir(),"xffm+"
#define APPLICATION_MIME_FILE 	SYSTEM_MODULE_DIR,"mime-module.xml"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>
#include <errno.h>


enum {
    SFX,
    ALIAS,
    COMMAND,
    GENERIC_ICON,
    COMMAND_ICON,
    COMMAND_TEXT,
    COMMAND_TEXT2,
    COMMAND_OUTPUT,
    COMMAND_OUTPUT_EXT,
    MIME_HASHES
};

#define GET_COMMAND_TEXT(X) Mime::command_text_hash.lookup(X, Mime::hash_data[COMMAND_TEXT])
#define GET_COMMAND_TEXT2(X) Mime::command_text2_hash.lookup(X, Mime::hash_data[COMMAND_TEXT2])
#define GET_COMMAND_ICON(X) Mime::command_icon_hash.lookup(X, Mime::hash_data[COMMAND_ICON])
#define GET_COMMAND_OUTPUT(X) Mime::command_output_hash.lookup(X, Mime::hash_data[COMMAND_OUTPUT])
#define GET_COMMAND_OUTPUT_EXT(X) Mime::command_ext_hash.lookup(X, Mime::hash_data[COMMAND_OUTPUT_EXT])

#define LOCATE_APPS(X) MimeApplicationHash<txt_hash_t>::lookup(type, hash_data[COMMAND])
#define LOCATE_MIME_T(X) MimeSfxHash<txt_hash_t>::get_type_from_sfx(X, hash_data[SFX])
#define MIME_GET_ALIAS_TYPE(X) MimeAliasHash<txt_hash_t>::get_alias_type(X, hash_data[ALIAS])


typedef struct mime_t {
    char *key;
    char *mimetype;
    char **apps;
} mime_t;

pthread_mutex_t mimetype_hash_mutex=PTHREAD_MUTEX_INITIALIZER;
GHashTable *mimetype_hash=NULL;
txt_hash_t hash_data[MIME_HASHES];


namespace xf {

template <class Type>
class Mime {
    const gchar *
    mimeable_file (struct stat *st_p) {
        const gchar *result = NULL;
#ifdef S_IFWHT
        if(st_p->st_mode == S_IFWHT) {
            TRACE("mime-module, S_IFWHT file!\n");
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

     
    void *
    put_mimetype_in_hash(const gchar *file, const gchar *mimetype){
        if (!mimetype_hash) return NULL;
        gchar *key = get_hash_key (file);
        pthread_mutex_lock(&mimetype_hash_mutex);
        g_hash_table_replace (mimetype_hash, g_strdup(key), g_strdup(mimetype));
        pthread_mutex_unlock(&mimetype_hash_mutex);
        g_free (key);
        return NULL;
    }
    gchar *
    mimetype1(const gchar *file){
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
    mimetype2(const gchar *file){
        const gchar *type = LOCATE_MIME_T(file);
        if(type && strlen(type)) {
            TRACE ("MIME:LOCATE_MIME_T(%s) -> %s\n", file, type);
            put_mimetype_in_hash(file, type);
            return g_strdup(type);
        }
        TRACE ("mime_type(): Could not locate mimetype for %s\n", file);
        return NULL;
    }


    const gchar *
    find_mimetype_in_hash(const gchar *file){
        const gchar *type=NULL; 
        if (!mimetype_hash) return type;
        gchar *key = get_hash_key (file);
        pthread_mutex_lock(&mimetype_hash_mutex);
        if (!mimetype_hash) ERROR("mimetype_hash is NULL.\n");
        type = (const gchar *)g_hash_table_lookup (mimetype_hash, key);
        pthread_mutex_unlock(&mimetype_hash_mutex);
        g_free (key);
        return type;
    }

    // This function will return a basic mimetype, never an alias.
     
    gchar *
    mime_type (const gchar *file){
        if (!file) return NULL;
        const gchar *old_mimetype = find_mimetype_in_hash(file);
        if (old_mimetype) {
            // already tabulated. Just return previous value.
            return g_strdup(old_mimetype);
        }
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
    mime_type (const gchar *file, struct stat *st_p) {
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
                TRACE ("MIME: stat mime_type(%s) -> %s\n", file, type);
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

//////////////////////////////////////////////////////////////////
    static void
    free_apps(void *data){
        if (!data) return;
        gchar **apps = (gchar **)data;
        g_strfreev(apps);
    }

    static void create_hash(txt_hash_t &T,  xmlDocPtr doc, const gchar *xmlkey, const gchar *xmldata){
            memset(&T, 0, sizeof(txt_hash_t));
            T.doc = doc;
            T.xmlkey=xmlkey;
            T.xmldata=xmldata;
            pthread_mutex_init(&(T.mutex), NULL); // for read/write hashes
            T.hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    }

    static void create_hash(txt_hash_t &T,  xmlDocPtr doc, const gchar *xmlkey, const gchar *xmldata, const gchar *xmlsubdata){
            memset(&T, 0, sizeof(txt_hash_t));
            T.doc = doc;
            T.xmlkey=xmlkey;
            T.xmldata=xmldata;
            T.xmlsubdata=xmlsubdata;
            pthread_mutex_init(&(T.mutex), NULL); // for read/write hashes
            if (xmldata) T.hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
            else  T.hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, free_apps);
    }

    void create_hashes(xmlDocPtr doc){
        // create hashes
        create_hash(hash_data[SFX], doc, "key", "value");
        create_hash(hash_data[ALIAS], doc, "alias", "type");
        create_hash(hash_data[GENERIC_ICON], doc, "generic-icon", "name");
        
        create_hash(hash_data[COMMAND], doc, "application", NULL, "type");
        create_hash(hash_data[COMMAND_ICON], doc, "application", "command", "icon");
        create_hash(hash_data[COMMAND_TEXT], doc, "application", "command", "text");
        create_hash(hash_data[COMMAND_TEXT2], doc, "application", "command", "text2");
        create_hash(hash_data[COMMAND_OUTPUT], doc, "application", "command", "output");
        create_hash(hash_data[COMMAND_OUTPUT_EXT], doc, "application", "command", "output_ext");
        
    }

    void
    build_hashes(xmlDocPtr doc, const gchar *mimefile){
        // build hashes from common XML input
        app_sfx_hash.build_hash(hash_data[SFX], mimefile);
        app_alias_hash.build_hash(hash_data[ALIAS], mimefile);
        app_genericicon_hash.build_hash(hash_data[GENERIC_ICON], mimefile);

        app_command_hash.build_hash(hash_data[COMMAND], mimefile);

        command_icon_hash.build_hash(hash_data[COMMAND_ICON], mimefile);
        command_text_hash.build_hash(hash_data[COMMAND_TEXT], mimefile);
        command_text2_hash.build_hash(hash_data[COMMAND_TEXT2], mimefile);
        command_output_hash.build_hash(hash_data[COMMAND_OUTPUT], mimefile);
        command_ext_hash.build_hash(hash_data[COMMAND_OUTPUT_EXT], mimefile);
    }

    void
    init_hashes (void) {
        gchar *mimefile = g_build_filename (APPLICATION_MIME_FILE, NULL);
        xmlDocPtr doc = MimeHash<txt_hash_t>::openXML(mimefile);
        if (!doc){
            g_free(mimefile);
        } else {
            create_hashes(doc);
            build_hashes(doc, mimefile);
            xmlFreeDoc (doc);
            g_free(mimefile);
        }
        mimetype_hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
        return;
    }



    gchar *
    mime_magic(const gchar *file){
        gchar *unalias = mime_magic_unalias(file);
        gchar *alias = MIME_GET_ALIAS_TYPE(unalias);
        g_free(unalias);
        return alias;
    }



     

    gchar *
    mime_function(const gchar *path, const gchar *function) {
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





     
    gboolean mime_is_valid_command (const char *cmd_fmt) {
        //return GINT_TO_POINTER(TRUE);
        TRACE ("MIME: mime_is_valid_command(%s)\n", cmd_fmt);
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
        TRACE ("mime_is_valid_command(): g_find_program_in_path(%s)=%s\n", argv[0], path);

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


    gchar *
    mime_command (const char *type) {
        TRACE ("APPS: mime_command(%s)\n", type);
        gchar **apps;
        int i;
        gchar *cmd_fmt = NULL;
        apps = LOCATE_APPS(type);
        if(!apps) {
            TRACE ("APPS: --> NULL\n");
            return NULL;
        }
        if(!apps[0]) {
            TRACE ("APPS: --> NULL\n");
            g_free (apps);
            return NULL;
        }

        for(i = 0; apps[i]; i++) {
            g_free (cmd_fmt);
            cmd_fmt = g_strcompress (apps[i]);
            if(mime_is_valid_command (cmd_fmt)) {
                g_strfreev (apps);
                TRACE ("APPS: --> %s\n", cmd_fmt);
                return cmd_fmt;
            }
        }
        g_free (cmd_fmt);
        g_strfreev (apps);
        TRACE ("APPS: --> NULL\n");
        return NULL;
    }


    gchar **
    mime_apps (const char *type) {
        TRACE("mime_apps()...\n");
        TRACE ("MIME: mime_apps(%s)\n", type);
        gchar **apps;
        apps = LOCATE_APPS(type);
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
    mime_add (gchar *type, gchar *q) {
        TRACE("mime_add()...\n");
        gchar *command = g_strdup(q);
        g_strstrip(command);
        if(!command || !strlen (command)){
            g_free(command);
            return NULL;
        }

        TRACE ("OPEN APPS: adding type %s->%s\n", type, command);
        add_type_to_hashtable(type, command, TRUE);
      

        // thread will dispose of config_command:
        gchar *config_command=g_strdup_printf("%s:%s", type, command);
        g_free(command);

        return NULL;
    }

    // Append a command to a mimetype. This will not regenerate the disk
    // cache, (see dotdesktop module for the reason why not)
     
    void *
    mime_append (gchar *type, gchar *q) {
        gchar *command = g_strdup(q);
        g_strstrip(command);
        if(!command || !strlen (command)){
            g_free(command);
            return NULL;
        }
        TRACE ("OPEN APPS: appending type %s->%s\n", type, command);
        add_type_to_hashtable(type, command, FALSE);
        g_free(command);
        return NULL;
    }


    gchar *
    mime_mk_command_line (const gchar *command_fmt, const gchar *path) {
        TRACE("mime_mk_command_line()...\n");

        TRACE ("MIME: mime_mk_command_line(%s)\n", path);
        gchar *command_line = NULL;
        gchar *fmt = NULL;

        if(!command_fmt)
            return NULL;
        if(!path)
            path = "";

        TRACE ("MIME: command_fmt=%s\n", command_fmt);

        /* this is to send path as an argument */

        if(strstr (command_fmt, "%s")) {
            fmt = g_strdup (command_fmt);
        } else {
            fmt = g_strconcat (command_fmt, " %s", NULL);
        }
        TRACE ("MIME: command_fmt fmt=%s\n", fmt);

        TRACE ("MIME: path=%s\n", path);
        gchar *esc_path = util_c::esc_string (path);
        command_line = g_strdup_printf (fmt, esc_path);
        g_free (esc_path);
        TRACE ("MIME2: command_line=%s\n", command_line);

        g_free (fmt);
        return command_line;
    }

     
    gchar *
    mime_mk_terminal_line (const gchar *command) {
        TRACE("mime_mk_terminal_line()...\n");
        TRACE ("MIME: mime_mk_command_line(%s)\n", command);
        gchar *command_line = NULL;

        if(!command)
            return NULL;

        const gchar *term = util_c::what_term ();
        const gchar *exec_flag = util_c::term_exec_option(term);
        /*
        // Validation is already done by rfm_what_term
        if(!mime_is_valid_command ((void *)term)) {
            DBG ("%s == NULL\n", term);
            return NULL;
        }*/
        command_line = g_strdup_printf ("%s %s %s", term, exec_flag, command);

        return command_line;
    }

///////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////


    gchar *
    get_hash_key (const gchar * pre_key) {
        GString *gs = g_string_new (pre_key);
        gchar *key;
        key = g_strdup_printf ("%10u", g_string_hash (gs));
        g_string_free (gs, TRUE);
        return key;
    }

    void
    add_type_to_hashtable(const gchar *type, const gchar *command, gboolean prepend){
        // Always use basic mimetype: avoid hashing alias mimetypes...
        gchar *basic_type = mime_aliashash_c<txt_hash_t>::get_alias_type(type, hash_data[ALIAS]);
        if (!basic_type) return;
        gchar *key = get_hash_key (basic_type);
        g_free(basic_type);
        mime_application_hash_c<txt_hash_t>::add(key, command, prepend, hash_data[COMMAND]);
        g_free(key);
        return;

    }


    const gchar *
    get_mimetype_iconname(const gchar *mimetype){
        return MimeHash<txt_hash_t>::lookup(mimetype, hash_data[GENERIC_ICON]); 
    }


    gchar *
    get_hash_key_strstrip (void *p){
        gchar *pp=g_strdup((char *)p);
        g_strstrip(pp);
        gchar *key=get_hash_key ((gchar *)pp);
        g_free(pp);
        return key;
    }




};
}
#endif // deprecated stuff
#endif
