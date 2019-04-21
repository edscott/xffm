#ifndef XFICONS_HH
#define XFICONS_HH
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "cairo.hh"
#include "hash.hh"

#ifndef PREFIX
# warning "PREFIX not defined: defaulting to /usr/local"
#define PREFIX "/usr/local"
#endif

static GThread *self;
static GtkIconTheme *icon_theme=NULL;
static pthread_mutex_t pixbuf_mutex;
    
namespace xf
{
    

template <class Type>
class Icons {
    typedef Util<Type> util_c;

    typedef Cairo<Type> pixbuf_cairo_c;
    typedef Hash<Type> pixbuf_hash_c;
public:
    static void
    init(void){
	TRACE("Calling Icons initializer...\n");
	//pixbuf_hash_c::init();
	pixbuf_mutex = PTHREAD_MUTEX_INITIALIZER;
	icon_theme = gtk_icon_theme_get_default ();
	if (!icon_theme){
	    ERROR("cannot get default icon theme!\n");
	    icon_theme = gtk_icon_theme_new();
	    //throw 1;
	}

	self = g_thread_self();
	auto resource_path = g_build_filename(PREFIX, "share", "icons", "xffm+", NULL);
	gtk_icon_theme_add_resource_path (icon_theme,resource_path);

	// This works, but icons should be fixed size. 
	// Scalable vector graphics dont work, last time I checked...
	// Icons should be in the "symbolic" internal gtk name format (hack...)
	// This is mandatory for non icon-themed boxes (which is not the usual setup).
        const gchar *subdirs[] = {
            "scalable",
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
	    
    static GdkPixbuf *
    absolute_path_icon(const gchar *icon_name, gint size){
	if (!icon_theme) init();
	if (!g_path_is_absolute (icon_name)) return NULL;
	if (!g_file_test (icon_name, G_FILE_TEST_EXISTS)) {
	    g_warning("*** %s does not exist.\n", icon_name);
	    return NULL;
	}
	auto pixbuf = pixbuf_new_from_file(icon_name, size, size); // width,height.
	if (pixbuf) return pixbuf;
	return NULL;
    }

    static gboolean iconThemeHasIcon(const gchar *icon_name){
	if (!icon_name) return FALSE;
	if (!icon_theme) init();
        return gtk_icon_theme_has_icon (icon_theme,icon_name);
    }

    static GdkPixbuf *
    get_theme_pixbuf(const gchar *icon_name, gint size){
	if (!icon_theme) init();
	GdkPixbuf *pixbuf = NULL;

	// Load the basic icontheme or backup icon
	GError *error = NULL;
	auto theme_pixbuf = gtk_icon_theme_load_icon (icon_theme,
		      icon_name,
		      size, 
		      GTK_ICON_LOOKUP_FORCE_SIZE,  // GtkIconLookupFlags flags,
		      &error);
	if (error) {
	    ERROR("icons.hh:get_theme_pixbuf: %s\n", error->message);
	    g_error_free(error);
	    return NULL;
	} else if (theme_pixbuf) {
	    // Release any reference to the icon theme.
	    pixbuf = gdk_pixbuf_copy(theme_pixbuf);
	    g_object_unref(theme_pixbuf);
	}

	return pixbuf;
    }

    static void *
    pixbuf_new_from_file_f(void *data){
	if (!icon_theme) init();
	auto arg = (void **)data;
	auto path = (gchar *)arg[0];
	auto width = GPOINTER_TO_INT(arg[1]);
	auto height = GPOINTER_TO_INT(arg[2]);
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
	    while (gtk_thread_wants_lock) util_c::threadwait();
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
    composite_icon(const gchar *icon_name, gint size){
	if (!icon_theme) init();
	// Preload and composite elements into pixbuf hash.
	
	if (!strchr(icon_name, '*') && !strchr(icon_name, '#') && !strchr(icon_name, '/')){
	    return NULL;
	}

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


	TRACE("***getting pixbuf for %s at size %d (%s, %s, %s)\n", name, size, label, color, emblems);
	auto pixbuf = get_theme_pixbuf(name, size);


	// Now decorate the pixbuf with label, color and emblems, if any.
	if (label || color || emblems) {
	    void *arg[] = {NULL, (void *)pixbuf, (void *)label, (void *)color, (void *)emblems };
	    // Done by main gtk thread:
	    util_c::context_function(insert_decoration_f, arg);
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

	cairo_t   *pixbuf_context = pixbuf_cairo_c::pixbuf_cairo_create(base_pixbuf);
	
	gdk_cairo_set_source_pixbuf(pixbuf_context, base_pixbuf,0,0);
	cairo_paint_with_alpha(pixbuf_context, 1.0);
	if (color){
	    pixbuf_cairo_c::add_color_pixbuf(pixbuf_context, base_pixbuf, color);
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
		GdkPixbuf *tag = pixbuf_hash_c::find_in_pixbuf_hash (emblem, 48);
		if (!tag) tag = get_theme_pixbuf(emblem, 48);
		if (tag) {
		    pixbuf_cairo_c::insert_pixbuf_tag (pixbuf_context, tag, base_pixbuf, position, scale, alpha);
		} else {
		    ERROR("insert_decoration_f(): Cannot get pixbuf for %s\n", emblem);
		}
	    }
	    g_strfreev(tokens);
	}
	if (label){
	    pixbuf_cairo_c::add_label_pixbuf(pixbuf_context, base_pixbuf, label);
	}

	pixbuf_cairo_c::pixbuf_cairo_destroy(pixbuf_context, base_pixbuf);
	return NULL;
    }

};
}


////////////////////////////////////////////////////////////////////////////////////////




#endif
