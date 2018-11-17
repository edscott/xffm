/*
 * Copyright 2005-2018 Edscott Wilson Garcia 
 * license: GPL v.3
 */

#define XFFM_CC
#include "config.h"
#include "types.h"
#include "intl.h"


#define FORCE_CORE
#ifdef FORCE_CORE
# include <sys/time.h>
# include <sys/resource.h>
#endif
#include <memory>

static const gchar *xffmProgram;
static const gchar *xffindProgram;
static GtkWidget *mainWindow = NULL;

#include "common/tubo.hh"
#include "common/print.hh"
#include "common/run.hh"
#include "common/util.hh"
#include "common/gtk.hh"
#include "common/tooltip.hh"
#include "common/pixbuf.hh"
#include "common/icons.hh"
#include "common/settings.hh"
#include "common/mime.hh"


#include "find/fgr.hh"
#include "find/find.hh"
#include "find/signals.hh"

#include "response/passwdresponse.hh"
#include "fm.hh"



int
main (int argc, char *argv[]) {
    xffindProgram = argv[0];
    xffmProgram = argv[0];
    // common stuff
    // FIXME: this should be called as a class 
#include "startup.hh"

    auto xffm = new(xf::fmDialog<double>)(path);
    xffm->setDialogTitle("Fm");
    xffm->setDialogIcon("system-file-manager");

    gtk_main();

    return 0;
}
