#ifndef JUMPRESPONSE_HH
#define JUMPRESPONSE_HH
namespace xf {

class jumpResponse {
    
public:
    const char *title(void){ return _("Go to");}
    const char *iconName(void){ return "dialog-question";}
    const char *label(void){ return _("Go to");}
    static void action(const char *path){    

      auto dialogObject = new DialogEntry<jumpResponse>;
      dialogObject->setParent(GTK_WINDOW(MainWidget));
      auto dialog = dialogObject->dialog();
      auto entry = GTK_ENTRY( g_object_get_data(G_OBJECT(dialog),"entry"));
      g_object_set_data(G_OBJECT(entry), "path", g_strdup(path));

 //     dialogObject->subClass()->setDefaults(dialog, dialogObject->label());
      
      dialogObject->run();
    }
    static void *asyncNo(void *data){
      TRACE("asyncNo\n");
      return NULL;
    }
    static void *asyncYes(void *data){
      auto dialogObject = (DialogPrompt<jumpResponse> *)data;
      auto dialog = dialogObject->dialog();
      auto path = dialogObject->getText();
      if (!g_file_test(path, G_FILE_TEST_IS_DIR)){
        if (!strlen(path)) return NULL;
        Print::printError(Child::getOutput(), g_strdup_printf("%s (%s)\n", _("The location does not exist."), path));
        g_free(path);
        return NULL;
      }
      Workdir<bool>::setWorkdir(path, true);
      gtk_window_present(GTK_WINDOW(MainWidget));
      g_free(path);
      TRACE("asyncYes\n");
      return NULL;
    }

 };




}
#endif
