/*
 * Copyright 2005-2012 Edscott Wilson Garcia 
 * license: GPL v.3
 */

#define XFTERM_CC
#include  "config.h"



#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <glob.h>
#include <limits.h>
#include <memory.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>

#include <iostream>

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gmodule.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#define FORCE_CORE
#ifdef FORCE_CORE
# include <sys/time.h>
# include <sys/resource.h>
#endif


# undef TRACE
# define TRACE(...)   { (void)0; }
//# define TRACE(...)  fprintf(stderr, "TRACE> "); fprintf(stderr, __VA_ARGS__);
# undef DBG
# define DBG(...)  fprintf(stderr, "DBG> "); fprintf(stderr, __VA_ARGS__);
static const gchar *xftermProgram;

#include "common/intl.h"
#include "common/response.hh"
#include "term.hh"
int
main (int argc, char *argv[]) {
    xftermProgram = argv[0];
    /* start loading required dynamic libraries here... */
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

    TRACE ("call to setlocale");
    setlocale (LC_ALL, "");
    TRACE ("call to gtk_init");
    gtk_init (&argc, &argv);

    // In order for SSH_ASKPASS to work (for sshfs or for executing
    // ssh commands at the lpterminal) we must detach the tty. 
    // This interferes with stepwise debugging with gdb.
    // Get password for ssh or sudo
    if (strstr(argv[0], "xfgetpass")){
        xf::Response<double>::sendPassword(argv+1);
        exit(1);
    } else {
	if(fork ()){
	    sleep(2);
	    _exit (123);
	}
	setsid(); // detach main process from tty
    }
    gchar *getpass = g_find_program_in_path("xfgetpass");
    if (!getpass) {
        std::cerr<<"*** Warning: Xffm not correctly installed. Cannot find xfgetpass in path\n";
    } else {
        DBG("get pass at %s\n", getpass);
        setenv("SUDO_ASKPASS", getpass, 1);
        setenv("SSH_ASKPASS", getpass, 1);
    }




    //xf::termDialog<double> term("Term","utilities-terminal");
    //term.createDialog("/home");

    auto xfterm = new(xf::termDialog<double>);
//    auto xfterm = new(xf::termDialog<xf::completionSignals<double> >);
//    auto xfterm = new(xf::termDialog<xf::LpTerm >);
    xfterm->setDialogTitle("Term");
    xfterm->setDialogIcon("utilities-terminal");

    gtk_main();

    return 0;
}
