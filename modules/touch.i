#ifdef COPYRIGHT_INFORMATION
#include "gplv3.h"
#endif
/* touch thread routines, included by rodent_popup.c */
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

static void
touch_clean (GtkWidget * dialog){
    GSList *list=g_object_get_data(G_OBJECT(dialog), "list");
    if (!list) return;
    GSList *tmp;
    for (tmp=list; tmp && tmp->data; tmp=tmp->next){
	g_free(tmp->data);
    }
    g_slist_free(list);
    g_object_set_data(G_OBJECT(dialog), "list", NULL);
}
static void
touch_destroy (GtkWidget * dialog, gpointer data) {
    touch_clean(dialog);
    //gtk_widget_destroy(dialog);
    gtk_main_quit();
}

static void
touch_close (GtkWidget * button, gpointer data) {
    GtkWidget *dialog=g_object_get_data(G_OBJECT(button), "dialog");
    gtk_widget_destroy(dialog);
}

static  gchar *touch_option[]=
    {"-a","-h","-m","-c","-f",NULL};
static  gchar *touch_option_name[]=
    {"a_option","h_option","m_option","c_option","f_option",NULL};
static  gchar *touch_option2[]=
    {"-d","-r","-t","--time=",NULL};
static  gchar *touch_option2_name[]=
    {"d_option","r_option","t_option","time_option",NULL};

static void
touch_ok (GtkWidget * button, gpointer data) {
    gchar *filename=NULL;
    GtkWidget *dialog=g_object_get_data(G_OBJECT(button), "dialog");
    widgets_t *widgets_p=g_object_get_data(G_OBJECT(dialog), "widgets_p");
    gint i=0;
    gint j=0;
    gchar *argv[256];
    argv[j++] = "touch";
    GtkToggleButton *tbutton;
    for (i=0;  touch_option[i]; i++){
	tbutton=g_object_get_data(G_OBJECT(dialog), touch_option_name[i]);
	if (gtk_toggle_button_get_active(tbutton)){
	    argv[j++] = touch_option[i];
	}
    }
    for (i=0;  touch_option2[i]; i++){
	tbutton=g_object_get_data(G_OBJECT(dialog), touch_option2_name[i]);
	if (gtk_toggle_button_get_active(tbutton)){
	    if (strcmp(touch_option2[i],"-d")==0) {
		GtkEntry *entry=g_object_get_data(G_OBJECT(dialog), "date_string");
		argv[j++] = touch_option2[i];
		argv[j++] = (gchar *)gtk_entry_get_text (entry);
	    }
	    else if (strcmp(touch_option2[i],"-t")==0) {
		GtkEntry *entry=g_object_get_data(G_OBJECT(dialog), "t_stamp");
		argv[j++] = touch_option2[i];
		argv[j++] = (gchar *)gtk_entry_get_text (entry);
	    }
	    else if (strcmp(touch_option2[i],"-r")==0) {
		GtkFileChooser *chooser=g_object_get_data(G_OBJECT(dialog), "reference");
		filename=gtk_file_chooser_get_filename (chooser);
		argv[j++] = touch_option2[i];
		argv[j++] = filename;
	    }
	    else if (strcmp(touch_option2[i],"--time=")==0) {
		GtkComboBox *combobox=g_object_get_data(G_OBJECT(dialog), "time_spec");
		gint index =gtk_combo_box_get_active (combobox);
		if (index < 2) {
		    argv[j++] = "-a";
		} else {
		    argv[j++]="-m";
		}
	    }
	}
    }
    GList *selection_list=g_object_get_data(G_OBJECT(dialog), "list");
    GList *tmp=selection_list;
    gchar *last_item=NULL;
    for (;tmp && tmp->data; tmp=tmp->next){
	last_item=tmp->data;
	argv[j++]=tmp->data;
    }
    argv[j++] = NULL;
    view_t *view_p = widgets_p->view_p;
    g_mutex_lock(view_p->mutexes.status_mutex);
    gint status = view_p->flags.status;
    g_mutex_unlock(view_p->mutexes.status_mutex);
    if (status != STATUS_EXIT)  rfm_show_text (widgets_p); 

    // Not for dummies. On touch we will check for permissions on target
    // directory, and if permission do not allow the operation,
    // then we will try to do it with sudo.
    if (rfm_write_ok_path(last_item)){
	NOOP(stderr, "rfm_thread_run_argv touch\n");
        rfm_thread_run_argv (widgets_p, argv, FALSE);
    } else {
	NOOP(stderr, "confirm_sudo touch\n");
	if (confirm_sudo(widgets_p, last_item, _("write failed"), "touch")){
	    RFM_TRY_SUDO (widgets_p, argv, FALSE);
	}
    }
    
    
    g_free(filename);

}
static void
touch_help (GtkWidget * button, gpointer data) {
    GtkWidget *dialog=g_object_get_data(G_OBJECT(button), "dialog");
    widgets_t *widgets_p=g_object_get_data(G_OBJECT(dialog), "widgets_p");
    view_t *view_p = widgets_p->view_p;
    g_mutex_lock(view_p->mutexes.status_mutex);
    gint status = view_p->flags.status;
    g_mutex_unlock(view_p->mutexes.status_mutex);
    if (status == STATUS_EXIT) return;
    gchar *argv[]={"man", "touch", NULL};
    rfm_show_text (widgets_p); 
    rfm_thread_run_argv (widgets_p, argv, FALSE);
}

