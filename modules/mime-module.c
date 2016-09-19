/*
 *
 * Edscott Wilson Garcia 2001-2012 edscott@users.sf.net
 *
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "rodent.h"


#if JUST_TRANSLATE 
// translations from applications.xml:
gchar *other_translations[]={
        N_("List files only"),
        N_("Extract files from the archive"),
        N_("Open in New Window"), 
        N_("Open in New Tab"),
        N_("Create a compressed archive with the selected objects"),
        N_("Create a new archive"),
        N_("Install"),
        N_("Uninstall"),
        N_("Information about the program"),
        N_("Simulation of data CD burning"),
        N_("Burn CD/DVD")
};
#endif

/* this should be first 2 lines after headers: */
G_MODULE_EXPORT LIBRFM_MODULE

//#ifdef HAVE_LIBMAGIC
//#undef HAVE_LIBMAGIC
//#endif

// Disable mimetype hash:
// Speed up is only marginal, probably not worth the memory cost...
#define NO_MIMETYPE_HASH

#include "mime-module.h"
#include "mime-module.i"
#include "mime-mouse_magic.i"
/****************************************************************************/
 
G_MODULE_EXPORT void *
module_active (void){
    return GINT_TO_POINTER(1);
}

G_MODULE_EXPORT 
void *
find_mimetype_in_hash(void *p){
    const gchar *type=NULL; 
#ifndef NO_MIMETYPE_HASH
    if (!mimetype_hash) return (void *)type;
    const gchar *file=p;
    gchar *key = get_hash_key (file);
    g_mutex_lock(mimetype_hash_mutex);
    const gchar *type = g_hash_table_lookup (mimetype_hash, key);
    g_mutex_unlock(mimetype_hash_mutex);
    g_free (key);
#endif
    return (void *)type;
}

static 
void *
put_mimetype_in_hash(const gchar *file, const gchar *mimetype){
#ifndef NO_MIMETYPE_HASH
    if (!mimetype_hash) return NULL;
    gchar *key = get_hash_key (file);
    g_mutex_lock(mimetype_hash_mutex);
    g_hash_table_replace (mimetype_hash, g_strdup(key), g_strdup(mimetype));
    g_mutex_unlock(mimetype_hash_mutex);
    g_free (key);
#endif
    return NULL;
}

