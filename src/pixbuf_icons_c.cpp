#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <errno.h>
#include "pixbuf_icons_c.hpp"

#ifndef PREFIX
# warning "PREFIX not defined!"
#endif

pixbuf_icons_c::pixbuf_icons_c(void){
    utility_p = new utility_c();
    pixbuf_mutex = PTHREAD_MUTEX_INITIALIZER;
    icon_theme = gtk_icon_theme_get_default ();
    if (!icon_theme){
        fprintf(stderr, "cannot get default icon theme!\n");
        throw 1;
    }
    self = g_thread_self();
    gchar *resource_path = g_build_filename(PREFIX, "share", "icons", "xffm+", NULL);
    gtk_icon_theme_add_resource_path (gtk_icon_theme_get_default (),resource_path);

    // This works, but icons should be fixed size. 
    // Scalable vector graphics dont work, last time I checked...
    // Icons should be in the "symbolic" internal gtk name format (hack...)
    // This is mandatory for non icon-themed boxes (which is not the usual setup).
    gchar *path = g_build_filename(resource_path, "24x24", NULL);
    gtk_icon_theme_append_search_path (gtk_icon_theme_get_default (), path);
    g_free(path);

    // xffm+ svg icons are at:
    path = g_build_filename(PREFIX, "share", "icons", "xffm+", "scalable", "stock", NULL);
    gtk_icon_theme_prepend_search_path (gtk_icon_theme_get_default (), path);
    g_free(path);
    path = g_build_filename(PREFIX, "share", "icons", "xffm+", "scalable", "emblems", NULL);
    gtk_icon_theme_prepend_search_path (gtk_icon_theme_get_default (), path);
    g_free(path);

    g_free(resource_path);
}
pixbuf_icons_c::~pixbuf_icons_c(void){
    pthread_mutex_destroy(&pixbuf_mutex);
    delete utility_p;
}

void
pixbuf_icons_c::threadwait (void) {
    struct timespec thread_wait = {
        0, 100000000
    };
    nanosleep (&thread_wait, NULL);
}

/*GdkPixbuf *
pixbuf_icons_c::xffm_icon(const gchar *icon_name, gint size){

}*/


gboolean
pixbuf_icons_c::is_composite_icon_name(const gchar *icon_name){
    const gchar *placements[]={
        "/NW/", "/N/","/NE/",
        "/W/",  "/C/","/E/",
        "/SW/", "/S/","/SE/",NULL};
    const gchar **p = placements;
    for (;p && *p; p++){
        if (strstr(icon_name, *p)){
            DBG("composite icon: %s\n", icon_name);
            return TRUE;
        }
    }
    return FALSE;
}
        
GdkPixbuf *
pixbuf_icons_c::absolute_path_icon(const gchar *icon_name, gint size){
    if (!g_path_is_absolute (icon_name)) return NULL;
    if (!g_file_test (icon_name, G_FILE_TEST_EXISTS)) {
        g_warning("*** %s does not exist.\n", icon_name);
        return NULL;
    }
    GdkPixbuf *pixbuf = pixbuf_new_from_file(icon_name, size, size); // width,height.
    // FIXME: (width, height) would be better than (size, size)
    if (pixbuf) return pixbuf;
    return NULL;
}

