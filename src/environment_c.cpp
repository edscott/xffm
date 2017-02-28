
#include "environment_options.i"

void
rfm_init_env (void) {
    gint i;
    static gsize initialized = 0;
    if (g_once_init_enter (&initialized)){
      environ_t *environ_v = rfm_get_environ();
      // XXX: This initialization may now be obsolete:
      //      must check.
      for(i = 0; environ_v[i].env_var; i++) {
        // copy default values from static memory to heap
        if(strcmp (environ_v[i].env_var, "SUDO_ASKPASS") == 0 ||
	    strcmp(environ_v[i].env_var, "SSH_ASKPASS") == 0 ) {
            environ_v[i].env_string = g_find_program_in_path ("rodent-getpass");
	} 
	else if(environ_v[i].env_string) {
            environ_v[i].env_string = g_strdup (environ_v[i].env_string);
            NOOP ("ENV: %s %s\n", environ_v[i].env_var, environ_v[i].env_string);
        }
      }
      g_once_init_leave (&initialized, 1);
    }
    return;
}



void
rfm_setenv (const gchar *name, gchar *value, gboolean verbose) {
    rfm_init_env();
    NOOP(stderr, "setting %s to %s\n", name, value);
    gint which;
    gboolean go_ahead = FALSE;
    environ_t *environ_v = rfm_get_environ();
    for(which = 0; environ_v[which].env_var; which++){
        if(strcmp (name, environ_v[which].env_var) == 0){
            break;
	}
    }
    if(!environ_v[which].env_var) return;
    if(!value || !strlen (value)) {
        g_free (environ_v[which].env_string);
#ifdef HAVE_UNSETENV
        environ_v[which].env_string = NULL;
        unsetenv (name);
#else
        environ_v[which].env_string = g_strconcat (name, "=", NULL);
        putenv (environ_v[which].env_string);
#endif
        /*if(verbose) {
            if(strcmp (name, "SMB_USER") == 0) {
                TRACE("Mcs manager changed rfm environment: %s\n", name);
            } else {
                TRACE("Mcs manager changed rfm environment: %s=%s\n", name, ((value) ? value : " "));
            }
        }*/
        return;
    }
    if(strcmp (name, "RFM_MAX_PREVIEW_SIZE") == 0) {
        if(is_number (value))
            go_ahead = TRUE;
    } else if(strcmp (name, "TERMCMD") == 0) {
        if(value && strlen (value)) {
            gchar *c;
            gchar *t = g_strdup (value);
            t = g_strstrip (t);
            if(strchr (t, ' '))
                t = strtok (t, " ");
            c = g_find_program_in_path (t);
            if(c && access (c, X_OK) == 0) {
                go_ahead = TRUE;
            }
            g_free (c);
            c = NULL;
            g_free (t);
            t = NULL;
        }
    } else
        go_ahead = TRUE;
    if(go_ahead) {
        g_free (environ_v[which].env_string);
	gchar *getpass = NULL;
	gchar *editor = NULL;
	if (strcmp (name, "EDITOR") == 0){
	    editor = rfm_get_text_editor_envar(value);
	    NOOP(stderr, "Setting text editor to %s\n", editor);
	}
	if (editor){
	    value = editor;
	} else {
	    if (strcmp (name, "SUDO_ASKPASS") == 0 ||
		strcmp(name, "SSH_ASKPASS") == 0 ){
		if (!g_file_test(value, G_FILE_TEST_EXISTS)){
		    getpass = g_find_program_in_path ("rodent-getpass");
		}
	    }
	}
	if (getpass) value = getpass;


	NOOP(stderr, "rfm_setenv(): setting %s -> %s \n", name, value);
#ifdef HAVE_SETENV
        environ_v[which].env_string = g_strdup (value);
        setenv (name, environ_v[which].env_string, 1);
#else
        environ_v[which].env_string = g_strconcat (name, "=", value, NULL);
        putenv (environ_v[which].env_string);
#endif
	g_free(editor);
    } else {                    /* not go_ahead */
        DBG ("failed to change rfm environment: %s\n", name);
    }
    return;
}

