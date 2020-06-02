#ifndef XFPIXBUF_HH
#define XFPIXBUF_HH
#include "pixbufhash.hh"
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "../common/util.hh"
#include "cairo.hh"

#ifndef PREFIX
# warning "PREFIX not defined: defaulting to /usr/local"
#define PREFIX "/usr/local"
#endif

namespace xf
{

static GThread *self;
static GtkIconTheme *icon_theme=NULL;
static pthread_mutex_t pixbuf_mutex;

template <class Type>
class Pixbuf {
    static void
    init(void){
	TRACE("Calling Pixbuf initializer...buildicons=%s\n", buildIcons);
	pixbuf_mutex = PTHREAD_MUTEX_INITIALIZER;
	icon_theme = gtk_icon_theme_get_default ();
	if (!icon_theme){
	    TRACE("cannot get default icon theme!\n");
	    icon_theme = gtk_icon_theme_new();
	    //throw 1;
	}

	self = g_thread_self();
	auto resource_path = g_build_filename(PREFIX, "share", "icons", "xffm+", NULL);
	if (g_file_test(resource_path, G_FILE_TEST_IS_DIR)) {
	    gtk_icon_theme_add_resource_path (icon_theme,resource_path);
	}
	
	if (buildIcons && g_file_test(buildIcons, G_FILE_TEST_IS_DIR)){
	    TRACE("adding resource path:%s\n", buildIcons);
	    gtk_icon_theme_add_resource_path (icon_theme,buildIcons);
	} else buildIcons = NULL;

	// This works, but icons should be fixed size. 
	// Scalable vector graphics dont work, last time I checked...
	// Icons should be in the "symbolic" internal gtk name format (hack...)
	// This is mandatory for non icon-themed boxes (which is not the usual setup).
        const gchar *subdirs[] = {
            "scalable",
            "96x96",
            "48x48",
            "24x24",
            "16x16",
            NULL
        };
        for (auto p=subdirs; p && *p; p++){
            auto path = g_build_filename(resource_path, *p, NULL);
	    gtk_icon_theme_append_search_path (icon_theme, path);
	    g_free(path);
        }
        if (buildIcons) for (auto p=subdirs; p && *p; p++){
            auto path = g_build_filename(buildIcons, *p, NULL);
	    gtk_icon_theme_append_search_path (icon_theme, path);
	    TRACE("appending icon search path: %s (%d)\n", path, g_file_test(path,G_FILE_TEST_IS_DIR));

	    g_free(path);
	    
        }
#if 0
	// xffm+ svg icons are at:
	path = g_build_filename(PREFIX, "share", "icons", "xffm+", "scalable", "stock", NULL);
	gtk_icon_theme_prepend_search_path (icon_theme, path);
	g_free(path);
	path = g_build_filename(PREFIX, "share", "icons", "xffm+", "scalable", "emblems", NULL);
	gtk_icon_theme_prepend_search_path (icon_theme, path);
	g_free(path);
#endif
	g_free(resource_path);
    }

    static gint
    get_pixel_size(gint size){
	gint pixels = 24;
	switch (size){
	    case GTK_ICON_SIZE_MENU:          // Size appropriate for menus (16px).
	    case GTK_ICON_SIZE_SMALL_TOOLBAR: // Size appropriate for small toolbars (16px).
	    case GTK_ICON_SIZE_BUTTON:        // Size appropriate for buttons (16px)
		pixels = 16; break;
	    case GTK_ICON_SIZE_LARGE_TOOLBAR: // Size appropriate for large toolbars (24px)
		pixels = 24; break;
	    case GTK_ICON_SIZE_DND:           // Size appropriate for drag and drop (32px)
		pixels = 32; break;
	    case GTK_ICON_SIZE_DIALOG:        // Size appropriate for dialogs (48px)
		pixels = 48; break;
	    default: 
		if (size < 0) pixels = abs(size);
		break;
	}
	return pixels;
    }

public:

    static GdkPixbuf *getImageAtSize(const gchar *iconName, gint pixels, 
	    struct stat *st_p = NULL){
	if (pixels > 24) return getPixbufWithThumb(iconName, pixels, st_p);
	auto pixbuf = getPixbuf("image-x-generic", -pixels);
	TRACE("getImageAtSize %s\n", iconName);
	if (strchr(iconName, '.')){
	    auto label = strrchr(iconName,'.')+1;
	    TRACE("insertPixbufLabel %s\n", label);
	    if (strlen(label)) Pixbuf<Type>::insertPixbufLabel(pixbuf, label);
	}
	return pixbuf;
    }

