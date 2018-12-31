#ifndef XFPIXBUF_HH
#define XFPIXBUF_HH
# include <config.h>
#include <stdlib.h>


#include "icons.hh"

namespace xf
{

template <class Type>
class Pixbuf {
    typedef Hash<Type> pixbuf_hash_c;
    typedef Icons<Type> pixbuf_icons_c;
public:
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

	// FIXME: check, remake out of date thumbnails/previews as needed 

    // last reference to the returned pixbuf (if any) belongs to the
    // pixbuf hashtable.
    static GdkPixbuf *
    get_pixbuf(const gchar *icon_name, gint size){
	TRACE("get_pixbuf(%s)\n", icon_name);
	if (!icon_name){
	    return NULL;
	    //ERROR("pixbuf_c::get_pixbuf() icon_name is NULL.\n");
	    //return get_pixbuf("image-missing", size);
	}
	// if item is not found in hash, it will be created and inserted into hash 
	// (whenever possible)
	// Pixbuf reference count increases each time a pixbuf is requested from 
	// hash table. Caller is responsible for unreferencing when no longer used.
	auto pixels = get_pixel_size(size);
	GdkPixbuf *pixbuf = pixbuf_hash_c::find_in_pixbuf_hash(icon_name, pixels);
	if (pixbuf) return pixbuf;
	// Not found, huh?
	TRACE("Create pixbuf and put in hashtable: \"%s\"\n", icon_name);
	pixbuf = pixbuf_icons_c::absolute_path_icon(icon_name, pixels);

	if (!pixbuf){
	    // check for composite icon definition or plain icon.
	    if (pixbuf_icons_c::is_composite_icon_name(icon_name)) pixbuf = pixbuf_icons_c::composite_icon(icon_name, pixels);
	    else pixbuf = pixbuf_icons_c::get_theme_pixbuf(icon_name, pixels);
	}
       
	if (pixbuf){
	    // put in iconhash...
	    pixbuf_hash_c::put_in_pixbuf_hash(icon_name, pixels, pixbuf);
	    return pixbuf;
	} 
	pixbuf = pixbuf_hash_c::find_in_pixbuf_hash("image-missing", pixels);
	if (pixbuf) return pixbuf;
	pixbuf = pixbuf_icons_c::get_theme_pixbuf("image-missing", pixels);
	if (pixbuf) pixbuf_hash_c::put_in_pixbuf_hash(icon_name, pixels, pixbuf);
	DBG("xf::Pixbuf::get_pixbuf(): image-missing. Please install icon \"%s\"\n", icon_name);
	return pixbuf;
     }

    static GdkPixbuf *
    find_pixbuf(const gchar *icon_name, gint size){
	GdkPixbuf *pixels = get_pixel_size(size);
	return pixbuf_hash_c::find_in_pixbuf_hash(icon_name, pixels);
    }


    static
    GdkPixbuf *
    fix_pixbuf_scale(GdkPixbuf *in_pixbuf, gint size){
	if (!in_pixbuf || !GDK_IS_PIXBUF(in_pixbuf)) return NULL;
	GdkPixbuf *out_pixbuf=NULL;
	gint height = gdk_pixbuf_get_height (in_pixbuf);
	gint width = gdk_pixbuf_get_width (in_pixbuf);

	// this is to fix paper size previews for text files and pdfs
	if((width < height && height != size) || 
	    (width >= height && width != size)) 
	{
	    out_pixbuf = gdk_pixbuf_scale_simple (in_pixbuf, 5*size/7,size,
		     GDK_INTERP_HYPER);
	    g_object_ref(out_pixbuf);
	    g_object_unref(in_pixbuf);
	    return out_pixbuf;
	} 
	return in_pixbuf;
    }


    static GdkPixbuf *
    pixbuf_from_file(const gchar *path, gint width, gint height){
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
	    TRACE(stderr, "** resizing %s\n", path);
	    GdkPixbuf *pix = gdk_pixbuf_scale_simple (pixbuf, width, height, GDK_INTERP_HYPER);
	    g_object_unref(pixbuf);
	    pixbuf = pix;

	}  
	
	if(error && !strstr(path, ".cache/rfm/thumbnails")) {
		ERROR ("pixbuf_from_file() %s:%s\n", error->message, path);
		g_error_free (error);
	}
	return pixbuf;
    }


    static void 
    pixbuf_save(GdkPixbuf *tgt, const gchar *path){
	if (!tgt || !path || !GDK_IS_PIXBUF(tgt)) {
	    ERROR("pixbuf_save_f(%s): !tgt || !path || !GDK_IS_PIXBUF(tgt)\n", 
		    path);
	    return ;
	}

	if (tgt && GDK_IS_PIXBUF(tgt)) {
	    GError *error = NULL;
	    gdk_pixbuf_save (tgt, path, "png", &error,
			     "tEXt::Software", "Rodent", NULL);
	    if (error){
		ERROR("pixbuf_save_f(%s): %s\n", path, error->message);
		g_error_free(error);
	    }
	}
	return ;
    }

};
}
///////////////////////////////////////////////////////////////////////

#endif
