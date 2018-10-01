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

// FIXME: this enum goes into localview class template
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
//FIXME: extended enums will depend on class template using them
//       all must be compatible with BASIC_COLS, for basicview
//       callbacks..
enum
{
  COL_xxDISPLAY_PIXBUF,
  COL_xxNORMAL_PIXBUF,
  COL_xxHIGHLIGHT_PIXBUF,
  COL_xxTOOLTIP_PIXBUF,
  COL_xxDISPLAY_NAME,
  COL_xxACTUAL_NAME,
  COL_xxTOOLTIP_TEXT,
  COL_xxICON_NAME,
  COL_xxTYPE,
  COL_xxMIMETYPE, 
  COL_xxMIMEFILE, 
  COL_xxSTAT,
  COL_xxPREVIEW_PATH,
  COL_xxPREVIEW_TIME,
  COL_xxPREVIEW_PIXBUF,
  NUM_xxCOLS
};

GHashTable *highlight_hash=NULL;
#define CONTROL_MODE (event->state & GDK_CONTROL_MASK)
#define SHIFT_MODE (event->state & GDK_SHIFT_MASK)
enum {
    TARGET_URI_LIST,
    TARGET_PLAIN,
    TARGET_UTF8,
    TARGET_STRING,
    TARGET_ROOTWIN,
    TARGET_MOZ_URL,
    TARGET_XDS,
    TARGET_RAW,
    TARGETS
};

static GtkTargetEntry targetTable[] = {
    {(gchar *)"text/uri-list", 0, TARGET_URI_LIST},
    {(gchar *)"text/x-moz-url", 0, TARGET_MOZ_URL},
    {(gchar *)"text/plain", 0, TARGET_PLAIN},
    {(gchar *)"UTF8_STRING", 0, TARGET_UTF8},
    {(gchar *)"STRING", 0, TARGET_STRING}
};

#define NUM_TARGETS (sizeof(targetTable)/sizeof(GtkTargetEntry))

namespace xf
{

template <class Type>
class BaseView{
    using util_c = Util<Type>;
    gint dragMode_;
    gint clickCancel_;
    GList *selectionList_;
    GtkTargetList *targetList_;
public:
    void init(const gchar *path){
        if (path) path = g_strdup(path);
        else path_ = NULL;
        dragMode_ = 0;
        clickCancel_ = 0;
        selectionList_ = NULL;
        
        iconView_=createIconview();
        createTargetList();
	
	treeModel_ = Type::mkTreeModel(path);
	g_object_set_data(G_OBJECT(treeModel_), "iconview", iconView_);
	gtk_icon_view_set_model(iconView_, treeModel_);

	gtk_icon_view_set_text_column (iconView_, textColumn());
	gtk_icon_view_set_pixbuf_column (iconView_,  iconColumn());
	gtk_icon_view_set_selection_mode (iconView_, GTK_SELECTION_SINGLE);


        if (!highlight_hash) highlight_hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
        g_signal_connect (this->iconView_, "item-activated", 
                ICONVIEW_CALLBACK (BaseView<Type>::item_activated), (void *)this);
        g_signal_connect (this->iconView_, "motion-notify-event", 
                ICONVIEW_CALLBACK (BaseView<Type>::motion_notify_event), (void *)this);
     //g_signal_connect (this->iconView_, "query-tooltip", 
       //     G_CALLBACK (query_tooltip_f), (void *)this);

     // Why not "clicked" signal? 
     // Because this is to filter cancelled dnd event from
     // actual activated events.
     g_signal_connect (this->iconView_, "button-release-event",
	    G_CALLBACK(button_click_f), (void *)this);
     g_signal_connect (this->iconView_, "button-release-event",
	    G_CALLBACK(button_release_f), (void *)this);

     g_signal_connect (this->iconView_, "button-press-event",
	    G_CALLBACK(button_press_f), (void *)this);

    g_signal_connect (G_OBJECT (this->iconView_), 
	    "drag-data-received", G_CALLBACK (signal_drag_data), (void *)this);
    g_signal_connect (G_OBJECT (this->iconView_), 
	    "drag-data-get", G_CALLBACK (signal_drag_data_get), (void *)this);
    g_signal_connect (G_OBJECT (this->iconView_), 
	    "drag-motion", G_CALLBACK (signal_drag_motion), (void *)this);
    g_signal_connect (G_OBJECT (this->iconView_), 
	    "drag-leave", G_CALLBACK (signal_drag_leave), (void *)this);
    g_signal_connect (G_OBJECT (this->iconView_), 
	    "drag-data-delete", G_CALLBACK (signal_drag_delete), (void *)this);

    g_signal_connect (G_OBJECT (this->iconView_), 
	    "drag-end", DRAG_CALLBACK (signal_drag_end), (void *)this);
    g_signal_connect (G_OBJECT (this->iconView_), 
	    "drag-begin", DRAG_CALLBACK (signal_drag_begin), (void *)this);
        
    }
    BaseView(void){
        init(NULL);
    }

