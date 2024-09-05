#ifndef FM_HH
#define FM_HH

#include "window.hh"

namespace xf {
//template <class Type> 
class Fm{
private:

  History *_history;

public:
  History *history(void) { return _history;}
  ~Fm(void){
      //ClipBoard<double>::stopClipBoard();  
  }

  Fm(const char *path){
    // Construct app hash
    MimeApplication::constructAppHash();
    History::init();  
    gtk_init ();

    // This is to avoid crashes on remote x connection which want to use audible bell:
    auto gtksettings = gtk_settings_get_default();
    g_object_set(G_OBJECT(gtksettings), "gtk-error-bell", FALSE, NULL);
    auto css = Util::setCSSprovider();
    gtk_style_context_add_provider_for_display(gdk_display_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_USER); 

    GdkDisplay *displayGdk = gdk_display_get_default();
    icon_theme = gtk_icon_theme_get_for_display(displayGdk);

    /*auto rp = gtk_icon_theme_get_resource_path(icon_theme);
    for (auto p=rp; p && *p; p++){
      TRACE("resourcepath = \"%s\"\n", *p);
    }*/

    auto resource_path = g_build_filename(PREFIX, "share", "icons", "hicolor", "scalable", NULL);
    if (g_file_test(resource_path, G_FILE_TEST_IS_DIR)) {
      gtk_icon_theme_add_resource_path(icon_theme, resource_path);// not always needed, maybe sometimes...
 
      const gchar *subdirs[] = {
                "emblems",
                "stock",
                NULL
      };
      for (auto p=subdirs; p && *p; p++){
          auto path = g_build_filename(resource_path, *p, NULL);
          gtk_icon_theme_add_search_path (icon_theme, path);
          TRACE("added search path=\"%s\"\n", path);
          g_free(path);
      }
      //TRACE("added resource path=\"%s\"\n", resource_path);
    }
    g_free(resource_path);


    auto xffm = new(xf::MainWindow<FMbuttonBox, FMpage>)(path);
    // It works!
    //auto xffm = new(xf::MainWindow<EmptyButtonBox>)(path);
  }

public:
};
}
#endif
