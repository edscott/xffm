#ifndef XF_FM__HH
# define XF_FM_HH


#include "common/types.h"
#include "common/icons.hh"
#include "common/pixbuf.hh"
#include "common/gtk.hh"
#include "common/tooltip.hh"
#include "common/dialog.hh"
#include "common/util.hh"

#include "common/print.hh"
#include "common/completion/csh.hh"

#include "common/view/rootview.hh"

namespace xf
{

template <class Type>
class fmDialog : public Dialog<Type>{
    using print_c = Print<double>;
public:
    fmDialog(const gchar *path):Dialog<Type>(path){
	// Here we override the default start up of the Dialog class template
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
	// Default into iconview...
        page->setDefaultIconview(TRUE);
	page->showIconview(TRUE, TRUE);
    }

    //FIXME:
    //  we need a reload function, which can choose which class to act upon.
    //  This would be a template function
    void addPage(const gchar *workdir){
        auto notebook = (Notebook<Type> *)this;
        auto page = notebook->addPage(workdir);

	DBG("adding page with workdir=%s\n", workdir);
	if (workdir && strcmp(workdir, "xffm:root")==0){
	    WARN("now adding RootView to xffm addPage()\n");
	    //auto baseView =  std::make_shared<BaseView>(double);
	    auto baseView =  new RootView<Type>("xffm:root");

	    g_object_set_data(G_OBJECT(page->top_scrolled_window()), "baseView", (void *)baseView);
	    gtk_container_add (GTK_CONTAINER (page->top_scrolled_window()), GTK_WIDGET(baseView->iconView()));
	    gtk_widget_show (GTK_WIDGET(baseView->iconView()));
	}
        

    }
};


template <class Type>
class Fm: protected fmDialog<Type> {
public:
    Fm(const gchar *path){
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
