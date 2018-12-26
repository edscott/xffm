/*
 * Copyright 2005-2018 Edscott Wilson Garcia 
 * license: GPL v.3
 */

#define XFFM_CC
#include "config.h"
#include "types.h"
#include "intl.h"


#define FORCE_CORE
#ifdef FORCE_CORE
# include <sys/time.h>
# include <sys/resource.h>
#endif
#include <memory>

#define URIFILE "file://"
enum
{
    ROOTVIEW_TYPE,
    LOCALVIEW_TYPE,
    FSTAB_TYPE,
    NFS_TYPE,
    SSHFS_TYPE,
    ECRYPTFS_TYPE,
    CIFS_TYPE,
    PKG_TYPE
};

enum
{
  FLAGS,
  TREEVIEW_PIXBUF,
  DISPLAY_PIXBUF,
  NORMAL_PIXBUF,
  HIGHLIGHT_PIXBUF,
  TOOLTIP_PIXBUF,
  DISPLAY_NAME,
  ACTUAL_NAME,
  PATH,
  SIZE,
  DATE,
  TOOLTIP_TEXT,
  ICON_NAME,
  TYPE,
  MIMETYPE, 
  PREVIEW_PATH,
  PREVIEW_TIME,
  PREVIEW_PIXBUF,
  NUM_COLS
};
    static const gchar *xffmProgram;
    static const gchar *xffindProgram;
    static GtkWindow *mainWindow = NULL;


namespace xf {
    static gboolean isTreeView;
    static GList *localMonitorList = NULL;

    template <class Type> class Page;
    template <class Type> class Pixbuf;
    template <class Type> class LocalClipboard;
    template <class Type> class BaseModel;
    template <class Type> class BaseView;
    template <class Type> class LocalMonitor;
    template <class Type> class FstabMonitor;
}

    
#include "common/util.hh"
#include "common/run.hh"
#include "common/gio.hh"
#include "common/dnd.hh"
#include "common/clipboard.hh"
#include "common/tubo.hh"
#include "common/print.hh"
#include "common/run.hh"
#include "common/util.hh"
#include "common/gtk.hh"
#include "common/tooltip.hh"
#include "common/pixbuf.hh"
#include "common/icons.hh"
#include "common/settings.hh"
#include "common/mime.hh"

#include "response/passwdresponse.hh"
#include "response/comboresponse.hh"
#include "response/commandresponse.hh"

#include "fm/base/signals.hh"
#include "fm/base/model.hh"

#include "find/fgr.hh"
#include "find/find.hh"
#include "find/signals.hh"

#include "fm/view/root/view.hh"
#include "fm/view/local/view.hh"
#include "fm/view/fstab/view.hh"
#include "dialog/dialog.hh"

#if 10

int
main (int argc, char *argv[]) {
    if (chdir(g_get_home_dir()) < 0){
        ERROR("Cannot chdir to %s (%s)\n", g_get_home_dir(), strerror(errno));
        exit(1);
    }
    xffindProgram = argv[0];
    xffmProgram = argv[0];
    // common stuff
    // FIXME: this should be called as a class 
#include "startup.hh"

    auto xffm = new(xf::Dialog<double>)(path);
    g_object_set_data(G_OBJECT(mainWindow), "xffm", xffm);
    //xffm->setDialogTitle("Fm");
    xffm->setDialogIcon("system-file-manager");

    xf::ClipBoard<double>::startClipBoard();  
    gtk_main();
    xf::ClipBoard<double>::stopClipBoard();  
    return 0;
}
#endif
