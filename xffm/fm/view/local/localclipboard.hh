#ifndef XF_LOCALCLIPBOARD__HH
# define XF_LOCALCLIPBOARD__HH
static GtkClipboard *clipBoard;
gboolean validClipBoard = FALSE;
gint clipBoardSemaphore = 1;
gchar *clipBoardCache = NULL;

namespace xf
{
template <class Type>
class LocalClipBoard {

public:
    static void
    pasteClip(GtkClipboard *clipBoard, const gchar *text, gpointer data){
        auto target = (const gchar *)data;
        WARN("pasteClip(target=%s):\n%s\n", target, text);
        if (strncmp(text, "copy\n", strlen("copy\n")) == 0){
            auto message = _("Copying files locally");
            auto command = "cp -R -b -f";
            DBG("execute(%s, %s, files, %s)\n", message, command, target);
            //if (!LocalDnd<Type>::execute(message, command, files, target);
            clearClipBoard();
        } else if (strncmp(text, "move\n", strlen("move\n")) == 0){
            auto message = _("Moving files");
            auto command = "mv -b -f";
            DBG("execute(%s, %s, files, %s)\n", message, command, target);
            //if (!LocalDnd<Type>::execute(message, command, files, target);
            clearClipBoard();
        } else {
            DBG("LocalClipBoard::pasteClip: Invalid clipboard contents.\n");
            return;
        }
    }

    static void
    paste(GtkMenuItem *menuItem, gpointer data) { 
        // Two options here, paste in local view or paste in highlight directory
        DBG("paste\n");
        // if the menuitem has the data object "path" set, then the
        // target is a highlighted folder. Otherwise, "path" may be
        // retrieved from the menu data object "path" or baseview
        // method.
        // XXX: Probably should retrieve path the same way from
        //      menu item in both cases... But that would use
        //      more memory and CPU unnecessarily...
	auto baseView = (BaseView<Type> *)g_object_get_data(G_OBJECT(data), "baseView");
        auto path = (const gchar *)g_object_get_data(G_OBJECT(menuItem), "path");
        if (!path) path = baseView->path();
        gtk_clipboard_request_text (clipBoard, pasteClip, (void *)path);
    }

    static void
    clearClipBoard(void){
        // for each file, send monitor the changed signal
        // this, to update icon
        gtk_clipboard_set_text (clipBoard, "", 1);
    }

    static void 
    putInClipBoard(BaseView<Type> *baseView, const gchar *instruction){
         if (!baseView || ! instruction){
            ERROR("baseView||instruction is null\n");
            exit(1);
        }
        DBG("%s\n", instruction); 
        GList *selection_list = gtk_icon_view_get_selected_items (baseView->iconView());
        baseView->setSelectionList(selection_list);
        gchar *clipData = getSelectionData(baseView,instruction );
        if (!g_utf8_validate (clipData, -1, NULL)){
            ERROR("LocalClipBoard::putInClipBoard(): Not a valid utf8 string: %s\n", clipData);
            gtk_clipboard_set_text (clipBoard, "", 1);
        } else gtk_clipboard_set_text (clipBoard, clipData, strlen(clipData)+1);
        
        // icon update business
        // for each file, send monitor the changed signal
        gchar **files = g_strsplit(clipData, "\n", -1);
        for (auto list = localMonitorList; list && list->data; list = list->next){
            auto monitor = (GFileMonitor *)list->data;
                WARN("monitor %p update: ***\n", list->data);
            for (auto f = files+1; f && *f; f++) {
                WARN("monitor %p update: *** %s\n", list->data, *f);
                GFile *child = g_file_new_for_path (*f); 
                g_file_monitor_emit_event (monitor,
                        child, NULL, G_FILE_MONITOR_EVENT_CHANGED);
                g_object_unref(child);
            }
        }
        g_strfreev(files);
	gtk_icon_view_unselect_all (baseView->iconView());
    
       // Seems that the event will manage reference to g_file
       
    }

    static void
    copy(GtkMenuItem *menuItem, gpointer data) { 
	auto baseView = (BaseView<Type> *)g_object_get_data(G_OBJECT(data), "baseView");
        putInClipBoard(baseView, "copy");
    }

    static void
    cut(GtkMenuItem *menuItem, gpointer data) { 
	auto baseView = (BaseView<Type> *)g_object_get_data(G_OBJECT(data), "baseView");
        putInClipBoard(baseView, "move");
    }

    static gchar *
    getSelectionData(BaseView<Type> *baseView, const gchar *instruction){
        GList *selection_list = baseView->selectionList();
        gchar *data = (instruction)?g_strdup_printf("%s\n", instruction): NULL;
        
        for(GList *tmp = selection_list; tmp && tmp->data; tmp = tmp->next) {
            GtkTreePath *tpath = (GtkTreePath *)tmp->data;
            gchar *path;
            GtkTreeIter iter;
            gtk_tree_model_get_iter (baseView->treeModel(), &iter, tpath);
            gtk_tree_model_get (baseView->treeModel(), &iter, PATH, &path, -1);
            if (!data) data = g_strconcat(URIFILE, path, NULL);
            else {
                gchar *e = g_strconcat(data, URIFILE, path, "\n", NULL);
                g_free(data);
                data = e;
            }
            WARN("BaseViewSignals::getSelectionData(): append: %s -> \"%s\"\n", path, data);
            g_free(path);
        }
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
    setValidity(GtkClipboard *clipBoard, const gchar *text, gpointer data){
        if (!text || strlen(text)<5){
            validClipBoard = FALSE;
        } else if (strncmp(text, "copy", strlen("copy")) == 0){
            validClipBoard = TRUE;
        } else if (strncmp(text, "move", strlen("move")) == 0){
            validClipBoard = TRUE;
        } else validClipBoard = FALSE;
        TRACE("Clip board is valid = %d\n", validClipBoard);
        if (!validClipBoard){
            g_free(clipBoardCache);
            clipBoardCache = NULL;
            return;
        }
        if (!clipBoardCache || strcmp(text, clipBoardCache)){
            g_free(clipBoardCache);
            clipBoardCache = g_strdup(text);
            return;
        }
        return;
    }

    static void *clipboardContextF(void *){
        gtk_clipboard_request_text (clipBoard, setValidity, NULL);
        return NULL;
    }
    
    static void *
    clipboardThreadF(void *data){
        while (clipBoardSemaphore){// data is semaphore to thread
            usleep(250000);
            Util<Type>::context_function(clipboardContextF, NULL);
        }
        DBG("*** clipboard thread exited.\n")
        return NULL;
    }

    static gboolean
    clipBoardIsValid(void){ return validClipBoard;}

    static void
    startClipBoard(void){
        // start clipBoard
        clipBoard = gtk_clipboard_get_for_display (gdk_display_get_default(), GDK_SELECTION_CLIPBOARD);
        clipBoardSemaphore = TRUE;
        pthread_t clipBoardThread;
        gint retval = pthread_create(&clipBoardThread, NULL, LocalClipBoard<Type>::clipboardThreadF, NULL);
        if (retval){
            ERROR("thread_create(): clipBoardThread %s\n", strerror(retval));
            //return retval;
        }
        DBG("*** clipboard thread started.\n")
    }

    static void
    stopClipBoard(void){
        clipBoardSemaphore = FALSE;
        sleep(1); // give it a bunch of time to shut down.
    }
        

};


}

#endif
