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
static GtkWindow *MainDialog = NULL;
static GList *textviewList = NULL;
static GList *run_button_list = NULL;
static GHashTable *iconPathHash = NULL;
static bool exitDialogs = false;
static void *threadPoolObject = NULL;
static pthread_mutex_t monitorMutex = PTHREAD_MUTEX_INITIALIZER;

#endif
