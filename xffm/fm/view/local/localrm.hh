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

/*static void * 
create_remove_f(gpointer data){
    void **arg = data;
    widgets_t *widgets_p = arg[0];
    gchar *text  = arg[1];
    gchar *message  = arg[2];
    GSList *list = arg[3];
    gboolean always = GPOINTER_TO_INT(arg[4]);
    g_free(arg);
    
    GtkWidget *dialog = create_remove (widgets_p, text, message, always);
    
    g_object_set_data(G_OBJECT(dialog), "widgets_p", widgets_p);
    g_object_set_data(G_OBJECT(dialog), "list", list);
    g_free(message);
    g_free(text);
    // dialog specifics 
    GtkWidget *togglebutton=g_object_get_data(G_OBJECT(dialog), "togglebutton");

    if(g_slist_length (list) < 2) {
	gtk_widget_hide(togglebutton);
    }
    gtk_main();
    return FALSE;
}*/

public:
    static void
    rm(GtkMenuItem *menuItem, gpointer data){
        
	DBG("LocalRm:: rm\n");
	auto baseView =  (BaseView<Type> *)g_object_get_data(G_OBJECT(data), "baseView");
        auto selection_list = gtk_icon_view_get_selected_items (baseView->iconView());
        if (!selection_list){
            DBG("rm(): nothing selected\n");
        }
        GList *list = NULL;
        for (auto tmp=selection_list; tmp && tmp->data; tmp = tmp->next){
            gchar *path;
            GtkTreeIter iter;
            auto tpath = (GtkTreePath *)tmp->data;
            gtk_tree_model_get_iter(baseView->treeModel(), &iter, tpath);
            gtk_tree_model_get(baseView->treeModel(), &iter, PATH, &path, -1);
            list = g_list_append(list, path);
        }
        while (list && g_list_length(list)>0) {
            list = rmQuery(baseView, list);
        }
    }

