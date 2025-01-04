#ifndef ICONTHEME_HH
#define ICONTHEME_HH
static GtkIconTheme *iconTheme=NULL;
namespace xf {
  class IconTheme  {

public:
    static void init(void){
      GdkDisplay *displayGdk = gdk_display_get_default();
      iconTheme = gtk_icon_theme_get_for_display(displayGdk);
      auto iconThemeName = gtk_icon_theme_get_theme_name(iconTheme);
      DBG("*** System icon theme: %s\n", gtk_icon_theme_get_theme_name(iconTheme));
      g_free(iconThemeName);
      addResource();
    }

private:
        
    static void 
    addResource(){
      /*auto rp = gtk_icon_theme_get_resource_path(iconTheme);
      for (auto p=rp; p && *p; p++){
        TRACE("resourcepath = \"%s\"\n", *p);
      }*/

      auto resource_path = g_build_filename(PREFIX, "share", "icons", "hicolor", "scalable", NULL);
      if (g_file_test(resource_path, G_FILE_TEST_IS_DIR)) {
        gtk_icon_theme_add_resource_path(iconTheme, resource_path);// not always needed, maybe sometimes...
   
        const gchar *subdirs[] = {
                  "emblems",
                  "stock",
                  NULL
        };
        for (auto p=subdirs; p && *p; p++){
            auto path = g_build_filename(resource_path, *p, NULL);
            gtk_icon_theme_add_search_path (iconTheme, path);
            TRACE("added search path=\"%s\"\n", path);
            g_free(path);
        }
        //TRACE("added resource path=\"%s\"\n", resource_path);
      }
      g_free(resource_path);

    }
    
  };
}
#endif
