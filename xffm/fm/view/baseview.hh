#ifndef XF_BASEVIEW__HH
# define XF_BASEVIEW__HH

#include "fm/model/base/basemodel.hh"

#include "fm/view/iconview.hh"
#include "fm/view/treeview.hh"

namespace xf
{

template <class Type>
class BaseView:
    public BaseModel<Type>
{
    using util_c = Util<Type>;
    using page_c = Page<Type>;
    
    
public:
    BaseView(page_c *page):BaseModel<Type>(page)
    {
    }

    ~BaseView(void){
        TRACE("BaseView destructor.\n");
    }
    
    gboolean loadModel(const gchar *path){

        if (isTreeView){
	    // hide iconview, show treeview
	    gtk_widget_hide(GTK_WIDGET(this->page()->topScrolledWindow()));
	    gtk_widget_show(GTK_WIDGET(this->page()->treeScrolledWindow()));
	} else {
	    // hide treeview, show iconview
	    gtk_widget_hide(GTK_WIDGET(this->page()->treeScrolledWindow()));
	    gtk_widget_show(GTK_WIDGET(this->page()->topScrolledWindow()));
	}

        this->setViewType(this->getViewType(path));
        this->setPath(path);
        // stop current monitor
        if (this->localMonitor_) {
            localMonitorList = g_list_remove(localMonitorList, (void *)this->localMonitor_->monitor());
            delete (this->localMonitor_);
            this->localMonitor_ = NULL;
        }
        if (this->fstabMonitor_) {
            delete (this->fstabMonitor_);
            this->fstabMonitor_ = NULL;
        }

        switch (this->viewType()){
            case (ROOTVIEW_TYPE):
                RootView<Type>::loadModel(this);
                this->page()->updateStatusLabel(NULL);
                break;
            case (LOCALVIEW_TYPE):
		if (strcmp(path, "xffm:local")==0) {
		    this->localMonitor_ = LocalView<Type>::loadModel(this, g_get_home_dir());
		} else {
		    this->localMonitor_ = LocalView<Type>::loadModel(this, path);
		}
                break;
            case (FSTAB_TYPE):
                this->fstabMonitor_ = Fstab<Type>::loadModel(this);
	        this->page()->updateStatusLabel(NULL);
                break;
            default:
                ERROR("ViewType %d not defined.\n", this->viewType());
                break;
        }
    
        return TRUE;
    }

    gboolean loadModel(GtkTreeModel *treeModel, const GtkTreePath *tpath, 
	    const gchar *path){
        if (g_file_test(path, G_FILE_TEST_EXISTS)){
	    TRACE("%s is  valid path\n", path);
	    if (!g_file_test(path, G_FILE_TEST_IS_DIR)){
                // Not a directory, but valid path: activate item.
		DBG("%s is not dir, will activate.\n", path);
                if (Fstab<Type>::isMounted(path)){
                    auto mntDir = Fstab<Type>::getMntDir(path);
                    auto retval = this->loadModel(mntDir);
                    g_free(mntDir);
                    return retval;
                }
		return LocalView<Type>::item_activated(this, treeModel, tpath, path);
	    }
	} else {
	    DBG("%s is not valid path\n", path);
        }
	return this->loadModel(path);
    }

    void reloadModel(void){
        auto path = g_strdup(this->path_);
        loadModel(path);
    }


    
};
}
#endif
