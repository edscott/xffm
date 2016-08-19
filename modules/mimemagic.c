/*
 *
 * Edscott Wilson Garcia 2001-2012 edscott@users.sf.net
 *
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; .
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "rodent.h"

/* this should be first 2 lines after headers: */
G_MODULE_EXPORT LIBRFM_MODULE

/****************************************************************************/

#ifdef HAVE_LIBMAGIC
static GMutex *magic_mutex = NULL;
static magic_t cookie;
static gsize initialized = 0;
#endif
 
G_MODULE_EXPORT void *
module_active (void){
    return GINT_TO_POINTER(1);
}

#ifdef HAVE_LIBMAGIC

// Lib magic is available...
//
// not thread safe: put in a mutex.
// This function may obtain a basic or alias mimetype, but will always
// return a basic mimetype.
static gchar *
lib_magic (const gchar * file, int flags) {
    gchar *type=NULL;
    g_mutex_lock (magic_mutex);    
    magic_setflags (cookie, flags);
    const char *ctype = magic_file (cookie, file);
    if (ctype) type = g_strdup(ctype);
    g_mutex_unlock (magic_mutex);    
    return type;
}

// see "man libmagic" for explantion of flags
// Since MAGIC_NO_CHECK_ENCODING is not in file 4.x, we take care
// of that here.
#ifndef MAGIC_MIME_TYPE
#define MAGIC_MIME_TYPE  0
#endif
#ifndef MAGIC_NO_CHECK_APPTYPE
#define MAGIC_NO_CHECK_APPTYPE  0
#endif
#ifndef MAGIC_NO_CHECK_ENCODING
#define MAGIC_NO_CHECK_ENCODING  0
#endif
#ifndef MAGIC_SYMLINK
#define MAGIC_SYMLINK  0
#endif
#ifndef MAGIC_NO_CHECK_COMPRESS
#define MAGIC_NO_CHECK_COMPRESS  0
#endif
#ifndef MAGIC_NO_CHECK_TAR
#define MAGIC_NO_CHECK_TAR  0
#endif
#ifndef MAGIC_PRESERVE_ATIME
#define  MAGIC_PRESERVE_ATIME 0
#endif

//#define DISABLE_MAGIC

G_MODULE_EXPORT 
void *
mime_magic (void *p) {
    const gchar *file = p;
    NOOP(stderr, "mime_magic(%s)...\n", 
	    file);
    // Does the user even have read permission?
    if (access(file, R_OK) < 0){
	const gchar *h_type =
	    _("No Read Permission");
        return (void *)g_strdup(h_type);
    }
    
    gint flags = MAGIC_MIME_TYPE | MAGIC_SYMLINK | MAGIC_PRESERVE_ATIME;
    gchar *mimemagic = lib_magic (file, flags);
    NOOP(stderr, "mime_magic(%s)...%s\n", file, mimemagic);
    gchar *old_type = mimemagic; 
    mimemagic = rfm_natural(RFM_MODULE_DIR, "mime", mimemagic, "mime_get_alias_type");
    g_free(old_type);
    return (void *)mimemagic;
}

G_MODULE_EXPORT 
void *
mime_encoding (void *p) {
    const gchar *file = p;
    NOOP(stderr, "mime_encoding(%s)...\n", file);
    // Does the user even have read permission?
    if (access(file, R_OK) < 0){
	const gchar *h_type =
	    _("No Read Permission");
        return (void *)g_strdup(h_type);
    }
    //int flags = MAGIC_MIME_ENCODING;
    int flags = MAGIC_MIME_ENCODING | MAGIC_PRESERVE_ATIME | MAGIC_SYMLINK;
    gchar *encoding = lib_magic (file, flags);
    if (encoding) {
	NOOP(stderr, "%s --> %s\n", file, encoding);
	return (void *)encoding;
    }
    return NULL;
}

G_MODULE_EXPORT 
void *
mime_file (void *p) {
    const gchar *file = p;
    NOOP(stderr, "mime_file(%s)...\n", file);
    gint flags =  MAGIC_PRESERVE_ATIME;
    gchar *f = lib_magic (file, flags);
    NOOP(stderr, "mime_file(%s)...%s\n", file, f);
    if (!f) {
	return NULL;
    }
    if (rfm_g_file_test(file, G_FILE_TEST_IS_SYMLINK)){
	flags |= MAGIC_SYMLINK;
	gchar *ff = f;
	f = lib_magic (file, flags);
	gchar *gf = g_strconcat(ff, "\n", f, NULL);
	g_free(f);
	g_free(ff);
	return gf;

    }
    return (void *)f;
}

#endif
G_MODULE_EXPORT
const gchar *
g_module_check_init (GModule * module) {
    NOOP("***************g_module_check_init\n");
#ifdef HAVE_LIBMAGIC
    if (g_once_init_enter(&initialized)){
        rfm_mutex_init(magic_mutex);
        cookie = magic_open (MAGIC_NONE);
        magic_load (cookie, NULL);
	g_once_init_leave(&initialized, 1);
    }
#endif
    return NULL;
}

G_MODULE_EXPORT
void
g_module_unload (GModule * module) {
#ifdef HAVE_LIBMAGIC
    initialized=0;
    rfm_mutex_free(magic_mutex);
#endif
}