    // last reference to the returned pixbuf (if any) belongs to the
    // pixbuf hashtable.

    static GdkPixbuf *
    getPixbuf(const gchar *iconName, gint size){
	// Gtk enum size, not pixels
	if (!iconName) {
	    DBG("getPixbuf(NULL)\n");
	    return NULL;
	}
	// If in hash return pixbuf
	// else build pixbuf; putInHash; return pixbuf.
	//
	// Get actual pixels from gtk size.
	// If less than 0, then abs(size) are pixels.
	auto pixels = get_pixel_size(size);
	auto pixbuf = PixbufHash<Type>::find_in_pixbuf_hash(iconName, pixels);
	if (pixbuf) {
	    TRACE("pixbuf %s (%d) loaded from hash.\n", iconName, pixels);
	    return pixbuf;
	} else TRACE("pixbuf %s (%d) not in hash.\n", iconName, pixels);
	pixbuf = buildPixbuf(iconName, pixels);
        putInHash(iconName, pixels, pixbuf);
	return pixbuf;
    }

    static GdkPixbuf *
    getPixbufWithThumb(const gchar *iconName, gint height, 
	    struct stat *st_p=NULL)
    {
	if (!iconName) {
	    TRACE("getPixbufWithThumb(NULL)\n");
	    return NULL;
	}
	if (height <= 24) return getPixbuf("default", -height);
	if (thumbnailOK(iconName, height, st_p)){
	    auto pixbuf = 
		PixbufHash<Type>::find_in_pixbuf_hash(iconName, height);
	    if (pixbuf) return pixbuf;
	    // Read thumbnail.
	    pixbuf = readThumbnail(iconName, height);
	    if (pixbuf){
		putInHash(iconName, height, pixbuf);
		return pixbuf;
	    }
	} 
	// Thumbnail not OK.
	auto pixbuf = buildImagePixbuf(iconName, height);

	if (!pixbuf) {
	    ERROR("buildImagePixbuf(%s)\n", iconName);
	    return NULL;
	}
	// Put extension...

	saveThumbnail(iconName, height, pixbuf);
	putInHash(iconName, height, pixbuf);
	return pixbuf;
    }

    static void
    pixbufSave(GdkPixbuf *pixbuf, const gchar *path){
	GError *error = NULL;
	gdk_pixbuf_save (pixbuf, path, "png", &error,
			 "tEXt::Software", "Xffm+", NULL);
	if (error){
	    ERROR("pixbuf_save_f(%s): %s\n", path, error->message);
	    g_error_free(error);
	}

    }

private:
    static void
    saveThumbnail(const gchar *iconName, gint height, GdkPixbuf *pixbuf){
	if (!iconName || !pixbuf || !GDK_IS_PIXBUF(pixbuf)) {
	    ERROR("pixbuf_save_f(%s): !iconName || !GDK_IS_PIXBUF(pixbuf)\n", 
		    iconName);
	    return ;
	}
	auto thumbnailPath = PixbufHash<Type>::get_thumbnail_path (iconName, height);
	pixbufSave(pixbuf, thumbnailPath);
	g_free(thumbnailPath);
   }


    static GdkPixbuf *
    buildImagePixbuf(const gchar *path, gint height){
	GError *error = NULL;
	GdkPixbuf *pixbuf = NULL;
	pixbuf = gdk_pixbuf_new_from_file (path, &error);
	auto pixbufWidth = gdk_pixbuf_get_width(pixbuf);
	auto pixbufHeight = gdk_pixbuf_get_height(pixbuf);
	    
	auto newHeight = height;
	auto newWidth = pixbufWidth * height / pixbufHeight;

	if (newWidth > newHeight){
	    newWidth = height;
	    newHeight = pixbufHeight * height / pixbufWidth;
	}
	
	auto newPixbuf = 
	    gdk_pixbuf_scale_simple (pixbuf, 
		newWidth, newHeight, GDK_INTERP_HYPER);
	if (newPixbuf) {
	    g_object_unref(pixbuf);
	    pixbuf = newPixbuf; 
	}
	if (strchr(path, '.')){
	    auto label = strrchr(path,'.')+1;
	    if (strlen(label)) insertPixbufLabel(pixbuf, label);
	}
	// FIXME: insert symlink emblem if necessary.
	return pixbuf;
	
    }


