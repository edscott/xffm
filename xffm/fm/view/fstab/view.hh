#ifndef XF_FSTAB_HH 
#define XF_FSTAB_HH
#ifdef ENABLE_FSTAB_MODULE


// FIXME: Are these useful any more?
#define MNTTYPE_PROCFS	"proc"
#define MNTTYPE_SMBFS	"smbfs"
#define MNTTYPE_DEV	"devpts"
#define MNTTYPE_SHM	"tmpfs"
#define	MNTTYPE_CDFS	"iso9660"
#define MNTTYPE_SWAP	"swap"
#define MNTTYPE_NFS	"nfs"

#define MNTTYPE_CACHEFS	"cachefs"       // proc
#define MNTTYPE_HSFS	"hsfs"  // proc

#define MNTTYPE_CODAFS	"coda"  //nfs
#define MNTTYPE_CDFS_AIX "cdrfs"        /* AIX */
#define MNTTYPE_KERNFS	"kernfs"        // proc
#define MNTTYPE_MFS	"mfs"   //proc

/*
msgid "eCryptfs Volume"
msgid "SSHFS Remote Synchronization Folder"
msgid "SFTP over SSH2"
msgid "SFTP (via SSH)"
msgid "Secure FTP (SFTP)"
msgid "Mount"
msgid "Mount %s"
msgid "Mount point"
msgid "Mount Point"
msgid "Mount Volume"
msgid "Mount user-space filesystems (FUSE)"
msgid "FUSE Volume"
msgid "Mount Helper"
msgid "Mount user-space filesystems (FUSE)"
msgid "Mount local disks and devices"
msgid "Unix networks (NFS)"
msgid "CIFS Volume"
msgid "NFS Network Volume"
msgid "NFS remote directory"
*/


#ifdef FREEBSD_FOUND
// BSD FIXME conditionals...
# include <fstab.h>
# include <sys/ucred.h>

# include <sys/param.h>
# ifdef HAVE_SYS_PARAM_H
#  include <sys/param.h>
# endif


# include <sys/mount.h>
# ifdef HAVE_SYS_MOUNT_H
#  include <sys/mount.h>
# endif

# ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif

# ifdef HAVE_sys_sysctl_H
#  include <sys/sysctl.h>
# endif

#else
// Linux
# ifdef HAVE_MNTENT_H
#  include <mntent.h>
# else
#  error "Linux: <mntent.h> not found"
# endif

#endif


#include "popup.hh"
#include "monitor.hh"
namespace xf {
template <class Type> class FstabPopUp;

#ifdef FREEBSD_FOUND

static pthread_mutex_t fsmutex = G_STATIC_MUTEX_INIT;
static pthread_mutex_t mntmutex = PTHREAD_MUTEX_INITIALIZER;
static GMutex *infomutex=NULL;


template <class Type>
class FstabView: public FstabPopUp<Type> {
    using pixbuf_c = Pixbuf<double>;
    using util_c = Util<double>;
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

    static void
    removeAllItems(GtkTreeModel *treeModel){
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_first (treeModel, &iter)){
			while (gtk_list_store_remove (GTK_LIST_STORE(treeModel),&iter));
		}
    }
    static guint
    getMntType (const gchar *path) {return 0;}
    static gchar *
    getMntDir (const gchar * mnt_fsname) {return NULL;}
    static gboolean
    isMounted (const gchar *mnt_fsname){return FALSE;}  
    static gchar *
    mountTarget (const gchar *label) {return NULL;}    
    static gboolean
    isInFstab (const gchar *path) {return FALSE;}
    static gboolean
    mountPath (View<Type> *view, const gchar *path, const gchar *mountPoint)
     	{return FALSE;}
	static gchar *
    e2Label(const gchar *partitionPath){return NULL;}
	static gchar *
    id2Partition(const gchar *id){return NULL;}

private:
//BSD


    static void 
    addAllItems(GtkTreeModel *treeModel){
		RootView<Type>::addXffmItem(treeModel);
		addFsentItems(treeModel);
        //addPartitionItems(treeModel);
	}

