
#include "pixbuf_hash_c.hpp"
#include <iostream>

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

pixbuf_hash_c::pixbuf_hash_c(window_c *data){
    window_p = data;
    pixbuf_hash = 
	g_hash_table_new_full (g_str_hash, g_str_equal,g_free, free_pixbuf_t);

}

pixbuf_hash_c::~pixbuf_hash_c(void){
    g_hash_table_destroy (pixbuf_hash);

}


static void 
free_pixbuf_t(void *data){
    pixbuf_t *pixbuf_p = data;
    if (!pixbuf_p) return ;
    //NOOP(stderr, "destroying pixbuf_t for %s size %d\n", pixbuf_p->path, pixbuf_p->size);
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


void
pixbuf_hash_c::zap_thumbnail_file(const gchar *file, gint size){
    //Eliminate from thumbnail cache:
    gchar *thumbnail_path = rfm_get_thumbnail_path (file, size);
    if (g_file_test(thumbnail_path, G_FILE_TEST_EXISTS)) {
	if (g_file_test(thumbnail_path, G_FILE_TEST_EXISTS) && unlink(thumbnail_path) < 0) {
	    DBG("Cannot unlink thumbnail file: %s (%s)\n",
		thumbnail_path, strerror(errno));
	}
    }
    g_free (thumbnail_path);
}


static void *
put_in_pixbuf_hash_f(void *data){
    if (!pixbuf_hash) pixbuf_hash = init_pixbuf_hash ();
    void **arg = data;
    gchar *hash_key = arg[0];
    pixbuf_t *pixbuf_p = arg[1];
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
    pixbuf_t *pixbuf_p = (pixbuf_t *) malloc (sizeof (pixbuf_t));
    if (!pixbuf_p) g_error("malloc: %s\n", strerror(errno));
    memset (pixbuf_p, 0, sizeof (pixbuf_t));
    pixbuf_p->path = g_strdup (path);
    pixbuf_p->size = size;
    pixbuf_p->pixbuf = (GdkPixbuf *)pixbuf;

    if(g_path_is_absolute (path)) {
        struct stat st;
        if (stat (path, &st)==0){
            pixbuf_p->mtime = st.st_mtime;
            pixbuf_p->st_size = st.st_size;
            pixbuf_p->st_ino = st.st_ino;
        } else DBG("cannot stat %s\n", path);
    } 
    // Replace or insert item in pixbuf hash
    gchar *hash_key = rfm_get_hash_key (pixbuf_p->path, pixbuf_p->size);
    void *arg[]={hash_key, pixbuf_p};
    void *result = window_p->context_function(put_in_pixbuf_hash_f, arg);
    if (!result){
        g_free(hash_key);
	free_pixbuf_t(pixbuf_p);
    }
    // hash_key is inserted into hash and should not be freed.
    return ;
}

// This is to remove thumbnails from the hash
// Thumbnails are always absolute paths.

static void *
rm_from_pixbuf_hash_f(void *data){
    if (!pixbuf_hash) pixbuf_hash = init_pixbuf_hash ();
    gchar *hash_key = data;
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
pixbuf_hash_c::rm_from_pixbuf_hash (const gchar *fullpath, gint size) {
    TRACE("rfm_rm_from_pixbuf_hash()\n");
    if (!fullpath) return ;
    if (!pixbuf_hash) return;
        TRACE("rm from pixbuf hash: %s(%d)\n", fullpath, size);

    if(!g_path_is_absolute (fullpath)){
        DBG("rm_from_pixbuf_hash(): %s is not an absolute path.\n", fullpath);
    }
    gchar *hash_key = rfm_get_hash_key (fullpath, size);

    window_p->context_function(rm_from_pixbuf_hash_f, hash_key);
    g_free(hash_key);
    TRACE("rfm_rm_from_pixbuf_hash() done\n");
    return;
}



static void *
find_in_pixbuf_hash_f(void *data){
    if (!pixbuf_hash) pixbuf_hash = init_pixbuf_hash ();
    void **arg = data;
    const gchar *hash_key = arg[0];
    const gchar *key = arg[1];
    gint size = GPOINTER_TO_INT(arg[2]);


    pixbuf_t *pixbuf_p = g_hash_table_lookup (pixbuf_hash, hash_key);

    if(!pixbuf_p || !GDK_IS_PIXBUF(pixbuf_p->pixbuf)) {
	return NULL;
    }
    if (g_path_is_absolute (key)) {
	// Check for out of date source image files
	struct stat st;
	stat (key, &st);
	if(pixbuf_p->mtime != st.st_mtime || 
		pixbuf_p->st_size != st.st_size||
		pixbuf_p->st_ino != st.st_ino)
	{
	    // Obsolete item must be replaced in pixbuf hash
	    // and eliminated from thumnail cache.
	    zap_thumbnail_file(key, size);
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
pixbuf_hash_c::find_in_pixbuf_hash(const gchar *key, gint size){
    if (!key) return NULL;
    // This will report out of date thumbnails/previews as not present
    // On successful call, returned pixbuf_p->pixbuf will have
    // an additional reference which must be freed after use...
    GdkPixbuf *pixbuf = NULL;
    if(g_path_is_absolute (key)){
        TRACE("find in pixbuf hash: %s(%d)\n", key, size);
	if (!g_file_test (key, G_FILE_TEST_EXISTS)) return NULL;
        gchar *hash_key = rfm_get_hash_key (key, size);
        void *arg[]={hash_key, (gpointer) key, GINT_TO_POINTER(size)};
	pixbuf = rfm_context_function(find_in_pixbuf_hash_f, arg);
        g_free(hash_key);
	return pixbuf;
    }
    // Non absolute path for key, may be an xffm/mimetype identifier.
    // This will check whether the identifier is mapped to a particular
    // icon theme source image file.

    gchar *file = NULL;
    if (strcmp(key, _("unknown"))==0) {
	file = ICON_get_filename_from_id ("xffm/stock_file");
    } else {
	file = ICON_get_filename_from_id (key);
    }
    if (file) {
	if (!g_path_is_absolute (file)){
	    DBG("incorrect type:file association (%s:%s)\n", key,file); 
	    g_free(file); 
	    return NULL;
	}
        gchar *hash_key = rfm_get_hash_key (file, size);
        void *arg[]={hash_key, file, GINT_TO_POINTER(size)};
	pixbuf = rfm_context_function(find_in_pixbuf_hash_f, arg);
        g_free(hash_key);
	g_free(file);
	return pixbuf;
    }
    // If no source file is associated, we are dealing with a ad hoc icon
    gchar *hash_key = rfm_get_hash_key (key, size);
    void *arg[]={hash_key, (gpointer)key, GINT_TO_POINTER(size)};
    pixbuf = window_p->context_function(find_in_pixbuf_hash_f, arg);
    g_free(hash_key);
    return pixbuf;
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

