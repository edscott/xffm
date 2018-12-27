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
    }

    void addPage(const gchar *workdir){
        TRACE("loading iconview page %s\n", workdir);
        auto notebook = (Notebook<Type> *)this;
        auto page = notebook->addPage(workdir);
        auto view = load(page->workDir());
	page->showIconview(1);
        if (workdir == NULL){
            view->loadModel(workdir);
        }
    }

   
    View<Type> *load(const gchar *workdir){
        auto notebook = (Notebook<Type> *)this;
	auto page = notebook->currentPageObject();
	TRACE("fm.hh::adding page: %s\n", workdir);
	// Create View object.
        auto view =  new View<Type>(page, workdir);
        g_object_set_data(G_OBJECT(page->topScrolledWindow()), "baseModel", view);
	
        g_object_set_data(G_OBJECT(page->topScrolledWindow()), "view", view);
        // Add the iconview into the scrolled window.
	gtk_container_add (GTK_CONTAINER (page->topScrolledWindow()),
		GTK_WIDGET(view->iconView()));
	gtk_widget_show (GTK_WIDGET(view->iconView()));
        // Load contents, depending on what path specifies.
	view->loadModel(workdir);
	while (gtk_events_pending())gtk_main_iteration();
        return view;
    }
};
} // namespace xf
#endif
