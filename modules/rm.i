#ifdef COPYRIGHT_INFORMATION
#include "gplv3.h"
#endif
/*
 * Copyright (C) 2002-2012 Edscott Wilson Garcia
 * EMail: edscott@users.sf.net
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; 
 */


///////////////////////////////////  RM  ///////////////////////////////////
//


// specify mode as shred or delete

static void *rm (widgets_t *widgets_p, GSList *list);

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

    gboolean ok = FALSE;
    gboolean excess=FALSE;

    GSList *free_list=NULL;
    while(rm_list && (rm_list)->data) {
        gchar *path = (gchar *) (rm_list)->data;
	NOOP(stderr, "removing %s\n", path);
        if(path && (rfm_g_file_test (path, G_FILE_TEST_EXISTS) || rfm_g_file_test (path, G_FILE_TEST_IS_SYMLINK))) {
	    if (rfm_g_file_test (path, G_FILE_TEST_IS_SYMLINK)
		    && unlink_mode == MODE_SHRED) {
		// file is a symlink
		gchar *text=g_strconcat(_("Symbolic Link"),":\n", path, "\n", NULL);
		if (!rfm_confirm(widgets_p, GTK_MESSAGE_WARNING,
		text,
		_("Don't follow symlinks"), _("Follow symlinks"))){
		    // skip file.
		    rm_list = g_slist_remove (rm_list, path);
		    g_free(path);
		    g_free(text);
		    continue;
		}
		g_free(text);
	    }

	    sources[i++] = path;
	    free_list = g_slist_prepend(free_list, path);
	    rm_list = g_slist_remove (rm_list, path);
	    if (i == MAX_COMMAND_ARGS - 1  && rm_list){
		excess=TRUE;
		// we are at the last argv, for NULL
		// remaining items will not be processed.
		while (rm_list) {rm_list = g_slist_remove (rm_list, path);}
		break;
	    }
	    ok = TRUE;

        } else {
            rm_list = g_slist_remove (rm_list, path);
	    g_free(path);
        }
    }
    sources[i++] = NULL;

    // note: with run_argv we do not need to escape paths.
    if(ok) {
	// is view still valid?
	view_t *view_p = widgets_p->view_p;
	
	g_mutex_lock(view_p->mutexes.status_mutex);
	gint status = view_p->flags.status;
	g_mutex_unlock(view_p->mutexes.status_mutex);
	gchar *tgt= g_path_get_dirname (sources[i-2]);
	if (rfm_write_ok_path(tgt)){
	    if (status != STATUS_EXIT) {
		rfm_context_function(rfm_show_text, widgets_p);
	    }
	    rfm_thread_run_argv ((void *)widgets_p, sources, FALSE );
	} else {
	    gchar *failed;
	    if (unlink_mode == MODE_RM){
		failed = g_strdup( _("Delete failed"));
	    } else {
		gchar *warning0=g_strdup_printf (_("Unexpected error: %s"), _("Shred"));
		gchar *warning1=g_strdup_printf ("%s: %s",
			_("Shred"),
			_("Are you sure you want to continue?"));
		failed = g_strconcat(warning0, "\n", warning1, "\n", NULL);
		g_free(warning0);
		g_free(warning1);
	    }
	    // This will go off in an rfm_context_function...
	    const gchar *cmd;
#ifdef GNU_RM
	    cmd = (unlink_mode == MODE_RM)?"rm":"shred";
#else
	    cmd = (unlink_mode == MODE_RM)?"rm":"rm -P";
#endif
	    if (confirm_sudo( widgets_p, tgt, failed, cmd)){
		if (status != STATUS_EXIT) {
		    rfm_context_function(rfm_show_text, widgets_p);
		}
		RFM_TRY_SUDO (widgets_p, sources, FALSE);
	    }
	    g_free(failed);
	}
    } else {
        NOOP (stderr, "rm: ok == FALSE\n");
    }
    if (excess){
        rfm_threaded_diagnostics(widgets_p, "xffm/stock_dialog-warning",NULL);
        gchar *max=g_strdup_printf("%d",MAX_COMMAND_ARGS);
        rfm_threaded_diagnostics (widgets_p, "xffm_tag/stderr", g_strconcat(sources[0], ": ", strerror(E2BIG)," (> ",max,")","\n", NULL));
        g_free(max);
    }
    g_slist_free (rm_list);
    GSList *tmp = free_list;
    for (;tmp && tmp->data; tmp = tmp->next) g_free(tmp->data);
    g_slist_free (free_list);

    TRACE("** rm is done\n");
    return NULL;
}

