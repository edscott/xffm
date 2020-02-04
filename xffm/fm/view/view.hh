#ifndef XF_BASEVIEW__HH
# define XF_BASEVIEW__HH

namespace xf{
    template <class Type> class View;
    template <class Type> class LocalView;
    template <class Type> class FstabView;
    template <class Type> class RootView;
    template <class Type> class PkgView;
    template <class Type> class LocalPopUp;
    template <class Type> class FstabPopUp;
    template <class Type> class RootPopUp;
    template <class Type> class LocalModel;
    template <class Type> class FstabModel;
    template <class Type> class RootModel;
    template <class Type> class PkgModel;
}
#include "fuse/ecryptfs.hh"
#include "base/base.hh"

#include "iconview.hh"
#include "treeview.hh"


#include "root/view.hh"
#include "local/view.hh"
#include "fstab/view.hh"
#include "pkg/view.hh"




namespace xf
{

template <class Type>
class View:
    public BaseModel<Type>
{
    using util_c = Util<Type>;
    using page_c = Page<Type>;
    
    BaseMonitor<Type> *monitorObject_; // public to switch treemodel...
    
public:
    /*gint items(void){ 
        return gtk_tree_model_iter_n_children(this->treeModel(), NULL);
    }*/
    View(page_c *page):BaseModel<Type>(page)
    {
        this->iconView_ = IconView<Type>::createIconview(this);
        this->treeView_ = TreeView<Type>::createTreeview(this);
        monitorObject_ = NULL;
    }

    ~View(void){
        TRACE("View destructor.\n");
    }

    BaseMonitor<Type> *monitorObject(void){
        return monitorObject_;
    }

    void setMonitorObject(BaseMonitor<Type> *monitorObject){
        monitorObject_ = monitorObject;
    }


    void disableMonitor(void){
	if (monitorObject_) monitorObject_->setActive(FALSE);
	std::this_thread::yield();
    }

    
    gboolean loadModel(const gchar *path){
	// This sets viewType
        TRACE("loadModel(%s)\n", path);
	return loadModel(path, this);
    }
    gboolean loadModel(const gchar *path, View<Type> *view){
	// This sets viewType
        if (isTreeView){
	    // hide iconview, show treeview
	    gtk_widget_hide(GTK_WIDGET(view->page()->topScrolledWindow()));
	    gtk_widget_show(GTK_WIDGET(view->page()->treeScrolledWindow()));
	} else {
	    // hide treeview, show iconview
	    gtk_widget_hide(GTK_WIDGET(view->page()->treeScrolledWindow()));
	    gtk_widget_show(GTK_WIDGET(view->page()->topScrolledWindow()));
	}

	auto type = BaseSignals<Type>::getViewType(path);
	if (type < 0) return FALSE;

        auto wait = g_strdup_printf(_("Loading %s...%s"), path, _("Please Wait..."));
	view->page()->updateStatusLabel(wait);
	g_free(wait);
	
	while(gtk_events_pending())gtk_main_iteration();
        view->setViewType(type);
        view->setPath(path);
	view->disableMonitor();
        // stop current monitor
       /* if (view->monitorObject_) {
            delete (view->monitorObject_);
            view->monitorObject_ = NULL;
        }*/

        switch (view->viewType()){
            case (ROOTVIEW_TYPE):
                RootView<Type>::loadModel(view);
                view->page()->updateStatusLabel(NULL);
                break;
            case (LOCALVIEW_TYPE):
                // stop current monitor
                if (view->monitorObject_) {
                    delete ((LocalMonitor<Type> *)view->monitorObject_);
                    view->monitorObject_ = NULL;
                }

		gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), FALSE);
		while (gtk_events_pending()) gtk_main_iteration();
		if (strcmp(path, "xffm:local")==0) {
		    //view->monitor_ = 
                        LocalView<Type>::loadModel(view, g_get_home_dir());
		} else {
		    //view->monitor_ = 
                        LocalView<Type>::loadModel(view, path);
		}
                break;
#ifdef ENABLE_FSTAB_MODULE
            case (FSTAB_TYPE):
                // stop current monitor
                if (view->monitorObject_) {
                    delete ((FstabMonitor<Type> *)view->monitorObject_);
                    view->monitorObject_ = NULL;
                }                //view->monitor_ = 
                FstabView<Type>::loadModel(view);
	        view->page()->updateStatusLabel(NULL);
                break;
            case (EFS_TYPE):
                if (FstabView<Type>::isMounted(path + strlen("efs:/"))){
                    this->loadModel(path + strlen("efs:/"));

                } else {
                    EFS<Type>::doDialog(path + strlen("efs:/"));
                }
                break;
#endif
#ifdef ENABLE_PKG_MODULE
            case (PKG_TYPE):
		gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), FALSE);
		while(gtk_events_pending())gtk_main_iteration();
                PkgView<Type>::loadModel(view);
	        view->page()->updateStatusLabel(NULL);
		gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), TRUE);
		while(gtk_events_pending())gtk_main_iteration();
                break;
