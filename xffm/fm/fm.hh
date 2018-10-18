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
#include "completion/csh.hh"
#include "view/baseview.hh"

namespace xf
{

template <class Type>
class fmDialog : public Dialog<Type>{
    using print_c = Print<double>;
public:
    fmDialog(const gchar *path):Dialog<Type>(path){
	
        auto page = this->currentPageObject();
        //page->setPageWorkdir(workdir);
	// Default into iconview...
        page->setDefaultIconview(TRUE);
	page->showIconview(1);
        //page->setDefaultIconview(FALSE);
	//page->showIconview(FALSE);
    }

    void addPage(const gchar *workdir){
        WARN("loading iconview page\n");
        auto notebook = (Notebook<Type> *)this;
        auto page = notebook->addPage(workdir);
        
        //while (gtk_events_pending())gtk_main_iteration();
        if (workdir == NULL) workdir = g_strdup("xffm:root");
        auto baseView = load(workdir);
	page->showIconview(1);
    }

    void removePage(GtkWidget *child){
        WARN("removePage iconview page\n");
        auto notebook = (Notebook<Type> *)this;
        //auto baseView = notebook->baseView(child);
        notebook->removePage(child);
        //delete baseView;
    }
   
    BaseView<Type> *load(const gchar *workdir){
        auto notebook = (Notebook<Type> *)this;
	auto page = notebook->currentPageObject();
	WARN("adding rootview\n");
	// Create BaseView object.
        auto baseView =  new BaseView<Type>(page, workdir);
        g_object_set_data(G_OBJECT(page->top_scrolled_window()), "baseView", baseView);
        // Add the iconview into the scrolled window.
	gtk_container_add (GTK_CONTAINER (page->top_scrolled_window()),
		GTK_WIDGET(baseView->iconView()));
	gtk_widget_show (GTK_WIDGET(baseView->iconView()));
        // Load contents, depending on what path specifies.
	baseView->loadModel(workdir);
	while (gtk_events_pending())gtk_main_iteration();
        return baseView;
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
