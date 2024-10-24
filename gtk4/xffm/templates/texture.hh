#ifndef XF_TEXTURE_HH
#define XF_TEXTURE_HH

//static bool greenLightPreview = true;
namespace xf {
template <class Type> class Preview;
template <class Type>  class Texture {
      public:
      //static bool previewOK(void) {return  greenLightPreview;} 
      /* obsolete
      static void redlight(void){
        TRACE("redlight...\n");
        greenLightPreview = false;
        return;
      }  
      static void greenlight(void){
        TRACE("greenlight...\n");
        greenLightPreview = true;
        return ;
      }  
      */
      static
      GdkPaintable *load(GFileInfo *info){
        auto gIcon = g_file_info_get_icon(info);
        GdkPaintable *texture;
        if (gIcon) {
          auto icon = gtk_icon_theme_lookup_by_gicon(iconTheme, gIcon, 96, 
              1, GTK_TEXT_DIR_NONE,(GtkIconLookupFlags)0);
          texture = GDK_PAINTABLE(icon);
        } else return NULL;
        return GDK_PAINTABLE(texture);
      }
      static
      GdkPaintable *load(const char *iconName, int size){
        GdkPaintable *texture;
        auto icon = gtk_icon_theme_lookup_icon(  //GtkIconPaintable*
            iconTheme, iconName,
            NULL, size, 1, GTK_TEXT_DIR_NONE, (GtkIconLookupFlags) 0);

        if (icon) {
          return GDK_PAINTABLE(icon);
        } else return NULL;
        
      }

      static 
      GdkPaintable *loadPath(const char *path){
        GError *error_ = NULL;
        if (!path) return NULL;
        
        if (!g_file_test(path, G_FILE_TEST_EXISTS)) return NULL;
        GdkPaintable *paintable = NULL;
        auto mimetype = Mime::mimeType(path);
        if (!mimetype) mimetype =  _("unknown");
        if (strstr(mimetype, "image")) {
          return Preview<Type>::getPaintableWithThumb(path);
        }

        bool textType =(
            strstr(mimetype, "text")
            || strstr(mimetype, "shellscript")
            || strcmp(path, "empty-file")==0
            || g_file_test(path, G_FILE_TEST_IS_DIR)
            );

        if (textType){
             GdkPaintable *paintable = Preview<Type>::textPreview (path, PREVIEW_IMAGE_SIZE); 
             TRACE("%s texttype paintable=%p\n", path, paintable);
             return paintable;
        }
        // pdf previews...
        bool useGhostScript = (strstr (mimetype, "pdf") || strstr (mimetype, "postscript") );
        if(useGhostScript) {  

            // decode delegate is ghostscript
             GdkPaintable *paintable =  Preview<Type>::gsPreview (path, PREVIEW_IMAGE_SIZE);// refs
             return paintable;
        }

        return NULL;
      }
      
     static 
      GdkPaintable *loadIconName(const char *item){
        if (!item) return NULL;      
        // From iconname.
        int size = Settings::getInteger("xfterm", "iconsize");
        auto icon = gtk_icon_theme_lookup_icon (iconTheme,
                       item,  
                       NULL, // const char** fallbacks,
                       size,
                       1,    // int scale,
                       GTK_TEXT_DIR_NONE, 
                       (GtkIconLookupFlags) 0);

        return GDK_PAINTABLE(icon);
      }

      static void *preview(void *data){
        // This is a pthread function, not active in g_main_context.
        auto arg = (void **)data;
        auto path = (char *)arg[0];
        auto imageBox = GTK_BOX(arg[1]);
        auto image = GTK_IMAGE(arg[2]);
        auto serial = GPOINTER_TO_INT(arg[3]);
        auto size = GPOINTER_TO_INT(arg[4]);
        auto child = arg[5];
        TRACE("Texture::preview: %s, box=%p, image=%p, serial=%d\n", 
            path,imageBox, image, serial);

        auto paintable = loadPath(path);
        // context function...
        g_free(path);
        g_free(arg);
        // replace image in main context
        void *replaceArg[] ={
          (void *) paintable,
          (void *) imageBox,
          (void *) image,
          GINT_TO_POINTER(serial),
          GINT_TO_POINTER(size),
          child,
          NULL
        };
        auto currentSerial = Child::getSerial(GTK_WIDGET(child));
        TRACE("Texture::preview: calling replace_f, current serial=%d, valid serial = %d\n",
            currentSerial, serial);

        if (serial != currentSerial){
          DBG("Current serial mismatch %d != %d. Dropping paintable %p from thread.\n", 
              currentSerial, serial, paintable);
        } else {
          // Here we execute the gtk widget replacement in the g_main_context.
          auto retval = Basic::context_function(replace_f, replaceArg);
          TRACE("UtilBasic::context_function(replace_f) retval = %d\n", GPOINTER_TO_INT(retval));
        }

        return NULL;
      }
      
      static void *replace_f(void *data){

        auto replaceArg = (void **)data;
        auto serial = GPOINTER_TO_INT(replaceArg[3]);
        auto activeSerial = Child::getSerial();

        if (serial != activeSerial){
          DBG("mainContext::replace_f(): serial mismatch %d != %d (dropping paintable)\n", 
              serial, Child::getSerial());
          return NULL;
        }
        auto paintable = GDK_PAINTABLE(replaceArg[0]);
        auto imageBox = GTK_BOX(replaceArg[1]);
        auto image = GTK_IMAGE(replaceArg[2]);
        //auto serial = GPOINTER_TO_INT(replaceArg[3]);
        auto size = GPOINTER_TO_INT(replaceArg[4]);
        auto child = GTK_WIDGET(replaceArg[5]);

        TRACE("replace_f in main context: paintable=%p, box=%p, image=%p, serial=%d\n", 
            paintable, imageBox, image, serial);

        gtk_box_remove(imageBox, GTK_WIDGET(image));
        GtkWidget *preview = gtk_image_new_from_paintable(paintable);
        gtk_widget_set_size_request(preview, size, size);
        Basic::boxPack0(GTK_BOX(imageBox), GTK_WIDGET(preview), FALSE, FALSE, 0);    
        return  GINT_TO_POINTER(serial);
      }



#if 0
        auto type = g_file_info_get_file_type(info);
        const char *iconLo = NULL;
        const char *iconHi = NULL;
        GFile *z = G_FILE(g_file_info_get_attribute_object(info, "standard::file"));
        auto path = g_file_get_path(z);
        switch (type){
          case G_FILE_TYPE_UNKNOWN:
            iconLo = "default";
            break;
          case G_FILE_TYPE_REGULAR:
            iconLo = "application-x-generic";
            break;
          case G_FILE_TYPE_DIRECTORY:                  
            if (strcmp(path, g_get_home_dir())==0) iconLo = "user-home";
            else iconLo = "folder";
            break;
          case G_FILE_TYPE_SYMBOLIC_LINK:
            iconLo = "emblem-symbolic-link";
            break;
          case G_FILE_TYPE_SPECIAL:
            iconLo = "application-x-generic";
            break;
          case G_FILE_TYPE_SHORTCUT: // Windows
            iconLo = "emblem-symbolic-link";
            break;
          case G_FILE_TYPE_MOUNTABLE:
            iconLo = "drive-harddisk";
            break;
        }
        g_free(path);
        if (iconLo == NULL) iconLo ="application-certificate";
        auto texture = Texture::load(iconLo);
        
#endif  
    static const char *locate(const char *name){
      char *tname = gtk_icon_theme_get_theme_name(iconTheme);
      const char *dirs[] = {"places", "mimetypes", "status", "devices", "emblems", "stock", NULL}; 
      for (auto p=dirs; p && *p; p++){
        auto *path = g_strdup_printf("/usr/share/icons/%s/scalable/%s/%s.svg", tname, *p, name);
        if (g_file_test(path, G_FILE_TEST_EXISTS)) return (const char *)path;
        g_free(path);
        path = g_strdup_printf("/usr/share/icons/hicolor/scalable/%s/%s.svg", *p, name);
        if (g_file_test(path, G_FILE_TEST_EXISTS)) return (const char *)path;
        g_free(path);
        path = g_strdup_printf("%s/share/icons/hicolor/scalable/%s/%s.svg",PREFIX, *p, name);
        if (g_file_test(path, G_FILE_TEST_EXISTS)) return (const char *)path;
        g_free(path);
      }
      return NULL;
     }

    static const char *
    findIconPath(GFileInfo *info){
          auto gIcon = g_file_info_get_icon(info);
          auto tIcon = G_THEMED_ICON(gIcon);
          auto names = g_themed_icon_get_names(tIcon);
          for (auto p=names; p && *p; p++) {
            if (!iconPathHash) iconPathHash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
            const char *path = (const char *)g_hash_table_lookup(iconPathHash, *p);
            if (path) return path;
            path = locate(*p);
            if (path) {
              g_hash_table_insert(iconPathHash, g_strdup(*p), (void *)path);
              TRACE("findIconPath hash insert name=%s -> %s\n", *p, path);
              return path;
            }
          }
          TRACE("*** Warning: could not find icon path for any of:\n");
          for (auto p=names; p && *p; p++) {
            fprintf(stderr, "    %s\n", *p);
          }
          TRACE("*** Warning: using application-x-generic instead...\n");

          const char *path = locate ("application-x-generic");
          if (path) {
              g_hash_table_insert(iconPathHash, g_strdup("application-x-generic"), (void *)path);
              return path;
          }
          return NULL;
    }

    static GdkPaintable *getSvgPaintable(const char *file, double width, double height){
          GError *error_ = NULL;
          RsvgHandle *handle = rsvg_handle_new_from_file (file, &error_ );
          if (error_) DBG("*** Error: %s\n", error_->message);
          cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
          cairo_t *cr = cairo_create (surface);
          RsvgRectangle  viewport;
          viewport.x = viewport.y = 0.0;
          viewport.width = width;
          viewport.height = height;
          if (!rsvg_handle_render_document (handle, cr, &viewport, &error_)){
             DBG("*** Error: %s\n", error_->message);
          }

        auto string = Settings::getString("xfterm", "iconsBg");
        if (string){
          char buffer[3];
          buffer[2]=0;
          buffer[0]=string[1];
          buffer[1]=string[2];
          double red = strtol(buffer, NULL, 16);
          buffer[0]=string[3];
          buffer[1]=string[4];
          double green = strtol(buffer, NULL, 16);
          buffer[0]=string[5];
          buffer[1]=string[6];
          double blue = strtol(buffer, NULL, 16);
          //TRACE("red=%lf green=%lf blue=%lf\n", red/255., green/255., blue/255.);
          cairo_set_source_rgba (cr, red/255., green/255., blue/255., .5);
        } else cairo_set_source_rgba (cr, 1,1,1, .5);
        cairo_set_line_width (cr, 1);
        cairo_rectangle (cr, 0.0, 0.0, width, height);
        cairo_fill (cr);
       /* cairo_surface_t *surface2 = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
        cairo_t *cr2 = cairo_create (surface2);

        cairo_set_source_rgba(cr2, 1,1,1,.5);
        cairo_set_source_surface(cr2, surface, 0, 0); // OjO: cr3 coordinates!
        cairo_rectangle (cr2, 0.0, 0.0, width, height);
        cairo_fill (cr2);*/


        auto texture = GDK_PAINTABLE(gdk_texture_new_for_surface(surface));
        cairo_surface_destroy(surface);
        return  texture;
    }

    static GdkMemoryFormat
    cairo_format_to_memory_format (cairo_format_t format)
    {
      switch (format)
      {
        case CAIRO_FORMAT_ARGB32:
          return GDK_MEMORY_DEFAULT;

        case CAIRO_FORMAT_RGB24:
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
          return GDK_MEMORY_B8G8R8X8;
#elif G_BYTE_ORDER == G_BIG_ENDIAN
          return GDK_MEMORY_X8R8G8B8;
#else
#error "Unknown byte order for Cairo format"
#endif
        case CAIRO_FORMAT_A8:
          return GDK_MEMORY_A8;
        case CAIRO_FORMAT_RGB96F:
          return GDK_MEMORY_R32G32B32_FLOAT;
        case CAIRO_FORMAT_RGBA128F:
          return GDK_MEMORY_R32G32B32A32_FLOAT;

        case CAIRO_FORMAT_RGB16_565:
        case CAIRO_FORMAT_RGB30:
        case CAIRO_FORMAT_INVALID:
        case CAIRO_FORMAT_A1:
        default:
          g_assert_not_reached ();
          return GDK_MEMORY_DEFAULT;
      }
    }

    
    // copy pasted from https://gitlab.gnome.org/GNOME/gtk/-/blob/main/gdk/gdktexture.c
    // both cairo_format_to_memory_format and gdk_texture_new_for_surface
    // pixbuf stuff is now deprecated.
    // Why:
    /* The reason we don’t expose that function is twofold:

        It requires a lot of invariants on the cairo surface - image surface, 
        no device scale, no device offset - and those are complex to document 
        and even more complex to reliably check.
        It’s small, so you can just copy/paste it. And then you can adapt it 
        to the invariants your code satisfies.
        So the recommended solution is indeed for everybody to copy 
        gdk_texture_new_for_surface() into their code - it’s why 
        GDK_MEMORY_DEFAULT exists after all.
    */
    static GdkTexture *
    gdk_texture_new_for_surface (cairo_surface_t *surface)
    {
      GdkTexture *texture;
      GBytes *bytes;

      g_return_val_if_fail (cairo_surface_get_type (surface) == CAIRO_SURFACE_TYPE_IMAGE, NULL);
      g_return_val_if_fail (cairo_image_surface_get_width (surface) > 0, NULL);
      g_return_val_if_fail (cairo_image_surface_get_height (surface) > 0, NULL);
      bytes = g_bytes_new_with_free_func (cairo_image_surface_get_data (surface),
                                          cairo_image_surface_get_height (surface)
                                          * cairo_image_surface_get_stride (surface),
                                          (GDestroyNotify) cairo_surface_destroy,
                                          cairo_surface_reference (surface));
      texture = gdk_memory_texture_new (cairo_image_surface_get_width (surface),
                                        cairo_image_surface_get_height (surface),
                                        cairo_format_to_memory_format (cairo_image_surface_get_format (surface)),
                                        bytes,
                                        cairo_image_surface_get_stride (surface));
      g_bytes_unref (bytes);
      return texture;
    }
        
  };

}

#endif