    BaseView(const gchar *path){
        init(path);
    }

    ~BaseView(void){
        g_free(path_); 
        g_object_unref(treeModel_);
    }

    GtkIconView *iconView(void){return iconView_;}
    GtkTreeModel *treeModel(void){return treeModel_;}
    GtkTargetList *getTargetList(void){return targetList_;}

protected:

    gint get_dir_count(void){ return dirCount_;}


    gboolean
    setDndData(GtkSelectionData * selection_data, GList *selection_list){
        WARN( "set_dnd_data() baseview default.\n");
        GtkTreeModel *treemodel = this->treeModel_;
        GList *tmp;
        const gchar *format = "file://";
        gint selection_len = 0;
        /* count length of bytes to be allocated */
        for(tmp = selection_list; tmp; tmp = tmp->next) {
            GtkTreePath *tpath = (GtkTreePath *)tmp->data;
            gchar *g;
            GtkTreeIter iter;
            gtk_tree_model_get_iter (treemodel, &iter, tpath);
            gtk_tree_model_get (treemodel, &iter, 
                ACTUAL_NAME, &g, -1);
            // 2 is added for the \r\n 
            selection_len += (strlen (g) + strlen (format) + 2);
            g_free(g);
           /* 
            gchar *dndpath = g_build_filename(view_path, g, NULL);
            g_free(g);
            // 2 is added for the \r\n 
            selection_len += (strlen (dndpath) + strlen (format) + 2);
            g_free(dndpath);
            */
        }
        /* 1 is added for terminating null character */
        fprintf(stderr, "allocating %d bytes for dnd data\n",selection_len + 1);
        //files = (gchar *)calloc (selection_len + 1,1);
        /*if (!files) {
            g_error("signal_drag_data_get(): malloc %s", strerror(errno));
            return;
        }*/
        gchar *files = g_strdup("");
        for(tmp = selection_list; tmp; tmp = tmp->next) {
            GtkTreePath *tpath = (GtkTreePath *)tmp->data;
            gchar *g;
            GtkTreeIter iter;
            gtk_tree_model_get_iter (treemodel, &iter, tpath);
            gtk_tree_model_get (treemodel, &iter, 
                ACTUAL_NAME, &g, -1); 

            gchar *gg=g_strconcat(files,format,g,"\n", NULL);
            g_free(g);
            g_free(files);
            files=gg;
/*
            gchar *dndpath = g_build_filename(view_path, g, NULL);
            g_free(g);
            g=g_strconcat(files,format,dndpath,"\n", NULL);
            g_free(files);
            files=g;
*/
            /*sprintf (files, "%s%s\r\n", format, dndpath);
            files += (strlen (format) + strlen (dndpath) + 2);
            g_free(dndpath);*/
        }
        gtk_selection_data_set (selection_data, 
            gtk_selection_data_get_selection(selection_data),
            8, (const guchar *)files, selection_len);
        DBG(">>> DND send, drag data is:\n%s\n", files);
        g_free(files);
        return TRUE;
    }

