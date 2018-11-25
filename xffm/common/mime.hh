#ifndef XF_MIME_HH
#define XF_MIME_HH

//#include <libxml/parser.h>
//#include <libxml/tree.h>
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
static pthread_mutex_t application_hash_mutex=PTHREAD_MUTEX_INITIALIZER;
static GHashTable *application_hash_sfx=NULL;
static GHashTable *alias_hash=NULL;
static GHashTable *application_hash_icon=NULL;
static GHashTable *application_hash_type=NULL;

static GHashTable *application_hash_text=NULL;
static GHashTable *application_hash_text2=NULL;
static GHashTable *application_hash_output=NULL;
static GHashTable *application_hash_output_ext=NULL;

template <class Type>
class Mime {
    using util_c = Util<Type>;
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

    static void
    add2sfx_hash (const gchar *file, const gchar *value) {
        gchar *sfx = g_path_get_basename(file);
        const gchar *sfx_key = get_hash_key (sfx);
        g_free(sfx);
        pthread_mutex_lock(&application_hash_mutex);
        g_hash_table_replace (application_hash_sfx, (void *)sfx_key, (void *)value);
        pthread_mutex_unlock(&application_hash_mutex);
    }

    static void
    add2ApplicationHash(const gchar *type, const gchar *command, gboolean prepend){
        // Always use basic mimetype: avoid hashing alias mimetypes...
        const gchar *basic_type = getBasicType(type);
        if (!basic_type) basic_type = type;
        gchar *key = get_hash_key (basic_type);
        pthread_mutex_lock(&application_hash_mutex);
	auto apps = (gchar **)g_hash_table_lookup(application_hash_type, key);
	if (apps) {
	    int size = 1; // final 0
	    gchar **p;
            // Check if command is already tabulated...
	    for (p=apps; p && *p; p++) {
                if (strcmp(*p, command)==0) {
                    pthread_mutex_unlock(&application_hash_mutex);
                    return;
                }
            }
            // get vector size.
	    for (p=apps; p && *p; p++) size++;

	    gchar **newApps = (gchar **)calloc(size+1, sizeof(gchar *));
	    if (!newApps){
		ERROR("add2ApplicationHash: calloc() %s\n", strerror(errno));
		exit(1);
	    }
	    int i=0;
	    if (prepend) newApps[i++] = g_strdup(command);
	    for (p=apps; p && *p; p++){
		newApps[i++] = g_strdup(*p);
	    }
	    if (!prepend) newApps[i++] = g_strdup(command);
	    g_hash_table_replace(application_hash_type, key, (void *)newApps);
            TRACE("replacing hash value %s -> %s\n", basic_type, command);
	} else {
	    gchar **newApps = (gchar **)calloc(2, sizeof(gchar *));
	    newApps[0] = g_strdup(command);
	    g_hash_table_insert(application_hash_type, key, (void *)newApps);
            TRACE("adding hash value %s -> %s\n", basic_type, command);
	} 
        pthread_mutex_unlock(&application_hash_mutex);
        return;

    }


public:    

    static const gchar *
    getBasicType(const gchar *mimetype) {
	const gchar *retval = mimetype;
	gchar *key = get_hash_key (mimetype);
        const gchar *alias = (const gchar *)g_hash_table_lookup (alias_hash, key);
        if (alias) retval = alias;
	g_free(key);
	return retval;
    }

    static const gchar *
    locate_icon (const gchar *mimetype) {
        const gchar *icon;
        if (!application_hash_icon) {
            ERROR("!application_hash_icon\n");
            return NULL;
        }
        TRACE("mime-module, locate_icon looking in icon hash for \"%s\"\n", mimetype);
        
	const gchar *basicType = getBasicType(mimetype);
	gchar *key = get_hash_key (mimetype);
        icon = (const gchar *)g_hash_table_lookup (application_hash_icon, key);
	g_free(key);

        if (!icon){
            const gchar *alias = getBasicType(mimetype);
            if (alias) {
                key = get_hash_key (alias);
                icon = (const gchar *)g_hash_table_lookup (application_hash_icon, key);
		g_free(key);
            }
        }
        return icon;
    }