    static GdkPixbuf *
    readThumbnail(const gchar *iconName, gint height){
	GError *error=NULL;
	auto thumbnailPath = PixbufHash<Type>::get_thumbnail_path (iconName, height);
	TRACE("Now loading readThumbnail from %s\n",  thumbnailPath);
	auto pixbuf = gdk_pixbuf_new_from_file (thumbnailPath, &error);
	if (error){
	    ERROR("readThumbnail(): %s (%s)\n", thumbnailPath, error->message);
	    g_error_free(error);
	    g_free(thumbnailPath);
	    return NULL;
	} 
	g_free(thumbnailPath);
	TRACE("preview.hh::loadFromThumbnails(%s): Success.\n", iconName);
	return pixbuf;
    }

    static gboolean thumbnailOK(const gchar *iconName, gint height, struct stat *st_p){
	auto thumbnailPath = PixbufHash<Type>::get_thumbnail_path (iconName, height);
	if (!g_file_test(thumbnailPath,G_FILE_TEST_EXISTS)) return FALSE;

	struct stat st;
	if (stat(thumbnailPath, &st)<0){
	    DBG("thumbnailOK(): stat %s (%s)\n",
		thumbnailPath, strerror(errno));
	    errno=0;
	    g_free(thumbnailPath);
	    return FALSE;
	}
	
	struct stat st_local;
	if (!st_p) st_p = &st_local;

	if (stat(iconName, st_p)<0) {
	    DBG("thumbnailOK(): stat %s (%s)\n",
		iconName, strerror(errno));
	    errno=0;
	    g_free(thumbnailPath);
	    return FALSE;    
	}

	if (st.st_mtime < st_p->st_mtime){
	    TRACE("thumbnailOK(%s): out of date. Removing %s.\n", 
		    iconName, thumbnailPath);
	    unlink(thumbnailPath);
	    g_free(thumbnailPath);
	    return FALSE;
	} 	
	return TRUE;
    }

    static void
    putInHash(const gchar *iconName, gint pixels, const GdkPixbuf *pixbuf){
	if (!iconName || !pixbuf){
	    ERROR("putInHash(): !iconName || !pixbuf\n");
	    return;
	}
	TRACE("putInHash() %s (%d)\n", iconName, pixels);
	PixbufHash<Type>::put_in_pixbuf_hash(iconName, pixels, pixbuf);
    }
    
    static GdkPixbuf *
    getThemePixbuf(const gchar *icon_name, gint pixels){
	if (!icon_theme) init();
	GdkPixbuf *pixbuf = NULL;

	// Load the basic icontheme icon
	GError *error = NULL;
	auto theme_pixbuf = gtk_icon_theme_load_icon (icon_theme,
		      icon_name,
		      pixels, 
		      GTK_ICON_LOOKUP_FORCE_SIZE,  // GtkIconLookupFlags flags,
		      &error);
	if (error) {
	    DBG("getThemePixbuf: %s\n", error->message);
	    g_error_free(error);
	    return NULL;
	} else if (theme_pixbuf) {
	    // Release any reference to the icon theme.
	    pixbuf = gdk_pixbuf_copy(theme_pixbuf);
	    g_object_unref(theme_pixbuf);
	}

	return pixbuf;
    }

private:

    static GdkPixbuf *
    buildPixbuf(const gchar *iconName, gint pixels){
	TRACE("buildPixbuf(): Create pixbuf and put in hashtable: \"%s\" (%d)\n", iconName, pixels);

	// check for composite icon definition or plain icon.
	auto isComposite = is_composite_icon_name(iconName);

	GdkPixbuf *pixbuf;
	if (isComposite){
	    pixbuf = composite_icon(iconName, pixels);
	} else {
	    pixbuf = getThemePixbuf(iconName, pixels);
	}
        if (pixbuf) return pixbuf;

	// Pixbuf not available. Fallback pixbuf: 
	//    "default" belongs to xffm+ package.	
	if (strcmp(iconName, "default") != 0){ // Avoid endless recurrence.
            DBG("Pixbuf::buildPixbuf(): missing icon: \"%s\"\n", iconName);
	    pixbuf = buildPixbuf("default", pixels);
	}
	// "default" should always work.
	if (!pixbuf){
	    ERROR("Unable to load missing icon %s or default\n",iconName);
	}
	return pixbuf;	
    }
public:

