#ifndef XF_BASEVIEW__HH
# define XF_BASEVIEW__HH

#define SET_DIR(x) x|=0x01
#define IS_DIR (x&0x01)

enum
{
  DISPLAY_PIXBUF,
  NORMAL_PIXBUF,
  HIGHLIGHT_PIXBUF,
  TOOLTIP_PIXBUF,
  DISPLAY_NAME,
  ACTUAL_NAME,
  TOOLTIP_TEXT,
  ICON_NAME,
  BASIC_COLS
};

enum
{
  COL_DISPLAY_PIXBUF,
  COL_NORMAL_PIXBUF,
  COL_HIGHLIGHT_PIXBUF,
  COL_TOOLTIP_PIXBUF,
  COL_DISPLAY_NAME,
  COL_ACTUAL_NAME,
  COL_TOOLTIP_TEXT,
  COL_ICON_NAME,
  COL_TYPE,
  COL_MIMETYPE, 
  COL_MIMEFILE, 
  COL_STAT,
  COL_PREVIEW_PATH,
  COL_PREVIEW_TIME,
  COL_PREVIEW_PIXBUF,
  NUM_COLS
};

GHashTable *highlight_hash=NULL;

namespace xf
{

template <class Type>
class BaseView{
    using util_c = Util<Type>;
public:
    void init(const gchar *path){
        if (path) path = g_strdup(path);
        else path_ = NULL;
        iconView_=createIconview();
        if (!highlight_hash) highlight_hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
        g_signal_connect (this->iconView_, "item-activated", 
                ICONVIEW_CALLBACK (BaseView<double>::item_activated), (void *)this);
    /*    g_signal_connect (get_iconview(), "motion-notify-event", 
                G_CALLBACK (motion_notify_event), (void *)this);
        g_signal_connect (get_iconview(), "leave-notify-event", 
                G_CALLBACK (leave_notify_event), (void *)this);*/
    }
    BaseView(void){
        init(NULL);
    }

    BaseView(const gchar *path){
        init(path);
    }

    ~BaseView(void){
        g_free(path_); 
        g_object_unref(treemodel_);
    }

    GtkIconView *iconView(void){return iconView_;}

protected:

    gint get_dir_count(void){ return dirCount_;}

    gboolean
    set_dnd_data(GtkSelectionData * selection_data, GList *selection_list){
        WARN( "set_dnd_data() not define for this class.\n");
        return FALSE;
    }

    gboolean
    receive_dnd(const gchar *target, GtkSelectionData *data, GdkDragAction action){
        WARN("receive_dnd() not define for this class.\n");
        return FALSE;
    }

    gchar *
    make_tooltip_text (GtkTreePath *tpath ) {
        return g_strdup("tooltip_text not defined in treemodel_!\n");
    }

    gchar *
    get_verbatim_name (GtkTreePath *tpath ) {
        GtkTreeIter iter;
        gchar *verbatim_name=NULL;
        gtk_tree_model_get_iter (treemodel_, &iter, tpath);
        gtk_tree_model_get (treemodel_, &iter, 
                ACTUAL_NAME, &verbatim_name, -1);
        return verbatim_name;
    }


    GdkPixbuf *
    get_normal_pixbuf (GtkTreePath *tpath ) {
        GtkTreeIter iter;
        GdkPixbuf *pixbuf=NULL;
        gtk_tree_model_get_iter (treemodel_, &iter, tpath);
        gtk_tree_model_get (treemodel_, &iter, 
                NORMAL_PIXBUF, &pixbuf, -1);
        return pixbuf;
    }

    GdkPixbuf *
    get_tooltip_pixbuf (GtkTreePath *tpath ) {
        GtkTreeIter iter;
        GdkPixbuf *pixbuf=NULL;
        gtk_tree_model_get_iter (treemodel_, &iter, tpath);
        gtk_tree_model_get (treemodel_, &iter, 
                TOOLTIP_PIXBUF, &pixbuf, -1);
        return pixbuf;
    }

    gchar *
    get_tooltip_text (GtkTreePath *tpath ) {
        GtkTreeIter iter;
        gchar *text=NULL;
        gtk_tree_model_get_iter (treemodel_, &iter, tpath);
        gtk_tree_model_get (treemodel_, &iter, 
                TOOLTIP_TEXT, &text, -1);
        return text;
    }