static void
apply_action(GtkWidget * button, gpointer data){
	    
   
    GtkWidget *dialog=g_object_get_data(G_OBJECT(button), "dialog");
    widgets_t *widgets_p = g_object_get_data(G_OBJECT(dialog), "widgets_p");
    GSList *list = g_object_get_data(G_OBJECT(dialog), "list");
    gtk_widget_hide(dialog);
    if (!widgets_p){
	DBG ("apply_action(): !widgets_p\n");
	gtk_widget_destroy(dialog);
	return;
    }

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

}


static gboolean
on_destroy_event (GtkWidget * dialog, GdkEvent * event, gpointer data) {
    GtkWidget *button=g_object_get_data(G_OBJECT(dialog), "cancelbutton");
    apply_action(button, GINT_TO_POINTER(RM_CANCEL));
    return TRUE;
}

static
void *
create_remove (widgets_t * widgets_p, gchar *text, gchar *message, gboolean always) {

    GtkWidget *vbox2;
    GtkWidget *hbox26;
    GtkWidget *question;
    GtkWidget *vbox12;
    GtkWidget *label16;
    GtkWidget *label20;
    GtkWidget *hbox9;
    GdkPixbuf *pb;

    GtkWidget *dialog = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_type_hint(GTK_WINDOW(dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
    view_t *view_p = widgets_p->view_p;

    if(view_p->flags.type == DESKVIEW_TYPE) {
        gtk_window_set_keep_above (GTK_WINDOW(dialog), TRUE);
        gtk_window_stick (GTK_WINDOW(dialog));
    } else {
	// we do not want dialog to be gtk style transient, really
        // rfm_global_t *rfm_global_p = rfm_global();
	//gtk_window_set_transient_for(GTK_WINDOW(dialog), 
	//	GTK_WINDOW(rfm_global_p->window));
    }
   // title
    gchar *g=g_strdup_printf("Rodent: %s", _("Remove"));
    gtk_window_set_title (GTK_WINDOW (dialog), g);
    // icon
    pb = rfm_get_pixbuf ("xffm/stock_delete", SIZE_ICON);
    gtk_window_set_icon (GTK_WINDOW (dialog), pb);
    g_object_unref(pb);
    //gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
    g_object_set_data(G_OBJECT(dialog),"widgets_p", widgets_p);

    vbox2 = rfm_vbox_new (FALSE, 0);
    gtk_widget_show (vbox2);
    gtk_container_add (GTK_CONTAINER (dialog), vbox2);

    hbox26 = rfm_hbox_new (FALSE, 0);
    gtk_widget_show (hbox26);
    gtk_box_pack_start (GTK_BOX (vbox2), hbox26, TRUE, TRUE, 0);

    pb = rfm_get_pixbuf ("xffm/stock_dialog-question", SIZE_ICON);
    question = gtk_image_new_from_pixbuf (pb);
    g_object_unref(pb);
    gtk_widget_show (question);
    gtk_box_pack_start (GTK_BOX (hbox26), question, TRUE, TRUE, 5);
    g_object_set_data(G_OBJECT(dialog), "question", question);


    vbox12 = rfm_vbox_new (FALSE, 0);
    gtk_widget_show (vbox12);
    gtk_box_pack_start (GTK_BOX (hbox26), vbox12, TRUE, TRUE, 0);

    label16 = gtk_label_new (text);
    gtk_label_set_markup(GTK_LABEL(label16), text);
    gtk_widget_show (label16);
    gtk_box_pack_start (GTK_BOX (vbox12), label16, FALSE, FALSE, 0);

    label20 = gtk_label_new (message);
    gtk_label_set_markup(GTK_LABEL(label20), message);
    gtk_widget_show (label20);
    gtk_box_pack_start (GTK_BOX (vbox12), label20, FALSE, FALSE, 0);

    hbox9 = rfm_hbox_new (FALSE, 0);
    gtk_widget_show (hbox9);
    gtk_box_pack_start (GTK_BOX (vbox12), hbox9, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox9), 5);

    GtkWidget *togglebutton = gtk_check_button_new_with_mnemonic (_("Apply to all"));
    gtk_widget_show (togglebutton);
    gtk_box_pack_start (GTK_BOX (hbox9), togglebutton, FALSE, FALSE, 0);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (togglebutton), !always);
    
    g_object_set_data(G_OBJECT(dialog),"togglebutton", togglebutton);

    GtkWidget *buttonbox = rfm_hbutton_box_new ();
    gtk_widget_show (buttonbox);
    gtk_box_pack_start (GTK_BOX (vbox12), buttonbox, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (buttonbox), 5);
    gtk_button_box_set_layout (GTK_BUTTON_BOX (buttonbox), GTK_BUTTONBOX_END);
    gtk_box_set_spacing (GTK_BOX (buttonbox), 5);

    GtkWidget *button = rfm_dialog_button ("xffm/stock_cancel", _("Cancel"));
    gtk_container_add (GTK_CONTAINER (buttonbox), button);
    g_signal_connect (G_OBJECT (button), "clicked", 
	    G_CALLBACK (apply_action), GINT_TO_POINTER(RM_CANCEL));
    g_object_set_data(G_OBJECT(button), "dialog", dialog);
    g_object_set_data(G_OBJECT(dialog), "cancelbutton", button);

/****************/
    // This is now available to BSD with rm -P option
    button = rfm_dialog_button ("xffm/emblem_unreadable", _("Shred"));
    gtk_container_add (GTK_CONTAINER (buttonbox), button);
    g_signal_connect (G_OBJECT (button), "clicked", 
	    G_CALLBACK (apply_action), GINT_TO_POINTER(SHRED_YES));
    g_object_set_data(G_OBJECT(button), "dialog", dialog);

/****************/

    button = rfm_dialog_button ("xffm/stock_delete", _("Delete"));
    gtk_container_add (GTK_CONTAINER (buttonbox), button);
    g_signal_connect (G_OBJECT (button), "clicked", 
	    G_CALLBACK (apply_action), GINT_TO_POINTER(RM_YES));
    g_object_set_data(G_OBJECT(button), "dialog", dialog);


    g_signal_connect (dialog, "delete-event", G_CALLBACK (on_destroy_event), widgets_p);
    g_signal_connect (dialog, "destroy-event", G_CALLBACK (on_destroy_event), widgets_p);


    gtk_widget_realize (dialog);

    gtk_widget_grab_focus (button);

    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
    gtk_widget_show (dialog);


 
    return dialog;
}