enum {
    TOUCH_HELP,
    TOUCH_OK,
    TOUCH_CANCEL,
    TOUCH_CLOSE,
    TOUCH_DESTROY
};


static void
touch_toggle (GtkWidget * button, gpointer data) {
    GtkWidget *widget=data;
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button))) {
	gtk_widget_set_sensitive(widget,TRUE);
    } else {
	gtk_widget_set_sensitive(widget,FALSE);
    }
}

static gboolean
test_has_symlink(GSList *selection_list){
    GSList *tmp=selection_list;
    for (;tmp && tmp->data; tmp=tmp->next){
	struct stat st;
	if (lstat((gchar *)tmp->data, &st)==0){
	    if (S_ISLNK(st.st_mode)) {
		return TRUE;
	    }
	}
    }
    return FALSE;
}

static void * 
touch_dialog(gpointer data){
    void **arg = data;
    widgets_t *view_widgets_p = arg[0];
    GSList *list = arg[1];
    g_free(arg);
    gboolean has_symlink=test_has_symlink(list);

    widgets_t widgets_v;
    widgets_t *widgets_p = &widgets_v;
    GtkWidget *dialog=gtk_dialog_new();
    gtk_window_set_type_hint(GTK_WINDOW(dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
   // transient, not for DESKVIEW_TYPE 
    view_t *view_p=view_widgets_p->view_p;
    if(view_p->flags.type == DESKVIEW_TYPE) {
        gtk_window_set_keep_above (GTK_WINDOW(dialog), TRUE);
        gtk_window_stick (GTK_WINDOW(dialog));
    } else {
	// we do not want dialog to be gtk style transient,
	// that is so much for dummies...
	//gtk_window_set_transient_for(GTK_WINDOW(dialog), 
        // rfm_global_t *rfm_global_p = rfm_global();
	//	GTK_WINDOW(rfm_global_p->window));
    }

    memset(widgets_p, 0, sizeof(widgets_t));
    widgets_p->view_p = view_p;
    
    GtkWidget *hbox;    
    GtkWidget *vbox = rfm_vbox_new (TRUE, 6);
    gtk_box_pack_start (
	    GTK_BOX (gtk_dialog_get_content_area(GTK_DIALOG (dialog))), 
	    vbox, TRUE, TRUE, 0);
    gtk_widget_show(vbox);
        
    GtkWidget *vpane = rfm_vpaned_new ();
    g_object_set_data(G_OBJECT(dialog), "vpane", vpane);
    // hack:
    widgets_p->paper = dialog;

    gtk_widget_show (vpane);
    gtk_box_pack_start (GTK_BOX (vbox), vpane, TRUE, TRUE, 0);
    gtk_paned_set_position (GTK_PANED (vpane), 1000);

    vbox = rfm_vbox_new (FALSE, 6);
    gtk_paned_pack1 (GTK_PANED (vpane), 
	    GTK_WIDGET (vbox), FALSE, TRUE);
    gtk_widget_show(vbox);

    hbox = rfm_hbox_new (FALSE, 6);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
    GtkWidget *vbox1 = rfm_vbox_new (FALSE, 6);
    gtk_box_pack_start (GTK_BOX (hbox), vbox1, FALSE, TRUE, 0);
    gchar *g=
            g_strdup_printf (ngettext ("%'d item selected", "%'d items selected",
                g_slist_length (list)),
                    g_slist_length (list));
    gchar *gg=g_strdup_printf("%s:",g);
    g_free(g);
    GtkWidget *label=gtk_label_new(gg);
    g_free(gg);
    gtk_box_pack_start (GTK_BOX (vbox1), label, FALSE, TRUE, 0);
    
    GtkWidget *vbox2 = rfm_vbox_new (FALSE, 6);
    gtk_box_pack_start (GTK_BOX (hbox), vbox2, FALSE, TRUE, 0);
    GSList *tmp;
    int count=0;
    gchar *label_text=NULL;
    gchar *old_text=NULL;
    for (tmp=list; tmp && tmp->data; tmp=tmp->next){
	gchar *basename = (count > 5)?
	    g_strdup(_("More...")):
	    g_path_get_basename((gchar *)tmp->data);
	old_text=label_text;
	label_text=(old_text)?g_strconcat(old_text,"\n",basename,NULL):g_strdup(basename);
	g_free(old_text);
	g_free(basename);
	if (count++ > 5) break;
    }
    label=gtk_label_new(label_text);
    g_free(label_text);
    gtk_box_pack_start (GTK_BOX (vbox2), label, FALSE, TRUE, 0);
    gtk_widget_show_all(hbox);

    hbox = rfm_hbox_new (FALSE, 6);
    g=g_strdup_printf("%s: ",_("Options"));
    label= gtk_label_new(g);
    g_free(g);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    gint i;
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    for (i=0; touch_option[i]; i++){
	GtkWidget *check_button=gtk_check_button_new_with_label(touch_option[i]);
	gtk_widget_set_can_focus(check_button, FALSE);
	gtk_box_pack_start (GTK_BOX (hbox), check_button, FALSE, FALSE, 0);
	if (strcmp(touch_option[i],"-a")==0 || strcmp(touch_option[i],"-m")==0) {
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button), TRUE);
	}
	if (strcmp(touch_option[i],"-c")==0 || strcmp(touch_option[i],"-f")==0) {
	    gtk_widget_set_sensitive(check_button, FALSE);
	}
	if (strcmp(touch_option[i],"-h")==0 ) {
	    // if no symlink, do this
	    if (!has_symlink) {
		gtk_widget_set_sensitive(check_button, FALSE);
	    }
	}
	g_object_set_data(G_OBJECT(dialog), touch_option_name[i], check_button);
    }
    gtk_widget_show_all(hbox);
    GtkWidget *date_string=NULL;
    GtkWidget *t_stamp=NULL;
    GtkWidget *time_spec=NULL;
    GtkWidget *reference=NULL;
    for (i=0; touch_option2[i]; i++){
	hbox = rfm_hbox_new (FALSE, 6);
	GtkWidget *check_button=gtk_check_button_new_with_label(touch_option2[i]);
	gtk_widget_set_can_focus(check_button, FALSE);
	gtk_box_pack_start (GTK_BOX (hbox), check_button, FALSE, TRUE, 0);

	if (strcmp(touch_option2[i],"-d")==0) {
	    //string
	    date_string=gtk_entry_new();
	    gtk_box_pack_start (GTK_BOX (hbox), date_string, TRUE, TRUE, 0);
	    gtk_widget_show(date_string);
	    gtk_widget_set_sensitive(date_string, FALSE);
	    g_signal_connect (G_OBJECT (check_button), "toggled", 
		    G_CALLBACK (touch_toggle),  date_string);
	    
	}
	else if (strcmp(touch_option2[i],"-t")==0) {
	    // string
	    t_stamp=gtk_entry_new();
	    gtk_box_pack_start (GTK_BOX (hbox), t_stamp, TRUE, TRUE, 0);
	    gtk_widget_show(t_stamp);
	    gtk_widget_set_sensitive(t_stamp, FALSE);
	    g_signal_connect (G_OBJECT (check_button), "toggled", 
		    G_CALLBACK (touch_toggle),  t_stamp);
	}
	else if (strcmp(touch_option2[i],"--time=")==0) {
	    // select list
#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
    // this is deprecated...
	    time_spec=gtk_combo_box_new_text();
	    gtk_combo_box_append_text(GTK_COMBO_BOX(time_spec), "atime");
	    gtk_combo_box_append_text(GTK_COMBO_BOX(time_spec), "access");
	    gtk_combo_box_append_text(GTK_COMBO_BOX(time_spec), "mtime");
	    gtk_combo_box_append_text(GTK_COMBO_BOX(time_spec), "modify");
#else
	    time_spec=gtk_combo_box_text_new();
	    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(time_spec), "atime");
	    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(time_spec), "access");
	    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(time_spec), "mtime");
	    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(time_spec), "modify");
