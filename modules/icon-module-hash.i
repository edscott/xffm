#ifdef COPYRIGHT_INFORMATION
#include "gplv3.h"
#endif
/*
 *
 *  Copyright (c) 2004-2014 Edscott Wilson Garcia <edscott@users.sf.net>
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


#ifndef ICON_MODULE_HASH_I
#define ICON_MODULE_HASH_I
static gchar *
get_cache_path (void) {
    gchar *cache_path = NULL;
    gchar *cache_dir;
    cache_dir = g_build_filename (USER_DBH_CACHE_DIR, NULL);
    if (!g_file_test(cache_dir, G_FILE_TEST_IS_DIR)){
	if (g_file_test(cache_dir, G_FILE_TEST_EXISTS)){
	    DBG("%s exists and is not a directory!\n", cache_dir);
	    g_free (cache_dir);
	    return NULL;
	}
	if (g_mkdir_with_parents(cache_dir, 0750) < 0){
	    DBG("g_mkdir_with_parents(%s): %s\n", cache_dir, strerror(errno));
	    g_free (cache_dir);
	    return NULL;
	}
    }

    if(!g_file_test(cache_dir, G_FILE_TEST_IS_DIR)) {
	g_error("!check_dir (%s)\n", cache_dir);
        g_free (cache_dir);
        return NULL;
    }

    cache_path = g_strdup_printf ("%s%cicon.list", cache_dir, G_DIR_SEPARATOR);
    g_free (cache_dir);
    NOOP ("ICON: using cache: %s\n", cache_path);
    return cache_path;
}


static GHashTable *cache_hash = NULL;
pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;

static gboolean
exit_condition(void){
    rfm_global_t *rfm_global_p = rfm_global();
    if (rfm_global_p){
        g_mutex_lock(rfm_global_p->status_mutex);
        gint status = rfm_global_p->status;
        g_mutex_unlock(rfm_global_p->status_mutex);
        if(status == STATUS_EXIT) return TRUE;
    }
    return FALSE;
}
    
static long long
recurse_basedir_sum (const gchar * dir) {
    long long sum = 0;
    const char *file;
    GDir *gdir;
    NOOP ("ICON: recurse_basedir_sum(%s)\n", dir);
    if((gdir = g_dir_open (dir, 0, NULL)) != NULL) {
        while((file = g_dir_read_name (gdir))) {
            gchar *path = g_build_filename (dir, file, NULL);
            if(g_file_test (path, G_FILE_TEST_IS_SYMLINK)) {
                g_free (path);
                continue;
            }
            if(g_file_test (path, G_FILE_TEST_IS_DIR)) {
                struct stat st;
                /* recurse */
                sum += recurse_basedir_sum (path);
                if(stat (path, &st) == 0) {
                    sum += st.st_mtime;
                    sum += st.st_dev;
                }
            }
            g_free (path);
        }
        g_dir_close (gdir);
    }
    return sum;
}


/* must get info for all directories within */
static off_t
get_basedir_sum (void) {
    gint build=0;
    off_t basedir_sum = 0;

    GtkIconTheme *icon_theme = gtk_icon_theme_get_default ();
    gchar **pathv=NULL;
    gtk_icon_theme_get_search_path (icon_theme, &pathv, NULL);
    gchar **p=pathv;
    for (;p && *p; p++) {
	const gchar *basedir = *p;
	struct stat st;
	if(stat (basedir, &st) == 0) {
	    NOOP ("ICON: stat ok\n");
	    gint sum = st.st_mtime + st.st_size + st.st_mode + st.st_nlink + st.st_uid + st.st_gid;
	    basedir_sum += sum;
	}
	basedir_sum += recurse_basedir_sum (basedir);
	NOOP ("ICON-DBH: basedir summing %s (%ld %ld)\n", basedir, (long)st.st_mtime, (long)st.st_dev);
	basedir_sum += build;
    }
    g_strfreev(pathv);
    return basedir_sum;
}

static const gchar *
get_supported_regex (void
    ) {
    GSList *pix_formats,
     *l;
    gchar **pix_extensions,
    **p;
    static gchar *reg = NULL,
        *r = NULL;
    /* check SVG format support */
    if((pix_formats = gdk_pixbuf_get_formats ()) != NULL) {
        for(l = pix_formats; l; l = l->next) {
            GdkPixbufFormat *fmt = l->data;
            pix_extensions = gdk_pixbuf_format_get_extensions (fmt);
            for(p = pix_extensions; *p; p++) {
                NOOP ("ICON: supported=%s\n", *p);
                if(reg) {
                    g_free (r);
                    r = reg;
                    reg = g_strconcat (r, "|", *p, NULL);
                } else
                    reg = g_strdup (*p);
            }
            g_strfreev (pix_extensions);
        }
        g_slist_free (pix_formats);
    }
    if(!reg)
        return "\\.(png|xpm)$)";
    g_free (r);
    r = g_strconcat ("\\.(", reg, ")$", NULL);
    g_free (reg);
    reg = NULL;

    NOOP ("ICON: regex=%s\n", r);
    return (const gchar *)r;
}

