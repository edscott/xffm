/*
 *
 * (C) Edscott Wilson Garcia 2001-2011 edscott@users.sf.net.
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
 * along with this program; 
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "rodent.h"

/* this should be first 2 lines after headers: */
G_MODULE_EXPORT LIBRFM_MODULE

typedef struct cache_info_t {
    gchar version[64];
    long long basedir_sum;
    gchar supported_regex[256];
} cache_info_t;

#include "icon-module.h"

gchar *mime_icon_get_local_xml_file (void);
gchar *mime_icon_get_global_xml_file (void);

static GHashTable *basename_hash = NULL;
RFM_RW_LOCK_INIT(basename_lock);


#include "icon-module-mime.i"
#include "icon-module-hash.i"
#include "icon-module-theme.i"
/******************************************************************/

G_MODULE_EXPORT 
void *
module_active (void){
    return GINT_TO_POINTER(1);
}
G_MODULE_EXPORT 
void *
g_module_check_init (GModule * module) {
    NOOP (stderr, "ICON: g_module_check_init...\n");
    rfm_rw_lock_init(&basename_lock);
    const gchar *regex = get_supported_regex ();
    if(regcomp (&supported, regex, REG_EXTENDED | REG_ICASE | REG_NOSUB) == 0)
	regex_compiled = TRUE;
    else
	regex_compiled = FALSE;
    
    create_icontheme_path_cache ();
    return NULL;
}
G_MODULE_EXPORT void 
g_module_unload(GModule * module){
    if(basename_hash) g_hash_table_destroy(basename_hash);
    rfm_rw_lock_clear(&basename_lock);
    regfree(&supported);
}

G_MODULE_EXPORT 
gchar *
mime_icon_get_local_xml_file (void) {
    NOOP ("ICON: mime_icon_get_local_xml_file...\n");
    gchar *mimefile;
    mimefile = g_build_filename (ICON_MODULE_USER_XML, NULL);
    return mimefile;
}


G_MODULE_EXPORT 
gchar *
mime_icon_get_global_xml_file (void) {
    NOOP ("ICON: mime_icon_get_global_xml_file...\n");
    gchar *mimefile = g_build_filename(ICON_MODULE_XML, NULL);
    NOOP ("ICON: mime_icon_get_global_xml_file...3:%s\n", mimefile);
    return mimefile;
}

G_MODULE_EXPORT 
gchar *
mime_icon_get_filename_from_basename (const gchar * basename) {
    NOOP ("ICON: mime_icon_get_filename_from_basename...\n");
    gchar *file = get_pixmap_path_from_cache (basename);
    return (file);
}

