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

#include "view/rootview.hh"
#include "view/localview.hh"

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
	page->showIconview(TRUE);
        //page->setDefaultIconview(FALSE);
	//page->showIconview(FALSE);
    }

    //FIXME:
    //  we need a reload function, which can choose which class to act upon.
    //  This would be a template function
    void addPage(const gchar *workdir){
        WARN("loading iconview page\n");
        auto notebook = (Notebook<Type> *)this;
        auto page = notebook->addPage(workdir);
        
        //while (gtk_events_pending())gtk_main_iteration();
        load(workdir);
	page->showIconview(TRUE);
    }

    
    void load(const gchar *workdir){
        auto notebook = (Notebook<Type> *)this;
	auto page = notebook->currentPageObject();
	WARN("adding rootview\n");
	auto baseView =  new BaseView<Type>((void *)this, workdir);

        if (!RootView<Type>::enableDragSource()){
            gtk_icon_view_unset_model_drag_source (baseView->iconView());
        }
        if (!RootView<Type>::enableDragDest()){
            gtk_icon_view_unset_model_drag_dest (baseView->iconView());
        }

	gtk_container_add (GTK_CONTAINER (page->top_scrolled_window()),
		GTK_WIDGET(baseView->iconView()));
	gtk_widget_show (GTK_WIDGET(baseView->iconView()));
	RootView<Type>::loadModel(baseView->treeModel(), workdir);
	while (gtk_events_pending())gtk_main_iteration();

#if 0
	if (g_object_get_data(G_OBJECT(page->top_scrolled_window()),
		    "rootView")){
	    g_object_set_data(G_OBJECT(page->top_scrolled_window()),
		    "rootView", NULL);
	    auto baseView = (BaseView<RootView<Type> > *)
		g_object_get_data(G_OBJECT(page->top_scrolled_window()),
			"baseView");
	    if (baseView){
		WARN("removing baseView\n");
		gtk_widget_hide (GTK_WIDGET(baseView->iconView()));		
		g_object_ref(G_OBJECT(baseView->iconView()));
		gtk_container_remove(GTK_CONTAINER (page->top_scrolled_window()), 
			    GTK_WIDGET(baseView->iconView()));
		delete baseView;
	    }
	}
	// FIXME 
	// basically the same. We need to define a template function
	if (g_object_get_data(G_OBJECT(page->top_scrolled_window()),
		    "localView")){
	    g_object_set_data(G_OBJECT(page->top_scrolled_window()),
		    "localView", NULL);
	    auto baseView = (BaseView<LocalView<Type> > *)
		g_object_get_data(G_OBJECT(page->top_scrolled_window()),
			"baseView");
	    if (baseView){
		WARN("removing baseView\n");
		gtk_widget_hide (GTK_WIDGET(baseView->iconView()));		
		g_object_ref(G_OBJECT(baseView->iconView()));
		gtk_container_remove(GTK_CONTAINER (page->top_scrolled_window()), 
			    GTK_WIDGET(baseView->iconView()));
		delete baseView;
	    }
	}
	if (!workdir || strcmp(workdir, "xffm:root")==0) {

	    WARN("adding rootview\n");
	    auto baseView =  new BaseView<RootView<Type> >((void *)this, "xffm:root");

	    gtk_container_add (GTK_CONTAINER (page->top_scrolled_window()),
		    GTK_WIDGET(baseView->iconView()));
	    gtk_widget_show (GTK_WIDGET(baseView->iconView()));
	    while (gtk_events_pending())gtk_main_iteration();
	    g_object_set_data(G_OBJECT(page->top_scrolled_window()), "baseView",(void *)baseView);
	    g_object_set_data(G_OBJECT(page->top_scrolled_window()), "rootView",(void *)baseView);
	}
	// FIXME 
	// basically the same. We need to define a template function
	if (g_file_test(workdir, G_FILE_TEST_IS_DIR)) {
	    WARN("adding localview\n");
	    auto baseView =  new BaseView<LocalView<Type> >((void *)this, workdir);

	    gtk_container_add (GTK_CONTAINER (page->top_scrolled_window()),
		    GTK_WIDGET(baseView->iconView()));
	    gtk_widget_show (GTK_WIDGET(baseView->iconView()));
	    while (gtk_events_pending())gtk_main_iteration();
	    g_object_set_data(G_OBJECT(page->top_scrolled_window()), "baseView",(void *)baseView);
	    g_object_set_data(G_OBJECT(page->top_scrolled_window()), "localView",(void *)baseView);
	}
#endif
	
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