static void
save_cache_info (gchar *version) {
    FILE *info;

    gchar *cache_file = get_cache_path ();
    if (!cache_file) return;
    gchar *cache_info_path = g_strconcat (cache_file, ".info", NULL);
    g_free(cache_file);

    cache_info_t cache_info;
    DBG ("0x%x: saving %s\n", GPOINTER_TO_INT(g_thread_self()), cache_info_path);

    strncpy (cache_info.supported_regex, get_supported_regex (), 255);
    cache_info.supported_regex[255] = 0;
    cache_info.basedir_sum = get_basedir_sum ();


    strncpy (cache_info.version, version, 63);
    cache_info.version[63]=0;

    info = fopen (cache_info_path, "wb");
    NOOP(stderr, "..... writing %s: %s %s \n", cache_info_path, cache_info.version,version);
    if(info) {
        if(fwrite (&cache_info, sizeof (cache_info_t), 1, info) < 1) {
            DBG ("cannot write to %s\n", cache_info_path);
        }
        fclose (info);
    } else {
        DBG ("cannot write to %s\n", cache_info_path);
    }
    DBG ("icon-module-hash: wrote basedir sum is %lld\n", cache_info.basedir_sum);
    g_free (cache_info_path);
}

/* criteria for cache regeneration:
 * 1- does not exist
 * 2- one of the base_dirs has been modified
 * 3- supported regexp has changed.
 * 4- prefix has changed
 *
 * */
static gboolean
compare_cache_info (void) {
#if 10
    FILE *info;
    gchar *cache_info_path;
    cache_info_t cache_info;
    cache_info_t  disk_cache_info;
    gint records;

    gchar *cache_file = get_cache_path ();
    if (!cache_file) {
	DBG("no cache file string\n");
        return TRUE;
    }
    DBG( "0x%x: comparing cache info \n", GPOINTER_TO_INT(g_thread_self()) );

    if(!g_file_test (cache_file, G_FILE_TEST_EXISTS)) {
        DBG ("ICON: %s not exists\n", cache_file);
        g_free (cache_file);
        return FALSE;
    }

    cache_info_path = g_strconcat (cache_file, ".info", NULL);
    g_free (cache_file);
    if((info = fopen (cache_info_path, "rb")) == NULL) {
        NOOP (stderr, "ICON: %s not exists\n", cache_info_path);
        g_free (cache_info_path);
        return FALSE;
    }

    // disk_cache_info is a structure, not a string.
    // coverity[string_null_argument : FALSE]
    records = fread (&disk_cache_info, sizeof (cache_info_t), 1, info);
    NOOP(stderr, ".... compare cache info: reading %s .....%s\n", cache_info_path, disk_cache_info.version);
    g_free (cache_info_path);
    fclose (info);
    if(records < 1) {
        NOOP (stderr, "records < 1\n");
	return FALSE;
    }

    gchar *version=NULL;
    GtkSettings *settings = gtk_settings_get_default();
    g_object_get( G_OBJECT(settings), 
	    "gtk-icon-theme-name", &version,
	    NULL);    
    if (version){
        if(strlen(version)>64) version[63]=0;

        DBG ("icon-module-theme: read basedir cache sum is %lld\n", disk_cache_info.basedir_sum);

        if(strncmp (disk_cache_info.version, version, 64) != 0) {
            NOOP (stderr, "Icontheme version cahnge: %s --> %s\n", disk_cache_info.version, version);
            g_free(version);
            return FALSE;
        }
        g_free(version);
    }


    cache_info.basedir_sum = get_basedir_sum ();
    strncpy (cache_info.supported_regex, get_supported_regex (), 255);
    cache_info.supported_regex[255] = 0;
    NOOP ("ICON: cache sum is %lld\n", cache_info.basedir_sum);
    if(cache_info.basedir_sum != disk_cache_info.basedir_sum) {
        NOOP
            (stderr, "ICON: cache_info.basedir_sum(%lld) != disk_cache_info.basedir_sum(%lld)\n",
             cache_info.basedir_sum, disk_cache_info.basedir_sum);
        return FALSE;
    }
    // false positive. disk_cache_info.supported_regex is a regex string,
    // not an array.
    // coverity[array_null : FALSE]
    if(disk_cache_info.supported_regex && 
	    strlen(disk_cache_info.supported_regex) &&
	    strcmp (cache_info.supported_regex, disk_cache_info.supported_regex)) {
        NOOP (stderr, "ICON: cache_info.supported_regex != disk_cache_info.supported_regex\n");
        return FALSE;
    }
    return TRUE;
#endif
}


