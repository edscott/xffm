#ifndef XF_LOCALCLIPBOARD__HH
# define XF_LOCALCLIPBOARD__HH


namespace xf
{
static GtkClipboard *clipBoard;
gboolean validClipBoard = FALSE;
gint clipBoardSemaphore = 1;
gchar *clipBoardCache = NULL;

template <class Type>
class ClipBoard {

public:
    static void
    pasteClip(GtkClipboard *clipBoard, const gchar *text, gpointer data){
        auto target = (const gchar *)data;
        gchar **files = g_strsplit(text, "\n", -1);

        TRACE("pasteClip(target=%s):\n%s\n", target, text);
        if (strncmp(text, "copy\n", strlen("copy\n")) == 0){
            auto message = _("Copying files locally");
            auto command = "cp -R -b -f";
            TRACE("execute(%s, %s, files, %s)\n", message, command, target);

            Gio<Type>::executeURL(files, target, MODE_COPY);
            clearClipBoard();
        } else if (strncmp(text, "move\n", strlen("move\n")) == 0){
            auto message = _("Moving files");
            auto command = "mv -b -f";
            TRACE("execute(%s, %s, files, %s)\n", message, command, target);
            Gio<Type>::executeURL(files, target, MODE_MOVE);
            clearClipBoard();
        } else {
            TRACE("ClipBoard::pasteClip: Invalid clipboard contents.\n");
        }
        if (files) g_strfreev(files);
    }

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
    clearClipBoard(void){
        // for each file, send monitor the changed signal
        // this, to update icon
        gtk_clipboard_set_text (clipBoard, "", 1);
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

    static void 
    updateClipBoardCache(const gchar *text){
	gboolean updateIconBusiness = FALSE;
        if (!validClipBoard){
	    if (clipBoardCache){
		// Update any previously set icons.
		clipBoardCache = removeClipBoardEmblems();
		updateIconBusiness = TRUE;
	    }
	}
	else if (!clipBoardCache || strcmp(text, clipBoardCache)){
	    // Update any previously set icons.
	    clipBoardCache = removeClipBoardEmblems();
            g_free(clipBoardCache);
            clipBoardCache = g_strdup(text);
	    updateIconBusiness = TRUE;
        }
	if (!updateIconBusiness) return;
	addClipBoardEmblems();
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
	updateClipBoardCache(text);
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
        TRACE("*** clipboard thread exited.\n")
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
        gint retval = pthread_create(&clipBoardThread, NULL, ClipBoard<Type>::clipboardThreadF, NULL);
        if (retval){
            ERROR("clipboard.hh::thread_create(): clipBoardThread %s\n", strerror(retval));
            //return retval;
        }
        TRACE("*** clipboard thread started.\n")
    }

    static void
    stopClipBoard(void){
        clipBoardSemaphore = FALSE;
        sleep(1); // give it a bunch of time to shut down.
    }
        

};


}

#endif