static gchar *mimetype1(const gchar *file){
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

static gchar *mimetype2(const gchar *file){
    const gchar *type = locate_mime_t (file);
    if(type && strlen(type)) {
        NOOP ("MIME:locate_mime_t(%s) -> %s\n", file, type);
	put_mimetype_in_hash(file, type);
        return (void *)g_strdup(type);
    }
    NOOP ("mime_type(): Could not locate mimetype for %s\n", file);
    return NULL;
}

// This function will return a basic mimetype, never an alias.
G_MODULE_EXPORT 
gchar *
mime_type_plain (void *p){
    if (!p) return NULL;
#ifndef NO_MIMETYPE_HASH
    const gchar *old_mimetype = find_mimetype_in_hash(p);
    if (old_mimetype) {
	// already tabulated. Just return previous value.
	return g_strdup(old_mimetype);
    }
#endif
    const gchar *file = p;
    if(file[strlen (file) - 1] == '~' || file[strlen (file) - 1] == '%') {
	gchar *r_file = g_strdup(file);
	r_file[strlen (r_file) - 1] = 0;
	gchar *retval = mime_type_plain(r_file);
	g_free(r_file);
	return retval;
    }
    gchar *retval = mimetype1(file);
    if (retval) return retval;  
    return mimetype2(file);
}
    
G_MODULE_EXPORT 
gchar *
mime_type (void *p, void *q) {
    if (!p) return NULL;
#ifndef NO_MIMETYPE_HASH
    const gchar *old_mimetype = find_mimetype_in_hash(p);
    if (old_mimetype) {
	// already tabulated. Just return previous value.
	return g_strdup(old_mimetype);
    }
#endif
    const gchar *file = (const gchar *)p;
    struct stat *st_p = q;
    if(file[strlen (file) - 1] == '~' || file[strlen (file) - 1] == '%') {
	gchar *r_file = g_strdup(file);
	r_file[strlen (r_file) - 1] = 0;
	gchar *retval = mime_type(r_file, q);
	g_free(r_file);
	return retval;
    }
    gchar *retval = mimetype1(file);
    if (retval) return retval;  
    
    // Get a mimetype from the stat information, if this applies.
    //


    // If stat information is not provided, then stat the item.
    struct stat st_v;
    memset(&st_v, 0, sizeof(struct stat));
    if(!st_p) {
        st_p = &st_v;
        if (stat (file, st_p) < 0) {
	    goto extension_only;
	}
    }
    const gchar *type = mimeable_file (st_p);
    if(type) {
	put_mimetype_in_hash(file, type);
        NOOP ("MIME: stat mime_type(%s) -> %s\n", file, type);
        return (void *)g_strdup(type);
    }

    // Empty files (st_ino is there to make sure we do not have an empty stat):
    if (st_p->st_size == 0 && st_p->st_ino != 0) {
	return g_strdup("text/plain");
    }

extension_only:
    retval = mimetype2(file);
    if (retval) return retval;  

    // 
    // Empty files (st_ino is there to make sure we do not have an empty stat):
    if (st_p->st_size == 0 && st_p->st_ino != 0) {
	return g_strdup("text/plain");
    }

    TRACE ("mime_type(): Could not locate mimetype for %s\n", file);

    return NULL;
}

G_MODULE_EXPORT
void *
mime_magic (void *p) {
    if (!p) return NULL;
    return rfm_natural(RFM_MODULE_DIR, "mimemagic", p, "mime_magic");
}

G_MODULE_EXPORT 
void *
mime_encoding (void *p) {
    if (!p) return NULL;
    return rfm_natural(RFM_MODULE_DIR, "mimemagic", p, "mime_encoding");
}
G_MODULE_EXPORT 
void *
mime_file (void *p) {
    if (!p) return NULL;
    gchar *info = rfm_natural(RFM_MODULE_DIR, "mimemagic", p, "mime_file");
    // Sun virtual disk mime type text has <> which screws up markup:
    gchar *f=info; for(;f && *f; f++){
        if (*f == '<' || *f == '>') *f =' ';
    }

    return info;
}



void *mime_function(void *p, void *q) {
    if (!p || !q) return NULL;
    record_entry_t *en = p;
    if (!IS_LOCAL_TYPE(en->type)) return g_strdup(_("unknown"));

    const gchar *function = q;

    if (strcmp(function, "mime_file")==0) {
	return mime_file(en->path);
    }
    if (strcmp(function, "mime_encoding")==0) {
	return mime_encoding(en->path);
    }
    if (strcmp(function, "mime_magic")==0) {
	return mime_magic(en->path);
    }
    return NULL;
}




G_MODULE_EXPORT 
void *
mime_is_valid_command (void *p) {
    //return GINT_TO_POINTER(TRUE);
    const char *cmd_fmt = p;
    NOOP ("MIME: mime_is_valid_command(%s)\n", cmd_fmt);
    GError *error = NULL;
    int argc;
    gchar *path;
    gchar **argv;
    if(!cmd_fmt)
        return GINT_TO_POINTER (FALSE);
    if(!g_shell_parse_argv (cmd_fmt, &argc, &argv, &error)) {
        gchar *msg = g_strcompress (error->message);
        DBG ("%s: %s\n", msg, cmd_fmt);
        g_error_free (error);
        g_free (msg);
        return GINT_TO_POINTER (FALSE);
    }
    gchar **app = argv;
    if (*app==NULL) {
        errno = ENOENT;
        return GINT_TO_POINTER (FALSE);
    }

    // assume command is correct if environment is being set
    if (strchr(*app, '=')){
        g_strfreev (argv);
        return GINT_TO_POINTER (TRUE);
    }

    path = g_find_program_in_path (*app);
    if(!path) {
        gboolean direct_path = rfm_g_file_test (argv[0], G_FILE_TEST_EXISTS) ||
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
        return GINT_TO_POINTER (FALSE);
    }
    // here we test for execution within sudo
    // XXX we could also check for commands executed in a terminal, but not today...
    void *retval=GINT_TO_POINTER(TRUE);
    if (strcmp(argv[0],"sudo")==0) {
        int i=1;
        if (strcmp(argv[i],"-A")==0) i++;
        retval=mime_is_valid_command((gpointer)argv[i]);
    }

    g_strfreev (argv);
    g_free (path);
    return retval;
}

static
gchar *get_hash_key_strstrip (void *p){
    gchar *pp=g_strdup((char *)p);
    g_strstrip(pp);
    gchar *key=get_hash_key ((gchar *)pp);
    g_free(pp);
    return key;
}

G_MODULE_EXPORT 
void *
mime_command_text (void *p) {
    NOOP("mime_command_text()...\n");
    if (!p) return NULL;
    gchar *key=get_hash_key_strstrip ((gchar *)p);
    void *value=g_hash_table_lookup (application_hash_text, key);
    g_free(key);
    return value;
}

G_MODULE_EXPORT 
void *
mime_command_text2 (void *p) {
    NOOP("mime_command_text2()...\n");
    if (!p) return NULL;
    gchar *key=get_hash_key_strstrip ((gchar *)p);
    void *value=g_hash_table_lookup (application_hash_text2, key);
    g_free(key);
    return value;
}

G_MODULE_EXPORT 
void *
mime_command_icon (void *p) {
    NOOP("mime_command_icon()...\n");
    if (!p) return NULL;
    gchar *key=get_hash_key_strstrip ((gchar *)p);
    void *value=g_hash_table_lookup (application_hash_icon, key);
    g_free(key);
    return value;
}

G_MODULE_EXPORT
void *
mime_command_output (void *p) {
    NOOP("mime_command_output()...\n");
    if (!p) return NULL;
    gchar *key=get_hash_key_strstrip ((gchar *)p);
    void *value=g_hash_table_lookup (application_hash_output, key);
    g_free(key);
    return value;
}
G_MODULE_EXPORT 
void *
mime_command_output_ext (void *p) {
    NOOP("mime_command_output_ext()...\n");
    if (!p) return NULL;
    gchar *key=get_hash_key_strstrip ((gchar *)p);
    void *value=g_hash_table_lookup (application_hash_output_ext, key);
    g_free(key);
    return value;
}

    
G_MODULE_EXPORT
void *
mime_command (void *p) {
    NOOP("mime_command()...\n");
    const char *type = p;
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
            return (void *)cmd_fmt;
        }
    }
    g_free (cmd_fmt);
    g_strfreev (apps);
    NOOP ("APPS: --> NULL\n");
    return NULL;
}

