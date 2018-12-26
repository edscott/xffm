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

static const gchar *xffmProgram;
static const gchar *xffindProgram;
static GtkWindow *mainWindow = NULL;
static gboolean isTreeView;


namespace xf
{
    template <class Type> class BaseModel;
    template <class Type> class BaseView;
    template <class Type> class Dialog;
    template <class Type> class LocalRm;
    template <class Type> class LocalRm;
    template <class Type> class Page;
    template <class Type> class LocalView;
    template <class Type> class EntryResponse;
    template <class Type> class EntryFolderResponse;
    template <class Type> class ComboResponse;
    template <class Type> class FstabMonitor;
    template <class Type> class FindDialog;
    template <class Type> class CommandResponse;
    template <class Type> class Properties;
    template <class Type> class BaseCompletion;
}



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

#include "model/base/basepopup.hh"

#include "view/baseview.hh"
#include "view/fstab/fstab.hh"
#include "view/fstab/fstabpopup.hh"
#include "view/fstab/fstabmonitor.hh"
#include "view/local/localclipboard.hh"
#include "view/local/localproperties.hh"

#include "find/fgr.hh"
#include "find/find.hh"
#include "find/signals.hh"

#include "response/passwdresponse.hh"
#include "response/comboresponse.hh"
#include "response/commandresponse.hh"



#include "dialog/notebook.hh"
#include "dialog/dialogsignals.hh"
#include "dialog/dialog.hh"
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

    xf::LocalClipBoard<double>::startClipBoard();  
    gtk_main();
    xf::LocalClipBoard<double>::stopClipBoard();  
    return 0;
}
