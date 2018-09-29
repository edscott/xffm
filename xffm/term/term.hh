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
    termDialog(const gchar *path):Dialog<Type>(path){
	// Here we override the default start up of the Dialog class template
        // gchar *workdir = g_get_current_dir();
	// set minimum size
        this->setMinimumSize(500,300);
	// set actual size (read this from rc file) FIXME
	this->setDefaultSize(750,500);
	// set actual size (read this from rc file) FIXME
	// FIXME: does not work, must retrieve adjustment
	//        for gtk range and set value (common/dialog.hh)
	this->setDefaultFixedFontSize(13);
        auto page = this->currentPageObject();
        //page->setPageWorkdir(workdir);
	// Default into terminal...
        page->setDefaultIconview(FALSE);
	page->showIconview(FALSE, TRUE);
        //gtk_widget_hide(GTK_WIDGET(page->toggleToTerminal()));
        //gtk_widget_hide(GTK_WIDGET(page->toggleToIconview()));
     
        print_c::print(page->output(), "red", g_strdup("Hello world!\n"));
        print_c::print(page->output(), "Red", g_strdup("Hello world!\n"));
        print_c::print(page->output(), "bold/red/white_bg", g_strdup("Hello world!\n"));
        print_c::print(page->output(), "bold/Red", g_strdup("Hello world!\n"));

        print_c::print(page->output(), "bold/Red", g_strdup("Hello world!\n"));
        //print_c::print(page->output(), "white", g_strdup("Use "));

        //print_c::print(page->output(), "bold/White", g_strdup("script -f -c \\\"command\\\" dev/null "));
        //print_c::print(page->output(), "white", g_strdup("for color output (YMMV)\n"));
    }
};

/*
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
};*/
} // namespace xf
#endif