    static gboolean
    is_composite_icon_name(const gchar *icon_name){
	if (!icon_theme) init();
	const gchar *placements[]={
	    "/NW/", "/N/","/NE/",
	    "/W/",  "/C/","/E/",
	    "/SW/", "/S/","/SE/",
	    "*","#",NULL};
	auto p = placements;
	for (;p && *p; p++){
	    if (strstr(icon_name, *p)){
		TRACE("composite icon: %s\n", icon_name);
		return TRUE;
	    }
	}
	return FALSE;
    }
    
    static gboolean iconThemeHasIcon(const gchar *icon_name){
	if (!icon_name) return FALSE;
	if (!icon_theme) init();
        return gtk_icon_theme_has_icon (icon_theme,icon_name);
    }

    static void *
    pixbuf_new_from_file_f(void *data){
	if (!icon_theme) init();
	auto arg = (void **)data;
	auto path = (gchar *)arg[0];
	auto width = GPOINTER_TO_INT(arg[1]);
	auto height = GPOINTER_TO_INT(arg[2]);
	TRACE("pixbuf_new_from_file_f: %s (%d,%d)\n", path, width, height);
	GError *error = NULL;
	GdkPixbuf *pixbuf = NULL;
	if (width < 0) {
	    pixbuf = gdk_pixbuf_new_from_file (path, &error);
	} else {
	    pixbuf = gdk_pixbuf_new_from_file_at_size (path, width, height, &error);
	}
	if (strchr(path, '.')){
	    auto label = strrchr(path,'.')+1;
	    if (strlen(label)) insertPixbufLabel(pixbuf, label);
	}
        if (error){
            TRACE("%s\n", error->message);
            g_error_free(error);
	    pixbuf = NULL;
        }

	// hmmm... from the scale_simple line below, it seems that the above two
	//         functions will do a g_object_ref on the returned pixbuf...

	// Gdkpixbuf Bug workaround 
	// (still necessary in GTK-3.8, not checked further down the road)
	// xpm icons not resized. Need the extra scale_simple. 
	if (pixbuf && width > 0 && strstr(path, ".xpm")) {
	    auto pix = gdk_pixbuf_scale_simple (pixbuf, width, height, GDK_INTERP_HYPER);
	    g_object_unref(pixbuf);
	    pixbuf = pix;

	}      
	return pixbuf;
    }

    static GdkPixbuf *
    pixbuf_new_from_file (const gchar *path, gint width, gint height){
	if (!icon_theme) init();
	if (!path) return NULL;
	if (!g_file_test(path, G_FILE_TEST_EXISTS)) return NULL;
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
	    while (gtk_thread_wants_lock) Util<Type>::threadwait();
	}
	pthread_mutex_lock(&pixbuf_mutex);

	//  g_warning("pthread_mutex_trylock(&pixbuf_mutex) on gtk thread failed for %s\n",
	
	auto pixbuf = (GdkPixbuf *)pixbuf_new_from_file_f((void *)arg);
	pthread_mutex_unlock(&pixbuf_mutex);
	if (self == g_thread_self()) gtk_thread_wants_lock = FALSE;

#else
	// This sends everything to the gtk thread... (slow)
	    pixbuf = (GdkPixbuf *)utility_p->context_function(pixbuf_new_from_file_f, (void *)arg);
#endif

	return pixbuf;
    }


