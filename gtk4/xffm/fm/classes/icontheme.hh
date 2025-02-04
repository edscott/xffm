#ifndef ICONTHEME_HH
#define ICONTHEME_HH
static GtkIconTheme *iconTheme=NULL;
namespace xf {
  class IconTheme  {

public:
    static void init(void){
#if 0
      GdkDisplay *displayGdk = gdk_display_get_default();
      iconTheme = gtk_icon_theme_get_for_display(displayGdk);
//      auto iconThemeName = gtk_icon_theme_get_theme_name(iconTheme);
//      DBG("*** System icon theme: %s\n", gtk_icon_theme_get_theme_name(iconTheme));
//      g_free(iconThemeName);
#else
      iconTheme = gtk_icon_theme_new();
     // gtk_icon_theme_set_theme_name(iconTheme, "Humanity");
      gtk_icon_theme_set_theme_name(iconTheme, "Adwaita");
     // gtk_icon_theme_set_theme_name(iconTheme, "Oxygen_Blue");
#endif
      
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
   
        //gtk_icon_theme_add_search_path (iconTheme, resource_path);
        const gchar *subdirs[] = {
                  "emblems",
                  "stock",
                  NULL
        };
        for (auto p=subdirs; p && *p; p++){
            auto path = g_build_filename(resource_path, *p, NULL);
            gtk_icon_theme_add_search_path (iconTheme, path);
            DBG("added search path=\"%s\"\n", path);
            g_free(path);
        }
        DBG("added resource path=\"%s\"\n", resource_path);
      }
      g_free(resource_path);

    /*
      const char *rp[] = {"/usr/local/hicolor/scalable/emblems",
                          "/usr/local/hicolor/scalable/stock",NULL};
      gtk_icon_theme_set_resource_path(iconTheme, rp);
    
    */
    }
  };
}
#endif
