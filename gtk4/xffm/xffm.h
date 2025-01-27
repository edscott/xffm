/*
 * Copyright 2005-2024 Edscott Wilson Garcia 
 * license: GPL v.3
 */
#ifndef XFFM_H
#define XFFM_H
#include "config.h"
#include "types.h"

#ifdef HAVE_ZIP_H
# include <zip.h>
#endif

#include <memory>
#include <cassert>

#ifdef HAVE_LIBMAGIC
# include <magic.h>
#endif


#define URIFILE "file://"
#define USER_DIR                 g_get_home_dir()
#define USE_LOCAL_MONITOR 1
#define ALPHA
#define USER_CACHE_DIR      g_get_user_cache_dir(),"xffm+"
#define XFTHUMBNAIL_DIR         USER_CACHE_DIR,"thumbnails"

#ifdef ALPHA
# define ENABLE_FSTAB_MODULE 1
# define FORCE_CORE
//# warning "Core dump enabled..."
# define CORE 1
# include <sys/time.h>
# include <sys/resource.h>
#endif


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
# define DBG(...)  {fprintf(stderr, "DBG> "); fprintf(stderr, __VA_ARGS__);}
#endif

# undef DBG_
# define DBG_(...)  {auto errorText = g_strdup_printf(__VA_ARGS__);xf::Fm<Type>::printDbg(errorText); }

#define DEFAULT_FIXED_FONT_SIZE 12
#define DEFAULT_FONT_SIZE    "12"
#define PREVIEW_IMAGE_SIZE  400
#define DEFAULT_FONT_FAMILY    "Sans"


static const gchar *xffmProgram;
static const gchar *xffindProgram;
static GtkWidget *MainWidget = NULL;
static GList *textviewList = NULL;
static GList *run_button_list = NULL;
static GHashTable *iconPathHash = NULL;
static bool exitDialogs = false;
static void *threadPoolObject = NULL;
static GtkButton *mainMenuButton = NULL;
static GtkButton *cutButton = NULL;
static GtkButton *copyButton = NULL;
static GtkButton *pasteButton = NULL;
static int longPressSerial = -1;
static GtkNotebook *mainNotebook = NULL;

// classes and templates
// basic classes
#include "fm/classes/icontheme.hh"  
#include "fm/classes/basic.hh"     
#include "fm/classes/child.hh"  
#include "fm/classes/settings.hh"  
#include "mime/mime.hh"
#include "fm/templates/texture.hh"  
#include "fm/classes/print.hh"  
#include "fm/classes/tubo.hh"  

#include "fm/templates/properties.hh"
#include "fm/classes/thread.hh"
#include "fm/templates/preview.hh"
#include "fm/classes/progress.hh"
#include "fm/templates/clipboard.hh"
#include "fm/classes/bash.hh"

#include "fm/classes/utilbasic.hh"
#include "fm/classes/css.hh"
#include "fm/classes/pathbarhistory.hh"     
#include "fm/templates/basicpathbar.hh"     


#include "fm/classes/bookmarks.hh"
#include "fm/classes/fstabutil.hh"

// basic templates
#include "fm/templates/run.hh"  
#include "fm/templates/runbutton.hh"

#include "fm/classes/localdir.hh"       // fm class
// dialogs 
#include "response/dialogs.hh"

#include "fm/classes/history.hh"

#include "find/find.hh"

// menu templates
#include "menus/templates/menu.hh"
#include "menus/templates/menucallbacks.hh"
#include "menus/templates/gridviewmenu.hh"
#include "menus/templates/pathbarmenu.hh"
#include "menus/templates/outputMenu.hh"
#include "menus/templates/inputMenu.hh"
#include "menus/templates/mainMenu.hh"
#include "menus/templates/iconColorMenu.hh"

// 


// fm classes/templates
//#include "fm/classes/localdir.hh"       // fm class
#include "fm/classes/fstab.hh"       // fm class
#include "fm/classes/root.hh"       // fm class
#include "fm/templates/dnd.hh"     // fm template
                                   //
#include "fm/templates/factory.hh"     // fm template
#include "fm/templates/gridview.hh"     // fm template
#include "fm/templates/utilpathbar.hh"  // fm template
#include "fm/templates/pathbar.hh"      // fm template
#include "fm/templates/workdir.hh"      // fm template
#include "fm/templates/util.hh"         // fm template
                                        //
#include "fm/templates/prompt.hh"       // fm template
#include "fm/classes/fmbuttonbox.hh"    // fm class
#include "fm/classes/fmpage.hh"         // fm class

#include "fm/templates/fstabmonitor.hh"

#include "fm/templates/window.hh" // template
#include "fm/classes/fm.hh"     // class

#endif
