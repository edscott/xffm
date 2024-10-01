
#ifndef DIALOGS_H
# define DIALOGS_H

// basic classes
#include "basic/basic.hh"     
#include "basic/icontheme.hh"  
#include "basic/child.hh"  
#include "basic/settings.hh"  
#include "basic/texture.hh"  
#include "basic/print.hh"  
#include "basic/tubo.hh"  

#include "basic/mime/mime.hh"
#include "basic/thread.hh"
#include "basic/progress.hh"
#include "basic/gio.hh"
#include "basic/clipboard.hh"
#include "basic/bash.hh"

#include "basic/utilbasic.hh"
#include "basic/history.hh"
#include "basic/css.hh"



// basic templates
#include "basic/run.hh"  

// dialog templates
#include "response/templates/dialog.hh"
#include "response/templates/dialogtimeout.hh"
#include "response/templates/dialogbuttons.hh"
#include "response/templates/dialogentry.hh"
#include "response/templates/dialogpath.hh"
#include "response/templates/openwith.hh"        

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

#include "utilpathbar.hh" 
#include "bookmarks.hh"
#include "fm/localdir.hh"
#include "fm/gridview.hh"

#include "workdir.hh"
#include "util.hh"

#include "menus/templates/outputMenu.hh"
#include "menus/templates/inputMenu.hh"
#include "menus/templates/mainMenu.hh"


// menu classes
#include "menus/classes/iconColorMenu.hh"

#endif