    static const gchar *
    locate_mime_t (const gchar * file) {
        const gchar *type;
        const gchar *p;
        if (!application_hash_sfx) {
	    mimeBuildHashes();
	    if (!application_hash_sfx) {
		ERROR("!application_hash_sfx\n");
		return NULL;
	    }
        }
        TRACE("mime-module, locate_mime_t looking in sfx hash for \"%s\"\n", file);

        ///  look in sfx hash...
        gchar *basename = g_path_get_basename (file);
        if (strchr (basename, '.')) p = strchr (basename, '.');
        else {
            gchar *key = get_hash_key (basename);
            pthread_mutex_lock(&application_hash_mutex);
            type = (const gchar *)g_hash_table_lookup (application_hash_sfx, key);
            pthread_mutex_unlock(&application_hash_mutex);
            g_free (key);
            if(type) {
                TRACE("mime-module(2), FOUND %s: %s\n", file, type);
                g_free (basename);
                return type;
            }
            return NULL;
        }
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

            pthread_mutex_lock(&application_hash_mutex);
            type = (const gchar *)g_hash_table_lookup (application_hash_sfx, key);
            pthread_mutex_unlock(&application_hash_mutex);
            g_free (key);
            if(type) {
                TRACE("mime-module, FOUND %s: %s\n", sfx, type);
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
            pthread_mutex_lock(&application_hash_mutex);
            type = (const gchar *)g_hash_table_lookup (application_hash_sfx, key);
            pthread_mutex_unlock(&application_hash_mutex);
            g_free (key);
            if(type) {
                TRACE("mime-module(2), FOUND %s: %s\n", sfx, type);
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

    static const gchar **
    locate_apps (const gchar * type) {

        //load_hashes ();
        ///  now look in hash...
        if (!type) return NULL;
        gchar *key = get_hash_key (type);
        pthread_mutex_lock(&application_hash_mutex);
	TRACE("loading apps for mimetype: %s\n", type);
        auto apps = (const gchar **)g_hash_table_lookup (application_hash_type, key);
        pthread_mutex_unlock(&application_hash_mutex);
        g_free (key);
        return apps;
    }


    static gchar *
    mimeMagic (const gchar *file){
#ifdef MIMETYPE_PROGRAM
	//only magic:
	gchar *command = g_strdup_printf("%s -L -M --output-format=\"%%m\" \"%s\"", MIMETYPE_PROGRAM, file);
	gchar *retval = mime(command);
	g_free(command);
	return retval;
#else
        return NULL;
#endif
    }

    static const gchar *
    mimeIcon (const gchar *file){
        const gchar *retval = locate_icon(file);
        if (retval) {
	    TRACE("locate_icon: %s --> %s\n", file, retval);
        }
	return retval;
   } 


    static const gchar *
    mimeType (const gchar *file){
        const gchar *retval = locate_mime_t(file);
        if (retval) {
	    TRACE("locate_mime_t: %s --> %s\n", file, retval);
            return retval;
        }
#ifdef MIMETYPE_PROGRAM
	gchar *command = g_strdup_printf("%s -L --output-format=\"%%m\" \"%s\"", MIMETYPE_PROGRAM, file);
	TRACE("mimeType command: %s\n", command);
 	retval = mime(command);
	TRACE("mimeType: %s --> %s\n", file, retval);
        if (retval) add2sfx_hash(file, retval);
	g_free(command);
	return retval;
#else
        struct stat st;
        if (stat(file, &st) < 0) return "unknown mimetype";
        const gchar *r = mimeType(file, &st);
        return g_strdup(r);
        
#endif
   } 

        
    static const gchar *
    mimeType (const gchar *file, struct stat *st_p) {
        if (!file) return NULL;
        if (st_p){
            if(S_ISSOCK (st_p->st_mode)) return "inode/socket";
            else if(S_ISBLK (st_p->st_mode)) return "inode/blockdevice";
            else if(S_ISCHR (st_p->st_mode)) return "inode/chardevice";
            else if(S_ISFIFO (st_p->st_mode)) return "inode/fifo";
            else if (S_ISLNK(st_p->st_mode)) return "inode/symlink";
            else if(S_ISDIR (st_p->st_mode)) return "inode/directory";
        }
        if(file[strlen (file) - 1] == '~' || file[strlen (file) - 1] == '%') {
            gchar *r_file = g_strdup(file);
            r_file[strlen (r_file) - 1] = 0;
            const gchar *retval = mimeType(r_file, st_p);
            g_free(r_file);
            return retval;
        }

        return "inode/regular";
        // mimemagic return value should be const gchar *
        //return mimeMagic(file);
    }

// FIXME: use language code -l code, --language=code 
    static gchar *
    mimeFile (const gchar *file){
#ifdef MIMETYPE_PROGRAM
	gchar *command = g_strdup_printf("%s -d -L --output-format=\"%%d\" \"%s\"", MIMETYPE_PROGRAM, file);
 	gchar *retval = mime(command);
	g_free(command);
	return retval;
#else
        return NULL;
#endif
   } 
   
    static const gchar *
    get_mimetype_iconname(const gchar *mimetype){
	//FIXME: pull in value built from hash:
	return NULL;
        //return MimeHash<txt_hash_t>::lookup(mimetype, hash_data[GENERIC_ICON]); 
    }

    static void freeStrV(void *data){
	auto p = (gchar **)data;
	g_strfreev(p);
    }

    static void
    mimeBuildHashes (void) {
        application_hash_sfx = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
        alias_hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
        application_hash_icon = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
        application_hash_type = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, freeStrV);
        FILE *input;
#ifdef FREEDESKTOP_GLOBS
        if ((input = fopen(FREEDESKTOP_GLOBS, "r")) != NULL) {
            gchar buffer[256]; memset(buffer, 0, 256);
            while (fgets(buffer, 255, input) && !feof(input)){
                if (!strchr(buffer, ':'))continue;
                gchar **x = g_strsplit(buffer, ":", -1);
                gint offset = 0;
                if (strchr(x[1], '\n')) *(strchr(x[1], '\n')) = 0;
                if (strncmp(x[1], "*.", strlen("*."))==0) offset = strlen("*.");

                gchar *key = get_hash_key (x[1]+offset);
                if(key) {
                     TRACE("GLOB mime-module,replacing hash element with key %s --> %s\n", 
                                    x[1]+offset, x[0]);
                     g_hash_table_replace (application_hash_sfx,  g_strdup(key), g_strdup(x[0]));
                }
                g_strfreev(x);
                g_free(key);
            }
            fclose(input);
        }else ERROR("Cannot open %s\n", FREEDESKTOP_GLOBS);
#endif
#ifdef FREEDESKTOP_ICONS
        if ((input = fopen(FREEDESKTOP_ICONS, "r")) != NULL) {
            gchar buffer[256]; memset(buffer, 0, 256);
            while (fgets(buffer, 255, input) && !feof(input)){
                if (!strchr(buffer, ':'))continue;
                gchar **x = g_strsplit(buffer, ":", -1);
                if (strchr(x[1], '\n')) *(strchr(x[1], '\n')) = 0;
                gchar *key = get_hash_key (x[0]);
                if(key) {
                     TRACE("ICON mime-module,replacing hash element \"%s\" with key %s --> %s\n", 
                                    x[0], key, x[1]);
                     g_hash_table_replace (application_hash_icon,  g_strdup(key), g_strdup(x[1]));
                }
                g_strfreev(x);
                g_free(key);
            }
            fclose(input);
        }else ERROR("Cannot open %s\n", FREEDESKTOP_ICONS);
#endif
#ifdef FREEDESKTOP_ALIAS

        if ((input = fopen(FREEDESKTOP_ALIAS, "r")) != NULL) {
            gchar buffer[256]; memset(buffer, 0, 256);
            while (fgets(buffer, 255, input) && !feof(input)){
                if (!strchr(buffer, ' '))continue;
                gchar **x = g_strsplit(buffer, " ", -1);
                if (strchr(x[1], '\n')) *(strchr(x[1], '\n')) = 0;
                gchar *key = get_hash_key (x[0]);
                if(key) {
                     TRACE("ALIAS mime-module,replacing hash element \"%s\" with key %s --> %s\n", 
                                    x[0], key, x[1]);
                     g_hash_table_replace (alias_hash,  g_strdup(key), g_strdup(x[1]));
                }
                g_strfreev(x);
                g_free(key);
            }
            fclose(input);
        } else ERROR("Cannot open %s\n", FREEDESKTOP_ALIAS);
#endif

	// FIXME: break the following code into routines...
	// mimetype registered applications...
	// /usr/share/applications
	// /usr/local/share/applications
	const gchar *directories[] = {
	    "/usr/share/applications",
	    "/usr/local/share/applications"
	};
	for (int i=0; i<2; i++) {
	    processApplicationDir(directories[i]);
	}
    }
    static void
    processApplicationDir(const gchar *dir){
	DIR *directory = opendir(dir);
	if (!directory) {
	    TRACE("mime_c:: opendir %s: %s\n", dir, strerror(errno));
	    return;
	}
	TRACE("Now reading directory: %s\n", dir);
	readApplicationDir(dir, directory);
	closedir (directory);
    }

    static void 
    readApplicationDir(const gchar *dir, DIR *directory){
	//  mutex protect...
	//pthread_mutex_lock(&readdir_mutex);
	struct dirent *d; // static pointer
	errno=0;
	while ((d = readdir(directory))  != NULL){
	    TRACE("Now reading file: %s\n", d->d_name);
	    parseDesktopFile(dir, d);
	}
    }

    static void
    parseDesktopFile(const gchar *dir, struct dirent *d){
	TRACE( "%p  %s\n", d, d->d_name);
	if(strstr(d->d_name, ".desktop") == NULL) return;
	// get [Desktop Entry] Exec
	// get [Desktop Entry] TryExec
	// get [Desktop Entry] MimeType
	//
	GKeyFile *key_file = g_key_file_new();
	gchar *file = g_build_filename(dir,d->d_name, NULL);
	gboolean loaded = g_key_file_load_from_file(key_file, file, 
		(GKeyFileFlags) (G_KEY_FILE_KEEP_COMMENTS |  G_KEY_FILE_KEEP_TRANSLATIONS), NULL);
	g_free(file);
	if (!loaded) return;
	const gchar *group = "Desktop Entry";
	if (!g_key_file_has_group (key_file, group)){
	    g_key_file_free(key_file);
	    return;
	}
	GError *error = NULL;
	if (!g_key_file_has_key (key_file, group, "MimeType", &error)){
	    g_key_file_free(key_file);
	    return;
	}
	error = NULL;
	gchar *exec = g_key_file_get_string (key_file, group, "Exec", &error);
	if (error){ exec = NULL; error=NULL;}
	gchar *tryExec = g_key_file_get_string (key_file, group, "TryExec", &error);
	if (error){ tryExec = NULL; error=NULL; }
	gchar *terminal = g_key_file_get_string (key_file, group, "Terminal", &error);
	if (error){ terminal = NULL; error=NULL; }
	gchar *icon = g_key_file_get_string (key_file, group, "Icon", &error);
	if (error){ icon = NULL;  error=NULL;}
	gchar *mimeType = g_key_file_get_string (key_file, group, "MimeType", &error);

	if (mimeType){
	    gchar **types = g_strsplit(mimeType, ";", -1);
	    gchar **p;
	    for (p=types; p && *p; p++){
		gchar *e = (exec)?exec:tryExec;
		
		if (*p && e){
		    if (strstr(e, "%U")) *(strstr(e, "%U")+1) = 's';
		    if (strstr(e, "%u")) *(strstr(e, "%u")+1) = 's';
		    if (strstr(e, "%F")) *(strstr(e, "%F")+1) = 's';
		    if (strstr(e, "%f")) *(strstr(e, "%f")+1) = 's';
		    TRACE("Adding application %s --> %s\n", *p, e);
		    add2ApplicationHash(*p, e, TRUE);
		}
	    }
	    g_strfreev(types);
	}
	g_free(mimeType);
	g_free(exec);
	g_free(tryExec);
	g_free(terminal);
	g_free(icon);
	g_key_file_free(key_file);
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
public:    
    static gchar *
    mkCommandLine (const gchar *command_fmt, const gchar *path) {
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
private:       
//////////////////////////////////////////////////////////////////////////////////////////////

public:     

    static gboolean
    fixedInTerminal(const gchar *app){
	gchar *a = Mime<Type>::baseCommand(app);
	gchar *b = strrchr(a, G_DIR_SEPARATOR);
	if (!b) b=a; else b++;
	gchar const *exceptions[] = {"vi", "vim", "vimdiff", "vimtutor", "nano", NULL};
	gchar const **q;
	gboolean retval = FALSE;
	for (q=exceptions; q && *q; q++){
	    if (strcmp(a, *q) == 0){
		retval=TRUE;
		break;
	    }
	}
	g_free(a);
	return retval;
    }
    static gchar *
    baseCommand(const gchar *commandFmt){
	if (!commandFmt) return NULL;
	gchar *a = g_strdup(commandFmt);
	g_strstrip(a);
	if (strchr(a, ' ')) *(strchr(a, ' ')) = 0;
	return a;
    }
    static gchar *
    baseIcon(const gchar *iconFmt){
	if (!iconFmt) return NULL;
	gchar *a = g_strdup(iconFmt);
	g_strstrip(a);
	if (strchr(a, ' ')) *(strchr(a, ' ')) = 0;
	gchar *g = g_path_get_basename(a);
	g_free(a);
	a=g;
	return a;
    }

    static gboolean
    runInTerminal(const gchar *commandFmt){
	if (fixedInTerminal(commandFmt)) return TRUE;
	gchar *a = baseCommand(commandFmt);
	gboolean retval = FALSE;
	if (Settings<Type>::keyFileHasGroupKey("Terminal",  a) &&
		Settings<Type>::getSettingInteger("Terminal", a))retval = TRUE;
	/*if (g_key_file_has_group(keyFile, "Terminal") &&
	    g_key_file_has_key (keyFile, "Terminal", a, NULL) && 
	    Dialog<Type>::getSettingInteger("Terminal", a))
		retval = TRUE;*/
	g_free(a);
	return retval;
    }

    static gchar *
    mkTerminalLine (const gchar *command, const gchar *path) {
        TRACE("mime_mk_terminal_line()...\n");
        TRACE ("MIME: mime_mk_command_line(%s)\n", command);
        gchar *command_line = NULL;

        if(!command) return NULL;
	gchar *a = mkCommandLine(command, path);

        const gchar *term = util_c::what_term ();
        const gchar *exec_flag = util_c::term_exec_option(term);
        command_line = g_strdup_printf ("%s %s %s", term, exec_flag, a);
	g_free(a);
        return command_line;
    }

    static gboolean isValidCommand (const char *cmd_fmt) {
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
            TRACE("argv[0]=%s\n",argv[0]);
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
            retval=isValidCommand(argv[i]);
        }

        g_strfreev (argv);
        g_free (path);
        return retval;
    }


 

//////////////////////////////////////////////////////////////////////////////////////////////
    

};
}
#endif