G_MODULE_EXPORT 
gchar *
mime_icon_get_filename_from_id (const gchar * id) {
    if (!id) return NULL;

    // Get from gtk icontheme
    if (cache_hash){
        NOOP("looking in hash for \"%s\"\n",id);
        gchar *cpath = g_hash_table_lookup(cache_hash, id);
        NOOP("got %s\n", cpath);
        if (cpath) return g_strdup(cpath);
    } else {
       DBG("mime_icon_get_filename_from_id: no hash available...\n");
    }


    if (!strchr(id, '/')){
	// named icon...
	return mime_icon_get_filename_from_basename(id);
    }
    NOOP ("ICON: mime_icon_get_filename_from_id...\n");

    const gchar *basename=NULL;
    NOOP ("ICON: g_hash_table_lookup: %s\n", id);
    // Short circuit builtin stock gtk items, if using gtk icon theme.
    // Nah! problem with stock icon sizes from builtin pixbufs...
    //if (icontheme_hash && strncmp(id, "xffm/stock_", strlen("xffm/stock_"))==0) {
	//return NULL;
    //}
    // Look for basename in hashtable...
    gchar *hash_key=rfm_get_hash_key(id, 0);
    rfm_rw_lock_reader_lock(&basename_lock);
    if (basename_hash){
        NOOP(stderr, "0x%x: lookup 1 basename_hash\n", 
		GPOINTER_TO_INT(g_thread_self()));
	basename = 
	    ((const gchar *) g_hash_table_lookup (basename_hash, hash_key));
    }
    g_free(hash_key);
    NOOP ("ICON: g_hash_table_lookup: %s --> found:  %s\n", id, basename);
    
    if(!basename && basename_hash) {
        if(!strchr (id, '/')) return NULL;
        gchar *g = g_strdup (id);
        *strchr (g, '/') = 0;
        gchar *gg = g_strconcat (g, "/", "default", NULL);
	hash_key=rfm_get_hash_key(gg, 0);
        NOOP(stderr, "0x%x: lookup 2 basename_hash\n", 
		GPOINTER_TO_INT(g_thread_self()));
        basename = (const gchar *)g_hash_table_lookup (basename_hash, hash_key);
	g_free(hash_key);
        NOOP ("ICON: trying g_hash_table_lookup: %s --> found:  %s\n", gg, basename);
        g_free (g);
        g_free (gg);
    }
    rfm_rw_lock_writer_unlock(&basename_lock);
    if(!basename) {
        NOOP (stderr, "cannot get base icon for id=%s\n", id);
        return NULL;
    }
    gchar *file=NULL;
    gboolean gtk_theme = (getenv("RFM_USE_GTK_ICON_THEME") &&
			    strlen(getenv("RFM_USE_GTK_ICON_THEME")));
    if (gtk_theme) {
	file = get_pixmap_path_from_cache (basename);
      if (!file) {
	// generic fallbacks
	if (strncmp(id, "audio/", strlen("audio/"))==0) {
	    file = get_pixmap_path_from_cache("audio-x-generic");
	}
	else if (strncmp(id, "application/x-font", strlen("application/x-font"))==0) {
	    file = get_pixmap_path_from_cache("font-x-generic");
	}
	else if (strncmp(id, "image/", strlen("image/"))==0) {
	    file = get_pixmap_path_from_cache("image-x-generic");
	}
	else if (strncmp(id, "video/", strlen("video/"))==0) {
	    file = get_pixmap_path_from_cache("video-x-generic");
	}
	else if (
		(strstr(id, "script")) ||
		(strstr(id, "perl")) ||
		(strstr(id, "python")) ||
		(strstr(id, "awk")) ||
		(strstr(id, "asp")) ||
		(strstr(id, "ruby")) ||
		(strstr(id, "x-csh")) ||
		(strstr(id, "x-ksh")) ||
		(strstr(id, "x-m4")) ||
		(strstr(id, "x-sh")) ||
		(strstr(id, "x-tsh")) 
		) {
	    file = get_pixmap_path_from_cache("text-x-script");
	}
	else if (strncmp(id, "text/", strlen("text/"))==0) {
	    file = get_pixmap_path_from_cache("text-x-generic");
	} 
	else if (strncmp(id, "application/", strlen("application/"))==0){
	    file = get_pixmap_path_from_cache("document");
	} 

      }
    } else {
        // not using gtk theme
        file = g_strdup_printf("%s/icons/rfm/scalable/stock/%s.svg",
                PACKAGE_DATA_DIR, basename);
        if (g_file_test(file, G_FILE_TEST_EXISTS)) return file;
        if (strstr(file, "execute") )DBG("not found: %s\n", file);
        g_free(file);
        file = g_strdup_printf("%s/icons/rfm/scalable/emblems/emblem-%s.svg",
                PACKAGE_DATA_DIR, basename);
        if (g_file_test(file, G_FILE_TEST_EXISTS)) return file;
        g_free(file);

        file = NULL;
    }
#if 10
    // Whether using gtk theme or not.
    if (!file && strstr(id, "xffm/emblem_")){
	gchar *icon = g_strdup(id+strlen("xffm/emblem_"));
	file = g_strdup_printf("%s/icons/rfm/scalable/emblems/emblem-%s.svg", 
		PACKAGE_DATA_DIR, icon);
	NOOP(stderr, "%s->%s\n", icon, file);
	g_free(icon);
    }
#endif

    if(!file || !g_file_test (file, G_FILE_TEST_EXISTS)) {
        NOOP (stderr, "mime_icon_get_filename_from_id(): %s->%s does not exist\n", id, file);
	if (file) {
	    g_free(file);
	    file = NULL;
	}
    }
    return (file);
}
	    // refinements