static void * 
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
    /* dialog specifics */
    GtkWidget *togglebutton=g_object_get_data(G_OBJECT(dialog), "togglebutton");

    if(g_slist_length (list) < 2) {
	gtk_widget_hide(togglebutton);
    }
    gtk_main();
    return FALSE;
}

// This is a thread function, must have GDK mutex set for gtk commands...
// (it is called by a thread function)
static void
make_dialog (widgets_t *widgets_p, GSList *list, gboolean always) 
{
    NOOP("make_dialog\n");
    gchar *path = (gchar *) list->data;
    gchar *message;
    gchar *text;
    text=g_strdup_printf("<b>%s</b>\n",
	    _("Delete Files/Directories"));
    gchar *b = g_path_get_basename (path);
    gchar *q = rfm_utf_string (rfm_chop_excess (b));
    g_free (b);
    b=g_strdup_printf("<tt><i><big><b>%s</b></big></i></tt>", q);
    g_free(q);
    q=g_strdup_printf(_("Delete %s"), b);
    g_free(b);

    struct stat st;
    if (stat(path, &st) < 0 && lstat(path, &st)==0){
	//Broken symlink
	message = g_strconcat (q, "\n", "(", _("Broken symbolic link"), ")", NULL);
    } else {
	gchar *s1 = rfm_time_to_string (st.st_mtime);
	gchar *s2 = rfm_sizetag ((off_t) st.st_size, -1);
	message = g_strconcat (q, "\n", "(", s1, " ", s2, ")", NULL);
	g_free (s1);
	g_free (s2);
    }
    g_free (q);
    if (g_slist_length(list) > 1){
	gint remaining = g_slist_length(list) - 1;
			    
	gchar *plural_string = 
	    g_strdup_printf (ngettext ("%d more item", "%d more items", remaining), remaining);
	q = g_strdup_printf("%s\n\n<i>%s %s</i>", message, _("Selection:"), plural_string);
	g_free(message);
	message = q;

    }

    void **arg = (void **) malloc(5*sizeof(void *));
    if (!arg) g_error("malloc: %s\n", strerror(errno));
    arg[0] = widgets_p;
    arg[1] = text;
    arg[2] = message;
    arg[3] = list;
    arg[4] = GINT_TO_POINTER(always);
    rfm_context_function(create_remove_f, arg);

    NOOP ("REMOVE: dialog done\n");
}

