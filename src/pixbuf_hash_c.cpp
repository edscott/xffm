#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <iostream>

#include "pixbuf_hash_c.hpp"
using namespace std;


#include "debug.h"
#include "intl.h"


pixbuf_hash_c::pixbuf_hash_c(data_c *data0){
    data_p=data0;
}

pixbuf_hash_c::~pixbuf_hash_c(void){

}

gchar *
pixbuf_hash_c::get_thumbnail_path (const gchar * file, gint size) {
    gchar *cache_dir;
    gchar *thumbnail_path = NULL;
    GString *gs;
    gchar key[11];

    cache_dir = g_build_filename (XFFM_THUMBNAIL_DIR, NULL);
    if(g_mkdir_with_parents (cache_dir, 0700) < 0) {
        g_free (cache_dir);
        return NULL;
    }

    /* thumbnails are not subject to thumbnailization: */
    gchar *dirname = g_path_get_dirname (file);
    if(strncmp (cache_dir, dirname, strlen (cache_dir)) == 0) {
        NOOP ("thumbnails cannot be thumbnailed:%s\n", file);
        g_free (cache_dir);
        g_free (dirname);
        return NULL;
    }

    gs = g_string_new (dirname);
    sprintf (key, "%10u", g_string_hash (gs));
    g_strstrip (key);
    g_string_free (gs, TRUE);
    g_free (dirname);

    gchar *thumbnail_dir = g_build_filename (cache_dir, key, NULL);
    if(g_mkdir_with_parents (thumbnail_dir, 0700) < 0) {
        g_free (thumbnail_dir);
        return NULL;
    }

    gchar *filename = g_path_get_basename (file);

    gs = g_string_new (file);
    sprintf (key, "%10u", g_string_hash (gs));
    g_strstrip (key);
    g_string_free (gs, TRUE);
    g_free (filename);

    filename = g_strdup_printf ("%s-%d.png", key, size);
    thumbnail_path = g_build_filename (thumbnail_dir, filename, NULL);
    g_free (filename);
    g_free (cache_dir);
    g_free (thumbnail_dir);
    NOOP ("thread: %s ->thumbnail_path=%s\n", file, thumbnail_path);

    return thumbnail_path;
}

void
pixbuf_hash_c::zap_thumbnail_file(const gchar *file, gint size){
    //Eliminate from thumbnail cache:
    gchar *thumbnail_path = get_thumbnail_path (file, size);
    if (g_file_test(thumbnail_path, G_FILE_TEST_EXISTS)) {
	if (g_file_test(thumbnail_path, G_FILE_TEST_EXISTS) && unlink(thumbnail_path) < 0) {
	    DBG("Cannot unlink thumbnail file: %s (%s)\n",
		thumbnail_path, strerror(errno));
	}
    }
    // Remove from hash table as well...
    rm_from_pixbuf_hash (thumbnail_path, size);
    g_free (thumbnail_path);
}


static void *
put_in_pixbuf_hash_f(void *data){
    void **arg = (void **)data;
    gchar *hash_key = (gchar *)arg[0];
    pixbuf_t *pixbuf_p = (pixbuf_t *)arg[1];
    GHashTable *pixbuf_hash = (GHashTable *)arg[2];
    if (!pixbuf_p) return  NULL;
    if (!pixbuf_p->pixbuf || !GDK_IS_PIXBUF(pixbuf_p->pixbuf)){
	DBG("put_in_pixbuf_hash: refuse to put !G_IS_OBJECT (pixbuf) into hash\n");
	return NULL;
    }
    g_object_ref(pixbuf_p->pixbuf);

    TRACE("replacing in hashtable: %s (%s)\n", pixbuf_p->path, hash_key);
    if (!pixbuf_hash) DBG("put_in_pixbuf_hash_f: hash is null!\n");
    g_hash_table_replace (pixbuf_hash, hash_key, pixbuf_p);
    return GINT_TO_POINTER(1);
}

void 
pixbuf_hash_c::put_in_pixbuf_hash(const gchar *path, gint size, const GdkPixbuf *pixbuf){
    if (!path || !pixbuf || !GDK_IS_PIXBUF(pixbuf)) {
	DBG("put_in_pixbuf_hash() %s is not a pixbuf\n", path);
	return;
    }
    TRACE("rfm_put_in_pixbuf_hash(%s, %d)\n", path, size);
    pixbuf_t *pixbuf_p = (pixbuf_t *) calloc (1, sizeof (pixbuf_t));
    if (!pixbuf_p) g_error("calloc: %s\n", strerror(errno));
    pixbuf_p->path = g_strdup (path);
    pixbuf_p->size = size;
    pixbuf_p->pixbuf = (GdkPixbuf *)pixbuf;

    if(g_path_is_absolute (path) && g_file_test(path, G_FILE_TEST_EXISTS)) {
        struct stat st;
        if (stat (path, &st)==0){
            pixbuf_p->mtime = st.st_mtime;
            pixbuf_p->st_size = st.st_size;
            pixbuf_p->st_ino = st.st_ino;
        } else DBG("cannot stat %s\n", path);
    } 
    // Replace or insert item in pixbuf hash
    gchar *hash_key = get_hash_key (pixbuf_p->path, pixbuf_p->size);
    void *arg[]={(void *)hash_key, (void *)pixbuf_p, (void *)data_p->pixbuf_hash};
    void *result = context_function(put_in_pixbuf_hash_f, (void *)arg);
    if (!result){
        g_free(hash_key);
	data_p->free_pixbuf_tt(pixbuf_p);
    }
    // hash_key is inserted into hash and should not be freed.
    return ;
}

