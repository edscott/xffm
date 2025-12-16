#ifndef CLIPBOARD_HH
#define CLIPBOARD_HH
#define CLIPBOARD_TAG "green/black_bg"
namespace xf {
  void *clipBoardObject = NULL;
  template <class Type> class GridView;
  template <class Type> 
  class ClipBoard {
    GdkClipboard *clipBoard_;
    bool validClipBoard_ = FALSE;
    int clipBoardSemaphore_ = 1;
    char *clipBoardCache_ = NULL;
    pthread_mutex_t clipCondMutex_ = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t clipCond_ = PTHREAD_COND_INITIALIZER;
    bool wait_ = false;
public:

    void conditionWait(void){
      wait_ = true;
      pthread_mutex_lock(&clipCondMutex_);
      TRACE("*** clipCond_ wait\n");
      pthread_cond_wait(&clipCond_, &clipCondMutex_);
      pthread_mutex_unlock(&clipCondMutex_);
      TRACE("*** clipCond_ go ahead\n");
      wait_ = false;
    }

    void signalConditionWait(void){
      if (!wait_) return;
      TRACE("*** clipCond signaling...\n");
      pthread_mutex_lock(&clipCondMutex_);
      pthread_cond_signal(&clipCond_);
      pthread_mutex_unlock(&clipCondMutex_);
      TRACE("*** clipCond signaled\n");
    }

    ClipBoard(void){
      g_object_set_data(G_OBJECT(xf::Child::mainWidget()), "ClipBoard", this);
      clipBoard_ = gdk_display_get_clipboard(gdk_display_get_default());
      clipBoardSemaphore_ = TRUE;
      new Thread("ClipBoard::startClipBoard()", clipboardThreadF, this);
      TRACE("*** clipboard thread started.\n")
    }
    ~ClipBoard(void){
      clipBoardSemaphore_ = FALSE;
      usleep(275000); // give it a bunch of time to shut down.
    }


    static void *mop_f(void *data){
      auto list = Child::getGridViewList(); 
      for (auto l=list; l && l->data; l=l->next){
        auto p = (GridView<Type> *)l->data;
        TRACE("*** clearClipBoard(): GridView %p, path=%s\n", p, p->path());
      }
      g_list_free(list);
      return NULL;
    }

    static void *mop(void *data){
        sleep(1);
      TRACE("Basic::context_function for mop_f\n");
        Basic::context_function(mop_f, data);
      return NULL;
    }

    void
    clearClipBoard(void){
        // for each file, send monitor the changed signal
        // this, to update icon
       

        gdk_clipboard_set_text (clipBoard_, "");
        if (pasteButton) gtk_widget_set_sensitive(GTK_WIDGET(pasteButton), false);

        pthread_t thread;
        pthread_create(&thread, NULL, mop, this);
        pthread_detach(thread);
    }

    static void
    clearPaste(void){
        auto c =(ClipBoard<Type> *)g_object_get_data(G_OBJECT(Child::mainWidget()), "ClipBoard");
        c->clearClipBoard();
    }

    static bool isCut(const char *path){
        auto c =(ClipBoard<Type> *)g_object_get_data(G_OBJECT(Child::mainWidget()), "ClipBoard");
        if (!c) return false; // before clipboard is associated to Child::mainWidget().
        if (!c->validClipBoard()) return false;
        auto string = c->clipBoardCache();
        if (strncmp(string, "move", strlen("move")) != 0) return false;
        if (strstr(string, path)) return true;
        return false;
    }

    static bool isCopy(const char *path){
        auto c =(ClipBoard<Type> *)g_object_get_data(G_OBJECT(Child::mainWidget()), "ClipBoard");
        if (!c) return false; // before clipboard is associated to Child::mainWidget().
        if (!c->validClipBoard()) return false;
        auto string = c->clipBoardCache();
        if (strncmp(string, "copy", strlen("copy")) != 0) return false;
        if (strstr(string, path)) return true;
        return false;
    }






