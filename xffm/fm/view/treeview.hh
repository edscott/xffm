#ifndef XF_TREEVIEW__HH
# define XF_TREEVIEW__HH
namespace xf
{

template <class Type>
class TreeView {
 
public: 
    static GtkTreeView *createTreeview(BaseView<Type> *baseView){
        auto treeView = GTK_TREE_VIEW(gtk_tree_view_new());
	gtk_tree_view_set_model(treeView, baseView->treeModel());
        g_object_set(G_OBJECT(treeView), "has-tooltip", TRUE, NULL);
        //gtk_icon_view_set_item_width (icon_view, 60);
        gtk_tree_view_set_activate_on_single_click(treeView, TRUE);
        appendColumnPixbuf(treeView, TREEVIEW_PIXBUF);
        appendColumnText(treeView, _("Name"), DISPLAY_NAME);
        appendColumnText(treeView, _("Size"), SIZE);
        appendColumnText(treeView, _("Date"), DATE);
        appendColumnText(treeView, _("Mimetype"), MIMETYPE);
        appendColumnText(treeView, _("ICON_NAME"), ICON_NAME);
        appendColumnText(treeView, _("TYPE"), TYPE);
        appendColumnText(treeView, _("TOOLTIP_TEXT"), TOOLTIP_TEXT);
        setUpSignals(baseView, G_OBJECT(treeView));
        auto selection = gtk_tree_view_get_selection (treeView);
        gtk_tree_selection_set_mode (selection,  GTK_SELECTION_MULTIPLE);
        gtk_tree_view_set_rubber_banding (treeView, TRUE);        
        
        return treeView;
    }

private:
    static void
    setUpSignals(BaseView<Type> *baseView, GObject * treeView){
        g_signal_connect (treeView, "row-activated", 
            G_CALLBACK (rowActivated), 
            (void *)baseView);
         g_signal_connect (treeView, "button-release-event",
             G_CALLBACK(buttonRelease), 
             (void *)baseView);
         g_signal_connect (treeView, "button-press-event",
             G_CALLBACK(buttonPress), 
             (void *)baseView);
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
        BaseViewSignals<Type>::activate(tpath, data);
    }
     static gboolean
    buttonRelease (GtkWidget *widget,
                   GdkEventButton  *event,
                   gpointer   data)
    {
        DBG("treeView: buttonRelease()\n");
	auto baseView = (BaseView<Type> *)data;
        auto selection = gtk_tree_view_get_selection (baseView->treeView());
        if (!dragOn_){
	    GtkTreePath *tpath;

	    if (rubberBand_) {
                baseView->selectables();
		return FALSE;
	    }
	    /*if (!gtk_icon_view_get_item_at_pos (baseView->iconView(),
                                   event->x, event->y,
                                   &tpath,NULL)){
		WARN("button down cancelled.\n");
		return TRUE;
	    }*/

	    //Cancel DnD prequel.
	    buttonPressX = buttonPressY = -1;
	    dragOn_ = FALSE;
	    if (dragMode) { // copy/move/link mode
		return TRUE;
	    }
	    // default mode:
            gtk_tree_view_get_path_at_pos (baseView->treeView(), 
                               event->x, event->y, &tpath,
                              NULL, // &column,
                              NULL, NULL);

	    // unselect everything
            gtk_tree_selection_unselect_all (selection);

	    // reselect item to activate
	    gtk_tree_selection_select_path (selection, tpath);

	    WARN("Here we do a call to activate item.\n");
	    BaseViewSignals<Type>::activate(tpath, data);
	    gtk_tree_path_free(tpath);

            // FIXME: maybe we have to do the same clear selectionlist for iconview
            baseView->setSelectionList(NULL);
            
	    return TRUE;
        }

	buttonPressX = buttonPressY = -1;
        dragOn_ = FALSE;
	
        dragMode = 0;
        return FALSE;

    }
  
