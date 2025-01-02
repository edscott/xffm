#ifndef MOUNT_HH
#define MOUNT_HH
// template dependencies

namespace xf {

//template <class Type>
class  mountResponse {
   const char *title_;
   const char *iconName_;
   GtkWindow *dialog_ = NULL;
   char *mountDir_ = NULL;
   const char *folder_ = NULL;
   GtkBox *mainBox_ = NULL;
  public:
    // Set a pointer to the GtkWindow in the FileResponse
    // object so that it can be referred to in the
    // async main context thread callbacks.
    // 
    void dialog(GtkWindow *value){ dialog_ = value; }
    GtkWindow *dialog(void){return dialog_;}
public:
    const char *folder(){return  folder_;}
    void folder(const char *value){folder_ = value;}
    
    ~mountResponse(void){
      g_free(mountDir_);
    }
    mountResponse(void){
      mountDir_ = g_strconcat(g_get_home_dir(), G_DIR_SEPARATOR_S, "mnt", NULL);
      if (g_file_test(mountDir_, G_FILE_TEST_EXISTS) &&
          !g_file_test(mountDir_, G_FILE_TEST_IS_DIR)){
        g_free(mountDir_);
        mountDir_ = g_strdup("/tmp");
      } else if (!g_file_test(mountDir_, G_FILE_TEST_IS_DIR)){
        if (mkdir(mountDir_, 0750) < 0){
          TRACE("mkdir %s: %s\n", mountDir_, strerror(errno));
        }
      }
    }
    GtkBox *mainBox(const char *folder) {
        mainBox_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));

        return mainBox_;
    }

    const char *title(void){ return _("Mount Volume");}
    const char *iconName(void){ return "dialog-question";}
    const char *label(void){ return _("Mount Device");}

    static void setDefaults(GtkWindow *dialog, GtkLabel *label){

      gtk_window_set_decorated(dialog, true);
      auto entryLabel = GTK_LABEL( g_object_get_data(G_OBJECT(dialog),"entryLabel"));
      gtk_label_set_markup(entryLabel, _("Mount point:"));
      auto entry = GTK_ENTRY( g_object_get_data(G_OBJECT(dialog),"entry"));
      auto path = (const char *)g_object_get_data(G_OBJECT(entry), "path");
      auto basename = g_path_get_basename(path);

        char *shortLabel = FstabUtil::e2Label(path);
        char *mountTarget = NULL;

        auto labelTxt = g_strdup_printf("LABEL=%s", shortLabel);
        g_free(shortLabel);
        DBG("is in fstab \"%s\" or \"%s\"\n", labelTxt, path);

        // mountTarget is the fstab file defined mount point.
        if (FstabUtil::isInFstab(labelTxt)) {
            DBG("is in fstab OK \"%s\"\n", labelTxt);
            mountTarget = FstabUtil::mountTarget(labelTxt);
        }
        g_free(labelTxt);
        if (!mountTarget && FstabUtil::isInFstab(path)) {
            DBG("is in fstab OK \"%s\"\n", path);
            mountTarget = FstabUtil::mountTarget(path);
        }
        auto mountSrc = FstabUtil::mountSrc(mountTarget);
        DBG("mountTarget=%s, mountSrc=%s\n", mountTarget, mountSrc);

        // If no fstab file defined mount point, dirname is the suggested user mount point.
        char *dirname = NULL;
        if (!mountTarget) {
          if (Settings::keyFileHasGroupKey("MountPoints", basename)){
              dirname = Settings::getString("MountPoints", basename);
          } 
          if (!dirname || !g_file_test(dirname, G_FILE_TEST_IS_DIR) ) {
              g_free(dirname);
              dirname = g_strconcat(g_get_home_dir(),G_DIR_SEPARATOR_S,"mnt",
                  G_DIR_SEPARATOR_S, basename, NULL);
          }         
        }



      auto buffer = gtk_entry_get_buffer(entry);
      gtk_entry_buffer_set_text(buffer, mountTarget?mountTarget:dirname, -1);

      auto string = g_strconcat("<span color=\"green\"><b>",_("Mount Device"), ":\n</b></span><span color=\"blue\"><b>", mountSrc?mountSrc:path, "</b></span>", NULL);
      gtk_label_set_markup(label, string);
      g_free(basename);
      g_free(string);
      g_free(dirname);
      g_free(mountTarget);
      g_free(mountSrc);
    }
    static void *asyncNo(void *data){
      DBG("mountResponse asyncNo\n"); 
       return NULL;

    }
    
    
    static void *asyncYes(void *data){
      DBG("FIXME: do the mount\n"); 
      //asyncYesArg(data, "mount");      
       return NULL;
    }
};

class Mount {
    char *folder_ = NULL;
    public:
  Mount(const char *folder){
      folder_ = g_strdup(folder);
      auto dialogObject = new DialogComplex<mountResponse>(folder);
      //auto dialogObject = new DialogEntryPath<mountResponse>(folder);
      
      dialogObject->setParent(GTK_WINDOW(MainWidget));
      auto dialog = dialogObject->dialog();
      gtk_window_set_decorated(dialog, true);
      dialogObject->setSubClassDialog();

      gtk_widget_realize(GTK_WIDGET(dialog));
      Basic::setAsDialog(GTK_WIDGET(dialog), "dialog", "Dialog");
      gtk_window_present(dialog);

      dialogObject->run();
    }
  ~Mount(void){
      g_free(folder_);
  }

};
}
#endif

