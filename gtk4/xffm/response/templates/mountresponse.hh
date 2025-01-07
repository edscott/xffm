#ifndef MOUNTRESPONSE_HH
#define MOUNTRESPONSE_HH

namespace xf {

template <class Type>
class  mountResponse {
   using subClass_t = mountResponse<Type>;
   const char *title_;
   const char *iconName_;
   GtkWindow *dialog_ = NULL;
   char *mountDir_ = NULL;
   char *folder_ = NULL;
   const char *path_ = NULL;
   GtkBox *mainBox_ = NULL;
   GtkEntry *remoteEntry_ = NULL;
  public:
    // Set a pointer to the GtkWindow in the FileResponse
    // object so that it can be referred to in the
    // async main context thread callbacks.
    // 
    GtkEntry *remoteEntry(void){return remoteEntry_;}
    void dialog(GtkWindow *value){ dialog_ = value; }
    GtkWindow *dialog(void){return dialog_;}
public:
    char *folder(){return  folder_;}
    void folder(const char *value){folder_ = value;}
    
    ~mountResponse(void){
     TRACE("*** ~mountResponse...\n");
      g_free(folder_);
      g_free(mountDir_);
    }
    mountResponse(void){
     TRACE("*** mountResponse...\n");
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
    GtkBox *mainBox(const char *folder, const char *path) {
        folder_ = folder? g_strdup(folder) : g_strdup("/");
        path_ = path? path : "/";
        mainBox_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        gtk_widget_set_size_request(GTK_WIDGET(mainBox_), 450, -1);
        gtk_widget_set_vexpand(GTK_WIDGET(mainBox_), false);
        gtk_widget_set_hexpand(GTK_WIDGET(mainBox_), false);

        auto hbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_vexpand(GTK_WIDGET(hbox), false);
        gtk_widget_set_hexpand(GTK_WIDGET(hbox), true);
        gtk_box_append(mainBox_, GTK_WIDGET(hbox));

        //auto paintable = Texture<Type>::load("drive-harddisk");    
        //auto image = gtk_image_new_from_paintable(paintable);
        //gtk_widget_set_size_request(image, 48, 48);
        auto image = Texture<bool>::getImage(HARD_DISK, 48);
        gtk_box_append(hbox, GTK_WIDGET(image));

       

        auto basename = g_path_get_basename(path_);
        char *shortLabel = FstabUtil::e2Label(path_);
        char *mountTarget = NULL;

        auto labelTxt = g_strdup_printf("LABEL=%s", shortLabel);
        g_free(shortLabel);
        TRACE("is in fstab \"%s\" or \"%s\"\n", labelTxt, path_);

        // mountTarget is the fstab file defined mount point.
        if (FstabUtil::isInFstab(labelTxt)) {
            TRACE("is in fstab OK \"%s\"\n", labelTxt);
            mountTarget = FstabUtil::mountTarget(labelTxt);
        }
        g_free(labelTxt);
        if (!mountTarget && FstabUtil::isInFstab(path_)) {
            TRACE("is in fstab OK \"%s\"\n", path_);
            mountTarget = FstabUtil::mountTarget(path_);
        }
        auto mountSrc = FstabUtil::mountSrc(mountTarget);
        TRACE("mountTarget=%s, mountSrc=%s\n", mountTarget, mountSrc);

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

        auto label = gtk_label_new("");
        auto string = g_strconcat("<span color=\"blue\"><b>",_("Mount Device"), ":\n</b></span>", mountSrc?mountSrc:path_, NULL);
        gtk_label_set_markup(GTK_LABEL(label), string);
        g_free(string);
        gtk_box_append(hbox, GTK_WIDGET(label));

        
/*
        auto text2 = g_strconcat(_("Mount Volume:"), NULL);        
        auto label = gtk_label_new(text2);
        g_free(text2);
        gtk_box_append(hbox, GTK_WIDGET(label));*/

        // mount point entry
        auto text = g_strconcat(_("Mount Point"), ": ",NULL);
        TRACE("subClass folder =%s, %s\n", folder_, this->folder());
        remoteEntry_ = FileResponse<Type, subClass_t>::addEntry(mainBox_, "entry1", text, this);
        g_free(text);
        //gtk_widget_set_sensitive(GTK_WIDGET(remoteEntry_), true); // FIXME: put to false 

        auto action_area = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_vexpand(GTK_WIDGET(action_area), false);
        gtk_widget_set_hexpand(GTK_WIDGET(action_area), false);
        gtk_box_append(mainBox_, GTK_WIDGET(action_area));

        auto cancelButton = Basic::mkButton("emblem-redball", _("Cancel"));
        gtk_box_append(action_area,  GTK_WIDGET(cancelButton));
        gtk_widget_set_vexpand(GTK_WIDGET(cancelButton), false);

        auto mountButton = Basic::mkButton ("emblem-greenball", _("Mount"));
        gtk_box_append(action_area,  GTK_WIDGET(mountButton));
        gtk_widget_set_vexpand(GTK_WIDGET(mountButton), false);
     

        g_signal_connect (G_OBJECT (mountButton), "clicked", G_CALLBACK (button_mount), this);
        g_signal_connect (G_OBJECT (cancelButton), "clicked", G_CALLBACK (button_cancel), this);
      
        g_free(basename);
        g_free(dirname);
        g_free(mountTarget);
        g_free(mountSrc);

        return mainBox_;
    }

    const char *title(void){ return _("Mount Volume");}
    const char *iconName(void){ return "dialog-question";}
    const char *label(void){ return _("Mount Device");}

 /*   static void setDefaults(GtkWindow *dialog, GtkLabel *label){

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
        TRACE("is in fstab \"%s\" or \"%s\"\n", labelTxt, path);

        // mountTarget is the fstab file defined mount point.
        if (FstabUtil::isInFstab(labelTxt)) {
            TRACE("is in fstab OK \"%s\"\n", labelTxt);
            mountTarget = FstabUtil::mountTarget(labelTxt);
        }
        g_free(labelTxt);
        if (!mountTarget && FstabUtil::isInFstab(path)) {
            TRACE("is in fstab OK \"%s\"\n", path);
            mountTarget = FstabUtil::mountTarget(path);
        }
        auto mountSrc = FstabUtil::mountSrc(mountTarget);
        TRACE("mountTarget=%s, mountSrc=%s\n", mountTarget, mountSrc);

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

*/

    static void *asyncNo(void *data){
      DBG("mountResponse asyncNo\n"); 
       return NULL;

    }
    
    
    static void *asyncYes(void *data){
      DBG("FIXME: do the mount\n"); 
      //asyncYesArg(data, "mount");      
       return NULL;
    }
private:

    static void
    button_mount (GtkButton * button, gpointer data) {
      auto subClass = (subClass_t *)data;
      g_object_set_data(G_OBJECT(subClass->dialog()), "response", GINT_TO_POINTER(1));
    }

    static void
    button_cancel (GtkButton * button, gpointer data) {
      auto subClass = (subClass_t *)data;
      g_object_set_data(G_OBJECT(subClass->dialog()), "response", GINT_TO_POINTER(-1));
    }

};
}
#endif

