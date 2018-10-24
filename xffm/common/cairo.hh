#ifndef XFCAIRO_HH
#define XFCAIRO_HH
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>

/////////////////////////////////////////////////////////////////////////////////////////
//
//  http://www.gtkforums.com/about5204.html
//  tadej tadeboro from slovenia http://tadeboro.blogspot.com/
//
//  Tadej Borovsak tadeboro@gmail.com
namespace xf
{


typedef struct _Data Data;
struct _Data
{
   GtkWidget *box;
   GtkWidget *image;
   GtkWidget *entry;
   GdkPixbuf *pixbuf;
};

template <class Type>
class Cairo{
public:

    static cairo_surface_t *
    pixbuf_cairo_surface( GdkPixbuf *pixbuf)
    {
	if (!pixbuf || !GDK_IS_PIXBUF(pixbuf)) return NULL;
       gint         width,        /* Width of both pixbuf and surface */
		    height,       /* Height of both pixbuf and surface */
		    p_stride,     /* Pixbuf stride value */
		    p_n_channels, /* RGB -> 3, RGBA -> 4 */
		    s_stride;     /* Surface stride value */
       guchar          *p_pixels,     /* Pixbuf's pixel data */
		   *s_pixels;     /* Surface's pixel data */
       cairo_surface_t *surface;      /* Temporary image surface */

       //g_object_ref( G_OBJECT( pixbuf ) );

       /* Inspect input pixbuf and create compatible cairo surface */
       g_object_get( G_OBJECT( pixbuf ), "width",           &width,
				 "height",          &height,
				 "rowstride",       &p_stride,
				 "n-channels",      &p_n_channels,
				 "pixels",          &p_pixels,
				 NULL );
       surface = cairo_image_surface_create( CAIRO_FORMAT_ARGB32, width, height );
       s_stride = cairo_image_surface_get_stride( surface );
       s_pixels = cairo_image_surface_get_data( surface );

       /* Copy pixel data from pixbuf to surface */
       while( height-- )
       {
	  gint    i;
	  guchar *p_iter = p_pixels,
		*s_iter = s_pixels;

	  for( i = 0; i < width; i++ )
	  {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
	     /* Pixbuf:  RGB(A)
	      * Surface: BGRA */
	     if( p_n_channels == 3 )
	     {
		s_iter[0] = p_iter[2];
		s_iter[1] = p_iter[1];
		s_iter[2] = p_iter[0];
		s_iter[3] = 0xff;
	     }
	     else /* p_n_channels == 4 */
	     {
		gdouble alpha_factor = p_iter[3] / (gdouble)0xff;

		s_iter[0] = (guchar)( p_iter[2] * alpha_factor + .5 );
		s_iter[1] = (guchar)( p_iter[1] * alpha_factor + .5 );
		s_iter[2] = (guchar)( p_iter[0] * alpha_factor + .5 );
		s_iter[3] =           p_iter[3];
	     }
#elif G_BYTE_ORDER == G_BIG_ENDIAN
	     /* Pixbuf:  RGB(A)
	      * Surface: ARGB */
	     if( p_n_channels == 3 )
	     {
		s_iter[3] = p_iter[2];
		s_iter[2] = p_iter[1];
		s_iter[1] = p_iter[0];
		s_iter[0] = 0xff;
	     }
	     else /* p_n_channels == 4 */
	     {
		gdouble alpha_factor = p_iter[3] / (gdouble)0xff;

		s_iter[3] = (guchar)( p_iter[2] * alpha_factor + .5 );
		s_iter[2] = (guchar)( p_iter[1] * alpha_factor + .5 );
		s_iter[1] = (guchar)( p_iter[0] * alpha_factor + .5 );
		s_iter[0] =           p_iter[3];
	     }
#else /* PDP endianness */
	     /* Pixbuf:  RGB(A)
	      * Surface: RABG */
	     if( p_n_channels == 3 )
	     {
		s_iter[0] = p_iter[0];
		s_iter[1] = 0xff;
		s_iter[2] = p_iter[2];
		s_iter[3] = p_iter[1];
	     }
	     else /* p_n_channels == 4 */
	     {
		gdouble alpha_factor = p_iter[3] / (gdouble)0xff;

		s_iter[0] = (guchar)( p_iter[0] * alpha_factor + .5 );
		s_iter[1] =           p_iter[3];
		s_iter[1] = (guchar)( p_iter[2] * alpha_factor + .5 );
		s_iter[2] = (guchar)( p_iter[1] * alpha_factor + .5 );
	     }
#endif
	     s_iter += 4;
	     p_iter += p_n_channels;
	  }
	  s_pixels += s_stride;
	  p_pixels += p_stride;
       }
       return surface;
    }

