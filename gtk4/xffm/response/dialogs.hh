#ifndef DIALOGS_HH
# define DIALOGS_HH
#include "response/templates/dialogbasic.hh"
#include "response/templates/dialogtimeout.hh"
#include "response/templates/dialogbuttons.hh"
#include "response/templates/dialogentry.hh"
#include "response/templates/dialogpath.hh"
#include "response/templates/openwith.hh"        
#include "response/templates/dialogprompt.hh"        

// response classes
// jumpresponse is a template to access Workdir<Type>
#include "response/classes/jumpresponse.hh"  // template
#include "response/classes/entryresponse.hh" // class
#include "response/classes/passwdresponse.hh"// class
#include "response/classes/rmresponse.hh"    // class
#include "response/classes/rmlistresponse.hh"    // class
#include "response/classes/pathresponse.hh"  // class
# include "response/classes/cpresponse.hh"   // class
# include "response/classes/mvresponse.hh"   // class
# include "response/classes/lnresponse.hh"   // class
# include "response/classes/info.hh"   // class

namespace xf
{
  class Dialogs {
    public:
    static void info(const char *text){
      auto dialogObject = new DialogTimeout<infoResponse>;
      dialogObject->setParent(GTK_WINDOW(MainWidget));
      dialogObject->setLabelText(text);

      DBG("create dialogObject=%p\n", dialogObject); 
      dialogObject->run();
    }

    static void rm(GFileInfo *info){
      auto dialogObject = new DialogButtons<rmResponse>;
      auto dialog = dialogObject->dialog();
      g_object_set_data(G_OBJECT(dialog), "info", info);
      dialogObject->setParent(GTK_WINDOW(MainWidget));
      dialogObject->subClass()->setDefaults(dialog, dialogObject->label());
      dialogObject->run();
    }

    static void rmList(GtkPopover *menu, GList *selectionList){
      auto dialogObject = new DialogButtons<rmListResponse>;
      auto dialog = dialogObject->dialog();
      g_object_set_data(G_OBJECT(dialog), "menu", menu);
      g_object_set_data(G_OBJECT(dialog), "selectionList", selectionList);
      dialogObject->setParent(GTK_WINDOW(MainWidget));
      dialogObject->subClass()->setDefaults(dialog, dialogObject->label());
      dialogObject->run();
    }

  };

}

#endif
