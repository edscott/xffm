#include <iostream>

#include "xfdir_c.hpp"
#include "view_c.hpp"

using namespace std;
static gboolean unhighlight (gpointer, gpointer, gpointer);

xfdir_c::xfdir_c(data_c *data0, const gchar *data): menu_c(data0){
    path = g_strdup(data);
    data_p = data0;
    gint result;

    if (result){
        cerr << "view_c::init(): " << strerror(result) << "\n";
        throw 1;
    }

}

xfdir_c::~xfdir_c(void){
    g_free(path);
    g_object_unref(treemodel);
}

gboolean
xfdir_c::set_dnd_data(GtkSelectionData * selection_data, GList *selection_list){
    fprintf(stderr, "set_dnd_data() not define for this class.\n");
    return FALSE;
}

gboolean
xfdir_c::receive_dnd(const gchar *target, GtkSelectionData *data, GdkDragAction action){
    fprintf(stderr, "receive_dnd() not define for this class.\n");
    return FALSE;
}

gchar *
xfdir_c::make_tooltip_text (GtkTreePath *tpath ) {
/*    gchar *tt_text;
    GtkTreeIter iter;
    gtk_tree_model_get_iter (treemodel, &iter, tpath);
    gtk_tree_model_get (treemodel, &iter, 
            COL_TOOLTIP_TEXT, &tt_text, -1);*/
    
    return g_strdup("xfdir_c::tooltip_text not defined in treemodel!\n");
}

gchar *
xfdir_c::get_verbatim_name (GtkTreePath *tpath ) {
    GtkTreeIter iter;
    gchar *verbatim_name=NULL;
    gtk_tree_model_get_iter (treemodel, &iter, tpath);
    gtk_tree_model_get (treemodel, &iter, 
            ACTUAL_NAME, &verbatim_name, -1);
    return verbatim_name;
}


GdkPixbuf *
xfdir_c::get_normal_pixbuf (GtkTreePath *tpath ) {
    GtkTreeIter iter;
    GdkPixbuf *pixbuf=NULL;
    gtk_tree_model_get_iter (treemodel, &iter, tpath);
    gtk_tree_model_get (treemodel, &iter, 
            NORMAL_PIXBUF, &pixbuf, -1);
    return pixbuf;
}

GdkPixbuf *
xfdir_c::get_tooltip_pixbuf (GtkTreePath *tpath ) {
    GtkTreeIter iter;
    GdkPixbuf *pixbuf=NULL;
    gtk_tree_model_get_iter (treemodel, &iter, tpath);
    gtk_tree_model_get (treemodel, &iter, 
            TOOLTIP_PIXBUF, &pixbuf, -1);
    return pixbuf;
}

gchar *
xfdir_c::get_tooltip_text (GtkTreePath *tpath ) {
    GtkTreeIter iter;
    gchar *text=NULL;
    gtk_tree_model_get_iter (treemodel, &iter, tpath);
    gtk_tree_model_get (treemodel, &iter, 
            TOOLTIP_TEXT, &text, -1);
    return text;
}



void
xfdir_c::set_tooltip_pixbuf (GtkTreePath *tpath, GdkPixbuf *pixbuf ) {
    GtkTreeIter iter;
    gtk_tree_model_get_iter (treemodel, &iter, tpath);
    gtk_list_store_set (GTK_LIST_STORE(treemodel), &iter,
            TOOLTIP_PIXBUF, pixbuf, 
        -1);

    return ;
}


void
xfdir_c::set_tooltip_text (GtkTreePath *tpath, const gchar *text ) {
    GtkTreeIter iter;
    gtk_tree_model_get_iter (treemodel, &iter, tpath);
    gtk_list_store_set (GTK_LIST_STORE(treemodel), &iter,
            TOOLTIP_TEXT, text, 
        -1);

    return ;
}

void 
xfdir_c::highlight_drop(GtkTreePath *tpath){
    return;
}

void
xfdir_c::set_show_hidden(gboolean state){shows_hidden = state;}

