#ifndef FM_HH
#define FM_HH

#include "window/window.hh"

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

    auto xffm = new(xf::MainWindow)(path);
  }

private:
};
}
#endif
