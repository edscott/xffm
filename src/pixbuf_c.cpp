/*
 * Copyright (C) 2002-2012 Edscott Wilson Garcia
 * EMail: edscott@users.sf.net
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; 
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "rfm.h"
#include "rfm_modules.h"

#include "primary-pixbuf-cairo.i"
#include "primary-icons-hash.i"
#include "primary-icons.i"

static void
add_theme_list(GSList **list_p, const gchar *path){
    if (!g_file_test(path, G_FILE_TEST_IS_DIR)) return;
    DIR *directory = opendir(path);
    if (!directory) return;
    struct dirent *d;
    
    while ((d=readdir(directory)) != NULL){
	gboolean unknown=TRUE;
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
	if (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN) continue;
	if (d->d_type == DT_DIR) unknown = FALSE;
#endif
	if (unknown) {
	    gchar *file =  g_strconcat(path, G_DIR_SEPARATOR_S, d->d_name, NULL);
	    struct stat st;
	    if (stat(file,&st) < 0){
		g_free(file);
		continue;
	    }
	    g_free(file);
	    if (!S_ISDIR(st.st_mode)) continue;
	}
	gchar *test_file = g_strconcat(path, G_DIR_SEPARATOR_S, d->d_name, G_DIR_SEPARATOR_S,"index.theme", NULL);
	if (g_file_test(test_file, G_FILE_TEST_EXISTS)){
	    NOOP(stderr, "%s exists...\n", test_file);
	    *list_p = g_slist_append(*list_p, g_strdup(d->d_name));
	}
	g_free(test_file);
    }
    closedir (directory);


}


static gchar **extend_icon_theme_path(gchar **pathv, int *l){
    gchar * s_p[]={"/usr/share/icons", "/usr/local/share/icons", "/usr/share/pixmaps", "usr/local/share/pixmaps", NULL};

    gint length = 0;
    gchar **p;
    for (p=pathv;p && *p;p++) {
        length++;
    }

    gchar **q;
    for(q = s_p;q && *q; q++){
        gboolean found = FALSE;
        for (p=pathv;p && *p;p++) {
            if (strcmp(*q, *p)==0) {
                found = TRUE;
                break;
            }
        }
        if (!found) length++;
    }

    gchar **new_pathv = (gchar **)malloc((length+1)*sizeof(gchar *));
    if (!new_pathv) g_error("malloc: %s\n", strerror(errno));
    gchar **r = new_pathv;
    memset(r, 0, (length+1)*sizeof(gchar *));

    for (p=pathv;p && *p;p++,r++) *r = g_strdup(*p);
        
    for(q = s_p;q && *q; q++){
        gboolean found = FALSE;
        for (p=pathv;p && *p;p++) {
            if (strcmp(*q, *p)==0) {
                found = TRUE;
                break;
            }
        }
        if (!found) {
             *r = g_strdup(*q);
             r++;
        }
    }
    *l = length; 
    return new_pathv;
}

gchar **rfm_get_iconthemes(void){
    gchar **pathv=NULL;
    GtkIconTheme *icon_theme = gtk_icon_theme_get_default ();
    gtk_icon_theme_get_search_path (icon_theme, &pathv, NULL);

    gint length;
    gchar **epathv = extend_icon_theme_path(pathv, &length);
    g_strfreev(pathv);

    
    gtk_icon_theme_set_search_path(icon_theme, (const gchar **)epathv, length);
    gtk_icon_theme_get_search_path (icon_theme, &pathv, NULL);
    g_strfreev(epathv);
    
    GSList *list=NULL;
    gchar **p;
    for (p=pathv;p && *p;p++){
	DBG("icontheme path=%s\n", *p);
	if (g_file_test(*p, G_FILE_TEST_EXISTS)) add_theme_list(&list, *p);
    }
    gchar **th_options = (gchar **)malloc((g_slist_length(list)+1)*sizeof(gchar *));
    if (!th_options) g_error("malloc: %s\n", strerror(errno));
    memset(th_options, 0, (g_slist_length(list)+1)*sizeof(gchar *));
    GSList *tmp=list;
    gint j;
    for (j=0; tmp && tmp->data ; tmp=tmp->next){
	DBG("theme: %s\n", (gchar *)tmp->data);
	th_options[j++] = tmp->data;
    }
    g_slist_free(list);
    g_strfreev(pathv);
    return th_options;
}
 
Pixmap
rfm_create_background_pixmap (char *file) {
    /* create Pixmap */

    gint root_x;
    gint root_y;
    gint root_w;
    gint root_h;
    gint root_d;

      Display 	*Xdisplay;
      Drawable 	root_Xwindow;
      Visual 	*Xvisual;
    rfm_global_t *rfm_global_p = rfm_global();
    if (rfm_global_p) {
	Xdisplay = rfm_global_p->Xdisplay;
	root_Xwindow = rfm_global_p->root_Xwindow;
	Xvisual = rfm_global_p->Xvisual;
    } else {
	root_Xwindow = gdk_x11_get_default_root_xwindow ();
	Xdisplay = gdk_x11_display_get_xdisplay(gdk_display_get_default());
	Xvisual = gdk_x11_visual_get_xvisual(gdk_visual_get_system());
	
    }

    rfm_get_drawable_geometry(root_Xwindow,
	    &root_x, &root_y, &root_w, &root_h, &root_d);
    Pixmap xpixmap = XCreatePixmap (Xdisplay, GDK_ROOT_WINDOW (), root_w, root_h, root_d);

    // The X way
    // create a graphic context for the pixmap
    GC graphic_context = XCreateGC(Xdisplay, xpixmap, 0, 0);;
    // get default colormap
    static Colormap colormap = None;
    if (colormap == None){
	colormap = DefaultColormap(Xdisplay, 0);
    }
    
    // parse the color specification
    XColor background_color;
    const gchar *background=NULL;
    if(getenv ("RFM_DESKTOP_COLOR") && strlen (getenv ("RFM_DESKTOP_COLOR"))) {
	background=getenv ("RFM_DESKTOP_COLOR");
        if (!XParseColor(Xdisplay, 
	    colormap, background, &background_color)) {
	    DBG("cannot parse background color: %s\n", background);
	    background=NULL;
	}
    }
    if (!background) {
	background="#000000";
        if (!XParseColor(Xdisplay, 
	    colormap, background, &background_color)) {
	    g_error("cannot parse default background color: %s", background);
	}
    }
    XAllocColor(Xdisplay, colormap, &background_color);
    XSetForeground(Xdisplay, graphic_context, background_color.pixel);
    XFillRectangle (Xdisplay, xpixmap, graphic_context, 0, 0, root_w, root_h);