#endif
            default:
                ERROR("fm/view/view.hh::ViewType %d not defined.\n", view->viewType());
                break;
        }
    
        return TRUE;
    }

    gboolean loadModel(GtkTreeModel *treeModel, 
	    const GtkTreePath *tpath, 
	    const gchar *path)
    {
	// Here viewType must be specified before any
	// static loadModel call (viz. PkgModel)
        TRACE("generalized view: loadModel: %s\n", path);
#ifdef ENABLE_PKG_MODULE
	if (strncmp(path, "xffm:pkg", strlen("xffm:pkg"))==0){
	    this->setViewType(PKG_TYPE);
	    this->setPath(path);

	    if(strcmp(path, "xffm:pkg:search") == 0) {
		return PkgModel<Type>::loadModel(treeModel, path);
	    }
	    if(strcmp(path, "xffm:pkg")==0) {
		return PkgModel<Type>::loadModel(treeModel, "xffm:pkg");
	    }
	    TRACE("fm/view.hh: loadModel: %s item activate?\n", path);
	    return this->loadModel(path);
	}
#endif
        if (g_file_test(path, G_FILE_TEST_EXISTS)){
	    TRACE("%s is  valid path\n", path);
	    if (!g_file_test(path, G_FILE_TEST_IS_DIR)){
                // Not a directory, but valid path: activate item.
		TRACE("%s is not dir, will activate.\n", path);
#ifdef ENABLE_FSTAB_MODULE
                if (FstabView<Type>::isMounted(path)){
                    auto mntDir = FstabView<Type>::getMntDir(path);
                    auto retval = this->loadModel(mntDir);
                    g_free(mntDir);
                    return retval;
                }
#endif
		return LocalView<Type>::item_activated(this, treeModel, tpath, path);
	    }
	} else if (strcmp(path,"xffm:root")){
            if (EFS<Type>::isEFS(path))
	        DBG("fm/view.hh: loadModel: %s does not exist\n", path);
            
	}
	return this->loadModel(path);
    }

    void reloadModel(void){
        auto path = g_strdup(this->path_);
        loadModel(path);
    }

    
    // Methinks this is useless
    /*
    void setPath(const gchar *path){
        auto lastPath =  g_object_get_data(G_OBJECT(this->iconView_), "path");
        g_free(lastPath); 
        g_object_set_data(G_OBJECT(this->iconView_), "path", g_strdup(this->path()));
	BaseModel<Type>::setPath(path);
    }*/

    void selectables(void){
        switch (this->viewType()){
            case (LOCALVIEW_TYPE):
                if (isTreeView) LocalView<Type>::selectables(this->treeView());
                else LocalView<Type>::selectables(this->iconView());        
                break;
            default:
                if (isTreeView){
                    auto selection = gtk_tree_view_get_selection (this->treeView());
		    gtk_tree_selection_unselect_all (selection);
                }
                TRACE("View::selectables(): No items are selectable for viewType: %d ()\n", this->viewType());
        }
        return;
    }
 
    gboolean isSelectable(GtkTreePath *tpath){
	GtkTreeIter iter;
        if (!gtk_tree_model_get_iter(this->treeModel(), &iter, tpath)) {
            DBG("isSelectable() cannot get iter\n");
            return FALSE;
        }
	switch (this->viewType()){
            case (LOCALVIEW_TYPE):
                return LocalView<Type>::isSelectable(this->treeModel(),&iter);
                break;
            case (ROOTVIEW_TYPE):
                return RootView<Type>::isSelectable(this->treeModel(),&iter);        
                break;
            case (FSTAB_TYPE):
                return FstabView<Type>::isSelectable(this->treeModel(),&iter);        
                break;
            default:
                DBG("View::selectables(): No items are selectable for viewType: %d ()\n", this->viewType());
        }
        return FALSE;
    }
