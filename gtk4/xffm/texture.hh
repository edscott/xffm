#ifndef XF_TEXTURE_HH
#define XF_TEXTURE_HH
namespace xf {
    class Texture {
      public:
      static
      GdkPaintable *load(const char *item){
        GError *error_ = NULL;
        if (!item) return NULL;
        if (g_file_test(item, G_FILE_TEST_EXISTS)) {
          // both absolute and relative here
          // FIXME: run in thread
          auto texture = gdk_texture_new_from_filename(item, &error_);
          if (error_){
            DBG("Texture::load(): %s\n", error_->message);
            return NULL;
          }
          return GDK_PAINTABLE(texture);
        }
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
       // auto v = gtk_icon_theme_get_resource_path(icon_theme);
      /*  auto v = gtk_icon_theme_get_search_path(icon_theme);
        for (char **p=v; p && *p; p++){
          DBG("search path=%s\n", *p);
        }
        g_strfreev(v);

        gtk_icon_theme_get_theme_name(icon_theme);*/

        return NULL;
      }
      private:
    };


}

#endif
