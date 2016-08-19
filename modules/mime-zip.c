//
//
/*
 * Copyright (C) 2002-2012 Edscott Wilson Garcia
 * EMail: edscott@users.sf.net
 *
 * This file include modifications of GPL code provided by
 *  Dov Grobgeld <dov.grobgeld@gmail.com>
 *  Tadej Borovsak tadeboro@gmail.com
 * see below for details.
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

// This is set in a separate module, so that broken system zip libraries
// (as in ubuntu 11.10) will not hamper loading of mime module.

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <zip.h>

#include "rodent.h"
/* this should be first 2 lines after headers: */
G_MODULE_EXPORT LIBRFM_MODULE 

G_MODULE_EXPORT const gchar *
g_module_check_init (GModule * module) {
    return NULL;
}


G_MODULE_EXPORT void *
module_active (void){
    return GINT_TO_POINTER(1);
}

static
GdkPixbuf *
zip_preview(struct zip *Z, const gchar *name){
    DBG("creating zip preview for %s\n",name);
    GdkPixbuf *pixbuf = NULL;
    struct zip_stat sb;
    if(zip_stat (Z, name, 0, &sb) == 0) {
	void *ptr = malloc(sb.size);
	if (!ptr) g_error("malloc: %s", strerror(errno));
	struct zip_file *ZF = zip_fopen (Z, name, 0);
	if(ZF) {
	    zip_fread (ZF, ptr, sb.size);
	    zip_fclose (ZF);
	    gchar *base = g_path_get_basename(name);
	    gchar *fname = g_strdup_printf("%s/%d-%s.png", 
		    g_get_tmp_dir(), getuid(), base);
	    g_free(base);
	    gint fd=creat(fname, S_IRUSR | S_IWUSR);
            if (fd >= 0){
                if (write(fd, ptr, sb.size) < 0){
                    DBG("could not write to %s\n", fname);
                }
                close(fd);
            }
	    if (rfm_g_file_test(fname, G_FILE_TEST_EXISTS)){
	      pixbuf = rfm_pixbuf_new_from_file (fname, -1, -1);
	    }    
	    if (g_file_test(fname, G_FILE_TEST_EXISTS)) unlink(fname);
	    g_free(fname);
	} else {
	    DBG("could not zip_fopen Thumbnails/thumbnail.png\n");
	}

	g_free(ptr);
    }
    return pixbuf;

}
// This is for openoffice documents
G_MODULE_EXPORT
GdkPixbuf *
get_zip_preview (const gchar *path){
    NOOP("get_zip_preview %s\n", path);
    int errorv;
    struct zip *Z = zip_open (path, 0, &errorv);
    if (Z == NULL) {
	DBG("could not zip_open %s\n", path);
	return NULL;
    }
    GdkPixbuf *pixbuf = zip_preview(Z, "Thumbnails/thumbnail.png");
    zip_close(Z);
    return pixbuf;
        
}

// This is for any zip file...

G_MODULE_EXPORT
GdkPixbuf *
get_zip_image (const gchar *path){
    GdkPixbuf *pixbuf = NULL;
    static GMutex *zip_mutex=NULL;
    if (!zip_mutex) rfm_mutex_init(zip_mutex);

    TRACE("get_zip_image\n");
    int errorv;
    g_mutex_lock(zip_mutex);
    struct zip *Z = zip_open (path, 0, &errorv);
    if (Z == NULL) {
	DBG("could not zip_open %s\n", path);
	return NULL;
    }
    int index=0;
    const char *name;
    while ( (name=zip_get_name(Z, index, ZIP_FL_UNCHANGED)) != NULL) {
	record_entry_t en_v;
	memset(&en_v, 0, sizeof(record_entry_t));
	en_v.path = (gchar *)name;
        TRACE("zip content: %s\n", name);
	if (rfm_entry_is_image(&en_v)){
	    // image determination is done on mimetype.
	    TRACE("zip name to preview = %s\n", name);
	    pixbuf = zip_preview(Z, name);
	    g_free(en_v.mimetype);
	    break;
	}
	index++;
    }
    zip_close(Z);
    g_mutex_unlock(zip_mutex);

    if (pixbuf) g_object_ref(pixbuf);

    return pixbuf;
        
}

G_MODULE_EXPORT
GdkPixbuf *
get_rar_image (const gchar *path){
    GdkPixbuf *pixbuf = NULL;
    gchar *unrar = g_find_program_in_path("unrar");
    if (!unrar) return NULL;

    gchar *command=g_strdup_printf("%s vb \"%s\"",unrar, path);
    NOOP(stderr, "get_rar_image: %s\n", command);
    FILE *pipe = popen (command, "r");
    g_free(command);
    gchar *extract=NULL;
    if(pipe) {
#define PAGE_LINE 256
        gchar line[PAGE_LINE];
        line[PAGE_LINE - 1] = 0;
        while(fgets (line, PAGE_LINE - 1, pipe) && !feof (pipe)) {
            if(strstr (line, ".jpg") || strstr(line, ".JPG")){
		NOOP(stderr, "rar=%s\n", line);
		gchar **a =g_strsplit(line, "\n", -1);
		g_strstrip(a[0]);
		extract = g_strdup(a[0]);
		NOOP(stderr, "unrar e \"%s\" \"%s\"\n", unrar, path, extract);
		g_strfreev(a);
		break;
		
	    }
        }
        pclose (pipe);
    } 

    if (extract){
	if (chdir(g_get_tmp_dir()) < 0){
	    DBG("This is borked! cannot chdir(%s)\n", g_get_tmp_dir());
	    g_free(extract);
	    g_free(unrar);
	    return NULL;
	}
	gchar *argv[]={unrar, "e", (gchar *)path, extract, NULL};
	pid_t child=fork();
	int status;
	if (!child){
	    execv(argv[0], argv);
	    _exit(123);
	}
	    
	if (waitpid(child, &status, 0) < 0){
	    g_free(extract);
	    g_free(unrar);
	    return NULL;
	}
	gchar *basename=g_path_get_basename(extract);
	g_free(extract);
	gchar *extracted=g_strdup_printf("%s/%s", g_get_tmp_dir(), basename);
	g_free(basename);
	if (rfm_g_file_test(extracted, G_FILE_TEST_EXISTS)){
	      pixbuf = rfm_pixbuf_new_from_file (extracted, -1, -1);
	}    
	if (g_file_test(extracted, G_FILE_TEST_EXISTS)) unlink(extracted);
	g_free(extracted);
    }
    g_free(unrar);
    return pixbuf;
        
}
///////////////////////////////////////////////////////////////////////
