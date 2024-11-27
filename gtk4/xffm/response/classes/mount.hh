#ifndef MOUNT_HH
#define MOUNT_HH
// template dependencies

namespace xf {
class  MountOp{
  public:
    static void *asyncYes(void *data, const char *op, GtkTextView *output){
      auto dialogObject = (DialogTimeout<pathResponse<MountOp>> *)data;
      auto dialog = dialogObject->dialog();
      
      auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(dialog), "entry"));
      auto path = (char *)g_object_get_data(G_OBJECT(entry), "path");
      auto buffer = gtk_entry_get_buffer(entry);
      auto text = gtk_entry_buffer_get_text(buffer);

      auto dir = g_path_get_dirname(path);
      auto newFile = g_strconcat(dir, G_DIR_SEPARATOR_S, text, NULL);
      //auto output = Child::getOutput();

      if (!g_file_test(newFile, G_FILE_TEST_EXISTS)){
        auto string = g_strconcat(_("Sorry"), ": ", _("Folder does not exist"), " (\"", text, "\") ", "\n", NULL);
        Print::printError(output, string);
      } else {
        DBG("perform mount operation.\n");
        /*
        if (strcmp(op, "cp") == 0 || strcmp(op, "mv") == 0) {          
          if (g_file_test(path, G_FILE_TEST_IS_DIR)){
            char *arg[]={(char *)op, (char *)"-a", (char *)"-v", path, newFile, NULL};
            Run<bool>::thread_run(output, (const char **)arg, false);
          } else {
            char *arg[]={(char *)op, (char *)"-v", path, newFile, NULL};
            Run<bool>::thread_run(output, (const char **)arg, false);
          }
        } else if (strcmp(op, "ln") == 0) {          
          char *arg[]={(char *)op, (char *)"-s", (char *)"-v", path, newFile, NULL};
          Run<bool>::thread_run(output, (const char **)arg, false);
        }*/
      }              
      g_free(path);
      g_free(dir);
      g_free(newFile);
      return NULL;
    }

};

class  mountResponse: public pathResponse<MountOp> {
   const char *title_;
   const char *iconName_;
public:


    const char *title(void){ return _("Path");}
    const char *iconName(void){ return "dialog-question";}
    const char *label(void){ return _("Mount Device");}

    static void setDefaults(GtkWindow *dialog, GtkLabel *label){

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

      auto string = g_strconcat("<span color=\"green\"><b>",_("Mount Device"), ":\n</b></span><span color=\"blue\"><b>", path, "</b></span>", NULL);
      gtk_label_set_markup(label, string);
      g_free(basename);
      g_free(string);
      g_free(dirname);
      g_free(mountTarget);
    }
    
    
    static void *asyncYes(void *data){
       asyncYesArg(data, "mount");      
       return NULL;
    }
};
}
#endif

