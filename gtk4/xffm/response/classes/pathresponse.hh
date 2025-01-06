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
      if (!op_f) {DBG("*** Error: %s not found\n", op); return NULL;}
/*      if (strcmp(op, "mount")==0) {
        if (!g_file_test(target, G_FILE_TEST_EXISTS)){
          if (mkdir(target,0777) < 0){
            auto string = g_strdup_printf(_("Cannot create directory '%s'"), target);
            Print::printError(output, g_strconcat(_("Sorry"), " ", string, " (", strerror(errno), ")\n", NULL));
            g_free(string);
            return NULL;
          }
        }
      } 
      else if (strcmp(op, "mkdir")==0){
        DBG("got mkdir operation:target=\"%s\", newFile=\"%s\".\n", target, newFile);
        if (mkdir(newFile,0777) < 0){
          auto string = g_strdup_printf(_("Cannot create directory '%s' (%s)\n"), newFile, strerror(errno));
          Print::printError(output, g_strconcat(_("Sorry"), " ", string, NULL));
          DBG("***%s\n", string);
          g_free(string);
          return NULL;
        }
      } else {
        if (g_file_test(newFile, G_FILE_TEST_EXISTS)){
          auto string = g_strconcat(_("Sorry"), " \"", newFile, "\" ", _("exists"), "\n", NULL);
          Print::printError(output, string);
          return NULL;
        }
      }*/

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
      } else if (strcmp(op, "mount") == 0) {      
        auto mountSrc = FstabUtil::mountSrc(target);
        if (!mountSrc) mountSrc = g_strdup(path);    
        char *arg[]={(char *)"sudo", (char *)"-A", (char *)op, (char *)"-v", mountSrc, (char *)target, NULL};
        Run<bool>::thread_run(output, (const char **)arg, true);
        g_free(mountSrc);
      }
      */
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
      if (!g_file_test(tgtDir, G_FILE_TEST_IS_DIR)){
        auto text =  g_strdup_printf("target %s is not a directory", tgtDir );
        Print::printWarning(Child::getOutput(), g_strconcat(" ", text, "\n", NULL));
        return;  
      }
      // Backup.
      auto base = g_path_get_basename(src);
      auto tgtFile = g_strconcat(tgtDir, G_DIR_SEPARATOR_S, base, NULL);
      g_free(base);
      if (g_file_test(tgtFile, G_FILE_TEST_EXISTS)){
        auto backup = g_strconcat(tgtDir, G_DIR_SEPARATOR_S, base, "~", NULL);
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
      DBG("*** cpmv_f: \n");
    auto arg =(char **)data;
    pid_t pid = Run<bool>::thread_run(NULL, (const char **)arg, false);
//    pid_t pid = Run<bool>::thread_run(Child::getOutput(), (const char **)arg, false);
    int wstatus;
    waitpid(pid, &wstatus, 0);
    for (auto p=arg; p && *p; p++) g_free(*p);
    g_free(arg);
    return GINT_TO_POINTER(wstatus);
  }
/*
    static void
    backup(const gchar *path, const gchar *target){
        auto base = g_path_get_basename(path);
        auto srcTarget = g_strconcat(target, G_DIR_SEPARATOR_S, base, NULL);
        g_free(base);
        if (g_file_test(srcTarget, G_FILE_TEST_EXISTS)){
          
          auto backup = g_strconcat(srcTarget, "~", NULL);
          auto b1 = g_path_get_basename(srcTarget);
          auto b2 = g_path_get_basename(backup);

          auto text1 = g_strdup_printf(_("Backup file of %s: %s"), b1, b2);
          //auto text = g_strconcat(_("Created: "), backup, "\n", NULL);
          auto text = g_strconcat(" ",text1, "\n", NULL);
          //g_free(text1);
          Print::printWarning(Child::getOutput(), text); // this is run in main context.

            
            const gchar *arg[] = { "mv", "-f", srcTarget, backup, NULL };
            Run<bool>::thread_run(NULL, arg, false);
            //const gchar *arg[] = { "mv", "-v", "-f", srcTarget, backup, NULL };
            TRACE("backup: %s -> %s\n", srcTarget, backup); 
            g_free(backup);
        }
        g_free(srcTarget);
    }
 */   

    
};

}
#endif
