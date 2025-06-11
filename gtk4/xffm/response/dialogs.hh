#ifndef DIALOGS_HH
# define DIALOGS_HH
#include "response/classes/dialog.hh"
#include "response/templates/dialogbasic.hh"
#include "response/templates/dialogcomplex.hh"
#include "response/templates/dialogsimple.hh"
#include "response/templates/dialogtimeout.hh"
#include "response/templates/dialogdrop.hh"
#include "response/templates/dialogbuttons.hh"
#include "response/templates/dialogpasswd.hh"
#include "response/templates/dialogentry.hh"
#include "response/templates/dialogpath.hh"
#include "response/templates/openwith.hh"        
#include "response/templates/dialogprompt.hh"        
#include "response/templates/jumpresponse.hh"  // template

// response classes
// jumpresponse is a template to access Workdir<Type>
#include "response/classes/entryresponse.hh" // class
#include "response/classes/wsresponse.hh"// class
#include "response/classes/passwdresponse.hh"// class
#include "response/classes/rmresponse.hh"    // class
#include "response/classes/rmlistresponse.hh"    // class
#include "response/classes/pathresponse.hh"  // class
# include "response/classes/cpresponse.hh"   // class
# include "response/classes/mvresponse.hh"   // class 
# include "response/classes/lnresponse.hh"   // class
# include "response/classes/inforesponse.hh"   // class
# include "response/classes/dndresponse.hh"   // class
# include "response/classes/envresponse.hh"   // class
                                              // 
#include "response/templates/mkdirresponse.hh"
#include "response/classes/fileresponsepathbar.hh"
#include "response/templates/fileresponse.hh" // class
#include "response/templates/efsresponse.hh" // class
#include "response/templates/efs.hh" // class
# include "response/templates/mountresponse.hh"   // class
# include "response/templates/mount.hh"   // class
# include "response/templates/bfishresponse.hh"   // class
# include "response/templates/bfish.hh"   // class
# include "response/templates/tarresponse.hh"   // class
# include "response/templates/tar.hh"   // class

namespace xf
{
  class Dialogs {
    public:
    static void info(const char *text){
      auto dialogObject = new DialogTimeout<infoResponse>;
      dialogObject->setParent(GTK_WINDOW(Child::mainWidget()));
      dialogObject->setLabelText(text);
      gtk_widget_realize(GTK_WIDGET(dialogObject->dialog()));
      Basic::setAsDialog(GTK_WINDOW(dialogObject->dialog()));
      gtk_window_present(GTK_WINDOW(dialogObject->dialog()));
      TRACE("create dialogObject=%p, dialog=%p\n", 
          dialogObject, dialogObject->dialog()); 
      dialogObject->run();
    }



    static void rm(GFileInfo *info){
      auto dialogObject = new DialogButtons<rmResponse>;
      auto dialog = dialogObject->dialog();
      g_object_set_data(G_OBJECT(dialog), "info", info);
      dialogObject->setParent(GTK_WINDOW(Child::mainWidget()));

      auto text = g_strdup_printf(_("Delete '%s'"), g_file_info_get_name(info));
      auto markup = g_strdup_printf("<span color=\"red\">%s</span>\n", text);
      dialogObject->setLabelText(markup);
      g_free(text);
      g_free(markup);

      dialogObject->run();
    }

    static void rmList(GList *selectionList){
      if (!selectionList) return;
      rmList(NULL, selectionList);
      return;
    }

    static void rmList(GtkPopover *menu, GList *selectionList){
      if (!selectionList) return;
      if (g_list_length(selectionList) == 1){
        auto info = G_FILE_INFO(selectionList->data);
        rm(info);
        return;
      }
      auto dialogObject = new DialogButtons<rmListResponse>;
      auto dialog = dialogObject->dialog();
      g_object_set_data(G_OBJECT(dialog), "menu", menu);
      g_object_set_data(G_OBJECT(dialog), "selectionList", selectionList);
      dialogObject->setParent(GTK_WINDOW(Child::mainWidget()));
      dialogObject->subClass()->setDefaults(dialog, dialogObject->label());
      dialogObject->run();
      return;
    }

  };

}

#endif