// cairo way
#ifndef CAIRO_HAS_XLIB_SURFACE
 DBG ("CAIRO_HAS_XLIB_SURFACE is not defined!\n"); 
#else
    GdkPixbuf *pixbuf = NULL;
    if (file) {
	pixbuf = rfm_create_background_pixbuf (file, root_w, root_h);
	if(!GDK_IS_PIXBUF(pixbuf)) {
	    DBG("cannot create pixbuf from %s\n", file);
	}
    }

    if (pixbuf && GDK_IS_PIXBUF(pixbuf)) {
	cairo_surface_t *pixmap_surface = 
	    cairo_xlib_surface_create (
		    Xdisplay, // Display *
		    xpixmap, // Drawable 
		    Xvisual, // Visual *
		    root_w, root_h);
	if(cairo_surface_status (pixmap_surface) != CAIRO_STATUS_SUCCESS) {
	    g_error ("cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS");
	}
	cairo_surface_reference(pixmap_surface);
	cairo_t *pixmap_context = cairo_create (pixmap_surface);
	cairo_reference(pixmap_context);

	gdk_cairo_set_source_pixbuf(pixmap_context, pixbuf,
		       (root_w - gdk_pixbuf_get_width (pixbuf)) / 2, 
		       (root_h - gdk_pixbuf_get_height (pixbuf)) / 2);
	cairo_paint (pixmap_context); 
	cairo_surface_destroy(pixmap_surface);
	cairo_destroy(pixmap_context);
	g_object_unref(pixbuf);
    }
#endif
    XFreeGC(Xdisplay, graphic_context);
    return xpixmap;
}
/******************************************************************/

gboolean
rfm_is_dark_background(view_t *view_p){
    gboolean is_dark = FALSE; // default is light
    // Get background color
    const gchar *bkg_var=(view_p->flags.type == DESKVIEW_TYPE)?
	"RFM_DESKTOP_COLOR": "RFM_ICONVIEW_COLOR";
    // figure out if light or dark background to use
    // dark or light color set
    if(getenv (bkg_var) && strlen (getenv (bkg_var))) {
#if GTK_MAJOR_VERSION==3
        GdkRGBA gdk_color;
	if (!gdk_rgba_parse (&gdk_color, getenv (bkg_var))){
	    DBG("cannot parse background color %s", getenv (bkg_var));
	} else {
	    gint sum = (gdk_color.red) +
		      (gdk_color.green) +
		      (gdk_color.blue);
	    is_dark = (sum < 1.5);
	}
#else
        GdkColor gdk_color;
	if (!gdk_color_parse (getenv (bkg_var), &gdk_color)){
	    DBG("cannot parse background color %s", getenv (bkg_var));
	} else {
	    gint sum = (gdk_color.red) +
		      (gdk_color.green) +
		      (gdk_color.blue);
	    is_dark = (sum < 0xffff * 3 / 2);
	}
#endif
    } 
    return is_dark;
}

#define NUM_COLORS 8
#if GTK_MAJOR_VERSION==3
GdkRGBA *
rfm_get_gdk_color (view_t *view_p, int p) {
    if(p < 0 || p >= NUM_COLORS) {
        DBG ("rodent_select_pen: pen %d out of range.\n", p);
        return NULL;
    }
    GdkRGBA pen_colors[] = {
/* light background */
        {1.0, 1.0, 1.0, 1.0},       /*white */
        { 0.0, 0.0, 0.0, 1.0},           /*black */
        { 1.0, 0.0, 0.0, 1.0},       /*red */
        { 0.0, 0.4577, 0.0, 1.0},       /*green */
        { 0.0, 0.0, 0.4882, 1.0},       /*blue */
        { 0.4882, 0.4882, 0.0, 1.0},   /*brown */
        { 1.0, 0.0, 1.0, 1.0},   /*magenta */
        { 0.7324, 0.7324, 0.7324, 1.0},       /*grey */
/* dark background */
        { 0.0, 0.0, 0.0, 1.0},           /*black */
        { 1.0, 1.0, 1.0, 1.0},       /*white */
        { 1.0, 0.3052, 0.3052, 1.0},      /*red */
        { 0.3052, 0.9155, 0.3052, 1.0},      /*green */
        { 0.6104, 0.6104, 1.0, 1.0},      /*blue */
        { 0.4882, 0.4882, 0.0, 1.0},  /*brown */
        { 1.0, 0.0, 1.0, 1.0},  /*magenta */
        { 0.7324, 0.7324, 0.7324, 1.0}       /*grey */
    };

    GdkRGBA *gdk_color = (GdkRGBA *) malloc(sizeof(GdkRGBA));
    if (!gdk_color) return NULL;
    memset(gdk_color, 0, sizeof(GdkRGBA));
        
    // if background color requested, return now 
    const gchar *bkg_var=(view_p->flags.type == DESKVIEW_TYPE)?
	"RFM_DESKTOP_COLOR": "RFM_ICONVIEW_COLOR";
    const gchar *bg_color = getenv (bkg_var);
    if (p == BACKGROUND_COLOR && bg_color && strlen (bg_color)) {
	if (!gdk_rgba_parse (gdk_color, bg_color)){
	    DBG("cannot parse background color %s\n", bg_color);
	} else {
	    NOOP ("DESK: selecting BACKGROUND_COLOR %s\n", bg_color);
	    return gdk_color;
	}
    }

    gint offset = (rfm_is_dark_background(view_p))? NUM_COLORS : 0;
    // return color entry
    memcpy(gdk_color,  pen_colors + p + offset, sizeof(GdkRGBA));
    return gdk_color;
}