    static GdkPixbuf *
    composite_icon(const gchar *icon_name, gint pixels){
	if (!icon_theme) init();
	// Preload and composite elements into pixbuf hash.
	
	if (!strchr(icon_name, '*') && !strchr(icon_name, '#') && !strchr(icon_name, '/')){
	    return NULL;
	}
        TRACE("composite_icon(%s, %d)\n", icon_name,pixels); 
	auto name = g_strdup(icon_name);
	auto label = strchr(name, '*');
	auto color = strchr(name, '#');
	auto emblems = strchr(name, '/');
	if (label){
	    *label=0;
	    label++;
	}
	if (color) {
	    *color = 0;
	    color++;
	}
	if (emblems) {
	    *emblems = 0;
	    emblems++;
	}


	TRACE("***getting pixbuf for %s at size %d (%s, %s, %s)\n", name, pixels, label, color, emblems);
	auto pixbuf = getThemePixbuf(name, pixels);
	if (!pixbuf){
		ERROR("composite_icon() icon \"%s\" not found\n", name);
		pixbuf = getThemePixbuf("default", pixels);
	}




	// Now decorate the pixbuf with label, color and emblems, if any.
	if (label || color || emblems) {
	    void *arg[] = {NULL, (void *)pixbuf, (void *)label, (void *)color, (void *)emblems };
	    // Done by main gtk thread:
	    Util<Type>::context_function(insert_decoration_f, arg);
	}
	

	return pixbuf;
    }
    static void *
    insert_decoration_f (void *data){
	if (!icon_theme) init();
	auto arg = (void **)data;
	//pixbuf_icons_c *pixbuf_icons_p =(pixbuf_icons_c *)arg[0];
	auto base_pixbuf = (GdkPixbuf *)arg[1];
	auto label = (const gchar *)arg[2];
	auto color = (const gchar *)arg[3];
	auto emblems = (const gchar *)arg[4];

	cairo_t   *pixbuf_context = Cairo<Type>::pixbuf_cairo_create(base_pixbuf);
	
	gdk_cairo_set_source_pixbuf(pixbuf_context, base_pixbuf,0,0);
	cairo_paint_with_alpha(pixbuf_context, 1.0);
	if (color){
	    Cairo<Type>::add_color_pixbuf(pixbuf_context, base_pixbuf, color);
	}

	if (emblems){
	    auto tokens = g_strsplit(emblems, "/", -1);
	    if (!tokens) return NULL;
	    auto p = tokens;
	    // format: [icon_name/position/scale/alpha]
	    gint i;
	    for (p=tokens; p && *p; p += 4){
		for (i=1; i<4; i++) if (*(p+i) == NULL) {
		    ERROR("*** composite_icon(): incorrect composite specification: %s\n %s\n",
			    emblems,
			    "*** (format: [[base_icon_name]/position/emblem_name/scale/alpha])");
		    g_strfreev(tokens);
		    return base_pixbuf;
		}
		gchar *position = p[0];
		gchar *emblem = p[1];
		gchar *scale = p[2];
		gchar *alpha = p[3];
                auto pixels = gdk_pixbuf_get_width(base_pixbuf);
		GdkPixbuf *tag = PixbufHash<Type>::find_in_pixbuf_hash (emblem, pixels);
		if (!tag) {
		    tag = getThemePixbuf(emblem, pixels);
		    putInHash(emblem, pixels, tag);
		} else {
		    TRACE("insert_decoration_f() found %s in icon hash.\n", emblem);
		}
		if (tag) {
		    Cairo<Type>::insert_pixbuf_tag (pixbuf_context, tag, base_pixbuf, position, scale, alpha);
		} else {
		    TRACE("insert_decoration_f(): Cannot get pixbuf for %s\n", emblem);
		}
	    }
	    g_strfreev(tokens);
	}
	if (label){
	    Cairo<Type>::add_label_pixbuf(pixbuf_context, base_pixbuf, label);
	}

	Cairo<Type>::pixbuf_cairo_destroy(pixbuf_context, base_pixbuf);
	return NULL;
    }

    static void
    insertPixbufLabel(GdkPixbuf *pixbuf, const gchar *label){
	// Done by main gtk thread:
	void *arg[] = {(void *)pixbuf, (void *)label};
	Util<Type>::context_function(insert_label_decoration_f, arg);
    }

    static void *
    insert_label_decoration_f (void *data){
	if (!icon_theme) init();
	auto arg = (void **)data;
	auto base_pixbuf = (GdkPixbuf *)arg[0];
	auto label = (const gchar *)arg[1];

	cairo_t   *pixbuf_context = Cairo<Type>::pixbuf_cairo_create(base_pixbuf);
	
	gdk_cairo_set_source_pixbuf(pixbuf_context, base_pixbuf,0,0);
	cairo_paint_with_alpha(pixbuf_context, 1.0);

	if (label){
	    Cairo<Type>::add_label_pixbuf(pixbuf_context, base_pixbuf, label);
	}

	Cairo<Type>::pixbuf_cairo_destroy(pixbuf_context, base_pixbuf);
	return NULL;
    }

};
}
///////////////////////////////////////////////////////////////////////

#endif
