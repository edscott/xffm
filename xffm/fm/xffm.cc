/*
 * Copyright 2005-2018 Edscott Wilson Garcia 
 * license: GPL v.3
 */

#define XFFM_CC
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
static const gchar *xffmProgram;
static const gchar *xftermProgram;
static const gchar *xffindProgram;



#include "common/intl.h"
#include "common/response.hh"
#include "fm.hh"

#include "find/fgr.hh"
#include "find/find.hh"
#include "find/signals.hh"


int
main (int argc, char *argv[]) {
    xffindProgram = argv[0];
    xftermProgram = argv[0];
    xffmProgram = argv[0];
    // common stuff
    // FIXME: this should be called as a class 
#include "common/startup.hh"

    auto xffm = new(xf::fmDialog<double>)(path);
    xffm->setDialogTitle("Fm");
    xffm->setDialogIcon("system-file-manager");

    gtk_main();

    return 0;
}