#else
GdkColor *
rfm_get_gdk_color (view_t *view_p, int p) {
    if(p < 0 || p >= NUM_COLORS) {
        DBG ("rodent_select_pen: pen %d out of range.\n", p);
        return NULL;
    }
    GdkColor pen_colors[] = {
/* light background */
        {0, 65535, 65535, 65535},       /*white */
        {1, 0, 0, 0},           /*black */
        {2, 65535, 0, 0},       /*red */
        {3, 0, 30000, 0},       /*green */
        {4, 0, 0, 32000},       /*blue */
        {5, 32000, 32000, 0},   /*brown */
        {6, 65535, 0, 65535},   /*magenta */
        {7, 48000, 48000, 48000},       /*grey */
/* dark background */
        {1, 0, 0, 0},           /*black */
        {0, 65535, 65535, 65535},       /*white */
        {12, 65535, 20000, 20000},      /*red */
        {13, 20000, 60000, 20000},      /*green */
        {14, 40000, 40000, 65535},      /*blue */
        {15, 32000, 32000, 0},  /*brown */
        {16, 65535, 0, 65535},  /*magenta */
        {17, 48000, 48000, 48000}       /*grey */
    };

    GdkColor *gdk_color = (GdkColor *) malloc(sizeof(GdkColor));
    memset(gdk_color, 0, sizeof(GdkColor));
        
    // if background color requested, return now 
    const gchar *bkg_var=(view_p->flags.type == DESKVIEW_TYPE)?
	"RFM_DESKTOP_COLOR": "RFM_ICONVIEW_COLOR";
    if (p == BACKGROUND_COLOR && getenv (bkg_var) && strlen (getenv (bkg_var))) {
	if (!gdk_color_parse (getenv (bkg_var), gdk_color)){
	    DBG("cannot parse background color %s\n", getenv (bkg_var));
	} else {
	    NOOP ("DESK: selecting BACKGROUND_COLOR %s\n", getenv (bkg_var));
	    return gdk_color;
	}
    }

    gint offset = (rfm_is_dark_background(view_p))? NUM_COLORS : 0;
    // return color entry
    memcpy(gdk_color,  pen_colors + p + offset, sizeof(GdkColor));
    return gdk_color;
}
#endif

GdkPixbuf *
rfm_create_background_pixbuf (const char *file, int width, int height) {

      Drawable root_Xwindow;
      //Display *Xdisplay;
      //Visual *Xvisual;
    rfm_global_t *rfm_global_p = rfm_global();
    if (rfm_global_p) {
	root_Xwindow = rfm_global_p->root_Xwindow;
	//Xdisplay = rfm_global_p->Xdisplay;
	//Xvisual = rfm_global_p->Xvisual;
    } else {
	root_Xwindow = gdk_x11_get_default_root_xwindow ();
	//Xdisplay = gdk_x11_display_get_xdisplay(gdk_display_get_default());
	//Xvisual = gdk_x11_visual_get_xvisual(gdk_visual_get_system());
	
    }
    if (width < 0 || height < 0) {
        rfm_get_drawable_geometry (root_Xwindow,
	    NULL, NULL, &width, &height, NULL);
    }
  
    GdkPixbuf *tgt;
    gdouble factor, x_factor, y_factor;
    gint w, h;
    if(!file) return NULL;
    NOOP ("getting pixbuf %s", file);
    GdkPixbuf *src = NULL;

    if (g_file_test(file, G_FILE_TEST_EXISTS)){
      src = rfm_pixbuf_new_from_file(file, -1, -1);
      if(!src) return NULL;
    }

    if(!src) return NULL;

    w = gdk_pixbuf_get_width (src);
    h = gdk_pixbuf_get_height (src);
    NOOP (stderr, " pixbuf is %d,%d width,height is %d,%d\n", w, h, width, height);
    x_factor = (double)width / w;
    y_factor = (double)height / h;
    factor = (x_factor < y_factor) ? x_factor : y_factor;
    NOOP (stderr, "factors %lf,%lf --> %lf, distorsion=%lf\n", x_factor, y_factor, factor, fabs (x_factor - y_factor) / factor);
    if(fabs (x_factor - y_factor) / factor < 0.20) {
    //if (1){
	NOOP (stderr, "scaling pixbuf %s to %d,%d\n", file, width, height);
	tgt = rfm_pixbuf_scale_stretch(src, width, height, GDK_INTERP_BILINEAR);
    } else {
	NOOP (stderr, "factor=%lf scaling pixbuf %s to %lf,%lf\n", factor, file, factor * w, factor * h);
	if (x_factor > y_factor) {
	    tgt = rfm_pixbuf_scale_stretch(src, 1.2*factor * w, factor * h,
		    GDK_INTERP_BILINEAR);
	} else {
	    tgt = rfm_pixbuf_scale_stretch(src, factor * w, 1.2*factor * h,
		    GDK_INTERP_BILINEAR);
	}
    }
    g_object_unref (G_OBJECT (src));
    NOOP ("got pixbuf %s\n", file);
    return tgt;
}


