#ifndef FM_HH
#define FM_HH


namespace xf {
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

    CSS::init();
    IconTheme::init();

    auto xffm = new(xf::MainWindow)(path);
  }

public:
};
}

#endif
