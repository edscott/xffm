/*
 * Copyright 2005-2018 Edscott Wilson Garcia 
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

#include "common/types.h"
static const gchar *xftermProgram;
static const gchar *xffindProgram;



#include "common/intl.h"
#include "common/response.hh"
#include "term.hh"

#include "find/fgr.hh"
#include "find/find.hh"
#include "find/signals.hh"


int
main (int argc, char *argv[]) {
    xffindProgram = argv[0];
    xftermProgram = argv[0];
    // common stuff
    // FIXME: this should be called as a class 
#include "common/startup.hh"

    auto xfterm = new(xf::termDialog<double>)(argv[1]);
//    auto xfterm = new(xf::termDialog<xf::completionSignals<double> >);
//    auto xfterm = new(xf::termDialog<xf::LpTerm >);
    xfterm->setDialogTitle("Term");
    xfterm->setDialogIcon("utilities-terminal");

    gtk_main();

    return 0;
}
