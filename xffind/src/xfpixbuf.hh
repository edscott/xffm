#ifndef XFPIXBUF_HH
#define XFPIXBUF_HH
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdlib.h>

#define SIZE_BUTTON	20
#define SIZE_DIALOG	36
#define SIZE_ICON	48
#define SIZE_PREVIEW	96
#define SIZE_TIP	128

#include "xficons.hh"

namespace xf
{

template <class Type>
class Pixbuf {
    typedef Hash<Type> pixbuf_hash_c;
    typedef Icons<Type> pixbuf_icons_c;
public:
    static gint
    get_pixel_size(gint size){
	gint pixels = 24;;
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
	// if item is not found in hash, it will be created and inserted into hash 
	// (whenever possible)
	// Pixbuf reference count increases each time a pixbuf is requested from 
	// hash table. Caller is responsible for unreferencing when no longer used.
	gint pixels = get_pixel_size(size);
	GdkPixbuf *pixbuf = pixbuf_hash_c::find_in_pixbuf_hash(icon_name, pixels);
	if (pixbuf) return pixbuf;
	// Not found, huh?
	std::cerr<<"Create pixbuf and put in hashtable: "<<icon_name<<"\n";
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
	std::cerr<<"image-missing pixbuf returned.\n";
	return pixbuf;
     }

    static GdkPixbuf *
    find_pixbuf(const gchar *icon_name, gint size){
	gint pixels = get_pixel_size(size);
	return pixbuf_hash_c::find_in_pixbuf_hash(icon_name, pixels);
    }
};
}
///////////////////////////////////////////////////////////////////////

#endif
