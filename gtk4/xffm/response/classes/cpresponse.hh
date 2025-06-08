#ifndef CPRESPONSE_HH
#define CPRESPONSE_HH

namespace xf {

class cpResponse: public pathResponse {
   using clipboard_t = ClipBoard<LocalDir>;
   const char *title_;
   const char *iconName_;
public:
    const char *title(void){ return _("Path");}
    const char *iconName(void){ return "dialog-question";}
    const char *label(void){ return _("Duplicate");}

    static void setDefaults(GtkWindow *dialog, GtkLabel *label){
      auto entry = GTK_ENTRY( g_object_get_data(G_OBJECT(dialog),"entry"));
      auto path = (const char *)g_object_get_data(G_OBJECT(entry), "path");
      auto base = g_path_get_basename(path);
      auto buffer = gtk_entry_get_buffer(entry);
      auto string = g_strconcat("<span color=\"green\"><b>",_("Duplicate"), "</b></span>", NULL);
      gtk_entry_buffer_set_text(buffer, base, -1);
     // auto string = g_strconcat("<span color=\"green\"><b>",_("Duplicate"), ":\n</b></span><span color=\"blue\"><b>", base, "</b></span>", NULL);
      gtk_label_set_markup(label, string);
      g_free(base);
      g_free(string);
    }
    
    
    static void *asyncYes(void *data){
       asyncYesArg(data, "cp");      
       return NULL;
    }
};

class cpDropResponse {
   using clipboard_t = ClipBoard<LocalDir>;
   const char *title_;
   const char *iconName_;
   int copy_ = 1;

  public:
    const char *title(void){ return _("Copy files");}
    const char *iconName(void){ return "copy";}
    const char *label(void){ return _("");}    
    int copy(void){ return copy_;}
    void copy(int value){ copy_ = value;}

    static void *asyncYes(void *data){
       TRACE("asyncYes::  complete\n");
       return NULL;
    }
    static void *asyncNo(void *data){
      TRACE("asyncNo::  cancelled\n");
      auto dialogObject = (DialogBasic<cpDropResponse> *)data;
      dialogObject->lockCondition();
      pthread_cond_wait(dialogObject->cond_p(), dialogObject->condMutex_p());
      dialogObject->unlockCondition();
      return NULL;
    }

    static void performPasteAsync(const char *target){
      auto c =(clipboard_t *)g_object_get_data(G_OBJECT(Child::mainWidget()), "ClipBoard");
      auto text = c->clipBoardCache();
      gchar **files = g_strsplit(text, "\n", -1);
      if (!files) {
        ERROR_("*** Error: no files in clipboard\n");
        return;
      }
      
      int mode = 1; // copy
      if (strcmp(files[0], "copy")==0) mode = 1; 
      else if (strcmp(files[0], "move")==0) mode = 0;
      else if (strcmp(files[0], "link")==0) mode = -1;

      auto list = clipboard_t::removeUriFormat(files);
      g_strfreev(files);

      auto dialogObject = new DialogDrop<cpDropResponse>;
      auto dialog = dialogObject->dialog();
      dialogObject->setParent(GTK_WINDOW(Child::mainWidget()));
      dialogObject->setCloseBox("delete", _("Cancel"));
      dialogObject->subClass()->copy(mode);
     
      dialogObject->run(); // running in a thread...
                           // This is a progress dialog 
                           // which allows cancelation.
                           // Shows only on long operations.
      

      void **arg = (void **)calloc(4, sizeof (void *));
      arg[0] = (void *)dialogObject;
      arg[1] = (void *)list;
      arg[2] = (void *)g_strdup(target);
      pthread_t thread;
      Thread::threadCount(true,  &thread, "performPasteAsync");
      pthread_create(&thread, NULL, thread1, arg);
      pthread_detach(thread);
      Thread::threadCount(false,  &thread, "performPasteAsync");

      // clipboard contents no longer needed
      c->clearClipBoard();

      TRACE("thread 1 detached\n");
     }
private:
  static void *thread1(void *data){
    void **arg = (void **)data;
    auto dialogObject = (DialogDrop<cpDropResponse> *)arg[0];
    pthread_t thread;
      Thread::threadCount(true,  &thread, "thread1");
    pthread_create(&thread, NULL, thread2, data);
    void *retval;
    pthread_join(thread, &retval);
      Thread::threadCount(false,  &thread, "thread1");
    TRACE("thread2 joined, copy complete.\n");
    dialogObject->lockCondition();
    TRACE("thread2 pthread_cond_signal.\n");
    pthread_cond_signal(dialogObject->cond_p());
    dialogObject->unlockCondition();
   
    
    TRACE("thread2 unlockCondition.\n");
    // copy move async is done.
    const char *mode ="";
    switch (dialogObject->subClass()->copy()){
      case 1:
        mode = _("copy"); break;
      case 0:
        mode = _("move"); break;
      case -1:
        mode = _("link"); break;
    }
    Print::showText(Child::getOutput());
    Print::printInfo(Child::getOutput(), g_strdup_printf(" %s (%s)\n",  _("Operation completed"), mode)); 
    return NULL;
  }
  
