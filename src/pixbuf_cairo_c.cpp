
/////////////////////////////////////////////////////////////////////////////////////////
//
//  http://www.gtkforums.com/about5204.html
//  tadej tadeboro from slovenia http://tadeboro.blogspot.com/
//
//  Tadej Borovsak tadeboro@gmail.com


typedef struct _Data Data;
struct _Data
{
   GtkWidget *box,
           *image,
           *entry;
   GdkPixbuf *pixbuf;
};



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
cairo_t *
pixbuf_cairo_create( GdkPixbuf *pixbuf)
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
   cairo_t         *cr;           /* Final context */

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
GdkPixbuf *
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
