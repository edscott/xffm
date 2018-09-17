#ifndef XF_TERM__HH
# define XF_TERM_HH


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
public:
    termDialog(void){
	// Here we override the default start up of the Dialog class template
        gchar *workdir = g_get_current_dir();
        this->setDialogSize(600,400);
        auto page = this->currentPageObject();
        page->setPageWorkdir(workdir);
	// Default into terminal...
	page->showIconview(FALSE, TRUE);
        page->setDefaultIconview(FALSE);
        //gtk_widget_hide(GTK_WIDGET(page->toggleToTerminal()));
        //gtk_widget_hide(GTK_WIDGET(page->toggleToIconview()));
        print_c::print(page->output(), "tag/red", g_strdup("Hello world!\n"));
    }
};


template <class Type>
class Term: protected termDialog<Type> {
public:
    Term(const gchar *path){
        gchar *fullPath = NULL;
        if (path){
            if (g_path_is_absolute(path)) fullPath = g_strdup(path);
            else fullPath = g_build_filename(g_get_home_dir(), path, NULL);
        }
	this->createDialog(fullPath);
        g_free(fullPath);
    }
};
} // namespace xf
#endif