	static void 
    addFsentItems (GtkTreeModel *treeModel) {
		auto list = fsentList();
		for (auto l=list; l && l->data; l=l->next){
			DBG("BSD fstab item=%s\n", (const gchar *)l->data);
		}
		// clear list
		clearFsentList(list);
        return;
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
		GSList *list=NULL;
		pthread_mutex_lock(&fsmutex);
		if(!setfsent ()) {
			pthread_mutex_unlock(&fsmutex);
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
	static gchar *
	getBsdPartition(const gchar *p){
		TRACE("getBsdPartition: %s\n", p);
		if (!p) return NULL;
		gchar *mnt_point = realpath((gchar *)p, NULL);
		if (!mnt_point) return NULL;

		pthread_mutex_lock(&mntmutex);
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
		pthread_mutex_unlock(&mntmutex);
		g_free(mnt_point);
		TRACE("getBsdPartition: %s -> %s\n", p, mnt_partition);
		return mnt_partition;
	}
	static gboolean
	isMounted (const gchar *p) {
		if(!p) {
			DBG ("fstab.i:private_is_mounted() mnt_point != NULL not met!\n");
			return FALSE;
		}
		TRACE("private_is_mounted: %s\n", p);
		gchar *mnt_partition = getBsdPartition(p);
		if (mnt_partition){
			g_free(mnt_partition);
			return TRUE;
		}
		return FALSE;

	}
	    

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
		pthread_mutex_lock(&fsmutex);
		if(!setfsent ()) {
			pthread_mutex_unlock(&fsmutex);
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

		pthread_mutex_unlock(&fsmutex);
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
		pthread_mutex_lock(&fsmutex);

		if(!setfsent ()) {
			pthread_mutex_unlock(&fsmutex);
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
		pthread_mutex_unlock(&fsmutex);
		return mnt_dir;
	}

	static  gchar *
	getMntFsname (gchar * mnt_dir) {
		struct fstab *fs;
		pthread_mutex_lock(&fsmutex);

		if(!setfsent ()) {
			pthread_mutex_unlock(&fsmutex);
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
		pthread_mutex_unlock(&fsmutex);
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
	static void *
	isInFstab (void *p) {
		int result = 0;
		struct fstab *fs;
		const gchar *path = (const gchar *)p;
		pthread_mutex_lock(&fsmutex);

		if(!setfsent ()) {
			pthread_mutex_unlock(&fsmutex);
			return (0);
		}


		for(fs = getfsent (); fs != NULL; fs = getfsent ()) {
			if(strcmp (MNTTYPE_SWAP, fs->fs_vfstype) == 0)
				continue;
			if(!rfm_g_file_test (fs->fs_file, G_FILE_TEST_IS_DIR))
				continue;

			if(strcmp (path, fs->fs_file) == 0) {
				if(strcmp (MNTTYPE_CDFS, fs->fs_vfstype) == 0)
					result = __CDFS_TYPE;
				else if(strcmp (MNTTYPE_CODAFS, fs->fs_vfstype) == 0)
					result = __NFS_TYPE;
				else if(strcmp (MNTTYPE_KERNFS, fs->fs_vfstype) == 0)
					result = __PROC_TYPE;
				else if(strcmp (MNTTYPE_MFS, fs->fs_vfstype) == 0)
					result = __PROC_TYPE;
				else if(strcmp (MNTTYPE_NFS, fs->fs_vfstype) == 0)
					result = __NFS_TYPE;
				else if(strcmp (MNTTYPE_PROCFS, fs->fs_vfstype) == 0)
					result = __PROC_TYPE;
				else if(strcmp (MNTTYPE_SMBFS, fs->fs_vfstype) == 0)
					result = __SMB_TYPE;
				else
					result = -1;
				break;
			}
		}
		endfsent ();
		pthread_mutex_unlock(&fsmutex);
		return GINT_TO_POINTER (result);
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

};

#else
    // XXX work in progress... this is Linux Version. 



template <class Type>
class FstabView: public FstabPopUp<Type> {
    using pixbuf_c = Pixbuf<double>;
    using util_c = Util<double>;
public:

    static FstabMonitor<Type> *
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
	// Linux monitor
        FstabMonitor<Type> *p = new(FstabMonitor<Type>)(treeModel, view);
        p->start_monitor(view, "/dev/disk/by-id");
        // already in start_monitor function:
        // view->setMonitorObject(p);
        DBG("parallel fstab monitor %p for fstab\n", p); 
//        p->start_monitor(treeModel, "/dev/disk/by-partuuid");
        return p;
    }

    static void
    removeAllItems(GtkTreeModel *treeModel){
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_first (treeModel, &iter)){
			while (gtk_list_store_remove (GTK_LIST_STORE(treeModel),&iter));
		}
    }

	// LINUX
	//
	//
    static void
    addDisksItem(GtkTreeModel *treeModel){ 
 	GtkTreeIter iter;
	// Root
	auto name = "/dev/disk";
	//Since /dev is not concrete, the following test returns false...
	//
	//if (!g_file_test(name, G_FILE_TEST_IS_DIR)) return;

	auto utf_name = util_c::utf_string(_("Disks"));
	auto icon_name = "drive-multidisk";
	auto highlight_name = g_strconcat(icon_name, "/", HIGHLIGHT_EMBLEM, NULL);

        auto treeViewPixbuf = Pixbuf<Type>::get_pixbuf(icon_name,  -24);
	auto normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  -48);
	auto highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  -48);  
	gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
	gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
		DISPLAY_NAME, utf_name,
		ICON_NAME, icon_name,
                PATH, name,
                TREEVIEW_PIXBUF, treeViewPixbuf, 
		DISPLAY_PIXBUF, normal_pixbuf,
		NORMAL_PIXBUF, normal_pixbuf,
		HIGHLIGHT_PIXBUF, highlight_pixbuf,
		TOOLTIP_TEXT,_("Disks"),

		-1);
	g_free(utf_name);
    }

    static void 
    addAllItems(GtkTreeModel *treeModel){
		RootView<Type>::addXffmItem(treeModel);
		addDisksItem(treeModel);
		//addNFSItem(treeModel);
		//addEcryptFSItem(treeModel);
		//addSSHItem(treeModel);
		//addCIFSItem(treeModel);
        addPartitionItems(treeModel);


    }

    static gchar *
    fsType(const gchar *partitionPath){
        gchar *command = g_strdup_printf("lsblk -no FSTYPE %s", partitionPath);
	FILE *pipe = popen (command, "r");
	if(pipe == NULL) {
	    ERROR("fstab/view.hh::Cannot pipe from %s\n", command);
	    g_free(command);
	    return NULL;
	}
	g_free(command);

        gchar line[256];
        memset(line, 0, 256);
	while (fgets (line, 255, pipe) && !feof(pipe)) {
	    if (strchr(line,'\n')) *strchr(line,'\n') = 0;
	    if (strstr(line, "swap")) return NULL;
	    if (strcmp(line, "")==0) return NULL;
	    break;
	}
        pclose (pipe);
	return g_strdup(line);
    }

    static gchar *
    partition2Id(const gchar *partition){ // disk partition only
        gchar *base = g_path_get_basename(partition);
        const gchar *command = "ls -l /dev/disk/by-id";
	FILE *pipe = popen (command, "r");
	if(pipe == NULL) {
	    ERROR("fstab/view.hh::Cannot pipe from %s\n", command);
	    return NULL;
	}
        gchar line[256];
        memset(line, 0, 256);
        gchar *id = NULL;
	while (fgets (line, 255, pipe) && !feof(pipe)) {
            if (!strstr(line, base)) {
                TRACE("%s not in %s\n", base, line);
                continue;
            }
            gchar **f = g_strsplit(line, "->", 2);
            if (!strstr(f[1], base)){
                TRACE("%s not in %s\n", base, f[1]); 
                g_strfreev(f);
                continue;
            }
            g_strstrip(f[0]);
            if (!strrchr(f[0], ' ')){
                ERROR("fstab/view.hh::partition2Id(): no space-chr in id\n");
                continue;
            }
            id = g_path_get_basename(strrchr(f[0], ' ')+1);
            g_strfreev(f);
            break;
	}
        pclose (pipe);
        TRACE("partition2Id() %s->%s\n", partition, id);
	return id;
    }

    static gchar *
    id2Partition(const gchar *id){ // disk partition only
        gchar *baseId = g_path_get_basename(id);
        const gchar *command = "ls -l /dev/disk/by-id";
	FILE *pipe = popen (command, "r");
	if(pipe == NULL) {
	    ERROR("fstab/view.hh::Cannot pipe from %s\n", command);
	    return NULL;
	}
        gchar line[256];
        memset(line, 0, 256);
        gchar *partition = NULL;
	while (fgets (line, 255, pipe) && !feof(pipe)) {
            if (!strstr(line, "-part")) continue;
            gchar **f = g_strsplit(line, "->", 2);
            if (!strstr(f[0], baseId)){
                g_strfreev(f);
                continue;
            }
            if (strchr(f[1], '\n')) *strchr(f[1], '\n') = 0;
            g_strstrip(f[1]);
            gchar *g = g_path_get_basename(f[1]);
            partition = g_strconcat("/dev/", g, NULL);
            g_free(g);
            g_strfreev(f);
            break;
	}
        pclose (pipe);
	return partition;
    }

    static gchar *
    e2Label(const gchar *partitionPath){
	if (!partitionPath) return NULL;
        const gchar *command = "ls -l /dev/disk/by-label";
	FILE *pipe = popen (command, "r");
	if(pipe == NULL) {
	    ERROR("fstab/view.hh::Cannot pipe from %s\n", command);
	    return NULL;
	}
        auto partition = g_path_get_basename(partitionPath); 
        gchar line[256];
        memset(line, 0, 256);
        gchar *label = NULL;
	while (fgets (line, 255, pipe) && !feof(pipe)) {
            if (strchr(line, '\n')) *strchr(line, '\n')=0;
            if (!strstr(line, "->")) continue;
            gchar **f = g_strsplit(line, "->", 2);
            gchar *base = g_path_get_basename(f[1]);
            TRACE("looking for %s in %s\n", base, partition);
            if (!strstr(partition, base)){
                g_free(base);
                g_strfreev(f);
                continue;
            }
            else TRACE("found it..\n");
            g_free(base);
            g_strstrip(f[0]);
            if (strrchr(f[0], ' ')){
                label = g_strdup(strrchr(f[0], ' ')+1);
                g_strfreev(f);
                break;
            }
	}
        pclose (pipe);
        g_free(partition);
	return label;

    }

    static gchar *
    getPartitionPath(const gchar *line){
        if(strlen (line) < 5) return NULL;
        if(strchr (line, '#')) return NULL;
        TRACE ("partitions: %s\n", line);
        if (!strrchr (line, ' ')) return NULL;
        gchar *p = g_strdup(strrchr (line, ' '));
        g_strstrip (p);
        TRACE ("partitions add input: %s\n", p);
        if(!strlen (p)) {
        g_free(p);
            return NULL;
        }
        gchar *path = NULL;
        if (strncmp(p, "sd", 2) == 0 || strncmp(p, "hd", 2)==0){
            //if (p[3] < '0' || p[3] >'9') return NULL;
            path = g_strdup_printf ("/dev/%s", p);
        }
        g_free(p);
        TRACE ("partitions add output: %s\n", path);
        return path;
    }

 
   static gchar *
    getPartitionDiskPath(const gchar *line){
        if(strlen (line) < 5) return NULL;
        if(strchr (line, '#')) return NULL;
        TRACE ("partitions: %s\n", line);
        if (!strrchr (line, ' ')) return NULL;
        gchar *p = g_strdup(strrchr (line, ' '));
        g_strstrip (p);
        TRACE ("partitions add: %s\n", p);
        if(!strlen (p)) {
            g_free(p);
            return NULL;
        }
        if (strncmp(p, "sd", 2) == 0 || strncmp(p, "hd", 2)==0){
            if (p[3] >= '0' || p[3] <= '9') return NULL;
            gchar *path = g_strdup_printf ("/dev/%s", p);
            g_free(p);
            return path;
        }
        g_free(p);
        return NULL;
    }

    static gchar *
    partition2uuid(const gchar *partitionPath){
        // Returns basename of partition uuid.
        const gchar *command = "ls -l /dev/disk/by-partuuid";
	FILE *pipe = popen (command, "r");
	if(pipe == NULL) {
	    ERROR("fstab/view.hh::Cannot pipe from %s\n", command);
	    return NULL;
	}
        gchar *partition = g_path_get_basename(partitionPath);
        gchar line[256];
        memset(line, 0, 256);
        gchar *uuid = NULL;
	while (fgets (line, 255, pipe) && !feof(pipe)) {
            if (strchr(line, '\n')) *strchr(line, '\n') = 0;
            TRACE("%s: %s\n", partition, line);
            if (strstr(line, "->") && strstr(line, partition)) {
                *strstr(line, "->") = 0;
                g_strstrip(line);
                if (strrchr(line, ' ')){
                    uuid = g_strdup(strrchr(line, ' '));
                    g_strstrip(uuid);
                }
                break;
            }
	}
        pclose (pipe);
        g_free(partition);
	return uuid;
    }

    static void
    addPartition(GtkTreeModel *treeModel, const gchar *path){
        if (!path){
            ERROR("fstab/view.hh::addPartition: path cannot be null\n");
            return;
        }
 	GtkTreeIter iter;
        gchar *basename = g_path_get_basename(path);
        gchar *mntDir = getMntDir(path);
        auto label = e2Label(basename);


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

    static void // Linux
    addPartitionItems (GtkTreeModel *treeModel) {
	FILE *partitions = fopen ("/proc/partitions", "r");
        if(!partitions) return;

        gchar line[1024];
        memset (line, 0, 1024);
        while(fgets (line, 1023, partitions) && !feof (partitions)) {
            gchar *path = getPartitionPath(line);
            if (!path) continue; // not a partition path line...
            if (!g_path_is_absolute(path)){
                ERROR("fstab/view.hh::partition path should be absolute: %s\n", path);
                continue;
            }
	    //gchar *fstype = fsType(path);
            //if (fstype) 
                addPartition(treeModel, path);
            g_free(path);
            //g_free(fstype);
            memset (line, 0, 1024);
        }
        fclose (partitions);
        return;
    }
/*
    static gboolean // Linux
    addPartitionItems(GtkTreeModel *treeModel){
	if (!g_file_test("/dev/disk/by-id", G_FILE_TEST_IS_DIR)) return FALSE;
        const gchar *command = "ls -l /dev/disk/by-id";
	FILE *pipe = popen (command, "r");
	if(pipe == NULL) {
	    ERROR("fstab/view.hh::Cannot pipe from %s\n", command);
	    return FALSE;
	}

        gchar line[256];
        memset(line, 0, 256);
        gchar *id = NULL;
	while (fgets (line, 255, pipe) && !feof(pipe)) {
            if (strchr(line, '\n')) *strchr(line, '\n') = 0;
            TRACE("addPartitionItems: %s\n", line);
            if (strstr(line, "->")==NULL) continue;
	    auto p = g_strsplit(line, "->", 2);

	    && strstr(line, partition)) {
                *strstr(line, "->") = 0;
                g_strstrip(line);
                if (strrchr(line, ' ')){
                    uuid = g_strdup(strrchr(line, ' '));
                    g_strstrip(uuid);
                }
                break;
            }
	}
        pclose (pipe);
        g_free(partition);
	return uuid;
    }
*/
    
    static void
    addNFSItem(GtkTreeModel *treeModel){
 	GtkTreeIter iter;
	auto name = "xffm:nfs";
	auto utf_name = util_c::utf_string(_("NFS Network Volume"));
	auto icon_name = "video-display/SE/emblem-nfs/2.0/225";
	auto highlight_name = "video-display/SE/emblem-nfs/2.0/225/NE/document-open/2.0/225";
        auto treeViewPixbuf = Pixbuf<Type>::get_pixbuf(icon_name,  -24);
	auto normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  -48);
	auto highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  -48);   
	gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
	gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
		//DISK_LABEL, utf_name,
		//PARTUUID, name,
		ICON_NAME, icon_name,
                PATH, name,
                TREEVIEW_PIXBUF, treeViewPixbuf, 
		DISPLAY_PIXBUF, normal_pixbuf,
		NORMAL_PIXBUF, normal_pixbuf,
		HIGHLIGHT_PIXBUF, highlight_pixbuf,
		TOOLTIP_TEXT,"xffm:nfs",

		-1);
	g_free(utf_name);
    }

    static void
    addSSHItem(GtkTreeModel *treeModel){
 	GtkTreeIter iter;
	auto name = "xffm:sshfs";
	auto utf_name = util_c::utf_string(_("SFTP (via SSH)"));
	auto icon_name = "video-display/SE/emblem-ssh/2.0/225";
	auto highlight_name = "video-display/SE/emblem-ssh/2.0/225/NE/document-open/2.0/225";
        auto treeViewPixbuf = Pixbuf<Type>::get_pixbuf(icon_name,  -24);
	auto normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  -48);
	auto highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  -48);   
	gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
	gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
		//DISK_LABEL, utf_name,
		//PARTUUID, name,
		ICON_NAME, icon_name,
                PATH, name,
                TREEVIEW_PIXBUF, treeViewPixbuf, 
		DISPLAY_PIXBUF, normal_pixbuf,
		NORMAL_PIXBUF, normal_pixbuf,
		HIGHLIGHT_PIXBUF, highlight_pixbuf,
		TOOLTIP_TEXT,"xffm:sshfs",

		-1);
	g_free(utf_name);
    }

    static void
    addEcryptFSItem(GtkTreeModel *treeModel){
 	GtkTreeIter iter;
	auto name = "xffm:ecryptfs";
	auto utf_name = util_c::utf_string(_("eCryptfs Volume"));
	auto icon_name = "video-display/SE/emblem-lock/2.0/225";
	auto highlight_name = "video-display/SE/emblem-lock/2.0/225/NE/document-open/2.0/225";
        auto treeViewPixbuf = Pixbuf<Type>::get_pixbuf(icon_name,  -24);
	auto normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  -48);
	auto highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  -48);   
	gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
	gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
		//DISK_LABEL, utf_name,
		//PARTUUID, name,
		ICON_NAME, icon_name,
                PATH, name,
                TREEVIEW_PIXBUF, treeViewPixbuf, 
		DISPLAY_PIXBUF, normal_pixbuf,
		NORMAL_PIXBUF, normal_pixbuf,
		HIGHLIGHT_PIXBUF, highlight_pixbuf,
		TOOLTIP_TEXT,"xffm:ecryptfs",

		-1);
	g_free(utf_name);
    }

    static void
    addCIFSItem(GtkTreeModel *treeModel){
 	GtkTreeIter iter;
	auto name = "xffm:cifs";
	auto utf_name = util_c::utf_string(_("CIFS Volume"));
	auto icon_name = "video-display/SE/emblem-smb/2.0/225";
	auto highlight_name = "video-display/SE/emblem-smb/2.0/225/NE/document-open/2.0/225";
        auto treeViewPixbuf = Pixbuf<Type>::get_pixbuf(icon_name,  -24);
	auto normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  -48);
	auto highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  -48);   
	gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
	gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
		//DISK_LABEL, utf_name,
		//PARTUUID, name,
		ICON_NAME, icon_name,
                PATH, name,
                TREEVIEW_PIXBUF, treeViewPixbuf, 
		DISPLAY_PIXBUF, normal_pixbuf,
		NORMAL_PIXBUF, normal_pixbuf,
		HIGHLIGHT_PIXBUF, highlight_pixbuf,
		TOOLTIP_TEXT,"xffm:cifs",

		-1);
	g_free(utf_name);
    }
   
