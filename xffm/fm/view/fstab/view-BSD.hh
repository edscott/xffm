#ifndef VIEW_BSD_HH 
#define VIEW_BSD_HH
public:
    static void
    loadModel (View<Type> *view)
    {
		TRACE("fstab loadModel()\n");
		view->disableDnD();	
        auto iconView = view->iconView();
        auto treeModel = gtk_icon_view_get_model (iconView);
		TRACE("mk_tree_model:: model = %p\n", treeModel);
        while (gtk_events_pending()) gtk_main_iteration();
		removeAllItems(treeModel);
        // Disable DnD
        //gtk_icon_view_unset_model_drag_source (iconView);
        //gtk_icon_view_unset_model_drag_dest (iconView);
        gtk_icon_view_set_selection_mode (iconView,GTK_SELECTION_SINGLE); 

        addAllItems(treeModel);
        return ;
    }

    static gboolean 
    isInFstab (const gchar *path) {
	gboolean result = FALSE;
	    TRACE("fsent.. isInFstab (%s)\n",path);
	struct fstab *fs;

	if(!setfsent ()) {
	    DBG("!setfsent ()\n");
	    return (FALSE);
	}

	for(fs = getfsent (); fs != NULL; fs = getfsent ()) {
	    if(strcmp (MNTTYPE_SWAP, fs->fs_vfstype) == 0)
		    continue;
	    TRACE("fstab/view.hh::isInFstab %s ? %s\n", path, fs->fs_file);

	    if(strcmp (path, fs->fs_file) == 0) {
		result = TRUE;    
		break;
	    }
	}
	endfsent ();
	return result;
    }

    static gchar *
    getMntType (const gchar *path) {
	struct fstab *fs;
	gchar *result = NULL;

	if(!setfsent ()) {
	    DBG("!setfsent ()\n");
	    return (NULL);
	}

	for(fs = getfsent (); fs != NULL; fs = getfsent ()) {
	    if(strcmp (path, fs->fs_file) == 0) {
		result = g_strdup(fs->fs_vfstype);
		break;
	    }
	}
	endfsent ();
	return result;
    }
    static gboolean
    isMounted (const gchar *path) {
	if(!path) {
		DBG ("fstab.i:private_is_mounted() mnt_point != NULL not met!\n");
		return FALSE;
	}
	TRACE("isMounted: %s\n", path);
	gchar *mnt_partition = getBsdPartition(path);
	if (mnt_partition){
		g_free(mnt_partition);
		return TRUE;
	}
	return FALSE;

    }
private:
    static gchar *
    getBsdPartition(const gchar *p){
	TRACE("getBsdPartition: %s\n", p);
	if (!p) return NULL;
	gchar *mnt_point = realpath((gchar *)p, NULL);
	if (!mnt_point) return NULL;

	//pthread_mutex_lock(&mntmutex);
	struct statfs *mnt_buf;
	size_t mnt_items = getmntinfo(&mnt_buf, MNT_NOWAIT);

	gint i=0;
	gchar *mnt_partition = NULL;
	for (;i<mnt_items; i++){
		if(strcmp (mnt_point, (mnt_buf+i)->f_mntonname) == 0 ||
		   strcmp (mnt_point, (mnt_buf+i)->f_mntfromname) == 0) {
			mnt_partition = g_strdup((mnt_buf+i)->f_mntfromname);
			break;
		}
	}
	//pthread_mutex_unlock(&mntmutex);
	g_free(mnt_point);
	TRACE("getBsdPartition: %s -> %s\n", p, mnt_partition);
	return mnt_partition;
    }
public:


    static gchar *
    getMntDir (const gchar * mnt_fsname) {return NULL;}
    static gchar *
    mountTarget (const gchar *label) {return NULL;}    
    static gchar *
    e2Label(const gchar *partitionPath){return NULL;}
    static gchar *
    id2Partition(const gchar *id){return NULL;} 
    static gchar *
    fsType(const gchar *partitionPath){return NULL;} 
    static gchar *
    partition2Id(const gchar *partition){return NULL;} 

    // mount from fstab data or directly
    static gboolean
    mountPath (View<Type> *view, const gchar *path, const gchar *mountPoint) 
    {
        DBG(" BSD pending... mount from fstab data or directly  \n");
        return FALSE;
    }
    static void
    removeAllItems(GtkTreeModel *treeModel){
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_first (treeModel, &iter)){
			while (gtk_list_store_remove (GTK_LIST_STORE(treeModel),&iter));
		}
    }