gchar *
rfm_get_thumbnail_path (const gchar * file, gint size) {
    gchar *cache_dir;
    gchar *thumbnail_path = NULL;
    GString *gs;
    gchar key[11];

    cache_dir = g_build_filename (RFM_THUMBNAIL_DIR, NULL);
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
/*
static void *
    replace_pixbuf_hash(void *data){
    GHashTable *new_pixbuf_hash = data;
    GHashTable *old_pixbuf_hash = pixbuf_hash;
    pixbuf_hash = new_pixbuf_hash;
    g_hash_table_destroy(old_pixbuf_hash);
    return NULL;
}
*/
/*
static gboolean
pixbuf_scale(GdkPixbuf *in_pixbuf, pixbuf_t *pixbuf_p){
    if (!in_pixbuf || !pixbuf_p || !pixbuf_p->pixbuf) return FALSE;
    gint width = gdk_pixbuf_get_width(pixbuf_p->pixbuf);
    gint height = gdk_pixbuf_get_height(pixbuf_p->pixbuf);
    gdk_pixbuf_scale  (in_pixbuf, pixbuf_p->pixbuf,
			    0, 0,
                          width, height, 
			  0, 0, 
			  1.0, 1.0, 
			  GDK_INTERP_NEAREST);
    g_object_unref(in_pixbuf);
    return FALSE;
}
*/
/*
static void
update_pixbuf_f (gpointer key, gpointer value, gpointer data){
    pixbuf_t *pixbuf_p = value;
    if (g_path_is_absolute(pixbuf_p->path)){
	// don't touch thumbnails.
	return;
    }
    gboolean replace_pixbuf = TRUE;
    GdkPixbuf *pixbuf =
	get_pixbuf(pixbuf_p->mime_id, pixbuf_p->size, replace_pixbuf);
    if (pixbuf && GDK_IS_PIXBUF(pixbuf)) {
	NOOP("updating pixbuf: %s (%d)\n", pixbuf_p->mime_id, pixbuf_p->size);
        pixbuf_scale(pixbuf, pixbuf_p);
	// in function... g_object_unref(pixbuf);
    }
}
*/
/*
static void *
rfm_change_icontheme_f(void *data){
    NOOP(stderr, "-----> rfm_change_icontheme_f\n");
    if (rfm_get_gtk_thread() != g_thread_self()){
	DBG("update_pixbuf_f(): rfm_get_gtk_thread() != g_thread_self()\n");
        return NULL;
    }
    g_hash_table_foreach (pixbuf_hash, update_pixbuf_f, NULL);
    return NULL;
}
*/
static void *
change_pixbuf_hash(void *data){
    GHashTable *old_pixbuf_hash = pixbuf_hash;
    pixbuf_hash = g_hash_table_new_full (g_str_hash, g_str_equal,g_free, free_pixbuf_t);
    g_hash_table_destroy(old_pixbuf_hash);
    return NULL;
}
#if 0
void
rfm_change_icontheme(void){
    if(!pixbuf_hash) return;
	
    if (rfm_get_gtk_thread()==g_thread_self()){
	rfm_change_icontheme_f(NULL);
    } else {
	rfm_context_function(rfm_change_icontheme_f, NULL);
    }
}
#else
// XXX here's the rub... We are using gtk icon theme to get icon search path,
//     we should not do this when icontheme not selected, but we should not alter 
//     the gtk search path either. (Dont use gtk search path when no icontheme selected...)
void
rfm_change_icontheme(void){
    widgets_t *widgets_p = rfm_get_widget("widgets_p");
    const gchar *icontheme_name = getenv("RFM_USE_GTK_ICON_THEME");
    if (icontheme_name && strlen(icontheme_name)) {
	GtkSettings *settings = gtk_settings_get_default();
	g_object_set( G_OBJECT(settings), 
		"gtk-icon-theme-name", icontheme_name,
		NULL);
    }

    gchar *text = g_strdup_printf("%s %s (%s)\n",
	    _("Icon theme:"), (icontheme_name && strlen(icontheme_name))?icontheme_name:_("None"),
	    _("Note: Some changes will not take effect until restart"));

    if (rfm_get_gtk_thread() != g_thread_self()){
	rfm_threaded_show_text(widgets_p);
	rfm_threaded_diagnostics(widgets_p, "xffm/stock_dialog-warning", NULL);
	rfm_threaded_diagnostics(widgets_p, "xffm_tag/red", text);
    } else {
	rfm_show_text(widgets_p);
	rfm_diagnostics(widgets_p, "xffm/stock_dialog-warning", NULL);
	rfm_diagnostics(widgets_p, "xffm_tag/red", text, NULL);
	g_free(text);
    }
    // Recreate pixbuf hash.
    rfm_context_function(change_pixbuf_hash, NULL);
    //rfm_context_function(rfm_change_icontheme_f, NULL);
}

#endif

static gboolean
pixbuf_save_f(void *data){
    void **arg = data;
    GdkPixbuf *tgt = arg[0];
    gchar *path = arg[1];
    g_free(arg);
    gint status = 0;
    rfm_global_t *rfm_global_p = rfm_global();
    if (rfm_global_p){
	g_mutex_lock(rfm_global_p->status_mutex);
	status = rfm_global_p->status;
	g_mutex_unlock(rfm_global_p->status_mutex);
    }
    if (tgt && GDK_IS_PIXBUF(tgt) && status != STATUS_EXIT) {
	GError *error = NULL;
	gdk_pixbuf_save (tgt, path, "png", &error,
			 "tEXt::Software", "Rodent", NULL);
	if (error){
	    DBG("pixbuf_save_f(%s): %s\n", path, error->message);
	    g_error_free(error);
	}
    }
    g_free(path);
    return FALSE;
}

static void *
pixbuf_scale_simple_f(void *data){
    void **arg = data;
    GdkPixbuf *in_src = arg[0];
    gint width = GPOINTER_TO_INT(arg[1]);
    gint height = GPOINTER_TO_INT(arg[2]);
    gint type = GPOINTER_TO_INT(arg[3]);
    g_free(arg);
    return gdk_pixbuf_scale_simple (in_src, width, height, type);

    // XXX Where is this hack necessary?
#if 0
    GdkPixbuf *src = NULL;
    gint ph = gdk_pixbuf_get_height (in_src);
    gint pw = gdk_pixbuf_get_width (in_src);
	    

    if (ph > pw) {
	    gint w = height * pw / ph;
	    src = gdk_pixbuf_scale_simple (in_src, w, height, type);
    } else {
	    gint h = width * ph / pw;
	    src = gdk_pixbuf_scale_simple (in_src, width, h, type);
    }
    return src;
#endif
}


GdkPixbuf *
rfm_pixbuf_scale_simple (GdkPixbuf *in_pixbuf,  gint size, gint type){
    //g_object_ref(in_pixbuf); return (in_pixbuf);

    void **arg = (void **)malloc(4*sizeof(void *));
    if (!arg) g_error("malloc: %s\n", strerror(errno));
    arg[0] = in_pixbuf;
    arg[1] = GINT_TO_POINTER(size);
    arg[2] = GINT_TO_POINTER(size);
    arg[3] = GINT_TO_POINTER(type);
    GdkPixbuf *pixbuf;
    if (rfm_get_gtk_thread() != g_thread_self()){
	pixbuf = rfm_context_function(pixbuf_scale_simple_f, arg);
    } else {
	pixbuf = pixbuf_scale_simple_f(arg);
    }
    return pixbuf;
}

GdkPixbuf *
rfm_pixbuf_scale_stretch (GdkPixbuf *in_pixbuf,  gint width, gint height, gint type){
    void **arg = (void **)malloc(4*sizeof(void *));
    if (!arg) g_error("malloc: %s\n", strerror(errno));
    arg[0] = in_pixbuf;
    arg[1] = GINT_TO_POINTER(width);
    arg[2] = GINT_TO_POINTER(height);
    arg[3] = GINT_TO_POINTER(type);
    GdkPixbuf *pixbuf;
    if (rfm_get_gtk_thread() != g_thread_self()){
	pixbuf = rfm_context_function(pixbuf_scale_simple_f, arg);
    } else {
	pixbuf = pixbuf_scale_simple_f(arg);
    }
    return pixbuf;
}

	

void 
rfm_pixbuf_save(GdkPixbuf *tgt, const gchar *path){
    if (!tgt || !path || !GDK_IS_PIXBUF(tgt)) return ;
    void **arg =(void **)malloc(2*sizeof(void *));
    if (!arg) g_error("malloc: %s\n", strerror(errno));
    arg[0] = tgt;
    arg[1] = g_strdup(path);
    // Additional reference to pixbuf to be removed
    // by save function.
    if (rfm_get_gtk_thread() != g_thread_self()){
	// don't wait here
	g_main_context_invoke(NULL, pixbuf_save_f, arg);
    } else {
	pixbuf_save_f(arg);
    }
    return;
}


static void *
pixbuf_from_file_f(void *data){
    void **arg = data;
    gchar *path = arg[0];
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
    
    if(error && !strstr(path, ".cache/rfm/thumbnails")) {
	    DBG ("pixbuf_from_file() %s:%s\n", error->message, path);
	    g_error_free (error);
    }
    return pixbuf;
}

static pthread_mutex_t pixbuf_mutex = PTHREAD_MUTEX_INITIALIZER;

GdkPixbuf *
rfm_pixbuf_new_from_file (const gchar *path, gint width, gint height){
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
    if (rfm_get_gtk_thread() == g_thread_self()) {
        gtk_thread_wants_lock = TRUE;
    } else {
        // hold your horses...
        while (gtk_thread_wants_lock) rfm_threadwait();
    }
    pthread_mutex_lock(&pixbuf_mutex);

    //  g_warning("pthread_mutex_trylock(&pixbuf_mutex) on gtk thread failed for %s\n",
    
    pixbuf = pixbuf_from_file_f(arg);
    pthread_mutex_unlock(&pixbuf_mutex);
    if (rfm_get_gtk_thread() == g_thread_self()) gtk_thread_wants_lock = FALSE;

#else
    // This sends everything to the gtk thread...
	pixbuf = rfm_context_function(pixbuf_from_file_f, arg);
#endif

    return pixbuf;
}

#if 10
// Bug workaround in opensuse 12.3. 
// Something with their gdk/cairo construction
// is broken... See workaround below...
//
static void *
insert_pixbuf_tag_f (void *data){
    void **arg = data;
    const GdkPixbuf *tag = arg[0];
    GdkPixbuf *composite_pixbuf = arg[1];
    gchar *where = arg[2];
    double *scale_factor = arg[3];
    gint overall_alpha = GPOINTER_TO_INT(arg[4]);
    g_free(arg);

    gdouble scale_x = 1.0 / (*scale_factor);
    gdouble scale_y = 1.0 / (*scale_factor);
    gint width = gdk_pixbuf_get_width(tag);
    gint height = gdk_pixbuf_get_height(tag);

    GdkPixbuf *tag_s = gdk_pixbuf_scale_simple(tag, 
	    floor(scale_x*width), floor(scale_y*height),
	    GDK_INTERP_BILINEAR);   


    gint dest_width = gdk_pixbuf_get_width (composite_pixbuf);
    gint dest_height = gdk_pixbuf_get_height (composite_pixbuf);
    gint s_width = gdk_pixbuf_get_width (tag);
    gint s_height = gdk_pixbuf_get_height (tag);
    // "SW"
    //gint dest_x = 0;
    //gint dest_y = 0;
    s_width = ((gdouble) s_width) * scale_x;
    s_height = ((gdouble) s_height) * scale_y;

    // default SW
    gdouble offset_x = 0.0;
    gdouble offset_y = dest_height - s_height;

    if(strcmp (where, "SW") == 0) {
        offset_x = dest_width - s_width;
    } else if(strcmp (where, "SE") == 0) {
        offset_x = dest_width - s_width;
    } else if(strcmp (where, "NW") == 0) {
        offset_y = 0.0;
    } else if(strcmp (where, "NE") == 0) {
        offset_x = dest_width - s_width;
        offset_y = 0.0;
    } else if(strcmp (where, "C") == 0) {
        offset_x = (dest_width - s_width) / 2;
        offset_y = (dest_height - s_height) / 2;
    } else if(strcmp (where, "SC") == 0) {
        offset_x = (dest_width - s_width) / 2;
    } else if (strcmp (where, "NC") == 0) {
        offset_y = 0.0;
        offset_x = (dest_width - s_width) / 2;
    }



    cairo_t   *pixbuf_context = pixbuf_cairo_create(composite_pixbuf);
    
    // This proved necessary in opensuse-12.3,
    // but not in gentoo nor ubuntu. Go figure...
	gdk_cairo_set_source_pixbuf(pixbuf_context, composite_pixbuf,0,0);
	cairo_paint_with_alpha(pixbuf_context, 1.0);
	
    gdk_cairo_set_source_pixbuf(pixbuf_context, tag_s, offset_x,offset_y);
    cairo_paint_with_alpha(pixbuf_context, (double)overall_alpha/255.0);
    pixbuf_cairo_destroy(pixbuf_context, composite_pixbuf);
    g_object_unref(tag_s);

    g_free(scale_factor);
    g_free(where);
    return NULL;
}


#else
// This is the old method. Using cairo is a bit less bug prone:
// not really bug free. Does not work right in opensuse12.3
// without a slight workaround...
<snip>
}
#endif
gboolean
rfm_insert_pixbuf_tag (GdkPixbuf *tag, GdkPixbuf *composite_pixbuf,
	const gchar *where, gdouble scale_factor, gint overall_alpha){
    if (!tag || !composite_pixbuf || !GDK_IS_PIXBUF(composite_pixbuf) || !where) {
	return FALSE;
    }
    void **arg = (void **)malloc(5*sizeof(void *));
    if (!arg) g_error("malloc: %s\n", strerror(errno));
    arg[0] = tag;
    arg[1] = composite_pixbuf;
    arg[2] = g_strdup(where);
    arg[3] = (gdouble *)malloc(sizeof(gdouble));
    if (!arg[3]) g_error("malloc: %s\n", strerror(errno));
    memcpy(arg[3], &scale_factor, sizeof(gdouble));
    arg[4] = GINT_TO_POINTER(overall_alpha);
    
    if (rfm_get_gtk_thread() != g_thread_self()){
	rfm_context_function(insert_pixbuf_tag_f, arg);
    } else {
	insert_pixbuf_tag_f(arg);
    }
    return TRUE;
}


