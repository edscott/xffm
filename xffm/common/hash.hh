#ifndef XFHASH_HH
#define XFHASH_HH
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <iostream>

#define USER_CACHE_DIR      g_get_user_cache_dir(),"xffm+"
#define XFTHUMBNAIL_DIR 	USER_CACHE_DIR,"thumbnails"

//#include "pixbuf_hash_c.hpp"
using namespace std;

//static void free_pixbuf_t(void *);
typedef struct pixbuf_t {
    time_t mtime; // stat mtime info for thumbnails
    gint   size;  // pixbuf icon size
    off_t  st_size; // stat st_size for thumbnails
    ino_t  st_ino; // stat st_ino for thumbnails
    GdkPixbuf *pixbuf;
    union {
        gchar *mime_id;
        gchar *path;
    };
} pixbuf_t;

static GHashTable *pixbuf_hash;

namespace xf
{
template <class Type>
class Hash {
//private:
	
public:
    //static GHashTable *pixbuf_hash;
    static void createHash(void){
	pixbuf_hash = g_hash_table_new_full (g_str_hash, g_str_equal,g_free, free_pixbuf_t);
	TRACE( "Common pixbuf hash table address: %p.\n", (void *)pixbuf_hash);
    }


    static void 
    free_pixbuf_tt(void *data){
	free_pixbuf_t(data);
    }

