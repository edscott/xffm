#ifndef XF_FSTAB_HH 
#define XF_FSTAB_HH
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

#ifdef HAVE_MNTENT_H
// Linux
# include <mntent.h>
#endif

// BSD
#ifdef HAVE_SYS_MOUNT_H
# include <sys/mount.h>
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_sys_sysctl_H
# include <sys/sysctl.h>
#endif
#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif


#define PARTUUID        ACTUAL_NAME
#define DISK_LABEL      DISPLAY_NAME
#define FSTYPE          MIMETYPE


    // XXX this is Linux Version. FreeBSD differs (see fstab module)
namespace xf {
template <class Type> class FstabPopUp;
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
        FstabMonitor<Type> *p = new(FstabMonitor<Type>)(treeModel, view);
        p->start_monitor(treeModel, "/dev/disk/by-partuuid");
        return p;
    }

    static void
    removeAllItems(GtkTreeModel *treeModel){
 	GtkTreeIter iter;
	if (gtk_tree_model_get_iter_first (treeModel, &iter)){
	    while (gtk_list_store_remove (GTK_LIST_STORE(treeModel),&iter));
	}
    }

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
	auto highlight_name = "drive-multidisk/SE/document-open/2.0/225";
        auto treeViewPixbuf = Pixbuf<Type>::get_pixbuf(icon_name,  -24);
	auto normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  -48);
	auto highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  -48);   
	gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
	gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
		DISPLAY_NAME, utf_name,
		ACTUAL_NAME, name,
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
	    ERROR("Cannot pipe from %s\n", command);
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
    id2Partition(const gchar *id){ // disk partition only
        gchar *baseId = g_path_get_basename(id);
        const gchar *command = "ls -l /dev/disk/by-id";
	FILE *pipe = popen (command, "r");
	if(pipe == NULL) {
	    ERROR("Cannot pipe from %s\n", command);
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
	    ERROR("Cannot pipe from %s\n", command);
	    return NULL;
	}
        auto partition = g_path_get_basename(partitionPath); 
        gchar line[256];
        memset(line, 0, 256);
        gchar *label = NULL;
	while (fgets (line, 255, pipe) && !feof(pipe)) {
            if (strstr(line, "->") && strstr(line, partition)) {
                *(strstr(line, "->")) = 0;
                g_strstrip(line);
                if (strrchr(line, ' ')) label = g_strdup(strrchr(line, ' ')+1);
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
        TRACE ("partitions add: %s\n", p);
        if(!strlen (p)) {
            g_free(p);
            return NULL;
        }
        if (strncmp(p, "sd", 2) == 0 || strncmp(p, "hd", 2)==0){
            if (p[3] < '0' || p[3] >'9') return NULL;
            gchar *path = g_strdup_printf ("/dev/%s", p);
            g_free(p);
            return path;
        }
        g_free(p);
        return NULL;
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
	    ERROR("Cannot pipe from %s\n", command);
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
    addPartition(GtkTreeModel *treeModel, const gchar *path, const gchar *fstype){
        if (!path){
            ERROR("addPartition: path cannot be null\n");
            return;
        }
 	GtkTreeIter iter;
        gchar *basename = g_path_get_basename(path);
        gchar *mntDir = getMntDir(path);
        auto label = e2Label(basename);
        if (label){
           g_free(basename);
        } else {
           label = basename;
        }

        gboolean mounted = isMounted(path);
        gchar *text;
	if (mntDir) { 
	    gchar *mountedOn = (mounted)?
		g_strconcat (_("Mounted on:"), " ", mntDir, " <span color=\"red\">(", fstype, ")</span>", NULL):
		g_strconcat (_("Mount on"), " ", mntDir, " <span color=\"red\">(", fstype, ")</span>", NULL);
	    text = (label)?	g_strdup_printf("%s (%s)\n<span size=\"large\">%s</span>",
			label, path, mountedOn):
		g_strdup_printf("%s\n<span size=\"large\">%s</span>",
			path, mountedOn);
	    g_free(mountedOn);
	} else { // implies not mounted
	    text = (label)?g_strdup_printf("%s (%s)\n<span size=\"large\">%s <span color=\"red\">(%s)</span></span>",
			label, path, _("Not mounted"), fstype):
			g_strdup_printf("%s\n<span size=\"large\">%s <span color=\"red\">(%s)</span></span>",
			path, _("Not mounted"), fstype);
	}

        auto utf_name = util_c::utf_string(label);
        g_free(label);

        auto icon_name = (mounted)?"drive-harddisk/NW/greenball/3.0/180":
            "drive-harddisk/NW/grayball/3.0/180";
        auto highlight_name = "drive-harddisk/NW/edit-select-symbolic/2.0/225";
        auto treeViewPixbuf = Pixbuf<Type>::get_pixbuf(icon_name,  -24);
        auto normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  -48);
        auto highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  -48);   
        auto uuid = partition2uuid(path);
        gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
        gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
                DISK_LABEL, utf_name, // path-basename or label
                FSTYPE, fstype,
                PARTUUID, uuid, // partition-basename
                ICON_NAME, icon_name,
                PATH, path, // absolute
                TREEVIEW_PIXBUF, treeViewPixbuf, 
                DISPLAY_PIXBUF, normal_pixbuf,
                NORMAL_PIXBUF, normal_pixbuf,
                HIGHLIGHT_PIXBUF, highlight_pixbuf,
                TOOLTIP_TEXT,text,
                -1);
        g_free(utf_name);
        // fstype is constant
        g_free(uuid);
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
                ERROR("partition path should be absolute: %s\n", path);
                continue;
            }
            if (!path) continue;
	    gchar *fstype = fsType(path);
            if (fstype) addPartition(treeModel, path, fstype);
            g_free(path);
            g_free(fstype);
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
	    ERROR("Cannot pipe from %s\n", command);
	    return FALSE;
	}

        gchar line[256];
        memset(line, 0, 256);
        gchar *id = NULL;
	while (fgets (line, 255, pipe) && !feof(pipe)) {
            if (strchr(line, '\n')) *strchr(line, '\n') = 0;
            DBG("addPartitionItems: %s\n", line);
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
		DISK_LABEL, utf_name,
		PARTUUID, name,
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
		DISK_LABEL, utf_name,
		PARTUUID, name,
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
		DISK_LABEL, utf_name,
		PARTUUID, name,
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
		DISK_LABEL, utf_name,
		PARTUUID, name,
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
        if (!found) ERROR("getMntType (): %s not found in /etc/fstab\n", path);
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
            ERROR ("isMounted() mnt_point != NULL not met!\n");
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
            ERROR("mountTarget() label is null\n");
            return NULL;
        }
        struct mntent *mnt_struct;
        FILE *fstab_fd;
        gchar *result = NULL;
        if((fstab_fd = setmntent ("/etc/fstab", "r")) == NULL) {
            ERROR ("mountTarget(): Unable to open %s\n", "/etc/fstab");
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
            ERROR("isInFstab() path is null\n");
            return FALSE;
        }
        struct mntent *mnt_struct;
        FILE *fstab_fd;
        gboolean result = FALSE;
        if((fstab_fd = setmntent ("/etc/fstab", "r")) == NULL) {
            ERROR ("isInFstab(): Unable to open %s\n", "/etc/fstab");
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
                ERROR("%s\n", text);
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
	    ERROR("mountPath: %s is not absolute.\n", path);
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
        CommandResponse<Type>::dialog(message, "system-run", arg);

/*
	// open follow dialog for long commands...
	auto command = g_strdup_printf((mounted)?
		    _("Unmounting %s"):_("Mounting %s"), path);
	CommandResponse<Type>::dialog(command,"system-run", Tubo<Type>::getChild(controller) );
	TRACE("%s %s\n", command, mountPoint);
	g_free(command);

        TRACE ("fstab_mount %s done \n",path);*/
        return TRUE;
    }

private:

    static gboolean done_f(void *data) {
        auto view = (View<Type> *)data;
	if (!View<Type>::validBaseView(view)) {
            ERROR("FstabView::done_f(): invalid view: %p\n", view);
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
}
#endif