// This function gets static information. Better would be to get the
// number of currently available cores for processing.
static gint
get_max_threads(void){
    gint cores=MAX_PREVIEW_THREADS;
    gchar *nproc=g_find_program_in_path("nproc");
    if (nproc){
        // LINUX
        FILE *pipe = popen (nproc, "r");
	if(pipe) {
            gchar buf[256];
	    memset(buf, 0, 256);
            if (fgets (buf, 255, pipe) && !feof(pipe)){
	        errno = 0;
                long lcore = strtol(buf, NULL, 10);
                if (!errno && lcore > 0) cores = lcore;
            }
            pclose(pipe);
	}
	g_free (nproc);
        return cores;
    }
    // No nproc?
    gchar *sysctl=g_find_program_in_path("sysctl");
    if (sysctl){
        // BSD
        gchar *cmd = g_strdup_printf("%s -a", sysctl);
        g_free(sysctl);
        FILE *pipe = popen (cmd, "r");
	if(pipe) {
            gchar buf[256];
	    memset(buf, 0, 256);
            while (fgets (buf, 255, pipe) && !feof(pipe)){
                if (strstr(buf,"hw.ncpu:")){
	            errno = 0;
                    long lcore = strtol(buf+strlen("hw.ncpu:"), NULL, 10);
                    // overflow not going to happen.
                    // coverity[overflow_assign : FALSE]
                    if (!errno && lcore > 0) cores = lcore;
                    break;
                }
            }
            pclose(pipe);
	}
	g_free (cmd);
    }

    return cores;
}

static GThread *gtk_thread=NULL;
void rfm_set_gtk_thread(GThread *thread){
    gtk_thread = thread;
}

GThread *rfm_get_gtk_thread(void){
    return gtk_thread;
}

static void my_mkdir(gchar *g){
    g_mkdir_with_parents (g, 0700);
    g_free(g);
}
void rfm_init(void){
#ifdef ENABLE_NLS
    /* this binds rfm domain: */
    bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
# ifdef HAVE_BIND_TEXTDOMAIN_CODESET
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
# endif
#endif
    rfm_set_gtk_thread(g_thread_self());
    my_mkdir(g_build_filename (DEFAULT_DESKTOP_DIR, NULL));
    my_mkdir(g_build_filename (USER_PIXMAPS, NULL));
    my_mkdir(g_build_filename (USER_RFM_DIR, NULL));
    my_mkdir(g_build_filename (USER_RFM_CACHE_DIR, NULL));
    my_mkdir(g_build_filename (USER_DBH_DIR, NULL));
    my_mkdir(g_build_filename (USER_DBH_CACHE_DIR, NULL));
    my_mkdir(g_build_filename (RFM_THUMBNAIL_DIR, NULL));
}


