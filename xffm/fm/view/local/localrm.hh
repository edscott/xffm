#ifndef XF_LOCALRM__HH
# define XF_LOCALRM__HH

# define RM_NO			0
# define RM_YES			1
# define RM_YES_ALL		2
# define SHRED_YES              3
# define SHRED_YES_ALL          4
# define RM_TRASH		5
# define RM_CANCEL		6

# define MODE_RM                 1
# define MODE_SHRED              2
# define MODE_DUMMY_TRASH        3
namespace xf
{

template <class Type>
class LocalRm {

public:
    static void
    rm(GtkMenuItem *menuItem, gpointer data){
	DBG("LocalRm:: rm\n");
	auto rmDialog = createRemove("text...", "message..", TRUE);
	gtk_dialog_run(GTK_DIALOG(rmDialog));
    }

private:
    static
    GtkWindow *
    createRemove (const gchar *text, const gchar *message, gboolean always) {
	auto rmDialog = GTK_WINDOW(gtk_window_new (GTK_WINDOW_TOPLEVEL));
	gtk_window_set_type_hint(GTK_WINDOW(rmDialog), GDK_WINDOW_TYPE_HINT_DIALOG);

       // title
	gchar *g=g_strdup_printf("Rodent: %s", _("Remove"));
	gtk_window_set_title (rmDialog, g);
	// icon
	auto pb = Pixbuf<Type>::get_pixbuf("edit-delete", SIZE_ICON);
	gtk_window_set_icon (rmDialog, pb);
	g_object_unref(pb);
	//gtk_window_set_modal (rmDialog, TRUE);

	auto vbox2 = Gtk<Type>::vboxNew (FALSE, 0);
	gtk_widget_show (GTK_WIDGET(vbox2));
	gtk_container_add (GTK_CONTAINER (rmDialog), GTK_WIDGET(vbox2));

	auto hbox26 = Gtk<Type>::hboxNew (FALSE, 0);
	gtk_widget_show (GTK_WIDGET(hbox26));
	gtk_box_pack_start (vbox2, GTK_WIDGET(hbox26), TRUE, TRUE, 0);

	pb = Pixbuf<Type>::get_pixbuf ("dialog-question", SIZE_ICON);
	auto question = gtk_image_new_from_pixbuf (pb);
	g_object_unref(pb);
	gtk_widget_show (GTK_WIDGET(question));
	gtk_box_pack_start (hbox26, GTK_WIDGET(question), TRUE, TRUE, 5);
	g_object_set_data(G_OBJECT(rmDialog), "question", question);


	auto vbox12 = Gtk<Type>::vboxNew (FALSE, 0);
	gtk_widget_show (GTK_WIDGET(vbox12));
	gtk_box_pack_start (hbox26, GTK_WIDGET(vbox12), TRUE, TRUE, 0);

	auto label16 = GTK_LABEL(gtk_label_new (text));
	gtk_label_set_markup(label16, text);
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
		G_CALLBACK (apply_action), GINT_TO_POINTER(RM_CANCEL));
	g_object_set_data(G_OBJECT(button), "rmDialog", rmDialog);
	g_object_set_data(G_OBJECT(rmDialog), "cancelbutton", button);

	button = Gtk<Type>::dialog_button ("user-trash", _("Trash"));
	gtk_container_add (GTK_CONTAINER (buttonbox), GTK_WIDGET(button));
	g_signal_connect (G_OBJECT (button), "clicked", 
		G_CALLBACK (apply_action), GINT_TO_POINTER(RM_CANCEL));
	g_object_set_data(G_OBJECT(button), "rmDialog", rmDialog);
	g_object_set_data(G_OBJECT(rmDialog), "cancelbutton", button);

    /****************/
	// This is now available to BSD with rm -P option
	button = Gtk<Type>::dialog_button ("emblem-unreadable", _("Shred"));
	gtk_container_add (GTK_CONTAINER (buttonbox), GTK_WIDGET(button));
	g_signal_connect (G_OBJECT (button), "clicked", 
		G_CALLBACK (apply_action), GINT_TO_POINTER(SHRED_YES));
	g_object_set_data(G_OBJECT(button), "rmDialog", rmDialog);

    /****************/

	button = Gtk<Type>::dialog_button ("edit-delete", _("Delete"));
	gtk_container_add (GTK_CONTAINER (buttonbox), GTK_WIDGET(button));
	g_signal_connect (G_OBJECT (button), "clicked", 
		G_CALLBACK (apply_action), GINT_TO_POINTER(RM_YES));
	g_object_set_data(G_OBJECT(button), "rmDialog", rmDialog);


	g_signal_connect (rmDialog, "delete-event", G_CALLBACK (on_destroy_event), rmDialog);
	g_signal_connect (rmDialog, "destroy-event", G_CALLBACK (on_destroy_event), NULL);