    /* Key for automated pixbuf updating and destruction */
    //static const cairo_user_data_key_t pixbuf_key;

    /**
    * pixbuf_cairo_create:
    * @pixbuf: GdkPixbuf that you wish to wrap with cairo context
    *
    * This function will initialize new cairo context with contents of @pixbuf. You
    * can then draw using returned context. When finished drawing, you must call
    * pixbuf_cairo_destroy() or your pixbuf will not be updated with new contents!
    *
    * Return value: New cairo_t context. When you're done with it, call
    * pixbuf_cairo_destroy() to update your pixbuf and free memory.
    */
    static cairo_t *
    pixbuf_cairo_create( GdkPixbuf *pixbuf)
    {
       cairo_t         *cr;           /* Final context */
       cairo_surface_t *surface = pixbuf_cairo_surface(pixbuf);
       /* Create context and set user data */
       cr = cairo_create( surface );
       cairo_surface_destroy( surface );
       //cairo_set_user_data( cr, pixbuf_key, pixbuf, g_object_unref );
       /* Return context */
       return( cr );
    }

    /**
    * pixbuf_cairo_destroy:
    * @cr: Cairo context that you wish to destroy
    * @create_new_pixbuf: If TRUE, new pixbuf will be created and returned. If
    *                     FALSE, input pixbuf will be updated in place.
    *
    * This function will destroy cairo context, created with pixbuf_cairo_create().
    *
    * Return value: New or updated GdkPixbuf. You own a new reference on return
    * value, so you need to call g_object_unref() on returned pixbuf when you don't
    * need it anymore.
    */
    static GdkPixbuf *
    pixbuf_cairo_destroy( cairo_t  *cr, GdkPixbuf *pixbuf)
    {
       gint             width,        /* Width of both pixbuf and surface */
		    height,       /* Height of both pixbuf and surface */
		    p_stride,     /* Pixbuf stride value */
		    p_n_channels, /* RGB -> 3, RGBA -> 4 */
		    s_stride;     /* Surface stride value */
       guchar          *p_pixels,     /* Pixbuf's pixel data */
		   *s_pixels;     /* Surface's pixel data */
       cairo_surface_t *surface;      /* Temporary image surface */

       /* Obtain pixbuf to be returned */
       /*tmp_pix = cairo_get_user_data( cr, pixbuf_key );
       if( create_new_pixbuf )
	  pixbuf = gdk_pixbuf_copy( tmp_pix );
       else
	  pixbuf = g_object_ref( G_OBJECT( tmp_pix ) );*/

       /* Obtain surface from where pixel values will be copied */
       surface = cairo_get_target( cr );

       /* Inspect pixbuf and surface */
       g_object_get( G_OBJECT( pixbuf ), "width",           &width,
				 "height",          &height,
				 "rowstride",       &p_stride,
				 "n-channels",      &p_n_channels,
				 "pixels",          &p_pixels,
				 NULL );
       s_stride = cairo_image_surface_get_stride( surface );
       s_pixels = cairo_image_surface_get_data( surface );

       /* Copy pixel data from surface to pixbuf */
       while( height-- )
       {
	  gint    i;
	  guchar *p_iter = p_pixels,
		*s_iter = s_pixels;

	  for( i = 0; i < width; i++ )
	  {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
	     /* Pixbuf:  RGB(A)
	      * Surface: BGRA */
	     gdouble alpha_factor = (gdouble)0xff / s_iter[3];

	     p_iter[0] = (guchar)( s_iter[2] * alpha_factor + .5 );
	     p_iter[1] = (guchar)( s_iter[1] * alpha_factor + .5 );
	     p_iter[2] = (guchar)( s_iter[0] * alpha_factor + .5 );
	     if( p_n_channels == 4 )
		p_iter[3] = s_iter[3];
#elif G_BYTE_ORDER == G_BIG_ENDIAN
	     /* Pixbuf:  RGB(A)
	      * Surface: ARGB */
	     gdouble alpha_factor = (gdouble)0xff / s_iter[0];

	     p_iter[0] = (guchar)( s_iter[1] * alpha_factor + .5 );
	     p_iter[1] = (guchar)( s_iter[2] * alpha_factor + .5 );
	     p_iter[2] = (guchar)( s_iter[3] * alpha_factor + .5 );
	     if( p_n_channels == 4 )
		p_iter[3] = s_iter[0];
#else /* PDP endianness */
	     /* Pixbuf:  RGB(A)
	      * Surface: RABG */
	     gdouble alpha_factor = (gdouble)0xff / s_iter[1];

	     p_iter[0] = (guchar)( s_iter[0] * alpha_factor + .5 );
	     p_iter[1] = (guchar)( s_iter[3] * alpha_factor + .5 );
	     p_iter[2] = (guchar)( s_iter[2] * alpha_factor + .5 );
	     if( p_n_channels == 4 )
		p_iter[3] = s_iter[1];
#endif
	     s_iter += 4;
	     p_iter += p_n_channels;
	  }
	  s_pixels += s_stride;
	  p_pixels += p_stride;
       }

       /* Destroy context */
       cairo_destroy( cr );

       /* Return pixbuf */
       return( pixbuf );
    }