static void *
duplicate_pixbuf(void *data){
    GdkPixbuf *pixbuf = data;
    if (!pixbuf || !G_IS_OBJECT(pixbuf)) {
	return NULL;
    }
    GdkPixbuf *new_pixbuf = gdk_pixbuf_copy (pixbuf);
    return new_pixbuf;
}

GdkPixbuf *
rfm_pixbuf_duplicate(GdkPixbuf *pixbuf){
    if (!pixbuf) return NULL;
    GdkPixbuf *new_pixbuf = NULL;
    if (rfm_get_gtk_thread() != g_thread_self()){
	//g_mutex_lock (pixbuf_hash_mutex);
	new_pixbuf = rfm_context_function(duplicate_pixbuf, pixbuf);
    } else {
	//while (!g_mutex_trylock (pixbuf_hash_mutex)) gtk_main_iteration();
	new_pixbuf = duplicate_pixbuf(pixbuf);
    }
    //g_mutex_unlock (pixbuf_hash_mutex);
    return new_pixbuf;
}



////////////////   module wraparounds:
//

gint
rfm_svg_supported (void) {
    void *value = rfm_void (RFM_MODULE_DIR, "icons", "svg_supported");
    return GPOINTER_TO_INT (value);
}

///////////////////////  primary_icon_resolve.i   /////////////////////


