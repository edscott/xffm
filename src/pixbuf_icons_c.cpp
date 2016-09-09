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
    gtk_icon_theme_add_resource_path (icon_theme,resource_path);

    // This works, but icons should be fixed size. 
    // Scalable vector graphics dont work, last time I checked...
    // Icons should be in the "symbolic" internal gtk name format (hack...)
    // This is mandatory for non icon-themed boxes (which is not the usual setup).
    gchar *path = g_build_filename(resource_path, "24x24", NULL);
    gtk_icon_theme_append_search_path (icon_theme, path);
    g_free(path);

    // xffm+ svg icons are at:
    path = g_build_filename(PREFIX, "share", "icons", "xffm+", "scalable", "stock", NULL);
    gtk_icon_theme_prepend_search_path (icon_theme, path);
    g_free(path);
    path = g_build_filename(PREFIX, "share", "icons", "xffm+", "scalable", "emblems", NULL);
    gtk_icon_theme_prepend_search_path (icon_theme, path);
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


