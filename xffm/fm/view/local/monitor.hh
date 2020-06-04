#ifndef XF_LOCALMONITOR__HH
# define XF_LOCALMONITOR__HH
#ifdef HAVE_MNTENT_H
#endif

namespace xf
{

template <class Type>
class LocalMonitor: public BaseMonitor<Type>
{
    //void *mountArg_[5]; // Needs to exist until destructor is called.
    static gboolean findInModel (GtkTreeModel *treeModel,
                            GtkTreePath *tpath,
                            GtkTreeIter *iter,
                            gpointer data){
        auto arg = (void **)data;
        auto needle = (const gchar *)arg[0];
        gchar *path;
        gtk_tree_model_get (treeModel, iter, PATH, &path, -1);
        if (strcmp(path, needle) == 0){
            arg[1] = GINT_TO_POINTER(1);
            g_free(path);
            return TRUE;
        }
        g_free(path);
        return FALSE;
    }
    static gboolean isInModel(GtkTreeModel *treeModel, const gchar *path){
        void *arg[2] = {
            (void *)path,
            NULL}
        ;
        gtk_tree_model_foreach(treeModel, findInModel, (void *)arg);
        if (arg[1]) return TRUE;
        return FALSE;
    }
public:    
    LocalMonitor(GtkTreeModel *treeModel, View<Type> *view):
        BaseMonitor<Type>(treeModel, view)
    {  
        this->mountArg_[2]=NULL;     
    }
    ~LocalMonitor(void){
#ifndef USE_LOCAL_MONITOR
	return;
#endif
        TRACE("***Destructor:~local_monitor_c(): %p\n", this->monitor());

#ifdef ENABLE_FSTAB_MODULE
        // stop mountThread
        this->mountArg_[1] = NULL;
        while (this->mountArg_[2]){
            TRACE("***Waiting for mountThread to exit\n");
            usleep(250000);
        }
#endif

        TRACE("***Destructor:~local_monitor_c() complete\n");
    }

#ifdef ENABLE_FSTAB_MODULE
#ifdef BSD_FOUND
    void startMountThread(void){
    }
#else
    void startMountThread(void){
        // start mountThread
        pthread_t mountThread;
        //TRACE("LocalMonitor thread itemshash=%p\n", this->itemsHash());
        this->mountArg_[0] = (void *)this;
        this->mountArg_[1] = GINT_TO_POINTER(TRUE);
        this->mountArg_[2] = GINT_TO_POINTER(TRUE);
	gint retval = pthread_create(&mountThread, NULL, FstabMonitor<Type>::mountThreadF, (void *)this->mountArg_);
	if (retval){
	    ERROR("fm/view/local/model.hh::finishLoad():thread_create(): %s\n", strerror(retval));
	    //return retval;
	}
    }
#endif
#endif

    void
    start_monitor(View<Type> *view, const gchar *path){
#ifndef USE_LOCAL_MONITOR
	DBG("*** Local monitor at %s s disabled.\n", path);
#else 
        TRACE("monitor started for %s\n", path);
        this->startMonitor(view->treeModel(), path, (void *)monitor_f);
        view->setMonitorObject(this);
	TRACE("*** Local monitor %p starting  %s.\n", this->monitor(), path);
        localMonitorList = g_list_append(localMonitorList, (void *)this->monitor());
#ifdef ENABLE_FSTAB_MODULE
        startMountThread();
#endif
        // XXX:   Start mountThread...
        //        this here now crashes ...
        //startMountThread();
        //        reason, swiching treemodels...
        //        Now moved to when load thread has completed treemodel switch.
        //        At local/model.hh
# if 0
        // start mountThread
        pthread_t mountThread;
        //TRACE("LocalMonitor thread itemshash=%p\n", this->itemsHash());
        mountArg_[0] = (void *)this;
        mountArg_[1] = GINT_TO_POINTER(TRUE);
        mountArg_[2] = GINT_TO_POINTER(TRUE);
	gint retval = pthread_create(&mountThread, NULL, FstabMonitor<Type>::mountThreadF, (void *)mountArg_);
	if (retval){
	    ERROR("fm/view/local/monitor::thread_create(): %s\n", strerror(retval));
	    //return retval;
	}
# endif
#endif
    }

