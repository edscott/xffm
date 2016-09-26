#include "mime_magic_c.hpp"

mime_magic_c::mime_magic_c(void){
    magic_mutex = PTHREAD_MUTEX_INITIALIZER;
    cookie = magic_open (MAGIC_NONE);
    magic_load (cookie, NULL);

}



// Lib magic is available...
//
// not thread safe: put in a mutex.
// This function may obtain a basic or alias mimetype, but will always
// return a basic mimetype.
gchar *
mime_magic_c::lib_magic (const gchar * file, int flags) {
    gchar *type=NULL;
    pthread_mutex_lock (&magic_mutex);    
    magic_setflags (cookie, flags);
    const char *ctype = magic_file (cookie, file);
    if (ctype) type = g_strdup(ctype);
    pthread_mutex_unlock (&magic_mutex);    
    return type;
}

// see "man libmagic" for explantion of flags
// Since MAGIC_NO_CHECK_ENCODING is not in file 4.x, we take care
// of that here.
#ifndef MAGIC_MIME_TYPE
#define MAGIC_MIME_TYPE  0
#endif
#ifndef MAGIC_NO_CHECK_APPTYPE
#define MAGIC_NO_CHECK_APPTYPE  0
#endif
#ifndef MAGIC_NO_CHECK_ENCODING
#define MAGIC_NO_CHECK_ENCODING  0
#endif
#ifndef MAGIC_SYMLINK
#define MAGIC_SYMLINK  0
#endif
#ifndef MAGIC_NO_CHECK_COMPRESS
#define MAGIC_NO_CHECK_COMPRESS  0
#endif
#ifndef MAGIC_NO_CHECK_TAR
#define MAGIC_NO_CHECK_TAR  0
#endif
#ifndef MAGIC_PRESERVE_ATIME
#define  MAGIC_PRESERVE_ATIME 0
#endif


 
gchar *
mime_magic_c::mime_magic_unalias (const gchar *file) {
    NOOP(stderr, "mime_magic(%s)...\n", 
	    file);
    // Does the user even have read permission?
    if (access(file, R_OK) < 0){
	const gchar *h_type =
	    _("No Read Permission");
        return g_strdup(h_type);
    }
    
    gint flags = MAGIC_MIME_TYPE | MAGIC_SYMLINK | MAGIC_PRESERVE_ATIME;
    gchar *mimemagic = lib_magic (file, flags);
    return mimemagic;
}

 
gchar *
mime_magic_c::mime_encoding (const gchar *file) {
    NOOP(stderr, "mime_encoding(%s)...\n", file);
    // Does the user even have read permission?
    if (access(file, R_OK) < 0){
	const gchar *h_type =
	    _("No Read Permission");
        return g_strdup(h_type);
    }
    //int flags = MAGIC_MIME_ENCODING;
    int flags = MAGIC_MIME_ENCODING | MAGIC_PRESERVE_ATIME | MAGIC_SYMLINK;
    gchar *encoding = lib_magic (file, flags);
    if (encoding) {
	NOOP(stderr, "%s --> %s\n", file, encoding);
	return encoding;
    }
    return NULL;
}

const gchar *
mime_magic_c::mime_file (const gchar *file) {
    NOOP(stderr, "mime_file(%s)...\n", file);
    gint flags =  MAGIC_PRESERVE_ATIME;
    gchar *f = lib_magic (file, flags);
    NOOP(stderr, "mime_file(%s)...%s\n", file, f);
    if (!f) {
	return NULL;
    }
    if (g_file_test(file, G_FILE_TEST_IS_SYMLINK)){
	flags |= MAGIC_SYMLINK;
	gchar *ff = f;
	f = lib_magic (file, flags);
	gchar *gf = g_strconcat(ff, "\n", f, NULL);
	g_free(f);
	g_free(ff);
	return gf;

    }
    return f;
}


//////////////////////////////////////////////////////////////////////////////


