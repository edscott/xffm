
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
#include "response/templates/dialogentry.hh"
#include "response/templates/dialogpath.hh"
#include "response/templates/openwith.hh"        
#include "response/templates/dialogprompt.hh"        

// response classes
#include "response/classes/entryresponse.hh"
#include "response/classes/passwdresponse.hh"
#include "response/classes/rmresponse.hh"
#include "response/classes/pathresponse.hh" 
# include "response/classes/cpresponse.hh"
# include "response/classes/mvresponse.hh"
# include "response/classes/lnresponse.hh"

// menu templates
#include "menus/templates/menu.hh"
#include "menus/templates/menucallbacks.hh"
#include "menus/templates/gridviewmenu.hh"
#include "menus/templates/pathbarmenu.hh"
#include "menus/templates/outputMenu.hh"
#include "menus/templates/inputMenu.hh"
#include "menus/templates/mainMenu.hh"
#include "menus/templates/iconColorMenu.hh"


#include "fm/classes/localdir.hh" // fm classes

// fm templates
#include "fm/templates/gridview.hh"
#include "fm/templates/utilpathbar.hh" 
#include "fm/templates/workdir.hh"
#include "fm/templates/util.hh"
#include "fm/templates/prompt.hh"

#include "response/classes/jumpresponse.hh" // fixme, after Workdir...

// fm classes
#include "fm/classes/fmbuttonbox.hh" // fm classes
#include "fm/classes/pathbar.hh" // fm classes
#include "fm/classes/fmpage.hh" // fm classes


#include "window.hh"
#include "fm.hh"

#endif