static environ_t environ_v[RFM_OPTIONS + 1] = {        
    // +1 because zero does not count in enum...
    /* general:: */
    { "RFM_DOUBLE_CLICK_NAVIGATION", "", N_("Activate items with a double click"), NULL}, 
    {"RFM_VERTICAL_TOOLBAR","",N_("Use vertical toolbar"),NULL},
    { "RFM_USE_GTK_ICON_THEME", "", N_("Icon Theme Specification"), NULL}, 
    { "RFM_DRAG_DOES_MOVE", "Yes", N_("Drag: move"), NULL}, 
    { "RFM_CONTENT_FOLDER_ICONS", N_("Yes"), N_("Emblems"), NULL},
    { "RFM_ENABLE_TIPS", N_("Yes"), N_("Enable tooltips"), NULL},
    { "RFM_ENABLE_LABEL_TIPS", N_("Yes"), N_("Enable tooltips"), NULL},
    {"RFM_PREVIEW_IMAGE_SIZE","",N_("Preview image size in pixels"),NULL},
    { "RFM_FIXED_FONT_SIZE", "9", N_("The font size"), NULL},
    { "RFM_VARIABLE_FONT_SIZE", "9", N_("The font size"), NULL},
    { "RFM_FIXED_FONT_FAMILY", "monospace", N_("The font family"), NULL},
    { "RFM_VARIABLE_FONT_FAMILY", "serif", N_("The font family"), NULL},
    { "RFM_DEFAULT_ICON_SIZE", "Normal", N_("Icon size"), NULL},
    { "TERMINAL_CMD", "roxterm", N_("Terminal Emulator"), NULL}, 
    { "EDITOR", "gvim -f", N_("Text Editor"), NULL},
    { "RFM_MAXIMUM_COMPLETION_OPTIONS", "104", N_("Maximum completion options displayed"), NULL},
    { "RFM_LOAD_TIMEOUT", "5", N_("Maximum time (seconds) to wait for a load directory"), NULL},
    { "RFM_MAXIMUM_DIAGNOSTIC_LINES", "1000", N_("Maximum lines in lp terminal buffer"), NULL},
    /* desktop:: */
    { "RFM_ENABLE_DESKTOP", "", N_("Show Desktop Grid"), NULL}, 
    { "RFM_ENABLE_DESKTOP_DIAGNOSTICS", N_("Yes"), N_("Console Message Viewer"), NULL},
    { "RFM_NAVIGATE_DESKTOP", "", N_("Navigation Window"), NULL}, 

    { "RFM_DESKTOP_TOP_MARGIN", "20", N_("Top Margin"), NULL},
    { "RFM_DESKTOP_BOTTOM_MARGIN", "40", N_("Bottom Margin"), NULL},
    { "RFM_DESKTOP_RIGHT_MARGIN", "25", N_("Right margin"), NULL},
    { "RFM_DESKTOP_LEFT_MARGIN", "50", N_("Left Margin"), NULL},
   
    { "RFM_DESKTOP_DIR", "", N_("Desktop path:"), NULL},
    { "RFM_DESKTOP_IMAGE", PREFIX"/share/images/roa153b.jpg", N_("Background image"), NULL}, 
    { "RFM_DESKTOP_COLOR", "#4C1E0C", N_("Background color"), NULL}, 
    { "RFM_ICONVIEW_COLOR", "#383C3F", N_("Background color"), NULL},
    { "RFM_TRANSPARENCY", "", N_("Background transparency:"), NULL},

    { "RFM_PLUGIN_FLAGS", "0xffffffff", "plugin bitflags (lite)", NULL}, 
    { "RFM_MODULE_FLAGS", "0xffffffff", "module bitflags (lite)", NULL}, 

    {"RFM_TOOLBAR", "", N_("Toolbar configuration")},
    {"RFM_PASTEBOARD_SERIAL", "0", N_("Pasteboard serial control"), NULL},
    {"RFM_BOOKMARK_SERIAL", "0", N_("Bookmark serial control"), NULL},

    ///// core options ///////////////////////////////////////////////////////
    { "RFM_SHRED_FLAGS", "0x163", NULL, NULL}, 
#ifdef GNU_CP
    { "RFM_LS_FLAGS", "0x2040400011", NULL, NULL}, 
    { "RFM_CP_FLAGS", "0x8000891", NULL, NULL}, 
    { "RFM_MV_FLAGS", "0x40b", NULL, NULL}, 
    { "RFM_LN_FLAGS", "0x1113", NULL, NULL}, 
    { "RFM_RM_FLAGS", "0x3aa", NULL, NULL}, 
#else
    { "RFM_LS_FLAGS", "0x820080", NULL, NULL}, 
    { "RFM_CP_FLAGS", "0xc91", NULL, NULL}, 
    { "RFM_MV_FLAGS", "0x15", NULL, NULL}, 
    { "RFM_LN_FLAGS", "0x791", NULL, NULL}, 
    { "RFM_RM_FLAGS", "0x3ea", NULL, NULL}, 
#endif

    // core option parameters
    { "RFM_SHRED_iterations", "3", NULL, shred_iterations}, 
    { "RFM_SHRED_size", "100K", NULL, shred_size}, 

#ifdef GNU_LS
    { "RFM_LS_ignore", "", "", NULL},
    { "RFM_LS_tabsize", "", "", NULL},
    { "RFM_LS_blocksize", "", "", NULL},
    { "RFM_LS_hide", "", "", NULL},
    { "RFM_LS_width", "", "", NULL},
    { "RFM_LS_format", NULL, NULL, ls_format},
    { "RFM_LS_istyle", NULL, NULL, ls_istyle},
    { "RFM_LS_qstyle", NULL, NULL, ls_qstyle},
    { "RFM_LS_sort", NULL, NULL, ls_sort},
    { "RFM_LS_time", NULL, NULL, ls_time},
    { "RFM_LS_tstyle", NULL, NULL, ls_tstyle},
#endif

#ifdef GNU_CP
    { "RFM_CP_backup", NULL, NULL, cp_v_control}, 
    { "RFM_CP_suffix", NULL, NULL, cp_suffix}, 
    { "RFM_CP_preserve", NULL, NULL, cp_attributes}, 
    { "RFM_CP_no_preserve", NULL, NULL, cp_attributes}, 
    { "RFM_CP_reflink", NULL, NULL, cp_when}, 
    { "RFM_CP_sparse", NULL, NULL, cp_when}, 
#endif

#ifdef GNU_MV
    { "RFM_MV_backup", NULL, NULL, cp_v_control}, 
    { "RFM_MV_suffix", NULL, NULL, cp_suffix}, 
#endif

#ifdef GNU_LN
    { "RFM_LN_backup", NULL, NULL, cp_v_control}, 
    { "RFM_LN_suffix", NULL, NULL, cp_suffix}, 
#endif

#ifdef GNU_RM
    { "RFM_RM_interactive", NULL, NULL, rm_interactive}, 

#endif


    {"SMB_USER", "", N_("Samba default remote user"), NULL}, 
    {"SUDO_ASKPASS", PREFIX"/bin/rodent-getpass", N_("Sudo ask password program"), NULL}, 
    {"SSH_ASKPASS", PREFIX"/bin/rodent-getpass", N_("Ssh ask passphrase program"), NULL}, 

    
    { "VERSION_CONTROL", "existing", NULL, cp_v_control},
    { "PWD", "", "third party stuff", NULL}, 

    { NULL, NULL, NULL}

};

environ_t  *rfm_get_environ(void){return environ_v;}

