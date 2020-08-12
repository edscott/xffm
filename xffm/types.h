#ifndef TYPES_H
# define TYPES_H


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <glob.h>
#include <limits.h>
#include <memory.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>

#include <iostream>

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gmodule.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#ifdef ENABLE_NLS
# include <libintl.h>
# define _(String) dgettext(GETTEXT_PACKAGE,String)
# define N_(String)  String

#else
# warning "Translations not enabled: Gettext not found during configure."
# define _(String) String
# define N_(String) String
# define ngettext(Format1,Format2,N) Format1
# define textdomain(String) 
# define bindtextdomain(Domain,Directory)
#endif

#ifndef DT_BLK
enum {
    DT_BLK=1,
    DT_CHR=2,
    DT_DIR=3,
    DT_FIFO=4,
    DT_LNK=5,
    DT_REG=6,
    DT_SOCK=7
};
#endif

enum
{
  FLAGS,
  TREEVIEW_PIXBUF,
  DISPLAY_PIXBUF,
  NORMAL_PIXBUF,
  HIGHLIGHT_PIXBUF,
  TOOLTIP_PIXBUF,
  DISPLAY_NAME,
  PATH,
  DISK_ID,
  SIZE,
  DATE,
  TOOLTIP_TEXT,
  ICON_NAME,
  MIMETYPE, 
  PREVIEW_PATH,
  PREVIEW_TIME,
  PREVIEW_PIXBUF,
  NUM_COLS
};

enum
{
    ROOTVIEW_TYPE,
    LOCALVIEW_TYPE,
    FSTAB_TYPE,
    NFS_TYPE,
    SSHFS_TYPE,
    EFS_TYPE,
    CIFS_TYPE,
    PKG_TYPE
};
namespace xf {
#ifdef XFFM_CC
template <class Type> class fmDialog;
template <class Type> class BaseView;
#endif
template <class Type> class Notebook;
}

//#define HIGHLIGHT_OPEN_EMBLEM "NE/document-open-symbolic/3.0/220"
//#define HIGHLIGHT_EXEC_EMBLEM "NE/run/2.5/220"

#define HIGHLIGHT_EMBLEM "NE/document-open/2.5/220"
#define HIGHLIGHT_TEXT "NE/accessories-text-editor/2.5/220"
#define HIGHLIGHT_APP "NE/application-x-executable/2.5/220"
#define GO_UP "go-up"
#define HIGHLIGHT_UP "go-up/NW/go-up-symbolic/2.0/225"
//#define HIGHLIGHT_UP_EMBLEM "NW/go-up-symbolic/2.0/225"

#define DRAG_CALLBACK(X)  G_CALLBACK((void (*)(GtkWidget *,GdkDragContext *,gpointer)) X)


#define EVENT_CALLBACK(X)  G_CALLBACK((gint (*)(GtkWidget *,GdkEvent *,gpointer)) X)
#define BUTTON_EVENT_CALLBACK(X)  G_CALLBACK((gint (*)(GtkWidget *,GdkEventButton *,gpointer)) X)
#define KEY_EVENT_CALLBACK(X)  G_CALLBACK((gint (*)(GtkWidget *,GdkEventKey *,gpointer)) X)
#define ICONVIEW_CALLBACK(X)  G_CALLBACK((void (*)(GtkIconView *,GtkTreePath *,gpointer)) X)
#define SELECTION_RECEIVED_CALLBACK(X)  G_CALLBACK((void (*)(GtkWidget *,GtkSelectionData *,guint,gpointer)) X)
#define RESPONSE_EVENT_CALLBACK(X)  G_CALLBACK((gint (*)(GtkDialog *,GdkEvent *,gpointer)) X)
#define ENTRY_CALLBACK(X)  G_CALLBACK((void (*)(GtkEntry *,gpointer)) X)
#define RANGE_CALLBACK(X)  G_CALLBACK((gboolean (*)(GtkRange *,GtkScrollType,gdouble,gpointer)) X)
#define SIZE_CALLBACK(X)  G_CALLBACK((void (*)(GtkWidget *,GdkRectangle *,gpointer)) X)
#define MENUITEM_CALLBACK(X)  G_CALLBACK((void (*)(GtkMenuItem *,gpointer)) X)
#define COMBO_CALLBACK(X)  G_CALLBACK((void (*)(GtkComboBox *,gpointer)) X)
#define BUTTON_CALLBACK(X)  G_CALLBACK((void (*)(GtkButton *,gpointer)) X)
#define WIDGET_CALLBACK(X)  G_CALLBACK((void (*)(GtkWidget *,gpointer)) X)
#define CONTEXT_CALLBACK(X)  (gboolean (*)(gpointer)) X