    xd_t *
    get_xd_p(GFile *first){
	gchar *path = g_file_get_path(this->gfile_);
	gchar *basename = g_file_get_basename(first);
	struct dirent *d; // static pointer
	TRACE("looking for %s info\n", basename);
	DIR *directory = opendir(path);
	xd_t *xd_p = NULL;
	if (directory) {
	  while ((d = readdir(directory))  != NULL) {
	    if(strcmp (d->d_name, basename)) continue;
	    xd_p = LocalView<Type>::get_xd_p(path, d, TRUE);
	    break;
	  }
	  closedir (directory);
	} else {
	  ERROR("fm/view/local/monitor::monitor_f(): opendir %s: %s\n", path, strerror(errno));
	}
	g_free(basename); 
	g_free(path); 
	return xd_p;
    }


    gboolean
    add_new_item(GFile *file){
       xd_t *xd_p = get_xd_p(file);
        gboolean showHidden = (Settings<Type>::getSettingInteger("LocalView", "ShowHidden") > 0);
        if (xd_p) {
	    if (xd_p->d_name[0] == '.' && !showHidden) return FALSE;
            // here we should insert according to sort order...
            LocalView<Type>::insertLocalItem(this->store_, xd_p);
            // this just appends:
            //LocalView<Type>::add_local_item(store_, xd_p);
            // use hashkey
            gchar *key = PixbufHash<Type>::get_hash_key(xd_p->path, 10);
	    TRACE("add_new_item ...(%s --> %s) shows:hidden=%d\n", key, xd_p->path, showHidden);
            g_hash_table_replace(this->itemsHash(), key, g_strdup(xd_p->path));
            LocalView<Type>::free_xd_p(xd_p);
            return TRUE;
        } 
        return FALSE;
    }

