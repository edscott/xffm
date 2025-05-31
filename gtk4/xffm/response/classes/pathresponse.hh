#ifndef PATHRESPONSE_HH
#define PATHRESPONSE_HH
// template dependencies

namespace xf {
class pathResponse {
public:

    static void *asyncYesArg(void *data, const char *op){
      TRACE("*** asyncYesArg: %s\n", op);
      if (!op) return NULL;
      auto dialogObject = (DialogTimeout<pathResponse> *)data;
      auto dialog = dialogObject->dialog();
      
      auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(dialog), "entry"));
      auto path = (char *)g_object_get_data(G_OBJECT(entry), "path");
      auto buffer = gtk_entry_get_buffer(entry);
      auto target = gtk_entry_buffer_get_text(buffer);

      auto dir = g_path_get_dirname(path);
      auto newFile = g_strconcat(dir, G_DIR_SEPARATOR_S, target, NULL);
      auto output = Child::getOutput();

      auto op_f = g_find_program_in_path(op);
      if (!op_f) {ERROR_("*** Error: %s not found\n", op); return NULL;}
      if (strcmp(op, "cp") == 0){
        cpmv( path, newFile, 1);
      } else if (strcmp(op, "mv") == 0) {    
        cpmv( path, newFile, 0);
      } else if (strcmp(op, "ln") == 0) { 
        cpmv( path, newFile, -1);
      }
     
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
public:
    static void
    cpmv(const gchar *src, const gchar *tgtDir, int modeCopy){
      TRACE("*** cpmv: %s --> %s (%d)\n", src, tgtDir, modeCopy);
      // Code sanity.
      char *tgtFile;
      if (!g_file_test(tgtDir, G_FILE_TEST_IS_DIR)){
        tgtFile = g_strdup(tgtDir);
      } else {
        auto base = g_path_get_basename(src);
        tgtFile = g_strconcat(tgtDir, G_DIR_SEPARATOR_S, base, NULL);
        g_free(base);
      }
      // Backup.
      if (g_file_test(tgtFile, G_FILE_TEST_EXISTS)){
        auto backup = g_strconcat(tgtFile, "~", NULL);
        TRACE("*** cpmv: backup %s --> %s (%d)\n", tgt, backup, modeCopy);
        if (rename(tgtFile, backup) != 0){
          auto text = g_strdup_printf(" rename(%s, %s): %s\n", tgtFile, backup,strerror(errno));
          Print::printWarning(Child::getOutput(), text);
        }
        g_free(backup);
      }
      // Command.
      auto arg = (char **)calloc(10, sizeof(char *));
      int k = 0;
 
      if (modeCopy > 0){
        arg[k++] = g_strdup("cp");
        if (g_file_test(src, G_FILE_TEST_IS_DIR)) arg[k++] = g_strdup("-a");
        arg[k++] = g_strdup("-R");
      } else if (modeCopy < 0) {
        arg[k++] = g_strdup("ln");
        arg[k++] = g_strdup("-s");
      } else {
        arg[k++] = g_strdup("mv");
      }      
      arg[k++] = g_strdup("-v");
      arg[k++] = g_strdup("-f");
      arg[k++] = g_strdup(src);
      arg[k++] = g_strdup(tgtFile);
      g_free(tgtFile);
      //backup(src, tgt);
      THREADPOOL->add(cpmv_f, (void *)arg);
     }  

private:
  static void *cpmv_f(void *data){
      TRACE("*** cpmv_f: \n");
    auto arg =(char **)data;
    pid_t pid = Run<bool>::thread_run(NULL, (const char **)arg, false);
    int wstatus;
    waitpid(pid, &wstatus, 0);
    for (auto p=arg; p && *p; p++) g_free(*p);
    g_free(arg);
    return GINT_TO_POINTER(wstatus);
  }
    
};

}
#endif
