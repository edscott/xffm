/*
 * Copyright 2005-2012 Edscott Wilson Garcia 
 * license: GPL v.3
 */

#define XFFIND_CC
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

#include "intl.h"

# undef TRACE
# define TRACE(...)   { (void)0; }
//# define TRACE(...)  fprintf(stderr, "TRACE> "); fprintf(stderr, __VA_ARGS__);
# undef DBG
# define DBG(...)  fprintf(stderr, "DBG> "); fprintf(stderr, __VA_ARGS__);

static const gchar *xffindProgram;

#include "fgr.hh"
#include "find.hh"
#include "signals.hh"
int
main (int argc, char *argv[]) {
    xffindProgram = argv[0];
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

    TRACE ("call to setlocale");
    setlocale (LC_ALL, "");
    if (argv[1] && strcmp(argv[1],"--fgr")==0){
        xf::find::Fgr<double> *fgr = new(xf::find::Fgr<double>);
        fgr->main(argc, argv);
        exit(1);
    }
    TRACE ("call to gtk_init");
    gtk_init (&argc, &argv);
    xf::find::Find<xf::find::Signals<double>> dialog((const gchar *)argv[1]);
    //g_idle_add(set_up_dialog, path);
    gtk_main();

    return 0;
}