private:
    static GList *
    rmQuery(BaseView<Type> *baseView, GList *list){
	DBG("rmQuery: rm\n");
        if (!list || !g_list_length(list)) {
            g_list_free(list);
            return NULL;
        }
        auto text = g_strdup_printf(_("Delete %s"), (gchar *)list->data);
        auto message = g_list_length(list) > 1 ? 
        g_strdup_printf("<span color=\"red\">%s (%d)</span>", _("Multiple selections"), g_list_length(list)):
        "";
        auto rmDialog = createRemove(baseView, text, message, TRUE);
        g_object_set_data(G_OBJECT(rmDialog), "list", list);
        /* dialog specifics */
        auto togglebutton=GTK_WIDGET(g_object_get_data(G_OBJECT(rmDialog), "togglebutton"));
        if(g_list_length (list) < 2) {
            gtk_widget_hide(togglebutton);
        }
	gtk_widget_show (GTK_WIDGET(rmDialog));
	gint response = gtk_dialog_run(GTK_DIALOG(rmDialog));
        //list = (GList *)g_object_get_data(G_OBJECT(rmDialog), "list");

	apply_action(rmDialog, response);
        list = (GList *)g_object_get_data(G_OBJECT(rmDialog), "list");
	
        gtk_widget_hide(GTK_WIDGET(rmDialog));
        gtk_widget_destroy(GTK_WIDGET(rmDialog));
	
        return list;
   }

    static
    GtkDialog *
    createRemove (BaseView<Type> *baseView, const gchar *text, const gchar *message, gboolean always) {
	DBG("createRemove: rm\n");
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

	pb = Pixbuf<Type>::get_pixbuf ("edit-delete", -96);
	auto q = gtk_image_new_from_pixbuf (pb);
	gtk_widget_show (GTK_WIDGET(q));
	gtk_box_pack_start (vbox2, GTK_WIDGET(q), TRUE, TRUE, 5);

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

    static void
    removeAllFromList(GtkDialog *dialog){
        auto list = (GList *)g_object_get_data(G_OBJECT(dialog), "list");
        if (!list){
            DBG("removeAllFromList(): list is NULL\n");
            return;
        }
        for (auto tmp = list; tmp && tmp->data; tmp=tmp->next){
            g_free(tmp->data);
        }
        g_list_free(list);
        g_object_set_data(G_OBJECT(dialog), "list", NULL);
    }
        
    static void
    removeItemFromList(GtkDialog *dialog, void *path){
        auto list = (GList *)g_object_get_data(G_OBJECT(dialog), "list");
        if (!list){
            DBG("removeItemFromList(): list is NULL\n");
            return;
        }
        list = g_list_remove (list, path);
        g_free(path);
        g_object_set_data(G_OBJECT(dialog), "list", list);
       /* if (g_list_length(list)==0) {
            g_list_free(list);
            g_object_set_data(G_OBJECT(dialog), "list", NULL);
        } else {
            g_object_set_data(G_OBJECT(dialog), "list", NULL);
        }*/
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

    static void
    //apply_action(GtkWidget * button, gpointer data){
    apply_action(GtkDialog *rmDialog, gint result){
        auto list = (GList *)g_object_get_data(G_OBJECT(rmDialog), "list");
       
        DBG( "**apply_action: 0x%x\n", result);

        switch (result) {
            case TRASH_YES:
                DBG( "**single trash: %s\n", (gchar *)list->data);
                // Trash operation
                if (!Gio<Type>::doIt(rmDialog, (gchar *)list->data, MODE_TRASH)){
                    DBG("Cannot trash %s\n", (gchar *)list->data);
                   break;
                }
                removeItemFromList(rmDialog, list->data);

                break;
            case TRASH_YES_ALL:
                DBG( "trash all\n");
                if (!Gio<Type>::multiDoIt(rmDialog, _("Trash"), "user-trash", list, MODE_TRASH)){
                    DBG("Cannot multiTrash %s\n", (gchar *)list->data);
                    break;
                }

                removeAllFromList(rmDialog);
                break;
            case RM_YES:
            {
                DBG( "**single remove: %s\n", (gchar *)list->data);
                 // rm operation
                if (!Gio<Type>::doIt(rmDialog, (gchar *)list->data, MODE_RM)){
                    DBG("Cannot delete %s\n", (gchar *)list->data);
                   break;
                }
                removeItemFromList(rmDialog, list->data);
                break;
            }
            case RM_YES_ALL:
            {
                if (!Gio<Type>::multiDoIt(rmDialog, _("Delete"), "edit-delete", list, MODE_RM)){
                    DBG("Cannot multiDelete %s\n", (gchar *)list->data);
                    break;
                }
                removeAllFromList(rmDialog);
                break;
            }

            case SHRED_YES:
                DBG( "**single shred: %s\n", (gchar *)list->data);
                 // Shred operation
                removeItemFromList(rmDialog, list->data);
               break;
            case RM_NO:
            {
                DBG( "remove cancelled: %s\n", (gchar *)list->data);
                removeItemFromList(rmDialog, list->data);
                break;
            }
            ////////////////////////////////
            case SHRED_YES_ALL:
                DBG( "shred all\n");
                removeAllFromList(rmDialog);
                break;
            case RM_CANCEL:
                DBG( "**cancel remove\n");
                removeAllFromList(rmDialog);
                break;
            default:
            {
                DBG( "**default : cancel remove all\n");
                removeAllFromList(rmDialog);
               break;
            }
        }
        gtk_widget_hide(GTK_WIDGET(rmDialog));
        //gtk_main_quit();

        // We are already in a thread environment here, so there is no need to
        // spawn another thread.
    /*
        if (list) rm(widgets_p, list);
        */
    }

#if 0
static void *
do_the_remove (void *data) {
    TRACE(stderr, "do_the_remove....\n");
    void **arg = data;
    widgets_t *widgets_p =arg[0];
    GSList * rm_list = arg[1];
    gboolean unlink_mode = GPOINTER_TO_INT(arg[2]);
    g_free(arg);

    gint64 flag;
    const gchar *cflag;
    gchar *sources[MAX_COMMAND_ARGS];
    gint i = 0;
    RfmProgramOptions *options_p=NULL;
    if(unlink_mode == MODE_SHRED) {
#ifdef GNU_RM
	//Linux: uses gnu shred
	options_p = get_shred_options();
	cflag = getenv("RFM_SHRED_FLAGS");
        sources[i++] = "shred";
#else 
	// BSD: uses rm -P option
	options_p = get_rm_options();
	cflag = getenv("RFM_RM_FLAGS");
        sources[i++] = "rm";
        sources[i++] = "-P";
#endif
    } else if(unlink_mode == MODE_RM) {
	options_p = get_rm_options();
	cflag = getenv("RFM_RM_FLAGS");
        sources[i++] = "rm";
    } else if (unlink_mode == MODE_DUMMY_TRASH) {
	DBG("do_the_remove(): method %d (MODE_DUMMY_TRASH) not implemented\n", unlink_mode);
	return NULL;
    } else {
	DBG("do_the_remove(): method %d not implemented\n", unlink_mode);
	return NULL;
    }
    errno=0;
    flag = strtoll(cflag, NULL, 16);
    if (errno){
	DBG("rm.i: strtollfailed \n");
    }


    //gint interactive = 0; //
    // flag j==0 is the always ask flag, which is not a command flag
    gint j = 0;
    for (; options_p && options_p->option; options_p++, j++){
	if (j==0) continue;
	if (!(options_p->sensitive)) continue;
	if (!(flag & (ONE64<<j))) continue;
	if (!options_p->choice_id){
	    if (strcmp(options_p->option, "-i")==0) continue;
	    else if (strcmp(options_p->option, "-I")==0) continue;
	    else {
		TRACE( "adding option: %s  0x%llx & 0x%llx (%d)\n",
			options_p->option,
			(long long)flag, (long long)(ONE64<<j), j);
		sources[i++] = g_strdup(options_p->option);
	    }
	} else {
	    if (strcmp(options_p->option, "--interactive=")==0) continue;

	    /*if (strcmp(options_p->option, "--interactive")==0) {
		if(strcmp(options_p->choice_id,"always")==0){
		    interactive = 1;
		} else if(strcmp(options_p->choice_id,"once")==0){
		    interactive = -1;
		} else if(strcmp(options_p->choice_id,"never")==0){
		    interactive = 0;
		}
	    } else {*/
		const gchar *item=NULL;
		switch (options_p->choice_id){
		    case RFM_SHRED_iterations:
			item = getenv("RFM_SHRED_iterations");
			break;
		    case RFM_SHRED_size:
			item = getenv("RFM_SHRED_size");
			break;
		    /*case RFM_SHRED_source:
			item = getenv("RFM_SHRED_source");
			break;*/
		}
		sources[i++] = g_strconcat(options_p->option,item,NULL);
	   }
	}
    
    }
}
    
#endif


    
};
}

#endif