    static gboolean stat_func (GtkTreeModel *model,
				GtkTreePath *tpath,
				GtkTreeIter *iter,
				gpointer data){
        gchar *path;
	auto arg = (void **)data;
	auto inPath = (const gchar *)arg[0];
        auto view = (View<Type> *)arg[1];

	gtk_tree_model_get (model, iter, PATH, &path, -1);  

	TRACE("stat_func: %s <--> %s\n", path, inPath);
	if (strcmp(path, inPath)){
            g_free(path);
	    return FALSE;
	}
        g_free(path);
	TRACE("stat_func: gotcha %s\n", inPath);
	GtkListStore *store = GTK_LIST_STORE(model);

	auto directory = g_path_get_dirname(inPath);
	struct dirent d;
	auto basename = g_path_get_basename(inPath);
	strncpy(d.d_name, basename, 256);
	g_free(basename);
	auto xd_p = LocalModel<Type>::get_xd_p(directory, &d, TRUE);
	g_free(directory);
	TRACE("%s --> %s --> %s\n", xd_p->path, xd_p->mimetype, xd_p->icon);
	if (strcmp(xd_p->mimetype, "inode/unknown")==0){
	    g_free(xd_p->mimetype);
	    if (g_file_test(xd_p->path, G_FILE_TEST_IS_DIR)){
		xd_p->mimetype = g_strdup("inode/directory");
		xd_p->d_type = DT_DIR;
	    } else {
		xd_p->mimetype = Mime<Type>::mimeType(xd_p->path);
	    }
	    if (!xd_p->mimetype) xd_p->mimetype = g_strdup("inode/unknown");
	    g_free(xd_p->icon);
	    xd_p->icon = LocalIcons<Type>::getIconname(xd_p);   
	}
	TRACE("2. %s --> %s --> %s\n", xd_p->path, xd_p->mimetype, xd_p->icon);
        gchar *iconName = xd_p->icon;

	
        TRACE("***localmonitor stat_func(): iconname=%s\n", iconName);
	GdkPixbuf *pixbuf;
	GdkPixbuf *treepixbuf;
	if (g_path_is_absolute(iconName)){
	    auto page_p = Fm<Type>::getCurrentPage();
	    auto pixels = page_p->getImageSize();
	    pixbuf = Pixbuf<Type>::getImageAtSize(iconName, pixels, xd_p->mimetype, xd_p->st);
	    treepixbuf = Pixbuf<Type>::getImageAtSize(iconName, 24, xd_p->mimetype);
	} else {
	    pixbuf = Pixbuf<Type>::getPixbuf(iconName,  -48);
	    treepixbuf = Pixbuf<Type>::getPixbuf(iconName,  -24);
	}



	// decorate image preview with cut emblem
	if (Gtk<Type>::isImage(xd_p->mimetype)){
	    const gchar *clipEmblem=NULL;
	    clipEmblem= ClipBoard<Type>::clipBoardEmblem(xd_p->path);

	    if (clipEmblem){
		void *arg2[] = {NULL, (void *)pixbuf, NULL, NULL, (void *)(clipEmblem+1) };
		Util<Type>::context_function(Pixbuf<Type>::insert_decoration_f, arg2);

		void *arg3[] = {NULL, (void *)treepixbuf, NULL, NULL, (void *)(clipEmblem+1) };
		Util<Type>::context_function(Pixbuf<Type>::insert_decoration_f, arg3);
	    } else {
	    }
	}

	GdkPixbuf *highlight_pixbuf;
	if (strcmp(xd_p->mimetype, "inode/directory")==0){
	    //FIXME: this is not working as intended...
	    //       highlight_pixbuf is also incorrect for images...
	    //       but works ok for other file types...
	    highlight_pixbuf = Pixbuf<Type>::getPixbuf("document-open",  -48);
	    TRACE("highlight pixbuf = %s -> document-open\n", xd_p->path);
	}
	else {
	    TRACE("local/monitor: mimetype=%s\n", xd_p->mimetype);
	    TRACE("highlight pixbuf = %s->%s\n", xd_p->path, iconName);
	    highlight_pixbuf = gdk_pixbuf_copy(pixbuf);
	}
        //Highlight emblem macros are defined in types.h
	//
	// Decorate highlight pixbuf
	// (duplicate code in monitor.hh/model.hh)
	const gchar *emblem="";
	if (xd_p->mimetype){
	    if (strncmp(xd_p->mimetype, "inode/regular", strlen("inode/regular"))==0){
		emblem = HIGHLIGHT_TEXT;
	    }
	    if (strncmp(xd_p->mimetype, "text", strlen("text"))==0){
		emblem = HIGHLIGHT_TEXT;
	    }
	    if (strncmp(xd_p->mimetype, "application", strlen("application"))==0){
		emblem = HIGHLIGHT_APP;
	    }
	    else emblem = HIGHLIGHT_EMBLEM;
	}
	// Images as well...
	if (strlen(emblem)){
	    // Now decorate the pixbuf with emblem (types.h).
	    void *arg[] = {NULL, (void *)highlight_pixbuf, NULL, NULL, (void *)emblem };
	    TRACE("pixbuf emblem = %s->%s\n", xd_p->path, emblem);
	    // Done by main gtk thread:
	    Util<Type>::context_function(Pixbuf<Type>::insert_decoration_f, arg);
	} else {
	    TRACE("no emblem for %s\n", xd_p->path);
	}

        auto date = LocalModel<Type>::dateString((xd_p->st)?xd_p->st->st_mtime:0);
        auto size = LocalModel<Type>::sizeString((xd_p->st)?xd_p->st->st_size:0);
        TRACE("local/monitor gtk_list_store_set(%s)\n", iconName);
	gtk_list_store_set (store, iter, 
                SIZE, size, 
                DATE, date,
                ICON_NAME, iconName,
                DISPLAY_PIXBUF, pixbuf,
                TREEVIEW_PIXBUF, treepixbuf,
                NORMAL_PIXBUF, pixbuf,
                HIGHLIGHT_PIXBUF, highlight_pixbuf, 
                FLAGS, xd_p->d_type,
		-1);
        LocalModel<Type>::free_xd_p(xd_p);
        g_free(date);
        g_free(size);
	return TRUE;
    }
    gboolean 
    restat_item(GFile *src){
        gchar *path = g_file_get_path(src);
	auto retval = restat_item(path);
        g_free(path);
	return retval;
    }

    gboolean 
    restat_item(const gchar *path){
        // First we use a hash to check if item is in treemodel.
        // Then, if found, we go on to find the item in the treemodel and update.
        // If not found, we should add item.
	TRACE("restat_item %s \n", path);
        gboolean showHidden = (Settings<Type>::getSettingInteger("LocalView", "ShowHidden") > 0);
	if (path[0] == '.' && !showHidden) {
	    return FALSE;
	}
	// XXX Look for specific item within the treemodel.
	//     When found, do the restat business.
        void *arg[] = {(void *)(path), (void *)this->baseView_};
        gtk_tree_model_foreach (GTK_TREE_MODEL(this->store_), stat_func, (gpointer) arg); 
        return TRUE;
    }

private:
    static gboolean changeItem(void *data){
	auto arg = (void **)data;
        auto p = (LocalMonitor<Type> *)arg[0];
	auto f = (gchar *)arg[1];
	// If an direct path icon (image for example) clear hash first
	// This will clear the thumbnail since item is no longer hashed.
	if (g_path_is_absolute(f)){
	    PixbufHash<Type>::rm_from_pixbuf_hash(f, 24);
	    PixbufHash<Type>::rm_from_pixbuf_hash(f, 48);
	}
	// XXX: doesn't restat item do the above?
	p->restat_item(f);
	g_free(f);
	g_free(arg);
	return G_SOURCE_REMOVE;
    }
	
	
    
