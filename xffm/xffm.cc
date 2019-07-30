/*
 * Copyright 2005-2018 Edscott Wilson Garcia 
 * license: GPL v.3
 */

#define XFFM_CC
#define ALPHA

#ifdef ALPHA
# warning "Alpha modules enabled..."
# undef ENABLE_CUSTOM_RESPONSE 
# define ENABLE_CUSTOM_RESPONSE 1
# undef ENABLE_PKG_MODULE 
# define ENABLE_PKG_MODULE 1
# undef ENABLE_DIFF_MODULE 
# define ENABLE_DIFF_MODULE 1
# undef ENABLE_FSTAB_MODULE 
# define ENABLE_FSTAB_MODULE 1
# define FORCE_CORE


#endif

#include "autotools.h"
#include "config.h"
#include "types.h"
#include "intl.h"


#define USE_LOCAL_MONITOR 1
//#define FORK 1
#ifdef FORCE_CORE
# warning "Core dump enabled..."
#define CORE 1
# include <sys/time.h>
# include <sys/resource.h>
#endif

#ifndef FORK
# warning "Fork not active..."
#endif

#ifdef HAVE_LIBMAGIC
# include <magic.h>
#endif

#include <memory>

#define URIFILE "file://"

static const gchar *xffmProgram;
static const gchar *xffindProgram;
static GtkWindow *mainWindow = NULL;
static gboolean isTreeView = FALSE;
GList *customDialogs = NULL;
#include "common/common.hh"
#include "mime/mime.hh"
#include "completion/completion.hh"
#include "response/response.hh"
#include "find/find.hh"
#include "fm/fm.hh"



int
main (int argc, char *argv[]) {
    if (chdir(g_get_home_dir()) < 0){
        ERROR("xffm.cc::Cannot chdir to %s (%s)\n", g_get_home_dir(), strerror(errno));
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
