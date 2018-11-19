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

    // XXX this is Linux Version. FreeBSD differs (see fstab module)
namespace xf {
template <class Type>
class Fstab {
public:
    
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
	if (!BaseView<Type>::validBaseView(baseView)) return FALSE;
        auto page = baseView->page();
        auto viewPath = page->workDir();            
        baseView->loadModel(viewPath);
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

            if (strcmp(path, mnt_struct->mnt_dir)) continue;
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
