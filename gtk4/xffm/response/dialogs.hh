#ifndef DIALOGS_HH
# define DIALOGS_HH
#include "response/classes/dialog.hh"
#include "response/templates/dialogbasic.hh"
#include "response/templates/dialogtimeout.hh"
#include "response/templates/dialogdrop.hh"
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
# include "response/classes/dndresponse.hh"   // class

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

    static char **getUriFiles(const char *uriList, char **path_p){
      auto files = g_strsplit(uriList, "\n", -1);
      for (auto p=files ; p && *p; p++){
        if (strncmp(*p, "file://", strlen("file://")) == 0){
          for (int i=0; i<strlen("file://"); i++) (*p)[i] = ' ';
          g_strstrip(*p);
          if (*path_p == NULL) *path_p = g_path_get_dirname(*p);
        }
        if (strlen(*p) == 0) *p = NULL;
      }
      return files;
    }

    
    static char *getFilesBlock(char **files){
      auto block = g_strdup("");
      int count = 0;
      int total = 0;
      for (auto p=files ; p && *p; p++)total++;
      for (auto p=files ; p && *p; p++, count++){
        if (count >= 5) {
          auto h = g_strdup_printf("%d %s", total-count, _("files"));
          auto hh = g_strdup_printf(_("+ %s more"), h);
          Basic::concat(&block, hh);
          g_free(h);
          g_free(hh);
          break;
        }
        auto g = g_path_get_basename(*p);
        Basic::concat(&block, _("file"));
        Basic::concat(&block, _(": "));
        Basic::concat(&block, g);
        Basic::concat(&block, "\n");
        g_free(g);
      }
      return block;
    }

    static void dnd(const char *uriList, char *target){
      char *source = NULL;
      auto files = getUriFiles(uriList, &source);
      auto block = getFilesBlock(files);
      auto markup = g_strconcat("<span color=\"red\">",_("Target"), ": </span>", target, "\n",
         "<span color=\"green\">", _("Source"),  ": </span>", source, "\n",
         "<span color=\"black\">", block, "\n</span>", NULL);
      if (strcmp(source, target)) {
        auto dialogObject = new DialogButtons<dndResponse>;
        dialogObject->setParent(GTK_WINDOW(MainWidget));

        dialogObject->setLabelText(markup);
        DBG("create dialogObject=%p\n", dialogObject); 
        dialogObject->run();
        
      } else {
        DBG("Source and target are the same: %s\n", source);
      }
      g_free(markup);
      g_free(block);
      g_strfreev(files);


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