// This is to remove thumbnails from the hash, basically.
// Thumbnails are always absolute paths.

static void *
rm_from_pixbuf_hash_f(void *data){
    void **arg = (void **)data;
    gchar *hash_key = (gchar *)arg[0];
    GHashTable *pixbuf_hash = (GHashTable *)arg[1];
    if (!hash_key) return  NULL;
    if (!pixbuf_hash) DBG("rm_from_pixbuf_hash_f: hash is null!\n");
    void *d = g_hash_table_lookup(pixbuf_hash, hash_key);
    
    if (d) {
        TRACE("removing key %s from hashtable\n", hash_key);
        g_hash_table_remove(pixbuf_hash, hash_key);
    } else {
        TRACE("key %s not in hashtable\n", hash_key);
    }
    return NULL;
}

void
pixbuf_hash_c::rm_from_pixbuf_hash (const gchar *icon_name, gint size) {
    TRACE("rfm_rm_from_pixbuf_hash()\n");
    if (!icon_name) return ;
    gchar *hash_key = get_hash_key (icon_name, size);
    void *arg[]={(void *)hash_key, (void *)data_p->pixbuf_hash};

    context_function(rm_from_pixbuf_hash_f, (void *)arg);
    g_free(hash_key);
    TRACE("rfm_rm_from_pixbuf_hash() done\n");
    return;
}



static void *
find_in_pixbuf_hash_f(void *data){
    void **arg = (void **)data;
    const gchar *hash_key = (const gchar *)arg[0];
    const gchar *icon_name = (const gchar *)arg[1];
    gint size = GPOINTER_TO_INT(arg[2]);
    GHashTable *pixbuf_hash = (GHashTable *)arg[3];
    pixbuf_hash_c *pixbuf_hash_p= (pixbuf_hash_c *)arg[4];


    if (!pixbuf_hash) DBG("find_in_pixbuf_hash_f: hash is null!\n");
    pixbuf_t *pixbuf_p = (pixbuf_t *)g_hash_table_lookup (pixbuf_hash, hash_key);

    if(!pixbuf_p || !GDK_IS_PIXBUF(pixbuf_p->pixbuf)) {
	return NULL;
    }
    if (g_path_is_absolute (icon_name)) {
	// Check for out of date source image files
	struct stat st;
	stat (icon_name, &st);
	if(pixbuf_p->mtime != st.st_mtime || 
		pixbuf_p->st_size != st.st_size||
		pixbuf_p->st_ino != st.st_ino)
	{
	    // Obsolete item must be replaced in pixbuf hash
	    // and eliminated from thumnail cache.
	    pixbuf_hash_p->zap_thumbnail_file(icon_name, size);
	    // Eliminate from pixbuf hash:
	    // this will be done when pixbuf is replaced...
	    //g_mutex_unlock (pixbuf_hash_mutex);
	    return NULL;
	}
    }
    return pixbuf_p->pixbuf;
}

GdkPixbuf *
pixbuf_hash_c::lookup_icon(const gchar *icon_name, gint size){
    GdkPixbuf *pixbuf = NULL;
    TRACE("find in pixbuf hash: %s(%d)\n", icon_name, size);
    gchar *hash_key = get_hash_key (icon_name, size);
    void *arg[]={(void *)hash_key, (void *) icon_name, GINT_TO_POINTER(size), (void *)data_p->pixbuf_hash, (void *)this};
    pixbuf = (GdkPixbuf *)context_function(find_in_pixbuf_hash_f, (void *)arg);
    g_free(hash_key);
   return pixbuf;
}

// pixbuf_hash_c::find_in_pixbuf_hash()
// On successful call, NO additional reference to g_object is added
// Reference to object belongs to hash table and is released
// on hash table destruction, only (each view has its own
// hash table).
GdkPixbuf *
pixbuf_hash_c::find_in_pixbuf_hash(const gchar *icon_name, gint size){
    if (!icon_name) return NULL;

   
    GdkPixbuf *pixbuf = lookup_icon(icon_name, size);
    if (pixbuf) return pixbuf;

    return NULL;
}


gchar *
pixbuf_hash_c::get_hash_key (const gchar * key, gint size) {
    gchar *hash_key = NULL;
    GString *gs = g_string_new (key);
    if (size <=0) {
	hash_key = g_strdup_printf ("%010u", g_string_hash (gs));
    } else {
	gint usize = 999;
	if (size <= 999) usize = size;
	hash_key = g_strdup_printf ("%010u-%d", g_string_hash (gs), usize);
    }
    g_string_free (gs, TRUE);
    NOOP("%s: hashkey=%s\n", key, hash_key);
    return hash_key;
}


///////////////////////////////////////////////////////////////////////////////


