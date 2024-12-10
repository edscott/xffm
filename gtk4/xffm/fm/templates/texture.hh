#ifndef XF_TEXTURE_HH
#define XF_TEXTURE_HH
/* vim:set foldmethod=marker expandtab: */
//static bool greenLightPreview = true;
namespace xf {
  GHashTable *iconHash = NULL;
  GHashTable *iconHash16= NULL;
template <class Type> class Preview;
template <class Type>  class Texture {
  /* {{{ Load */
  private:
      static GdkPaintable *getIcon16(const char *iconName){
        if (!iconHash16) iconHash16 = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);
        auto paintable = g_hash_table_lookup(iconHash16, iconName);
        if (paintable) {
          g_object_ref(paintable); // increment ref count...
                                   // This because gtkImage holds a reference
                                   // and when the widget is unparented,
                                   // it seems to call unref on the paintable.
          return GDK_PAINTABLE(paintable);
        }
        auto icon = gtk_icon_theme_lookup_icon(  //GtkIconPaintable*
            iconTheme, iconName,
            NULL, 16, 1, GTK_TEXT_DIR_NONE, (GtkIconLookupFlags) 0);
        g_object_ref(G_OBJECT(icon));
        g_hash_table_insert(iconHash16, g_strdup(iconName), GDK_PAINTABLE(icon));
        return GDK_PAINTABLE(icon);
      }

      static GdkPaintable *getIcon(const char *iconName){
        if (!iconHash) iconHash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);
        auto paintable = g_hash_table_lookup(iconHash, iconName);
        if (paintable) {
          g_object_ref(paintable); // increment ref count...
                                   // This because gtkImage holds a reference
                                   // and when the widget is unparented,
                                   // it seems to call unref on the paintable.
          return GDK_PAINTABLE(paintable);
        }

        auto icon = gtk_icon_theme_lookup_icon(  //GtkIconPaintable*
            iconTheme, iconName,
            NULL, 96, 1, GTK_TEXT_DIR_NONE, (GtkIconLookupFlags) 0);
        g_object_ref(G_OBJECT(icon));
        g_hash_table_insert(iconHash, g_strdup(iconName), GDK_PAINTABLE(icon));
        return GDK_PAINTABLE(icon);
      }

      static GdkPaintable *getIcon(GIcon *gIcon){
        if (!iconHash) iconHash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);
        auto iconName = g_icon_to_string(gIcon);
        auto paintable = g_hash_table_lookup(iconHash, iconName);
        if (paintable) {
          g_free(iconName);
          g_object_ref(paintable); // increment ref count...
                                   // This because gtkImage holds a reference
                                   // and when the widget is unparented,
                                   // it seems to call unref on the paintable.
          return GDK_PAINTABLE(paintable);
        }
        auto icon = gtk_icon_theme_lookup_by_gicon(iconTheme, gIcon, 96, 
            1, GTK_TEXT_DIR_NONE,(GtkIconLookupFlags)0);

        g_object_ref(G_OBJECT(icon));
        g_hash_table_insert(iconHash, iconName, GDK_PAINTABLE(icon));

        return GDK_PAINTABLE(icon);
      }
      static
      GdkPaintable *load(GIcon *gIcon){
        if (!gIcon){
          DBG("Texture::load():gicon is null\n");
          return NULL;
        }
        return getIcon(gIcon);
      }
  public:        

      static
      GdkPaintable *load(GFileInfo *info){
        auto gIcon = g_file_info_get_icon(info);
        return load(gIcon);
      }

      static
      GdkPaintable *load(const char *iconName){
      //GdkPaintable *load(const char *iconName){
        if (!iconName){
          DBG("Texture::load(iconName): iconName is NULL\n");
          return NULL;
        }
        return getIcon(iconName);
      }

      static
      GdkPaintable *load16(const char *iconName){
      //GdkPaintable *load(const char *iconName){
        if (!iconName){
          DBG("Texture::load(iconName): iconName is NULL\n");
          return NULL;
        }
        return getIcon16(iconName);
      }

    /* }}} */
    /* {{{ Shading and emblems  */