private:
//BSD


    static void 
    addAllItems(GtkTreeModel *treeModel){
        TRACE("addAllItems\n");
	RootView<Type>::addXffmItem(treeModel);
	//addFsentItems(treeModel);
        addPartitionItems(treeModel);
    }


/*	
    static GSList *
    partitions_list (void) {
        GSList *list=NULL;

        TRACE( "count partitions...\n");
        DIR *directory;
        struct dirent *d;
        directory = opendir ("/dev");
        if(!directory){
            DBG("cannot open /dev for read...\n");
            return 0;
        }
        while((d = readdir (directory)) != NULL) {
	    gboolean ok = FALSE;
	    if (strncmp (d->d_name, "da", strlen("da"))==0
		    ||
                strncmp (d->d_name, "ad", strlen("ad"))==0)
	    {
		if (strchr (d->d_name, 's') || strchr (d->d_name, 'p')) ok = TRUE;
	    }
	    if (strncmp (d->d_name, "wd", strlen("wd"))==0) ok = TRUE;
            if (ok){
		TRACE("gotcha: %s\n", d->d_name);
	    
		gchar *device = g_strdup_printf("/dev/%s", d->d_name);
		if (isMounted(device)){ 
		    list = g_slist_prepend(list, device); 
		} else g_free(device);
	    }
	
        }
        closedir (directory);
        

        return list;
    }
*/

    static void 
    addFsentItems (GtkTreeModel *treeModel) {
        TRACE("addFsentItems\n");
        auto list = fsentList();
        for (auto l=list; l && l->data; l=l->next){
            TRACE("BSD fstab item=%s\n", (const gchar *)l->data);
            addFsentItem(treeModel, (const gchar *)l->data);
        }
        // clear list
        clearFsentList(list);
        return;
    }

    static void
    addFsentItem(GtkTreeModel *treeModel, const gchar *path){
        if (!path){
            ERROR("fstab/view.hh::addPartition: path cannot be null\n");
            return;
        }
 	GtkTreeIter iter;
        gchar *basename = g_path_get_basename(path);
        gchar *mntDir = getMntDir(path);
        auto label = e2Label(basename);

	TRACE("fstab/addFsentItem()...\n");
        gboolean mounted = isMounted(path);
        gchar *text;
        auto fstype = fsType(path);        
        gchar *fileInfo = util_c::fileInfo(path);
 	//text = g_strdup_printf("<span size=\"large\">%s (%s)</span>\n<span color=\"red\">%s</span>\n%s %s\n%s",
 	text = g_strdup_printf("** %s (%s):\n%s\n%s %s\n%s",
			basename, 
                        label?label:_("No Label"),
                        fstype?fstype:_("There is no file system available (unformatted)"),
                        _("Mount point:"), mounted?mntDir:_("Not mounted"),
                        fileInfo);
        g_free(mntDir);
        g_free(fstype);
        if (label){
            gchar *g = g_strdup_printf("%s\n(%s)", basename, label);
            g_free(label);
            label = g;
           g_free(basename);
        } else {
           label = basename;
        }

        auto utf_name = util_c::utf_string(label);
        g_free(label);

        auto icon_name = (mounted)?"drive-harddisk/NW/greenball/3.0/180":
            "drive-harddisk/NW/grayball/3.0/180";
        auto highlight_name = "drive-harddisk/NW/blueball/3.0/225";
        auto treeViewPixbuf = Pixbuf<Type>::get_pixbuf(icon_name,  -24);
        auto normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  -48);
        auto highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  -48);   
        //auto uuid = partition2uuid(path);
        auto id = partition2Id(path);
        gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
        gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
                DISPLAY_NAME, utf_name, // path-basename or label
                ICON_NAME, icon_name,
                PATH, path, // absolute
                TREEVIEW_PIXBUF, treeViewPixbuf, 
                DISPLAY_PIXBUF, normal_pixbuf,
                NORMAL_PIXBUF, normal_pixbuf,
                HIGHLIGHT_PIXBUF, highlight_pixbuf,
                TOOLTIP_TEXT,text,
                DISK_ID, id,
                -1);
        g_free(utf_name);
        // fstype is constant
        g_free(id);
        // icon_name is constant
        // path is constant
        // pixbufs belong to pixbuf hash
        g_free(text);
    }

    static void
    clearFsentList(GSList *list){
        for (auto l=list; l && l->data; l=l->next){
                g_free(l->data);
        }
        g_slist_free(list);
    }

    static GSList *
    fsentList (void) {
        // with ZFS, this returns NULL.
        GSList *list=NULL;
        if(!setfsent ()) {
            return NULL;
        }
        GSList *elements = NULL;
        struct fstab *fs;
        for(fs = getfsent (); fs != NULL; fs = getfsent ()) {
            if (!g_path_is_absolute(fs->fs_file)) continue;
            TRACE("elements_list: %s\n", fs->fs_file);
            elements = g_slist_prepend(elements, g_strdup(fs->fs_file));
        }
        endfsent ();
        return elements;
    }




