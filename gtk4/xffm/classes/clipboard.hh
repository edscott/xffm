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



#if 0

    static void
    pasteInto(GtkMenuItem *menuItem, gpointer data) { 
        // paste into:
         auto path = (const gchar *)g_object_get_data(G_OBJECT(menuItem), "path");
        if (!path) {
            DBG("path not in menuitem...\n");
            return;
        }
        gtk_clipboard_request_text (clipBoard, pasteClip, (void *)path);
    }

    static void
    paste(GtkMenuItem *menuItem, gpointer data) { 
        // Two options here, paste in local view or paste in highlight directory
        TRACE("paste\n");
        // if the menuitem has the data object "path" set, then the
        // target is a highlighted folder. Otherwise, "path" may be
        // retrieved from the menu data object "path" or baseview
        // method.
        // XXX: Probably should retrieve path the same way from
        //      menu item in both cases... But that would use
        //      more memory and CPU unnecessarily...
        auto view = (View<Type> *)g_object_get_data(G_OBJECT(data), "view");
        // paste into:
        // auto path = (const gchar *)g_object_get_data(G_OBJECT(menuItem), "path");
        // if (!path) path = view->path();
        auto path = view->path();
        gtk_clipboard_request_text (clipBoard, pasteClip, (void *)path);
    }

    static void 
    putInClipBoard(View<Type> *view, const gchar *instruction){
         if (!view || ! instruction){
            ERROR("clipboard.hh::view||instruction is null\n");
            exit(1);
        }
        TRACE("%s\n", instruction); 
        //  single or multiple item selected?
        GList *selectionList;
        if (isTreeView){
            auto treeModel = view->treeModel();
            auto selection = gtk_tree_view_get_selection (view->treeView());
            selectionList = gtk_tree_selection_get_selected_rows (selection, &treeModel);
        } else {
            selectionList = gtk_icon_view_get_selected_items (view->iconView());
        }
        view->setSelectionList(selectionList);
        gchar *clipData = getSelectionData(view,instruction );
        if (!g_utf8_validate (clipData, -1, NULL)){
            ERROR("clipboard.hh::::putInClipBoard(): Not a valid utf8 string: %s\n", clipData);
            gtk_clipboard_set_text (clipBoard, "", 1);
        } else gtk_clipboard_set_text (clipBoard, clipData, strlen(clipData)+1);
        gtk_icon_view_unselect_all (view->iconView());
    }

    static void
    copy(GtkMenuItem *menuItem, gpointer data) { 
        auto view = (View<Type> *)g_object_get_data(G_OBJECT(data), "view");
        putInClipBoard(view, "copy");
    }

    static void
    cut(GtkMenuItem *menuItem, gpointer data) { 
        auto view = (View<Type> *)g_object_get_data(G_OBJECT(data), "view");
        putInClipBoard(view, "move");
    }

    static gchar *
    getSelectionData(View<Type> *view, const gchar *instruction){
        GList *selection_list = view->selectionList();
        gchar *data = (instruction)?g_strdup_printf("%s\n", instruction): NULL;
        
        for(GList *tmp = selection_list; tmp && tmp->data; tmp = tmp->next) {
            GtkTreePath *tpath = (GtkTreePath *)tmp->data;
            gchar *path;
            GtkTreeIter iter;
            gtk_tree_model_get_iter (view->treeModel(), &iter, tpath);
            gtk_tree_model_get (view->treeModel(), &iter, PATH, &path, -1);
            if (g_file_test(path, G_FILE_TEST_EXISTS)){
                if (!data) data = g_strconcat(URIFILE, path, "\n", NULL);
                else {
                    gchar *e = g_strconcat(data, URIFILE, path, "\n", NULL);
                    g_free(data);
                    data = e;
                }
                TRACE("getSelectionData(): append: %s -> \"%s\"\n", path, data);
            }
            g_free(path);
        }
        // remove trailing \n
        if (strchr(data, '\n')) *strrchr(data, '\n')=0;
        return data;
    }

    static gboolean
    isClipBoardCut(void){
        if (!clipBoardCache) return FALSE;
        if (strncmp(clipBoardCache, "move", strlen("move")) == 0) return TRUE;
        return FALSE;
    }

    static gboolean
    isInClipBoard(const gchar *path){
        if (!clipBoardCache) return FALSE;
        gchar *p = g_strconcat(path, "\n", NULL);
        if (strstr(clipBoardCache, p)) return TRUE;
        return FALSE;
    }

    static void
    addClipBoardEmblems(void){
        TRACE("*** addClipBoardEmblems\n");
        gchar **files = NULL;
        if (clipBoardCache) files = g_strsplit(clipBoardCache, "\n", -1);
        sendMonitorSignals(files);
        g_strfreev(files);
    }

    static gchar *
    removeClipBoardEmblems(void){
        TRACE("*** removeClipBoardEmblems\n");
        if (!clipBoardCache) return NULL;
        gchar **files = g_strsplit(clipBoardCache, "\n", -1);

        g_free(clipBoardCache);
        clipBoardCache = NULL;  

        sendMonitorSignals(files);
        g_strfreev(files);
        return NULL;
    }

    static void
    sendMonitorSignals(gchar **files){
        // icon update business
        // for each file, send monitor the changed signal
        for (auto list = localMonitorList; list && list->data; list = list->next){
            auto monitor = (GFileMonitor *)list->data;
            TRACE("Sending signal to monitor %p to update icons, files=%p. ***\n", 
                    list->data, files);
            for (gchar **f = files; f && *f; f++) {
                if (strncmp(*f, URIFILE, strlen(URIFILE))) {
                    TRACE("sendMonitorSignals: %s is not URL.\n", *f);
                    continue;
                }
                else TRACE("sendMonitorSignals: signaling change for %s.\n", *f);
                const gchar *path = *f + strlen(URIFILE);
                TRACE("*** monitor %p update: %s\n", list->data, path);
                GFile *child = g_file_new_for_path (path); 
                g_file_monitor_emit_event (monitor,
                        child, NULL, G_FILE_MONITOR_EVENT_CHANGED);
                g_object_unref(child);
            }
        }
    }

    static gchar *
    clipBoardEmblem(const gchar *path){
        gchar *emblem = NULL;
        if (isInClipBoard(path)){
            if(isClipBoardCut()) {
                emblem = g_strdup("/NE/edit-cut-symbolic/2.0/220");
            } else {
                emblem = g_strdup("/NE/edit-copy-symbolic/2.0/220");
            }
        }
        TRACE("clipBoardEmblem for %s %s\n", path, emblem);
        return emblem;
    }

    public:

    static void 
    printClipBoard(void){
        GdkClipboard *clipBoard = gdk_display_get_clipboard(gdk_display_get_default());
        if (!clipBoard) return;
        //gdk_clipboard_set_text (clipBoardTxt, "foo");
        gdk_clipboard_read_text_async (clipBoard, NULL, outputContent, NULL);
        return;
    }