public:
    static void setShading(cairo_t *cr){
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
        } else {
          // XXX default background color is white. 
          // this may be broken if there is a system defined CSS for gridviews. 
          cairo_set_source_rgba (cr, 1,1,1, .5);
        }
    }

    static GdkPaintable *addEmblem(GdkPaintable *paintable, const char *emblem, double width, double height){
        GtkSnapshot  *snapshot = gtk_snapshot_new();
        graphene_rect_t bounds;
        bounds.origin.x = bounds.origin.y = 0.0;
        bounds.size.width = width;
        bounds.size.height = height;

        gdk_paintable_snapshot (paintable, snapshot, width, height);
        /*if (!emblem){
          DBG("addEmblem()\n");
        }*/
        auto emblemIcon = gtk_icon_theme_lookup_icon(  //GtkIconPaintable*
            iconTheme, emblem,
            NULL, width, 1, GTK_TEXT_DIR_NONE, (GtkIconLookupFlags) 0);
  
        graphene_point_t point;
        point.x = 2*width/3; // width - (width/3);
        point.y = 0.0;
        gtk_snapshot_translate (snapshot, &point);

        gdk_paintable_snapshot (GDK_PAINTABLE(emblemIcon), snapshot, width/3, height/3);
        auto texture = gtk_snapshot_free_to_paintable(snapshot, &(bounds.size));
        return GDK_PAINTABLE(texture);
    }

    static GdkPaintable *addEmblem(GIcon *gIcon, const char *emblem, double width, double height){
        //
        if (!gIcon || !emblem) return NULL;
        auto icon = gtk_icon_theme_lookup_by_gicon(iconTheme, gIcon, width, 
              1, GTK_TEXT_DIR_NONE,(GtkIconLookupFlags)0);
        return addEmblem(GDK_PAINTABLE(icon), emblem, width, height);
    }

    static GdkPaintable *addEmblem(GFileInfo *info, const char *emblem, double width, double height)
    {
        auto gIcon = g_file_info_get_icon(info);
        return addEmblem(gIcon, emblem, width, height);
    }

    static GdkPaintable *addEmblem(const char *iconName, const char *emblem, double width, double height)
    {
        auto icon = gtk_icon_theme_lookup_icon(  //GtkIconPaintable*
            iconTheme, iconName,
            NULL, width, 1, GTK_TEXT_DIR_NONE, (GtkIconLookupFlags) 0);
        return addEmblem(GDK_PAINTABLE(icon), emblem, width, height);
    }
 

    static GdkPaintable *getShadedIcon(GFileInfo *info, double width, double height, const char *emblem)
    {
        auto gIcon = g_file_info_get_icon(info);
        GtkIconPaintable *icon;
        if (gIcon) {
          icon = gtk_icon_theme_lookup_by_gicon(iconTheme, gIcon, 96, 
              1, GTK_TEXT_DIR_NONE,(GtkIconLookupFlags)0);
        } else {
          auto path = Basic::getPath(info);
          DBG("Error:: no GIcon at getShadedIcon(%s)\n", path);
          g_free(path);
          return NULL;
        }
        return getShadedIcon(GDK_PAINTABLE(icon), width, height, emblem);
    }

    static GdkPaintable *getShadedIcon(const char *iconName, double width, double height, const char *emblem)
    {
        auto icon = gtk_icon_theme_lookup_icon(  //GtkIconPaintable*
            iconTheme, iconName,
            NULL, width, 1, GTK_TEXT_DIR_NONE, (GtkIconLookupFlags) 0);
        return getShadedIcon(GDK_PAINTABLE(icon), width, height, emblem);
    }
    
    static GdkPaintable *getShadedIcon(GdkPaintable *icon, double width, double height, const char *emblem)
    {
        GtkSnapshot  *snapshot = gtk_snapshot_new();
        gdk_paintable_snapshot (GDK_PAINTABLE(icon), snapshot, width, height);
        graphene_rect_t bounds;
        bounds.origin.x = bounds.origin.y = 0.0;
        bounds.size.width = width;
        bounds.size.height = height;
        cairo_t *cr = gtk_snapshot_append_cairo(snapshot, &bounds);

        // Uses user defined background color to configure shading.
        setShading(cr);
        // Apply shading mask:
        cairo_set_line_width (cr, 1);
        cairo_rectangle (cr, 0.0, 0.0, width, height);
        cairo_fill (cr);       
        cairo_destroy(cr);
        auto texture = gtk_snapshot_free_to_paintable(snapshot, &(bounds.size));

        // emblem after shading (if emblem)
        if (emblem != NULL) {
          GdkPaintable *emblemed = addEmblem(GDK_PAINTABLE(texture), emblem, width, height);
          g_object_unref(texture);
          return GDK_PAINTABLE(emblemed);
        }
        return GDK_PAINTABLE(texture);
    }
    /* }}} */
    /*{{{ Deprecated */ 