    static gboolean
    buttonPress (GtkWidget *widget,
                   GdkEventButton  *event,
                   gpointer   data)
    {
        auto baseView = (BaseView<Type> *)data;
        buttonPressX = buttonPressY = -1;
        dragOn_ = FALSE;
        auto selection = gtk_tree_view_get_selection (baseView->treeView());
        GtkTreePath *tpath;

        if (event->button == 1) {
            DBG("treeview: button press 1\n");
	    controlMode = FALSE;
            gboolean retval = FALSE;
            gint mode = 0;
            if (gtk_tree_view_get_path_at_pos (baseView->treeView(), 
                               event->x, event->y, &tpath,
                              NULL, // &column,
                              NULL, NULL)) // &cellX, &cellY))
            {
                
		DBG("button press %d mode %d\n", event->button, mode);
		buttonPressX = event->x;
		buttonPressY = event->y;
		rubberBand_ = FALSE;
                if (CONTROL_MODE && SHIFT_MODE) {
		    dragMode = -3; // link mode
		} else if (CONTROL_MODE) {
		    controlMode = TRUE;
		    dragMode = -2; // copy
		    // select item and add to selection list
		    if (gtk_tree_selection_path_is_selected (selection, tpath)) {
			// if selected
			gtk_tree_selection_unselect_path (selection, tpath);
		    } else { // not selected
                        gtk_tree_selection_select_path (selection, tpath);
                        baseView->selectables();
		    }
		} else if (SHIFT_MODE) {
		    dragMode = -1; // move
                    // FIXME
                    // viewShiftSelect(baseView, tpath);
		} else {
		    // unselect all
                    gtk_tree_selection_unselect_all (selection);
		    // select single item
		    gtk_tree_selection_select_path (selection, tpath);
		    dragMode = 0; // default (move)
		}
                retval = TRUE; 
		gtk_tree_path_free(tpath);
		tpath = NULL; // just in case.
            } else { 
                tpath=NULL;
		dragMode = 0;
		rubberBand_ = TRUE;
            }
            return retval;
        }

        // long press or button 3 should do popup menu...
        if (event->button != 3) return FALSE;
            GtkTreeViewColumn *column;
            gint cellX, cellY;
            gtk_tree_view_get_path_at_pos (baseView->treeView(), 
                               event->x, event->y, &tpath,
                              NULL, // &column,
                               &cellX, &cellY);
            selection = gtk_tree_view_get_selection (baseView->treeView());
            if (CONTROL_MODE) gtk_tree_selection_unselect_all (selection);
	    else {
		gtk_tree_selection_select_path (selection, tpath);
		//FIXME: treeview selection should probably be rowreference
                /*GList *list = baseView->selectionList();
                for (;list && list->data; list = list->next){
                    gtk_tree_selection_select_path (selection, (GtkTreePath *)list->data);
                }   */
            } 
	    gtk_tree_path_free(tpath);
            DBG("(%lf,%lf) -> %d,%d\n", event->x, event->y, cellX, cellY);

        return viewPopUp(baseView, event);
    }

    static gboolean 
    viewPopUp(BaseView<Type> *baseView, GdkEventButton  *event){
        TRACE("button press event: button 3 should do popup, as well as longpress...\n");
        GtkTreePath *tpath;
        gboolean retval = TRUE;
        GtkMenu *menu = NULL;
        auto selection = gtk_tree_view_get_selection (baseView->treeView());
        auto treeModel = baseView->treeModel();
        
        GList *selectionList = gtk_tree_selection_get_selected_rows (selection, &treeModel);
        baseView->setSelectionList(selectionList);
        DBG("selectionList length = %d\n", g_list_length(selectionList));
   /*     if (gtk_icon_view_get_item_at_pos (baseView->iconView(),
                                   event->x,
                                   event->y,
                                   &tpath, NULL))
        {
           if (!CONTROL_MODE){
                // unselect all
                gtk_icon_view_unselect_all (baseView->iconView());
            }
            gtk_icon_view_select_path (baseView->iconView(), tpath);
            gtk_tree_path_free(tpath);
        }
     */  
        gchar *path = NULL;
    
         if (selectionList && g_list_length(selectionList) == 1) {
            GtkTreeIter iter;
            gtk_tree_model_get_iter(baseView->treeModel(), &iter, 
                    (GtkTreePath *)selectionList->data);
            gtk_tree_model_get(baseView->treeModel(), &iter, PATH, &path, -1);
            DBG("selected path is %s\n", path);
        }
        gboolean items = (g_list_length(selectionList) >0);
        IconView<Type>::setMenuData(baseView, path, items);
        menu = IconView<Type>::configureMenu(baseView, items);
        if (menu) {
            gtk_menu_popup_at_pointer (menu, (const GdkEvent *)event);
        }   
        //g_list_free_full (selectionList, (GDestroyNotify) gtk_tree_path_free);
        return retval;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////
    
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
