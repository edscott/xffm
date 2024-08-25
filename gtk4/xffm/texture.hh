#ifndef XF_TEXTURE_HH
#define XF_TEXTURE_HH
namespace xf {
    class Texture {
      public:
      static
      GdkPaintable *load(GFileInfo *info){
        auto gIcon = g_file_info_get_icon(info);
        GdkPaintable *texture;
        if (gIcon) {
          auto icon = gtk_icon_theme_lookup_by_gicon(icon_theme, gIcon, 96, 
              1, GTK_TEXT_DIR_NONE,(GtkIconLookupFlags)0);
          texture = GDK_PAINTABLE(icon);
        } else return NULL;
        return GDK_PAINTABLE(texture);
      }
      static 
      GdkPaintable *load(const char *item){
        GError *error_ = NULL;
        if (!item) return NULL;
        
        GdkTexture *texture = NULL;
        if (g_file_test(item, G_FILE_TEST_EXISTS)) {
          // both absolute and relative here
          texture = gdk_texture_new_from_filename(item, &error_);
          if (error_){
            TRACE("Texture::load(): %s\n", error_->message);
            g_error_free(error_);
            return NULL;
          }
          return GDK_PAINTABLE(texture);
        }
        return NULL;
      }
     static 
      GdkPaintable *loadIconName(const char *item){
        if (!item) return NULL;      
        // From iconname.
        int size = Settings::getInteger("xfterm", "iconsize");
        auto icon = gtk_icon_theme_lookup_icon (icon_theme,
                       item,  
                       NULL, // const char** fallbacks,
                       size,
                       1,    // int scale,
                       GTK_TEXT_DIR_NONE, 
                       (GtkIconLookupFlags) 0);

        return GDK_PAINTABLE(icon);
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
        
       private:
    };


}

#endif
