#ifndef XFFM_PLUS_X
# define XFFM_PLUS_X
# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif
#ifdef HAVE_LIBMAGIC
#include <magic.h>
#else
#error "libmagic not found during configure!"
#endif

#include <string.h>
#include <strings.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include <pthread.h>
#include <iostream>

# include "intl.h"
# include "debug.h"
#define USER_RFM_CACHE_DIR      g_get_user_cache_dir(),"rfm"

enum {
    MATCH_COMMAND,
    MATCH_FILE,
    MATCH_HISTORY,
    MATCH_USER,
    MATCH_VARIABLE,
    MATCH_HOST,
    MATCH_NONE
};

#endif

