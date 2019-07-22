#ifndef XF_MIME_HH
#define XF_MIME_HH

// We can use either libmagic or perl mimetype, depending on configuration


namespace xf {

// For starters, we need mime_type() and mime_file(), 
// then type_from_sfx and alias_type and apps and command

// We have now simplified, removing custom made mime determination
// with now mature shared-mime-info package.
// This follows the same principle used in replacing custom made
// iconview with now mature gtk iconview.
// Things get simpler and maintainance not so complicated (methinks).
// 
// Remake: simplify with now mature shared-mime-info package


static GHashTable *application_hash_text=NULL;
static GHashTable *application_hash_text2=NULL;
static GHashTable *application_hash_output=NULL;
static GHashTable *application_hash_output_ext=NULL;

template <class Type>
class Mime {
    using util_c = Util<Type>;
public:
    static gchar *
    encoding (const gchar *file) { 
#ifdef HAVE_LIBMAGIC  
	return  MimeMagic<Type>::encoding(file);
#else
	// If not libmagic, assume the encoding is already utf-8. (whatever...)
	return  g_strdup("UTF-8");
#endif
    }

    static gchar *
    mimeMagic (const gchar *file){
#ifdef MIMETYPE_PROGRAM
	return MimeType<Type>::mimeMagic(file);
#else
# if LIBMAGIC
	return MimeMagic<Type>::mimeMagic(file);
# else
        return NULL;
# endif
#endif
    }

    static gchar *
    mimeType (const gchar *file){
        gchar *retval = MimeSuffix<Type>::mimeType(file);
        if (retval) {
	    TRACE("mimeType: %s --> %s\n", file, retval);
            return retval;
        }
#ifdef MIMETYPE_PROGRAM
	return MimeType<Type>::mimeType(file);
#else
	errno=0;
        struct stat st;
        if (stat(file, &st) < 0) {
	    DBG("mime.hh::mimeType(): stat %s (%s)\n",
		file, strerror(errno));
	    errno=0;
	    return g_strdup("unknown mimetype");
	}
        gchar *r = mimeType(file, &st);
        return r;
        
#endif
   } 

// FIXME: use language code -l code, --language=code 
    static gchar *
    mimeFile (const gchar *file){
#ifdef MIMETYPE_PROGRAM
	return MimeType<Type>::mimeFile(file);
#else
        return NULL;
#endif
   } 
   

