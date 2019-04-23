#ifndef XF_LOCALRM__HH
# define XF_LOCALRM__HH
#include "common/gio.hh"

# define RM_NO			0
# define RM_YES			1
# define RM_YES_ALL		2
# define SHRED_YES              3
# define SHRED_YES_ALL          4
# define TRASH_YES		5
# define TRASH_YES_ALL		6
# define RM_CANCEL		7
namespace xf
{
template <class Type>
class LocalRm {


public:
    static void
    rm(GtkMenuItem *menuItem, gpointer data){
        
	TRACE("LocalRm:: rm\n");
	auto view =  (View<Type> *)g_object_get_data(G_OBJECT(data), "view");
        //  single or multiple item selected?
        GList *selectionList;
        if (isTreeView){
            auto treeModel = view->treeModel();
            auto selection = gtk_tree_view_get_selection (view->treeView());
            selectionList = gtk_tree_selection_get_selected_rows (selection, &treeModel);
        } else {
            selectionList = gtk_icon_view_get_selected_items (view->iconView());
        }
        if (!selectionList){
            ERROR("local/rm.hh::rm(): nothing selected\n");
        }
        GList *list = NULL;
        for (auto tmp=selectionList; tmp && tmp->data; tmp = tmp->next){
            gchar *path;
            GtkTreeIter iter;
            auto tpath = (GtkTreePath *)tmp->data;
            gtk_tree_model_get_iter(view->treeModel(), &iter, tpath);
            gtk_tree_model_get(view->treeModel(), &iter, PATH, &path, -1);
            list = g_list_append(list, path);
        }
        while (list && g_list_length(list)>0) {
            list = rmQuery(view, list);
        }
    }

private:
    static GList *
    rmQuery(View<Type> *view, GList *list){
	TRACE("rmQuery: rm\n");
        if (!list || !g_list_length(list)) {
            g_list_free(list);
            return NULL;
        }
        auto text = g_strdup_printf(_("Delete %s"), (gchar *)list->data);
        auto message = g_list_length(list) > 1 ? 
        g_strdup_printf("<span color=\"red\" size=\"larger\">%s\n%s (%d)</span>",_("Warning: "), _("Multiple selections"), g_list_length(list)):
        "";
        auto rmDialog = createRemove(view, text, message, FALSE, g_list_length(list)>1);
        g_object_set_data(G_OBJECT(rmDialog), "list", list);
        /* dialog specifics */
        auto togglebutton=GTK_WIDGET(g_object_get_data(G_OBJECT(rmDialog), "togglebutton"));
        if(g_list_length (list) < 2) {
            gtk_widget_hide(togglebutton);
        }
	gtk_widget_show (GTK_WIDGET(rmDialog));
        gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), FALSE);
	gint response = gtk_dialog_run(GTK_DIALOG(rmDialog));
        gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), TRUE);

	list = apply_action(rmDialog, response, list);
	
        gtk_widget_hide(GTK_WIDGET(rmDialog));
        gtk_widget_destroy(GTK_WIDGET(rmDialog));
	
        return list;
   }

    static
    GtkDialog *
    createRemove (View<Type> *view, const gchar *text, const gchar *message, gboolean always, gboolean multiple) {
	TRACE("createRemove: rm\n");
	//auto rmDialog = GTK_DIALOG(gtk_window_new (GTK_WINDOW_TOPLEVEL));
	auto rmDialog = GTK_DIALOG(gtk_dialog_new ());
	//gtk_window_set_type_hint(GTK_WINDOW(rmDialog), GDK_WINDOW_TYPE_HINT_DIALOG);
        
        // title
	gchar *g=g_strdup_printf("Xffm+ %s", _("Remove"));
	gtk_window_set_title (GTK_WINDOW(rmDialog), g);
	// icon
	auto pb = Pixbuf<Type>::get_pixbuf("edit-delete", SIZE_ICON);
	gtk_window_set_icon (GTK_WINDOW(rmDialog), pb);
	gtk_window_set_modal (GTK_WINDOW(rmDialog), TRUE);
        gtk_window_set_transient_for(GTK_WINDOW(rmDialog), GTK_WINDOW(mainWindow));

	auto vbox2 = Gtk<Type>::vboxNew (FALSE, 0);
	gtk_widget_show (GTK_WIDGET(vbox2));
	gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area(rmDialog)), GTK_WIDGET(vbox2));

	if (multiple){
	    pb = Pixbuf<Type>::get_pixbuf ("dialog-warning", -96);
	    auto q = gtk_image_new_from_pixbuf (pb);
	    gtk_widget_show (GTK_WIDGET(q));
	    gtk_box_pack_start (vbox2, GTK_WIDGET(q), TRUE, TRUE, 5);
        } else {
	    pb = Pixbuf<Type>::get_pixbuf ("edit-delete", -48);
	    auto q = gtk_image_new_from_pixbuf (pb);
	    gtk_widget_show (GTK_WIDGET(q));
	    gtk_box_pack_start (vbox2, GTK_WIDGET(q), TRUE, TRUE, 5);
	}

	auto hbox26 = Gtk<Type>::hboxNew (FALSE, 0);
	gtk_widget_show (GTK_WIDGET(hbox26));
	gtk_box_pack_start (vbox2, GTK_WIDGET(hbox26), TRUE, TRUE, 0);


	auto vbox12 = Gtk<Type>::vboxNew (FALSE, 0);
	gtk_widget_show (GTK_WIDGET(vbox12));
	gtk_box_pack_start (hbox26, GTK_WIDGET(vbox12), TRUE, TRUE, 0);

	auto label16 = GTK_LABEL(gtk_label_new (""));
        auto markup = g_strdup_printf("<span size=\"larger\"><b>  %s  </b></span>", text);
	gtk_label_set_markup(label16, markup);
        g_free(markup);
	gtk_widget_show (GTK_WIDGET(label16));
	gtk_box_pack_start (vbox12, GTK_WIDGET(label16), FALSE, FALSE, 0);

	auto label20 = GTK_LABEL(gtk_label_new (message));
	gtk_label_set_markup(label20, message);
	gtk_widget_show (GTK_WIDGET(label20));
	gtk_box_pack_start (vbox12, GTK_WIDGET(label20), FALSE, FALSE, 0);

	auto hbox9 = Gtk<Type>::hboxNew (FALSE, 0);
	gtk_widget_show (GTK_WIDGET(hbox9));
	gtk_box_pack_start (vbox12, GTK_WIDGET(hbox9), TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox9), 5);

	auto togglebutton = gtk_check_button_new_with_mnemonic (_("Apply to all"));
	gtk_widget_show (GTK_WIDGET(togglebutton));
	gtk_box_pack_start (hbox9, GTK_WIDGET(togglebutton), FALSE, FALSE, 0);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (togglebutton), !always);
	
	g_object_set_data(G_OBJECT(rmDialog),"togglebutton", togglebutton);

	auto buttonbox = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_widget_show (GTK_WIDGET(buttonbox));
	gtk_box_pack_start (GTK_BOX (vbox12), GTK_WIDGET(buttonbox), TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (buttonbox), 5);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (buttonbox), GTK_BUTTONBOX_END);
	gtk_box_set_spacing (GTK_BOX (buttonbox), 5);

	auto button = Gtk<Type>::dialog_button ("window-close", _("Cancel"));
	gtk_container_add (GTK_CONTAINER (buttonbox),GTK_WIDGET(button));
	g_signal_connect (G_OBJECT (button), "clicked", 
		G_CALLBACK (responseAction), GINT_TO_POINTER(RM_CANCEL));
	g_object_set_data(G_OBJECT(button), "rmDialog", rmDialog);
	g_object_set_data(G_OBJECT(rmDialog), "cancelbutton", button);
    /****************/
	// This is now available to BSD with rm -P option
	button = Gtk<Type>::dialog_button ("edit-delete/NE/edit-delete-symbolic/2.0/150", _("Shred"));
	gtk_container_add (GTK_CONTAINER (buttonbox), GTK_WIDGET(button));
	g_signal_connect (G_OBJECT (button), "clicked", 
		G_CALLBACK (responseAction), GINT_TO_POINTER(SHRED_YES));
	g_object_set_data(G_OBJECT(button), "rmDialog", rmDialog);
	gchar *shred = g_find_program_in_path("shred");
	gtk_widget_set_sensitive(GTK_WIDGET(button), shred != NULL);
	g_free(shred);

    /****************/

	button = Gtk<Type>::dialog_button ("edit-delete", _("Delete"));
	gtk_container_add (GTK_CONTAINER (buttonbox), GTK_WIDGET(button));
	g_signal_connect (G_OBJECT (button), "clicked", 
		G_CALLBACK (responseAction), GINT_TO_POINTER(RM_YES));
	g_object_set_data(G_OBJECT(button), "rmDialog", rmDialog);


	button = Gtk<Type>::dialog_button ("user-trash", _("Trash"));
	gtk_container_add (GTK_CONTAINER (buttonbox), GTK_WIDGET(button));
	g_signal_connect (G_OBJECT (button), "clicked", 
		G_CALLBACK (responseAction), GINT_TO_POINTER(TRASH_YES));
	g_object_set_data(G_OBJECT(button), "rmDialog", rmDialog);
	g_object_set_data(G_OBJECT(rmDialog), "trashbutton", button);

	g_signal_connect (rmDialog, "delete-event", G_CALLBACK (on_destroy_event), rmDialog);
	g_signal_connect (rmDialog, "destroy-event", G_CALLBACK (on_destroy_event), NULL);


	gtk_widget_realize (GTK_WIDGET(rmDialog));

	gtk_widget_grab_focus (GTK_WIDGET(button));

	gtk_window_set_position(GTK_WINDOW(rmDialog), GTK_WIN_POS_CENTER_ON_PARENT);


     
	return rmDialog;
    }

