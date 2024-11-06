#ifndef CLIPBOARD_HH
#define CLIPBOARD_HH
namespace xf {
//  static GdkClipboard *clipBoardTxt=NULL;
  class ClipBoard {
    GdkClipboard *clipBoard_;
    bool validClipBoard_ = FALSE;
    int clipBoardSemaphore_ = 1;
    char *clipBoardCache_ = NULL;
public:

    ClipBoard(void){
      clipBoard_ = gdk_display_get_clipboard(gdk_display_get_default());
      clipBoardSemaphore_ = TRUE;
      new Thread("ClipBoard::startClipBoard()", clipboardThreadF, this);
      TRACE("*** clipboard thread started.\n")
    }
    ~ClipBoard(void){
      clipBoardSemaphore_ = FALSE;
      usleep(275000); // give it a bunch of time to shut down.
    }

    void
    clearClipBoard(void){
        // for each file, send monitor the changed signal
        // this, to update icon
        gdk_clipboard_set_text (clipBoard_, "");
    }

    static void
    clearPaste(void){
        auto c =(ClipBoard *)g_object_get_data(G_OBJECT(MainWidget), "ClipBoard");
        // for each file, send monitor the changed signal
        // this, to update icon
        c->clearClipBoard();
    }

    void resetClipBoardCache(const char *text){
        g_free(clipBoardCache_);
        clipBoardCache_ = g_strdup(text);
        TRACE("resetClipBoardCache(): %s\n", clipBoardCache_);
    }

    void setValidity(bool value){ validClipBoard_ = value;}
    bool validClipBoard(void){ return validClipBoard_;}
    int clipBoardSemaphore(void){ return clipBoardSemaphore_;}
    GdkClipboard *clipBoard(void){ return clipBoard_;}
    char *clipBoardCache(void){return clipBoardCache_;}

    static void 
    printClipBoard(void){
      auto c = (ClipBoard *)g_object_get_data(G_OBJECT(MainWidget), "ClipBoard");
      auto string = c->clipBoardCache();
      auto output = Child::getOutput();
      Print::showText(output);
      Print::print(output, "blue/default_output_bg", g_strdup("\n "));
      if (!string || strlen(string) == 0){
        Print::print(output, "edit-paste", "blue/default_output_bg", g_strdup(_("Clipboard is empty.")) );
      } else {
        auto text = g_strconcat(" ", _("Clipboard contents"), ":\n", NULL);
        Print::print(output, "edit-paste", "blue/default_output_bg", text);
        Print::print(output, "brown/default_output_bg", g_strdup(string));
      }
    }

#if 0
    static void
    pasteClip(const gchar *target){
        auto c =(ClipBoard *)g_object_get_data(G_OBJECT(MainWidget), "ClipBoard");
        auto text = c->clipBoardCache();
        gchar **files = g_strsplit(text, "\n", -1);
      
        auto dialogObject = new DialogBasic<cpDropResponse>;

        DBG("pasteClip(target=%s):\n%s\n", target, text);
        if (strncmp(text, "copy\n", strlen("copy\n")) == 0){
          
        } else if (strncmp(text, "move\n", strlen("move\n")) == 0){
        } else {
            DBG("ClipBoard::pasteClip: Invalid clipboard contents.\n");
        }

        /*
        if (strncmp(text, "copy\n", strlen("copy\n")) == 0){
            auto message = _("Copying files locally");
            auto command = "cp -R -b -f";
            DBG("execute(%s, %s, files, %s)\n", message, command, target);

            Gio::executeURL(files, target, MODE_COPY);
            c->clearClipBoard();
        } else if (strncmp(text, "move\n", strlen("move\n")) == 0){
            auto message = _("Moving files");
            auto command = "mv -b -f";
            DBG("execute(%s, %s, files, %s)\n", message, command, target);
            Gio::executeURL(files, target, MODE_MOVE);
            c->clearClipBoard();
        } else {
            DBG("ClipBoard::pasteClip: Invalid clipboard contents.\n");
        }
        */
        if (files) g_strfreev(files);
    }
#endif
    
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
     
    static void
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
     
    static void
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
      g_free(data);
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
        auto c =(ClipBoard *)g_object_get_data(G_OBJECT(MainWidget), "ClipBoard");
        auto text = c->clipBoardCache();
        if (!text) return 0;
        return strlen(text);
    }

private:
    static void *
    clipboardThreadF(void *data){
      auto c = (ClipBoard *)data;
      while (c->clipBoardSemaphore()){// data is semaphore to thread
          usleep(250000);
          Basic::context_function(clipboardContextF, c);
      }
      TRACE("*** clipboard thread exited.\n")
      return NULL;
    }

    static void *clipboardContextF(void *data){
        auto c = (ClipBoard *)data;
        auto clipboard = c->clipBoard();
//        auto clipboard = gdk_display_get_clipboard(gdk_display_get_default());
        gdk_clipboard_read_text_async (clipboard, NULL, setValidity, c);
        return NULL;
    }

    static void
    setValidity(GObject* source_object, GAsyncResult* result,  gpointer data){
        auto c = (ClipBoard *)data;
        GError *error_ = NULL;
        auto clipBoard = GDK_CLIPBOARD(source_object);
        auto text = gdk_clipboard_read_text_finish(clipBoard, result, &error_);
         if (error_){
          TRACE("Error:: setValidity(): %s. text=\"%s\"\n", error_->message, text);
          g_error_free(error_);
          return;
        }
     
        if (!text || strlen(text)<5) c->setValidity(false);
        else if (strncmp(text, "copy", strlen("copy")) == 0) c->setValidity(true);
        else if (strncmp(text, "move", strlen("move")) == 0) c->setValidity(true);
        else c->setValidity(false);
        TRACE("Clip board is valid = %d\n", c->validClipBoard());
        updateClipBoardCache(c, text);
        return;
    }

    static void 
    updateClipBoardCache(ClipBoard *c, const gchar *text){
        gboolean updateIconBusiness = FALSE;
      /*  if (!c->validClipBoard()){
            if (c->clipBoardCache()){
                // Update any previously set icons.
                // FIXME clipBoardCache = removeClipBoardEmblems();
                updateIconBusiness = TRUE;
            }
        }
        else */
        if (!c->clipBoardCache() || strcmp(text, c->clipBoardCache())){
            // Update any previously set icons.
            // FIXME clipBoardCache = removeClipBoardEmblems();
            c->resetClipBoardCache(text);
            updateIconBusiness = TRUE;
        }
        //c->resetClipBoardCache(text);
        if (!updateIconBusiness) return;
        // FIXME addClipBoardEmblems();
    }




  };

}

#endif