#if 0        

    static const char *locate(const char *name){
      char *tname = gtk_icon_theme_get_theme_name(iconTheme);
      const char *dirs[] = {"places", "mimetypes", "status", "devices", "emblems", "stock", NULL}; 
      // adwaita format.
      for (auto p=dirs; p && *p; p++){
        // adwaita
        auto path = g_strdup_printf("/usr/share/icons/%s/scalable/%s/%s.svg", tname, *p, name);
        if (g_file_test(path, G_FILE_TEST_EXISTS)) return (const char *)path;
        g_free(path);
      }
      // kde breeze format
      for (auto p=dirs; p && *p; p++){
        auto path = g_strdup_printf("/usr/share/icons/%s/%s/16/%s.svg", tname, *p, name);
        if (g_file_test(path, G_FILE_TEST_EXISTS)) return (const char *)path;
        g_free(path);
      }

      // hicolor adwaita format.
      for (auto p=dirs; p && *p; p++){
        // hicolor in /usr
        auto path = g_strdup_printf("/usr/share/icons/hicolor/scalable/%s/%s.svg", *p, name);
        if (g_file_test(path, G_FILE_TEST_EXISTS)) return (const char *)path;
        g_free(path);

        // hicolor in PREFIX
        path = g_strdup_printf("%s/share/icons/hicolor/scalable/%s/%s.svg",PREFIX, *p, name);
        TRACE("locate  %s\n",path);

        if (g_file_test(path, G_FILE_TEST_EXISTS)) return (const char *)path;
        g_free(path);
      }
      return NULL;
     }

    static cairo_surface_t *getCairoSurfaceFromSvg(const char *file, double width, double height){
      GError *error_ = NULL;
      RsvgHandle *handle = rsvg_handle_new_from_file (file, &error_ );
      if (error_) {
        DBG("*** Error: %s\n", error_->message);
        return NULL;
      }
      cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
      cairo_t *cr = cairo_create (surface);
      RsvgRectangle  viewport;
      viewport.x = viewport.y = 0.0;
      viewport.width = width;
      viewport.height = height;
      if (!rsvg_handle_render_document (handle, cr, &viewport, &error_)){
        DBG("*** Error: %s\n", error_->message);
        cairo_destroy(cr);
        cairo_surface_destroy(surface);
        return NULL;
      }
      cairo_destroy(cr);
      return surface;
    }

    static const char *
    findIconPath(const char *name){
      if (!iconPathHash) iconPathHash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
      const char *path = (const char *)g_hash_table_lookup(iconPathHash, name);
      if (path) return path;
      path = locate(name);
      if (path) {
        g_hash_table_insert(iconPathHash, g_strdup(name), (void *)path);
        TRACE("findIconPath hash insert name=%s -> %s\n", name, path);
        return path;
      } else {
        TRACE("*** Warning:findIconPath(): cannot gicon for %s-->%s\n", name, path);
      }
      return path;
    }
    static const char *
    findIconPath(GFileInfo *info){
          auto gIcon = g_file_info_get_icon(info);
          auto tIcon = G_THEMED_ICON(gIcon);
          auto names = g_themed_icon_get_names(tIcon);
          for (auto p=names; p && *p; p++) {
            auto path = findIconPath(*p);
            if (path) return path;
          }
          TRACE("*** Warning: \n");
          for (auto p=names; p && *p; p++) {
            DBG("Texture.hh::findIconPath(): Could not find icon path for   %s\n", *p);
          }
          TRACE("*** Warning: using application-x-generic instead...\n");

          const char *path = locate ("application-x-generic");
          if (path) {
              g_hash_table_insert(iconPathHash, g_strdup("application-x-generic"), (void *)path);
              return path;
          }
          TRACE("*** Warning:findIconPath(): cannot find icon path for %s\n", path);
          return NULL;
    }
    static GdkPaintable *getEmblemedIcon(const char *file, const char *emblem, double width, double height){
      // debug
  /*    findIconPath("*** getSvgPaintable(%s, %lf,%lf)\n", file, width, height);
      auto retval = loadPath(file);
      if (!retval){
        auto base = g_path_get_basename(file);
        if (strchr(base, '.')) *strchr(base, '.') = 0;
        retval = loadIconName(base);
      }
      return retval;*/
        auto surface = getCairoSurfaceFromSvg (file, width, height);
        cairo_t *cr = cairo_create (surface);

        // Uses user defined background color to configure shading.
        setShading(cr);
        // Apply shading mask:
        cairo_set_line_width (cr, 1);
        cairo_rectangle (cr, 0.0, 0.0, width, height);
        cairo_fill (cr);
        cairo_destroy(cr);
        auto texture = GDK_PAINTABLE(gdk_texture_new_for_surface(surface));
        cairo_surface_destroy(surface);
        return  texture;
    }
