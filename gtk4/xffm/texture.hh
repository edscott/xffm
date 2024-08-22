#ifndef XF_TEXTURE_HH
#define XF_TEXTURE_HH
namespace xf {
    class Texture {
      public:
      static
      GdkTexture *load(const char *item){
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
          return texture;
        }
        // From iconname.
        return NULL;
      }
      private:
    };


}

#endif