    gboolean
    receive_dnd(const gchar *target, GtkSelectionData *data, GdkDragAction action){
        WARN("receive_dnd() not define for this class.\n");
        return FALSE;
    }
    
    gchar *
    make_tooltip_text (GtkTreePath *tpath ) {
        return g_strdup("tooltip_text not defined in treeModel_!\n");
    }

    gchar *
    get_verbatim_name (GtkTreePath *tpath ) {
        GtkTreeIter iter;
        gchar *verbatim_name=NULL;
        gtk_tree_model_get_iter (treeModel_, &iter, tpath);
        gtk_tree_model_get (treeModel_, &iter, 
                ACTUAL_NAME, &verbatim_name, -1);
        return verbatim_name;
    }


    GdkPixbuf *
    get_normal_pixbuf (GtkTreePath *tpath ) {
        GtkTreeIter iter;
        GdkPixbuf *pixbuf=NULL;
        gtk_tree_model_get_iter (treeModel_, &iter, tpath);
        gtk_tree_model_get (treeModel_, &iter, 
                NORMAL_PIXBUF, &pixbuf, -1);
        return pixbuf;
    }

    GdkPixbuf *
    get_tooltip_pixbuf (GtkTreePath *tpath ) {
        GtkTreeIter iter;
        GdkPixbuf *pixbuf=NULL;
        gtk_tree_model_get_iter (treeModel_, &iter, tpath);
        gtk_tree_model_get (treeModel_, &iter, 
                TOOLTIP_PIXBUF, &pixbuf, -1);
        return pixbuf;
    }

    gchar *
    get_tooltip_text (GtkTreePath *tpath ) {
        GtkTreeIter iter;
        gchar *text=NULL;
        gtk_tree_model_get_iter (treeModel_, &iter, tpath);
        gtk_tree_model_get (treeModel_, &iter, 
                TOOLTIP_TEXT, &text, -1);
        return text;
    }



    void
    set_tooltip_pixbuf (GtkTreePath *tpath, GdkPixbuf *pixbuf ) {
        GtkTreeIter iter;
        gtk_tree_model_get_iter (treeModel_, &iter, tpath);
        gtk_list_store_set (GTK_LIST_STORE(treeModel_), &iter,
                TOOLTIP_PIXBUF, pixbuf, 
            -1);

        return ;
    }


