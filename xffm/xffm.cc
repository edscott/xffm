/*
 * Copyright 2005-2018 Edscott Wilson Garcia 
 * license: GPL v.3
 */

#define XFFM_CC
#include "autotools.h"
#include "config.h"
#include "types.h"
#include "intl.h"


#define FORCE_CORE
#ifdef FORCE_CORE
# include <sys/time.h>
# include <sys/resource.h>
#endif

#ifdef HAVE_LIBMAGIC
# include <magic.h>
#endif

#include <memory>
#define USE_LOCAL_MONITOR 1
#define FORK

#define URIFILE "file://"

static const gchar *xffmProgram;
static const gchar *xffindProgram;
static GtkWindow *mainWindow = NULL;
static gboolean isTreeView;
#include "common/common.hh"
#include "completion/completion.hh"
#include "response/response.hh"
#include "find/find.hh"
#include "fm/fm.hh"



int
main (int argc, char *argv[]) {
    if (chdir(g_get_home_dir()) < 0){
        ERROR("Cannot chdir to %s (%s)\n", g_get_home_dir(), strerror(errno));
        exit(1);
    }
    xffindProgram = argv[0];
    xffmProgram = argv[0];

    auto fm = new(xf::Fm<double>)(argc, argv);
    //xf::Fm<double> *fm = std::make_shared<xf::Fm>(argc, argv);
    gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), FALSE);
    
#ifdef FORCE_CORE
    //auto text = g_strdup_printf("Xffm+-%s", VERSION);
    //xf::TimeoutResponse<double>::dialogFull(NULL, text, "xffm_logo", -200, 1);
    //g_free(text);
#endif
    gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), TRUE);
    gtk_main();
    //delete(fm);
    return 0;
}