#define FILTER_HISTORY g_get_user_data_dir(),"xffm+","xffind.filter",NULL
#define GREP_HISTORY g_get_user_data_dir(),"xffm+","xffind.grep",NULL
#define PATH_HISTORY g_get_user_data_dir(),"xffm+","xffind.path",NULL

#define BIG_ICON_SIZE                 96 
#define MEDIUM_ICON_SIZE         72 
#define SMALL_ICON_SIZE         48 
#define TINY_ICON_SIZE                 24 
#define LIST_ICON_SIZE          0

#define TINY_BUTTON        16
#define SIZE_BUTTON        22
#define SIZE_DIALOG        24
#define SIZE_ICON        48
#define SIZE_PREVIEW        96
#define SIZE_TIP        128

#ifdef ALPHA
# undef WARN
# define WARN(...)  {fprintf(stderr, "WARN> "); fprintf(stderr, __VA_ARGS__);}
#else
# define WARN(...)   { (void)0; }
#endif
typedef enum {
    NULL_TYPE,
    SUBMENU_TYPE,
    MENUITEM_TYPE,
    CHECKITEM_TYPE,
    RADIOITEM_TYPE,
    SEPARATOR_TYPE
}RodentMenuType;


typedef struct menuItem_t {
    const gchar *label;
    void *callback;
    void *callbackData;
    const gchar *toggleID;
}menuItem_t;

typedef struct menuItem2_t {
    const gchar *label;
    const gchar *icon;
    void *callback;
    void *callbackData;
    const gchar *tooltip;
    gboolean protect;
}menuItem2_t;

typedef struct radio_t {
    GtkBox *box;
    GtkToggleButton *toggle[5];
} radio_t;

typedef struct sequence_t {
    const gchar *id;
    const gchar *sequence;
} sequence_t;


typedef struct lpterm_colors_t {
    const gchar *id;
    GdkColor color;
} lpterm_colors_t;


/** higher level types **/

#define __MONITOR_TYPE                (0x1<<8)
#define IS_MONITOR_TYPE(x)        ((x) & __MONITOR_TYPE)
#define SET_MONITOR_TYPE(x)        (x) |=  __MONITOR_TYPE
#define UNSET_MONITOR_TYPE(x)        (x) &= (__MONITOR_TYPE ^ __MASK )

#define __LOCAL_TYPE                (0x1<<9)
#define IS_LOCAL_TYPE(x)        ((x) & __LOCAL_TYPE)
#define SET_LOCAL_TYPE(x)        (x) |=  __LOCAL_TYPE
#define UNSET_LOCAL_TYPE(x)        (x) &= (__LOCAL_TYPE ^ __MASK )

#define __DUMMY_TYPE                (0x1<<10)
#define IS_DUMMY_TYPE(x)        ((x) & __DUMMY_TYPE)
#define SET_DUMMY_TYPE(x)        (x) |= __DUMMY_TYPE
#define UNSET_DUMMY_TYPE(x)        (x) &= (__DUMMY_TYPE ^ __MASK )

#define __ROOT_TYPE                (0x1<<11)
#define IS_ROOT_TYPE(x)                ((x) & __ROOT_TYPE)
#define SET_ROOT_TYPE(x)        (x) |= __ROOT_TYPE
#define UNSET_ROOT_TYPE(x)        (x) &= (__ROOT_TYPE ^ __MASK )

#define __UP_TYPE                (0x1<<12)
#define IS_UP_TYPE(x)                ((x) & __UP_TYPE)
#define SET_UP_TYPE(x)                (x) |= __UP_TYPE
#define UNSET_UP_TYPE(x)        (x) &= (__UP_TYPE ^ __MASK )

#define __PROC_TYPE                (0x1<<13)
#define IS_PROC_TYPE(x)                ((x) & __PROC_TYPE)
#define SET_PROC_TYPE(x)        (x) |= __PROC_TYPE
#define UNSET_PROC_TYPE(x)        (x) &= (__PROC_TYPE ^ __MASK )

#define __CDFS_TYPE                (0x1<<14)
#define IS_CDFS_TYPE(x)                ((x) & __CDFS_TYPE)
#define SET_CDFS_TYPE(x)        (x) |= __CDFS_TYPE
#define UNSET_CDFS_TYPE(x)        (x) &= (__CDFS_TYPE ^ __MASK )

#define __USER_TYPE                (0x1<<15)
#define IS_USER_TYPE(x)                ((x) & __USER_TYPE)
#define SET_USER_TYPE(x)        (x) |= __USER_TYPE
#define UNSET_USER_TYPE(x)        (x) &= (__USER_TYPE ^ __MASK )

#define __NFS_TYPE                (0x1<<16)
#define IS_NFS_TYPE(x)                ((x) & __NFS_TYPE)
#define SET_NFS_TYPE(x)                (x) |= __NFS_TYPE
#define UNSET_NFS_TYPE(x)        (x) &= (__NFS_TYPE ^ __MASK )