    void
    set_tooltip_pixbuf (GtkTreePath *tpath, GdkPixbuf *pixbuf ) {
        GtkTreeIter iter;
        gtk_tree_model_get_iter (treemodel_, &iter, tpath);
        gtk_list_store_set (GTK_LIST_STORE(treemodel_), &iter,
                TOOLTIP_PIXBUF, pixbuf, 
            -1);

        return ;
    }


    void
    set_tooltip_text (GtkTreePath *tpath, const gchar *text ) {
        GtkTreeIter iter;
        gtk_tree_model_get_iter (treemodel_, &iter, tpath);
        gtk_list_store_set (GTK_LIST_STORE(treemodel_), &iter,
                TOOLTIP_TEXT, text, 
            -1);

        return ;
    }

    void 
    highlight_drop(GtkTreePath *tpath){
        return;
    }

    void
    highlight(GtkTreePath *tpath){
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
        gtk_tree_model_get_iter (treemodel_, &iter, tpath);
        
        GdkPixbuf *highlight_pixbuf;
        gtk_tree_model_get (treemodel_, &iter, 
                HIGHLIGHT_PIXBUF, &highlight_pixbuf, -1);
        gtk_list_store_set (GTK_LIST_STORE(treemodel_), &iter,
                DISPLAY_PIXBUF, highlight_pixbuf, 
                -1);
        return;
    }


    void
    clear_highlights(void){
        if (!this) return;
        if (!highlight_hash || g_hash_table_size(highlight_hash) == 0) return;
        g_hash_table_foreach_remove (highlight_hash, BaseView<double>::unhighlight, (void *)this);
    }

    gint 
    get_icon_column(void){ return DISPLAY_PIXBUF;}

    gint 
    get_text_column(void){ return DISPLAY_NAME;}

    const gchar *
    get_label(void){
        return get_path();
    }

    const gchar *
    get_path(void){return (const gchar *)path_;}
    GtkTreeModel *
    get_tree_model (void){return treemodel_;}


    gint 
    get_icon_highlight_size(const gchar *name){
        return GTK_ICON_SIZE_DIALOG;
    }