#if 0

	static gboolean
	    

	static gboolean
	isUserType (const gchar * mnt_point) {
		return FALSE;
	}

	static gboolean
	include_in_xfdir (struct fstab *fs) {
		if(strcmp (MNTTYPE_SWAP, fs->fs_vfstype) == 0)
			return FALSE;
		if(!g_file_test (fs->fs_file, G_FILE_TEST_IS_DIR))
			return FALSE;
		return TRUE;
	}

	static void
	set_fs_type(record_entry_t *en, const gchar *fs_vfstype){
		SET_FSTAB_TYPE (en->type);
		if(strcmp (MNTTYPE_CDFS, fs_vfstype) == 0) SET_CDFS_TYPE (en->type);
		else if(strcmp (MNTTYPE_CODAFS, fs_vfstype) == 0) SET_NFS_TYPE (en->type);
		else if(strcmp (MNTTYPE_KERNFS, fs_vfstype) == 0) SET_PROC_TYPE (en->type);
		else if(strcmp (MNTTYPE_MFS, fs_vfstype) == 0) SET_PROC_TYPE (en->type);
		else if(strcmp (MNTTYPE_NFS, fs_vfstype) == 0) SET_NFS_TYPE (en->type);
		else if(strcmp (MNTTYPE_PROCFS, fs_vfstype) == 0) SET_PROC_TYPE (en->type);
		else if(strcmp (MNTTYPE_SMBFS, fs_vfstype) == 0) SET_SMB_TYPE (en->type);
		return;
	}
	static GSList *
	elements_list (void) {
		GSList *list=NULL;
		if(!setfsent ()) {
			return (0);
		}
		struct fstab *fs;
		int i;
		typedef struct fstab_t{
			gchar *fs_file;
			gchar *fs_vfstype;
		}fstab_t;

		GSList *elements = NULL;
		for(i = 0, fs = getfsent (); fs != NULL; fs = getfsent ()) {
			if(!include_in_xfdir (fs)) continue;
			TRACE("elements_list: %s\n", fs->fs_file);
			fstab_t *f = (fstab_t *)calloc(1,sizeof(fstab_t));
			if (!f) g_error("calloc: %s \n", strerror(errno));
			f->fs_file = g_strdup(fs->fs_file);
			f->fs_vfstype = g_strdup(fs->fs_vfstype);
			elements = g_slist_prepend(elements, f);
			i++;
		}
		endfsent ();

		for(auto tmp = elements; tmp && tmp->data; tmp = tmp->next) {
			fstab_t *f = tmp->data;
			record_entry_t *en = rfm_stat_entry(f->fs_file, 0);
			set_fs_type(en, f->fs_vfstype);
			list = g_slist_prepend(list, en); 
			g_free(f->fs_file);
			g_free(f->fs_vfstype);
			g_free(f);
		}
		g_slist_free(elements);

		if (!infomutex) rfm_mutex_init(infomutex);
		g_mutex_lock(infomutex);
		struct statfs *mntbuf;
		gint count = getmntinfo(&mntbuf,  MNT_NOWAIT);
		if (count){
			gint j;
			for (j=0; j<count; j++){
				for (auto tmp=list; tmp && tmp->data; tmp = tmp->next){
					record_entry_t *en = tmp->data;
					if (strcmp(en->path, (mntbuf+j)->f_mntonname)==0) break;
				}
				if (!tmp) {
					record_entry_t *en = rfm_stat_entry((mntbuf+j)->f_mntonname, 0);
					set_fs_type(en, (mntbuf+j)-> f_fstypename);
					list=g_slist_prepend(list, en);
				}
			}
		}
		g_mutex_unlock(infomutex);
		return list;
	}

	static void
	clearSList(GSList **list_p){
		GSList *tmp;
		for (tmp=*list_p; tmp && tmp->data; tmp = tmp->next){
			record_entry_t *en = tmp->data;
			TRACE("clearing item: %s\n", en->path);
			rfm_destroy_entry(en);
		}
		g_slist_free(*list_p);
		return;
	}

	static gint
	countElements (void) {
		GSList *list = elements_list();
		gint count = g_slist_length(list);
		clear_slist(&list);
		return count;
	}

	static int
	countPartitions (void) {
		TRACE("count partitions...\n");
		GSList *list = partitions_list();
		gint count = g_slist_length(list);
		clear_slist(&list);
		return count;
	}

	static gchar *
	getMntDir (gchar * mnt_fsname) {
		struct fstab *fs;

		if(!setfsent ()) {
			return (0);
		}

		gchar *mnt_dir = NULL;
		for(fs = getfsent (); fs != NULL; fs = getfsent ()) {
			if(!include_in_xfdir (fs))
				continue;

			if(strcmp (mnt_fsname, fs->fs_spec) == 0) {
				//if(strcmp (mnt_fsname, mnt_struct->mnt_fsname) == 0) {
				// hit: multiple entries use first listed 
				// user types have preference and use last listed 
				if(strstr (fs->fs_mntops, "user")) {
					g_free (mnt_dir);
					mnt_dir = g_strdup (fs->fs_file);
				}
				if(!mnt_dir) {
					mnt_dir = g_strdup (fs->fs_file);
				}
			}
		}

		endfsent ();
		return mnt_dir;
	}

	static  gchar *
	getMntFsname (gchar * mnt_dir) {
		struct fstab *fs;

		if(!setfsent ()) {
			return (0);
		}


		gchar *mnt_fsname = NULL;

		for(fs = getfsent (); fs != NULL; fs = getfsent ()) {
			if(!include_in_xfdir (fs))
				continue;
			if(strcmp (mnt_dir, fs->fs_file) == 0) {
				//if(strcmp (mnt_dir, mnt_struct->mnt_dir) == 0) {
				// hit: multiple entries use first listed 
				// user types have preference and use last listed 
				if(strstr (fs->fs_mntops, "user")) {
					g_free (mnt_fsname);
					mnt_fsname = g_strdup (fs->fs_spec);
				}
				if(!mnt_fsname) {
					mnt_fsname = g_strdup (fs->fs_spec);
				}
			}
		}
		endfsent ();
		return mnt_fsname;
	}
	static void
	set_mounts_info (record_entry_t * en) {
		if (!en || !en->path) return;
		const gchar *mnt_point = en->path;
		pthread_mutex_lock(&mntmutex);
		struct statfs *mnt_buf;
		size_t mnt_items = getmntinfo(&mnt_buf, MNT_NOWAIT);
		gchar *mnt_to=NULL;
		gint i=0; for (;i<mnt_items; i++){
		TRACE("%s == %s or %s\n", mnt_point,
			(mnt_buf+i)->f_mntonname, (mnt_buf+i)->f_mntfromname);
		
			if(strcmp (mnt_point, (mnt_buf+i)->f_mntonname) == 0 ||
			   strcmp (mnt_point, (mnt_buf+i)->f_mntfromname) == 0) {
			TRACE("match\n");
			mnt_to = g_strdup((mnt_buf+i)->f_mntonname);
			break;
		}
		}
		pthread_mutex_unlock(&mntmutex);
		
		g_free(en->tag);
		en->tag = mnt_to;
	}
	static xfdir_t *
	private_get_xfdir (xfdir_t * xfdir_p) {
		struct fstab *fs;

		TRACE("elements_list ()\n");
		GSList *list = elements_list ();
		gint elements = g_slist_length(list);
		TRACE("partitions_list ()\n");
		
		GSList *p_list = partitions_list ();
		gint partitions = g_slist_length(p_list);

	 
		TRACE("malloc_items %d\n", elements+partitions);
		gint first = malloc_items(xfdir_p, elements+partitions);
		// g_error taken care of within function.
		GSList *tmp=list;

		gint i;
		for(i = first; tmp && tmp->data; tmp=tmp->next) {
		record_entry_t *en = tmp->data;
		xfdir_p->gl[i].en = en;
			xfdir_p->gl[i].pathv = g_strdup (en->path);
			TRACE ("fstab element: %d --> %s\n", i, en->path);
			i++;
		}
		tmp=p_list;
		for(; tmp && tmp->data; tmp=tmp->next) {
			record_entry_t *en = tmp->data;
			xfdir_p->gl[i].en = en;
			xfdir_p->gl[i].pathv = g_strdup (en->path);
			TRACE ("fstab partition: %d --> %s\n", i, en->path);
			i++;
		}

		g_slist_free(list);
		g_slist_free(p_list);
		return (xfdir_p);
	}


	static gchar *
	df (void) {
		gchar *df_string = NULL;
		int line_count = 0;
		char line[2048];
		FILE *pipe;
		memset ((void *)line, 0, 2048);
		gchar *command = g_find_program_in_path ("df");
		pipe = popen (command, "r");
		g_free (command);
		if(!pipe) {
			DBG ("unable to pipe df\n");
			return "";
		}
		while(fgets (line, 2047, pipe) && !feof (pipe)) {
			line_count++;
		}
		pclose (pipe);
		df_string = g_strdup_printf ("line_count=%d", line_count);
		TRACE ("DF: %s\n", df_string);
		return df_string;
	}


	static gchar *
	entryTip (gchar *path) {
		if(!path) return NULL;
		gchar *mnt_point = realpath(path, NULL);
		if (!mnt_point) return NULL;
		
		


		pthread_mutex_lock(&mntmutex);
		struct statfs *mnt_buf;
		size_t mnt_items = getmntinfo(&mnt_buf, MNT_NOWAIT);

		gchar *mnt_to=NULL;
		gchar *mnt_from=NULL;
		gint item=-1;
		gint i=0;
		for (;i<mnt_items; i++){
			TRACE("%s == %s or %s\n", mnt_point,
			(mnt_buf+i)->f_mntonname, (mnt_buf+i)->f_mntfromname);
		
			if(strcmp (mnt_point, (mnt_buf+i)->f_mntonname) == 0 ||
			   strcmp (mnt_point, (mnt_buf+i)->f_mntfromname) == 0) {
				TRACE("match\n");
				mnt_to = g_strdup((mnt_buf+i)->f_mntonname);
				mnt_from = g_strdup((mnt_buf+i)->f_mntfromname);
				item = i;
				break;
			}
		}
		pthread_mutex_unlock(&mntmutex);
		TRACE("item = %d\n", item);
		if (item < 0){
			return mnt_point;
		}
		
		gchar *text = g_strdup_printf("%s%s \n%s%s\n",
			_("Mount point: "), mnt_to?mnt_to:"none",
			_("Mount device: "), mnt_from?mnt_from:"none");
		g_free(mnt_point);
		g_free(mnt_to);
		g_free(mnt_from);
		return text;
	}

#endif


#endif