#define __SMB_TYPE                (0x1<<17)
#define IS_SMB_TYPE(x)                ((x) & __SMB_TYPE)
#define SET_SMB_TYPE(x)                (x) |= __SMB_TYPE
#define UNSET_SMB_TYPE(x)        (x) &= (__SMB_TYPE ^ __MASK )

#define __FSTAB_TYPE                (0x1<<18)
#define IS_FSTAB_TYPE(x)        ((x) & __FSTAB_TYPE)
#define SET_FSTAB_TYPE(x)        (x) |= __FSTAB_TYPE
#define UNSET_FSTAB_TYPE(x)        (x) &= (__FSTAB_TYPE ^ __MASK )

#define __MTAB_TYPE                (0x1<<19)
#define IS_MTAB_TYPE(x)                ((x) & __MTAB_TYPE)
#define SET_MTAB_TYPE(x)        (x) |= __MTAB_TYPE
#define UNSET_MTAB_TYPE(x)        (x) &= (__MTAB_TYPE ^ __MASK )

#define __PARTITION_TYPE        (0x1<<20)
#define IS_PARTITION_TYPE(x)        ((x) & __PARTITION_TYPE)
#define SET_PARTITION_TYPE(x)        (x) |= __PARTITION_TYPE
#define UNSET_PARTITION_TYPE(x)        (x) &= (__PARTITION_TYPE ^ __MASK )

#define __EXE_TYPE                (0x1<<21)
#define IS_EXE_TYPE(x)                ((x) & __EXE_TYPE)
#define SET_EXE_TYPE(x)                (x) |= __EXE_TYPE
#define UNSET_EXE_TYPE(x)        (x) &= (__EXE_TYPE ^ __MASK )

#define __NOWRITE_TYPE                (0x1<<22)
#define IS_NOWRITE_TYPE(x)        ((x) & __NOWRITE_TYPE)
#define SET_NOWRITE_TYPE(x)        (x) |= __NOWRITE_TYPE
#define UNSET_NOWRITE_TYPE(x)        (x) &= (__NOWRITE_TYPE ^ __MASK )

#define __NOACCESS_TYPE                (0x1<<23)
#define IS_NOACCESS_TYPE(x)        ((x) & __NOACCESS_TYPE)
#define SET_NOACCESS_TYPE(x)        (x) |= __NOACCESS_TYPE
#define UNSET_NOACCESS_TYPE(x)        (x) &= (__NOACCESS_TYPE ^ __MASK )

#define __BROKEN_LNK                (0x1<<24)
#define IS_BROKEN_LNK(x)        ((x) & __BROKEN_LNK)
#define SET_BROKEN_LNK(x)        (x) |= __BROKEN_LNK
#define UNSET_BROKEN_LNK(x)        (x) &= (__BROKEN_LNK ^ __MASK )

#define __MOUNTED_TYPE                (0x1<<25)
#define IS_MOUNTED_TYPE(x)        ((x) & __MOUNTED_TYPE)
#define SET_MOUNTED_TYPE(x)        (x) |= __MOUNTED_TYPE
#define UNSET_MOUNTED_TYPE(x)        (x) &= (__MOUNTED_TYPE ^ __MASK )

#define __DOTDESKTOP                (0x1<<26)
#define IS_DOTDESKTOP(x)        ((x) & __DOTDESKTOP)
#define SET_DOTDESKTOP(x)        (x) |= __DOTDESKTOP
#define UNSET_DOTDESKTOP(x)        (x) &= (__DOTDESKTOP ^ __MASK )

#define __PLUGIN_TYPE1                (0x1<<27)
#define IS_PLUGIN_TYPE1(x)        ((x) & __PLUGIN_TYPE1)
#define SET_PLUGIN_TYPE1(x)        (x) |= __PLUGIN_TYPE1
#define UNSET_PLUGIN_TYPE1(x)        (x) &= (__PLUGIN_TYPE1 ^ __MASK )

#define __PLUGIN_TYPE2                (0x1<<28)
#define IS_PLUGIN_TYPE2(x)        ((x) & __PLUGIN_TYPE2)
#define SET_PLUGIN_TYPE2(x)        (x) |= __PLUGIN_TYPE2
#define UNSET_PLUGIN_TYPE2(x)        (x) &= (__PLUGIN_TYPE2 ^ __MASK )

#define __PLUGIN_TYPE3                (0x1<<29)
#define IS_PLUGIN_TYPE3(x)        ((x) & __PLUGIN_TYPE3)
#define SET_PLUGIN_TYPE3(x)        (x) |= __PLUGIN_TYPE3
#define UNSET_PLUGIN_TYPE3(x)        (x) &= (__PLUGIN_TYPE3 ^ __MASK )




#endif