    static gchar *
    get_thumbnail_path (const gchar * file, gint size) {
        if (!file) return NULL;
	gchar *cache_dir;
	gchar *thumbnail_path = NULL;
	GString *gs;
	gchar key[11];

	cache_dir = g_build_filename (XFTHUMBNAIL_DIR, NULL);
	if(g_mkdir_with_parents (cache_dir, 0700) < 0) {
	    g_free (cache_dir);
	    return NULL;
	}

	/* thumbnails are not subject to thumbnailization: */
	gchar *dirname = g_path_get_dirname (file);
	if(strncmp (cache_dir, dirname, strlen (cache_dir)) == 0) {
	    TRACE ("thumbnails cannot be thumbnailed:%s\n", file);
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
	TRACE ("thread: %s ->thumbnail_path=%s\n", file, thumbnail_path);

	return thumbnail_path;
    }

    static void
    zap_thumbnail_file(const gchar *file, gint size){
	//Eliminate from thumbnail cache:
	gchar *thumbnail_path = get_thumbnail_path (file, size);
	if (g_file_test(thumbnail_path, G_FILE_TEST_EXISTS)) {
	    if (g_file_test(thumbnail_path, G_FILE_TEST_EXISTS) && unlink(thumbnail_path) < 0) {
		ERROR("Cannot unlink thumbnail file: %s (%s)\n",
		    thumbnail_path, strerror(errno));
	    }
	}
	// Remove from hash table as well...
	rm_from_pixbuf_hash (thumbnail_path, size);
	g_free (thumbnail_path);
    }

    static void 
    put_in_pixbuf_hash(const gchar *path, gint size, const GdkPixbuf *pixbuf){
	if (!path || !pixbuf || !GDK_IS_PIXBUF(pixbuf)) {
	    ERROR("put_in_pixbuf_hash() %s is not a pixbuf\n", path);
	    return;
	}
	TRACE("rfm_put_in_pixbuf_hash(%s, %d)\n", path, size);
	pixbuf_t *pixbuf_p = (pixbuf_t *) calloc (1, sizeof (pixbuf_t));
	if (!pixbuf_p) {
	    g_error("calloc: %s\n", strerror(errno));
	    return;
	}
	pixbuf_p->path = g_strdup (path);
	pixbuf_p->size = size;
	pixbuf_p->pixbuf = (GdkPixbuf *)pixbuf;
	g_object_ref(pixbuf_p->pixbuf);

	if(g_path_is_absolute (path) && g_file_test(path, G_FILE_TEST_EXISTS)) {
	    struct stat st;
	    errno=0;
	    if (stat (path, &st)==0){
		pixbuf_p->mtime = st.st_mtime;
		pixbuf_p->st_size = st.st_size;
		pixbuf_p->st_ino = st.st_ino;
	    } else {
		if (errno){
		    DBG("base.hh::baseExecCompletionList(): stat %s (%s)\n",
			path, strerror(errno));
		    errno=0;
		}
	    }
	} 
	// Replace or insert item in pixbuf hash
	gchar *hash_key = get_hash_key (pixbuf_p->path, pixbuf_p->size);
       
	g_hash_table_replace (pixbuf_hash, hash_key, pixbuf_p);
	// hash_key is now hash property and should not be freed.
	return ;
    }

    // This is to remove thumbnails from the hash, basically.
    // Thumbnails are always absolute paths.

    static void
    rm_from_pixbuf_hash (const gchar *icon_name, gint size) {
	TRACE("rfm_rm_from_pixbuf_hash()\n");
	if (!icon_name) return ;
	gchar *hash_key = get_hash_key (icon_name, size);

	TRACE( "rm_from_pixbuf_hash: %s\n", hash_key);
	if (!pixbuf_hash) fprintf(stderr, "pixbuf_hash!\n");

	void *d = g_hash_table_lookup(pixbuf_hash, hash_key);
	
	if (d) {
	    TRACE("removing key %s from hashtable\n", hash_key);
	    g_hash_table_remove(pixbuf_hash, hash_key);
	} else {
	    TRACE("key %s not in hashtable\n", hash_key);
	}
	g_free(hash_key);
	TRACE("rfm_rm_from_pixbuf_hash() done\n");
	return;
    }


    static GdkPixbuf *
    lookup_icon(const gchar *icon_name, gint size){
	if (!pixbuf_hash) {
	    TRACE("Hash::lookup_icon(): Creating new hashtable\n");
	    createHash();
	   // return NULL;
	}
	TRACE( "find in pixbuf hash(%p): %s(%d)\n",(void *)pixbuf_hash, icon_name, size);
	gchar *hash_key = get_hash_key (icon_name, size);

	pixbuf_t *pixbuf_p = (pixbuf_t *)g_hash_table_lookup (pixbuf_hash, hash_key);
	if(!pixbuf_p || !GDK_IS_PIXBUF(pixbuf_p->pixbuf)) {
	    return NULL;
	}
	if (g_path_is_absolute (icon_name)) {
	    // Check for out of date source image files
	    struct stat st;
	    errno = 0;
	    if (stat (icon_name, &st) ==0){
	    if(pixbuf_p->mtime != st.st_mtime || 
		    pixbuf_p->st_size != st.st_size||
		    pixbuf_p->st_ino != st.st_ino)
	    {
		// Obsolete item must be replaced in pixbuf hash
		// and eliminated from thumnail cache.
		zap_thumbnail_file(icon_name, size);
		// Eliminate from pixbuf hash:
		// this will be done when pixbuf is replaced...
		//g_mutex_unlock (pixbuf_hash_mutex);
		return NULL;
	    }
	    } else {
		DBG("hash.hh::lookup_icon(): stat %s (%s)\n",
		    icon_name, strerror(errno));
		errno=0;
	    }
	}
	g_free(hash_key);
	return pixbuf_p->pixbuf;
    }

    // find_in_pixbuf_hash()
    // On successful call, NO additional reference to g_object is added
    // Reference to object belongs to hash table and is released
    // on hash table destruction, only (each view has its own
    // hash table).
    static GdkPixbuf *
    find_in_pixbuf_hash(const gchar *icon_name, gint size){
	if (!icon_name) return NULL;

       
	GdkPixbuf *pixbuf = lookup_icon(icon_name, size);
	if (pixbuf) return pixbuf;

	return NULL;
    }


    static gchar *
    get_hash_key (const gchar * key, gint size) {
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
	TRACE("%s: hashkey=%s\n", key, hash_key);
	return hash_key;
    }

    static void 
    free_pixbuf_t(void *data){
	pixbuf_t *pixbuf_p = (pixbuf_t *) data;
	if (!pixbuf_p) return ;
	TRACE( "destroying pixbuf_t for %s size %d\n", pixbuf_p->path, pixbuf_p->size);
	if (pixbuf_p->pixbuf && !G_IS_OBJECT (pixbuf_p->pixbuf)) {
	    cerr << "This should not happen: pixbuf_p->mime_id, not a pixbuf:"
		<< pixbuf_p->mime_id << "\n";
	} else {
	    g_object_unref (pixbuf_p->pixbuf);
	}
	g_free(pixbuf_p->path);
	g_free(pixbuf_p);
	return;
    }
};
}

///////////////////////////////////////////////////////////////////////////////

#endif
