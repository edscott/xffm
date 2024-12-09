#ifndef MOUNT_HH
#define MOUNT_HH
// template dependencies

namespace xf {

class  mountResponse: public pathResponse {
   const char *title_;
   const char *iconName_;
public:


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
              dirname = g_strdup_printf("/tmp/%s", basename);
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
    
    
    static void *asyncYes(void *data){
       asyncYesArg(data, "mount");      
       return NULL;
    }
};
}
#endif