#ifdef HAVE_MNTENT_H

    static guint
    getMntType (const gchar *path) {
        struct mntent *mnt_struct;
        TRACE("FSTAB:  parsing %s\n", "/etc/fstab");
        FILE *fstab_fd = setmntent ("/etc/fstab", "r");

        guint type=0;
        struct mntent mntbuf;
        gchar buf[2048]; 
        gboolean found = FALSE;
        while ((mnt_struct = getmntent_r (fstab_fd, &mntbuf, buf, 2048)) != NULL) {

            if (strcmp(path, mnt_struct->mnt_dir) && strcmp(path, mnt_struct->mnt_fsname)) continue;
            TRACE ("getMntType(): found item %s\n", path);
            // Flag is as present in fstab
            SET_FSTAB_TYPE (type);
            // Flag it as set with "-o user"
            if(strstr (mnt_struct->mnt_opts, "user")) {
                SET_USER_TYPE (type);
            }
            /* Set fs type */
            TRACE ("getMntType(): checking %s for fstab type\n", mnt_struct->mnt_type);
            gint fstabType = getFstabType (mnt_struct->mnt_type);
            switch (fstabType) {
            case __NFS_TYPE:
                SET_NFS_TYPE (type);
                break;
            case __CDFS_TYPE:
                SET_CDFS_TYPE (type);
                break;
            case __PROC_TYPE:
                SET_PROC_TYPE (type);
                break;
            case __SMB_TYPE:
                SET_SMB_TYPE (type);
                break;
            default:
                break;
            }
            found = TRUE;
        }
        (void)endmntent (fstab_fd);
        if (!found) ERROR("fstab/view.hh::getMntType (): %s not found in /etc/fstab\n", path);
        return type;
    }

    static gchar *
    getMntDir (const gchar * mnt_fsname) {
        if (!mnt_fsname) return NULL;
        FILE *fstab_fd = setmntent ("/etc/mtab", "r");
        if(!fstab_fd)
            return NULL;
        struct mntent *mnt_struct;
        gchar *mnt_dir = NULL;
        struct mntent mntbuf;
        gchar buf[2048]; 
        while ((mnt_struct = getmntent_r (fstab_fd, &mntbuf, buf, 2048)) != NULL) {
            if(strcmp (mnt_fsname, mnt_struct->mnt_fsname) == 0) {
                // hit: multiple entries use first listed 
                // user types have preference and use last listed 
                if(strstr (mnt_struct->mnt_opts, "user")) {
                    g_free (mnt_dir);
                    mnt_dir = g_strdup (mnt_struct->mnt_dir);
                }
                if(!mnt_dir) {
                    mnt_dir = g_strdup (mnt_struct->mnt_dir);
                }
            }
        }
        (void)endmntent (fstab_fd);
        return mnt_dir;
    }

  
    static gboolean
    isMounted (const gchar *mnt_fsname) {

        if(!mnt_fsname) {
            ERROR ("fstab/view.hh::isMounted() mnt_point != NULL not met!\n");
            return FALSE;
        }
        gchar *mnt_point;
	if (g_path_is_absolute(mnt_fsname)) {
	    mnt_point = realpath(mnt_fsname, NULL);
	} else {
	    mnt_point = g_strdup(mnt_fsname);
	}
        TRACE("*** is mounted: %s\n", mnt_point);
        
        struct mntent *m;
        //const gchar *mnttab;
        FILE *tab_file;

        // try both /etc/mtab and /proc/mounts 
        //const gchar *mfile[]={"/proc/mounts", "/etc/mtab", NULL};
        const gchar *mfile[]={"/proc/mounts", NULL};
        const gchar **pfile;
        for (pfile=mfile; pfile && *pfile; pfile++){
	    TRACE("isMounted: %s\n", *pfile);
            if((tab_file = fopen (*pfile, "r")) == NULL) {
                continue;
            }
            fclose(tab_file);
            tab_file = setmntent (*pfile, "r");

            if(!tab_file) {
                perror ("setmntent:");
                g_free(mnt_point);
                return FALSE;
            }
            struct mntent mntbuf;
            gchar buf[2048]; 
            while ((m = getmntent_r (tab_file, &mntbuf, buf, 2048)) != NULL) {	
                TRACE(".isMounted():%s:  %s  or  %s\n", mnt_point, m->mnt_dir, m->mnt_fsname);
                if((mnt_point && strcmp (m->mnt_dir, mnt_point) == 0) || 
                   (mnt_point && strcmp (m->mnt_fsname, mnt_point) == 0)) {
                   //(mnt_fsname && strcmp (m->mnt_fsname, mnt_fsname) == 0)) {
                    TRACE("..isMounted(): GOTCHA  mnt_dir=%s  mnt_fsname=%s mnt_point=%s\n", m->mnt_dir, m->mnt_fsname, mnt_point);
                    endmntent (tab_file);
                    g_free(mnt_point);
                    return TRUE;
                }
            }
            endmntent (tab_file);
        }
        g_free(mnt_point);
        return FALSE;
    }

    static gchar *
    mountTarget (const gchar *label) {
        if (!label){
            ERROR("fstab/view.hh::mountTarget() label is null\n");
            return NULL;
        }
        struct mntent *mnt_struct;
        FILE *fstab_fd;
        gchar *result = NULL;
        if((fstab_fd = setmntent ("/etc/fstab", "r")) == NULL) {
            ERROR ("fstab/view.hh::mountTarget(): Unable to open %s\n", "/etc/fstab");
            return result;
        }

        struct mntent mntbuf;
        gchar buf[2048]; 
        while ((mnt_struct = getmntent_r (fstab_fd, &mntbuf, buf, 2048)) != NULL) {
            if(strstr (mnt_struct->mnt_type, MNTTYPE_SWAP))
                continue;
            if(!g_file_test (mnt_struct->mnt_dir, G_FILE_TEST_IS_DIR))
                continue;

            TRACE("mountTarget():%s --->  %s   or   %s\n", 
                    label, mnt_struct->mnt_dir, mnt_struct->mnt_fsname);

            if(strcmp (label, mnt_struct->mnt_fsname)==0) {
                TRACE("mountTarget():%s ---> %d %s\n", 
                        mnt_struct->mnt_fsname, result, mnt_struct->mnt_type);
		result = g_strdup(mnt_struct->mnt_dir);
                break;
            }
        }

        (void)endmntent (fstab_fd);
        return result;
    }

    static gboolean
    isInFstab (const gchar *path) {
        if (!path){
            ERROR("fstab/view.hh::isInFstab() path is null\n");
            return FALSE;
        }
        struct mntent *mnt_struct;
        FILE *fstab_fd;
        gboolean result = FALSE;
        if((fstab_fd = setmntent ("/etc/fstab", "r")) == NULL) {
            ERROR ("fstab/view.hh::isInFstab(): Unable to open %s\n", "/etc/fstab");
            return result;
        }

        struct mntent mntbuf;
        gchar buf[2048]; 
        while ((mnt_struct = getmntent_r (fstab_fd, &mntbuf, buf, 2048)) != NULL) {
            if(strstr (mnt_struct->mnt_type, MNTTYPE_SWAP))
                continue;
            if(!g_file_test (mnt_struct->mnt_dir, G_FILE_TEST_IS_DIR))
                continue;

            TRACE("isInFstab():%s --->  %s   or   %s\n", 
                    path, mnt_struct->mnt_dir, mnt_struct->mnt_fsname);

            if(strcmp (path, mnt_struct->mnt_dir) == 0 || 
                    strcmp (path, mnt_struct->mnt_fsname) == 0) {
                if (getFstabType (mnt_struct->mnt_type)) result = TRUE;
                TRACE("isInFstab():%s ---> %d %s\n", 
                        mnt_struct->mnt_fsname, result, mnt_struct->mnt_type);
                break;
            }
        }

        (void)endmntent (fstab_fd);
        return result;
    }

