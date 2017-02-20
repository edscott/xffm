#include <iostream>

#include "xfdir_c.hpp"
#include "view_c.hpp"

using namespace std;
static gboolean unhighlight (gpointer, gpointer, gpointer);

xfdir_c::xfdir_c(data_c *data0, const gchar *data): gtk_c(data0){
    path = g_strdup(data);
    large = FALSE;
    gint result;
    population_mutex = PTHREAD_MUTEX_INITIALIZER;
    population_cond = PTHREAD_COND_INITIALIZER;
    result = pthread_rwlock_init(&population_lock, NULL);

    if (result){
        cerr << "view_c::init(): " << strerror(result) << "\n";
        throw 1;
    }
    population_condition = 0;
    highlight_hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

}

xfdir_c::~xfdir_c(void){
    g_free(path);
    g_object_unref(treemodel);
    pthread_mutex_destroy(&population_mutex);
    pthread_cond_destroy(&population_cond);
    pthread_rwlock_destroy(&population_lock);
    g_hash_table_destroy(highlight_hash);
}

gboolean
xfdir_c::is_large(void){return large;}

gchar *
xfdir_c::make_tooltip_text (GtkTreePath *tpath ) {
    gchar *tt_text;
    GtkTreeIter iter;
    gtk_tree_model_get_iter (treemodel, &iter, tpath);
    gtk_tree_model_get (treemodel, &iter, 
            COL_TOOLTIP_TEXT, &tt_text, -1);
    
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

#if 0
void
xfdir_c::tooltip(GtkIconView *icon_view, GtkTreePath *tpath){
    fprintf(stderr, "fallback tooltip callback in xfdir_c class\n");
   /* gtk_icon_view_set_tooltip_item (icon_view,
                                GtkTooltip *tooltip,
                                tpath);*/
    gtk_tree_path_free(tpath);
#endif

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
    if (g_hash_table_lookup(highlight_hash, tree_path_string)) {
        //TRACE("%s already in hash\n", tree_path_string);
        g_free (tree_path_string);
        gtk_tree_path_free (tpath);
        return;
    }

    // Not highlighted. First clear any other item which highlight remains.
    clear_highlights();
    // Now do highlight dance. 
    g_hash_table_insert(highlight_hash, tree_path_string, GINT_TO_POINTER(1));
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
    if (!highlight_hash || g_hash_table_size(highlight_hash) == 0) return;
    g_hash_table_foreach_remove (highlight_hash, unhighlight, (void *)this);
}

gint 
xfdir_c::get_icon_column(void){ return DISPLAY_PIXBUF;}

gint 
xfdir_c::get_text_column(void){ return DISPLAY_NAME;}

void
xfdir_c::item_activated (GtkIconView *iconview, GtkTreePath *tpath, void *data)
{
    view_c *view_p = (view_c *)data;
    GtkTreeModel *tree_model = gtk_icon_view_get_model (iconview);
    GtkTreeIter iter;
    if (!gtk_tree_model_get_iter (tree_model, &iter, tpath)) return;
    
    gchar *full_path;
    gchar *ddname;
    gtk_tree_model_get (tree_model, &iter,
                          ACTUAL_NAME, &ddname,-1);
    if (strcmp(ddname, "..")==0 && strcmp(path, "/")==0){
        full_path = g_strdup("xffm:root");
    } else {
        if (g_file_test(path, G_FILE_TEST_IS_DIR)) 
            full_path = g_strconcat(path, G_DIR_SEPARATOR_S, ddname, NULL);
        else 
            full_path = g_strdup(ddname);
    }
    TRACE("dname = %s, path = %s\n", ddname, full_path);


    if (g_file_test(full_path, G_FILE_TEST_IS_DIR)){
        view_p->reload(full_path);
    } else {
        // uses mimetype, should be delegated to xfdir_local_c...
        gchar *mimetype = mime_type(full_path);
        const gchar *command = mime_command(mimetype);
        const gchar *command1 = mime_command_text(mimetype);
        const gchar *command2 = mime_command_text2(mimetype);
        view_p->get_lpterm_p()->print_error(g_strdup_printf("%s: %s\n", mimetype, command));
        view_p->get_lpterm_p()->print_error(g_strdup_printf("%s: %s\n", mimetype, command1));
        view_p->get_lpterm_p()->print_error(g_strdup_printf("%s: %s\n", mimetype, command2));
        g_free(mimetype);
        if (strncmp(full_path,"xffm:", strlen("xffm:"))==0){
            gchar *module = full_path + strlen("xffm:");
            DBG("module = \"%s\"\n", module);
            if (strcmp(module, "root")==0){
                view_p->root();
            } else if (strcmp(module, "fstab")==0){
                // load fstab root
            } 
            // other modules:
            // cifs
            // nfs
            // ecryptfs
            // sshfs
            // pacman
        }
    }
    g_free(ddname);
    g_free(full_path);
}



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
xfdir_c::get_icon_size(const gchar *name){
    if (strcmp(name, "..")==0) return GTK_ICON_SIZE_DND;
    return GTK_ICON_SIZE_DIALOG;
}

gint 
xfdir_c::get_icon_highlight_size(const gchar *name){
    return GTK_ICON_SIZE_DIALOG;
}

gboolean
xfdir_c::popup(GtkTreePath *tpath){
    GtkTreeIter iter;
    gtk_tree_model_get_iter (treemodel, &iter, tpath);
    
    gchar *name;
    gchar *actual_name;
    gtk_tree_model_get (treemodel, &iter, 
            DISPLAY_NAME, &name, 
            ACTUAL_NAME, &actual_name, 
	    -1);
    // here we do the particular xfdir popup menu method (overloaded)
    fprintf(stderr, "xfdir_c::popup: popup for %s (%s)\n", name, actual_name);
    g_free(name);
    g_free(actual_name);
    return TRUE;
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


