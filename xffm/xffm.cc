/*
 * Copyright 2005-2018 Edscott Wilson Garcia 
 * license: GPL v.3
 */

#define XFFM_CC
#include "config.h"

#ifdef ALPHA

# warning "Alpha modules enabled..."
// Easier debugging:
# undef FORK 
# warning "Fork not active..."
// Templates in test mode:
# undef ENABLE_CUSTOM_RESPONSE 
# define ENABLE_CUSTOM_RESPONSE 1
# undef ENABLE_PKG_MODULE 
# define ENABLE_PKG_MODULE 1
# undef ENABLE_DIFF_MODULE 
# define ENABLE_DIFF_MODULE 1
# undef ENABLE_FSTAB_MODULE 
# define ENABLE_FSTAB_MODULE 1
// Core dumps for debugging:
# define FORCE_CORE
# warning "Core dump enabled..."
# define CORE 1
# include <sys/time.h>
# include <sys/resource.h>

#else
// No templates in test mode.
// No default core dumps (system setting may override).
// Run in background, detached.
# define FORK 1
#endif

#include "types.h"

#ifdef ENABLE_NLS
# include <libintl.h>
# define _(String) dgettext(GETTEXT_PACKAGE,String)
# define N_(String)  String

#else
# warning "Translations not enabled: Gettext not found during configure."
# define _(String) String
# define N_(String) String
# define ngettext(Format1,Format2,N) Format1
# define textdomain(String) 
# define bindtextdomain(Domain,Directory)
#endif


#define USE_LOCAL_MONITOR 1


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