	gtk_widget_realize (GTK_WIDGET(rmDialog));

	gtk_widget_grab_focus (GTK_WIDGET(button));

	gtk_window_set_position(rmDialog, GTK_WIN_POS_CENTER);
	gtk_widget_show (GTK_WIDGET(rmDialog));


     
	return rmDialog;
    }

private:

    static gboolean
    on_destroy_event (GtkWidget * rmDialog, GdkEvent * event, gpointer data) {
	gtk_widget_hide(GTK_WIDGET(data));
	gtk_widget_destroy(GTK_WIDGET(data));
	// Send cancel response
	//GtkWidget *button=g_object_get_data(G_OBJECT(rmDialog), "cancelbutton");
	//apply_action(button, GINT_TO_POINTER(RM_CANCEL));
	
	return TRUE;
    }
static void
apply_action(GtkWidget * button, gpointer data){
	    
#if 0   
    GtkWidget *dialog=g_object_get_data(G_OBJECT(button), "dialog");
    GSList *list = g_object_get_data(G_OBJECT(dialog), "list");
    gtk_widget_hide(dialog);

    GtkWidget *togglebutton=g_object_get_data(G_OBJECT(dialog), "togglebutton");
    gboolean apply_to_all=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(togglebutton));
    gint result=GPOINTER_TO_INT(data);

    if (result == RM_YES && apply_to_all) result=RM_YES_ALL;
    else if (result == SHRED_YES && apply_to_all) result=SHRED_YES_ALL;
    else if (result == RM_NO && apply_to_all) result=RM_CANCEL;
    
    NOOP( "**apply_action: 0x%x\n", result);

    gint mode = MODE_RM;
    GSList *tmp;
    gchar *path;
    switch (result) {
	case SHRED_YES:
	    mode = MODE_SHRED;
            // case falls through. This is intentional
            // coverity[unterminated_case : FALSE]
	case RM_YES:;
	    NOOP( "**single remove: %s\n", (gchar *)list->data);

	    GSList *single_list = g_slist_append (NULL, g_strdup((gchar *)list->data));
	    {
		void **arg =(void **)malloc(3*sizeof(void *));
		if (!arg) g_error("malloc: %s\n", strerror(errno));
		arg[0]=widgets_p;
		arg[1]=single_list;
		arg[2]=GINT_TO_POINTER(mode);
                TRACE("** single rm thread requested\n");
                
		rfm_view_thread_create(widgets_p->view_p, do_the_remove, arg, "do_the_remove");   
	    }
	    path = (gchar *) list->data;
	    list = g_slist_remove (list, list->data);
	    g_free(path);
	    if (g_slist_length(list)==0) {
		g_slist_free(list);
		list = NULL;
	    }
	    break;
	case RM_NO:
	    NOOP( "**cancel remove: %s\n", (gchar *)list->data);
	    path = (gchar *) list->data;
	    list = g_slist_remove (list, list->data);
	    g_free(path);
	    if (g_slist_length(list)==0) {
		g_slist_free(list);
		list = NULL;
	    }
	    break;
	case SHRED_YES_ALL:
	    mode = MODE_SHRED;
            // case falls through. This is intentional
            // coverity[unterminated_case : FALSE]
	case RM_YES_ALL:;
	    GSList *full_list = NULL;
	    NOOP( "**remove all\n");
	    tmp = list;
	    for (; tmp && tmp->data; tmp=tmp->next){
		NOOP( "**remove all: %s\n", (gchar *)tmp->data);
		full_list = g_slist_append (full_list, g_strdup((gchar *)tmp->data));
	    }
	    {
		void **arg =(void **)malloc(3*sizeof(void *));
		if (!arg) g_error("malloc: %s\n", strerror(errno));
		arg[0]=widgets_p;
		arg[1]=full_list;
		arg[2]=GINT_TO_POINTER(mode);
		rfm_view_thread_create(widgets_p->view_p, do_the_remove, arg, "do_the_remove");
	    }
	    tmp=list;
	    for (; tmp && tmp->data; tmp=tmp->next){
		g_free(tmp->data);
	    }
	    g_slist_free(list);
	    list = NULL;
	    break;
	case RM_CANCEL:
            // case falls through. This is intentional
            // coverity[unterminated_case : FALSE]
	default:
	    NOOP( "**cancel remove all\n");
	    tmp=list;
	    for (; tmp && tmp->data; tmp=tmp->next){
		g_free(tmp->data);
	    }
	    g_slist_free(list);
	    list=NULL;
	    break;
    }
    gtk_widget_destroy(dialog);
    gtk_main_quit();

    // We are already in a thread environment here, so there is no need to
    // spawn another thread.

    if (list) rm(widgets_p, list);
#endif
}

#if 0
static void *
do_the_remove (void *data) {
    NOOP(stderr, "do_the_remove....\n");
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
