#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdlib.h>
#include "pixbuf_c.hpp"

gint
pixbuf_c::get_pixel_size(gint size){
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
// last reference to the returned pixbuf (if any) belongs to the
// pixbuf hashtable.
GdkPixbuf *
pixbuf_c::get_pixbuf(const gchar *icon_name, gint size){
    // if item is not found in hash, it will be created and inserted into hash 
    // (whenever possible)
    // Pixbuf reference count increases each time a pixbuf is requested from 
    // hash table. Caller is responsible for unreferencing when no longer used.
    gint pixels = get_pixel_size(size);
    GdkPixbuf *pixbuf = find_in_pixbuf_hash(icon_name, pixels);
    if (pixbuf) return pixbuf;
    return find_in_pixbuf_hash("image-missing", pixels);
 }
GdkPixbuf *
pixbuf_c::find_pixbuf(const gchar *icon_name, gint size){
    gint pixels = get_pixel_size(size);
    return find_in_pixbuf_hash(icon_name, pixels);
}

///////////////////////////////////////////////////////////////////////


