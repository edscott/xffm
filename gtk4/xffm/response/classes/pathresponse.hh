#ifndef PATHRESPONSE_HH
#define PATHRESPONSE_HH
// template dependencies

namespace xf {
class pathResponse {
public:

    static void *asyncYesArg(void *data, const char *op){
      auto dialogObject = (DialogTimeout<pathResponse> *)data;
      auto dialog = dialogObject->dialog();
      
      auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(dialog), "entry"));
      auto path = (char *)g_object_get_data(G_OBJECT(entry), "path");
      auto buffer = gtk_entry_get_buffer(entry);
      auto text = gtk_entry_buffer_get_text(buffer);

      auto dir = g_path_get_dirname(path);
      auto newFile = g_strconcat(dir, G_DIR_SEPARATOR_S, text, NULL);
      auto output = Child::getOutput();

      auto op_f = g_find_program_in_path(op);
      if (!op_f) DBG("*** Error: %s not found\n", op);
      if (op) {
        if (g_file_test(newFile, G_FILE_TEST_EXISTS)){
          auto string = g_strconcat(_("Sorry"), " \"", text, "\" ", _("exists"), "\n", NULL);
          Print::printError(output, string);
        } else {
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
          }
        }
      }      
     
done:
      g_free(op_f);
      g_free(path);
      g_free(dir);
      g_free(newFile);
      return NULL;
    }

    static void *asyncNo(void *data){
      auto dialogObject = (DialogEntry<pathResponse> *)data;
      auto dialog = dialogObject->dialog();
      auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(dialog), "entry"));
      auto path = g_object_get_data(G_OBJECT(entry), "path");
      g_free(path);

      return NULL;
    }
};

}
#endif