void
xfdir_c::highlight(GtkTreePath *tpath){
        //TRACE("highlight %d, %d\n", highlight_x, highlight_y);
    gchar *tree_path_string = NULL;
    
    if (tpath == NULL){
        // No item at position?
        // Do we need to clear hash table?
        clear_highlights();
        return;
    }

    // Already highlighted?
    tree_path_string = gtk_tree_path_to_string (tpath);
    if (g_hash_table_lookup(data_p->highlight_hash, tree_path_string)) {
        //TRACE("%s already in hash\n", tree_path_string);
        g_free (tree_path_string);
        gtk_tree_path_free (tpath);
        return;
    }

    // Not highlighted. First clear any other item which highlight remains.
    clear_highlights();
    // Now do highlight dance. 
    g_hash_table_insert(data_p->highlight_hash, tree_path_string, GINT_TO_POINTER(1));
    GtkTreeIter iter;
    gtk_tree_model_get_iter (treemodel, &iter, tpath);
    
    GdkPixbuf *highlight_pixbuf;
    gtk_tree_model_get (treemodel, &iter, 
            HIGHLIGHT_PIXBUF, &highlight_pixbuf, -1);
    gtk_list_store_set (GTK_LIST_STORE(treemodel), &iter,
            DISPLAY_PIXBUF, highlight_pixbuf, 
            -1);
    return;
}


void
xfdir_c::clear_highlights(void){
    if (!this) return;
    if (!data_p->highlight_hash || g_hash_table_size(data_p->highlight_hash) == 0) return;
    g_hash_table_foreach_remove (data_p->highlight_hash, unhighlight, (void *)this);
}

gint 
xfdir_c::get_icon_column(void){ return DISPLAY_PIXBUF;}

gint 
xfdir_c::get_text_column(void){ return DISPLAY_NAME;}

const gchar *
xfdir_c::get_label(void){
    return get_path();
}

const gchar *
xfdir_c::get_path(void){return (const gchar *)path;}
GtkTreeModel *
xfdir_c::get_tree_model (void){return treemodel;}

gint
xfdir_c::get_dir_count(void){ return dir_count;}


gint 
xfdir_c::get_icon_highlight_size(const gchar *name){
    return GTK_ICON_SIZE_DIALOG;
}


gchar *
xfdir_c::get_window_name (void) {
    gchar *iconname;
    if(!path) {
        iconname = utf_string (g_get_host_name());
    } else if(g_path_is_absolute(path) &&
	    g_file_test (path, G_FILE_TEST_EXISTS)) {
        gchar *basename = g_path_get_basename (path);
        gchar *pathname = g_strdup (path);
        gchar *b = utf_string (basename);   // non chopped
        chop_excess (pathname);
        gchar *q = utf_string (pathname);   // non chopped

        g_free (basename);
        g_free (pathname);
	//iconname = g_strconcat (display_host, ":  ", b, " (", q, ")", NULL);
	iconname = g_strconcat (b, " (", q, ")", NULL);
        g_free (q);
        g_free (b);
    } else {
        iconname = utf_string (path);
        chop_excess (iconname);
    }

#ifdef DEBUG
    gchar *gg = g_strdup_printf("%s-%d-D", iconname, getpid());
    g_free(iconname);
    iconname = gg;
#else
#ifdef CORE
    gchar *gg = g_strdup_printf("%s-%d-C", iconname, getpid());
    g_free(iconname);
    iconname = gg;
#endif
#endif
    return (iconname);
}

/////////////////////////////////////////////////////////////////////////////////


static gboolean
unhighlight (gpointer key, gpointer value, gpointer data){
    xfdir_c *xfdir_p = (xfdir_c *)data;
    TRACE("unhighlight %s\n", (gchar *)key);
    GtkTreeModel *model = xfdir_p->get_tree_model();
            
    GtkTreePath *tpath = gtk_tree_path_new_from_string ((gchar *)key);
    if (!tpath) return FALSE;

    GtkTreeIter iter;
    gtk_tree_model_get_iter (model, &iter, tpath);
    gtk_tree_path_free (tpath);
    GdkPixbuf *normal_pixbuf;
    gtk_tree_model_get (model, &iter, 
            NORMAL_PIXBUF, &normal_pixbuf, -1);
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
            DISPLAY_PIXBUF, normal_pixbuf, 
        -1);

    return TRUE;
}


