
#ifndef DIALOGS_H
# define DIALOGS_H

// basic classes
#include "classes/basic.hh"     
#include "classes/icontheme.hh"  
#include "classes/child.hh"  
#include "classes/settings.hh"  
#include "classes/texture.hh"  
#include "classes/print.hh"  
#include "classes/tubo.hh"  

#include "classes/mime/mime.hh"
#include "classes/thread.hh"
#include "classes/progress.hh"
#include "classes/gio.hh"
#include "classes/clipboard.hh"
#include "classes/bash.hh"

#include "classes/utilbasic.hh"
#include "classes/history.hh"
#include "classes/css.hh"
#include "classes/bookmarks.hh"

// basic templates
#include "templates/run.hh"  
#include "templates/runbutton.hh"

// dialog templates
#include "response/templates/dialog.hh"
#include "response/templates/dialogtimeout.hh"
#include "response/templates/dialogbuttons.hh"
#include "response/templates/dialogtimeoutbuttons.hh"
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
#include "response/classes/pathresponse.hh"  // class
# include "response/classes/cpresponse.hh"   // class
# include "response/classes/mvresponse.hh"   // class
# include "response/classes/lnresponse.hh"   // class

// menu templates
#include "menus/templates/menu.hh"
#include "menus/templates/menucallbacks.hh"
#include "menus/templates/gridviewmenu.hh"
#include "menus/templates/pathbarmenu.hh"
#include "menus/templates/outputMenu.hh"
#include "menus/templates/inputMenu.hh"
#include "menus/templates/mainMenu.hh"
#include "menus/templates/iconColorMenu.hh"

// menu classes: none

// fm classes/templates
#include "fm/classes/localdir.hh"       // fm class
#include "fm/templates/gridview.hh"     // fm template
#include "fm/templates/utilpathbar.hh"  // fm template
#include "fm/templates/workdir.hh"      // fm template
#include "fm/templates/util.hh"         // fm template
#include "fm/templates/prompt.hh"       // fm template
#include "fm/classes/fmbuttonbox.hh"    // fm class
#include "fm/templates/pathbar.hh"      // fm template
#include "fm/classes/fmpage.hh"         // fm class

#include "window.hh" // template
#include "fm.hh"     // class

#endif
