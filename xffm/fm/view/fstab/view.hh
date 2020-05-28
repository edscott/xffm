#ifndef XF_FSTAB_HH 
#define XF_FSTAB_HH

// These are the strings returned from getMntType()
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


# include <fstab.h>
# include <sys/ucred.h>


# ifdef HAVE_SYS_PARAM_H
#  include <sys/param.h>
# endif
# ifdef HAVE_SYS_MOUNT_H
#  include <sys/mount.h>
# endif
# ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif
# ifdef HAVE_SYS_SYSCTL_H
#  include <sys/sysctl.h>
# endif
# ifdef HAVE_MNTENT_H
#  include <mntent.h>
# endif



#include "popup.hh"
#include "monitor.hh"
namespace xf {
template <class Type> class FstabPopUp;


//static pthread_mutex_t mntmutex = PTHREAD_MUTEX_INITIALIZER;
//static GMutex *infomutex=NULL;


template <class Type>
class FstabView: public FstabPopUp<Type> {
    using pixbuf_c = Pixbuf<double>;
    using util_c = Util<double>;
public:
    static gboolean
    isSelectable(GtkTreeModel *treeModel, GtkTreeIter *iter){
        TRACE("fstab isSelectable()...\n");
        return TRUE;
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
	TRACE("fstab/addPartition()...\n");

        gboolean mounted = isMounted(path);
        gchar *text;
        auto fstype = fsType(path);        
        gchar *fileInfo = util_c::fileInfo(path);
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

#ifdef BSD_FOUND
#include "view-BSD.hh"
#else
#include "view-Linux.hh"
#endif
    
};
}
#endif