static const gchar *
get_type_icon_id(record_entry_t *en){
    if(IS_BROKEN_LNK (en->type)) return ("xffm/stock_missing-image");
    if(IS_UP_TYPE (en->type)) {
	// When up item is not a directory.
        return "xffm/stock_go-up";
    } 
    if (IS_SDIR (en->type)){
	// No access directory_:
        if(IS_NOACCESS_TYPE (en->type)) {
	    return "xffm/stock_directory/compositeC/emblem_unreadable";
        } 
	// Invariant under write or no write flag:
	if(strcmp (en->path, g_get_home_dir ()) == 0) {
	    return "xffm/stock_home";
	} else if(strcmp (en->path, "/") == 0){
		return "xffm/stock_directory";
	} else if(
		(strstr (en->path, "cdrom") ||
		 strstr (en->path, "cdrw") ||
		 strstr (en->path, "dvd"))
		 &&  FSTAB_is_in_fstab(en->path))
	{
	    return "xffm/emblem_disk";
	} 
	else if(IS_NOWRITE_TYPE (en->type)) {
	    // No write directory
	    return "xffm/stock_directory";
        } else if(strcmp (en->path, "/") == 0){
		return "xffm/stock_directory";
	} else if (getenv("RFM_DESKTOP_DIR") && strcmp(en->path, getenv("RFM_DESKTOP_DIR"))==0){
		return "xffm/emblem_desktop";
	} else if(strcmp (en->path, "/") == 0){
		return "xffm/stock_directory";
	}
	    
	//write ok
	return "xffm/stock_directory/compositeSE/emblem_write-ok";
    } 
    if (IS_SCHR(en->type)) return ("xffm/emblem_chardevice");
    if (IS_SBLK(en->type)) return ("xffm/emblem_blockdevice");
    if (IS_SFIFO(en->type)) return ("xffm/emblem_fifo");
    if (IS_SSOCK(en->type)) return ("xffm/emblem_network/compositeSE/emblem_fifo");
    if (IS_PARTITION_TYPE(en->type)) return ("xffm/emblem_harddisk");
//    if (IS_SLNK(en->type)) return g_strdup("inode/symlink");
//    if ((en->type)) return g_strdup("inode/virtual-disk");
    return NULL;

}