    void clipBoardCache(const char *value){
      g_free(clipBoardCache_);
      if (value) clipBoardCache_ = g_strdup(value);
      else clipBoardCache_ = g_strdup("");
    }
    const char *clipBoardCache(void){
      if (!clipBoardCache_) return "";
      return clipBoardCache_;
    }



    void setValidity(bool value){ validClipBoard_ = value;}
    bool validClipBoard(void){ return validClipBoard_;}
    int clipBoardSemaphore(void){ return clipBoardSemaphore_;}
    GdkClipboard *clipBoard(void){ return clipBoard_;}

    static void 
    printClipBoard(void){
      auto c = (ClipBoard<Type> *)g_object_get_data(G_OBJECT(Child::mainWidget()), "ClipBoard");
      auto string = c->clipBoardCache();
      auto output = Child::getOutput(NULL);
      Print::showText(output);
      Print::print(output, CLIPBOARD_TAG, g_strdup("\n "));
      if (!string || strlen(string) == 0){
        Print::printInfo(output, g_strdup("") );
        Print::print(output, CLIPBOARD_TAG, g_strdup(_("Clipboard is empty.")) );
      } else {
        if(!c->validClipBoard()){
          Print::print(output, CLIPBOARD_TAG, g_strconcat(_("Clipboard contents"), ": ", NULL));
          Print::printWarning(output, g_strconcat(_("Invalid clip"),"\n",NULL));
        } else {
          Print::print(output, CLIPBOARD_TAG, g_strconcat(_("Clipboard contents"), ":\n", NULL));
          auto v = g_strsplit(string, "\n", -1);
          for (auto p = v; p && *p; p++){
            Print::printInfo(output, *p);
          }
          g_free(v);
          //Print::print(output, CLIPBOARD_TAG, g_strdup(string));
        }
      }
    }

    
    static GList *
    removeUriFormat(gchar **files) {
        GList *fileList = NULL;
        for (auto f=files; f && *f; f++){
            gchar *file = *f;
            if (!strstr(file, URIFILE)) continue;
            if (strlen(file) > strlen(URIFILE)){
                if (strncmp(file, URIFILE, strlen(URIFILE))==0){
                    file = *f + strlen(URIFILE);
                }
            }
            fileList = g_list_prepend(fileList, g_strdup(file));
        }
        fileList = g_list_reverse(fileList);
        return fileList;
    }
     
    void
    copyClipboardPath(const char *path){ 
      auto clipBoardTxt = gdk_display_get_clipboard(gdk_display_get_default());
      gdk_clipboard_set_text (clipBoardTxt, "");
      gchar *data = g_strdup_printf("copy\n");
      Basic::concat(&data, URIFILE);
      Basic::concat(&data, path);
      Basic::concat(&data, "\n");
      gdk_clipboard_set_text (clipBoardTxt, data);
      g_free(data);
    }
     
    void
    copyClipboardList(GList *list){ 
      auto clipBoardTxt = gdk_display_get_clipboard(gdk_display_get_default());
      gdk_clipboard_set_text (clipBoardTxt, "");
      gchar *data = g_strdup_printf("copy\n");
      for (auto l=list; l && l->data; l=l->next){
        auto info = G_FILE_INFO(l->data);
        auto path = Basic::getPath(info);
        Basic::concat(&data, URIFILE);
        Basic::concat(&data, path);
        Basic::concat(&data, "\n");
        g_free(path);
      }
      gdk_clipboard_set_text (clipBoardTxt, data);
      TRACE("clipboard data: %s\n",data);
      g_free(data);
    }

    bool
    isCutItem(const char *path){
      auto clipTxt = clipBoardCache();
      if (!clipTxt || strncmp(clipTxt, "move\n", strlen("move\n"))){
        return false;
      }
      auto g = g_strconcat(path, "\n", NULL);
      if (!strstr(clipTxt, g)){
        g_free(g);
        return false;
      }
      g_free(g);
      return true;
    }
     
    static void
    cutClipboardPath(const char *path){ 
      auto clipBoardTxt = gdk_display_get_clipboard(gdk_display_get_default());
      gdk_clipboard_set_text (clipBoardTxt, "");
      gchar *data = g_strdup_printf("move\n");
      Basic::concat(&data, URIFILE);
      Basic::concat(&data, path);
      Basic::concat(&data, "\n");
      gdk_clipboard_set_text (clipBoardTxt, data);
      g_free(data);
    }
    

