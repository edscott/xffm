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
    //MimeApplication<Type>::constructAppHash();
    History::init();  
    gtk_init ();

    // This is to avoid crashes on remote x connection which want to use audible bell:
    auto gtksettings = gtk_settings_get_default();
    g_object_set(G_OBJECT(gtksettings), "gtk-error-bell", FALSE, NULL);
    setCSSprovider();

    GdkDisplay *displayGdk = gdk_display_get_default();
    icon_theme = gtk_icon_theme_get_for_display(displayGdk);

    /*auto rp = gtk_icon_theme_get_resource_path(icon_theme);
    for (auto p=rp; p && *p; p++){
      DBG("resourcepath = \"%s\"\n", *p);
    }*/

    auto resource_path = g_build_filename(PREFIX, "share", "icons", "xffm+", NULL);
    if (g_file_test(resource_path, G_FILE_TEST_IS_DIR)) {
      gtk_icon_theme_add_resource_path(icon_theme, resource_path);// not always needed, maybe sometimes...
 
      const gchar *subdirs[] = {
                "scalable",
                "96x96",
                "48x48",
                "24x24",
                NULL
      };
      for (auto p=subdirs; p && *p; p++){
          auto path = g_build_filename(resource_path, *p, NULL);
          gtk_icon_theme_add_search_path (icon_theme, path);
          g_free(path);
      }
      TRACE("added resource path=\"%s\"\n", resource_path);
    }
    g_free(resource_path);


    auto xffm = new(xf::MainWindow<FMbuttonBox, FMpage>)(path);
    // It works!
    //auto xffm = new(xf::MainWindow<EmptyButtonBox>)(path);
  }

private:
  void setCSSprovider(void){
    GtkCssProvider *css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string (css_provider, 
    "\
    .vbox {\
      background-color: #888888;\
      border-width: 0px;\
      border-radius: 0px;\
      border-color: transparent;\
    }\
    .output {\
      background-color: #bbbbbb;\
      color: #111111;\
    }\
    .input {\
      background-color: #dddddd;\
      color: #000000;\
    }\
    .prompt {\
      background-color: #333333;\
      color: #00ff00;\
    }\
    .tooltip {\
      border-width: 0px;\
      border-radius: 0px;\
      border-color: transparent;\
    }\
    .pathbarbox * {\
      background-color: #dcdad5;\
      border-width: 0px;\
      border-radius: 0px;\
      border-color: transparent;\
    }\
    .pathbarboxNegative * {\
      background-color: #acaaa5;\
      border-width: 0px;\
      border-radius: 0px;\
      border-color: transparent;\
    }\
    ");
    gtk_style_context_add_provider_for_display(gdk_display_get_default(),GTK_STYLE_PROVIDER(css_provider),GTK_STYLE_PROVIDER_PRIORITY_USER); 
  }
};
}
#endif