    gchar *
    get_window_name (void) {
        gchar *iconname;
        if(!path_) {
            iconname = util_c::utf_string (g_get_host_name());
        } else if(g_path_is_absolute(path_) &&
                g_file_test (path_, G_FILE_TEST_EXISTS)) {
            gchar *basename = g_path_get_basename (path_);
            gchar *pathname = g_strdup (path_);
            gchar *b = util_c::utf_string (basename);   // non chopped
            util_c::chop_excess (pathname);
            gchar *q = util_c::utf_string (pathname);   // non chopped

            g_free (basename);
            g_free (pathname);
            //iconname = g_strconcat (display_host, ":  ", b, " (", q, ")", NULL);
            iconname = g_strconcat (b, " (", q, ")", NULL);
            g_free (q);
            g_free (b);
        } else {
            iconname = util_c::utf_string (path_);
            util_c::chop_excess (iconname);
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
public:
    static gboolean
    unhighlight (gpointer key, gpointer value, gpointer data){
        auto baseView = (BaseView<Type> *)data;
        if (!baseView)  return FALSE;
        TRACE("unhighlight %s\n", (gchar *)key);
        GtkTreeModel *model =baseView->get_tree_model();
        if (!baseView) return FALSE;
                
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


    static void
    item_activated (GtkIconView *iconview,
                    GtkTreePath *tpath,
                    gpointer     data)
    {
        DBG("item activated\n");
        /*
        auto view = (BaseView *)data;
        xfdir_c *xfdir_p = view_p->get_xfdir_p();
        xfdir_p->item_activated(iconview, tpath, data);
        */
    }
 
#if 0
    void 
    highlight(void){
        highlight(highlight_x, highlight_y);
    }

    void 
    highlight(gdouble X, gdouble Y){
        if (!xfdir_p) return; // avoid race condition here.
        highlight_x = X;
        highlight_y = Y;
        GtkTreeIter iter;
        GtkIconView *iconview = get_iconview();
        
        GtkTreePath *tpath = gtk_icon_view_get_path_at_pos (iconview, X, Y); 
        if (tpath) {
            xfdir_p->highlight(tpath);
            //xfdir_p->tooltip(iconview, gtk_tree_path_copy(tpath));
        }
        else xfdir_p->clear_highlights();
    }
    

    static gboolean
    motion_notify_event (GtkWidget *widget,
                   GdkEvent  *ev,
                   gpointer   data)
    {
        GdkEventMotion *e = (GdkEventMotion *)ev;
        GdkEventButton  *event = (GdkEventButton  *)ev;
        view_c * view_p = (view_c *)data;
        if (!data) {
            g_warning("motion_notify_event: data cannot be NULL\n");
            return FALSE;
        }
        if (!view_p->all_set_up) return FALSE;
    //    if (!view_p->get_xfdir_p()) return FALSE;
        NOOP("motion_notify, drag mode = %d\n", view_p->get_drag_mode());
        // Are we intending to set up a DnD?
        gint mode = view_p->get_drag_mode();
        // But is there a selection for the mode?
        if (mode){
            NOOP("// Are we intending to set up a DnD? Maybe...mode = %d\n",  view_p->get_drag_mode());
            NOOP("// Are we already in drag mode? answer: %d\n", view_p->get_drag_mode()>0);

            if (mode>0) {
                return FALSE;
            }
            // Valid selection?
            GList *selection_list = gtk_icon_view_get_selected_items (view_p->get_iconview());
            if (!selection_list) {
                view_p->set_drag_mode(0);
                return FALSE;
            }

            view_p->set_selection_list(selection_list);
            NOOP("// Have we dragged outside the icon area?\n");
            if (!gtk_icon_view_get_item_at_pos (view_p->get_iconview(), e->x, e->y, NULL,NULL)) 
            {
                fprintf(stderr, "// Yeah. Let us start drag action now\n");
                // First de allow this to work as a click cancellation.
                // (if not rubberbanding)
                view_p->set_click_cancel(1);
                
                // Set up for for move||copy||link drag now
          /*      gtk_drag_source_set (GTK_WIDGET(view_p->get_iconview()),
                             (GdkModifierType)(GDK_BUTTON1_MASK), target_table,
                             NUM_TARGETS, GDK_ACTION_MOVE);  
                gtk_drag_source_set_target_list (GTK_WIDGET(view_p->get_iconview()),
                        view_p->get_target_list());*/

                GdkDragContext *context = 
                    gtk_drag_begin_with_coordinates (GTK_WIDGET(view_p->get_iconview()),
                       view_p->get_target_list(),
                       (GdkDragAction)(((gint)GDK_ACTION_MOVE)|
                       ((gint)GDK_ACTION_COPY)|
                       ((gint)GDK_ACTION_LINK)),
                       1, //drag button
                       ev,
                       e->x, e->y);
     
                if (g_list_length(selection_list) >1){
                    gtk_drag_set_icon_name (context, "edit-copy", 0, 0);
                } else {
                    xfdir_c *x = view_p->get_xfdir_p();
                    
                    GtkTreeModel *treemodel = x->get_tree_model();
                    GtkTreePath *tpath = (GtkTreePath *)selection_list->data;
                    GtkTreeIter iter;
                    gtk_tree_model_get_iter (treemodel, &iter, tpath);
                    GdkPixbuf *pixbuf;
                    // XXX  will this add a ref to pixbuf? nah!
                    gtk_tree_model_get (treemodel, &iter, 
                        NORMAL_PIXBUF, &pixbuf, -1);       
                    gtk_drag_set_icon_pixbuf (context, pixbuf, 0, 0);
                }
                view_p->set_drag_mode(1);
            }
        }
                                     



        if (view_p->get_dir_count() > 500) return FALSE;
        view_p->highlight(e->x, e->y);
        return FALSE;
    }

    static gboolean
    leave_notify_event (GtkWidget *widget,
                   GdkEvent  *event,
                   gpointer   data){
        if (!data) {
            g_warning("leave_notify_event: data cannot be NULL\n");
            return FALSE;
        }
        view_c * view_p = (view_c *)data;
        if (!view_p->all_set_up) return FALSE;

        //fprintf(TRACE("leave_notify_event\n");
        view_p->get_xfdir_p()->clear_highlights();
        window_c *window_p = (window_c *)view_p->get_window_v();
        window_p->set_tt_window(NULL, NULL);
        return FALSE;
    }
#endif
protected:

    gchar *path_;
    GtkTreeModel *treemodel_;
    gint dirCount_; 
    GtkIconView *iconView_;

private:
    GtkIconView *createIconview(void){
        auto icon_view = GTK_ICON_VIEW(gtk_icon_view_new());
        g_object_set(G_OBJECT(icon_view), "has-tooltip", TRUE, NULL);
        gtk_icon_view_set_item_width (icon_view, 60);
        gtk_icon_view_set_activate_on_single_click(icon_view, TRUE);
        return icon_view;
    }

};
}
#endif