#endif

	    gtk_combo_box_set_active (GTK_COMBO_BOX(time_spec), 0);
	    gtk_box_pack_start (GTK_BOX (hbox), time_spec, FALSE, TRUE, 0);
	    gtk_widget_show(time_spec);
	    gtk_widget_set_sensitive(time_spec, FALSE);
	    g_signal_connect (G_OBJECT (check_button), "toggled", 
		    G_CALLBACK (touch_toggle),  time_spec);
	}
	else if (strcmp(touch_option2[i],"-r")==0) {
	    // file selector combo
	    reference=
		gtk_file_chooser_button_new (_("Select A File"),GTK_FILE_CHOOSER_ACTION_OPEN);
	    gtk_box_pack_start (GTK_BOX (hbox), reference, TRUE, TRUE, 0);
	    gtk_widget_show(reference);
	    gtk_widget_set_sensitive(reference, FALSE);
	    g_signal_connect (G_OBJECT (check_button), "toggled", 
		    G_CALLBACK (touch_toggle),  reference);
	}
	g_object_set_data(G_OBJECT(dialog), touch_option2_name[i], check_button);

	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	gtk_widget_show_all(hbox);

    }

    hbox = rfm_hbox_new (TRUE, 6);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
    gtk_widget_show(hbox);

    {
        (widgets_p->diagnostics) = gtk_text_view_new ();
        GtkWidget *scrolledwindow5 = gtk_scrolled_window_new (NULL, NULL);
        gtk_widget_show (scrolledwindow5);
        gtk_widget_show ((widgets_p->diagnostics));
        gtk_paned_pack2 (GTK_PANED (vpane), scrolledwindow5, TRUE, TRUE);
        //gtk_container_add (GTK_CONTAINER (hbox), scrolledwindow5);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow5), 
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

        gtk_container_add (GTK_CONTAINER (scrolledwindow5), (widgets_p->diagnostics));
        gtk_container_set_border_width (GTK_CONTAINER ((widgets_p->diagnostics)), 2);
	gtk_widget_set_can_focus((widgets_p->diagnostics), FALSE);
        gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW ((widgets_p->diagnostics)), GTK_WRAP_WORD);
        gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW ((widgets_p->diagnostics)), FALSE);
    }
    
    g_object_set_data(G_OBJECT(dialog), "list", list);
    g_object_set_data(G_OBJECT(dialog), "widgets_p", widgets_p);
    g_object_set_data(G_OBJECT(dialog), "date_string", date_string);
    g_object_set_data(G_OBJECT(dialog), "t_stamp", t_stamp);
    g_object_set_data(G_OBJECT(dialog), "time_spec", time_spec);
    g_object_set_data(G_OBJECT(dialog), "reference", reference);

    GtkWidget *button = rfm_dialog_button ("xffm/stock_help", _("Help"));
      
    gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_NONE);
    g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (touch_help), NULL);
    g_object_set_data(G_OBJECT(button), "dialog", dialog);


    button = rfm_dialog_button ("xffm/stock_close", _("Close"));
    gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_CANCEL);
    g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (touch_close), NULL);
    g_object_set_data(G_OBJECT(button), "dialog", dialog);
   
    button = rfm_dialog_button ("xffm/stock_ok", _("Ok"));
    gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_YES);
    g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (touch_ok),  NULL);
    g_object_set_data(G_OBJECT(button), "dialog", dialog);

    /* set minimum window size: */
    gtk_widget_set_size_request (dialog, 500, -1);

    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_MOUSE);
    gtk_widget_realize (dialog);
    GdkPixbuf *pb =  rfm_get_pixbuf("xffm/stock_index", SIZE_ICON);
    gtk_window_set_icon (GTK_WINDOW (dialog), pb);
    g_object_unref(pb);
    gtk_window_set_title (GTK_WINDOW (dialog), "touch");

    g_signal_connect (G_OBJECT (dialog), "destroy", G_CALLBACK (touch_destroy), NULL);
    
    gtk_widget_show (dialog);
    gtk_main();
    return NULL;

}