    static void
    monitor_f (GFileMonitor      *mon,
              GFile             *first,
              GFile             *second,
              GFileMonitorEvent  event,
              gpointer           data)
    {
        gchar *f= first? g_file_get_path (first):g_strdup("--");
        gchar *s= second? g_file_get_path (second):g_strdup("--");
       

        TRACE("*** monitor_f call...\n");
        auto p = (LocalMonitor<Type> *)data;
	if (!p->active()){
	    TRACE("monitor_f(): monitor not currently active.\n");
	    return;
	}
        if (!BaseSignals<Type>::validBaseView(p->view())) return;

        
        gboolean verbose = FALSE;
        switch (event){
            case G_FILE_MONITOR_EVENT_DELETED:
            case G_FILE_MONITOR_EVENT_MOVED_OUT:
                if (verbose) DBG("Received DELETED  (%d): \"%s\", \"%s\"\n", event, f, s);
                
                p->remove_item(first);
                p->updateFileCountLabel();
                break;
            case G_FILE_MONITOR_EVENT_CREATED:
            case G_FILE_MONITOR_EVENT_MOVED_IN:
                if (verbose) DBG("Received  CREATED (%d): \"%s\", \"%s\"\n", event, f, s);
                //p->restat_item(first);
#if 10
                /*if (isInModel(p->treeModel(), f)){
                    p->restat_item(first);
                } else*/ 
                {
                    p->add_new_item(first);
                    p->updateFileCountLabel();
                }
#endif
                break;
            case G_FILE_MONITOR_EVENT_CHANGED:
	    {
                if (verbose) DBG("monitor_f(): Received  CHANGED (%d): \"%s\", \"%s\"\n", event, f, s);
         /*       // reload icon
		PixbufHash<Type>::rm_from_pixbuf_hash(f, 24);
		PixbufHash<Type>::rm_from_pixbuf_hash(f, 48);
		// Thumbnails are not thumbnailed.
		//PixbufHash<Type>::zap_thumbnail_file(f, 24);
		//PixbufHash<Type>::zap_thumbnail_file(f, 48);
                p->restat_item(first);*/
		auto arg = (void **)calloc(2, sizeof(void *));
		if (!arg){
		    ERROR("local/monitor.hh::monitor_f(): %s\n",strerror(errno));
		} else {
		    arg[0]=(void *)p;
		    arg[1]=g_strdup(f);
		    g_timeout_add(500, changeItem, arg);
		}
	    } break;
            case G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED:
                if (verbose) DBG("Received  ATTRIBUTE_CHANGED (%d): \"%s\", \"%s\"\n", event, f, s);
                p->restat_item(first);
                break;
            case G_FILE_MONITOR_EVENT_PRE_UNMOUNT:
                if (verbose) DBG("Received  PRE_UNMOUNT (%d): \"%s\", \"%s\"\n", event, f, s);
                break;
            case G_FILE_MONITOR_EVENT_UNMOUNTED:
                if (verbose) DBG("Received  UNMOUNTED (%d): \"%s\", \"%s\"\n", event, f, s);
                break;
            case G_FILE_MONITOR_EVENT_MOVED:
            case G_FILE_MONITOR_EVENT_RENAMED:
                if (verbose) DBG("Received  MOVED (%d): \"%s\", \"%s\"\n", event, f, s);
                p->add2reSelect(f); // Only adds to selection list if item is selected.
                p->remove_item(first); 
                if (isInModel(p->treeModel(), s))
                {
                    p->restat_item(second);
                } else p->add_new_item(second);
                break;
            case G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT:
                if (verbose) DBG("Received  CHANGES_DONE_HINT (%d): \"%s\", \"%s\"\n", event, f, s);
                p->reSelect(f); // Will only select if in selection list (from move).
                break;       
        }
        g_free(f);
        g_free(s);
    }



};
}
#endif

