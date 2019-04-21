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
    
    gint dirCount_; 
    GtkIconView *iconView_;
    GtkTreeView *treeView_;
    
public:
    View(page_c *page):BaseModel<Type>(page)
    {
        iconView_ = IconView<Type>::createIconview(this);
        treeView_ = TreeView<Type>::createTreeview(this);
    }

    ~View(void){
        TRACE("View destructor.\n");
    }

    GtkTreeView *treeView(void){return treeView_;}

    GtkIconView *iconView(void){return iconView_;}
    
    gboolean loadModel(const gchar *path){
	// This sets viewType
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
        if (view->localMonitor_) {
            localMonitorList = g_list_remove(localMonitorList, (void *)view->localMonitor_->monitor());
            delete (view->localMonitor_);
            view->localMonitor_ = NULL;
        }
        if (view->fstabMonitor_) {
            delete (view->fstabMonitor_);
            view->fstabMonitor_ = NULL;
        }

        switch (view->viewType()){
            case (ROOTVIEW_TYPE):
                RootView<Type>::loadModel(view);
                view->page()->updateStatusLabel(NULL);
                break;
            case (LOCALVIEW_TYPE):
		gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), FALSE);
		while (gtk_events_pending()) gtk_main_iteration();
		if (strcmp(path, "xffm:local")==0) {
		    view->localMonitor_ = LocalView<Type>::loadModel(view, g_get_home_dir());
		} else {
		    view->localMonitor_ = LocalView<Type>::loadModel(view, path);
		}
                break;
            case (FSTAB_TYPE):
                view->fstabMonitor_ = FstabView<Type>::loadModel(view);
	        view->page()->updateStatusLabel(NULL);
                break;
            case (PKG_TYPE):
		gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), FALSE);
		while(gtk_events_pending())gtk_main_iteration();
                PkgView<Type>::loadModel(view);
	        view->page()->updateStatusLabel(NULL);
		gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), TRUE);
		while(gtk_events_pending())gtk_main_iteration();
                break;
            default:
                ERROR("ViewType %d not defined.\n", view->viewType());
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
        if (g_file_test(path, G_FILE_TEST_EXISTS)){
	    TRACE("%s is  valid path\n", path);
	    if (!g_file_test(path, G_FILE_TEST_IS_DIR)){
                // Not a directory, but valid path: activate item.
		TRACE("%s is not dir, will activate.\n", path);
                if (FstabView<Type>::isMounted(path)){
                    auto mntDir = FstabView<Type>::getMntDir(path);
                    auto retval = this->loadModel(mntDir);
                    g_free(mntDir);
                    return retval;
                }
		return LocalView<Type>::item_activated(this, treeModel, tpath, path);
	    }
	} else {
	    DBG("fm/view.hh: loadModel: file:%s does not exist\n", path);
            
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
        auto lastPath =  g_object_get_data(G_OBJECT(iconView_), "path");
        g_free(lastPath); 
        g_object_set_data(G_OBJECT(iconView_), "path", g_strdup(this->path()));
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
        switch (this->viewType()){
            case (LOCALVIEW_TYPE):
                return LocalView<Type>::isSelectable(this->treeModel(), tpath);        
                break;
            default:
                TRACE("View::selectables(): No items are selectable for viewType: %d ()\n", this->viewType());
        }
        return FALSE;
    }
  
    void 
    highlight(gdouble X, gdouble Y){
        //if (!xfdir_p) return; // avoid race condition here.
        //highlight_x = X; highlight_y = Y;
        GtkTreeIter iter;
        
        GtkTreePath *tpath = gtk_icon_view_get_path_at_pos (iconView_, X, Y); 
        if (tpath) {
            BaseSignals<Type>::highlight(tpath, this);
            //xfdir_p->tooltip(iconview_, gtk_tree_path_copy(tpath));
        }
        else BaseSignals<Type>::clear_highlights(this);
    }

private:
    gint get_dir_count(void){ return dirCount_;}


    
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
