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

#ifdef THIS_IS_BSD
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/param.h>
#include <sys/mount.h>
#endif
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


# include <mntent.h>

#include "fstabpopup.hh"
#include "fstabmonitor.hh"


    // XXX this is Linux Version. FreeBSD differs (see fstab module)
namespace xf {
template <class Type>
class Fstab: public FstabPopUp<Type> {
    using pixbuf_c = Pixbuf<double>;
    using util_c = Util<double>;
public:

    static FstabMonitor<Type> *
    loadModel (BaseView<Type> *baseView)
    {
		
        auto iconView = baseView->iconView();
        auto treeModel = gtk_icon_view_get_model (iconView);
	TRACE("mk_tree_model:: model = %p\n", treeModel);
        while (gtk_events_pending()) gtk_main_iteration();
 	GtkTreeIter iter;
	if (gtk_tree_model_get_iter_first (treeModel, &iter)){
	    while (gtk_list_store_remove (GTK_LIST_STORE(treeModel),&iter));
	}
        // Disable DnD
        //gtk_icon_view_unset_model_drag_source (iconView);
        //gtk_icon_view_unset_model_drag_dest (iconView);
        gtk_icon_view_set_selection_mode (iconView,GTK_SELECTION_SINGLE); 

	RootView<Type>::addXffmItem(treeModel);
	addNFSItem(treeModel);
	addEcryptFSItem(treeModel);
	addSSHItem(treeModel);
	addCIFSItem(treeModel);
        addPartitionItems(treeModel);

        FstabMonitor<Type> *p = new(FstabMonitor<Type>)(treeModel, baseView);
        p->start_monitor(treeModel, "/dev/disk/by-partuuid");
        return p;
    }


    static gchar *
    e2Label(const gchar *partition){
        const gchar *command = "ls -l /dev/disk/by-label";
	FILE *pipe = popen (command, "r");
	if(pipe == NULL) {
	    ERROR("Cannot pipe from %s\n", command);
	    return NULL;
	}
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
	return label;

    }


    static gchar *
    getMntDir (const gchar * mnt_fsname) {
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
    partition2uuid(const gchar *partition){
        const gchar *command = "ls -l /dev/disk/by-partuuid";
	FILE *pipe = popen (command, "r");
	if(pipe == NULL) {
	    ERROR("Cannot pipe from %s\n", command);
	    return NULL;
	}
        gchar line[256];
        memset(line, 0, 256);
        gchar *uuid = NULL;
	while (fgets (line, 255, pipe) && !feof(pipe)) {
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
        //if (strchr(uuid, '\n')) *strchr(uuid, '\n') = 0;
	return uuid;
    }

    static void
    addPartition(GtkTreeModel *treeModel, const gchar *path){
 	GtkTreeIter iter;
        gchar *basename = g_path_get_basename(path);
        gchar *mntDir = getMntDir(path);
        auto label = e2Label(basename);
        auto name = (label)?label:basename;
        auto fullName = (mntDir)?g_strdup_printf("%s\n(%s)", name, mntDir): g_strdup(name);
        auto utf_name = util_c::utf_string(fullName);
        g_free(fullName);
        gboolean mounted = isMounted(path);
        auto icon_name = (mounted)?"drive-harddisk/NE/greenball/2.0/225":
            "drive-harddisk/NE/grayball/2.0/225";
        auto highlight_name = "drive-harddisk/NW/edit-select-symbolic/2.0/225";
        auto normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  GTK_ICON_SIZE_DIALOG);
        auto highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  GTK_ICON_SIZE_DIALOG);   
        auto uuid = partition2uuid(basename);
        gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
        gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
                DISPLAY_NAME, utf_name,
                ACTUAL_NAME, uuid,
                ICON_NAME, icon_name,
                PATH, path,
                DISPLAY_PIXBUF, normal_pixbuf,
                NORMAL_PIXBUF, normal_pixbuf,
                HIGHLIGHT_PIXBUF, highlight_pixbuf,
                TOOLTIP_TEXT,"FIXME: UUID or partition type...",

                -1);
        g_free(basename);
        g_free(utf_name);
        g_free(uuid);
    
    }

