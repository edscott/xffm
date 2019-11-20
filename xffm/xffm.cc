/*
 * Copyright 2005-2018 Edscott Wilson Garcia 
 * license: GPL v.3
 */

#define XFFM_CC
#include "config.h"

// Run in background, detached.
# define FORK 1

// Version 0.93, enable fstab and ecryptfs (Linux)
#ifdef FREEBSD_NOT_FOUND
# undef ENABLE_FSTAB_MODULE 
# define ENABLE_FSTAB_MODULE 1
# undef ENABLE_EFS_MODULE 
# define ENABLE_EFS_MODULE 1
#endif

#ifdef ALPHA

# warning "Alpha modules enabled..."
// Templates in test mode:
# undef ENABLE_CUSTOM_RESPONSE 
# define ENABLE_CUSTOM_RESPONSE 1
# undef ENABLE_PKG_MODULE 
# define ENABLE_PKG_MODULE 1
# undef ENABLE_DIFF_MODULE 
# define ENABLE_DIFF_MODULE 1
// Easier debugging:
//# undef FORK 
//# warning "Fork not active..."
// Core dumps for debugging:
# define FORCE_CORE
# warning "Core dump enabled..."
# define CORE 1
# include <sys/time.h>
# include <sys/resource.h>

#else
// No templates in test mode.
// No default core dumps (system setting may override).
#endif

#include "types.h"


#define USE_LOCAL_MONITOR 1


#ifdef HAVE_LIBMAGIC
# include <magic.h>
#endif

#include <memory>

#define URIFILE "file://"
#define USER_DIR 		g_get_home_dir()

static const gchar *xffmProgram;
static const gchar *xffindProgram;
static GtkWindow *mainWindow = NULL;
static gboolean isTreeView = FALSE;
GList *customDialogs = NULL;

#include "gtk/cairo.hh"
#include "gtk/pixbufhash.hh"
#include "gtk/icons.hh"
#include "gtk/pixbuf.hh"
#include "gtk/dialogs.hh"
#include "gtk/gtk.hh"


#include "common/common.hh"

#include "mime/mime.hh"
#include "completion/completion.hh"
#include "response/response.hh"
#include "find/find.hh"
#include "fm/fm.hh"



int
main (int argc, char *argv[]) {
#ifndef FORK
    DBG("FORK disabled: SSH_ASKPASS will not work.");
#endif
    if (chdir(g_get_home_dir()) < 0){
        ERROR("xffm.cc::Cannot chdir to %s (%s)\n", g_get_home_dir(), strerror(errno));
        exit(1);
    }
    xffindProgram = argv[0];
    xffmProgram = argv[0];
#ifdef ENABLE_NLS
	/* this binds domain: */
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);

	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
# ifdef HAVE_BIND_TEXTDOMAIN_CODESET
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	TRACE ("binding %s, at %s\n", GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
# endif
#endif

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
