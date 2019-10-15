
/*
 * Copyright (C) 2002-2012 Edscott Wilson Garcia
 * EMail: edscott@users.sf.net
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
#include "rfm_modules.h"

static GSList *bookmarks = NULL;
enum transport_e {
    LOCAL_TRANSPORT,
    NFS_TRANSPORT,
    CIFS_TRANSPORT,
    OBEX_TRANSPORT,
    SSH_TRANSPORT,
    FTP_TRANSPORT,
    EFS_TRANSPORT
};

static RFM_RW_LOCK_INIT(bookmarks_lock);
typedef struct bookmark_item_t {
    gchar *uri;
    gchar *path;
    gchar *hostname;
    enum transport_e transport_type;
}bookmark_item_t;

static
gchar *get_bookmarks_filename(void){
    return g_build_filename(BOOKMARKS_FILE, NULL);
}

static
bookmark_item_t *bookmark_item_new(void){
    bookmark_item_t *bookmark_item_p = (bookmark_item_t *)malloc(sizeof(bookmark_item_t));
    if (!bookmark_item_p) g_error("malloc: %s\n", strerror(errno));
    memset(bookmark_item_p, 0, sizeof(bookmark_item_t));
    return bookmark_item_p;
}

static
void bookmark_item_free( bookmark_item_t *bookmark_item_p){
    if (!bookmark_item_p) return;
    g_free(bookmark_item_p->uri);
    g_free(bookmark_item_p->path);
    g_free(bookmark_item_p->hostname);
    g_free(bookmark_item_p);
    return;
}


	    
static
gpointer
read_bookmark_file_f(gpointer data){ 
    NOOP(stderr,"now reading bookmark file\n"); 
    gchar *filename = get_bookmarks_filename();
    rfm_rw_lock_writer_lock(&bookmarks_lock);
    FILE *f=fopen(filename, "r");
    g_free(filename);
    if (!f) {
	NOOP("read_bookmark_file_f(): g_mutex_unlock\n");
    	rfm_rw_lock_writer_unlock(&bookmarks_lock);
	return NULL;
    }
    GSList *tmp=bookmarks;
    for (;tmp && tmp->data; tmp=tmp->next){
	bookmark_item_free((bookmark_item_t *)(tmp->data));
    }
    g_slist_free(bookmarks);
    bookmarks=NULL;

    gchar buffer[2048];

    while (fgets(buffer, 2047, f) && !feof(f)){
	if (strchr(buffer, '\n')) *strchr(buffer, '\n')=0;
	if (strlen(buffer)==0) continue;
	bookmark_item_t *bookmark_item_p = bookmark_item_new();
	bookmark_item_p->uri = g_strdup(buffer);
	GError *error = NULL;
	bookmark_item_p->path = g_filename_from_uri (bookmark_item_p->uri, &(bookmark_item_p->hostname), &error);
	if (error) {
	    bookmark_item_p->path = NULL;
	    DBG("rodent_bookmarks.c %s: %s\n", bookmark_item_p->uri, error->message);
	    g_error_free(error);
	    //continue;
	}
	bookmarks=g_slist_prepend(bookmarks, bookmark_item_p);
    }

    fclose(f);
    rfm_rw_lock_writer_unlock(&bookmarks_lock);
    return NULL;
}

static gpointer
save_bookmark_file_f(gpointer data){
    gchar *filename = get_bookmarks_filename();
    rfm_rw_lock_writer_lock(&bookmarks_lock);
    if (bookmarks==NULL || g_slist_length(bookmarks)==0){
	if (rfm_g_file_test(filename, G_FILE_TEST_EXISTS)){
	    if (unlink(filename) < 0)
		DBG("unlink(%s): %s\n", filename, strerror(errno));
		
	}
	rfm_rw_lock_writer_unlock(&bookmarks_lock);
	g_free(filename);
	return NULL;
    }
    FILE *f=fopen(filename, "w");
    g_free(filename);
    GSList *tmp=bookmarks;
    if (f) {
        for (;tmp && tmp->data; tmp=tmp->next){
            bookmark_item_t *bookmark_item_p = tmp->data;
            fprintf(f,"%s\n", bookmark_item_p->uri);
        }
        fclose(f);
    }
	
    rfm_rw_lock_writer_unlock(&bookmarks_lock);

    gint bookmark_serial;
    if (!getenv("RFM_BOOKMARK_SERIAL")||
	    strlen(getenv("RFM_BOOKMARK_SERIAL"))==0){
	bookmark_serial = 0;
    } else {
	errno=0;
	long li = strtol(getenv("RFM_BOOKMARK_SERIAL"), NULL, 10);
	if (errno==ERANGE) bookmark_serial=0;
	else bookmark_serial = li;
    }


    bookmark_serial++;
    gchar *g=g_strdup_printf("%d", bookmark_serial);
    if (rfm_rational (RFM_MODULE_DIR, "settings", (void *)"RFM_BOOKMARK_SERIAL", (void *)g, "mcs_set_var") == NULL){
        rfm_setenv ("RFM_BOOKMARK_SERIAL", g, TRUE);
        NOOP("cannot set RFM_BOOKMARK_SERIAL");
    }
    g_free(g);
    NOOP("save_bookmark_file_f(): g_mutex_unlock\n");
    return NULL;
}

static void
bookmarks_init(void){
    static gsize initialization_value = 0;
    if (g_once_init_enter (&initialization_value))
    {
	if (!bookmarks) read_bookmark_file_f(NULL);
	g_once_init_leave (&initialization_value, 1);
    }
}

static void
update_bookmark_icons (view_t * view_p, GSList **oldlist_p) {
    if (rfm_get_gtk_thread() == g_thread_self()) g_error("update_bookmark_icons() is a thread function.\n");
    bookmarks_init();
    GSList *t;

    // Construct a list of items to expose with altered
    // bookmarks emblem
    //
    // Remove items which are unaltered from the refresh list.
    rfm_rw_lock_reader_lock(&bookmarks_lock);
    GSList *tm=bookmarks;
    for (; tm && tm->data; tm=tm->next){
	bookmark_item_t *bookmark_item_p = tm->data;
	if (bookmark_item_p->path == NULL) continue;
        gboolean found=FALSE;
	if (!bookmark_item_p->path) continue;
	for (t=*oldlist_p; t && t->data; t=t->next){
	    if (strcmp((gchar *)t->data, bookmark_item_p->path)==0){
	    // Remove items which are unaltered from the refresh list.
	    void *p=t->data;
	    *oldlist_p = g_slist_remove(*oldlist_p, t->data);
	    g_free(p);
	    found=TRUE;
	    break;
	}
	// Add new items to the refresh list.
	if (!found) *oldlist_p = g_slist_prepend(*oldlist_p, g_strdup(bookmark_item_p->path));
      }
    }
    rfm_rw_lock_reader_unlock(&bookmarks_lock);
    
    // find and update records
    if(!rfm_population_read_lock (view_p, "update_bookmark_icons")) goto done;
    GSList *expose_list = NULL;
    population_t **tmp = view_p->population_pp;
    for(;tmp && *tmp; tmp++) {
        population_t *population_p = *tmp;
        if(population_p->en && population_p->en->path) {
	    for (t=*oldlist_p; t && t->data; t=t->next){
		if (strcmp( population_p->en->path, (gchar *)t->data)==0){
		    GdkRectangle rect;
		    if (rfm_get_population_icon_rect(view_p, population_p, &rect)){
			GdkRectangle *rect_p = (GdkRectangle *)malloc(sizeof(GdkRectangle));
			if (!rect_p) g_error("malloc: %s\n", strerror(errno));
			memset(rect_p, 0, sizeof(GdkRectangle));
			memcpy(rect_p, &rect, sizeof(GdkRectangle));
			expose_list = g_slist_prepend(expose_list, rect_p);
		    }
		    break;
		}
	    }
	}
    }
    rfm_population_read_unlock (view_p, "update_bookmark_icons");
    for (t=expose_list; t && t->data; t=t->next){
	GdkRectangle *rect_p = t->data;
	rfm_expose_rect (view_p, rect_p);
	g_free(rect_p);
    }
    g_slist_free(expose_list);
done:
    //
    // free oldlist
    for (t=*oldlist_p; t && t->data; t=t->next){
	g_free(t->data);
    }
    g_slist_free(*oldlist_p);
    return;
}

void *
rodent_bookmark_monitor(view_t *view_p, void *data){
    //return NULL;
    if(!getenv ("RFM_BOOKMARK_SERIAL") ||
            !strlen (getenv ("RFM_BOOKMARK_SERIAL"))) return NULL;
    if (rfm_get_gtk_thread() == g_thread_self()) 
        g_error("rodent_bookmark_monitor() is a thread function.\n");
    bookmarks_init();

    NOOP("got bookmark serial=%s\n",getenv ("RFM_BOOKMARK_SERIAL"));
    errno = 0;
    struct stat st;
    static time_t mtime=0;

    gchar *filename = get_bookmarks_filename();
    long value = strtol (getenv ("RFM_BOOKMARK_SERIAL"), NULL, 0);
    if(errno != 0 || value != view_p->flags.bookmark_serial 
            ||(stat(filename, &st)==0 && st.st_mtime != mtime)) {
        mtime = st.st_mtime;
        TRACE("Bookmark serial= %d --> %ld\n",
                view_p->flags.bookmark_serial, value);
        view_p->flags.bookmark_serial = value;
        
        // Here we create a new list of bookmark items to update
        // icons which may be displayed with the bookmark emblem
        GSList *oldlist=NULL;
        rfm_rw_lock_writer_lock(&bookmarks_lock);
        GSList *tmp=bookmarks;
        for (; tmp && tmp->data; tmp=tmp->next){
            bookmark_item_t *bookmark_item_p = tmp->data;
            if (bookmark_item_p->path == NULL) continue;
            oldlist = g_slist_prepend(oldlist, g_strdup(bookmark_item_p->path));
        }
        rfm_rw_lock_writer_unlock(&bookmarks_lock);
        
        read_bookmark_file_f(NULL);
        update_bookmark_icons (view_p, &oldlist);
    }
    g_free(filename);
    return NULL;
}

gboolean
rodent_path_has_bookmark(const gchar *path){
    rfm_global_t *rfm_global_p = rfm_global();
    g_mutex_lock(rfm_global_p->status_mutex);
    gint status = rfm_global_p->status;
    g_mutex_unlock(rfm_global_p->status_mutex);
    if(status == STATUS_EXIT) return FALSE;
    if (!path || !strlen(path)) {
	NOOP("rodent_path_has_bookmark() path is NULL or strlen==0");
	return FALSE;
    }
    bookmarks_init();
    

    gboolean retval = FALSE;
    rfm_rw_lock_reader_lock(&bookmarks_lock);
    GSList *tmp = bookmarks;
    for (;rfm_global_p->status != STATUS_EXIT && tmp && tmp->data;
	    tmp=tmp->next){
	bookmark_item_t *bookmark_item_p = tmp->data;
	if (!bookmark_item_p->path) continue;
	if (strcmp(bookmark_item_p->path, path)==0){
	    retval = TRUE;
	    break;
	}
    }
    rfm_rw_lock_reader_unlock(&bookmarks_lock);
    return retval;
}

GdkPixbuf *
rodent_get_bookmark_emblem(gint size){
    GdkPixbuf *bookmark_pixbuf = 
	rfm_get_pixbuf ("xffm/emblem_bookmark", size/3);
    // Bookmark_pixbuf should never fail, nonetheless...
    if (!bookmark_pixbuf) bookmark_pixbuf = 
	rfm_get_pixbuf ("xffm/emote_cool", size/3);
    return bookmark_pixbuf;
}

gboolean
rodent_path_is_bookmarkable(const gchar *path){
    if (!path || !strlen(path)) {
	DBG("rodent_path_is_bookmarkable() path is NULL or strlen==0");
	return FALSE;
    }
    if (!g_path_is_absolute(path)) return FALSE;
    bookmarks_init();

    GSList *tmp = bookmarks;
    gboolean retval = TRUE;
    rfm_rw_lock_reader_lock(&bookmarks_lock);
    for (;tmp && tmp->data; tmp=tmp->next){
	bookmark_item_t *bookmark_item_p = tmp->data;
	if (!bookmark_item_p->path) continue;
	// Is it already bookmarked?
	if (strcmp(bookmark_item_p->path, path)==0){
	    retval = FALSE;
	    break;
	}
    }
    rfm_rw_lock_reader_unlock(&bookmarks_lock);
    return retval;
}

gboolean
rodent_bookmarks_add(const gchar *path){
    if (!path || !strlen(path)) {
	DBG("rodent_bookmarks_add() path is NULL or strlen==0");
	return FALSE;
    }
    bookmarks_init();
    NOOP("Bookmarking %s\n", path);
    bookmark_item_t *bookmark_item_p = bookmark_item_new();
    bookmark_item_p->path = g_strdup(path);
    GError *error = NULL;
    bookmark_item_p->uri = g_filename_to_uri(path, NULL, &error);
    if (error){
	DBG("rodent_bookmarks_add(%s): %s\n", path, error->message);
	g_error_free(error);
    }

    rfm_rw_lock_writer_lock(&bookmarks_lock);
    bookmarks=g_slist_prepend(bookmarks, bookmark_item_p);
    rfm_rw_lock_writer_unlock(&bookmarks_lock);
    save_bookmark_file_f(NULL);
    return TRUE;
}


gboolean
rodent_bookmarks_remove(const gchar *path){
    if (!path || !strlen(path)) {
	DBG("rodent_bookmarks_remove() path is NULL or strlen==0");
	return FALSE;
    }
    bookmarks_init();
     NOOP("removing Bookmark  %s\n", path);
     gboolean retval = FALSE;
     rfm_rw_lock_writer_lock(&bookmarks_lock);
     GSList *tmp=bookmarks;
     for (;tmp && tmp->data; tmp=tmp->next){
	 bookmark_item_t *bookmark_item_p = tmp->data;
	 if (bookmark_item_p->path == NULL) continue;
	 if (strcmp(bookmark_item_p->path, path)==0){
	     NOOP("gotcha %s\n", path);
	     bookmarks = g_slist_remove(bookmarks, bookmark_item_p);
	     bookmark_item_free(bookmark_item_p);
	     retval = TRUE;
	     break;
	 }
     }
     rfm_rw_lock_writer_unlock(&bookmarks_lock);
     if (retval) save_bookmark_file_f(NULL);
     return retval;
}

void
rodent_bookmark_set_menuitems(widgets_t *widgets_p, const gchar *id){
	 NOOP("rodent_bookmark_set_menuitems\n");
    bookmarks_init();
     gint level=0;
     rfm_rw_lock_reader_lock(&bookmarks_lock);
     GSList *local_list = NULL;
     GSList *tmp=bookmarks;
     for (;tmp && tmp->data; tmp=tmp->next){
	bookmark_item_t *bookmark_item_p = tmp->data;
	if (bookmark_item_p->path == NULL) continue;
	local_list = g_slist_prepend(local_list, bookmark_item_p);
     }
     rfm_rw_lock_reader_unlock(&bookmarks_lock);

     tmp=local_list;
     for (;level < DEEPEST_BOOK_MENU_LEVELS && tmp && tmp->data; tmp=tmp->next){
	 bookmark_item_t *bookmark_item_p = tmp->data;
	 NOOP("setting bookmark item %d\n", level);
         gchar *name = g_strdup_printf ("%s-%d", id, level);
	 GtkWidget *menuitem = rfm_get_widget (name);
	 if (!menuitem){
	     DBG("rodent_bookmark_set_menuitems(): widget %s not found", name);
	     g_free(name);
	     continue;
	 }
	 g_free(name);
	 if (!rfm_g_file_test(bookmark_item_p->path, G_FILE_TEST_IS_DIR)) {
	     TRACE("Invalid bookmark: %s (not a directory)\n", bookmark_item_p->path);
	     continue;
	 }
	 gchar *path=g_object_get_data(G_OBJECT(menuitem), "path");
	 g_object_set_data(G_OBJECT(menuitem), "path", g_strdup(bookmark_item_p->path));
	 g_free(path);

	 gchar *q = g_path_get_basename(bookmark_item_p->path);
         rfm_replace_menu_label(menuitem, q);
	 g_free (q);
	 
	 level++;
     }
     g_slist_free(local_list);

     for (;level < DEEPEST_BOOK_MENU_LEVELS; level++){
         gchar *name = g_strdup_printf ("%s-%d", id, level);
	 GtkWidget *menuitem = rfm_get_widget(name);
	 if (!menuitem){
	     DBG("rodent_bookmark_set_menuitems(): widget %s not found", name);
	     g_free(name);
	     continue;
	 }
	 g_free(name);
	 gchar *path=g_object_get_data(G_OBJECT(menuitem), "path");
	 g_object_set_data(G_OBJECT(menuitem), "path", NULL);
	 g_free(path);
	if(GTK_IS_WIDGET(menuitem))  gtk_widget_hide(menuitem);
     }
    return;
}