GdkPixbuf *
pixbuf_icons_c::get_theme_pixbuf(const gchar *icon_name, gint size){
    GdkPixbuf *pixbuf = NULL;
    GError *error = NULL;
    GdkPixbuf *theme_pixbuf = gtk_icon_theme_load_icon (icon_theme,
                  icon_name,
                  size, 
                  GTK_ICON_LOOKUP_FORCE_SIZE,  // GtkIconLookupFlags flags,
                  &error);
    if (error) {
        TRACE("pixbuf_icons_c::get_theme_pixbuf: %s\n", error->message);
        g_error_free(error);
    } else if (theme_pixbuf) {
        // Release any reference to the icon theme.
        pixbuf = gdk_pixbuf_copy(theme_pixbuf);
        g_object_unref(theme_pixbuf);
    }
    return pixbuf;
}



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
// 	NOOP(stderr, "** resizing %s\n", path);
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
pixbuf_icons_c::pixbuf_new_from_file (const gchar *path, gint width, gint height){
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


static void *
insert_pixbuf_tag_f (void *data){
    void **arg = (void **)data;
    const GdkPixbuf *tag = (const GdkPixbuf *)arg[0];
    GdkPixbuf *composite_pixbuf = (GdkPixbuf *)arg[1];
    const gchar *where = (const gchar *)arg[2];
    const gchar *scale = (const gchar *)arg[3];
    const gchar *alpha = (const gchar *)arg[4];
    pixbuf_icons_c *pixbuf_icons_p =(pixbuf_icons_c *)arg[5];

    double scale_factor = strtod(scale, NULL);
    if (isnan(scale_factor)) return NULL;
    if (scale_factor < 1 || scale_factor > 5) return NULL;
    errno = 0;
    gint overall_alpha = strtol(alpha, NULL, 10);
    if (errno){
        g_warning("insert_pixbuf_tag_f(): strtol(%s) -> %s\n", alpha, strerror(errno));
        return NULL;
    }
    
    gdouble scale_x = 1.0 / (scale_factor);
    gdouble scale_y = 1.0 / (scale_factor);
    gint width = gdk_pixbuf_get_width(tag);
    gint height = gdk_pixbuf_get_height(tag);

    GdkPixbuf *tag_s = gdk_pixbuf_scale_simple(tag, 
	    floor(scale_x*width), floor(scale_y*height),
	    GDK_INTERP_BILINEAR);   


    gint dest_width = gdk_pixbuf_get_width (composite_pixbuf);
    gint dest_height = gdk_pixbuf_get_height (composite_pixbuf);
    gint s_width = gdk_pixbuf_get_width (tag);
    gint s_height = gdk_pixbuf_get_height (tag);

    s_width = ((gdouble) s_width) * scale_x;
    s_height = ((gdouble) s_height) * scale_y;

    // default SW
    gdouble offset_x = 0.0;
    gdouble offset_y = dest_height - s_height;

    if(strcmp (where, "SW") == 0) {
        offset_x = 0.0;
        offset_y = dest_height - s_height;
    } 
    else if(strcmp (where, "SE") == 0) {
        offset_x = dest_width - s_width;
        offset_y = dest_height - s_height;
    } 
    else if(strcmp (where, "S") == 0) {
        offset_x = (dest_width - s_width) / 2;
        offset_y = dest_height - s_height;
    }
    else if(strcmp (where, "NW") == 0) {
        offset_x = 0.0;
        offset_y = 0.0;
    } 
    else if(strcmp (where, "NE") == 0) {
        offset_x = dest_width - s_width;
        offset_y = 0.0;

    } 
    else if (strcmp (where, "N") == 0) {
        offset_x = (dest_width - s_width) / 2;
        offset_y = 0.0;
    } 
    else if(strcmp (where, "C") == 0) {
        offset_x = (dest_width - s_width) / 2;
        offset_y = (dest_height - s_height) / 2;
    } 
    else if (strcmp (where, "E") == 0) {
        offset_x = dest_width - s_width;
        offset_y = (dest_height - s_height) / 2;
    } else if (strcmp (where, "W") == 0) {
        offset_x = 0.0;
        offset_y = (dest_height - s_height) / 2;
    }
    cairo_t   *pixbuf_context = pixbuf_icons_p->pixbuf_cairo_create(composite_pixbuf);
    
    // This proved necessary in opensuse-12.3,
    // but not in gentoo nor ubuntu. Go figure...
    gdk_cairo_set_source_pixbuf(pixbuf_context, composite_pixbuf,0,0);
    cairo_paint_with_alpha(pixbuf_context, 1.0);
	
    gdk_cairo_set_source_pixbuf(pixbuf_context, tag_s, offset_x,offset_y);
    cairo_paint_with_alpha(pixbuf_context, (double)overall_alpha/255.0);
    pixbuf_icons_p->pixbuf_cairo_destroy(pixbuf_context, composite_pixbuf);
    g_object_unref(tag_s);
    return NULL;
}



gboolean
pixbuf_icons_c::insert_pixbuf_tag (GdkPixbuf *tag, GdkPixbuf *composite_pixbuf,
	const gchar *where, const gchar *scale, const gchar *alpha){
    gdouble scale_factor;
    gint overall_alpha;
    
    if (!tag || !composite_pixbuf || !GDK_IS_PIXBUF(composite_pixbuf) || !where
            || !scale || !alpha) {
	return FALSE;
    }

    void *arg[] = {
        (void *)tag,
        (void *)composite_pixbuf,
        (void *)where,
        (void *)scale,
        (void *)alpha,
        (void *)this
    };
    
    // Done by main gtk thread:
    utility_p->context_function(insert_pixbuf_tag_f, arg);
    return TRUE;
}









////////////////////////////////////////////////////////////////////////////////////////

#if 0
// def OLDCODE

#ifdef COPYRIGHT_INFORMATION
#include "gplv3.h"
#endif


static
GdkPixbuf *
get_pixbuf (const gchar * key, gint size, gboolean replace_pixbuf);

static pthread_mutex_t lite_hash_mutex = PTHREAD_MUTEX_INITIALIZER;

// internal hash record structure
static GHashTable *lite_hash = NULL;
static GHashTable *lite_type_hash = NULL;
static GHashTable *lite_key_hash = NULL;

typedef struct composite_t{
    const gchar *sub_id;
    const gchar *where;
    double scale_factor;
    gint overall_alpha;    
}composite_t;



static void
free_pixbuf(void *data){
    GdkPixbuf *pixbuf = data;
    if (pixbuf && !G_IS_OBJECT (pixbuf)) {
	DBG("This should not happen: pixbuf is not a pixbuf\n");
    } else {
	g_object_unref (pixbuf);
    }
}


typedef struct lite_t {
    gchar *id; 
    gchar *type;
    gchar *icon;
    guchar red;
    guchar green;
    guchar blue;
} lite_t;

// We could read these from an xml file...
// To add a new ad hoc icon, first define the lite key to use to associate
// the mimetype to the litetype. Then define the lite type (lite_t) with
// the values you wish. The type (translated item) should be the same as 
// whatever is after the / or - in the lite type. This will be the tag
// that shows up (translated).
lite_t lite_v[]={
    {"lite/regular", NULL, NULL, 0xff, 0xff, 0xff}, 
    {"lite/image", N_("image"), "xffm/emblem_image", 0xff, 0xcc, 0xff}, 
    {"lite/Audio", N_("Audio"), "xffm/emblem_music", 0, 0xff, 0xff},
    {"lite/Video", N_("Video"), "xffm/emblem_video", 0xff, 0xff, 0},
    {"lite/office", N_("office"), "xffm/emblem_oo", 0xff, 0xff, 0xff}, 
    {"lite/chemical", N_("chemical"), "xffm/emblem_science", 0xff, 0xff, 0xff}, 
    {"lite/lyx", N_("lyx"), "xffm/emblem_math", 0xaa, 0xaa, 0xff}, 
    {"lite/tex", N_("tex"), "xffm/emblem_math", 0xff, 0xff, 0xff}, 
    {"lite/text", N_("text"), "xffm/emblem_text", 0xff, 0xff, 0xff},
    {"lite/text-log", N_("log"), "xffm/emblem_bookmark", 0xff, 0xff, 0xff},
    {"lite/text-readme", N_("readme"), "xffm/emblem_star", 0xff, 0xff, 0xff},
    {"lite/text-credits", N_("credits"), "xffm/emblem_star", 0xff, 0xff, 0xff},
    {"lite/text-authors", N_("authors"), "xffm/emblem_user", 0xff, 0xff, 0xff},
    {"lite/text-install", N_("install"), "xffm/emblem_important", 0xff, 0xff, 0xff},
    {"lite/text-info", N_("info"), "xffm/emblem_important", 0xff, 0xff, 0xff},
    {"lite/text-html", N_("html"), "xffm/emblem_www", 0xff, 0xff, 0xff},
    {"lite/text-chdr", N_("chdr"), "xffm/emblem_text", 0xee, 0xd6, 0x80},
    {"lite/text-c++hdr", N_("c++hdr"), "xffm/emblem_text", 0xee, 0xd6, 0x80},
    {"lite/text-csrc", N_("csrc"), "xffm/emblem_text", 0x88, 0x7f, 0xa3},
    {"lite/text-c++", N_("c++"), "xffm/emblem_text", 0x88, 0x7f, 0xa3},
    {"lite/application", N_("App"), "xffm/emblem_application", 0xdd, 0xdd, 0xff},
    {"lite/graphics", N_("graphics"), "xffm/emblem_graphics", 0xff, 0xff, 0xff},
    {"lite/pgp", N_("pgp"), "xffm/emblem_lock", 0x88, 0x88, 0x88},
    {"lite/trash", N_("trash"), "xffm/emblem_bak", 0x22, 0x22, 0x22},
    {"lite/pdf", N_("pdf"), "xffm/emblem_pdf", 0xaa, 0xdd, 0xcc},
    {"lite/ps", N_("ps"), "xffm/emblem_print", 0xaa, 0xdd, 0xcc},
    {"lite/msoffice", N_("msoffice"), "xffm/emblem_msoffice", 0x44, 0x44, 0x44},
    {"lite/package", N_("package"), "xffm/emblem_package", 0xaa, 0, 0},
    {"lite/executable", N_("executable"), "xffm/emblem_exec", 0xaa, 0xff, 0xaa},
    {"lite/script", N_("script"), "xffm/emblem_script", 0xaa, 0xff, 0xaa},
    {"lite/core", N_("core"), "xffm/emblem_core", 0xaa, 0xaa, 0xaa},
    {NULL,NULL,NULL, 0,0,0}
};

gchar *lite_keys[] = {
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
	

static void
init_lite_hash () {
    // g_once init
    static gsize initialized = 0;
    if (g_once_init_enter (&initialized)){
        lite_hash = g_hash_table_new_full (g_str_hash, g_str_equal,g_free, free_pixbuf);
        lite_type_hash = g_hash_table_new (g_str_hash, g_str_equal);
        lite_key_hash = g_hash_table_new (g_str_hash, g_str_equal);
	lite_t *lite_type_p = lite_v;
	for (;lite_type_p && lite_type_p->id; lite_type_p++){
	    g_hash_table_insert(lite_type_hash, lite_type_p->id, lite_type_p);
	}
	gchar **cp = lite_keys;
	for (;cp && *cp; cp+=2){
	    g_hash_table_insert(lite_key_hash, cp[0], cp[1]);
	}
	g_once_init_leave (&initialized, 1);
    }
    return;
}




static void *
pixbuf_from_gtkid_f(void *data){
    void **arg = data;
    gchar *id = arg[0];
    gint size = GPOINTER_TO_INT(arg[1]);
    GdkPixbuf *pixbuf = NULL;

    gchar *file = g_strdup_printf("%s/icons/rfm/scalable/stock/%s.svg",
                PACKAGE_DATA_DIR, id);
    NOOP(stderr, "trying: \"%s\"\n", file);
    if (!g_file_test(file, G_FILE_TEST_EXISTS)) {
        fprintf(stderr, "Failed to get pixbuf from %s-->%s\n", id, file);
        g_free(file);
        file = g_strdup_printf("%s/icons/rfm/scalable/emblems/emblem-%s.svg",
        PACKAGE_DATA_DIR, id);
        NOOP(stderr, "now trying: \"%s\"\n", file);
        if (!g_file_test(file, G_FILE_TEST_EXISTS)){
            g_free(file); 
            file = NULL;
        }
    }
    if (file){
       pixbuf = rfm_pixbuf_new_from_file(file, size, size);
    }
       
    
    g_free(file);
    return pixbuf;	    
}

GdkPixbuf *
pixbuf_from_gtkid(const gchar * id, gint size){
    if (!id) return NULL;
    GdkPixbuf *pixbuf = NULL;
    void *arg[] = {(void *)id, GINT_TO_POINTER(size)};
    arg[0] = g_strdup(id);
    arg[1] = GINT_TO_POINTER(size);
	
    pixbuf = pixbuf_from_gtkid_f(arg);
    
    if (pixbuf) g_object_ref(pixbuf);
    return pixbuf;
}

static const gchar *
get_mask_color(const gchar *id, guchar *red, guchar *green, guchar *blue, gchar **tpath){
    // 30 black:
    // 31 red: bright red, tar files
    // 32 green: document files; bright: executables; 
    // 33 yellow:
    // 34 blue: directories;
    // 35 magenta: bright: images;
    // 36 cyan: audio; bright: symlinks;
    // 37 white
   
    lite_t *lite_type_p = NULL;

    const gchar *key = "lite/regular";

    if (strncmp(id,"image/", strlen("image/"))==0) 
	key = "lite/image";
    else if (strncmp(id,"audio/", strlen("audio/"))==0)
	key = "lite/Audio";
    else if (strncmp(id,"video/", strlen("video/"))==0)
	key = "lite/Video";
    else if (strstr(id,"opendocument") || strstr(id,"officedocument") || strstr(id,"application/rtf")
	    ||strstr(id,"vnd.sun.xml"))
	key = "lite/office";
    else if (strstr(id,"abobe"))
	key = "lite/graphics";
    else if (strncmp(id,"text/", strlen("text/"))==0){
	key = g_hash_table_lookup(lite_key_hash, id);
	if (!key) key = "lite/text";

    } else if (strncmp(id,"application/", strlen("application/"))==0){
	key = g_hash_table_lookup(lite_key_hash, id);
	if (!key) key = "lite/application";
    } 
    NOOP (stderr, "+++ getting lite id for %s: %s\n", id, key);

    lite_type_p = g_hash_table_lookup(lite_type_hash, key);

    if (!lite_type_p) {
	lite_type_p = g_hash_table_lookup(lite_type_hash, "lite/regular");
    }


    if (tpath && lite_type_p->icon) {
	if (!strstr(lite_type_p->icon, "xffm/emblem_")){
	    DBG("get_mask_color(): incorrect lite icon specification: %s\n", lite_type_p->icon);
	} else {
	gboolean gtk_theme = (getenv("RFM_USE_GTK_ICON_THEME") &&
			    strlen(getenv("RFM_USE_GTK_ICON_THEME")));
	    if (gtk_theme) {
		*tpath = ICON_get_filename_from_id(lite_type_p->icon);
	    } else {
		gchar *icon = g_strdup(lite_type_p->icon + strlen("xffm/emblem_"));
		*tpath = g_strdup_printf("%s/icons/rfm/scalable/emblems/emblem-%s.svg", 
			PACKAGE_DATA_DIR, icon);
		NOOP(stderr, "%s->%s\n", icon, file);
		g_free(icon);
	    }
	}
    }
    if (red) *red = lite_type_p->red;
    if (green) *green = lite_type_p->green;
    if (blue) *blue = lite_type_p->blue;
    
    return lite_type_p->id;
}

static void
add_label_pixbuf(cairo_t   *pixbuf_context,
	GdkPixbuf *pixbuf, 
	const gchar *lite_id, 
	gint size){
    rfm_global_t *rfm_global_p = rfm_global();
    if (!rfm_global_p) return;
    // Skip regular icons
    if (strcmp(lite_id, "lite/regular")==0) return;
    // Insert text into pixbuf
    gint x = 0;
    gint y = 0;
    const gchar *text_size = NULL;
    switch(size) {
	case SMALL_ICON_SIZE: text_size="xx-small"; break;
	case MEDIUM_ICON_SIZE: text_size="x-small"; break;
	case BIG_ICON_SIZE: text_size="medium"; break;
    }	
    if (!text_size) return ;	
    GdkPixbuf   *t_pixbuf = NULL;
    const gchar *icon_text;
    if (strchr(lite_id,'.')) icon_text = strrchr(lite_id,'.')+1;
    else if (strchr(lite_id,'-')) icon_text = strrchr(lite_id,'-')+1;
    else  icon_text = strrchr(lite_id,'/')+1;
    const gchar *color = "white";
    gchar *layout_text = g_strdup_printf("<span foreground=\"black\" background=\"%s\" size=\"%s\">%s </span>", color, text_size, _(icon_text));
    PangoLayout *layout = 
	gtk_widget_create_pango_layout (rfm_global_p->window, NULL);
    pango_layout_set_markup(layout, layout_text, -1);
    g_free(layout_text);
    PangoRectangle logical_rect;
    pango_layout_get_pixel_extents (layout, NULL, &logical_rect);
    x = gdk_pixbuf_get_width(pixbuf) - logical_rect.width-2;
    y = gdk_pixbuf_get_height(pixbuf) - logical_rect.height-2;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (logical_rect.width > gdk_pixbuf_get_width(pixbuf)-1) 
	logical_rect.width = gdk_pixbuf_get_width(pixbuf)-1;
    if (logical_rect.height > gdk_pixbuf_get_height(pixbuf)-1) 
	logical_rect.height = gdk_pixbuf_get_height(pixbuf)-1;

    //cairo_set_source_rgba(pixbuf_context, 1.0, 0.0, 0.0, 0.75);
    //
    t_pixbuf =  
	gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, logical_rect.width+2, logical_rect.height+2);
    cairo_t   *t_pixbuf_context = pixbuf_cairo_create(t_pixbuf);

    //cairo_set_source_rgba(t_pixbuf_context, 1.0, 1.0, 1.0, 1.0);
    
    cairo_rectangle(t_pixbuf_context, 0, 0, logical_rect.width+2, logical_rect.height+2);
    //cairo_fill(t_pixbuf_context);
    cairo_clip(t_pixbuf_context);


    cairo_move_to (t_pixbuf_context, 1, 1);
	
    cairo_set_source_rgba(t_pixbuf_context, 0, 0, 0, 1.0);
    //cairo_set_source_rgba(t_pixbuf_context, 0, 0, 0, 0.5);
    if (PANGO_IS_LAYOUT (layout)) {
        pango_cairo_show_layout (t_pixbuf_context, layout);
        g_object_unref(layout);
    }
    pixbuf_cairo_destroy(t_pixbuf_context, t_pixbuf);

    if (t_pixbuf) {
	gdk_cairo_set_source_pixbuf(pixbuf_context, t_pixbuf,x,y);
	cairo_paint_with_alpha(pixbuf_context, 0.650);
	g_object_unref(t_pixbuf);
    }

    return ;
}

static void
add_color_pixbuf(cairo_t   *pixbuf_context, GdkPixbuf *pixbuf, 
	guchar red, guchar green, guchar blue){
    GdkPixbuf *pixbuf_mask = create_pixbuf_mask(pixbuf, red, green, blue);  
    gdk_cairo_set_source_pixbuf(pixbuf_context, pixbuf_mask, 0,0);
    cairo_paint_with_alpha(pixbuf_context, 0.450);
    g_object_unref(pixbuf_mask);
}

static void
add_emblem_pixbuf(cairo_t   *pixbuf_context, GdkPixbuf *pixbuf, gchar *tpath, gint size){

    if (tpath && g_file_test(tpath, G_FILE_TEST_EXISTS)){
	gdouble b = (double) size / 1.618;
	gint ia = size - floor(b);
	//gint ia = size/4;
	GdkPixbuf *tag = rfm_pixbuf_new_from_file(tpath, size/2, size/2);
	gdk_cairo_set_source_pixbuf(pixbuf_context, tag,ia,size/4);
	cairo_paint_with_alpha(pixbuf_context, 1.0);
	//cairo_paint_with_alpha(pixbuf_context, 0.650);

	g_object_unref(tag);
    }
}

static GdkPixbuf *
ad_hoc_pixbuf(GdkPixbuf *pixbuf, const gchar *id, gint size){
    GdkPixbuf *old_p = pixbuf;
    pixbuf = rfm_pixbuf_duplicate(old_p);
    g_object_unref(old_p);
    g_object_ref(pixbuf);
    rfm_global_t *rfm_global_p = rfm_global();
    if (!rfm_global_p) return pixbuf;  


#if 10

    
    guchar red, green, blue;
    gchar *tpath = NULL;
    const gchar *lite_id = get_mask_color(id, &red, &green, &blue, &tpath);



#endif
    cairo_t   *pixbuf_context = pixbuf_cairo_create(pixbuf);

    // This proved necessary in opensuse-12.3,
    // but not in gentoo nor ubuntu. Go figure...
	gdk_cairo_set_source_pixbuf(pixbuf_context, pixbuf,0,0);
	cairo_paint_with_alpha(pixbuf_context, 1.0);

    // Color:
    add_color_pixbuf(pixbuf_context, pixbuf, red, green, blue);
	
    // Emblem:	
    add_emblem_pixbuf(pixbuf_context, pixbuf,tpath, size);
    g_free(tpath);

    // Text:
    add_label_pixbuf(pixbuf_context, pixbuf, lite_id, size);
	

    // Using the same hash is not correct.
    // rfm_put_in_pixbuf_hash(lite_id, size, pixbuf);
    pixbuf_cairo_destroy(pixbuf_context, pixbuf);
	
    gchar *hash_key = rfm_get_hash_key (lite_id, size);
    pthread_mutex_lock(&lite_hash_mutex);
    g_hash_table_replace(lite_hash, hash_key, pixbuf);
    pthread_mutex_unlock(&lite_hash_mutex);
    
    return pixbuf;
}

static GdkPixbuf *
try_app_icon(const gchar *id, gint size){
    GdkPixbuf *pixbuf = NULL;
    // try svg application icons?
    if (!pixbuf){
	gchar *fullpath = g_strdup_printf("%s/pixmaps/%s.svg", PACKAGE_DATA_DIR, id);
	pixbuf = rfm_pixbuf_new_from_file(fullpath, size, size);
        if (!pixbuf) {
            DBG("svg application icon not found %s -> %s\n", id, fullpath);
            g_free(fullpath);
	    fullpath = g_strdup_printf("%s/icons/hicolor/scalable/apps/%s.svg",
                    PACKAGE_DATA_DIR, id);
	    pixbuf = rfm_pixbuf_new_from_file(fullpath, size, size);
        }
        if (!pixbuf) DBG("svg application icon not found %s -> %s\n", id, fullpath);
        g_free(fullpath);
    }
    // try png application icons?
    if (!pixbuf){
	gchar *fullpath = g_strdup_printf("%s/pixmaps/%s.png", PACKAGE_DATA_DIR, id);
	pixbuf = rfm_pixbuf_new_from_file(fullpath, size, size);
        if (!pixbuf) {
            DBG("png application icon not found %s -> %s\n", id, fullpath);
            g_free(fullpath);
	    fullpath = g_strdup_printf("%s/icons/hicolor/48x48/apps/%s.png",
                    PACKAGE_DATA_DIR, id);
	    pixbuf = rfm_pixbuf_new_from_file(fullpath, size, size);
        }
        if (!pixbuf) DBG("svg application icon not found %s -> %s\n", id, fullpath);
	g_free(fullpath);
    }
    // try xpm application icons?
    if (!pixbuf){
	gchar *fullpath = g_strdup_printf("%s/pixmaps/%s.xpm", PACKAGE_DATA_DIR, id);
	pixbuf = rfm_pixbuf_new_from_file(fullpath, size, size);
        if (!pixbuf) DBG("png application icon not found %s -> %s\n", id, fullpath);
	g_free(fullpath);
    }
    return pixbuf;
}

static GdkPixbuf *
last_resort_pixbuf(gint size, const gchar *id){
    NOOP("last_resort_pixbuf: %s\n", id);
    init_lite_hash();
    GdkPixbuf *pixbuf = NULL;
    gchar *tpath = NULL;
    guchar red, green, blue;
    const gchar *lite_id = get_mask_color(id, &red, &green, &blue, &tpath);
    g_free(tpath); // No used... really now?
    tpath = NULL;
    if (!lite_id) {
	DBG("Could not retrieve lite_id\n");
	return NULL;
    }

    gchar *hash_key = rfm_get_hash_key (lite_id, size);
    pthread_mutex_lock(&lite_hash_mutex);
    pixbuf = g_hash_table_lookup(lite_hash, hash_key);
    pthread_mutex_unlock(&lite_hash_mutex);
    g_free(hash_key);

    if (pixbuf){
	g_object_ref(pixbuf);
	return pixbuf;
    }

    hash_key = rfm_get_hash_key ("lite/regular", size);
    pthread_mutex_lock(&lite_hash_mutex);
    pixbuf = g_hash_table_lookup(lite_hash, hash_key);
    pthread_mutex_unlock(&lite_hash_mutex);
    g_free(hash_key);

    if (pixbuf) g_object_ref(pixbuf);

    if ( !pixbuf) pixbuf = try_app_icon(id, size);


    if (!pixbuf) {
	gchar *fullpath = g_strdup_printf("%s/icons/rfm/scalable/emblems/emblem-file.svg", PACKAGE_DATA_DIR);
	pixbuf = rfm_pixbuf_new_from_file(fullpath, size, size);
	g_free(fullpath);
	if (pixbuf) {
	    g_object_ref(pixbuf); // refs.   
	    hash_key = rfm_get_hash_key ("lite/regular", size);
	    pthread_mutex_lock(&lite_hash_mutex);
	    g_hash_table_replace(lite_hash, hash_key, pixbuf);
	    pthread_mutex_unlock(&lite_hash_mutex);
	}
    }

    gboolean gtk_icontheme = (getenv("RFM_USE_GTK_ICON_THEME") && strlen(getenv("RFM_USE_GTK_ICON_THEME")));

    if (gtk_icontheme && !pixbuf) {
	pixbuf = pixbuf_from_gtkid("file", size); // refs.
	if (pixbuf) {
	    hash_key = rfm_get_hash_key ("lite/regular", size);
	    pthread_mutex_lock(&lite_hash_mutex);
	    g_hash_table_replace(lite_hash, hash_key, pixbuf);
	    pthread_mutex_unlock(&lite_hash_mutex);
	}
    }

    if (!pixbuf){
	DBG("unable to get lite/regular pixbuf\n");
	return NULL;
    }

    if (strcmp(lite_id, "lite/regular")==0)
    {
	return pixbuf;
    }

    // Create ad hoc pixbuf
    return ad_hoc_pixbuf(pixbuf, id, size);

}


static 
gchar *
fallback_icon_path(const gchar * id){
    gchar *fullpath=NULL;
    if (strstr(id, "xffm/emblem_")){   
	fullpath = g_strdup_printf("%s/icons/rfm/scalable/emblems/emblem-%s.svg", 
		PACKAGE_DATA_DIR, id+strlen("xffm/emblem_"));
    }
    return fullpath;
}



static GdkPixbuf *
make_pixbuf_from_id (const gchar * id, int size) {
    GdkPixbuf *pixbuf=NULL;
    // Short circuit to items in pixmaps directory... (ghack)
    pixbuf = try_app_icon(id, size);
    if(pixbuf){
        g_object_ref(pixbuf);
        return pixbuf;
    }
    

    NOOP (stderr, "ICONx:make_pixbuf_from_id: %s\n", id);
    if(!id || !strlen (id)) return NULL;
    const gchar *c_key="/compositeC";
    const gchar *cc_key="/compositeC/";
    const gchar *ccc_key="compositeC/";
    const gchar *position="C";
    gdouble scale_f=1.8;
    gint overall_alpha=180;
 
    if (strstr(id, "/compositeC/")){
	c_key="/compositeC";
	cc_key="/compositeC/";
	ccc_key="compositeC/";
	scale_f=2.2;
	position="C";
	overall_alpha=225;
    }
    else if (strstr(id, "/compositeX/")){
	c_key="/compositeX";
	cc_key="/compositeX/";
	ccc_key="compositeX/";
	scale_f=1.0;
	position="C";
        overall_alpha=150;
    }
    else if (strstr(id, "/compositeNE/")){
	c_key="/compositeNE";
	cc_key="/compositeNE/";
	ccc_key="compositeNE/";
	scale_f=3.0;
	position="NE";
    }
    else if (strstr(id, "/compositeNW/")){
	c_key="/compositeNW";
	cc_key="/compositeNW/";
	ccc_key="compositeNW/";
	position="NW";
	scale_f=3.0;
    }
    else if (strstr(id, "/compositeNC/")){
	c_key="/compositeNC";
	cc_key="/compositeNC/";
	ccc_key="compositeNC/";
	position="NC";
	scale_f=3.0;
    }
    else if (strstr(id, "/compositeSC/")){
	c_key="/compositeSC";
	cc_key="/compositeSC/";
	ccc_key="compositeSC/";
	position="SC";
    }
    else if (strstr(id, "/compositeSW/")){
	c_key="/compositeSW";
	cc_key="/compositeSW/";
	ccc_key="compositeSW/";
	position="SW";
	scale_f=1.5;
	overall_alpha=210;
    }
    /*else if (strstr(id, "/compositeHSW/")){
	c_key="/compositeSW";
	cc_key="/compositeSW/";
	ccc_key="compositeSW/";
	position="SW";
        scale_f=1.4;
	overall_alpha=255;
    }*/    
    else if (strstr(id, "/compositeSE/")){
	c_key="/compositeSE";
	cc_key="/compositeSE/";
	ccc_key="compositeSE/";
	position="SE";
    }

    if(strstr (id, c_key)) {
        gchar *basic_type = g_strdup (id);
        NOOP (stderr, "composite type = %s\n", id);

        GdkPixbuf *basic_pixbuf;
        if (strstr (basic_type, c_key)) *(strstr (basic_type, c_key)) = 0;
        else DBG("make_pixbuf_from_id(): strstr (basic_type, c_key) returns NULL.");

        basic_pixbuf = get_pixbuf (basic_type, size, FALSE); //refs
        if(!basic_pixbuf){
	    g_free (basic_type);
	    return NULL;
	}
        if(strstr (id, cc_key) == NULL) {
	    DBG("incomplete composite specification: %s\n", id);
	    g_object_unref(basic_pixbuf);
	    g_free (basic_type);
	    return NULL;
        }
	NOOP(stderr, "duplicating pixbuf for %s: %s\n", basic_type, id);
        g_free (basic_type);

        GdkPixbuf *composite_pixbuf = rfm_pixbuf_duplicate(basic_pixbuf);

	if (!composite_pixbuf) {
	    g_error("cannot duplicate basic pixbuf for %s\n", id);
	    return  basic_pixbuf;
	}
	g_object_unref(basic_pixbuf);
	g_object_ref(composite_pixbuf);


        gchar *composite_id = g_strdup(strstr(id, cc_key) + strlen(cc_key));

	// split
	gint count=0;
	gchar *p;
	gchar *comp_id[10];
	p=comp_id[count]=composite_id;
	count++;
	while (strchr(p,'/')){
	    *strchr(p,'/')=0;
	    p += (strlen(p)+1);
	    if (strncmp(p,ccc_key,strlen(ccc_key)!=0)){
		NOOP(" composite type= %s\n", p);
		comp_id[count]=p;
		count++; 
	    }
	}
	gint j;
	const gchar *group="xffm";
	for (j=0; j<count; j++){
	    // Determine the group to which the icon belongs to
	    gchar *groups[]={
		"application", "audio", 
		"chemical", "image", 
		"inode", "message",
		"model", "multipart", 
		"text", "video", 
		"x-conference",	"x-content", 
		"x-epoc", "x-world",
		NULL};
	    gchar **pp=groups;
	    for(; pp && *pp; pp++){
		if (strcmp(comp_id[j],*pp)==0){
		    group=*pp;
		    continue;
		}
	    }
	    // Don't composite with group names (only with xffm group).
	    if (strcmp(group, comp_id[j])==0) continue;
	    gchar *c_id = NULL;
	    if (!c_id) {
		// no special position found
		c_id = g_strdup_printf ("%s/%s", group, comp_id[j]);
	    }
	    NOOP(stderr, "id = %s composite tag=%s\n", id, c_id);
		    
	    GdkPixbuf *tag = get_pixbuf (c_id, size, FALSE); //refs
	    if(!tag) {
		NOOP ("cannot get pixbuf for id: %s\n", c_id);
		g_free(c_id);
		g_free(composite_id);
		return composite_pixbuf;
	    }
	    rfm_insert_pixbuf_tag (tag, composite_pixbuf, position, scale_f, overall_alpha);
	    g_free(c_id);
	    g_object_unref(tag);
	}
	g_free(composite_id);

        return composite_pixbuf;
    }

    gboolean gtk_icontheme = (getenv("RFM_USE_GTK_ICON_THEME") && strlen(getenv("RFM_USE_GTK_ICON_THEME")));

// hack...
    if (strcmp(id, "image/png")==0) id = "xffm/emblem_image";
    if (strcmp(id, "xffm/")==0) id = "xffm/stock_file";




    gchar *file = NULL;
    if (strcmp(id, _("unknown"))==0) {
	if (gtk_icontheme) file = ICON_get_filename_from_id ("xffm/stock_file");
	else file = ICON_get_filename_from_id ("xffm/emblem_file");

    } else {
	file = ICON_get_filename_from_id (id); 
    }

    // Fallback icons
    if (!file) {
	gboolean is_stock_icon = (strncmp(id, "xffm/stock_", strlen("xffm/stock_")) == 0);
	gboolean is_dir_icon = (strncmp(id, "xffm/stock_directory", strlen("xffm/stock_directory")) == 0);
	gboolean is_file_icon = (strncmp(id, "xffm/stock_file", strlen("xffm/stock_file")) == 0);	
	if (is_dir_icon || is_file_icon || !is_stock_icon){
	    gchar *new_id=NULL;
	    if (strcmp(id, "inode/directory")==0) id = "xffm/stock_directory";

	    if (!gtk_icontheme && is_dir_icon ) {
		new_id = g_strdup_printf("xffm/emblem_folder%s", id + strlen("xffm/stock_directory"));
	    } else if (!gtk_icontheme && is_file_icon) {
		new_id = g_strdup_printf("xffm/emblem_file%s", id + strlen("xffm/stock_file"));
	    } else {
		new_id = g_strdup(id);
	    }
	    file = fallback_icon_path(new_id);
	    g_free(new_id);
	}
    }
    // Create pixbuf from path to image file
    if (file){
	pixbuf = rfm_pixbuf_new_from_file(file, size, size); // refs
        // This is wrong. Pixbuf already has a reference returned from gdk_pixbuf function.
	/*if (pixbuf) g_object_ref(pixbuf);
	else DBG("make_pixbuf_from_id(%s, %d): returns NULL!\n", id, size);*/
#ifdef DEBUG
	if (pixbuf==NULL) DBG("make_pixbuf_from_id(%s, %d): returns NULL!\n", id, size);
#endif
	g_free(file);
	NOOP(stderr, "id=%s\n", id);
	if (!strstr(id, "xffm/stock_directory") 
		&& !strstr(id, "xffm/emblem")
		&& !strstr(id, "xffm/stock_home")
		&& !strstr(id, "inode/directory")
		&& !strstr(id, "xffm/stock_go-up")){
            if (pixbuf && GDK_IS_PIXBUF(pixbuf)){
	    guchar red, green, blue;
	    init_lite_hash();
	        gchar *tpath = NULL;
	        const gchar *lite_id = get_mask_color(id, &red, &green, &blue, &tpath);
	        cairo_t   *pixbuf_context = pixbuf_cairo_create(pixbuf);
                // This proved necessary in opensuse-12.3,
                // but not in gentoo nor ubuntu. Go figure...
		gdk_cairo_set_source_pixbuf(pixbuf_context, pixbuf,0,0);
		cairo_paint_with_alpha(pixbuf_context, 1.0);

                // Color:
                if (red != 0xff || green != 0xff || blue != 0xff)
                    add_color_pixbuf(pixbuf_context, pixbuf, red, green, blue);
                // Emblem:
                add_emblem_pixbuf(pixbuf_context, pixbuf,tpath, size);
                g_free(tpath);
                // Text:
                add_label_pixbuf(pixbuf_context, pixbuf, lite_id, size);
                pixbuf_cairo_destroy(pixbuf_context, pixbuf);
            }
	}


    } else {
	if (strncmp(id, "xffm/stock_", strlen("xffm/stock_"))==0) {
	    gchar *gtk_id = g_strdup_printf("%s", id+strlen("xffm/stock_"));
	    pixbuf = pixbuf_from_gtkid(gtk_id, size); // refs
            NOOP(stderr, "cannot get icon path from \"%s\", trying \"%s\" --> %d\n",
                    id, gtk_id, GPOINTER_TO_INT(pixbuf));
	    g_free(gtk_id);
	}
	
	if (!pixbuf) {
            if (strstr(id, "rodent")) DBG("Unknown pixbuf id: %s\n", id);
	}
    }
    if (!pixbuf) {
	pixbuf = last_resort_pixbuf(size, id); //refs belongs to litehash
	if(pixbuf) g_object_ref(pixbuf);
    }
    return pixbuf;
}

static
GdkPixbuf *
get_pixbuf (const gchar * key, gint size, gboolean replace_pixbuf) {
#if GTK_MAJOR_VERSION==3 
    //Bug workaround for non icontheme gtk-3.14, still present at time of test in gtk-3.16
    static gsize initialized = 0;
        if (g_once_init_enter (&initialized)) {
        // gtk 3.14 non deprecated method... 
        // add emblems and stock

        gchar *resource_path = g_build_filename(PREFIX, "share", "icons", "rfm", NULL);
        gtk_icon_theme_add_resource_path (gtk_icon_theme_get_default (),resource_path);

        // this works, but icons should be fixed size. scalable vector graphics dont work...
        // icons should be in the "symbolic" internal gtk name format (hack...)
        // This is for non icon-themed boxes, like mine.
        gchar *path = g_build_filename(resource_path, "24x24", NULL);
        gtk_icon_theme_append_search_path (gtk_icon_theme_get_default (), path);
        g_free(path);
        g_free(resource_path);
	g_once_init_leave (&initialized, 1);
    }
#endif
    


    NOOP ("get_pixbuf(%s, %d)\n", key, size);
    if(!key) {
        NOOP ("get_pixbuf(): key is NULL!\n");
	return NULL;
    }
    GdkPixbuf *pixbuf = NULL;
    // find in pixbuf hash
    if (!replace_pixbuf){
	pixbuf = rfm_find_in_pixbuf_hash (key, size); // refs
        if(pixbuf) {
	    NOOP ("HASH-primary-icons.c: %s found in pixbuf hash\n", key);
	    return pixbuf;
	}
    }


    // Pixbuf does not exist or will be replaced in the pixbuf hash table.
    const gchar *path = key;
    if(g_path_is_absolute (path)) {
	NOOP ("Direct access to source pixbuf file.\n");
	if (!g_file_test (path, G_FILE_TEST_EXISTS) || g_file_test (path, G_FILE_TEST_IS_DIR)){
	    NOOP ("get_pixbuf(%s): is not a pixbuf source file", path);
	    return NULL;
	}
        pixbuf = rfm_create_preview (path, size); // refs
        if(!pixbuf) {
            NOOP ("create_preview(%s, %d): returns NULL!\n", path, size);
            return NULL;
        }
    } else {
	NOOP (stderr, "Pixbuf creation from pixbuf id: make_pixbuf_from_id\n");
        pixbuf = make_pixbuf_from_id (path, size); // refs

        if(!pixbuf) {
            NOOP ("get_pixbuf(%s, %d): returns NULL!\n", path, size);
            return NULL;
        }
    }
    if (pixbuf && GDK_IS_PIXBUF(pixbuf)){
        rfm_put_in_pixbuf_hash(path, size, pixbuf); // no ref
    }
    return pixbuf;
}
 

#endif