G_MODULE_EXPORT
void *
mime_apps (void *p) {
    NOOP("mime_apps()...\n");
    const gchar *type = p;
    NOOP ("MIME: mime_apps(%s)\n", type);
    gchar **apps;
    apps = locate_apps (type);
    if(!apps)
        return NULL;
    if(!apps[0]) {
        g_free (apps);
        return NULL;
    }
    return (void *)apps;
}

// Insert a command to a mimetype. This will regenerate the disk
// cache.
G_MODULE_EXPORT 
void *
mime_add (void *p, void *q) {
    NOOP("mime_add()...\n");
    gchar *type = p;
    gchar *command = g_strdup((char *)q);
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
    rfm_view_thread_create(NULL, gencache, config_command, "gencache"); 
    return NULL;
}

// Append a command to a mimetype. This will not regenerate the disk
// cache, (see dotdesktop module for the reason why not)
G_MODULE_EXPORT 
void *
mime_append (void *p, void *q) {
    gchar *type = p;
    gchar *command = g_strdup((char *)q);
    g_strstrip(command);
    if(!command || !strlen (command)){
	g_free(command);
        return NULL;
    }
    NOOP ("OPEN APPS: appending type %s->%s\n", type, command);
    g_mutex_lock (cache_mutex);
    add_type_to_hashtable(type, command, FALSE);
    g_mutex_unlock (cache_mutex);
    g_free(command);
    return NULL;
}

