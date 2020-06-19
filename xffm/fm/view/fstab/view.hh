#ifndef XF_FSTAB_HH 
#define XF_FSTAB_HH

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

private:
    static GSList *dfInfo(void){
        GSList *list=NULL;
        gchar *df = g_find_program_in_path("df");
        if (!df) {
            DBG("\"df\" command not in path!");
            return NULL;
        }
        gchar *result = NULL; 
        gchar *command = g_strdup_printf("%s -h -l", df);
        result = Util<Type>::pipeCommandFull(command);
        if (!result) return NULL;
        g_free(command);
        g_free(df);

        auto lines = g_strsplit(result, "\n", -1);
        for (auto l=lines+1; l && *l; l++){
            if (strlen(*l))list = g_slist_prepend(list, g_strdup(*l)); 
        }
        if (list) list = g_slist_reverse(list);
        
        g_free(result);
        g_strfreev(lines);
        return list;
    }

    static gchar **compactStrV(const gchar *line)
    {
        if (!line) return NULL;
        auto strv = g_strsplit(line," ", -1);
        gchar **out = NULL;
        gint count = 0;
        for (auto w=strv; w && *w; w++){
            if (strlen(*w) > 0) count++;
        }
        DBG("count %d\n", count);
        if (!count) return NULL;
        out = (gchar **)calloc(count+1, sizeof(gchar *));
        auto v = out;
        for (auto w=strv; w && *w; w++){
            if (strlen(*w) > 0){
               *v = g_strdup(*w);
               v++;
            }
        }
        return out;
    }

    static void
    addDfPartition(GtkTreeModel *treeModel, const gchar *line){

        auto r = compactStrV(line);



        for (auto w=r; w && *w; w++){
            fprintf(stderr,"\"%s\" ",*w);
        }

        fprintf(stderr,"\n");
        addPartition2Model(treeModel, (const gchar **)r);

        g_strfreev(r);
    }

    static void
    addPartitionItems(GtkTreeModel *treeModel){
        auto list = dfInfo();
        for (auto l=list; l&&l->data; l=l->next){
            DBG("adding \"%s\"\n", (const gchar *)l->data);
            
            addDfPartition(treeModel, (const gchar *)l->data);
            g_free(l->data);
        }
        g_slist_free(list);
    }

    static void
    addPartition2Model(GtkTreeModel *treeModel, const gchar **strv){
        auto path = strv[0];
        if (!path){
            ERROR("fstab/view.hh::addPartition: path cannot be null\n");
            return;
        }
        auto mntdir = strv[5];
         GtkTreeIter iter;

        gboolean mounted = isMounted(path);
        gchar *text;
         text = g_strdup_printf("** %s (%s): %s\n%s %s\n%s %s\n%s %s",
                        strv[5], path, strv[1],  // size, mountpoint
                        _("Used"), strv[2],
                        _("Available"), strv[3],
                        _("Usage"), strv[4]);

        auto label = g_strdup_printf("%s", strv[5]);
        auto utf_name = util_c::utf_string(label);
        g_free(label);

        auto icon_name = (mounted)?"drive-harddisk/NW/greenball/3.0/180":
            "drive-harddisk/NW/grayball/3.0/180";
        auto highlight_name = "drive-harddisk/NW/blueball/3.0/225";
        auto treeViewPixbuf = Pixbuf<Type>::getPixbuf(icon_name,  -24);
        auto normal_pixbuf = pixbuf_c::getPixbuf(icon_name,  -48);
        auto highlight_pixbuf = pixbuf_c::getPixbuf(highlight_name,  -48);   
        //auto uuid = partition2uuid(path);
        auto id = partition2Id(path);
        gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
        gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
                DISPLAY_NAME, utf_name, // path-basename or label
                ICON_NAME, icon_name,
                PATH, strv[5], //path, // absolute
                TREEVIEW_PIXBUF, treeViewPixbuf, 
                DISPLAY_PIXBUF, normal_pixbuf,
                NORMAL_PIXBUF, normal_pixbuf,
                HIGHLIGHT_PIXBUF, highlight_pixbuf,
                TOOLTIP_TEXT,text,
               // DISK_ID, id,
                -1);
        g_free(utf_name);
        // icon_name is constant
        // pixbufs belong to pixbuf hash
        g_free(text);

    }


public:
    static gboolean
    isSelectable(GtkTreeModel *treeModel, GtkTreeIter *iter){
        TRACE("fstab isSelectable()...\n");
        return TRUE;
    }


public:
    // addPartition 2 b deprecated
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
        auto treeViewPixbuf = Pixbuf<Type>::getPixbuf(icon_name,  -24);
        auto normal_pixbuf = pixbuf_c::getPixbuf(icon_name,  -48);
        auto highlight_pixbuf = pixbuf_c::getPixbuf(highlight_name,  -48);   
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