/**************/




static void *
rm (widgets_t *widgets_p,  GSList *list) {
    if (!rfm_rational(RFM_MODULE_DIR, "settings", widgets_p, "RFM_RM_FLAGS", "options_dialog")){
	return NULL;
    } 
    if (!list){
	DBG("invalid call to rm()\n");
	return NULL;
    }
    if(g_slist_length(list)==0) {
	g_slist_free(list);
        NOOP ("subthread_p->selection_list)==0 for remove\n");
        return NULL;
    }

    const gchar *cflag=getenv("RFM_RM_FLAGS");
    errno=0;
    long long flag = strtoll(cflag, NULL, 16);
    if (errno){
	DBG("rm.i: strtollfailed \n");
    }
    gint interactive = 0;
    RfmProgramOptions *options_p=get_rm_options();
    gint j=0;
    for (; options_p && options_p->option; options_p++, j++){
	if (!(flag & (ONE64<<j))) continue;
	if (strcmp(options_p->option, "-i")==0) {
	    interactive = 1;
	    break;
	}
	if (strcmp(options_p->option, "-I")==0) {
	    interactive = -1;
	}
#ifdef GNU_RM
	if (options_p->choice_id == RFM_RM_interactive){
	    const gchar *item = getenv("RFM_RM_interactive");
	    if (strcmp(item,"always")==0) {
		interactive = 1;
		break;
	    } else interactive = -1;    
	}
#endif
    }
    if (interactive) {
	gboolean always = FALSE;
	if (interactive > 0) always=TRUE;
	make_dialog (widgets_p,  list, always); 
    } else {
	// proceed with rm. We are already in a thread here.
	// Only query for shred if in interactive mode.
	void **arg =(void **)malloc(3*sizeof(void *));
	if (!arg) g_error("malloc: %s\n", strerror(errno));
	arg[0]=widgets_p;
	arg[1]=list;
	arg[2]=GINT_TO_POINTER(MODE_RM);
	do_the_remove(arg);
    }
    return NULL;

}

