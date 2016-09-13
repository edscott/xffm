#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include "pixbuf_hash_c.hpp"
#include <iostream>
using namespace std;


#include "debug.h"
#include "intl.h"

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

static void free_pixbuf_t(void *);

pixbuf_hash_c::pixbuf_hash_c(void){
    pixbuf_hash = 
	g_hash_table_new_full (g_str_hash, g_str_equal,g_free, free_pixbuf_t);
}

pixbuf_hash_c::~pixbuf_hash_c(void){
    g_hash_table_destroy (pixbuf_hash);

}


static void 
free_pixbuf_t(void *data){
    pixbuf_t *pixbuf_p = (pixbuf_t *) data;
    if (!pixbuf_p) return ;
    NOOP(stderr, "destroying pixbuf_t for %s size %d\n", pixbuf_p->path, pixbuf_p->size);
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
	DBG("rfm_put_in_pixbuf_hash() %s is not a pixbuf\n", path);
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
    void *arg[]={(void *)hash_key, (void *)pixbuf_p, (void *)pixbuf_hash};
    void *result = utility_p->context_function(put_in_pixbuf_hash_f, (void *)arg);
    if (!result){
        g_free(hash_key);
	free_pixbuf_t(pixbuf_p);
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
    void *arg[]={(void *)hash_key, (void *)pixbuf_hash};

    utility_p->context_function(rm_from_pixbuf_hash_f, (void *)arg);
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
    void *arg[]={(void *)hash_key, (void *) icon_name, GINT_TO_POINTER(size), (void *)pixbuf_hash, (void *)this};
    pixbuf = (GdkPixbuf *)utility_p->context_function(find_in_pixbuf_hash_f, (void *)arg);
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

    // FIXME: check, remake out of date thumbnails/previews as needed 
    
    GdkPixbuf *pixbuf = lookup_icon(icon_name, size);
    if (pixbuf) return pixbuf;

    // Not found, huh?
    // no pixbuf found. Create one and put in hashtable.
    pixbuf = absolute_path_icon(icon_name, size);

    if (!pixbuf){
        // check for composite icon definition or plain icon.
        if (is_composite_icon_name(icon_name)) pixbuf = composite_icon(icon_name, size);
        else pixbuf = get_theme_pixbuf(icon_name, size);
    }
   
    if (pixbuf){
        // put in iconhash...
        put_in_pixbuf_hash(icon_name, size, pixbuf);
        return pixbuf;
    } 
    return NULL;
    //return find_in_pixbuf_hash( "image-missing", size);
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


GdkPixbuf *
pixbuf_hash_c::composite_icon(const gchar *icon_name, gint size){
    gchar **tokens = g_strsplit(icon_name, "/", -1);
    if (!tokens) return NULL;
    gchar **p = tokens;
    // format: [icon_name/position/scale/alpha]
    GdkPixbuf *base_pixbuf =  get_theme_pixbuf(*p, size);

    gint i;
    for (p=tokens+1; p && *p; p += 4){
        for (i=1; i<4; i++) if (*(p+i) == NULL) {
            fprintf(stderr,
                    "*** pixbuf_icons_c::composite_icon(): incorrect composite specification: %s\n %s\n",
                    icon_name,
                    "*** (format: [icon_name/position/emblem_name/scale/alpha])");
            g_strfreev(tokens);
            return base_pixbuf;
        }
        gchar *position = p[0];
        gchar *emblem = p[1];
        gchar *scale = p[2];
        gchar *alpha = p[3];
        
	GdkPixbuf *tag = find_in_pixbuf_hash (emblem, size); 
	if (!tag) {
            fprintf(stderr, "pixbuf_hash_c::composite_icon(): Cannot get pixbuf for %s\n", emblem);
            g_strfreev(tokens);
            return base_pixbuf;
        }
        insert_pixbuf_tag (tag, base_pixbuf, position, scale, alpha);
    }
    g_strfreev(tokens);

    return base_pixbuf;
}