#endif


#if 0    
    static void
    pasteClipboard(GtkButton *self, void *data){
      auto menu = GTK_WIDGET(g_object_get_data(G_OBJECT(self), "menu")); 
      gtk_popover_popdown(GTK_POPOVER(menu));
      if (!clipBoardTxt) clipBoardTxt = gdk_display_get_clipboard(gdk_display_get_default());

      auto textview = GTK_TEXT_VIEW(gtk_widget_get_parent(menu));
      auto buffer = gtk_text_view_get_buffer(textview);      
      gtk_text_buffer_paste_clipboard (buffer, clipBoardTxt, NULL, TRUE);
    }
    static void
    deleteSelectionTxt(GtkButton *self, void *data){
      auto menu = GTK_WIDGET(g_object_get_data(G_OBJECT(self), "menu")); 
      gtk_popover_popdown(GTK_POPOVER(menu));

      auto textview = GTK_TEXT_VIEW(gtk_widget_get_parent(menu));
      auto buffer = gtk_text_view_get_buffer(textview);      
      gtk_text_buffer_delete_selection (buffer, TRUE, TRUE);
    }
    static void
    selectAllTxt(GtkButton *self, void *data){
      auto menu = GTK_WIDGET(g_object_get_data(G_OBJECT(self), "menu")); 
      gtk_popover_popdown(GTK_POPOVER(menu));

      auto textview = GTK_TEXT_VIEW(gtk_widget_get_parent(menu));
      auto buffer = gtk_text_view_get_buffer(textview);  
      GtkTextIter start, end;
      gtk_text_buffer_get_bounds (buffer, &start, &end);
      gtk_text_buffer_select_range(buffer, &start, &end);
    }
private:
    static void
    outputContent(GObject* source_object, GAsyncResult* result,  gpointer data){
      GError *error_ = NULL;
      auto clipBoard = GDK_CLIPBOARD(source_object);
      auto string = gdk_clipboard_read_text_finish(clipBoard, result, &error_);
      if (error_){
        DBG("Error:: outputContent(): %s\n", error_->message);
        g_error_free(error_);
        return;
      }
      TRACE("readDone: string = %s\n", string);
      auto output = Child::getOutput();
      Print::showText(output);
      Print::print(output, "blue/default_output_bg", g_strdup("\n "));
      auto text = g_strconcat(" ", _("Clipboard contents"), ":\n", NULL);
      Print::print(output, "edit-paste", "blue/default_output_bg", text);
      Print::print(output, "brown/default_output_bg", string);
    }
#endif      
  };

}

#endif