// with cairo read from svg (do not remove FFI)
    static GdkPaintable *addEmblem(GtkIconPaintable *icon, const char *emblem, double width, double height){
        GtkSnapshot  *snapshot = gtk_snapshot_new();
        graphene_rect_t bounds;
        bounds.origin.x = bounds.origin.y = 0.0;
        bounds.size.width = width;
        bounds.size.height = height;

        gdk_paintable_snapshot (GDK_PAINTABLE(icon), snapshot, width, height);
        cairo_t *cr = gtk_snapshot_append_cairo(snapshot, &bounds);
        double size = 3.0;
        //double size = 4.0;
        auto *emblemPath = Texture<bool>::findIconPath(emblem);
        auto emblemSurface = getCairoSurfaceFromSvg (emblemPath, width/size, height/size);
        cairo_set_source_surface(cr, emblemSurface, 0.0, 0.0);
        cairo_rectangle (cr, 0.0, 0.0, width/size, height/size);
        cairo_fill (cr);

        auto texture = gtk_snapshot_free_to_paintable(snapshot, &(bounds.size));
        cairo_destroy(cr);
        cairo_surface_destroy(emblemSurface);
        return GDK_PAINTABLE(texture);
    }
     // With double read from svg...  (deprecated)
       auto *emblemPath = Texture<bool>::findIconPath(emblem);
        if (!iconPath || !emblemPath) return NULL;
        auto surface = getCairoSurfaceFromSvg (iconPath, width, height);
        double size = 3.0;
        //double size = 4.0;
        auto emblemSurface = getCairoSurfaceFromSvg (emblemPath, width/size, height/size);
        cairo_t *cr = cairo_create (surface);
        cairo_set_source_surface(cr, emblemSurface, 0.0, 0.0);
        cairo_rectangle (cr, 0.0, 0.0, width/size, height/size);
        cairo_fill (cr);

        auto texture = gdk_texture_new_for_surface(surface);
        cairo_destroy(cr);
        cairo_surface_destroy(surface);
        cairo_surface_destroy(emblemSurface);
        return GDK_PAINTABLE(texture);
    }

/*    static GdkPaintable *addEmblem(GFileInfo *info, const char *emblem, double width, double height){
        auto *iconPath = findIconPath(info);
        return addEmblem(iconPath, emblem, width, height);
    }*/

/*    static void addEmblem(cairo_t *cr, const char *emblem, double width, double height){
        if (emblem) {
          auto *emblemPath = Texture<bool>::findIconPath(emblem);
          auto emblemSurface = getCairoSurfaceFromSvg (emblemPath, width/4.0, height/4.0);
          cairo_set_source_surface(cr, emblemSurface, 0.0, 0.0);
          cairo_rectangle (cr, 0.0, 0.0, width/4.0, height/4.0);
          cairo_fill (cr);
        }
    }*/

#endif
    /*}}} */
  };

}

#endif