    /* Edscott Wilson Garcia: create a colored pixbuf mask. */

    static GdkPixbuf *
    create_pixbuf_mask(GdkPixbuf *in_pixbuf, guchar red, guchar green, guchar blue){
	g_object_ref(in_pixbuf);
	
	gint        width;        /* Width of both pixbufs */
	gint        height;       /* Height of both pixbufs */
	gint        p_stride;     /* Source  pixbuf's stride value */
	gint        s_stride;     /* Target pixbuf's stride value */
	gint        p_n_channels; /* RGB -> 3, RGBA -> 4 */
	guchar      *p_pixels;     /* Source pixbuf's pixel data */
	guchar      *s_pixels;     /* Target pixbuf's pixel data */
		       
	g_object_get( G_OBJECT( in_pixbuf ), "width",           &width,
				 "height",          &height,
				 "rowstride",       &p_stride,
				 "n-channels",      &p_n_channels,
				 "pixels",          &p_pixels,
				 NULL );
	GdkPixbuf   *pixbuf =  gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, width, height);
	g_object_get( G_OBJECT( pixbuf ), 
				 "rowstride",       &s_stride,
				 "pixels",          &s_pixels,
				 NULL );

	/* Copy pixel data from pixbuf to  pixbuf*/
	while( height--) {
	  gint    i;
	  guchar *p_iter = p_pixels;
	  guchar *s_iter = s_pixels;

	  for( i = 0; i < width; i++ )
	  {
	    /* Pixbuf:  RGB(A) */
	     if( p_n_channels == 3  || p_iter[3] > 0xf0) {
		s_iter[0] = red;
		s_iter[1] = green;
		s_iter[2] = blue;
		s_iter[3] = 0xff;
	     } else memset(s_iter, 0, 4*sizeof(guchar));
	     s_iter += 4;
	     p_iter += p_n_channels;
	  }
	  s_pixels += s_stride;
	  p_pixels += p_stride;
	}
	g_object_unref(in_pixbuf);
	return pixbuf;
    }