    void
    set_tooltip_text (GtkTreePath *tpath, const gchar *text ) {
        GtkTreeIter iter;
        gtk_tree_model_get_iter (treeModel_, &iter, tpath);
        gtk_list_store_set (GTK_LIST_STORE(treeModel_), &iter,
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
        gtk_tree_model_get_iter (treeModel_, &iter, tpath);
        
        GdkPixbuf *highlight_pixbuf;
        gtk_tree_model_get (treeModel_, &iter, 
                HIGHLIGHT_PIXBUF, &highlight_pixbuf, -1);
        gtk_list_store_set (GTK_LIST_STORE(treeModel_), &iter,
                DISPLAY_PIXBUF, highlight_pixbuf, 
                -1);
        return;
    }


    const gchar *
    get_label(void){
        return get_path();
    }

    const gchar *
    get_path(void){return (const gchar *)path_;}
    GtkTreeModel *
    get_tree_model (void){return treeModel_;}


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

 
   /* void 
    highlight(void){
        highlight(highlight_x, highlight_y);
    }*/

    void 
    highlight(gdouble X, gdouble Y){
        //if (!xfdir_p) return; // avoid race condition here.
        //highlight_x = X; highlight_y = Y;
        GtkTreeIter iter;
        
        GtkTreePath *tpath = gtk_icon_view_get_path_at_pos (iconView_, X, Y); 
        if (tpath) {
            highlight(tpath);
            //xfdir_p->tooltip(iconview_, gtk_tree_path_copy(tpath));
        }
        else clear_highlights();
    }

    void
    clear_highlights(void){
        if (!highlight_hash || g_hash_table_size(highlight_hash) == 0) return;
        g_hash_table_foreach_remove (highlight_hash, BaseView<double>::unhighlight, (void *)this);
    }

    void setClickCancel(gint state){ 
        clickCancel_ = state;
        /*if (state <= 0) {
            click_cancel = state;
            return;
        }
        if (click_cancel == 0) click_cancel = state;*/
    }

    gboolean 
    clickCancel(void){
        if (clickCancel_ <= 0) return FALSE;
        return TRUE;
    }


    void
    createTargetList (void) {
        DBG("create_target_list..\n");
        //if(target_list) return;
        targetList_ = gtk_target_list_new (targetTable, NUM_TARGETS);
        // The default dnd action: move.
        gtk_icon_view_enable_model_drag_dest (iconView_,
                                          targetTable, 
                                          NUM_TARGETS,
                                          (GdkDragAction)
                                    ((gint)GDK_ACTION_MOVE|
                                     (gint)GDK_ACTION_COPY|
                                     (gint)GDK_ACTION_LINK));
        gtk_icon_view_enable_model_drag_source
                                   (iconView_,
                                    (GdkModifierType)
                                    0,
                            //	((gint)GDK_SHIFT_MASK|(gint)GDK_CONTROL_MASK),
                                    //GdkModifierType start_button_mask,
                                    targetTable,
                                    NUM_TARGETS,
                                    (GdkDragAction)
                                    ((gint)GDK_ACTION_MOVE|
                                     (gint)GDK_ACTION_COPY|
                                     (gint)GDK_ACTION_LINK));
        return;
    }

    void setDragMode(gboolean state){ dragMode_ = state;};
    gint dragMode(void){return dragMode_;}

    void
    setSelectionList(GList *list){
        if (selectionList_) freeSelectionList();
        selectionList_ = list;
    }

    GList *
    selectionList(void){return selectionList_;}

    void
    freeSelectionList(void){
        if (selectionList_) 
            g_list_free_full (selectionList_, (GDestroyNotify) gtk_tree_path_free);
        selectionList_ = NULL;
    }

    
    static gboolean
    button_release_f (GtkWidget *widget,
                   GdkEventButton  *event,
                   gpointer   data)
    {
        //GdkEventButton *event_button = (GdkEventButton *)event;
	auto baseView = (BaseView<Type> *)data;
        baseView->setDragMode(0);
        if (!gtk_icon_view_get_item_at_pos (baseView->iconView(),
                                   event->x, event->y,
                                   NULL,NULL)){
            // What here?
        }
        return FALSE;
    }

    static gboolean
    button_click_f (GtkWidget *widget,
                   GdkEventButton  *event,
                   gpointer   data)
    {
	auto baseView = (BaseView<Type> *)data;
        if (baseView->clickCancel()) return TRUE;
        return FALSE;
    }



    static gboolean
    button_press_f (GtkWidget *widget,
                   GdkEventButton  *event,
                   gpointer   data)
    {
        auto baseView = (BaseView<Type> *)data;

        if (event->button == 1) {
            gboolean retval = FALSE;
            //GList *selection_list = gtk_icon_view_get_selected_items (baseView->iconView());
            gint mode = 0;
            GtkTreePath *tpath;
            if (gtk_icon_view_get_item_at_pos (baseView->iconView(),
                                   event->x, event->y,
                                   &tpath,NULL)) {
                
                if (CONTROL_MODE && SHIFT_MODE) mode = -3; // link
                else if (CONTROL_MODE) mode = -2; // copy
                else if (SHIFT_MODE) mode = -1; // move
                else mode = -1; // default (move)
                baseView->setClickCancel(0);
            } else { 
                tpath=NULL;
                baseView->setClickCancel(-1);
            }
            DBG("button press %d mode %d\n", event->button, mode);
            baseView->setDragMode(mode);
            if (CONTROL_MODE){
                // select item
                gtk_icon_view_select_path (baseView->iconView(), tpath);
                retval = TRUE; 
            }
            if (tpath) gtk_tree_path_free(tpath);
            return retval;
        }

        // long press or button 3 should do popup menu...
        if (event->button != 3) return FALSE;
        GtkTreePath *tpath;

        WARN("button press event: button 3 should do popup, as well as longpress...\n");
        if (!gtk_icon_view_get_item_at_pos (GTK_ICON_VIEW(widget),
                                   event->x,
                                   event->y,
                                   &tpath, NULL)) {

            tpath = NULL;
        }

        
        gboolean retval = FALSE;

        /* FIXME: enable popup...       
        if (tpath) {
            retval = ((view_c *)data)->get_xfdir_p()->popup(tpath);
            gtk_tree_path_free(tpath);
        } else {
            retval = ((view_c *)data)->get_xfdir_p()->popup();
        }
        */

        return retval;
    }

    

    static gboolean
    motion_notify_event (GtkWidget *widget,
                   GdkEvent  *ev,
                   gpointer   data)
    {
        GdkEventMotion *e = (GdkEventMotion *)ev;
        GdkEventButton  *event = (GdkEventButton  *)ev;
	auto baseView = (BaseView<Type> *)data;
        if (!data) {
            DBG("BaseView::motion_notify_event: data cannot be NULL\n");
            return FALSE;
        }
	TRACE("motion_notify_event\n");

#if 0
	// drag mode stuff...
	//
        //if (!view_p->all_set_up) return FALSE;
        TRACE("motion_notify, drag mode = %d\n", baseView->dragMode());
        // Are we intending to set up a DnD?
        // Say yes...
        gint mode = baseView->dragMode();
        // -1 : move
        // -1 : move (with shift)
        // -2 : copy (with ctl)
        // -3 : ln (with ctl+shift)
        // But is there a selection for the mode?
        if (mode){
            TRACE("// Are we intending to set up a DnD? Maybe...mode = %d\n",  mode);
            // should not happen:
            // if (mode>0) { return FALSE; }
            // Valid selection?
            GList *selection_list = gtk_icon_view_get_selected_items (baseView->iconView());
            if (!selection_list) {
                baseView->setDragMode(0);
                return FALSE;
            }

            baseView->setSelectionList(selection_list);
            TRACE("// Have we dragged outside the icon area?\n");
            if (!gtk_icon_view_get_item_at_pos (baseView->iconView(), e->x, e->y, NULL,NULL)) 
            {
                DBG("// Yeah. Let us start drag action now\n");
                // First de allow this to work as a click cancellation.
                // (if not rubberbanding)
                baseView->setClickCancel(1);
                
                // Set up for for move||copy||link drag now
                // Deprecated method
          /*      gtk_drag_source_set (GTK_WIDGET(view_p->get_iconview()),
                             (GdkModifierType)(GDK_BUTTON1_MASK), target_table,
                             NUM_TARGETS, GDK_ACTION_MOVE);  
                gtk_drag_source_set_target_list (GTK_WIDGET(view_p->get_iconview()),
                        view_p->get_target_list());*/

                GdkDragContext *context = 
                    gtk_drag_begin_with_coordinates (GTK_WIDGET(baseView->iconView()),
                       baseView->getTargetList(),
                       (GdkDragAction)(((gint)GDK_ACTION_MOVE)|
                       ((gint)GDK_ACTION_COPY)|
                       ((gint)GDK_ACTION_LINK)),
                       1, //drag button
                       ev,
                       e->x, e->y);
     
                if (g_list_length(selection_list) >1){
                    gtk_drag_set_icon_name (context, "edit-copy-symbolic", 0, 0);
                } else {
                    gtk_drag_set_icon_name (context, "document-send-symbolic", 0, 0);
                    // FIXME: get item pixbuf
                    /*xfdir_c *x = view_p->get_xfdir_p();
                    GtkTreeModel *treemodel = x->get_tree_model();
                    GtkTreePath *tpath = (GtkTreePath *)selection_list->data;
                    GtkTreeIter iter;
                    gtk_tree_model_get_iter (treemodel, &iter, tpath);
                    GdkPixbuf *pixbuf;
                    // XXX  will this add a ref to pixbuf? nah!
                    gtk_tree_model_get (treemodel, &iter, 
                        NORMAL_PIXBUF, &pixbuf, -1);       
                    gtk_drag_set_icon_pixbuf (context, pixbuf, 0, 0);*/
                }
                baseView->setDragMode(1);
            }
        }
#endif                                     

	// XXX: Why this limitation?
        // if (view_p->get_dir_count() > 500) return FALSE;
        baseView->highlight(e->x, e->y);
        return FALSE;
    }


/////////////////////////////////  DnD   ///////////////////////////
//receiver:

    static void
    signal_drag_data (GtkWidget * widget,
                      GdkDragContext * context,
                      gint x, gint y, 
                      GtkSelectionData * selection_data, 
                      guint info, 
                      guint time, 
                      gpointer data){
        DBG( "DND>> signal_drag_data\n");
	auto baseView = (BaseView<Type> *)data;

        gboolean result = FALSE;
        gchar *target = NULL;
        GtkTreePath *tpath=NULL;



        GdkDragAction action = gdk_drag_context_get_selected_action(context);
        
        TRACE("rodent_mouse: DND receive, info=%d (%d,%d)\n", info, TARGET_STRING, TARGET_URI_LIST);
        if(info != TARGET_URI_LIST) {
            goto drag_over;         /* of course */
        }

        TRACE("rodent_mouse: DND receive, action=%d\n", action);
        if(action != GDK_ACTION_MOVE && 
           action != GDK_ACTION_COPY &&
           action != GDK_ACTION_LINK) {
            DBG("Drag drop mode is not GDK_ACTION_MOVE | GDK_ACTION_COPY | GDK_ACTION_LINK\n");
            goto drag_over;         /* of course */
        }


        if (gtk_icon_view_get_item_at_pos (baseView->iconView(),
                                   x, y, &tpath, NULL))
        {
            GtkTreeIter iter;
            gtk_tree_model_get_iter (baseView->treeModel(), &iter, tpath);
            gtk_tree_model_get (baseView->treeModel(), &iter, 
                    ACTUAL_NAME, &target, -1);	
        } else tpath=NULL;
                    // nah
       /* gtk_icon_view_get_drag_dest_item (view_p->get_iconview(),
                                      &tpath,
                                      GtkIconViewDropPosition *pos);*/
        // this stuff will be immersed in specific class
        // FIXME:
        // result = view_p->get_xfdir_p()->receive_dnd(target, selection_data, action);
   
        
        if (tpath) gtk_tree_path_free(tpath);
        g_free(target);
      drag_over:
        auto dndData = (const char *)gtk_selection_data_get_data (selection_data);
        
	DBG("dndData = \"%s\"\n", dndData);

        gtk_drag_finish (context, TRUE, 
                (action == GDK_ACTION_MOVE) ? TRUE : FALSE, 
                time);
        DBG("rodent_mouse: DND receive, drag_over\n");
        return;

    } 


    static gboolean
    signal_drag_motion (GtkWidget * widget, 
            GdkDragContext * dc, gint drag_x, gint drag_y, 
            guint t, gpointer data) {
	auto baseView = (BaseView<Type> *)data;

                                        
        GtkTreePath *tpath;
                                        
        GtkIconViewDropPosition pos;
            
        if (gtk_icon_view_get_dest_item_at_pos (baseView->iconView(),
                                        drag_x, drag_y,
                                        &tpath,
                                        &pos)){
            // drop into?
            // must be a directory
            // FIXME:
            //view_p->get_xfdir_p()->highlight_drop(tpath);
            ////view_p->highlight(drag_x, drag_y);
        } else {
            // FIXME:
            //view_p->get_xfdir_p()->clear_highlights();
        }
        // Called by the receiving end of the DnD
        //
     //   GdkDragAction action = gdk_drag_context_get_actions(dc);
            
      //  gdk_drag_status (dc, action, t);

        
      //  fprintf (stderr, "DND>> drag_motion\n");
        // Set drag source to move copy or link here.
       
        return FALSE;
    }

    // sender:
    static void
    signal_drag_end (GtkWidget * widget, GdkDragContext * context, gpointer data) {
        WARN("signal_drag_end\n");
        
	auto baseView = (BaseView<Type> *)data;

        baseView->setDragMode(0);
        //gtk_drag_source_unset(GTK_WIDGET(baseView->iconView()));
        baseView->freeSelectionList();
        
    }


    static void
    signal_drag_begin (GtkWidget * widget, GdkDragContext * drag_context, gpointer data) {
        WARN("signal_drag_begin\n");
	auto baseView = (BaseView<Type> *)data;

    //  single or multiple item selected?
        GList *selection_list = gtk_icon_view_get_selected_items (baseView->iconView());
        baseView->setSelectionList(selection_list);
        if (g_list_length(selection_list)==1){
            DBG("Single selection\n");
        } else if (g_list_length(selection_list)>1){
            DBG("Multiple selection\n");
        } else return;
    
    //  set drag icon
    /*
        drag_view_p = view_p;
        rodent_hide_tip ();
        if (!view_p->en || !view_p->en->path) return; 
        write_drag_info(view_p->en->path, view_p->en->type);
        view_p->mouse_event.drag_event.context = drag_context;*/
    }

    static void
    signal_drag_data_get (GtkWidget * widget,
                       GdkDragContext * context, 
                       GtkSelectionData * selection_data, 
                       guint info, 
                       guint time,
                       gpointer data) {
        WARN("signal_drag_data_get\n");
        //g_free(files);
        
        //int drag_type;

	auto baseView = (BaseView<Type> *)data;


        /* prepare data for the receiver */
        switch (info) {
#if 10
          case TARGET_RAW:
            DBG( ">>> DND send, TARGET_RAW\n"); return;;
          case TARGET_UTF8:
            DBG( ">>> DND send, TARGET_UTF8\n"); return;
#endif
          case TARGET_URI_LIST:
            {
                DBG( ">>> DND send, TARGET_URI_LIST\n"); 
                // FIXME:
                
                //xfdir_c *xfdir_p = view_p->get_xfdir_p();
                GList *selection_list = baseView->selectionList();
                gboolean result = baseView->setDndData(selection_data, selection_list);
              
            }
            break;
          default:
            DBG( ">>> DND send, non listed target\n"); 
            break;
        }
    }

    static void
    signal_drag_leave (GtkWidget * widget, GdkDragContext * drag_context, guint time, gpointer data) {
        DBG("signal_drag_leave\n");

    }

    static void
    signal_drag_delete (GtkWidget * widget, GdkDragContext * context, gpointer data) {
        DBG("signal_drag_delete\n");
    }

protected:

    gchar *path_;
    GtkTreeModel *treeModel_;
    gint dirCount_; 
    GtkIconView *iconView_;

private:
 
    static gint 
    iconColumn(void){ return Type::iconColumn();}

    static gint 
    textColumn(void){ return Type::textColumn();}

    static void
    item_activated (GtkIconView *iconview,
                    GtkTreePath *tpath,
                    gpointer     data)
    {
        DBG("BaseView::item activated\n");
	Type::item_activated(iconview, tpath, data);
        /*
        auto view = (BaseView *)data;
        xfdir_c *xfdir_p = view_p->get_xfdir_p();
        xfdir_p->item_activated(iconview, tpath, data);
        */
    }
   
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
