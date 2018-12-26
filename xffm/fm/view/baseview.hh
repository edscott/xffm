#ifndef XF_BASEVIEW__HH
# define XF_BASEVIEW__HH

#include "fm/model/base/basemodel.hh"
#include "fm/model/base/basesignals.hh"

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
    
    gint dirCount_; 
    GtkIconView *iconView_;
    GtkTreeView *treeView_;
    
public:
    BaseView(page_c *page):BaseModel<Type>(page)
    {
        iconView_ = IconView<Type>::createIconview(this);
        treeView_ = TreeView<Type>::createTreeview(this);
    }

    ~BaseView(void){
        TRACE("BaseView destructor.\n");
    }

    GtkTreeView *treeView(void){return treeView_;}

    GtkIconView *iconView(void){return iconView_;}
    
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

        this->setViewType(BaseSignals<Type>::getViewType(path));
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
                LocalView<Type>::selectables(this->iconView());        
                break;
            default:
                DBG("BaseView::selectables(): All icons are selectable for viewType: %d\n",
                        this->viewType());
        }
        return;
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
        gchar *verbatim_name=NULL;
        gtk_tree_model_get_iter (this->treeModel(), &iter, tpath);
        gtk_tree_model_get (this->treeModel(), &iter, 
                ACTUAL_NAME, &verbatim_name, -1);
        return verbatim_name;
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
