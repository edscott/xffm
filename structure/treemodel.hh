#ifndef TREEMODEL_HH
#define TREEMODEL_HH

namespace xf {


enum {
    ICON,
    ICON2,
    TYPEDEF_NAME,
    PROPERTY_NAME,
    PROPERTY_VALUE,
    PROPERTY_SOURCE,
    TYPETAG_INHERITS,
    REALPATH,
    COLUMNS
};
    
template <class Type>
class TreeModel{
    GtkTreeIter fileParent;

protected:
    GtkTreeStore *treeStore_;
    GtkTreeIter propertyParent;
    GtkTreeIter typeTagParent;
    GtkTreeRowReference *referenceParent;

public:
    TreeModel(void){
	treeStore_ = gtk_tree_store_new(COLUMNS, 
		GDK_TYPE_PIXBUF, // icon in treeView display
		GDK_TYPE_PIXBUF, // icon in treeView display
		G_TYPE_STRING,   // typedef name or all-properties 
		G_TYPE_STRING,   // property name
		G_TYPE_STRING,   // property value
		G_TYPE_STRING,   // property source
		G_TYPE_STRING,   // property typetag
		G_TYPE_STRING);   // realpath (for DND)
	gtk_tree_store_append(treeStore_, &fileParent, NULL);
	gtk_tree_store_set(treeStore_, &fileParent, TYPEDEF_NAME, "Files", -1);
	auto tpath = gtk_tree_model_get_path(GTK_TREE_MODEL(treeStore_), &fileParent);
	referenceParent = gtk_tree_row_reference_new(GTK_TREE_MODEL(treeStore_), tpath);
	gtk_tree_path_free(tpath);
	//filesParent = gtk_tree_iter_copy(&fileParent);
	gtk_tree_store_append(treeStore_, &propertyParent, NULL);
	gtk_tree_store_set(treeStore_, &propertyParent, TYPEDEF_NAME, "Properties", -1);
	gtk_tree_store_append(treeStore_, &typeTagParent, NULL);
	gtk_tree_store_set(treeStore_, &typeTagParent, TYPEDEF_NAME, "TypeTags", -1);
    }
    GtkTreeStore *treeStore(){return treeStore_;}

private:
        
    static GtkTreeViewColumn * 
    mkColumn(void){
        auto column = gtk_tree_view_column_new ();
        gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
        gtk_tree_view_column_set_resizable(column, FALSE);
        gtk_tree_view_column_set_reorderable(column, TRUE);
        gtk_tree_view_column_set_spacing(column,2);
        return column;
        
    }

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