// Partition greenball resolution is now set in fstab module.
static gchar *greenball_id(view_t *view_p, record_entry_t *en, gchar *icon_id){
    if (!icon_id || !en) return NULL;
    /* block controlled in fstab plugin: */
    gboolean valid_mount_view = 
	(view_p && (!view_p->en || view_p->en->module ||  IS_LOCAL_TYPE(view_p->en->type)));
    gboolean valid_mount_point =
	(en->path && IS_SDIR(en->type));
//	(en->path && (IS_SDIR(en->type) || IS_PARTITION_TYPE (en->type)));
    if (valid_mount_point && valid_mount_view){
	gint mounted = GPOINTER_TO_INT(rfm_natural(PLUGIN_DIR, "fstab", en, "entry_is_mounted"));
	if (mounted<0) {
	    gchar *g = g_strconcat (icon_id, "/compositeNW/emblem_unreadable", NULL);
	    g_free(icon_id);
	    icon_id = g;
	} else	if (mounted>0) {
	    gchar *g = g_strconcat (icon_id, "/compositeNW/emblem_greenball", NULL);
	    g_free(icon_id);
	    icon_id = g;
	} else if (rfm_natural(PLUGIN_DIR, "fstab", en->path, "is_in_fstab")){
	    gchar *g = g_strconcat (icon_id, "/compositeNW/emblem_redball", NULL);
	    g_free(icon_id);
	    icon_id = g;
	}
    } 
    return icon_id;
}

gboolean
rfm_save_icon_id_to_cache(record_entry_t *en, const gchar *id){
    if (en && (IS_UP_TYPE(en->type) || IS_DUMMY_TYPE(en->type))) return FALSE;
    const gchar *path;
    if (en) path = en->path;
    else path = "RODENT_ROOT";

    DBHashTable *icon_id_cache;
    gchar *g;
    if(getenv("RFM_CONTENT_FOLDER_ICONS") && strlen(getenv("RFM_CONTENT_FOLDER_ICONS"))){
	g = g_build_filename (ICON_ID_DBH_FILE, NULL);
    } else {
	g = g_build_filename (ICON_ID_PLAIN_DBH_FILE, NULL);
    }
    TRACE("opening %s...\n",g); 
    if (!g_file_test(g, G_FILE_TEST_EXISTS)){
	unsigned char key_length=11;
        gchar *directory = g_path_get_dirname(g);
        if (!g_file_test(directory, G_FILE_TEST_IS_DIR)){
            g_mkdir_with_parents(directory, 0700);
        }
        g_free(directory);
        icon_id_cache = dbh_new (g, &key_length, DBH_CREATE|DBH_PARALLEL_SAFE|DBH_THREAD_SAFE);
    } else {
	icon_id_cache = dbh_new (g, NULL, DBH_PARALLEL_SAFE|DBH_THREAD_SAFE);
    }
    TRACE("open %s.\n",g); 
    if (!icon_id_cache){
	DBG("cannot open %s for write.\n", g);
	g_free(g);
	return FALSE;
    }
    dbh_set_parallel_lock_timeout(icon_id_cache, 3);
    GString *gs = g_string_new (path);
    gchar *key = g_strdup_printf ("%10u", g_string_hash (gs));
    dbh_set_key  (icon_id_cache,  (unsigned char *)key);
    dbh_set_data (icon_id_cache, (void *)id, strlen(id)+1);
    g_string_free (gs, TRUE);
    g_free(key);
    gboolean retval = TRUE;
    if (dbh_update (icon_id_cache) == 0){
	DBG("cannot write record %s to cache: %s.\n", path, g);
	retval = FALSE;
    }
    dbh_close(icon_id_cache);
    g_free(g);
    return retval;
}
	
gchar *
rfm_get_icon_id_from_cache(record_entry_t *en){
    //return NULL;
    if (en && (IS_UP_TYPE(en->type) || IS_DUMMY_TYPE(en->type))) return NULL;
    const gchar *path;
    if (en) path = en->path;
    else path = "RODENT_ROOT";

    DBHashTable *icon_id_cache;
    gchar *cache_value=NULL;
    gchar *g;
    if(getenv("RFM_CONTENT_FOLDER_ICONS") && strlen(getenv("RFM_CONTENT_FOLDER_ICONS"))){
	g = g_build_filename (ICON_ID_DBH_FILE, NULL);
    } else {
	g = g_build_filename (ICON_ID_PLAIN_DBH_FILE, NULL);
    }
    if (!g_file_test(g, G_FILE_TEST_EXISTS)){
	g_free(g);
	return FALSE;
    }
    TRACE("opening %s...\n",g); 
    if((icon_id_cache = dbh_new (g, NULL, DBH_READ_ONLY|DBH_PARALLEL_SAFE|DBH_THREAD_SAFE)) != NULL) {
	dbh_set_parallel_lock_timeout(icon_id_cache, 3);
	GString *gs = g_string_new (path);
	sprintf ((char *)DBH_KEY (icon_id_cache), "%10u", g_string_hash (gs));
	g_string_free (gs, TRUE);
        TRACE("open load %s...\n",g); 
	if (dbh_load (icon_id_cache) != 0){
	    cache_value = g_strdup((gchar *)icon_id_cache->data);
	}
	dbh_close(icon_id_cache);
    }
    TRACE("open and close %s.\n",g); 
    g_free(g);
    if (cache_value && strlen(cache_value)==0) {
	g_free(cache_value);
	return NULL;
    }
    return cache_value;
}