    static void
    cutClipboardList(GList *list){ 
      auto clipBoardTxt = gdk_display_get_clipboard(gdk_display_get_default());
      gdk_clipboard_set_text (clipBoardTxt, "");
      gchar *data = g_strdup_printf("move\n");
      for (auto l=list; l && l->data; l=l->next){
        auto info = G_FILE_INFO(l->data);
        auto path = Basic::getPath(info);
        Basic::concat(&data, URIFILE);
        Basic::concat(&data, path);
        Basic::concat(&data, "\n");
        g_free(path);
      }
      gdk_clipboard_set_text (clipBoardTxt, data);
      g_free(data);
    }
   
    static void
    copyClipboardTxt(GtkTextView *textview){ 
      auto clipBoardTxt = gdk_display_get_clipboard(gdk_display_get_default());
      auto buffer = gtk_text_view_get_buffer(textview);
      gtk_text_buffer_copy_clipboard (buffer,clipBoardTxt);
    }

    static void
    cutClipboardTxt(GtkTextView *textview){ 
      auto clipBoardTxt = gdk_display_get_clipboard(gdk_display_get_default());
      auto buffer = gtk_text_view_get_buffer(textview);
      gtk_text_buffer_cut_clipboard (buffer,clipBoardTxt, TRUE); 
    }

    static void
    pasteClipboardTxt(GtkTextView *textview){
      auto clipBoardTxt = gdk_display_get_clipboard(gdk_display_get_default());
      auto buffer = gtk_text_view_get_buffer(textview);      
      gtk_text_buffer_paste_clipboard (buffer, clipBoardTxt, NULL, TRUE);
    }

    static int
    clipBoardSize(void){
        auto c =(ClipBoard<Type> *)g_object_get_data(G_OBJECT(Child::mainWidget()), "ClipBoard");
        auto text = c->clipBoardCache();
        if (!text) return 0;
        return strlen(text);
    }

private:
    static void *
    clipboardThreadF(void *data){
      auto c = (ClipBoard<Type> *)data;
      while (c->clipBoardSemaphore()){// data is semaphore to thread
          usleep(250000);
          TRACE("Basic::context_function for clipboardContextF\n");
          Basic::context_function(clipboardContextF, c);
      }
      TRACE("*** clipboard thread exited.\n")
      return NULL;
    }

    static void *clipboardContextF(void *data){
      TRACE("clipboardContextF now to read.\n");
        auto c = (ClipBoard<Type> *)data;
        auto clipboard = c->clipBoard();
//        auto clipboard = gdk_display_get_clipboard(gdk_display_get_default());
        gdk_clipboard_read_text_async (clipboard, NULL, setValidity, c);
        return NULL;
    }

    static void
    setValidity(GObject* source_object, GAsyncResult* result,  gpointer data){
        auto c = (ClipBoard<Type> *)data;
        GError *error_ = NULL;
        auto clipBoard = GDK_CLIPBOARD(source_object);
        auto text = gdk_clipboard_read_text_finish(clipBoard, result, &error_);
         if (error_){
          TRACE("Error:: setValidity(): %s. text=\"%s\"\n", error_->message, text);
          g_error_free(error_);
          return;
        }

        TRACE("setValidity for %s\n", text);
     
        if (!text || strlen(text)<5) c->setValidity(false);
        else if (strncmp(text, "copy", strlen("copy")) == 0) c->setValidity(true);
        else if (strncmp(text, "move", strlen("move")) == 0) c->setValidity(true);
        else c->setValidity(false);
        TRACE("Clip board is valid = %d\n", c->validClipBoard());
        // Global paste button:
        if (pasteButton) gtk_widget_set_sensitive(GTK_WIDGET(pasteButton), c->validClipBoard());
        c->clipBoardCache(text);
        // Signal condition
        c->signalConditionWait();
        return;
    }

  };

}

#endif