  static void *thread2(void *data){
    void **arg = (void **)data;
    auto dialogObject = (DialogDrop<cpDropResponse> *)arg[0];
    auto dialog = dialogObject->dialog();
    auto list = (GList *)arg[1];
    auto path = (char *)arg[2];
    int response;
    auto total = g_list_length(list);
    int bytes = 0;
    int totalBytes = 0;

    for (auto l=list; l && l->data; l=l->next){
      if (skip((char *)l->data, path)) continue;
      struct stat st;
      stat((char *)l->data, &st);
      totalBytes += st.st_size;
    }

    int k = 1;
    for (auto l=list; l && l->data; l=l->next, k++){
      if (skip((char *)l->data, path)) continue;

      struct stat st;
      stat((char *)l->data, &st);
      bytes += st.st_size;
      
      dialogObject->lockResponse();
      response = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(dialog), "response"));
      dialogObject->unlockResponse();
      if (response == 0) {
        dialogObject->setProgress(k, total, path, (char *)l->data, bytes, totalBytes);
        TRACE("thread2 %s --> %s\n", (char *)l->data, path);
        // thread copy
        pathResponse::cpmv((char *)l->data, path, dialogObject->subClass()->copy());
      }
      if (response < 0) {
        dialogObject->cancel();
        THREADPOOL->clear();

        break;
      }
      //sleep(1); // slow motion
      usleep(150);
    }

    //void *arg2[] = {(void *)path, (void *)list};
    //Basic::context_function(overwriteMessage, arg2);

    for (auto l=list; l && l->data; l=l->next){ g_free(l->data);}
    g_list_free(list);
    g_free(path);
        
    TRACE("thread2 loop exited\n");
    dialogObject->lockResponse();
    if (!dialogObject->cancelled()) {
      TRACE("thread2 setting response to OK\n");
      response = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(dialog), "response"));
      if (response == 0) g_object_set_data(G_OBJECT(dialog), "response", GINT_TO_POINTER(1)); //yes
    }
    dialogObject->unlockResponse();
    TRACE("thread2 exit\n");

    return NULL;
  }
/*
    static void *overwriteMessage(void *data){
      void **arg = (void **)data;
      auto target= (char *)arg[0];
      auto list = (GList *)(arg[1]);

    TRACE("overwriteMessage target=%s, list=%p\n", target, list);
      for (auto l=list; l && l->data; l=l->next){
        auto base = g_path_get_basename((const char *)l->data);
        auto f = g_strconcat(target, G_DIR_SEPARATOR_S, (const char *)base, NULL);
        TRACE("final target=%s\n", f);
        if (g_file_test(f, G_FILE_TEST_EXISTS)){
          auto b = g_strconcat(f, "~", NULL);
          auto text1 = g_strdup_printf(_("Backup file of %s: %s"), f, b);
          auto text = g_strconcat(_("Overwrite Destination"), ": ", text1, "\n", NULL);
          g_free(text1);
          auto output = Child::getOutput();
    TRACE(text);
          Print::printWarning(output, text); // this is run in main context.
        } TRACE("final target=\"%s\" does not exist\n", f);
        g_free(base);
        g_free(f);
      }
      for (auto l=list; l && l->data; l=l->next){ g_free(l->data);}
      g_list_free(list);
      g_free(target);

      return NULL;
    }
*/
  static bool skip(const char *src, const char *tgt){
    if (strcmp(src, tgt)==0){
      TRACE("skipping %s\n", src);
      return true;
    }
    auto dir = g_path_get_dirname(src);
    if (strcmp(dir, tgt)==0){
      TRACE("skipping %s\n", src);
      g_free(dir);
      return true;
    }
    g_free(dir);
    return false;
  }
  
};

}
#endif