private:

    static gboolean
    on_destroy_event (GtkWidget * rmDialog, GdkEvent * event, gpointer data) {
        // Send cancel response
	auto  button=GTK_WIDGET(g_object_get_data(G_OBJECT(rmDialog), "cancelbutton"));
	responseAction(button, GINT_TO_POINTER(RM_CANCEL));
	return TRUE;
    }

    static GList *
    removeAllFromList(GList *list){
        if (!list){
            ERROR("local/rm.hh::removeAllFromList(): list is NULL\n");
            return NULL;
        }
        for (auto tmp = list; tmp && tmp->data; tmp=tmp->next){
            g_free(tmp->data);
        }
        g_list_free(list);
	return NULL;
    }
        
    static GList *
    removeItemFromList(GList *list){
        if (!list){
            ERROR("local/rm.hh::removeItemFromList(): list is NULL\n");
            return NULL;
        }
	void *path = list->data;
	TRACE("*** removing %s from list\n", (gchar *)path);
        list = g_list_remove (list, path);
        g_free(path);
	return list;
    }


    static void
    responseAction(GtkWidget * button, gpointer data){
        auto dialog=GTK_DIALOG(g_object_get_data(G_OBJECT(button), "rmDialog"));
        auto list = (GList *)g_object_get_data(G_OBJECT(dialog), "list");

        auto togglebutton=GTK_TOGGLE_BUTTON(g_object_get_data(G_OBJECT(dialog), "togglebutton"));
        auto apply_to_all=gtk_toggle_button_get_active(togglebutton);
        auto result=GPOINTER_TO_INT(data);

        if (result == RM_YES && apply_to_all) result=RM_YES_ALL;
        else if (result == SHRED_YES && apply_to_all) result=SHRED_YES_ALL;
        else if (result == RM_NO && apply_to_all) result=RM_CANCEL;
        else if (result == TRASH_YES && apply_to_all) result=TRASH_YES_ALL;
	gtk_dialog_response(dialog, result);

    }

    static GList *
    apply_action(GtkDialog *rmDialog, gint result, GList *list){
       
        TRACE( "**apply_action: 0x%x\n", result);

        switch (result) {
            case TRASH_YES:
                TRACE( "**single trash: %s\n", (gchar *)list->data);
                // Trash operation
                if (!Gio<Type>::execute(rmDialog, (gchar *)list->data, MODE_TRASH)){
                    ERROR("local/rm.hh::Cannot trash %s\n", (gchar *)list->data);
                }
                list = removeItemFromList(list);
                break;
            case TRASH_YES_ALL:
                TRACE( "trash all\n");
                if (!Gio<Type>::execute(rmDialog, list, MODE_TRASH)){
                    ERROR("local/rm.hh::Cannot multiTrash %s\n", (gchar *)list->data);
                }
                list = removeAllFromList(list);
                break;
            case RM_YES:
            {
                TRACE( "**single remove: %s\n", (gchar *)list->data);
                 // rm operation
                if (!Gio<Type>::execute(rmDialog, (gchar *)list->data, MODE_RM)){
                    ERROR("local/rm.hh::Cannot delete %s\n", (gchar *)list->data);
                }
                list = removeItemFromList(list);
                break;
            }
            case RM_YES_ALL:
            {
                if (!Gio<Type>::execute(rmDialog, list, MODE_RM)){
                    ERROR("local/rm.hh::Cannot multiDelete %s\n", (gchar *)list->data);
                }
                list = removeAllFromList(list);
                break;
            }

            case SHRED_YES:
                TRACE( "**single shred: %s\n", (gchar *)list->data);
                 // Shred operation
                if (!Gio<Type>::execute(rmDialog, (gchar *)list->data, MODE_SHRED)){

                    ERROR("local/rm.hh::Cannot shred %s\n", (gchar *)list->data);
                }
                list = removeItemFromList(list);
               break;
            case SHRED_YES_ALL:
                TRACE( "shred all\n");
                if (!Gio<Type>::execute(rmDialog, list, MODE_SHRED)){
                    ERROR("local/rm.hh::Cannot multishred %s\n", (gchar *)list->data);
                }
                list = removeAllFromList(list);
                break;
            case RM_NO:
            {
                TRACE( "remove cancelled: %s\n", (gchar *)list->data);
                list = removeItemFromList(list);
                break;
            }
            ////////////////////////////////
            case RM_CANCEL:
                TRACE( "**cancel remove\n");
                list = removeAllFromList(list);
                break;
            default:
            {
                TRACE( "**default : cancel remove all\n");
               list = removeAllFromList(list);
               break;
            }
        }
        gtk_widget_hide(GTK_WIDGET(rmDialog));
	return list;
    }

    
};
}

#endif
