#ifndef XF_MIMEMAGIC_HH
#define XF_MIMEMAGIC_HH

// We can use either libmagic or perl mimetype, depending on configuration


namespace xf {
#ifdef HAVE_LIBMAGIC
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

static pthread_mutex_t magic_mutex = PTHREAD_MUTEX_INITIALIZER;
static magic_t cookie;
template <class Type>
class MimeMagic {
    //
    // This function may obtain a basic or alias mimetype, but will always
    // return a basic mimetype.
    static gchar *
    lib_magic (const gchar * file, int flags) {
	gchar *type=NULL;
	pthread_mutex_lock (&magic_mutex);  
	static gboolean initialized = FALSE;
	if (!initialized){
	    cookie = magic_open (MAGIC_NONE);
	    magic_load (cookie, NULL);
	    initialized = TRUE;
	}

	magic_setflags (cookie, flags);
	const char *ctype = magic_file (cookie, file);
	if (ctype) type = g_strdup(ctype);
	pthread_mutex_unlock (&magic_mutex);    
	return type;
    }

public:

    static gchar *
    mimeMagic(const gchar *file) {
	TRACE("mime_magic(%s)...\n", 
		file);
	// Does the user even have read permission?
	if (access(file, R_OK) < 0){
	    const gchar *h_type =
		_("No Read Permission");
	    return g_strdup(h_type);
	}
	
	gint flags = MAGIC_MIME_TYPE | MAGIC_SYMLINK;
	//preserve atime changes file attributes and loops monitor function
        //gint flags = MAGIC_MIME_TYPE | MAGIC_SYMLINK | MAGIC_PRESERVE_ATIME;
	gchar *mimemagic = lib_magic (file, flags);
	TRACE("mime_magic(%s)...%s\n", file, mimemagic);
	/*gchar *old_type = mimemagic; 
	mimemagic = rfm_natural(RFM_MODULE_DIR, "mime", mimemagic, "mime_get_alias_type");
	g_free(old_type);*/
	return mimemagic;
    }

 
    static gchar *
    mimeFile(const gchar *file) {
	TRACE("mimeFile(%s)...\n", file);
	gint flags =  MAGIC_PRESERVE_ATIME;
	gchar *f = lib_magic (file, flags);
	TRACE("mimeFile(%s)...%s\n", file, f);
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

    static gchar *
    encoding (const gchar *file) {
	TRACE("mime_encoding(%s)...\n", file);
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
	    TRACE("%s --> %s\n", file, encoding);
	    return encoding;
	}
	return NULL;
    }

};
#endif
}
#endif
