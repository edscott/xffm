#ifdef COPYRIGHT_INFORMATION
#include "gplv3.h"
#endif
/*
 *
 *  Copyright (c) 2004-2011 Edscott Wilson Garcia <edscott@users.sf.net>
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;  */

#include <sys/types.h>
#include <sys/wait.h>

#include "rfm.h"


static regex_t supported;
static gboolean regex_compiled = FALSE;

G_MODULE_EXPORT gboolean
svg_supported (void) {
    static int support = 2;
    if(support < 2)
        return (gboolean) support;
    else
        support = 0;
    GSList *l;
    GSList *pix_formats = NULL;
    if(!pix_formats)
        pix_formats = gdk_pixbuf_get_formats ();
    for(l = pix_formats; l; l = l->next) {
        gchar **pix_mimetypes;
        int i;
        GdkPixbufFormat *fmt = l->data;
        pix_mimetypes = gdk_pixbuf_format_get_mime_types (fmt);
        for(i = 0; pix_mimetypes[i]; i++) {
            if(g_ascii_strcasecmp (pix_mimetypes[i], "image/svg") == 0) {
                support = 1;
                break;
            }
        }
        g_strfreev (pix_mimetypes);
    }
    g_slist_free (pix_formats);
    return (gboolean) support;
}



static int
open_theme (void) {
    // If there is a version mismatch, then the cache will not
    // work with this version of Rodent and must be regenerated
    // before the main thread can continue.
    //
    if (gtk_icon_theme_get_default ()) {
	// Main thread should wait for cache to be generated.
	// If cache is up to date, then this function call should
	// return ipso facto.
	//
	// We pass NULL here so that function will not
	// enter endless loop to monitor gtk icon theme. 
	// Data parameter is also the nice wait period, in
	// seconds.
	if (!compare_cache_info ()){
	    DBG("** creating icontheme cache...\n");
	    create_new_gtk_cache(NULL);
	} else {
	    DBG("** loading icontheme cache...\n");
	    load_path_cache();
	} 
	// fire up gtk theme monitor.
	NOOP ("** background Starting GTK theme monitor\n");
	THREAD_CREATE( create_new_gtk_cache, GINT_TO_POINTER(5), "create_new_gtk_cache");
    }
    return 1;
}


static 
gint
create_icontheme_path_cache (void) {
    // This is to create a cache for full paths for icons.
    if(open_theme () < 0) {
        DBG ("create_icontheme_path_cache(): cannot load theme\n");
        return -1;
    }

    // This is to create a g_hash which will associate mimetype with an icon
    // the mimetype/icon association hash is created by parsing the xml file.
    gchar *mimefile = mime_icon_get_local_xml_file ();
    NOOP ("create_icontheme_path_cache(): looking for: %s\n", (mimefile ? mimefile : "null"));

    /* test for default mimefile  */
    if(!mimefile || !g_file_test (mimefile, G_FILE_TEST_EXISTS)) {
       NOOP ("%s: not found. \n", (mimefile ? mimefile : "null"));
       g_free (mimefile);
       mimefile = mime_icon_get_global_xml_file ();
       NOOP ("Now looking for: %s\n", (mimefile ? mimefile : "null"));
    }

    if(!mimefile || !g_file_test (mimefile, G_FILE_TEST_EXISTS)) {
        DBG ("No system wide mime file found: %s\n", (mimefile ? mimefile : "null"));
        g_free (mimefile);
        mimefile = NULL;
	return FALSE;
    }

    gboolean result = create_icon_hash (mimefile);
    g_free (mimefile);
    if(!result) {
	DBG ("cannot create basename_hash from mimefile:%s!\n", (mimefile ? mimefile : "null"));
    }
    return result;
}


