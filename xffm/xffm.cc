/*
 * Copyright 2005-2018 Edscott Wilson Garcia 
 * license: GPL v.3
 */

#define XFFM_CC
#include "config.h"
// Run in background, detached.
// Otherwise, use argv[1]=="-f"

// Version 0.93, enable fstab and ecryptfs (Linux)
#ifdef BSD_NOT_FOUND
# undef ENABLE_EFS_MODULE 
# define ENABLE_EFS_MODULE 1
#endif

#ifdef BSD_FOUND
# undef ENABLE_EFS_MODULE 
#endif


# define ENABLE_FSTAB_MODULE 1

#ifdef ALPHA

# warning "Alpha modules enabled..."
// Templates in test mode:
# undef ENABLE_CUSTOM_RESPONSE 
# define ENABLE_CUSTOM_RESPONSE 1
# undef ENABLE_PKG_MODULE 
# define ENABLE_PKG_MODULE 1
# undef ENABLE_DIFF_MODULE 
# define ENABLE_DIFF_MODULE 1
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

//#define XFFM_ICON "emblem-bsd"
#define XFFM_ICON "rodent_logo"
#define FSTAB_ICON "media-eject/SE/drive-harddisk/2.0/225"
#define EFS_ICON "drive-harddisk/SE/emblem-readonly/2.0/225"
#define HOME_ICON "go-home"
#define TRASH_ICON "user-trash"
#define TRASH_FULL_ICON "user-trash-full"

#ifdef HAVE_EMERGE 
# define PKG_ICON "emblem-gentoo"
#else
# ifdef HAVE_PACMAN
# define PKG_ICON "emblem-archlinux"
# else
# define PKG_ICON "emblem-bsd"
# endif
#endif



#define URIFILE "file://"
#define USER_DIR                 g_get_home_dir()


namespace xf {
    template <class Type> class Fm;
}


# undef TRACE
# define TRACE(...)   { (void)0; }
//# define TRACE(...)  {fprintf(stderr, "TRACE> "); fprintf(stderr, __VA_ARGS__);}
# undef ERROR
# define ERROR(...)  {fprintf(stderr, "ERROR> "); fprintf(stderr, __VA_ARGS__);}
# define ERROR_(...)  {auto errorText = g_strdup_printf(__VA_ARGS__);xf::Fm<Type>::printError(errorText); }
# undef INFO
# define INFO(...)  {fprintf(stderr, "INFO> "); fprintf(stderr, __VA_ARGS__);}
# define INFO_(...)  {auto errorText = g_strdup_printf(__VA_ARGS__);xf::Fm<Type>::printInfo(errorText); }

# undef DBG

#ifdef NODEBUG
# define DBG(...)   { (void)0; }
#else
# define DBG(...)  {fprintf(stderr, "DBG***> "); fprintf(stderr, __VA_ARGS__);}
#endif

# undef DBG_
# define DBG_(...)  {auto errorText = g_strdup_printf(__VA_ARGS__);xf::Fm<Type>::printDbg(errorText); }


namespace xf {
    template <class Type> class Thread;
}

static gchar *buildDir=NULL;
static const gchar *buildXml=NULL;
static const gchar *buildIcons=NULL;
static const gchar *xffmProgram;
static const gchar *xffindProgram;
static GtkWindow *mainWindow = NULL;
static gboolean isTreeView = FALSE;
GList *customDialogs = NULL;

#include "gtk/gtk3.hh"
#include "common/common.hh"
#include "mime/mime.hh"
#include "completion/completion.hh"
#include "response/response.hh"
#include "find/find.hh"
#include "fm/fm.hh"



int
main (int argc, char *argv[]) {
    TRACE("argv[0]= %s\n", argv[0]);
    buildDir = g_path_get_dirname(argv[0]);
    if (!g_path_is_absolute(buildDir)){
        auto current = g_get_current_dir();
        auto g = g_build_path(G_DIR_SEPARATOR_S, current, buildDir, NULL);
        TRACE("current=%s g=%s\n", current, g);
        g_free(current);
        g_free(buildDir);
        buildDir = g;
    }
    TRACE("dir= %s\n", buildDir);
    if (strstr(buildDir, "/build/xffm")){
        *strstr(buildDir, "/build/xffm") = 0;
        buildXml = g_build_path(G_DIR_SEPARATOR_S,buildDir, "xffm", "xml", NULL);
        buildIcons = g_build_path(G_DIR_SEPARATOR_S,buildDir, "xffm", "icons", NULL);
        TRACE("buildIcons=%s\n", buildIcons);
    } 


    if (chdir(g_get_home_dir()) < 0){
        fprintf(stderr, "xffm.cc::Cannot chdir to %s (%s)\n", g_get_home_dir(), strerror(errno));
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
