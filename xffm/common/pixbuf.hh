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
	    ERROR("pixbuf_c::get_pixbuf() icon_name is NULL.\n");
	    return get_pixbuf("image-missing", size);
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
	WARN("xf::Pixbuf::get_pixbuf(): image-missing. Please install icon \"%s\"\n", icon_name);
	return pixbuf;
     }

    static GdkPixbuf *
    find_pixbuf(const gchar *icon_name, gint size){
	GdkPixbuf *pixels = get_pixel_size(size);
	return pixbuf_hash_c::find_in_pixbuf_hash(icon_name, pixels);
    }
};
}
///////////////////////////////////////////////////////////////////////

#endif