/*    gboolean isSelectable(GtkTreePath *tpath){
        switch (this->viewType()){
            case (LOCALVIEW_TYPE):
                return LocalView<Type>::isSelectable(this->treeModel(), tpath);        
                break;
            case (ROOT_TYPE):
                return RootView<Type>::isSelectable(this->treeModel(), tpath);        
                break;
            default:
                TRACE("View::selectables(): No items are selectable for viewType: %d ()\n", this->viewType());
        }
        return FALSE;
    }*/
  
    void 
    highlight(gdouble X, gdouble Y){
	if (isTreeView) return;
	GdkPixbuf *pixbuf;
        GtkTreeIter iter;
	TRACE("highlight X,Y=%lf,%lf\n", X,Y);
        GtkTreePath *tpath = gtk_icon_view_get_path_at_pos (this->iconView_, X, Y); 
	if (!tpath) {
	    // clear highlight
            if (!isTreeView) gtk_icon_view_set_drag_dest_item(this->iconView(), NULL, GTK_ICON_VIEW_DROP_BELOW);
            if (this->items() <= 260){
                BaseSignals<Type>::clear_highlights(this);
            }
	    return;
	}
	// highlight item
//            gtk_icon_view_set_drag_dest_item(this->iconView(), tpath, GTK_ICON_VIEW_DROP_INTO);
        if (this->items() > 260){
            if (!isTreeView) gtk_icon_view_set_drag_dest_item(this->iconView(), tpath, GTK_ICON_VIEW_DROP_BELOW);
        } else {
            BaseSignals<Type>::highlight(tpath, this);
        }
        return;
    }

private:


    
    gchar *
    make_tooltip_text (GtkTreePath *tpath ) {
        return g_strdup("tooltip_text not defined in treeModel_!\n");
    }

    gchar *
    get_verbatim_name (GtkTreePath *tpath ) {
        GtkTreeIter iter;
        gchar *path;
        gtk_tree_model_get_iter (this->treeModel(), &iter, tpath);
        gtk_tree_model_get (this->treeModel(), &iter, PATH, &path, -1);
        auto basename = g_path_get_basename(path);
        g_free(path);
        return basename;
    }

    GdkPixbuf *
    get_normal_pixbuf (GtkTreePath *tpath ) {
        GtkTreeIter iter;
        GdkPixbuf *pixbuf=NULL;
        gtk_tree_model_get_iter (this->treeModel(), &iter, tpath);
        gtk_tree_model_get (this->treeModel(), &iter, 
                NORMAL_PIXBUF , &pixbuf, -1);
        return pixbuf;
    }

    GdkPixbuf *
    get_tooltip_pixbuf (GtkTreePath *tpath ) {
        GtkTreeIter iter;
        GdkPixbuf *pixbuf=NULL;
        gtk_tree_model_get_iter (this->treeModel(), &iter, tpath);
        gtk_tree_model_get (this->treeModel(), &iter, 
                TOOLTIP_PIXBUF, &pixbuf, -1);
        return pixbuf;
    }

    gchar *
    get_tooltip_text (GtkTreePath *tpath ) {
        GtkTreeIter iter;
        gchar *text=NULL;
        gtk_tree_model_get_iter (this->treeModel(), &iter, tpath);
        gtk_tree_model_get (this->treeModel(), &iter, 
                TOOLTIP_TEXT, &text, -1);
        return text;
    }



    void
    set_tooltip_pixbuf (GtkTreePath *tpath, GdkPixbuf *pixbuf ) {
        GtkTreeIter iter;
        gtk_tree_model_get_iter (this->treeModel(), &iter, tpath);
        gtk_list_store_set (GTK_LIST_STORE(this->treeModel()), &iter,
                TOOLTIP_PIXBUF, pixbuf, 
            -1);

        return ;
    }


    void
    set_tooltip_text (GtkTreePath *tpath, const gchar *text ) {
        GtkTreeIter iter;
        gtk_tree_model_get_iter (this->treeModel(), &iter, tpath);
        gtk_list_store_set (GTK_LIST_STORE(this->treeModel()), &iter,
                TOOLTIP_TEXT, text, 
            -1);

        return ;
    }
    const gchar *
    get_label(void){
        return this->path();
    }

    gint 
    get_icon_highlight_size(const gchar *name){
        return GTK_ICON_SIZE_DIALOG;
    }

    
    
};
}
#endif
