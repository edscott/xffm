#ifndef XF_BASEVIEW__HH
# define XF_BASEVIEW__HH

#include "basemodel.hh"
#include "baseviewsignals.hh"

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
    
public:
private:


public:
    gboolean loadModel(const gchar *path){
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
		DBG("%s is not dir\n", path);
		return LocalView<Type>::item_activated(this, treeModel, tpath, path);
	    }
	} else {
	    DBG("%s is not valid path\n", path);
        }
	return this->loadModel(path);
    }

    
    BaseView(page_c *page):BaseModel<Type>(page)
    {
    //BaseView(page_c *page, const gchar *path){
        iconView_=createIconview();

	
	g_object_set_data(G_OBJECT(this->treeModel()), "iconview", iconView_);
	gtk_icon_view_set_model(iconView_, this->treeModel());

	gtk_icon_view_set_text_column (iconView_, DISPLAY_NAME);
	gtk_icon_view_set_pixbuf_column (iconView_,  DISPLAY_PIXBUF);
	gtk_icon_view_set_selection_mode (iconView_, GTK_SELECTION_SINGLE);


        g_signal_connect (this->iconView_, "item-activated", 
            ICONVIEW_CALLBACK (BaseViewSignals<Type>::item_activated), (void *)this);
        g_signal_connect (this->iconView_, "motion-notify-event", 
                ICONVIEW_CALLBACK (BaseViewSignals<Type>::motion_notify_event), (void *)this);
         //g_signal_connect (this->iconView_, "query-tooltip", 
           //     G_CALLBACK (query_tooltip_f), (void *)this);
 

         g_signal_connect (this->iconView_, "button-release-event",
                G_CALLBACK(BaseViewSignals<Type>::button_release_f), (void *)this);
         g_signal_connect (this->iconView_, "button-press-event",
                G_CALLBACK(BaseViewSignals<Type>::button_press_f), (void *)this);

        // source widget
        g_signal_connect (G_OBJECT (this->source()), 
                "drag-begin", DRAG_CALLBACK (BaseViewSignals<Type>::signal_drag_begin), (void *)this);
        g_signal_connect (G_OBJECT (this->source()), 
                "drag-data-get", G_CALLBACK (BaseViewSignals<Type>::signal_drag_data_send), (void *)this);
      
        // destination widget
        g_signal_connect (G_OBJECT (this->destination()), 
                "drag-data-received", G_CALLBACK (BaseViewSignals<Type>::signal_drag_data_receive), (void *)this);

        // Not necessary with GTK_DEST_DEFAULT_MOTION
	// while using default iconview dnd, but this is not
	// our case.
        g_signal_connect (G_OBJECT (this->destination()), 
              "drag-motion", 
	      G_CALLBACK (BaseViewSignals<Type>::signal_drag_motion),
	      (void *)this);
        // Not necessary with GTK_DEST_DEFAULT_DROP
        //
        //g_signal_connect (G_OBJECT (this->destination()), 
        //        "drag-drop", G_CALLBACK (BaseViewSignals<Type>::signal_drag_drop), (void *)this);
        
         // Why not "clicked" signal? 
         // Because this is to filter cancelled dnd event from
         // actual activated events.
         g_signal_connect (this->iconView_, "button-release-event",
                G_CALLBACK(button_click_f), (void *)this);

            
    }

    ~BaseView(void){
        TRACE("BaseView destructor.\n");
	

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

    

    GtkIconView *iconView(void){return iconView_;}
   
    void 
    highlight(gdouble X, gdouble Y){
        //if (!xfdir_p) return; // avoid race condition here.
        //highlight_x = X; highlight_y = Y;
        GtkTreeIter iter;
        
        GtkTreePath *tpath = gtk_icon_view_get_path_at_pos (iconView_, X, Y); 
        if (tpath) {
            BaseModel<Type>::highlight(tpath, this);
            //xfdir_p->tooltip(iconview_, gtk_tree_path_copy(tpath));
        }
        else BaseModel<Type>::clear_highlights(this);
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

 
    static GtkIconView *createIconview(void){
        auto icon_view = GTK_ICON_VIEW(gtk_icon_view_new());
        g_object_set(G_OBJECT(icon_view), "has-tooltip", TRUE, NULL);
        gtk_icon_view_set_item_width (icon_view, 60);
        gtk_icon_view_set_activate_on_single_click(icon_view, TRUE);
        return icon_view;
    }

    static gboolean
    button_click_f (GtkWidget *widget,
                   GdkEventButton  *event,
                   gpointer   data)
    {
	//auto baseView = (BaseView<Type> *)data;
	WARN("no action on button_click_f\n");
        return FALSE;
    }
    
    
};
}
#endif
