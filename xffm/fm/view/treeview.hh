#ifndef XF_TREEVIEW__HH
# define XF_TREEVIEW__HH
namespace xf
{

template <class Type>
class TreeView {
 
public: 
    static GtkTreeView *createTreeview(View<Type> *view){
        auto treeView = GTK_TREE_VIEW(gtk_tree_view_new());
	gtk_tree_view_set_model(treeView, view->treeModel());
        g_object_set(G_OBJECT(treeView), "has-tooltip", TRUE, NULL);
        //gtk_icon_view_set_item_width (icon_view, 60);
        gtk_tree_view_set_activate_on_single_click(treeView, TRUE);
        appendColumnPixbuf(treeView, TREEVIEW_PIXBUF);
        appendColumnText(treeView, _("Name"), DISPLAY_NAME);
        appendColumnText(treeView, _("Details"), TOOLTIP_TEXT);
        appendColumnText(treeView, _("Size"), SIZE);
        appendColumnText(treeView, _("Date"), DATE);
#ifdef DEBUG_COLUMNS
        appendColumnText(treeView, _("Mimetype"), MIMETYPE);
        appendColumnText(treeView, "ICON_NAME", ICON_NAME);
#endif
        setUpSignals(view, G_OBJECT(treeView));
        auto selection = gtk_tree_view_get_selection (treeView);
        gtk_tree_selection_set_mode (selection,  GTK_SELECTION_MULTIPLE);
        gtk_tree_view_set_rubber_banding (treeView, TRUE);        
        gtk_tree_view_set_headers_visible(treeView, FALSE);
        return treeView;
    }

private:
    static void
    setUpSignals(View<Type> *view, GObject * treeView){
        g_signal_connect (treeView, "row-activated", 
            G_CALLBACK (TreeView<Type>::rowActivated), 
            (void *)view);
         g_signal_connect (treeView, "button-release-event",
             G_CALLBACK(BaseSignals<Type>::buttonRelease), 
             (void *)view);
         g_signal_connect (treeView, "button-press-event",
             G_CALLBACK(BaseSignals<Type>::buttonPress), 
             (void *)view);
        
        // source widget
        g_signal_connect (treeView, "motion-notify-event", 
            G_CALLBACK (BaseSignals<Type>::motionNotifyEvent), 
            (void *)view);

    }

    

    //////////////////////////////////   signal handlers ///////////////////////////////////////

    static void
    rowActivated (GtkTreeView     *treeView,
               GtkTreePath       *tpath,
               GtkTreeViewColumn *column,
               gpointer           data)
    {
        // XXX Do different things depending on which column has been clicked
        //     (allow rename on editable colums)
        BaseSignals<Type>::activate(tpath, data);
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////
    
    static GtkTreeViewColumn * 
    mkColumn(void){
        auto column = gtk_tree_view_column_new ();
        gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_column_set_reorderable(column, TRUE);
        gtk_tree_view_column_set_spacing(column,2);
        return column;
        
    }

public:
    static void
    appendColumnText(GtkTreeView *treeView, const gchar *title, gint columnID){
        auto column = mkColumn();
        /*g_object_set (G_OBJECT (cell), "editable",TRUE,NULL); */
        auto cell = gtk_cell_renderer_text_new();
        gtk_tree_view_column_pack_start(column, cell, FALSE);
        gtk_tree_view_column_set_attributes(column, cell, 
                        "text", columnID, 
                        NULL);
        //gtk_tree_view_column_pack_start (column, GtkCellRenderer *cell, FALSE);
        gtk_tree_view_insert_column (treeView,column,-1);
	if (title) gtk_tree_view_column_set_title(column,title);

    }

    static void
    appendColumnPixbuf(GtkTreeView *treeView, gint columnID){
        auto column = mkColumn();
        /*g_object_set (G_OBJECT (cell), "editable",TRUE,NULL); */
        auto cell = gtk_cell_renderer_pixbuf_new();
        gtk_tree_view_column_pack_start(column, cell, FALSE);
        gtk_tree_view_column_set_attributes(column, cell, 
                        "pixbuf", columnID, 
                        NULL);
        gtk_tree_view_insert_column (treeView,column,-1);

    }

};
}
#endif