#else
    static guint
    getMntType (const gchar *path) {return 0;}
   static gchar *
    getMntDir (const gchar * mnt_fsname) {return NULL;}
    static gboolean
    isMounted (const gchar *mnt_fsname){return FALSE;}  
    static gchar *
    mountTarget (const gchar *label) {return NULL;}    
    static gboolean
    isInFstab (const gchar *path) {return FALSE;}
#endif

    static gboolean
    sudoMount(const gchar *mnt){
       // Sudo check...
        // 
        // BSD sudo not necessary if vfs.usermount?
        gboolean useSudo = TRUE;
        // not for root
        if(!getuid ()) useSudo = FALSE;
        // not for general user mounts
        if(isInFstab(mnt)){
            // Is it user type? No need for sudo then.
            if (IS_USER_TYPE(getMntType(mnt))) useSudo = FALSE;
        } 
        // sudo requested but not installed, barf.
        
        if (useSudo) {
            auto p = g_find_program_in_path ("sudo");
            if(p == NULL) {
                // barf!
                auto text = g_strdup_printf("%s is not installed in the path.", "sudo");
                auto markup = g_strdup_printf("\n\n<span size =\"larger\" color=\"red\"%s</span>\n\n",
                       text); 
                Gtk<Type>::quickHelp(GTK_WINDOW(mainWindow), markup, "face-sick-symbolic");
                ERROR("fstab/view.hh::%s\n", text);
                g_free(text);
                g_free(markup);
                return FALSE;

            } else g_free (p);
        }
	return useSudo;
    }

    // mount from fstab data or directly
    static gboolean
    mountPath (View<Type> *view, const gchar *path, const gchar *mountPoint) 
    {
        TRACE("FstabView<Type>::mountPath(%s, %s)\n", path, mountPoint);
	if (!g_path_is_absolute(path)){
	    ERROR("fstab/view.hh::mountPath: %s is not absolute.\n", path);
	    return FALSE;
	}
        const gchar *umount = "umount";
        const gchar *mount = "mount";
        gboolean useSudo = sudoMount(path);
 
	gboolean mounted = isMounted(path);

	const gchar *arg[10];
	gint i=0;


	if (useSudo) {
	    arg[i++] = "sudo";
	    arg[i++] = "-A";
	}
	arg[i++] = (mounted)?umount:mount;
	arg[i++] = path;
	arg[i++] = mountPoint;
	arg[i++] = NULL;

	auto message = g_strdup_printf((mounted)?
		    _("Unmounting %s"):_("Mounting %s"), path);
/*
 * no good...
        const gchar *sudo = "";
	if (useSudo) sudo = "sudo -A";
        gchar *command = g_strdup(sudo);
        gchar *g = g_strconcat(command, " ", (mounted)?umount:mount, " ", path, 
                " ", mountPoint?mountPoint:"" , NULL);
        g_free(command);
        command=g;
        if (mountPoint){
            g= g_strconcat(command, " && ", sudo, " touch ", mountPoint, NULL);
        } else {
            g= g_strconcat(command, " && ", sudo, " touch ", path, NULL);
        }
        g_free(command);
        command=g;
        CommandResponse<Type>::dialog(message, "system-run", command);
        g_free(command);
 */       
        CommandResponse<Type>::dialog(message, "system-run", arg);
        g_free(message);


        return TRUE;
    }