gchar *
rfm_get_entry_icon_id(view_t *view_p, record_entry_t *en, gboolean magic_icon){
    if (!en) return g_strdup("xffm/emblem_computer");
    const gchar *id = NULL;

    // Modules may override icon resolution.
    if (en->module){
	id = rfm_natural (PLUGIN_DIR, en->module, en, "item_icon_id");
        // if module does not resolve icon, proceed as normal.
        if(id) {
	    NOOP("rfm_get_entry_icon_id(): %s: %s\n", en->path, id);
	    return greenball_id(view_p, en, g_strdup(id));
        }
	NOOP("module: %s does not resolve icon for %s\n", en->module, en->path);
    }

#if 0
    if (!IS_LOCAL_TYPE(en->type) || en->module)
    {
	// Remote files or module items will never be done magic.
	magic_icon = FALSE;	
    } 
    if (!magic_icon ) {
	// valid view? directory type icon?
	if (view_p->en && IS_LOCAL_TYPE(view_p->en->type) && IS_SDIR(en->type)){
	    id = get_plain_icon_id(en);	    
	    return greenball_id(view_p, en, g_strdup(id));
	}
	gchar *cache_value=get_icon_id_from_cache(en);
	if (cache_value) return cache_value;
	id = get_plain_icon_id (en);
	// remote file but valid view entry?
	// (this does not do the trick...)
	//if (IS_SDIR(en->type)) return greenball_id(view_p, en, g_strdup(id));
	return g_strdup(id);
    }
#endif

    // Try to resolve from readdir type information first.
    id = get_type_icon_id(en);
    if (id) {
	// greenball resolution
	//save_icon_id_to_cache(en, id);
	return  greenball_id(view_p, en, g_strdup(id));
    }

    // We need the stat record now.
    if (!en->st){
        DBG("rfm_get_entry_icon_id(): no stat record for %s\n", en->path);
        return (g_strdup("xffm/stock_dialog-warning"));
    }

    if (!en->st->st_blksize && !en->st->st_mtime){
	DBG("mime magic will not work if stat record is not initialized.\n");
	return g_strdup("xffm/stock_file/compositeC/stock_dialog-question");
    }

    // Try to get a valid extension mimetype first.
    g_free(en->mimetype);
    en->mimetype = MIME_type(en->path, en->st);
    if (!en->mimetype) {
	    en->mimetype = g_strdup(_("unknown"));
    }
    
    // Only in the event that a valid mime type is not found, 
    // and magic option is set, do magic.
    if (magic_icon && strcmp(en->mimetype, _("unknown"))==0) {
	// Does the user have read permission for the file?
	// Mime magic will not work otherwise...
	// We need a valid stat record for this.
	g_free(en->mimemagic);
	if (rfm_read_ok_path(en->path)) {
	    // Magic at this step as a last resort
	    en->mimemagic = rfm_rational(RFM_MODULE_DIR, "mime", en, "mime_magic", "mime_function");
	    if (!en->mimemagic) en->mimemagic = g_strdup(_("unknown"));
	} else {
	    en->mimemagic = g_strdup(_("No Read Permission"));
	}
    }

    // Resolve icon id from mimetype or mimemagic.
    if (en->mimetype && strcmp(en->mimetype, _("unknown"))){
	id = en->mimetype;
    } 
    if (!id) {
	id = en->mimemagic;
    }
    // Override of particular mimetypes
    if (!id || strcmp(id, _("unknown"))==0) {
	return g_strdup("xffm/stock_file");
    }
    if (strcmp(id, _("No Read Permission"))==0){
	return g_strdup("xffm/stock_file/compositeC/emblem_unreadable");
    }
    // Override dotdesktop icons with module icon determination.
    if (strcmp("application/x-desktop", id)==0) {
	const gchar *icon_name=rfm_natural(PLUGIN_DIR, "dotdesktop", en, "item_icon_id");
	if (icon_name) {
	    //save_icon_id_to_cache(en, icon_name);
	    return g_strdup(icon_name);
	}
    }
    //save_icon_id_to_cache(en, id);
    
#if 0
    // This would need fine tuning...
    if (!IS_LOCAL_TYPE(en->type)){
	gchar *g = g_strconcat(icon_id, "/compositeC/emblem_shared", NULL);
	g_free(icon_id);
	icon_id = g;
    }
#endif
    // Greenball resolution 
    return greenball_id(view_p, en, g_strdup(id));
}


void rfm_free_lite_hash(void){
    pthread_mutex_lock (&lite_hash_mutex);
    if(lite_hash) g_hash_table_destroy(lite_hash);
    pthread_mutex_unlock (&lite_hash_mutex);

    if(lite_type_hash) g_hash_table_destroy(lite_type_hash);
    if(lite_key_hash) g_hash_table_destroy(lite_key_hash);
}


// hmmmmmm serialized....
static void *
get_pixbuf_f(void *data){
    void **arg=data;
    const gchar *key = (const gchar *)arg[0];
    gint size = GPOINTER_TO_INT(arg[1]);
    GdkPixbuf *pixbuf = get_pixbuf(key, size, FALSE); //refs
    return pixbuf;
}

GdkPixbuf *
rfm_get_pixbuf (const gchar * key, gint size) {
    void *arg[2];
    arg[0] = (void *)key;
    arg[1] = GINT_TO_POINTER(size);
    GdkPixbuf *pixbuf = rfm_context_function(get_pixbuf_f, arg);
    return pixbuf;
}

// find in hash : refs
// put in hash: refs not
// get: refs
// create: refs

