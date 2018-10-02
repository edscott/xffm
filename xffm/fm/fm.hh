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
        //page->setDefaultIconview(FALSE);
	//page->showIconview(FALSE, FALSE);
    }

    //FIXME:
    //  we need a reload function, which can choose which class to act upon.
    //  This would be a template function
    void addPage(const gchar *workdir){
        WARN("loading iconview page\n");
        auto notebook = (Notebook<Type> *)this;
        auto page = notebook->addPage(workdir);
        
        while (gtk_events_pending())gtk_main_iteration();
        load(workdir);
    }
    void load(const gchar *workdir){
        auto notebook = (Notebook<Type> *)this;
        auto page = notebook->currentPageObject();

        // FIXME: if object data, then hide current and show other
        //        otherwise hide, create and show.

	WARN("adding page with workdir=%s\n", workdir);
	if (workdir && strcmp(workdir, "xffm:root")==0)
        {
            auto rootView =  new BaseView<RootView<Type>>("xffm:root");
            g_object_set_data(G_OBJECT(page->top_scrolled_window()), "rootView", (void *)rootView);
            g_object_set_data(G_OBJECT(page->top_scrolled_window()), "baseView", (void *)rootView);
            gtk_container_add (GTK_CONTAINER (page->top_scrolled_window()),
                    GTK_WIDGET(rootView->iconView()));
            gtk_widget_show (GTK_WIDGET(rootView->iconView()));
            while (gtk_events_pending())gtk_main_iteration();
        }
    }

#if 0

            auto rootView = (BaseView<RootView<Type>> *)
                g_object_get_data(G_OBJECT(page->top_scrolled_window()), "rootView");
            auto baseView = (BaseView<Type> *)
                g_object_get_data(G_OBJECT(page->top_scrolled_window()), "baseView");
            if (!rootView) {
                WARN("new rootview\n");
                rootView =  new BaseView<RootView<Type>>("xffm:root");
                g_object_set_data(G_OBJECT(page->top_scrolled_window()), "rootView", (void *)rootView);
            }

            if ((void *)baseView != (void *)rootView){
                if (baseView){
                WARN("removing baseview\n");
                    g_object_ref(G_OBJECT(baseView->iconView()));
                    gtk_container_remove(GTK_CONTAINER (page->top_scrolled_window()), 
                                GTK_WIDGET(baseView->iconView()));
                }
                WARN("adding rootview\n");
                gtk_container_add (GTK_CONTAINER (page->top_scrolled_window()),
                        GTK_WIDGET(rootView->iconView()));
                gtk_widget_show (GTK_WIDGET(rootView->iconView()));
                while (gtk_events_pending())gtk_main_iteration();
                if (baseView){
              //      g_object_unref(G_OBJECT(baseView->iconView()));
                //    delete (baseView);
                }
                g_object_set_data(G_OBJECT(page->top_scrolled_window()), "baseView", 
                        (void *)rootView);
            }


/*

            if (baseView) {
                    WARN("deleting baseView...\n");
	            gtk_widget_hide (GTK_WIDGET(baseView->iconView()));
                    gtk_container_remove(GTK_CONTAINER (page->top_scrolled_window()), 
                            GTK_WIDGET(baseView->iconView()));
	            //delete(baseView);
            }
            WARN("now creating RootView\n");
            //auto baseView =  std::make_shared<BaseView>(double);
            //auto baseView =  new RootView<Type>("xffm:root");
            auto rootView =  new BaseView<RootView<Type>>("xffm:root");
            
            g_object_set_data(G_OBJECT(page->top_scrolled_window()), "baseView", (void *)rootView);
            //g_object_set_data(G_OBJECT(page->top_scrolled_window()), "rootView", (void *)rootView);
            gtk_container_add (GTK_CONTAINER (page->top_scrolled_window()), GTK_WIDGET(rootView->iconView()));
            gtk_widget_show (GTK_WIDGET(rootView->iconView()));
	    //gtk_widget_hide (GTK_WIDGET(rootView->iconView()));
            */
	}
      

    }
#endif
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