private:

    static gboolean done_f(void *data) {
        auto view = (View<Type> *)data;
	if (!View<Type>::validBaseView(view)) {
            ERROR("fstab/view.hh::done_f(): invalid view: %p\n", view);
            return FALSE;
        }
        auto page = view->page();
        auto viewPath = page->workDir();  
        TRACE("FstabView::done_f(): view->loadModel(%s)\n", viewPath);
        auto viewType = view->viewType();
        switch (view->viewType()){
            case (LOCALVIEW_TYPE):
                view->loadModel(viewPath);
                break;
            case (FSTAB_TYPE):
                view->loadModel("xffm:fstab");
                break;
        }
        return FALSE;
    }

    static void
    fork_finished_function (void *data) {
        g_timeout_add(5, done_f, data);
    }
    static void
    run_operate_stdout (void *data, void *stream, int childFD){
	auto view = (View<Type> *)data;
	Run<Type>::run_operate_stdout((void *)view->page()->output(), stream, childFD);
    }
    static void
    run_operate_stderr (void *data, void *stream, int childFD){
	auto view = (View<Type> *)data;
	Run<Type>::run_operate_stderr((void *)view->page()->output(), stream, childFD);
    }

private:
    
    static int
    getFstabType (const char *type) {
        if(strstr (type, MNTTYPE_NFS)) {
            TRACE ("    got %s type for fstab item %s\n", MNTTYPE_NFS, type);
            return __NFS_TYPE;
        } else if(strstr (type, MNTTYPE_SMBFS)) {
            TRACE ("    got %s type for fstab item %s\n", MNTTYPE_SMBFS, type);
            return __SMB_TYPE;
        } else if(strstr (type, MNTTYPE_PROCFS)) {
            TRACE ("    got %s type for fstab item %s\n", MNTTYPE_PROCFS, type);
            return __PROC_TYPE;
        } else if(strstr (type, MNTTYPE_DEV)) {
            TRACE ("    got %s type for fstab item %s\n", MNTTYPE_DEV, type);
            return __PROC_TYPE;
        } else if(strstr (type, MNTTYPE_SHM)) {
            TRACE ("    got %s type for fstab item %s\n", MNTTYPE_SHM, type);
            return __PROC_TYPE;
        } else if(strstr (type, MNTTYPE_CDFS)) {
            TRACE ("    got %s type for fstab item%s\n", MNTTYPE_CDFS, type);
            return __CDFS_TYPE;
        } else if(strstr (type, MNTTYPE_CDFS_AIX))
            return __CDFS_TYPE;
        else if(strstr (type, MNTTYPE_CACHEFS))
            return __PROC_TYPE;
        else if(strstr (type, MNTTYPE_HSFS))
            return __PROC_TYPE;
        else if(strstr (type, MNTTYPE_KERNFS))
            return __PROC_TYPE;
        else if(strstr (type, MNTTYPE_MFS))
            return __PROC_TYPE;
        else if(strstr (type, MNTTYPE_MFS))
            return __NFS_TYPE;

        return 1;
    }

};
#endif
}
#endif
#endif
