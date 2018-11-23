#ifndef XF_FM__HH
# define XF_FM_HH



#include "dialog.hh"
#include "completion/csh.hh"
#include "fm/view/base/baseview.hh"

namespace xf
{

template <class Type>
class fmDialog : public Dialog<Type>{
    using print_c = Print<double>;
public:
    fmDialog(const gchar *path):Dialog<Type>(path){
        Mime<Type>::mimeBuildHashes();
	
        auto page = this->currentPageObject();
	// Default into iconview...
        page->setDefaultIconview(TRUE);
	page->showIconview(1);
        //page->setDefaultIconview(FALSE);
	//page->showIconview(FALSE);
    }

    void addPage(const gchar *workdir){
        TRACE("loading iconview page %s\n", workdir);
        auto notebook = (Notebook<Type> *)this;
        auto page = notebook->addPage(workdir);
        auto baseView = load(page->workDir());
	page->showIconview(1);
        if (workdir == NULL){
            baseView->loadModel(workdir);
        }
    }

    void removePage(GtkWidget *child){
        TRACE("removePage iconview page\n");
        auto notebook = (Notebook<Type> *)this;
        //auto baseView = notebook->baseView(child);
        notebook->removePage(child);
        //delete baseView;
    }
   
    BaseView<Type> *load(const gchar *workdir){
        auto notebook = (Notebook<Type> *)this;
	auto page = notebook->currentPageObject();
	TRACE("fm.hh::adding page: %s\n", workdir);
	// Create BaseView object.
        auto baseView =  new BaseView<Type>(page, workdir);
        g_object_set_data(G_OBJECT(page->topScrolledWindow()), "baseView", baseView);
        // Add the iconview into the scrolled window.
	gtk_container_add (GTK_CONTAINER (page->topScrolledWindow()),
		GTK_WIDGET(baseView->iconView()));
	gtk_widget_show (GTK_WIDGET(baseView->iconView()));
        // Load contents, depending on what path specifies.
	baseView->loadModel(workdir);
	while (gtk_events_pending())gtk_main_iteration();
        return baseView;
    }
};

/*
template <class Type>
class Fm: protected fmDialog<Type> {
public:
    Fm(const gchar *path){
        gchar *fullPath = NULL;
        if (path){
            if (g_path_is_absolute(path)) fullPath = g_strdup(path);
            else fullPath = g_build_filename(g_get_home_dir(), path, NULL);
            this->createDialog(fullPath);
            g_free(fullPath);
        } else this->createDialog(path);
    }
};
*/
} // namespace xf
#endif
