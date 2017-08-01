#ifndef LITE_C_HPP
#define LITE_C_HPP
#include "xffm+.h"

class lite_c {
    typedef struct composite_t{
	const gchar *sub_id;
	const gchar *where;
	double scale_factor;
	gint overall_alpha;    
    }composite_t;


    typedef struct lite_t {
	const gchar *id; 
	const gchar *type;
	const gchar *icon;
	guchar red;
	guchar green;
	guchar blue;
    } lite_t;

    public:

static void init(GHashTable **type_hash, GHashTable **key_hash){
    static gboolean initialized = FALSE;
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static GHashTable *lite_type_hash;
    static GHashTable *lite_key_hash;
    pthread_mutex_lock (&mutex);  
    if (!initialized) {

// We could read these from an xml file...
// To add a new ad hoc icon, first define the lite key to use to associate
// the mimetype to the litetype. Then define the lite type (lite_t) with
// the values you wish. The type (translated item) should be the same as 
// whatever is after the / or - in the lite type. This will be the tag
// that shows up (translated).

	lite_t lite_v[]={
	    {"lite/regular", NULL, NULL, 0xff, 0xff, 0xff}, 
	    {"lite/image", N_("image"), "emblem-image", 0xff, 0xcc, 0xff}, 
	    {"lite/Audio", N_("Audio"), "emblem-music", 0, 0xff, 0xff},
	    {"lite/Video", N_("Video"), "emblem-video", 0xff, 0xff, 0},
	    {"lite/office", N_("office"), "emblem-oo", 0xff, 0xff, 0xff}, 
	    {"lite/chemical", N_("chemical"), "emblem-science", 0xff, 0xff, 0xff}, 
	    {"lite/lyx", N_("lyx"), "emblem-math", 0xaa, 0xaa, 0xff}, 
	    {"lite/tex", N_("tex"), "emblem-math", 0xff, 0xff, 0xff}, 
	    {"lite/text", N_("text"), "emblem-text", 0xff, 0xff, 0xff},
	    {"lite/text-log", N_("log"), "emblem-bookmark", 0xff, 0xff, 0xff},
	    {"lite/text-readme", N_("readme"), "emblem-star", 0xff, 0xff, 0xff},
	    {"lite/text-credits", N_("credits"), "emblem-star", 0xff, 0xff, 0xff},
	    {"lite/text-authors", N_("authors"), "emblem-user", 0xff, 0xff, 0xff},
	    {"lite/text-install", N_("install"), "emblem-important", 0xff, 0xff, 0xff},
	    {"lite/text-info", N_("info"), "emblem-important", 0xff, 0xff, 0xff},
	    {"lite/text-html", N_("html"), "emblem-www", 0xff, 0xff, 0xff},
	    {"lite/text-chdr", N_("chdr"), "emblem-text", 0xee, 0xd6, 0x80},
	    {"lite/text-c++hdr", N_("c++hdr"), "emblem-text", 0xee, 0xd6, 0x80},
	    {"lite/text-csrc", N_("csrc"), "emblem-text", 0x88, 0x7f, 0xa3},
	    {"lite/text-c++", N_("c++"), "emblem-text", 0x88, 0x7f, 0xa3},
	    {"lite/application", N_("App"), "emblem-application", 0xdd, 0xdd, 0xff},
	    {"lite/graphics", N_("graphics"), "emblem-graphics", 0xff, 0xff, 0xff},
	    {"lite/pgp", N_("pgp"), "emblem-lock", 0x88, 0x88, 0x88},
	    {"lite/trash", N_("trash"), "emblem-bak", 0x22, 0x22, 0x22},
	    {"lite/pdf", N_("pdf"), "emblem-pdf", 0xaa, 0xdd, 0xcc},
	    {"lite/ps", N_("ps"), "emblem-print", 0xaa, 0xdd, 0xcc},
	    {"lite/msoffice", N_("msoffice"), "emblem-msoffice",  0xaa, 0xdd, 0xcc},
	    {"lite/package", N_("package"), NULL, 0x88, 0x44, 0x44},
	//    {"lite/package", N_("package"), "emblem-package", 0xaa, 0, 0},
	//    {"lite/executable", N_("executable"), "emblem-exec", 0xaa, 0xff, 0xaa},
	//    {"lite/script", N_("script"), "emblem-script", 0xaa, 0xff, 0xaa},
	    {"lite/core", N_("core"), "emblem-core", 0xaa, 0, 0},
	    {NULL,NULL,NULL, 0,0,0}
	};

	const gchar *lite_keys[] = {
	    //"image/jpeg", "lite/image",

	    "text/x-tex", "lite/tex",
	    "text/x-log", "lite/text-log",
	    "text/x-readme", "lite/text-readme",
	    "text/x-chdr", "lite/text-chdr",
	    "text/x-c++hdr", "lite/text-c++hdr",
	    "text/x-c", "lite/text-csrc",
	    "text/x-c++", "lite/text-c++",
	    "text/x-c++src", "lite/text-c++",
	    "text/x-csrc", "lite/text-csrc",
	    "text/x-credits", "lite/text-credits",
	    "text/x-authors", "lite/text-authors",
	    "text/x-install", "lite/text-install",
	    "text/x-info", "lite/text-info",
	    "text/html", "lite/text-html",
	    "application/x-lyx", "lite/lyx",
	    "application/x-trash", "lite/trash",
	    "application/pdf", "lite/pdf",
	    "application/x-bzpdf", "lite/pdf",
	    "application/x-gzpdf", "lite/pdf",

	    "application/x-dia-diagram", "lite/graphics",
	    "application/pgp", "lite/pgp",
	    "application/pgp-encrypted", "lite/pgp",

	    "application/x-core", "lite/core",
	    "application/x-coredump", "lite/core",

	    "application/postscript", "lite/ps",
	    "application/x-bzpostscript", "lite/ps",
	    "application/x-gzpostscript", "lite/ps",

	    "application/msword", "lite/msoffice",
	    "application/ms-powerpoint", "lite/msoffice",
	    "application/ms-excel", "lite/msoffice",
	    "application/ms-project", "lite/msoffice",
	    "application/msword-template", "lite/msoffice",
	    "application/vnd.ms-powerpoint", "lite/msoffice",
	    "application/vnd.ms-excel", "lite/msoffice",
	    "application/vnd.ms-project", "lite/msoffice",
	    "application/msword-template", "lite/msoffice",
	    "application/vnd.msword", "lite/msoffice",
	    
	    "application/x-tar", "lite/package",
	    "application/x-compress", "lite/package",
	    "application/x-compressed-tar", "lite/package",
	    "application/x-arj", "lite/package",
	    "application/x-lha", "lite/package",
	    "application/x-lzma", "lite/package",
	    "application/zip", "lite/package",
	    "application/x-gzip", "lite/package",
	    "application/x-lzip", "lite/package",
	    "application/x-cbz", "lite/package",
	    "application/x-cbr", "lite/package",
	    "application/x-xz", "lite/package",
	    "application/x-xz-compressed-tar", "lite/package",
	    "application/x-bzip", "lite/package",
	    "application/x-bzip2", "lite/package",
	    "application/x-bzip-compressed-tar", "lite/package",
	    "application/x-deb", "lite/package",
	    "application/x-rpm", "lite/package",
	    "application/x-java-archive", "lite/package",
	    "application/x-rar", "lite/package",
	    "application/x-ace", "lite/package",
	    "application/x-zoo", "lite/package",
	    "application/x-cpio", "lite/package",
	    "application/x-cpio-compressed", "lite/package",
	    "application/x-7z-compressed", "lite/package",

	    "application/x-shellscript", "lite/script",
	    "application/x-csh", "lite/script",
	    "text/x-csh", "lite/script",
	    "text/x-sh", "lite/script",
	    "text/x-shellscript", "lite/script",
	    "text/x-python", "lite/script",
	    "application/javascript", "lite/script",
	    "application/sieve", "lite/script",
	    "application/x-awk", "lite/script",
	    "application/x-m4", "lite/script",
	    "application/x-markaby", "lite/script",
	    "application/x-perl", "lite/script",
	    "application/x-php", "lite/script",
	    "application/x-ruby", "lite/script",
	    "application/x-setupscript", "lite/script",
	    "text/javascript", "lite/script",
	    "text/scriptlet", "lite/script",
	    "text/x-dcl", "lite/script",
	    "text/x-lua", "lite/script",
	    "text/x-matlab", "lite/script",
	    "text/x-tcl", "lite/script",
	    NULL, NULL
	};
	initialized = TRUE;
	lite_type_hash = g_hash_table_new (g_str_hash, g_str_equal);
	lite_key_hash = g_hash_table_new (g_str_hash, g_str_equal);
	lite_t *lite_type_p = lite_v;
	for (;lite_type_p && lite_type_p->id; lite_type_p++){
	    void *key = (void *)g_strdup(lite_type_p->id);
	    void *data = calloc(1, sizeof(lite_t));
	    memcpy(data, (void *)lite_type_p, sizeof(lite_t));
	    g_hash_table_insert(lite_type_hash,key, data);
	}
	const gchar **cp = lite_keys;
	for (;cp && *cp; cp+=2){
	    void *key = (void *)g_strdup(cp[0]);
	    void *data = (void *)g_strdup(cp[1]);
	    g_hash_table_insert(lite_key_hash, key, data);
	}
    }
    *type_hash = lite_type_hash;
    *key_hash = lite_key_hash;
    pthread_mutex_unlock (&mutex);  
    return;
}

static const gchar *
get_lite_emblem(const gchar *mimetype){
    static GHashTable *type_hash=NULL;
    static GHashTable *key_hash=NULL;
    if (!mimetype) return NULL;
    if (!key_hash) lite_c::init(&type_hash, &key_hash);
    const gchar *lite_key = (const gchar *)g_hash_table_lookup(key_hash, mimetype);
    if (!lite_key) return NULL;
    lite_t *lite_p = (lite_t *)g_hash_table_lookup(type_hash, lite_key);
    if (!lite_p) return NULL;
    return lite_p->icon;
}

static gboolean 
get_lite_colors(const gchar *mimetype, guchar *r, guchar *g, guchar *b){
    static GHashTable *type_hash=NULL;
    static GHashTable *key_hash=NULL;
    if (!mimetype) return FALSE;
     if (!key_hash) lite_c::init(&type_hash, &key_hash);
    const gchar *lite_key = (const gchar *)g_hash_table_lookup(key_hash, mimetype);
    if (!lite_key) return FALSE;
    lite_t *lite_p = (lite_t *)g_hash_table_lookup(type_hash, lite_key);
    if (!lite_p) return FALSE;
    *r = lite_p->red;
    *g = lite_p->green;
    *b = lite_p->blue;
    return TRUE;
}

};


#endif
