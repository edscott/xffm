#ifndef RMLISTRESPONSE_HH
#define RMLISTRESPONSE_HH
namespace xf {

class rmListResponse {
  private:
    GtkButton *button_[4];
public:
    const char *title(void){ return _("Delete");}
    const char *iconName(void){ return "dialog-question";}
    const char *label(void){ return _("Delete");
    }

    ~rmListResponse(void){
    }
    rmListResponse(void){
      button_[0] = UtilBasic::mkButton("dialog-warning", _("Shred")); //1
      button_[1] = UtilBasic::mkButton("delete", _("Delete")); //2
      button_[2] = UtilBasic::mkButton("user-trash", _("Trash")); //3
      button_[3] = NULL;
    }
    GtkButton **getButtons(void){ return button_;}
    
    static void setDefaults(GtkWindow *dialog, GtkLabel *label){
      auto string = g_strconcat(
          "<span color=\"red\" size=\"large\"><b>",_("WARNING"),
          ": ", _("Delete"), 
           "</b></span>\n"," \n",
          "<span color=\"blue\"><b>", 
          _("You have selected multiple files or folders"), 
          "</b></span>", NULL);
      gtk_label_set_markup(label, string);
      gtk_widget_set_halign(GTK_WIDGET(label), GTK_ALIGN_CENTER);
      g_free(string);
    }
    
    static void *asyncYes(void *data){
      auto dialogObject = (DialogTimeout<rmResponse> *)data;
      auto dialog = dialogObject->dialog();
      auto selectionList = (GList *)g_object_get_data(G_OBJECT(dialog), "selectionList");
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(dialog), "menu"));

      for (auto l=selectionList; l && l->data; l=l->next){
        auto info = G_FILE_INFO(l->data);
        auto file = G_FILE(g_file_info_get_attribute_object (info, "standard::file"));
        auto path = Basic::getPath(info);
        int response = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(dialog), "response"));
        GError *error_ = NULL;
        auto output = Child::getOutput(NULL);
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
              ERROR_("*** Error:: %s: %s\n", path, error_->message);
            }
            break;
        }

        g_free(path);
      } // end selection list loop
      Basic::freeSelectionList(selectionList);
      g_object_set_data(G_OBJECT(dialog), "selectionList", NULL);
      return NULL;
    }
    static void *asyncNo(void *data){
      auto dialogObject = (DialogTimeout<rmResponse> *)data;
      auto dialog = dialogObject->dialog();
      auto selectionList = (GList *)g_object_get_data(G_OBJECT(dialog), "selectionList");
      Basic::freeSelectionList(selectionList);
      return NULL;
    }
};

}
#endif
