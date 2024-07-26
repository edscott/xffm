#ifndef FM_HH
#define FM_HH

#include "window.hh"

namespace xf {
//template <class Type> 
class Fm{
private:



public:

  ~Fm(void){
      //ClipBoard<double>::stopClipBoard();  
  }

  Fm(const char *path){
    // Construct app hash
    //MimeApplication<Type>::constructAppHash();
      
    gtk_init ();
    // This is to avoid crashes on remote x connection which want to use audible bell:
    auto gtksettings = gtk_settings_get_default();
    g_object_set(G_OBJECT(gtksettings), "gtk-error-bell", FALSE, NULL);
    setCSSprovider();

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
    ");
    gtk_style_context_add_provider_for_display(gdk_display_get_default(),GTK_STYLE_PROVIDER(css_provider),GTK_STYLE_PROVIDER_PRIORITY_USER); 
  }
};
}
#endif
