#ifndef XF_TEXTURE_HH
#define XF_TEXTURE_HH
/* vim:set foldmethod=marker expandtab: */
//static bool greenLightPreview = true;
namespace xf {
  GHashTable *iconHash = NULL;
template <class Type> class Preview;
template <class Type>  class Texture {
  public:

    static GtkImage *getImage(const char *iconName){
      auto paintable = GDK_PAINTABLE(load(iconName, 24));
      auto image = gtk_image_new_from_paintable(paintable);
      return GTK_IMAGE(image);
    }

    static GtkImage *getImage(const char *iconName, int size){
      auto paintable = GDK_PAINTABLE(lookupIcon(iconName, size));
      auto image = gtk_image_new_from_paintable(paintable);
      gtk_widget_set_size_request(GTK_WIDGET(image), size, size);
      return GTK_IMAGE(image);
    }

    static GtkImage *getImage(GFileInfo *info, int size){
      auto gIcon = g_file_info_get_icon(info);
      auto paintable = getIcon(gIcon, size);
      auto image = gtk_image_new_from_paintable(paintable);
      gtk_widget_set_size_request(GTK_WIDGET(image), size, size);
      return GTK_IMAGE(image);
    }

  private:

      static GtkIconPaintable *findIconInHash(const char *iconName, int size){
        static bool initialized = false;
        if (!initialized){
          initialized = true;
          iconHash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);
        }

        auto key = g_strdup_printf("%s-%d", iconName, size);
        auto paintable = g_hash_table_lookup(iconHash, key);
        g_free(key);
        if (paintable) {
          g_object_ref(paintable); // increment ref count...
                                   // This because gtkImage holds a reference
                                   // and when the widget is unparented,
                                   // it seems to call unref on the paintable.
          return GTK_ICON_PAINTABLE(paintable);
        }
        return NULL;
      }

      static  GtkIconPaintable *lookupIcon(const char *iconName, int size){
        auto paintable = findIconInHash(iconName, size);        
        if (paintable) return paintable;

        TRACE("*** Lookup icon \"%s-%d\"\n", iconName, size);
        auto icon = gtk_icon_theme_lookup_icon(  //GtkIconPaintable*
            iconTheme, iconName,
            NULL, size, 1, GTK_TEXT_DIR_NONE, (GtkIconLookupFlags) 0);
        g_object_ref(G_OBJECT(icon));

        auto key = g_strdup_printf("%s-%d", iconName, size);
        g_hash_table_insert(iconHash, key, (void *)icon);
        return GTK_ICON_PAINTABLE(icon);
      }

       static GdkPaintable *getIcon(const char *iconName, int width){
        return GDK_PAINTABLE(lookupIcon(iconName, width));
       }

//      static GdkPaintable *getIcon(GIcon *gIcon){
      static GdkPaintable *getIcon(GIcon *gIcon, int width){
        auto key = g_icon_hash(gIcon);
        auto iconName = g_strdup_printf("%d-%d", key, width);
        
        GdkPaintable *paintable = GDK_PAINTABLE(findIconInHash(iconName, width));
        if (paintable){
          g_free(iconName);
          return paintable;
        }

        auto icon = gtk_icon_theme_lookup_by_gicon(iconTheme, gIcon, width, 
            1, GTK_TEXT_DIR_NONE,(GtkIconLookupFlags)0);
        
        g_object_ref(G_OBJECT(icon));
        g_hash_table_insert(iconHash, iconName, (void *)icon);

        return GDK_PAINTABLE(icon);
      }
      static
      GdkPaintable *load(GIcon *gIcon, int size){
        if (!gIcon){
          ERROR_("Texture::load():gicon is null\n");
          return NULL;
        }
        return getIcon(gIcon, size);
      }
  public:        

      static
      GdkPaintable *load(GFileInfo *info, int size){
        auto gIcon = g_file_info_get_icon(info);
        return load(gIcon, size);
      }