    static void // Linux
    addPartitionItems (GtkTreeModel *treeModel) {
        FILE *partitions = fopen ("/proc/partitions", "r");
        if(!partitions) return;

        gchar line[1024];
        memset (line, 0, 1024);
        while(fgets (line, 1023, partitions) && !feof (partitions)) {
            gchar *path = getPartitionPath(line);
            if (!path) continue;
            addPartition(treeModel, path);
            g_free(path);
            memset (line, 0, 1024);
        }
        fclose (partitions);
        return;
    }

    
    static void
    addNFSItem(GtkTreeModel *treeModel){
 	GtkTreeIter iter;
	auto name = "xffm:nfs";
	auto utf_name = util_c::utf_string(_("NFS Network Volume"));
	auto icon_name = "video-display/SE/emblem-nfs/2.0/225";
	auto highlight_name = "video-display/SE/emblem-nfs/2.0/225/NE/document-open/2.0/225";
	auto normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  GTK_ICON_SIZE_DIALOG);
	auto highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  GTK_ICON_SIZE_DIALOG);   
	gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
	gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
		DISPLAY_NAME, utf_name,
		ACTUAL_NAME, name,
		ICON_NAME, icon_name,
                PATH, name,
		DISPLAY_PIXBUF, normal_pixbuf,
		NORMAL_PIXBUF, normal_pixbuf,
		HIGHLIGHT_PIXBUF, highlight_pixbuf,
		TOOLTIP_TEXT,_("xffm:nfs"),

		-1);
	g_free(utf_name);
    }

    static void
    addSSHItem(GtkTreeModel *treeModel){
 	GtkTreeIter iter;
	auto name = "xffm:nfs";
	auto utf_name = util_c::utf_string(_("SFTP (via SSH)"));
	auto icon_name = "video-display/SE/emblem-ssh/2.0/225";
	auto highlight_name = "video-display/SE/emblem-ssh/2.0/225/NE/document-open/2.0/225";
	auto normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  GTK_ICON_SIZE_DIALOG);
	auto highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  GTK_ICON_SIZE_DIALOG);   
	gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
	gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
		DISPLAY_NAME, utf_name,
		ACTUAL_NAME, name,
		ICON_NAME, icon_name,
                PATH, name,
		DISPLAY_PIXBUF, normal_pixbuf,
		NORMAL_PIXBUF, normal_pixbuf,
		HIGHLIGHT_PIXBUF, highlight_pixbuf,
		TOOLTIP_TEXT,_("xffm:sshfs"),

		-1);
	g_free(utf_name);
    }

    static void
    addEcryptFSItem(GtkTreeModel *treeModel){
 	GtkTreeIter iter;
	auto name = "xffm:nfs";
	auto utf_name = util_c::utf_string(_("eCryptfs Volume"));
	auto icon_name = "video-display/SE/emblem-lock/2.0/225";
	auto highlight_name = "video-display/SE/emblem-lock/2.0/225/NE/document-open/2.0/225";
	auto normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  GTK_ICON_SIZE_DIALOG);
	auto highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  GTK_ICON_SIZE_DIALOG);   
	gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
	gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
		DISPLAY_NAME, utf_name,
		ACTUAL_NAME, name,
		ICON_NAME, icon_name,
                PATH, name,
		DISPLAY_PIXBUF, normal_pixbuf,
		NORMAL_PIXBUF, normal_pixbuf,
		HIGHLIGHT_PIXBUF, highlight_pixbuf,
		TOOLTIP_TEXT,_("xffm:ecryptfs"),

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
	auto normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  GTK_ICON_SIZE_DIALOG);
	auto highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  GTK_ICON_SIZE_DIALOG);   
	gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
	gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
		DISPLAY_NAME, utf_name,
		ACTUAL_NAME, name,
		ICON_NAME, icon_name,
                PATH, name,
		DISPLAY_PIXBUF, normal_pixbuf,
		NORMAL_PIXBUF, normal_pixbuf,
		HIGHLIGHT_PIXBUF, highlight_pixbuf,
		TOOLTIP_TEXT,_("xffm:cifs"),

		-1);
	g_free(utf_name);
    }
   
   
    static gboolean
    isMounted (const gchar *mnt_fsname) {
        if(!mnt_fsname) {
            DBG ("isMounted() mnt_point != NULL not met!\n");
            return FALSE;
        }
        gchar *mnt_point = realpath(mnt_fsname, NULL);
        
        struct mntent *m;
        //const gchar *mnttab;
        FILE *tab_file;

        // try both /etc/mtab and /proc/mounts 
        const gchar *mfile[]={"/proc/mounts", "/etc/mtab", NULL};
        const gchar **pfile;
        for (pfile=mfile; pfile && *pfile; pfile++){
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
                TRACE("isMounted():%s:  %s  or  %s\n", mnt_point, m->mnt_dir, m->mnt_fsname);
                if((mnt_point && strcmp (m->mnt_dir, mnt_point) == 0) || 
                   (mnt_fsname && strcmp (m->mnt_fsname, mnt_fsname) == 0)) {
                    TRACE("isMounted(): GOTCHA  %s  %s:%s\n", m->mnt_dir, m->mnt_fsname, mnt_point);
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
            DBG ("isInFstab(): Unable to open %s\n", "/etc/fstab");
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

    // mount from fstab data
    static gboolean
    mount (BaseView<Type> *baseView, const gchar *path) {
        TRACE("enter Fstab<Type>::mount(%s)\n", path);
        const gchar *argument[10];
        const gchar **ap;
        const gchar *umount = "umount";
        const gchar *mount = "mount";


        ap = argument;

        // Sudo check...
        // 
        // BSD sudo not necessary if vfs.usermount?
        gboolean useSudo = TRUE;
        // not for root
        if(!getuid ()) useSudo = FALSE;
        // not for general user mounts
        if(isInFstab(path)){
            // Is it user type? No need for sudo then.
            if (IS_USER_TYPE(getMntType(path))) useSudo = FALSE;
        } else {
            // barf: function incorrectly called with non fstab item.
            ERROR("%s not in /etc/fstab\n", path);
            return FALSE;
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

#if 1
	// Simple fstab item mount...
	const gchar *arg[5];
	gint i=0;
	if (useSudo) {
	    arg[i++] = "sudo";
	    arg[i++] = "-A";
	}
	arg[i++] = (isMounted(path))?umount:mount;
	arg[i++] = path;
	arg[i++] = NULL;
	auto voidP = (void **)calloc (2, sizeof(void *));
	if (!voidP){
	    ERROR("mount(%s): calloc: %s\n", path,strerror(errno));
	    exit(1);
	}
	pid_t controller = Run<Type>::thread_run(
		(void *)baseView, // data to fork_finished_function
		arg,
		Run<Type>::run_operate_stdout,
		Run<Type>::run_operate_stderr,
		fork_finished_function);

	    

#else

        gchar *commandFmt;

        if(useSudo) commandFmt = g_strdup("sudo -A");
        else commandFmt = g_strdup("");
        gchar *g = g_strconcat(commandFmt, " ", (isMounted(path))?umount:mount, NULL);
        g_free(commandFmt);
        commandFmt = g;
	gchar *command = Mime<Type>::mkCommandLine(commandFmt, path);
	// get baseView
	auto page = baseView->page();
	page->command(command);
        g_free(commandFmt);
        g_free(command);
#endif

        TRACE ("fstab_mount %s done \n",path);
        return TRUE;
    }

private:

    static gboolean done_f(void *data) {
        auto baseView = (BaseView<Type> *)data;
	if (!BaseView<Type>::validBaseView(baseView)) {
            ERROR("Fstab::done_f(): invalid baseView: %p\n", baseView);
            return FALSE;
        }
        auto page = baseView->page();
        auto viewPath = page->workDir();  
        ERROR("Fstab::done_f(): baseView->loadModel(%s)\n", viewPath);
        auto viewType = baseView->viewType();
        switch (baseView->viewType()){
            case (LOCALVIEW_TYPE):
                baseView->loadModel(viewPath);
                break;
            case (FSTAB_TYPE):
                baseView->loadModel("xffm:fstab");
                break;
        }
        return FALSE;
    }

    static void
    fork_finished_function (void *data) {
        g_timeout_add(5, done_f, data);
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

};
}
#endif