static gchar *
get_pixmap_path_from_cache (const gchar * key) {
    if (!key || !cache_hash) return NULL;
    NOOP ("get_pixmap_path_from_cache(GTK): icon lookup: %s\n", key);

    pthread_mutex_lock(&cache_mutex);
    const gchar *path = g_hash_table_lookup(cache_hash, key);
    pthread_mutex_unlock(&cache_mutex);
    if (path){
        if (g_file_test (path, G_FILE_TEST_EXISTS)) return g_strdup(path);
    }
    return NULL;
}



typedef struct icon_info_t {
    gchar *name;
    gchar *path;
}icon_info_t;

// Main thread gtk call:
static void *list_gtk_icons(void *data){
    TRACE("list_gtk_icons...\n");
    GSList *path_list = NULL;
    if (rfm_get_gtk_thread() != g_thread_self()){
	g_error("add_gtk_icons should be main thread run...\n");
    }
    GtkIconInfo *icon_info;
    GtkIconTheme *icon_theme = gtk_icon_theme_get_default ();
    GList *list=NULL;
    if (icon_theme) list = gtk_icon_theme_list_icons (icon_theme, NULL);
    GList *tmp=list;
    for (;tmp && tmp->data; tmp=tmp->next){
        const gchar *path;
	gchar *icon_name = tmp->data;
	// Get path for size 128 or 48.
	icon_info =
	    gtk_icon_theme_lookup_icon (icon_theme,
		    icon_name, 128, GTK_ICON_LOOKUP_GENERIC_FALLBACK);
	if (!icon_info){
	    icon_info =
		gtk_icon_theme_lookup_icon (icon_theme,
		    icon_name, 48, GTK_ICON_LOOKUP_GENERIC_FALLBACK);
	}
	if (!icon_info){
	    NOOP(stderr, "no theme icon for %s\n", icon_name);
	    continue;
	}

	path = gtk_icon_info_get_filename (icon_info);
	if (!path){
	    NOOP(stderr, "Path for %s is NULL\n", icon_name);
	    continue;
	}

        icon_info_t *icon_info_p = (icon_info_t *)malloc(sizeof(icon_info_t));
        if (!icon_info_p) g_error("malloc: %s\n", strerror(errno));
        memset(icon_info_p, 0, sizeof(icon_info_t));
        icon_info_p->path = g_strdup(path);
        icon_info_p->name = g_strdup(icon_name);
        path_list = g_slist_prepend(path_list, icon_info_p);

	// Free no longer useful gtk_icon_info.
	// Beware that const gchar *path belongs to gtk_icon_info
#if GTK_MAJOR_VERSION==3 && GTK_MINOR_VERSION>=8
	if (icon_info)g_object_unref(G_OBJECT(icon_info));
#else
	if (icon_info) gtk_icon_info_free (icon_info); 
#endif
	NOOP("icon: %s\n", (gchar *)tmp->data);
	g_free(tmp->data);
    }
    g_list_free(list);     
    return path_list;
}

static void
load_path_cache(void){
    //load from dump for quick retrieval
    pthread_mutex_lock(&cache_mutex);
    if (!cache_hash) cache_hash = g_hash_table_new_full(g_str_hash, g_str_equal,g_free, g_free);
    pthread_mutex_unlock(&cache_mutex);
    gchar *fname = get_cache_path();
    FILE *in=fopen(fname, "r");
    g_free(fname);
    if (in){
	gchar buffer[4096];
	memset(buffer, 0, 4096);
	while (fgets(buffer, 4095, in) && !feof(in)){
	    if (strchr(buffer, '\n')) *strchr(buffer, '\n') = 0;
	    if (!strchr(buffer, ':')) continue;
	    gchar **gg = g_strsplit(buffer, ":", -1);
	    pthread_mutex_lock(&cache_mutex);
	    g_hash_table_replace(cache_hash, g_strdup(gg[0]), g_strdup(gg[1]));
	    pthread_mutex_unlock(&cache_mutex);
	    g_strfreev(gg);
	}
        fclose(in);
    } else {
        g_warning("cannot open %s for read (%s)\n", fname, strerror(errno));
    }
}