G_MODULE_EXPORT
void *
mime_mk_command_line (void *p, void *q) {
    NOOP("mime_mk_command_line()...\n");
    const gchar *command_fmt = p;
    const gchar *path = q;

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
    gchar *esc_path = rfm_esc_string (path);
    command_line = g_strdup_printf (fmt, esc_path);
    g_free (esc_path);
    NOOP ("MIME2: command_line=%s\n", command_line);

    g_free (fmt);
    return (void *)command_line;
}

G_MODULE_EXPORT 
void *
mime_mk_terminal_line (void *p) {
    NOOP("mime_mk_terminal_line()...\n");
    const gchar *command = p;
    NOOP ("MIME: mime_mk_command_line(%s)\n", command);
    gchar *command_line = NULL;

    if(!command)
        return NULL;

    const gchar *term = rfm_what_term ();
    const gchar *exec_flag = rfm_term_exec_option(term);
    /*
    // Validation is already done by rfm_what_term
    if(!mime_is_valid_command ((void *)term)) {
        DBG ("%s == NULL\n", term);
        return NULL;
    }*/
    command_line = g_strdup_printf ("%s %s %s", term, exec_flag, command);

    return command_line;
}

/////////////////////////////////////////////////////////////////
G_MODULE_EXPORT
void *
mime_generate_cache(void){
    gencache(NULL);     
    return NULL;
}

static void
free_apps(void *data){
    if (!data) return;
    gchar **apps = data;
    g_strfreev(apps);
}

G_MODULE_EXPORT
const gchar *
g_module_check_init (GModule * module) {
    NOOP(stderr, "***************g_module_check_init\n");
    rfm_mutex_init(cache_mutex);
    rfm_mutex_init(mimetype_hash_mutex);
    rfm_mutex_init(alias_hash_mutex);
    rfm_mutex_init(application_hash_mutex);
    // module setup writelock
#ifndef NO_MIMETYPE_HASH
    mimetype_hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
#endif
    alias_hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

    application_hash_type = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, free_apps);

    // Read only hashes:
    application_hash_sfx = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    application_hash_icon = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    application_hash_text = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    application_hash_text2 = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    application_hash_output = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    application_hash_output_ext = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    if(!load_hashes_from_cache ()) 
    {
            mime_build_hashes ();
	    mime_generate_cache();
     }
    return NULL;
}


G_MODULE_EXPORT void 
g_module_unload(GModule * module){
    if(mimetype_hash) g_hash_table_destroy(mimetype_hash);
    if(alias_hash) g_hash_table_destroy(alias_hash);
    if(application_hash_type) g_hash_table_destroy(application_hash_type);

    if(application_hash_sfx) g_hash_table_destroy(application_hash_sfx);
    if(application_hash_icon) g_hash_table_destroy(application_hash_icon);
    if(application_hash_text) g_hash_table_destroy(application_hash_text);
    if(application_hash_text2) g_hash_table_destroy(application_hash_text2);
    if(application_hash_output) g_hash_table_destroy(application_hash_output);
    if(application_hash_output_ext) g_hash_table_destroy(application_hash_output_ext);
    rfm_mutex_free(cache_mutex);
    rfm_mutex_free(mimetype_hash_mutex);
    rfm_mutex_free(alias_hash_mutex);
    rfm_mutex_free(application_hash_mutex);
}


G_MODULE_EXPORT
gchar *mime_get_alias_type(void *p){
    const gchar *type = p;
    if(type) {
	gchar *hash_key=get_hash_key(type);
	g_mutex_lock (alias_hash_mutex);
	const gchar *basic_type = g_hash_table_lookup(alias_hash, hash_key);
	g_mutex_unlock (alias_hash_mutex);
	g_free(hash_key);
	if (basic_type) return g_strdup(basic_type);
	return g_strdup(type);
    } 
    return g_strdup(inode[INODE_UNKNOWN]);
}
 
