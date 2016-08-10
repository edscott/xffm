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
    utility_p = new utility_c();
    pixbuf_hash = 
	g_hash_table_new_full (g_str_hash, g_str_equal,g_free, free_pixbuf_t);
    pixbuf_mutex = PTHREAD_MUTEX_INITIALIZER;
    self = g_thread_self();
}

pixbuf_hash_c::~pixbuf_hash_c(void){
    g_hash_table_destroy (pixbuf_hash);
    delete utility_p;

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

// hmmmm.... serialized....
//  only the main thread will access the pixbuf hash,


/////    exported /////
//


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
    g_object_ref(pixbuf_p->pixbuf);
    return pixbuf_p->pixbuf;
}

GdkPixbuf *
pixbuf_hash_c::find_in_pixbuf_hash(const gchar *icon_name, gint size){
    if (!icon_name) return NULL;
    // This will report out of date thumbnails/previews as not present
    // On successful call, returned pixbuf_p->pixbuf will have
    // an additional reference which must be freed after use...
    GdkPixbuf *pixbuf = NULL;
    TRACE("find in pixbuf hash: %s(%d)\n", icon_name, size);
    gchar *hash_key = get_hash_key (icon_name, size);
    void *arg[]={(void *)hash_key, (void *) icon_name, GINT_TO_POINTER(size), (void *)pixbuf_hash, (void *)this};
    pixbuf = (GdkPixbuf *)utility_p->context_function(find_in_pixbuf_hash_f, (void *)arg);
    g_free(hash_key);
    if (pixbuf) return pixbuf;

    // Not found, huh?
    if(g_path_is_absolute (icon_name)){
        if (! g_file_test (icon_name, G_FILE_TEST_EXISTS)) {
            return find_in_pixbuf_hash( "image-missing", size);
        }
        // FIXME: (width, height) would be better than (size, size)
        pixbuf = pixbuf_new_from_file(icon_name, size, size); // width,height.
        if (pixbuf) {
            put_in_pixbuf_hash(icon_name, size, pixbuf);
            return pixbuf;
        }
        return find_in_pixbuf_hash( "image-missing", size);
    }
    
    // no pixbuf found. Create one and put in hashtable.
    GtkIconTheme *icon_theme = gtk_icon_theme_get_default ();
    // if no filename found, search in xffm+ icons
    // also add path for application icons
    /* void gtk_icon_theme_add_resource_path (GtkIconTheme *icon_theme,
                                  const gchar *path);
                                  */
    if (icon_theme) {
        GError *error = NULL;
        GdkPixbuf *theme_pixbuf = gtk_icon_theme_load_icon (icon_theme,
                      icon_name,
                      size, 
                      GTK_ICON_LOOKUP_FORCE_SIZE,  // GtkIconLookupFlags flags,
                      &error);
        if (error) {
            fprintf(stderr, "pixbuf_hash_c::find_in_pixbuf_hash: error->message\n");
            g_error_free(error);
        } else if (theme_pixbuf) {
            // Release any reference to the icon theme.
            pixbuf = gdk_pixbuf_copy(theme_pixbuf);
            g_object_unref(theme_pixbuf);
        }
    }
    if (pixbuf){
        // put in iconhash...
        put_in_pixbuf_hash(icon_name, size, pixbuf);
    }
    return pixbuf;
#if 0
    if (file) {
	if (!g_path_is_absolute (file)){
	    DBG("incorrect type:file association (%s:%s)\n", key,file); 
	    g_free(file); 
	    return NULL;
	}
        gchar *hash_key = get_hash_key (file, size);
        void *arg[]={(void *)hash_key, (void *)file, GINT_TO_POINTER(size), (void *)pixbuf_hash};
	pixbuf = (GdkPixbuf *)utility_p->context_function(find_in_pixbuf_hash_f, (void *)arg);
        g_free(hash_key);
	g_free(file);
	return pixbuf;
    }
    // If no source file is associated, we are dealing with a ad hoc icon
    gchar *hash_key = get_hash_key (key, size);
    void *arg[]={(void *)hash_key, (void *)key, GINT_TO_POINTER(size), (void *)pixbuf_hash};
    pixbuf = (GdkPixbuf *)utility_p->context_function(find_in_pixbuf_hash_f, (void *)arg);
    g_free(hash_key);
    return pixbuf;
#endif

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


#if 0
static void *
replace_pixbuf_hash_f(void *data){
    //GHashTable *old_hash = pixbuf_hash;
    pixbuf_hash = data;
    // XXX leak. Pointers may be in use by other threads.
    //if (old_hash) g_hash_table_destroy(old_hash);
    return NULL;
}

void
rfm_replace_pixbuf_hash (void) {
    GHashTable *hash = init_pixbuf_hash();
    rfm_context_function(replace_pixbuf_hash_f, hash);
    return;
}
#endif

static void *
pixbuf_new_from_file_f(void *data){
    void **arg = (void **)data;
    gchar *path = (gchar *)arg[0];
    gint width = GPOINTER_TO_INT(arg[1]);
    gint height = GPOINTER_TO_INT(arg[2]);
    GError *error = NULL;
    GdkPixbuf *pixbuf = NULL;
    if (width < 0) {
	pixbuf = gdk_pixbuf_new_from_file (path, &error);
    } else {
	pixbuf = gdk_pixbuf_new_from_file_at_size (path, width, height, &error);
    }
    // hmmm... from the scale_simple line below, it seems that the above two
    //         functions will do a g_object_ref on the returned pixbuf...


    // Gdkpixbuf Bug workaround 
    // (necessary for GTK-2, still necessary in GTK-3.8)
    // xpm icons not resized. Need the extra scale_simple. 


    //if (pixbuf && width > 0 && gdk_pixbuf_get_width(pixbuf) != width){
    //if (pixbuf && strstr(path, ".xpm")){
    if (pixbuf && width > 0 && strstr(path, ".xpm")) {
	NOOP(stderr, "** resizing %s\n", path);
	GdkPixbuf *pix = gdk_pixbuf_scale_simple (pixbuf, width, height, GDK_INTERP_HYPER);
	g_object_unref(pixbuf);
	pixbuf = pix;

    }  
    
/*    if(error && !strstr(path, ".cache/rfm/thumbnails")) {
	    DBG ("pixbuf_from_file() %s:%s\n", error->message, path);
	    g_error_free (error);
    }*/
    return pixbuf;
}

GdkPixbuf *
pixbuf_hash_c::pixbuf_new_from_file (const gchar *path, gint width, gint height){
    if (!path) return NULL;
    if (!g_file_test(path, G_FILE_TEST_EXISTS)) return NULL;
    GdkPixbuf *pixbuf;
    void *arg[3];
    arg[0] = (void *)path;
    arg[1] = GINT_TO_POINTER(width);
    arg[2] = GINT_TO_POINTER(height);
#if 1
    // This gives priority to gtk thread...
    static gboolean gtk_thread_wants_lock = FALSE;
    if (self == g_thread_self()) {
        gtk_thread_wants_lock = TRUE;
    } else {
        // hold your horses...
        while (gtk_thread_wants_lock) threadwait();
    }
    pthread_mutex_lock(&pixbuf_mutex);

    //  g_warning("pthread_mutex_trylock(&pixbuf_mutex) on gtk thread failed for %s\n",
    
    pixbuf = (GdkPixbuf *)pixbuf_new_from_file_f((void *)arg);
    pthread_mutex_unlock(&pixbuf_mutex);
    if (self == g_thread_self()) gtk_thread_wants_lock = FALSE;

#else
    // This sends everything to the gtk thread... (slow)
	pixbuf = (GdkPixbuf *)utility_p->context_function(pixbuf_new_from_file_f, (void *)arg);
#endif

    return pixbuf;
}


void
pixbuf_hash_c::threadwait (void) {
    struct timespec thread_wait = {
        0, 100000000
    };
    nanosleep (&thread_wait, NULL);
}

