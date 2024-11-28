#ifndef XF_FSTABUTIL_HH 
#define XF_FSTABUTIL_HH

// These are the strings returned from getMntType()
#define MNTTYPE_PROCFS        "proc"
#define MNTTYPE_SMBFS        "smbfs"
#define MNTTYPE_DEV        "devpts"
#define MNTTYPE_SHM        "tmpfs"
#define        MNTTYPE_CDFS        "iso9660"
#define MNTTYPE_SWAP        "swap"
#define MNTTYPE_NFS        "nfs"

#define MNTTYPE_CACHEFS        "cachefs"       // proc
#define MNTTYPE_HSFS        "hsfs"  // proc

#define MNTTYPE_CODAFS        "coda"  //nfs
#define MNTTYPE_CDFS_AIX "cdrfs"        /* AIX */
#define MNTTYPE_KERNFS        "kernfs"        // proc
#define MNTTYPE_MFS        "mfs"   //proc

# include <fstab.h>
# ifdef HAVE_SYS_UCRED_H
#  include <sys/ucred.h>
# endif
# ifdef HAVE_SYS_PARAM_H
#  include <sys/param.h>
# endif
# ifdef HAVE_SYS_MOUNT_H
#  include <sys/mount.h>
# endif
# ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif
# ifdef BSD_FOUND
#  ifdef HAVE_SYS_SYSCTL_H
#   include <sys/sysctl.h>
#  endif
# endif
# ifdef HAVE_MNTENT_H
#  include <mntent.h>
# endif


namespace xf {

class FstabUtil {

  public:

    static gchar *
    mountSrc (const char *mountTarget) {
        if (!mountTarget){
            ERROR("mountSrc() mountTarget is null\n");
            return NULL;
        }
        struct mntent *mnt_struct;
        FILE *fstab_fd;
        gchar *result = NULL;
        if((fstab_fd = setmntent ("/etc/fstab", "r")) == NULL) {
            ERROR ("mountSrc(): Unable to open %s\n", "/etc/fstab");
            return result;
        }

        struct mntent mntbuf;
        gchar buf[2048]; 
        while ((mnt_struct = getmntent_r (fstab_fd, &mntbuf, buf, 2048)) != NULL) {
            if(strstr (mnt_struct->mnt_type, MNTTYPE_SWAP))
                continue;
            if(!g_file_test (mnt_struct->mnt_dir, G_FILE_TEST_IS_DIR))
                continue;
            if (strcmp(mountTarget, mnt_struct->mnt_dir) != 0) 
                continue;
            TRACE("mountTarget():mnt_dir = %s; mnt_fsname =  %s \n", 
                    mnt_struct->mnt_dir, mnt_struct->mnt_fsname);
            result = g_strdup(mnt_struct->mnt_fsname);

        }

        (void)endmntent (fstab_fd);
        return result;
    }

    static gchar *
    mountTarget (const char *label) {
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

            if(strcmp (label, mnt_struct->mnt_dir)==0) {
                TRACE("mountTarget(): gotcha mnt_dir %s ---> %s\n", 
                        label, mnt_struct->mnt_dir);
                result = g_strdup(mnt_struct->mnt_dir);
                break;
            }
            if(strcmp (label, mnt_struct->mnt_fsname)==0) {
                TRACE("mountTarget(): gotcha fsname %s ---> %s\n", 
                        label, mnt_struct->mnt_fsname);
                result = g_strdup(mnt_struct->mnt_dir);
                break;
            }
        }

        (void)endmntent (fstab_fd);
        return result;
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

    static void setMountableIcon(GFileInfo *info, const char *path){
      int size = Settings::getInteger("xfterm", "iconsize");
      const char *iconPath;
      if (g_file_test(path, G_FILE_TEST_IS_DIR)) iconPath = Texture<bool>::findIconPath("folder-remote");
      else  iconPath = Texture<bool>::findIconPath("drive-harddisk");
      
      const char *ball = "emblem-redball";
      if (isMounted(path)) ball = "emblem-greenball";
      auto paintable = Texture<bool>::addEmblem(iconPath, ball, size, size);
      g_file_info_set_attribute_object(info, "xffm:paintable", G_OBJECT(paintable));  
      return;  
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
        TRACE("test for mount status: %s\n", mnt_point);
        
        struct mntent *m;
        //const gchar *mnttab;
        FILE *tab_file;

        // try both /etc/mtab and /proc/mounts 
        const gchar *mfile[]={"/proc/mounts", "/etc/mtab", NULL};
        //const gchar *mfile[]={"/proc/mounts", NULL};
        const gchar **pfile;
        for (pfile=mfile; pfile && *pfile; pfile++){
            TRACE("From /proc/mounts and /etc/mtab: %s\n", *pfile);
            if((tab_file = fopen (*pfile, "r")) == NULL) {
                DBG("%s: %s\n", strerror(ENOENT), *pfile);
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
                    TRACE("%s ..isMounted(): mnt_dir=%s  mnt_fsname=%s mnt_point=%s\n", 
                            *pfile, m->mnt_dir, m->mnt_fsname, mnt_point);
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
