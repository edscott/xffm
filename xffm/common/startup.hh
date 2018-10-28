#ifdef ENABLE_NLS
    /* this binds rfm domain: */
    bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);

    bindtextdomain ("librfm", PACKAGE_LOCALE_DIR);
    bindtextdomain ("rodent", PACKAGE_LOCALE_DIR);
# ifdef HAVE_BIND_TEXTDOMAIN_CODESET
    bind_textdomain_codeset ("librfm", "UTF-8");
    bind_textdomain_codeset ("rodent", "UTF-8");
# endif
# ifdef HAVE_BIND_TEXTDOMAIN_CODESET
    TRACE ("binding %s, at %s", GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
# endif
#endif
    /* ignore hangups? */
    (void)signal (SIGHUP, SIG_IGN);
#define CORE 1
#ifdef CORE
    struct rlimit rlim;
    if (!strstr(argv[0], "getpass")) {
	fprintf(stderr, "Enabling core dumps...\n");
    }
    rlim.rlim_cur = RLIM_INFINITY;
    rlim.rlim_max = RLIM_INFINITY;
    setrlimit (RLIMIT_CORE, &rlim);
#endif

    TRACE ("call to setlocale");
    setlocale (LC_ALL, "");
    if (argv[1] && strcmp(argv[1],"--fgr")==0){
        xf::Fgr<double> *fgr = new(xf::Fgr<double>);
        fgr->main(argc, argv);
        return 0;
    }


    
    TRACE ("call to gtk_init");
    gtk_init (&argc, &argv);
    if (argv[1] && strcmp(argv[1],"--find")==0){
        xf::Find<xf::findSignals<double>> dialog((const gchar *)argv[2]);
        //g_idle_add(set_up_dialog, path);
        gtk_main();
        return 0;
    }

    // In order for SSH_ASKPASS to work (for sshfs or for executing
    // ssh commands at the lpterminal) we must detach the tty. 
    // This interferes with stepwise debugging with gdb.
    // Get password for ssh or sudo
    if (strstr(argv[0], "xfgetpass")){
        xf::Response<double>::sendPassword(argv+1);
        exit(1);
    } else {
//#define FORK
#ifdef FORK
	if(fork ()){
	    sleep(2);
	    _exit (123);
	}
	setsid(); // detach main process from tty
#endif
    }

    const gchar *path = argv[1];

    // FIXME: set these environment variables *only* if they are not already set
    //        in the environment (allow user override)

    // XXX xterm is mandatory...
    
    setenv("TERMINAL", "xterm -rv", 1);
    const gchar *term_cmd = getenv("TERMINAL_CMD");
    if (!term_cmd || !strlen(term_cmd)) term_cmd = "xterm -e";

    gchar *getpass = g_find_program_in_path("xfgetpass");
    if (!getpass) {
        std::cerr<<"*** Warning: Xffm not correctly installed. Cannot find xfgetpass in path\n";
    } else {
        DBG("get pass at %s\n", getpass);
        setenv("SUDO_ASKPASS", getpass, 1);
        setenv("SSH_ASKPASS", getpass, 1);
    }
    
    gchar *e = NULL;
    if (getenv("EDITOR")) e = g_find_program_in_path(getenv("EDITOR"));
    if (e && strstr(e, "nano")){
          e = g_strdup_printf("%s nano", term_cmd);
    }
     
    if (!e){
        e = g_find_program_in_path("gvim");
        if (!e){
            e = g_find_program_in_path("vi");
            if (!e) {
                e = g_find_program_in_path("nano");
                if(!e){
                    // nano is mandatory
                    std::cerr<<"*** Warning: No suitable EDITOR found (tried gvim, vi, nano)\n";
                } else {
                    g_free(e);
                    e = g_strdup_printf("%s nano", term_cmd);
                }
            } else {
                g_free(e);
                e = g_strdup_printf("%s vi", term_cmd);
            }
        } else {
            g_free(e);
            e = g_strdup("gvim -f");
        }
    }
    setenv("EDITOR", e, 1);