    static void
    add2ApplicationHash(const gchar *type, const gchar *command, gboolean prepend){
        // Always use basic mimetype: avoid hashing alias mimetypes...
        const gchar *basic_type = getBasicType(type);
        if (!basic_type) basic_type = type;
        gchar *key = get_hash_key (basic_type);
        pthread_mutex_lock(&mimeHashMutex);
	auto apps = (gchar **)g_hash_table_lookup(application_hash_type, key);
	if (apps) {
	    int size = 1; // final 0
	    gchar **p;
            // Check if command is already tabulated...
	    for (p=apps; p && *p; p++) {
                if (strcmp(*p, command)==0) {
                    pthread_mutex_unlock(&mimeHashMutex);
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
        pthread_mutex_unlock(&mimeHashMutex);
        return;

    }



    static gchar *
    getBasicType(const gchar *mimetype) {
	gchar *retval;
	gchar *key = get_hash_key (mimetype);
        const gchar *alias = (const gchar *)g_hash_table_lookup (mimeHashAlias, key);
        if (alias) retval = g_strdup(alias);
	else retval = g_strdup(mimetype);
	g_free(key);
	return retval;
    }

    static const gchar *
    locate_icon (const gchar *mimetype) {
        const gchar *icon;
        if (!mimeHashIcon) {
            ERROR("!mimeHashIcon\n");
            return NULL;
        }
        TRACE("mime-module, locate_icon looking in icon hash for \"%s\"\n", mimetype);
        
	const gchar *basicType = getBasicType(mimetype);
	gchar *key = get_hash_key (mimetype);
        icon = (const gchar *)g_hash_table_lookup (mimeHashIcon, key);
	g_free(key);

        if (!icon){
            const gchar *alias = getBasicType(mimetype);
            if (alias) {
                key = get_hash_key (alias);
                icon = (const gchar *)g_hash_table_lookup (mimeHashIcon, key);
		g_free(key);
            }
        }
        return icon;
    }


public:
    static const gchar **
    locate_apps (const gchar * mimetype) {

        //load_hashes ();
        ///  now look in hash...
        if (!mimetype) return NULL;
        gchar *key = get_hash_key (mimetype);
        pthread_mutex_lock(&mimeHashMutex);
	TRACE("loading apps for mimetype: %s\n", mimetype);
        auto apps = (const gchar **)g_hash_table_lookup (application_hash_type, key);
        pthread_mutex_unlock(&mimeHashMutex);
        g_free (key);
        return apps;
    }

private:

    static const gchar *
    mimeIcon (const gchar *file){
        const gchar *retval = locate_icon(file);
        if (retval) {
	    TRACE("locate_icon: %s --> %s\n", file, retval);
        }
	return retval;
   } 

public:

public: 
    static gchar *
    basicMimeType(unsigned char d_type){
	gchar *retval=NULL;
	if (d_type == DT_DIR ) retval= g_strdup("inode/directory");
        // Character device:
	else if (d_type == DT_CHR ) retval= g_strdup("inode/chardevice");   
        // Named pipe (FIFO):
        else if (d_type == DT_FIFO ) retval= g_strdup("inode/fifo");
	// UNIX domain socket:
        else if (d_type == DT_SOCK ) retval= g_strdup("inode/socket");
        // Block device
        else if (d_type == DT_BLK ) retval= g_strdup("inode/blockdevice");
        // Unknown:
        else if (d_type == DT_UNKNOWN) retval= g_strdup("inode/unknown");
        // Regular file:
        else if (d_type == DT_REG ) retval= g_strdup("inode/regular");
        else if (d_type == DT_LNK ) retval= g_strdup("inode/symlink");
	if (!d_type || d_type == DT_UNKNOWN) {
	    TRACE("Mime::basicMimeType: %d: %s\n", d_type, retval);
	}
	if (retval) return retval;
	return  g_strdup("inode/unknown");
    }
    
    static gchar *
    statMimeType (struct stat *st_p) {
	    if(S_ISSOCK (st_p->st_mode)) return g_strdup("inode/socket");
	    else if(S_ISBLK (st_p->st_mode)) return g_strdup("inode/blockdevice");
	    else if(S_ISCHR (st_p->st_mode)) return g_strdup("inode/chardevice");
	    else if(S_ISFIFO (st_p->st_mode)) return g_strdup("inode/fifo");
	    else if (S_ISLNK(st_p->st_mode)) return g_strdup("inode/symlink");
	    else if(S_ISDIR (st_p->st_mode)) return g_strdup("inode/directory");
	    else if(S_ISREG (st_p->st_mode)) return g_strdup("inode/regular");
	return  g_strdup("inode/unknown");
    }

    static gchar *
    mimeType (const gchar *file, struct stat *st_p) {
        if (!file){
	    ERROR("mimeType (file, st_p) file cannot be nil\n");
	    return g_strdup("inode/regular");
	}



        if (!st_p){
	    if(S_ISSOCK (st_p->st_mode)) return g_strdup("inode/socket");
	    else if(S_ISBLK (st_p->st_mode)) return g_strdup("inode/blockdevice");
	    else if(S_ISCHR (st_p->st_mode)) return g_strdup("inode/chardevice");
	    else if(S_ISFIFO (st_p->st_mode)) return g_strdup("inode/fifo");
	    else if (S_ISLNK(st_p->st_mode)) return g_strdup("inode/symlink");
	    else if(S_ISDIR (st_p->st_mode)) return g_strdup("inode/directory");
	}

        if(file[strlen (file) - 1] == '~' || file[strlen (file) - 1] == '%') {
            gchar *r_file = g_strdup(file);
            r_file[strlen (r_file) - 1] = 0;
            gchar *retval = mimeType(r_file, st_p);
            g_free(r_file);
            return retval;
        }
	auto type = MimeSuffix<Type>::mimeType(file);
	if (!type) type = g_strdup("inode/regular");
	return type;
        //return g_strdup("inode/regular");
        //return mimeMagic(file);
    }

private:
    static const gchar *
    get_mimetype_iconname(const gchar *mimetype){
	//FIXME: pull in value built from hash:
	return NULL;
        //return MimeHash<txt_hash_t>::lookup(mimetype, hash_data[GENERIC_ICON]); 
    }



public:

private:
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
		(GKeyFileFlags) (0), NULL);
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
    get_hash_key (const gchar * preKey) {
	return g_strdup(preKey);
    }

    /*static gchar *
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
    }*/
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
            ERROR ("%s: %s\n", msg, cmd_fmt);
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
