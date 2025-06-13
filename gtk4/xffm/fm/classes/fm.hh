#ifndef FM_HH
#define FM_HH


namespace xf {
class Fm{
private:

  History *_history;
  MainWindow<LocalDir> *xffm_;
  GtkWindow *mainWindow_ = NULL;
public:
    History *history(void) { return _history;}
    GtkWidget *mainWidget(void){return GTK_WIDGET(mainWindow_);}
    ~Fm(void){
      //delete xffm_;
        //ClipBoard<double>::stopClipBoard();  
    }

    Fm(const char *path, bool doFind){
      TRACE("*** doFind = %d\n", doFind);
      // Construct app hash
      MimeApplication::constructAppHash();
      History::init();  
      gtk_init ();

      // This is to avoid crashes on remote x connection which want to use audible bell:
      auto gtksettings = gtk_settings_get_default();
      g_object_set(G_OBJECT(gtksettings), "gtk-error-bell", FALSE, NULL);

      CSS::init();
      IconTheme::init();
      Basic::setPasswordPrompt();
      Basic::setEditor();
      Basic::setTerminal();


      if (doFind){ // find dialog
        //xffm_->hideWindow();
        {
          using subClass_t = FindResponse<bool>;
          using dialog_t = DialogComplex<subClass_t>;
          //Dialogs::info("find in files, test");
          auto dialogObject = new dialog_t(NULL, path);
          mainWindow_ = dialogObject->dialog();
          Child::mainWidget(GTK_WIDGET(mainWindow_));
        }
        return;
      }
      
      //g_object_set_data(G_OBJECT(Child::mainWidget()), "MainWindow", xffm_);
      //xffm_ = new(xf::MainWindow<LocalDir>)(path, doFind); 
      xffm_ = new(xf::MainWindow<LocalDir>)(path, doFind); 

      //mainWindow_ = xffm_->mainWindow();
      //Child::mainWidget(GTK_WIDGET(mainWindow_));
    }

private:


};
}

#endif
