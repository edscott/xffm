#ifndef RMRESPONSE_HH
#define RMRESPONSE_HH
namespace xf {

class rmResponse {
  private:
    GtkButton *button_[4];
public:
    const char *title(void){ return _("Delete");}
    const char *iconName(void){ return "dialog-question";}
    const char *label(void){ return _("Delete");
    }

    ~rmResponse(void){
    }
    rmResponse(void){
      button_[0] = Basic::mkButton("dialog-warning", _("Shred")); //1
      button_[1] = Basic::mkButton("delete", _("Delete")); //2
      button_[2] = Basic::mkButton("user-trash", _("Trash")); //3
      button_[3] = NULL;
    }
    GtkButton **getButtons(void){ return button_;}
    
    static void setDefaults(GtkWindow *dialog, GtkLabel *label){
      auto info = G_FILE_INFO(g_object_get_data(G_OBJECT(dialog), "info"));
      auto path = Basic::getPath(info);
      auto base = g_path_get_basename(path);
      auto string = g_strconcat("<span color=\"red\"><b>",_("Delete"), ":</b></span>\n<span color=\"blue\"><b>", base, "</b></span>", NULL);
      gtk_label_set_markup(label, string);
      g_free(path);
      g_free(base);
      g_free(string);
    }
    
    static void *asyncYes(void *data){
      auto dialogObject = (DialogTimeout<rmResponse> *)data;
      auto dialog = dialogObject->dialog();
      auto info = G_FILE_INFO(g_object_get_data(G_OBJECT(dialog), "info"));
      auto file = G_FILE(g_file_info_get_attribute_object (info, "standard::file"));
      auto path = Basic::getPath(info);
      int response = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(dialog), "response"));
      GError *error_ = NULL;
      auto output = Child::getOutput();
      switch (response){
        case 1:
          {
              Print::showText(output);
              char *arg[]={(char *)"shred", (char *)"-fzuv", path, NULL};
              Run<bool>::thread_run(output, (const char **)arg, false);
          }
          break;
        case 2:
          {
              char *arg[]={(char *)"rm", (char *)"-frv", (char *)"--preserve-root=all", (char *)"--one-file-system",path, NULL};
              Run<bool>::thread_run(output, (const char **)arg, false);
          }
          break;
        case 3:
          if (!g_file_trash(file, NULL, &error_)){
            DBG("*** Error:: %s: %s\n", path, error_->message);
          }
          break;
      }

      g_free(path);
      return NULL;
    }
    static void *asyncNo(void *data){
      return NULL;
    }
};

}
#endif
