#ifndef MAINMENU_HH
#define MAINMENU_HH
#include "menu.hh"
namespace xf {
  class MainMenu {
    public:
    const char **keys(void){
      static const char *keys_[] = { // Order is important.
        _("Open in New Window"),  // removed then from vbutton box
        _("Copy"), 
        _("Cut"), 
        _("Paste"), 
        _("Delete"), 
        _("Select All"), 
        _("Match regular expression"), 
        _("Close"), 
        NULL
      };
      return keys_;
    }
    MenuInfo_t *iconNames(void){
      static MenuInfo_t menuIconNames_[] = { // Need not be complete with regards to keys_.
        {_("Open in New Window"),(void *) DUAL_VIEW}, 
        {_("Copy"), (void *) EDIT_COPY},
        {_("Cut"),(void *) EDIT_CUT}, 
        {_("Paste"),(void *) EDIT_PASTE}, 
        {_("Delete"),(void *) EDIT_DELETE}, 
        {_("Select All"),(void *)  VIEW_MORE},
        {_("Match regular expression"),(void *)  DIALOG_QUESTION}, 
        {_("Close"),(void *)  WINDOW_SHUTDOWN},
        {NULL, NULL}
      }; 
      return menuIconNames_;
    }
    MenuInfo_t *callbacks(void){
      static MenuInfo_t menuCallbacks_[] = { // Need not be complete with regards to keys_.
        {_("Open in New Window"),(void *) openXffm}, 
        {_("Copy"), (void *) NULL},
        {_("Cut"),(void *) NULL}, 
        {_("Paste"),(void *) NULL}, 
        {_("Delete"),(void *) NULL}, 
        {_("Select All"),(void *)  NULL},
        {_("Match regular expression"),(void *)  NULL}, 
        {_("Close"),(void *)  close},
        {NULL, NULL}
      };
      return menuCallbacks_;
    }
    MenuInfo_t *data(void){
      static MenuInfo_t menuData_[] = { // Need not be complete with regards to keys_ nor menuCallbacks_.
        {_("Open in New Window"),(void *) NULL}, 
        {_("Copy"), (void *) NULL},
        {_("Cut"),(void *) NULL}, 
        {_("Paste"),(void *) NULL}, 
        {_("Delete"),(void *) NULL}, 
        {_("Select All"),(void *)  NULL},
        {_("Match regular expression"),(void *)  NULL}, 
        {_("Close"),(void *)  NULL},
        {NULL, NULL}
      };
      return menuData_;      
    }

  private:
    static void
    openXffm(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
      gtk_popover_popdown(menu);
      auto childWidget =Util::getCurrentChild();
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(childWidget), "output"));
      auto buttonSpace = GTK_BOX(g_object_get_data(G_OBJECT(childWidget), "buttonSpace"));
      auto workDir = Child::getWorkdir(childWidget);

      auto xffm = g_strdup_printf("xffm -f %s", workDir);
      pid_t childPid = Run::shell_command(output, xffm, false, false);

      auto runButton = new (RunButton);
      runButton->init(runButton, xffm, childPid, output, workDir, buttonSpace);
      g_free(xffm);
      return;
    }
    static void
    close(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu"));
      gtk_popover_popdown(menu);
      gtk_widget_set_visible(MainWidget, FALSE);
      gtk_window_destroy(GTK_WINDOW(MainWidget));
    }
    

  };


}
#endif
