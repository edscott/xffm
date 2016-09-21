#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <errno.h>
#include "pixbuf_icons_c.hpp"

#ifndef PREFIX
# warning "PREFIX not defined!"
#endif
static void *insert_label_f (void *);
static void *insert_pixbuf_tag_f (void *);

    
pixbuf_icons_c::pixbuf_icons_c(void){
    pixbuf_mutex = PTHREAD_MUTEX_INITIALIZER;
    icon_theme = gtk_icon_theme_get_default ();
    if (!icon_theme){
        DBG("cannot get default icon theme!\n");
        icon_theme = gtk_icon_theme_new();
        //throw 1;
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
}



void
pixbuf_icons_c::threadwait (void) {
    struct timespec thread_wait = {
        0, 100000000
    };
    nanosleep (&thread_wait, NULL);
}


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
    if (pixbuf) return pixbuf;
    return NULL;
}

GdkPixbuf *
pixbuf_icons_c::get_theme_pixbuf(const gchar *icon_name, gint size){
    GdkPixbuf *pixbuf = NULL;
    GError *error = NULL;
    gchar *name = g_strdup(icon_name);
    if (strchr(name, '*')) *strrchr(name, '*') = 0;
    GdkPixbuf *theme_pixbuf = gtk_icon_theme_load_icon (icon_theme,
                  name,
                  size, 
                  GTK_ICON_LOOKUP_FORCE_SIZE,  // GtkIconLookupFlags flags,
                  &error);
    g_free(name);
    if (error) {
        TRACE("pixbuf_icons_c::get_theme_pixbuf: %s\n", error->message);
        g_error_free(error);
    } else if (theme_pixbuf) {
        // Release any reference to the icon theme.
        pixbuf = gdk_pixbuf_copy(theme_pixbuf);
        g_object_unref(theme_pixbuf);
    }
    if (strchr(icon_name, '*')) {
        void *arg[] = {(void *)this, (void *)pixbuf, (void *)(strrchr(icon_name, '*')+1)};
        // Done by main gtk thread:
        context_function(insert_label_f, arg);
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
    // (still necessary in GTK-3.8, not checked further down the road)
    // xpm icons not resized. Need the extra scale_simple. 
    if (pixbuf && width > 0 && strstr(path, ".xpm")) {
	GdkPixbuf *pix = gdk_pixbuf_scale_simple (pixbuf, width, height, GDK_INTERP_HYPER);
	g_object_unref(pixbuf);
	pixbuf = pix;

    }      
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
    context_function(insert_pixbuf_tag_f, arg);
    return TRUE;
}


//////////////////////////////////////////////////////////////////////////////////////////////


static void
add_label_pixbuf(pixbuf_icons_c *pixbuf_icons_p, cairo_t   *pixbuf_context, GdkPixbuf *pixbuf, const gchar *icon_text){
    // Insert text into pixbuf
    gint x = 0;
    gint y = 0;
    const gchar *text_size = "small";
    /*switch(size) {
	case SMALL_ICON_SIZE: text_size="xx-small"; break;
	case MEDIUM_ICON_SIZE: text_size="x-small"; break;
	case BIG_ICON_SIZE: text_size="medium"; break;
    }	
    if (!text_size) return ;*/

    GdkPixbuf   *t_pixbuf = NULL;
    //const gchar *color = "white";
    // FIXME: USE different colors for C, H files.
    const gchar *color = "yellow";
    gchar *layout_text = g_strdup_printf("<span foreground=\"black\" background=\"%s\" size=\"%s\">%s </span>", color, text_size, _(icon_text));

    PangoContext *context = gdk_pango_context_get_for_screen (gdk_screen_get_default());

    PangoLayout *layout = pango_layout_new (context);

    //PangoLayout *layout = 
//	gtk_widget_create_pango_layout (rfm_global_p->window, NULL);


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

    t_pixbuf =  
	gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, logical_rect.width+2, logical_rect.height+2);
    cairo_t   *t_pixbuf_context = pixbuf_icons_p->pixbuf_cairo_create(t_pixbuf);

    cairo_rectangle(t_pixbuf_context, 0, 0, logical_rect.width+2, logical_rect.height+2);
    cairo_clip(t_pixbuf_context);


    cairo_move_to (t_pixbuf_context, 1, 1);
	
    cairo_set_source_rgba(t_pixbuf_context, 0, 0, 0, 1.0);
    if (PANGO_IS_LAYOUT (layout)) {
        pango_cairo_show_layout (t_pixbuf_context, layout);
        g_object_unref(layout);
        g_object_unref(context);
    }
    pixbuf_icons_p->pixbuf_cairo_destroy(t_pixbuf_context, t_pixbuf);

    if (t_pixbuf) {
	gdk_cairo_set_source_pixbuf(pixbuf_context, t_pixbuf,x,y);
	cairo_paint_with_alpha(pixbuf_context, 0.650);
	g_object_unref(t_pixbuf);
    }

    return ;
}

static void
add_color_pixbuf(pixbuf_icons_c *pixbuf_icons_p, cairo_t *pixbuf_context, GdkPixbuf *pixbuf, 
	guchar red, guchar green, guchar blue){
    GdkPixbuf *pixbuf_mask = pixbuf_icons_p->create_pixbuf_mask(pixbuf, red, green, blue);  
    gdk_cairo_set_source_pixbuf(pixbuf_context, pixbuf_mask, 0,0);
    cairo_paint_with_alpha(pixbuf_context, 0.450);
    g_object_unref(pixbuf_mask);
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

    //add_color_pixbuf(pixbuf_icons_p, pixbuf_context, composite_pixbuf, 0x88, 0x7f, 0xa3);
    //add_label_pixbuf(pixbuf_icons_p, pixbuf_context, composite_pixbuf, "FIXME");


    pixbuf_icons_p->pixbuf_cairo_destroy(pixbuf_context, composite_pixbuf);
    g_object_unref(tag_s);
    return NULL;
}


static void *
insert_label_f (void *data){
    void **arg = (void **)data;
    pixbuf_icons_c *pixbuf_icons_p =(pixbuf_icons_c *)arg[0];
    GdkPixbuf *composite_pixbuf = (GdkPixbuf *)arg[1];
    const gchar *text = (const gchar *)arg[2];

    cairo_t   *pixbuf_context = pixbuf_icons_p->pixbuf_cairo_create(composite_pixbuf);
    
    gdk_cairo_set_source_pixbuf(pixbuf_context, composite_pixbuf,0,0);
    cairo_paint_with_alpha(pixbuf_context, 1.0);

//    add_color_pixbuf(pixbuf_icons_p, pixbuf_context, composite_pixbuf, 0x88, 0x7f, 0xa3);
    add_label_pixbuf(pixbuf_icons_p, pixbuf_context, composite_pixbuf, text);

    pixbuf_icons_p->pixbuf_cairo_destroy(pixbuf_context, composite_pixbuf);
    return NULL;
}






////////////////////////////////////////////////////////////////////////////////////////