    static void
    add_color_pixbuf(cairo_t *pixbuf_context, GdkPixbuf *pixbuf, 
	    const gchar *color){
	if (!pixbuf || !pixbuf_context) return;
	gchar buf[3];
	buf[2]=0;
	buf[0]=color[0];
	buf[1]=color[1];
	glong r = strtol(buf, NULL, 16);  
	buf[0]=color[2];
	buf[1]=color[3];
	glong g = strtol(buf, NULL, 16);  
	buf[0]=color[4];
	buf[1]=color[5];
	glong b = strtol(buf, NULL, 16);  

	guchar red = r;
	guchar green = g;
	guchar blue = b;
	TRACE("pixbuf_hash_c::add_color_pixbuf: color %x,%x,%x\n", red, green, blue);  
    //    GdkPixbuf *pixbuf_mask = create_pixbuf_mask(pixbuf, 0x11, 0x11, 0x11);  
	GdkPixbuf *pixbuf_mask = create_pixbuf_mask(pixbuf, red, green, blue);  
	gdk_cairo_set_source_pixbuf(pixbuf_context, pixbuf_mask, 0,0);
	cairo_paint_with_alpha(pixbuf_context, 0.450);
	g_object_unref(pixbuf_mask);
    }

    static void 
    insert_pixbuf_tag (cairo_t *pixbuf_context, GdkPixbuf *tag,
	    GdkPixbuf *composite_pixbuf,
	    const gchar *where, const gchar *scale, const gchar *alpha)
    {

	if (!tag) {return ; }

       double scale_factor = strtod(scale, NULL);
	if (isnan(scale_factor)) return ;
	if (scale_factor < 1 || scale_factor > 5) return ;
	errno = 0;
	gint overall_alpha = strtol(alpha, NULL, 10);
	if (errno){
	    g_warning("insert_pixbuf_tag_f(): strtol(%s) -> %s\n", alpha, strerror(errno));
	    return ;
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
	
	// This proved necessary in opensuse-12.3,
	// but not in gentoo nor ubuntu. Go figure...
	//gdk_cairo_set_source_pixbuf(pixbuf_context, composite_pixbuf,0,0);
	// cairo_paint_with_alpha(pixbuf_context, 1.0);
	    
	gdk_cairo_set_source_pixbuf(pixbuf_context, tag_s, offset_x,offset_y);
	cairo_paint_with_alpha(pixbuf_context, (double)overall_alpha/255.0);

	g_object_unref(tag_s);
	return ;
    }

    static void
    add_label_pixbuf(cairo_t *pixbuf_context, GdkPixbuf *pixbuf, const gchar *icon_text){
	if (!icon_text || !pixbuf || !pixbuf_context) return;
	// Insert text into pixbuf
	gint x = 0;
	gint y = 0;
	const gchar *text_size = "x-small";
	/*switch(size) {
	    case SMALL_ICON_SIZE: text_size="xx-small"; break;
	    case MEDIUM_ICON_SIZE: text_size="x-small"; break;
	    case BIG_ICON_SIZE: text_size="medium"; break;
	}	
	if (!text_size) return ;*/

	GdkPixbuf   *t_pixbuf = NULL;
	gchar *layout_text;
	    layout_text = g_strdup_printf("<span foreground=\"black\" background=\"#ffffff\" size=\"%s\" font_weight=\"bold\">%s </span>", text_size, _(icon_text));

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
	cairo_t   *t_pixbuf_context = pixbuf_cairo_create(t_pixbuf);

	cairo_rectangle(t_pixbuf_context, 0, 0, logical_rect.width+2, logical_rect.height+2);
	cairo_clip(t_pixbuf_context);


	cairo_move_to (t_pixbuf_context, 1, 1);
	    
	cairo_set_source_rgba(t_pixbuf_context, 0, 0, 0, 1.0);
	if (PANGO_IS_LAYOUT (layout)) {
	    pango_cairo_show_layout (t_pixbuf_context, layout);
	    g_object_unref(layout);
	    g_object_unref(context);
	}
	pixbuf_cairo_destroy(t_pixbuf_context, t_pixbuf);

	if (t_pixbuf) {
	    gdk_cairo_set_source_pixbuf(pixbuf_context, t_pixbuf,x,y);
	    cairo_paint_with_alpha(pixbuf_context, 0.650);
	    g_object_unref(t_pixbuf);
	}

	return ;
    }


};
}
#endif
