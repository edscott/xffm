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
static GtkWindow *mainWindow = NULL;
#include "fm.hh"


int
main (int argc, char *argv[]) {
    if (chdir(g_get_home_dir()) < 0){
        ERROR("Cannot chdir to %s (%s)\n", g_get_home_dir(), strerror(errno));
        exit(1);
    }
    xffindProgram = argv[0];
    xffmProgram = argv[0];

    auto fm = new(xf::Fm<double>)(argc, argv);
    xf::TimeoutResponse<double>::dialog(NULL, "Xffm+", "rodent_logo", 5);
    gtk_main();
    delete(fm);
    return 0;
}