G_MODULE_EXPORT
gboolean
create_cache(void){
    // Exit condition test
    if (exit_condition()) return 0;
    DBG ("thread ----> Generating GTK icon-module cache. This may take a bit...\n");
    GSList *path_list = (GSList *) rfm_context_function( list_gtk_icons, NULL);
    if (!path_list){
	DBG("** Could not create GTK icon module cache\n" );
	return 0;
    }
    // Exit condition test
    if (exit_condition()) return 0;

    // This add non themed icons.
    // add_theme_directories();
    DBG("g_hash_table_new_full...\n");
    pthread_mutex_lock(&cache_mutex);
    if (!cache_hash) cache_hash = g_hash_table_new_full(g_str_hash, g_str_equal,g_free, g_free);
    pthread_mutex_unlock(&cache_mutex);

    
    GSList *list = path_list;
    for (; list && list->data; list=list->next){
        icon_info_t *icon_info_p = list->data;
        pthread_mutex_lock(&cache_mutex);
        g_hash_table_replace(cache_hash, icon_info_p->name, icon_info_p->path);
        pthread_mutex_unlock(&cache_mutex);
    }
    DBG("cache_hash is now created....\n"); 

    //dump for quick retrieval and cleanup list
    gchar *fname = get_cache_path();
    FILE *out=fopen(fname, "w");
    if (out){
        DBG("dumping icon cache info: %s\n", fname);
        gchar *current_version=NULL;
        GtkSettings *settings = gtk_settings_get_default();
        g_object_get( G_OBJECT(settings), "gtk-icon-theme-name", &current_version, NULL);    
        save_cache_info (current_version);
        g_free(current_version);  //       
        
        list = path_list;
        for (;list && list->data; list=list->next){
            icon_info_t *icon_info_p = list->data;
            fprintf(out, "%s:%s\n", icon_info_p->name, icon_info_p->path);
            g_free(icon_info_p);
        }
        fclose(out);
    } else {
        g_warning("cannot open %s for write (%s)\n", fname, strerror(errno));
    }
    g_free(fname);
    g_slist_free(path_list);

       
    
    DBG ("thread -----> GTK cache complete:\n");

    // cleanout Composite icon cache here. 
    gchar *g = g_build_filename (ICON_ID_DBH_FILE, NULL);
    unlink(g);
    g_free(g);

    // Cleanout pixbuf hash here.
    rfm_replace_pixbuf_hash (); 
    
    return 1;
}

static void *
create_new_gtk_cache(void *data) {
    // data != NULL means we are in background thread.
    // data == NULL means the main thread is here, updating
    //         the cache before any icon is displayed.

    if (!data) {
	create_cache();
	return NULL;
    }

    static gchar *icon_theme_name;
    if (getenv("RFM_USE_GTK_ICON_THEME"))
	icon_theme_name = g_strdup(getenv("RFM_USE_GTK_ICON_THEME"));
    else icon_theme_name = g_strdup("");
    
    // If running by thread, give main thread a head start.
    gint wait_period=GPOINTER_TO_INT(data);
    if (wait_period > 5){
	DBG("wait_period > 5 is dumb.\n");
	wait_period=5;
    }
    DBG("0x%x waiting %d seconds before starting monitor of GTK icontheme\n", 
	    GPOINTER_TO_INT(g_thread_self()), wait_period);

    sleep(wait_period);

    // This will monitor changes in icontheme name
    do {
	if (exit_condition()) return FALSE;
	// Loop wait. Use rfm_threadwait() for stress tests...
	//rfm_threadwait();
	sleep(2);
	if (exit_condition()) return FALSE;
	const gchar *current_icon_theme = getenv("RFM_USE_GTK_ICON_THEME");
        if (!current_icon_theme) current_icon_theme="";
        NOOP("checking for icontheme changes...(\"%s\" == \"%s\")\n", current_icon_theme, icon_theme_name);

	if (strcmp(current_icon_theme, icon_theme_name)==0) continue;
	g_free(icon_theme_name);
	icon_theme_name = g_strdup(current_icon_theme);
        DBG("Now queueing icontheme cache creation to main thread\n");
        // This will queue the icon theme change to the main thread.
	create_cache();
        
    } while (1);
    NOOP(stderr, "--Ending gtk icon theme thread (will not monitor changes in theme).\n");
	    
    return NULL;
}
#endif
