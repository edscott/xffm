#ifndef XF_TERM_DIALOG
#define XF_TERM_DIALOG


#include "common/types.h"
#include "common/icons.hh"
#include "common/pixbuf.hh"
#include "common/gtk.hh"
#include "common/tooltip.hh"
#include "common/dialog.hh"
#include "common/util.hh"

#include "common/print.hh"
#include "common/completion/csh.hh"

namespace xf 
{
    
template <class Type>
class termDialog : public Dialog<Type>{
    using print_c = Print<double>;
    using util_c = Util<double>;
public:
    termDialog(void){
	// Here we override the default start up of the Dialog class template
	//DBG("%s %s\n", title, icon);
        gchar *workdir = g_get_current_dir();
        this->setDialogSize(600,400);
        auto page = this->currentPageObject();
        page->setPageWorkdir(workdir);

	PageSignals<Type>::onIconviewIcon(NULL, page);

        print_c::print(page->diagnostics(), "tag/red", g_strdup("Hello world!\n"));
        //DBG("current page = %d\n", this->currentPage());
        //this->setPageLabel(this->currentPage(), "foo");
    }
 
private:


};
}



#endif