      static
      GdkPaintable *load(const char *iconName, int size){
      //GdkPaintable *load(const char *iconName){
        if (!iconName){
          ERROR_("Texture::load(iconName): iconName is NULL\n");
          return NULL;
        }
        return getIcon(iconName, size);
      }

      static
      GdkPaintable *load16(const char *iconName){
      //GdkPaintable *load(const char *iconName){
        if (!iconName){
          ERROR_("Texture::load(iconName): iconName is NULL\n");
          return NULL;
        }
        return getIcon(iconName, 16);
      }


public:
    static void setShading(cairo_t *cr){
        // This may be broken if there is a system defined CSS for gridviews. 
        auto string = Settings::getString("xfterm", "iconsBg", "#ffffff");
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
          Basic::Exit("setShading(): this should never happen... Texture::setShading().\n");
          // XXX default background color is white. 
          // cairo_set_source_rgba (cr, 1,1,1, .5);
        }
        g_free(string);
    }

    static GdkPaintable *addEmblem(GdkPaintable *paintable, const char *emblem, double width, double height){
        GtkSnapshot  *snapshot = gtk_snapshot_new();
        graphene_rect_t bounds;
        bounds.origin.x = bounds.origin.y = 0.0;
        bounds.size.width = width;
        bounds.size.height = height;

        gdk_paintable_snapshot (paintable, snapshot, width, height);
        auto emblemIcon = lookupIcon(emblem, width);  
//        auto emblemIcon = lookupIcon(emblem, width);  
        graphene_point_t point;
        point.x = width/2; // width - (width/2);
//        point.x = 2*width/3; // width - (width/3);
        point.y = 0.0;
        gtk_snapshot_translate (snapshot, &point);

        gdk_paintable_snapshot (GDK_PAINTABLE(emblemIcon), snapshot, width/2, height/2);
//        gdk_paintable_snapshot (GDK_PAINTABLE(emblemIcon), snapshot, width/3, height/3);
        auto texture = gtk_snapshot_free_to_paintable(snapshot, &(bounds.size));
        return GDK_PAINTABLE(texture);
    }

    static GdkPaintable *addEmblem(GIcon *gIcon, const char *emblem, double width, double height){
        //
        if (!gIcon || !emblem) return NULL;
        auto icon = getIcon(gIcon, width);
        return addEmblem(GDK_PAINTABLE(icon), emblem, width, height);
    }

    static GdkPaintable *addEmblem(GFileInfo *info, const char *emblem, double width, double height)
    {
        auto gIcon = g_file_info_get_icon(info);
        if (gIcon == NULL) {
          auto paintable = getIcon(EMBLEM_BROKEN, width);
          return addEmblem(paintable, emblem, width, height);
        }
        return addEmblem(gIcon, emblem, width, height);
    }

    static GdkPaintable *addEmblem(const char *iconName, const char *emblem, double width, double height)
    {
        auto icon = lookupIcon(iconName, width);
        return addEmblem(GDK_PAINTABLE(icon), emblem, width, height);
    }
 

    static GdkPaintable *getShadedIcon(GFileInfo *info, double width, double height, const char *emblem)
    {
        auto gIcon = g_file_info_get_icon(info);
        GdkPaintable *icon;
        if (gIcon) {
          icon = getIcon(gIcon, width);
          //icon = gtk_icon_theme_lookup_by_gicon(iconTheme, gIcon, 96, 1, GTK_TEXT_DIR_NONE,(GtkIconLookupFlags)0);
        } else {
          auto path = Basic::getPath(info);
          ERROR_("Error:: no GIcon at getShadedIcon(%s)\n", path);
          g_free(path);
          return NULL;
        }
        return getShadedIcon(GDK_PAINTABLE(icon), width, height, emblem);
    }

    static GdkPaintable *getShadedIcon(const char *iconName, double width, double height, const char *emblem)
    {
        TRACE("*** Lookup icon \"%s\"\n", iconName);
        auto icon = lookupIcon(iconName, width);
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
      
  };
}
#endif
