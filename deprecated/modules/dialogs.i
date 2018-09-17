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

#include "gplv3.h"

typedef struct response_t {
    const gchar * title_txt;// history-response
    const gchar * label_txt;// history-response
    const gchar * check_label; // history-response
    const gchar * folder;// history-response
    gint completion_type;// history-response
    GtkWindow *parent_window;

    union {// history-response/response
	const gchar * extra_txt;
	const gchar * default_txt;
    };
    union {// history-response/radio-response	
	gchar * history_file;
	gchar **radio_options;
    };
    union { // history-response/confirm
	gint filechooser_action; 
	gint type;
    };
    union { // history-response/confirm	
        gchar * flagfile; 
	const gchar * text;
    };
    union {// history-response/confirm	
        const gchar * path;
	const gchar * action_false;
    };
    union {// history-response/confirm
        const gchar * entry_text;
	const gchar * action_true;
    };
} response_t;

typedef struct filechooser_t {
    GtkWidget *parent;
    void *combo_info;
    int filechooser_action;
    const gchar *folder;
    GtkEntry *entry;
    void (*activate_func) (GtkEntry * entry, gpointer activate_user_data);
    gpointer activate_user_data;
    const gchar *title;

} filechooser_t;

static gpointer
thread_preload_f (gpointer data) {
    const gchar *folder = (const gchar *)data;
    if(folder && rfm_g_file_test_with_wait (folder, G_FILE_TEST_IS_DIR)) {
        GError *error = NULL;
        const gchar *path;
        GDir *dir = g_dir_open (folder, 0, &error);
        while((path = g_dir_read_name (dir)) != NULL) {
            /* just to avoid compiler optimization: */
            if(strcmp (path, ".") == 0)
                DBG ("This should never happen: strcmp (path, \".\") == 0\n");
        }
        g_dir_close (dir);
    }

    return NULL;
}

static void
preload (const gchar * folder) {
    // This thread is just to get the kernel cache hot.
    // Neither view nor window depend on the routine.
    rfm_thread_create ("thread_preload_f", thread_preload_f, (gpointer) (folder), FALSE);
}


static gint
extra_key_completionR (gpointer data) {
    extra_key_t *extra_key_p = (extra_key_t *) data;

    NOOP("extra_key_completionR\n");
    if(!extra_key_p) {
        DBG ("!extra_key_p\n");
        return FALSE;
    }
    if(!GTK_IS_ENTRY (extra_key_p->entry))
        return FALSE;
    extra_key_p->response = gtk_entry_get_text (extra_key_p->entry);

    rodent_recover_flags (extra_key_p);
    gtk_toggle_button_set_active ((GtkToggleButton *) extra_key_p->check1, extra_key_p->flag1);
    return FALSE;
}

static
    void
filechooser (GtkButton * button, gpointer user_data) {
    filechooser_t *filechooser_p = (filechooser_t *) user_data;
    const gchar *text=filechooser_p->title;
    if (text==NULL) {
      if(filechooser_p->filechooser_action == GTK_FILE_CHOOSER_ACTION_OPEN) {
        text = _("Select File");
      } else if(filechooser_p->filechooser_action == GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER) {
        text = _("Select Folder");
      } 
    }
      
      GtkWidget *dialog = gtk_file_chooser_dialog_new (text,
                                                     GTK_WINDOW (filechooser_p->parent),
                                                     filechooser_p->filechooser_action,
                                                     _("Cancel"),
                                                     GTK_RESPONSE_CANCEL,
                                                     _("Open"),
                                                     GTK_RESPONSE_ACCEPT,
                                                     NULL);
    
      // not necessary when using ~/.gtkbookmarks file:
      // rodent_set_filechooser_bookmarks(GTK_FILE_CHOOSER (dialog));

     gtk_file_chooser_set_use_preview_label(GTK_FILE_CHOOSER (dialog), FALSE);
    gtk_file_chooser_set_preview_widget_active (GTK_FILE_CHOOSER (dialog), FALSE);
    gtk_file_chooser_set_action (GTK_FILE_CHOOSER (dialog), filechooser_p->filechooser_action);
    //gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), filechooser_p->folder);

    // XXX gtk_file_chooser_set_show_hidden isn't working. Gtk bug?
    //     Maybe. It will not work if you set the current folder with
    //	   gtk_file_chooser_set_current_folder()
    gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER (dialog), TRUE);

    gint response = rfm_dialog_run_response(dialog);

    if(response == GTK_RESPONSE_ACCEPT) {
        char *filename;
        void *combo_info = filechooser_p->combo_info;
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
	if (rfm_void(RFM_MODULE_DIR, "combobox", "module_active")) {
	    COMBOBOX_set_entry (combo_info, filename);
	} else {
	    gtk_entry_set_text (filechooser_p->entry, filename);
	}
        NOOP ("Got %s\n", filename);
        g_free (filename);
	gtk_widget_destroy (dialog);
	if(filechooser_p->activate_func) {
	     NOOP("filechooser_p->activate_func \n");
            (*(filechooser_p->activate_func)) (filechooser_p->entry, filechooser_p->activate_user_data);
	}
	else NOOP("filechooser_p->activate_func is NULL\n");

    } else {
	gtk_widget_destroy (dialog);
    }

}

static gboolean 
response_delete(GtkWidget *dialog, GdkEvent *event, gpointer data){
    gtk_dialog_response (GTK_DIALOG(dialog),GTK_RESPONSE_CANCEL);
    return TRUE;
}

static void add_cancel_ok(widgets_t *widgets_p, GtkDialog *dialog){
    // button no
    GtkWidget *button =
        rfm_dialog_button ("xffm/stock_cancel", _("Cancel"));
    gtk_widget_show (button);
    gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_NO);
    g_object_set_data (G_OBJECT (dialog), "action_false_button", button);
    // button yes
    button = rfm_dialog_button ("xffm/stock_ok", _("Ok"));
    gtk_widget_show (button);
    g_object_set_data (G_OBJECT (dialog), "action_true_button", button);
    gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_YES);
}


static void
activate_entry (GtkEntry * entry, gpointer data) {
    GtkWidget *dialog = g_object_get_data(G_OBJECT(entry), "dialog");
    gtk_dialog_response (GTK_DIALOG(dialog),GTK_RESPONSE_YES);
}

static void
cancel_entry (GtkEntry * entry, gpointer data) {
    GtkWidget *dialog = g_object_get_data(G_OBJECT(entry), "dialog");
    gtk_dialog_response (GTK_DIALOG(dialog),GTK_RESPONSE_CANCEL);
}

static void *
get_response_f (gpointer data) {
    response_t *rh_p = data;
    if (g_thread_self() != rfm_get_gtk_thread()){
	g_error("get_response_f() is a main thread function\n");
    }
    widgets_t *widgets_p = rfm_get_widget ("widgets_p");
    gchar *response_txt = NULL;
    gint response = GTK_RESPONSE_NONE;
    GtkWidget *hbox, *label, *entry, *dialog;
    if(!rh_p->default_txt) rh_p->default_txt = "";
    dialog = gtk_dialog_new ();
    gtk_window_set_type_hint(GTK_WINDOW(dialog), GDK_WINDOW_TYPE_HINT_DIALOG);

    response_txt = NULL;
    if(widgets_p) {
	view_t *view_p=widgets_p->view_p;
        if(view_p && view_p->flags.type == DESKVIEW_TYPE) {
	    gtk_window_set_keep_above (GTK_WINDOW(dialog), TRUE);
	    gtk_window_stick (GTK_WINDOW(dialog));
	} else {   
            gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	    if (rh_p->parent_window){
		gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (rh_p->parent_window));
	    }
        } 
    } else {
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
    }
    gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);
    gtk_container_set_border_width (GTK_CONTAINER (dialog), 6);

    GtkWidget *title_label=NULL;
    if (rh_p->title_txt) {
	title_label = gtk_label_new ("");
	gchar *markup = g_strdup_printf("<b>%s</b>", rh_p->title_txt);
	gtk_label_set_markup(GTK_LABEL(title_label), markup);
	g_free(markup);
    }
    if(rh_p->label_txt)
        label = gtk_label_new (rh_p->label_txt);
    else
        label = gtk_label_new (_("Preparing"));

    hbox = rfm_hbox_new (TRUE, 6);
    GtkWidget *vbox = rfm_vbox_new (TRUE, 6);
    gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area(GTK_DIALOG (dialog))), vbox, FALSE, FALSE, 0);

    entry = gtk_entry_new ();

    if (title_label){
	gtk_box_pack_start (GTK_BOX (vbox), title_label, TRUE, TRUE, 0);
    }
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);
    gtk_widget_show_all (hbox);


    gtk_entry_set_text ((GtkEntry *) entry, rh_p->default_txt);

    g_object_set_data(G_OBJECT(entry),"dialog", dialog);
    g_signal_connect (G_OBJECT (entry), "activate", G_CALLBACK (activate_entry), dialog);

    add_cancel_ok(widgets_p, GTK_DIALOG (dialog));

    gtk_widget_realize (dialog);
    if(rh_p->title_txt){
	// This may or may not work, depending on the window manager.
	// That is why we duplicate above with markup.
        gtk_window_set_title (GTK_WINDOW (dialog), rh_p->title_txt);
    } else {
        gdk_window_set_decorations (gtk_widget_get_window(dialog), GDK_DECOR_BORDER);
    }

    g_signal_connect (G_OBJECT (dialog), "delete-event", G_CALLBACK (response_delete), dialog);
    /* show dialog and return */
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
    gtk_widget_show_all (dialog);
    response  = gtk_dialog_run(GTK_DIALOG(dialog));


    if(response == GTK_RESPONSE_YES) {
        const gchar *et = gtk_entry_get_text (GTK_ENTRY(entry));
        if(et && strlen (et)) {
            response_txt = g_strdup (et);
        }
    }
    gtk_widget_hide (dialog);
    gtk_widget_destroy (dialog);
    if(response_txt != NULL){
	g_strstrip (response_txt);
    }

    return response_txt;
}

static gchar *
get_response(const gchar * title_txt, const gchar * label_txt, const gchar * default_txt) {
    if (g_thread_self() == rfm_get_gtk_thread()){
	g_error("get_response() is a thread function\n");
    }
    response_t *rh_p = 
	(response_t *)malloc(sizeof(response_t));
    if (!rh_p) g_error("malloc: %s\n", strerror(errno));
    memset(rh_p, 0, sizeof(response_t));
    rh_p->title_txt = title_txt;
    rh_p->label_txt = label_txt;
    rh_p->default_txt = default_txt;
    if (rfm_global()){
        rh_p->parent_window = GTK_WINDOW(rfm_global()->window);
    } else {
        rh_p->parent_window = NULL;
    }
    gchar *result = rfm_context_function(get_response_f, rh_p);
    g_free(rh_p);
    return result;

    
}

static void *
get_radio_response_f (gpointer data) {
    response_t *rh_p = data;
    if (g_thread_self() != rfm_get_gtk_thread()){
	g_error("get_radio_response_f() is a main thread function\n");
    }
    NOOP( "get_radio_response\n");
    widgets_t *widgets_p = rfm_get_widget ("widgets_p");
    gint response = GTK_RESPONSE_NONE;
    GtkWidget *hbox, *label,  *dialog;

    
    gint radiobuttons = 0;
    gchar **p;
    for (p = rh_p->radio_options;p && *p; p++, radiobuttons++);

    GtkWidget *radio[radiobuttons];

    NOOP( "get_radio_response: got GDK mutex\n");
    dialog = gtk_dialog_new ();
    gtk_window_set_type_hint(GTK_WINDOW(dialog), GDK_WINDOW_TYPE_HINT_DIALOG);

    if(widgets_p) {
	view_t *view_p=widgets_p->view_p;
        if(view_p && view_p->flags.type == DESKVIEW_TYPE) {
	    gtk_window_set_keep_above (GTK_WINDOW(dialog), TRUE);
	    gtk_window_stick (GTK_WINDOW(dialog));
	} else {   
            gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	    if (rh_p->parent_window){
		gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (rh_p->parent_window));
	    }
        } 
    } else {
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
    }
    gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);
    gtk_container_set_border_width (GTK_CONTAINER (dialog), 6);

    if(rh_p->label_txt) label = gtk_label_new (rh_p->label_txt);
    else label = gtk_label_new ("get_radio_response");

    hbox = rfm_hbox_new (TRUE, 6);
    gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area(GTK_DIALOG (dialog))), hbox, FALSE, FALSE, 0);

    
    GtkWidget *vbox = rfm_vbox_new (TRUE, 6);
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start (GTK_BOX (hbox), scrolled_window, TRUE, TRUE, 0);

#if GTK_MAJOR_VERSION==3 && GTK_MINOR_VERSION>=8
    gtk_container_add(GTK_CONTAINER(scrolled_window), vbox);

#else
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window), vbox);
#endif

    gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, TRUE, 0);
    gtk_widget_set_size_request(scrolled_window, -1, 200);

    GSList *group=NULL;
    gint i=0;
    for (p = rh_p->radio_options;p && *p; p++, i++){
	radio[i] = gtk_radio_button_new_with_label(group, *p);
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON(radio[i]));
	g_object_set_data(G_OBJECT(radio[i]), "type", *p);
	gtk_box_pack_start (GTK_BOX (vbox), radio[i], TRUE, TRUE, 0);
    }



    gtk_widget_show_all (hbox);

    add_cancel_ok(widgets_p, GTK_DIALOG (dialog));
    gtk_widget_realize (dialog);

    if(rh_p->title_txt)
        gtk_window_set_title (GTK_WINDOW (dialog), rh_p->title_txt);
    else
        gdk_window_set_decorations (gtk_widget_get_window(dialog), GDK_DECOR_BORDER);

    g_signal_connect (G_OBJECT (dialog), "delete-event", G_CALLBACK (response_delete), dialog);
    /* show dialog and return */
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
    gtk_widget_show_all (dialog);
    NOOP( "get_radio_response: entering loop now\n");
    response  = gtk_dialog_run(GTK_DIALOG(dialog));
    //    gtk_main();
 
    const gchar *type = NULL;
    if(response == GTK_RESPONSE_YES) {
	for (i=0; i<radiobuttons; i++){
	    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio[i]))){
		type = g_object_get_data(G_OBJECT(radio[i]), "type");
		break;
	    }
	}
    }
    gtk_widget_hide (dialog);
    gtk_widget_destroy (dialog);

    gchar *result;
    if (!type) result = NULL;
    else result = g_strdup(type);

    return result;
}

G_MODULE_EXPORT
gchar *
get_radio_response (const gchar * title_txt, const gchar * label_txt, gchar **radio_options) {
    response_t *rh_p = 
	(response_t *)malloc(sizeof(response_t));
    if (!rh_p){
	g_error("malloc: %s\n", strerror(errno));
    }
    memset(rh_p, 0, sizeof(response_t));
    if (!rh_p) g_error("malloc: %s\n", strerror(errno));
    rh_p->title_txt = title_txt;
    rh_p->label_txt = label_txt;
    rh_p->radio_options = radio_options;

    gchar *result = rfm_context_function( get_radio_response_f, rh_p);
    g_free(rh_p);
    return result;
}


static void
toggle_activate (GtkToggleButton * togglebutton, gpointer user_data){
    extra_key_t *extra_key_p = (extra_key_t *) user_data;
    if(gtk_toggle_button_get_active (togglebutton))
        extra_key_p->flag1 = TRUE;
    else
        extra_key_p->flag1 = FALSE;
    rodent_save_flags (extra_key_p);
}

// This is exported to fstab plugin

static void *
get_response_history_f (gpointer data) {
    response_t *rh_p = data;
    if (g_thread_self() != rfm_get_gtk_thread()){
	g_error("get_response_history_f() is a main thread function\n");
    }
    TRACE("get_response_history_f() ...\n");
    
    widgets_t *widgets_p = rfm_get_widget ("widgets_p");
    extra_key_t extra_key;
    memset(&extra_key, 0, sizeof(extra_key_t));
    gint response = GTK_RESPONSE_NONE;
    GtkWidget *hbox, *label, *button, *dialog;
    GtkWidget *combo = NULL;
    void *combo_info = NULL;
    filechooser_t filechooser_v;
    filechooser_v.entry = NULL;

    if (rh_p->folder && chdir(rh_p->folder) < 0){
	DBG("cannot chdir(%s)\n", rh_p->folder);
    }
    dialog = gtk_dialog_new ();
    gtk_window_set_type_hint(GTK_WINDOW(dialog), GDK_WINDOW_TYPE_HINT_DIALOG);

    if(widgets_p) {
	view_t *view_p=widgets_p->view_p;
        if(view_p && view_p->flags.type == DESKVIEW_TYPE) {
	    gtk_window_set_keep_above (GTK_WINDOW(dialog), TRUE);
	    gtk_window_stick (GTK_WINDOW(dialog));
	} else {   
            gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	    if (rh_p->parent_window){
		gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (rh_p->parent_window));
	    }
        } 
    } else {
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
    }


    gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);
    gtk_container_set_border_width (GTK_CONTAINER (dialog), 6);

    combo =  rfm_combo_box_new_with_entry ();
    gtk_widget_set_size_request (GTK_WIDGET (combo), 350, -1);

    if (rh_p->extra_txt) {
	gchar *markup;
	label=gtk_label_new ("");
	markup = g_markup_printf_escaped ("<span style=\"italic\">%s</span>\n", rh_p->extra_txt);
	gtk_label_set_markup (GTK_LABEL (label), markup);
	g_free (markup);	
        hbox = rfm_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area(GTK_DIALOG (dialog))), hbox, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	gtk_widget_show_all(hbox);
    }

    if(rh_p->label_txt)
        label = gtk_label_new (rh_p->label_txt);
    else
        label = gtk_label_new (_("Preparing"));
    hbox = rfm_hbox_new (FALSE, 6);
    gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area(GTK_DIALOG (dialog))), hbox, FALSE, FALSE, 0);
    GtkWidget *vbox = rfm_vbox_new (FALSE, 6);
    gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
    vbox = rfm_vbox_new (FALSE, 6);
    gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);

    gtk_box_pack_start (GTK_BOX (vbox), (GtkWidget *) combo, FALSE, FALSE, 0);


    gboolean combobox_active =GPOINTER_TO_INT(rfm_void(RFM_MODULE_DIR, "combobox", "module_active"));
    if(combo_info == NULL) {
	    combo_info = COMBOBOX_init_combo (combo, rh_p->completion_type);
    } else {
	DBG("This is not happening... (get_response_history_f)\n");
	COMBOBOX_clear_history (combo_info);
    }
    COMBOBOX_set_quick_activate(combo_info, GINT_TO_POINTER(TRUE));
    GtkEntry *entry = COMBOBOX_get_entry_widget(combo_info);
    g_object_set_data(G_OBJECT(entry), "dialog", dialog);
    COMBOBOX_set_activate_function(combo_info, activate_entry);
    COMBOBOX_set_cancel_function(combo_info, cancel_entry);
    COMBOBOX_set_activate_user_data(combo_info, &response);
    COMBOBOX_set_cancel_user_data(combo_info, dialog);
    COMBOBOX_set_extra_key_completion_function(combo_info, extra_key_completionR);
    COMBOBOX_set_extra_key_completion_data(combo_info, &extra_key);


    COMBOBOX_read_history (combo_info, rh_p->history_file);
    COMBOBOX_set_combo (combo_info);

    
    if(rh_p->filechooser_action == GTK_FILE_CHOOSER_ACTION_OPEN || rh_p->filechooser_action == GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER) {
        filechooser_v.combo_info = combo_info;
        filechooser_v.parent = dialog;
        filechooser_v.folder = rh_p->folder;
        filechooser_v.title = rh_p->title_txt;
	if (combobox_active) {
	    filechooser_v.entry = GTK_ENTRY(COMBOBOX_get_entry_widget (combo_info));
	} else {
	    filechooser_v.entry = GTK_ENTRY(gtk_bin_get_child (GTK_BIN(combo)));

	}
        filechooser_v.activate_func = activate_entry;
        filechooser_v.activate_user_data = &response;

        filechooser_v.filechooser_action = rh_p->filechooser_action;
        preload (filechooser_v.folder);

        button = gtk_button_new ();
	GdkPixbuf *pixbuf=rfm_get_pixbuf("xffm/stock_directory", SIZE_BUTTON);
        GtkWidget *image;
	if (pixbuf) {
	    image = gtk_image_new_from_pixbuf (pixbuf);
	} else {
	    image = gtk_image_new_from_icon_name ("folder", GTK_ICON_SIZE_BUTTON);
	    //image = gtk_image_new_from_icon_name (GTK_STOCK_DIRECTORY, GTK_ICON_SIZE_BUTTON);
	}
	g_object_unref(pixbuf);
        gtk_button_set_image ((GtkButton *) button, image);
        vbox = rfm_vbox_new (FALSE, 6);
        gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);
        gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
        g_signal_connect (button, "clicked", G_CALLBACK (filechooser), (gpointer) (&filechooser_v));
        gtk_widget_show (button);
    }


    gtk_widget_show_all (hbox);
    if(rh_p->path) {
        gchar *type = MIME_type ((void *)rh_p->path, NULL);
        gchar *p = NULL;
	//   This may be borked if rh_p->path is a non local file (i.e., obexfs)
	//   so we verify on the parent folder.
        if(!type){
	    if (widgets_p && widgets_p->view_p && widgets_p->view_p->en &&
		    IS_LOCAL_TYPE(widgets_p->view_p->en->type)){
		type = MIME_magic (rh_p->path);
	    }
	} 
	if (!type) type = g_strdup(_("unknown"));
        p = MIME_command (type);
        g_free (type);
        if(p){
	    NOOP("COMBO: setting entry to %s\n", p);
	    if (combobox_active) {
		COMBOBOX_set_entry (combo_info, p);
	    } else {
		if (filechooser_v.entry) gtk_entry_set_text (filechooser_v.entry,p);
	    }
	    g_free(p);
	}

    }

    if (rh_p->entry_text) {
	NOOP ("COMBO: setting combo to %s\n", rh_p->entry_text);
	if (combobox_active) {
	    COMBOBOX_set_entry (combo_info, rh_p->entry_text);
 	} else {
	    if (filechooser_v.entry) gtk_entry_set_text (filechooser_v.entry,rh_p->entry_text);
	}
   }
    if (!rh_p->entry_text && ! rh_p->path) {
	COMBOBOX_set_default (combo_info);
	NOOP ("COMBO: setting combo to default (!rh_p->entry_text && ! rh_p->path)\n");
    }

    // Check button
    // This demands deprecated gtk_dialog_get_action_area since 
    // gtk_dialog_add_action_widget is only for widgets which emit 
    // response. Bleak workaround, use content area...
    //GtkWidget *action_area = gtk_dialog_get_action_area (GTK_DIALOG(dialog));
    GtkWidget *action_area = gtk_dialog_get_content_area (GTK_DIALOG(dialog));
    if(rh_p->check_label && rh_p->flagfile) {
        extra_key.check1 = (GtkWidget *) gtk_check_button_new_with_mnemonic (rh_p->check_label);
        g_signal_connect (extra_key.check1, "toggled", G_CALLBACK (toggle_activate), (gpointer) & extra_key);
        extra_key.entry = filechooser_v.entry;
        gtk_box_pack_start (GTK_BOX (action_area), GTK_WIDGET (extra_key.check1), FALSE, FALSE, 0);
        gtk_widget_show (extra_key.check1);
    }
    add_cancel_ok(widgets_p, GTK_DIALOG (dialog));
    gtk_widget_realize (dialog);
    if(rh_p->flagfile) {
        extra_key.flagfile = rh_p->flagfile;
        extra_key_completionR (&extra_key);
    } else
        extra_key.flagfile = NULL;

    if(rh_p->title_txt)
        gtk_window_set_title (GTK_WINDOW (dialog), rh_p->title_txt);
    else
        gdk_window_set_decorations (gtk_widget_get_window(dialog), GDK_DECOR_BORDER);

    g_signal_connect (G_OBJECT (dialog), "delete-event", G_CALLBACK (response_delete), dialog);
    /* show dialog and return */
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
    gtk_widget_show_all (dialog);
    response  = gtk_dialog_run(GTK_DIALOG(dialog));
    //    gtk_main();
    gchar *response_txt = NULL;
    if(response == GTK_RESPONSE_YES) {
        const gchar *et = NULL;
        if (combobox_active){
	    et = COMBOBOX_get_entry (combo_info);
        } else if (filechooser_v.entry) {
	    et = gtk_entry_get_text (filechooser_v.entry);
        }
        if(et && strlen (et)) {
            response_txt = (gchar *) malloc ((strlen (et) + 3) * sizeof (gchar));
	    if (!response_txt) {
		DBG("malloc: %s\n", strerror(errno));
		return NULL;
	    }
            memset (response_txt, 0, (strlen (et) + 3) * sizeof (gchar));
            strcpy (response_txt, et);
	    if(response_txt) g_strstrip (response_txt);
            COMBOBOX_save_to_history (rh_p->history_file, (char *)response_txt);
            if(rh_p->flagfile) rodent_save_flags (&extra_key);
        
	    if(rh_p->flagfile && extra_key.check1 && GTK_IS_TOGGLE_BUTTON(extra_key.check1)) {
		gboolean active = gtk_toggle_button_get_active ((GtkToggleButton *)
                                                            extra_key.check1);
		//NOOP("active=%d\n",active);
		if(active)
		    response_txt[strlen (response_txt) + 1] = 1;
	    }
        }
    }
    gtk_widget_hide (dialog);
    // cleanup combobox module objects:
    COMBOBOX_destroy_combo(combo_info);
    
    gtk_widget_destroy (dialog);
    if (chdir(g_get_home_dir()) < 0){
	DBG("cannot chdir(g_get_home_dir())\n");
    }


    return response_txt;
}

// This is exported to fstab plugin
G_MODULE_EXPORT
gchar *
get_response_history (const gchar * title_txt,
                             const gchar * label_txt,
                             const gchar * extra_txt,
                             gchar * history_file,
                             const gchar * path,
                             const gchar * entry_text,
                             gchar * flagfile, 
			     const gchar * check_label, 
			     gint filechooser_action, 
			     const gchar * folder,
			     gint completion_type) {

    response_t *rh_p = 
	(response_t *)malloc(sizeof(response_t));
    if (!rh_p) g_error("malloc: %s\n", strerror(errno));
    memset(rh_p, 0, sizeof(response_t));
    rh_p->title_txt = title_txt;
    rh_p->label_txt = label_txt;
    rh_p->extra_txt = extra_txt;
    rh_p->history_file = history_file;
    rh_p->path = path;
    rh_p->entry_text = entry_text;
    rh_p->flagfile = flagfile;
    rh_p->check_label = check_label;
    rh_p->filechooser_action = filechooser_action;
    rh_p->folder = folder;
    rh_p->completion_type = completion_type;
    if (rfm_global()){
        rh_p->parent_window = GTK_WINDOW(rfm_global()->window);
    } else {
        rh_p->parent_window = NULL;
    }


    gchar *result = rfm_context_function(get_response_history_f, rh_p);
    g_free(rh_p);
    return result;

}

static gboolean
save_last_mnt_point(const gchar *path, const gchar *mnt_path){
    gchar *mnt_history_new = g_build_filename (MOUNT_HISTORY_FILE".new", NULL);
    gchar *mnt_history = g_build_filename (MOUNT_HISTORY_FILE, NULL);
    FILE *history = fopen(mnt_history, "r");
    FILE *new_history = fopen(mnt_history_new, "w");
    if (!new_history){
	if (history) fclose(history);
	g_free(mnt_history);
	g_free(mnt_history_new);
	return FALSE;
    }
    gchar buffer[256];

    if (history) while (fgets(buffer, 255, history) && !feof(history)){
	if (!strchr(buffer, '|')) continue;
	gchar *test = g_strdup(buffer);
	*strchr(test, '|') = 0;
	g_strstrip(test);
	if (strcmp(test, path)) fprintf(new_history, "%s", buffer);
	g_free(test);
    }
    if (history) fclose(history);

    fprintf(new_history, "%s|%s\n", path, mnt_path);
    fclose(new_history);
    if (rename (mnt_history_new,  mnt_history)<0){
	DBG ("rename(%s, %s): %s\n", mnt_history_new,  mnt_history,
		strerror(errno));
	g_free(mnt_history);
	g_free(mnt_history_new);
	return FALSE;
    }
    g_free(mnt_history);
    g_free(mnt_history_new);
    return TRUE;
}


static gchar *
get_last_mnt_point(const gchar *path){
    gchar *mnt_history = g_build_filename (MOUNT_HISTORY_FILE, NULL);
    FILE *history = fopen(mnt_history, "r");
    g_free(mnt_history);
    if (!history) return NULL;
    gchar buffer[256];
    memset(buffer, 0, 256);
    while (fgets(buffer, 255, history) && !feof(history)){
	if (!strchr(buffer, '|')) continue;
	if (strchr(buffer, '\n')) *strchr(buffer, '\n') = 0;
	gchar *mnt_point = strchr(buffer, '|') + 1;
	*strchr(buffer, '|') = 0;
	g_strstrip(buffer);
	g_strstrip(mnt_point);
	if (strcmp(path, buffer) == 0){
	    gchar *result = g_strdup(mnt_point);
	    fclose(history);
	    return result;
	}
    }
    fclose(history);
    return NULL;
}

// This got moved from a static function in fstab module to a 
// exported function here (reason: use of get_response_history())
G_MODULE_EXPORT
void *
callback_mnt_point(record_entry_t *en) {
    widgets_t *widgets_p = rfm_get_widget("widgets_p");
        gchar *f = g_build_filename (MOUNT_DBH_FILE, NULL);
        gchar *device = g_path_get_basename (en->path);
        gchar *default_mount;
        if( en->tag && g_path_is_absolute (en->tag)){
            default_mount = g_strdup (en->tag);
	} else if ((default_mount=get_last_mnt_point(en->path))==NULL){
	    const gchar *user = getenv("USER");
	    if (!user) user="rodent";
            default_mount = g_build_filename (g_get_tmp_dir (), user, "mnt", device, NULL);
	}
        gchar *p = default_mount;
        for (;p && *p; p++){
            if (*p == ' ') *p = '-';
        }

	gchar *real_default = realpath(default_mount, NULL);
	if (real_default) {
	    if (strcmp(real_default, default_mount)==0){
		g_free(real_default);
	    } else {
		g_free(default_mount);
		default_mount = real_default;
	    }
	}


        g_free (device);
	gchar *extra_text = g_strdup_printf(_("Edit mount point for %s"), en->path);
	NOOP(stderr, "get_response_history: %s\n", _("Select mount point"));
        gchar *g = get_response_history (_("Select mount point"),
						_("Mount Point"), 
						extra_text,
						f,
                                                NULL,
                                                default_mount,
                                                NULL, NULL,
                                                GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                                "/",
						MATCH_FILE);
	g_free(extra_text);
        g_free (default_mount);
        g_free (f);
        if(g == NULL) return NULL;
        
        if(g_mkdir_with_parents (g, 0700) < 0) {
	    rfm_threaded_show_text(widgets_p);
            rfm_threaded_diagnostics (widgets_p, "xffm/stock_dialog-error",  NULL);
            rfm_threaded_diagnostics (widgets_p, "xffm_tag/stderr", g_strconcat("mkdir(", g, "): ",strerror (errno), "\n", NULL));
	    g_free(g);
            return NULL;
        }
	// is mount point already mounted?
	// This relies on fstab module (legacy)
	if (rfm_natural(PLUGIN_DIR, "fstab", g, "is_mounted")){
	    rfm_threaded_show_text(widgets_p);
            rfm_threaded_diagnostics (widgets_p, "xffm/stock_dialog-error", NULL);
            rfm_threaded_diagnostics (widgets_p, "xffm_tag/stderr", g_strconcat(g, ": ", _("Unable to mount location"), " (", _("address already in use"), ")", "\n", NULL));
	    g_free(g);
            return NULL;
	}
	save_last_mnt_point(en->path, g);
        return (void *)g;
}

  
static void * 
confirm_sudo(widgets_t *widgets_p, const gchar *tgt, 
       const gchar *failed, 
       const gchar *operation){
    

	gchar *altoperation=
	    g_strconcat(_("sudo"), " ", operation, NULL);
	gchar *text=g_strconcat(
	    _("Command:"), " \"", operation, "\"",
	    "\n\n",
	    failed,"\n",
	    "<span foreground=\"red\">",
               _("Permission denied"), 
            "</span>",
            ": ", tgt, "\n\n",
	     _("Try to approach a problem from different angles."), "\n\n",
	    "<b>", _("Do you want to retry?"), "</b>","\n",
	     	    _("Alternate:"), " \"", altoperation, "\"",
	    "\n",
	    NULL
		);
	gboolean retval= rfm_confirm(widgets_p,
                GTK_MESSAGE_QUESTION,
		text, _("No"), altoperation);
	g_free(altoperation);
	g_free(text);
	return GINT_TO_POINTER(retval);
}


static void *
about_dialog_f(gpointer data){
    void **arg=data;
    if (g_thread_self() != rfm_get_gtk_thread()){
	g_error("about_dialog_f() is a main thread function\n");
    }
    widgets_t *widgets_p = arg[0];
    const gchar *in_text = arg[1];
    gchar *tests=NULL;
    if (!tests) {
	tests=g_strdup_printf("%s (%s)", _("Tests"), _("development version"));
    }
    gchar *artists[] = {
	 "Francois Le Clainche <fleclainche at wanadoo.fr>",
	 "Pablo Morales Romero <pg.morales.romero@gmail.com>",
	 NULL};

#if 0

     gchar *authors[] = {"whatever", "whatever", "whatever", "whatever", "whatever", NULL};
#else
     gchar *authors[] = { "   Edscott Wilson Garcia", 
	 "",
	 _("Initial idea, basic architecture, much initial source code"),
	 "   Rasca, Berlin",
	 "",
	 tests,
	 "   Gregorio Inda",
	 "   Harold Aling",
	 "   Juri Hamburg",
	 "   Populus Tremula",
	 "",
	 _("Contributors"),
	 "   GNU cp:",
	 "      Torbjorn Granlund",
	 "      David MacKenzie",
	 "      Jim Meyering",
	 "   GNU mv:",
	 "      Mike Parker",
	 "      David MacKenzie",
	 "      Jim Meyering",
	 "   GNU touch:",
	 "      Paul Rubin",
	 "      Arnold Robbins",
	 "      Jim Kingdon",
	 "      David MacKenzie",
	 "      Randy Smith",
	 "   GNU rm:",
	 "      Paul Rubin",
	 "      David MacKenzie",
	 "      Richard M. Stallman",
	 "      Jim Meyering",
	 "   GNU shred:",
	 "      Colin Plumb",
	 "   libmagic:",
	 "      Mans Rullgard",
	 "      Christos Zoulas",
	 "      Guy Harris",
	 "      Rob McMahon",
	 "      Geoff Collyer",
	 "      John Gilmore",
	 "      Ian Darwin",
	 "   GNU ps:",
	 "      Branko Lankester",
	 "      Michael K. Johnson",
	 "      Michael Shields",
	 "      Charles Blake",
	 "      Albert Cahalan",
	 "      David Mossberger-Tang",
	 "",
	 _("Open Source:"),
	 "   Free Software Foundation, Inc.",
	 "   Nils Rennebarth",
	 "   Bill Wilson",
	 "   Dov Grobgeld",
	 "   Tadej Borovsak",
	 "",
	 _("Contributors to older versions:"),
	 "   Olivier Fourdan",
	 "   Jasper Huijsmans",
	 "   Benedikt Meurer",
	 "   Eduard Roccatello",
	 "   Brian Tarricone",
	 NULL };
#endif
//  XXX: gtk_about dialog requires static strings,
//  otherwise internal gtk will do memory overruns.
//  (this is a gtk limitation)
    static gchar *copyright=NULL;
    static gchar *gtk_info=NULL;
    static gchar *comments=NULL;
    static gchar *pname=NULL;
    static gchar *version=NULL;

     if (!pname)pname = g_strdup_printf("%s\n", in_text);
     if (!version)version = g_strdup_printf("librfm-%s", PACKAGE_VERSION);
     if (!copyright)copyright=g_strdup_printf("%s\n%s", COPYRIGHT, _("This is free software with ABSOLUTELY NO WARRANTY."));
     if (!gtk_info)gtk_info = 
	 g_strdup_printf("Built with GTK+-%d.%d.%d,linked with GTK+-%d.%d.%d.\n",
                     GTK_MAJOR_VERSION, GTK_MINOR_VERSION, GTK_MICRO_VERSION,
                     gtk_major_version, gtk_minor_version, gtk_micro_version);
     if (!comments)comments=g_strdup_printf("\
%s\n\
%s: %s\n\n\
%s\n\
%s\n",
_("Hello World"),
_("Suggestions, bug reports"), PACKAGE_BUGREPORT,
_("Rodent applications include fgr, rodent-fgr, rodent-diff \n\
rodent-iconmgr and rodent-fm. \n\
\n\
Fgr is a command line search tool.\n\
Rodent-fgr is a graphic front end to fgr command.\n\
Rodent-diff is a graphic front end to diff command.\n\
Rodent-iconmgr is a icontheme manager.\n\
Rodent-fm is a plugin enabled graphic shell front end.\n\
A fast, small and powerful file manager for GNU/BSD\n\
operating systems.\n\
\n\
Rodent applications are *not* for dummies. Emphasis is \n\
on ease of use for the advanced user, not the computer illiterate.\n\
\n\
"),
gtk_info
); 
     g_free(gtk_info);

     //     How do we tell the about dialog
     //     to stay above and be sticky for deskview?
     //     We cannot get the GtkWidget pointer if we do it the 
     //     easy way.

     GtkWidget *about= gtk_about_dialog_new();
     // set icon
     GdkPixbuf *pixbuf=rfm_get_pixbuf("xffm/stock_about", BIG_ICON_SIZE);
     if (pixbuf) {
         gtk_window_set_icon(GTK_WINDOW(about), pixbuf);
         g_object_unref(pixbuf);
     }

     gchar *title=g_strdup_printf("%s Rodent %s", _("About"), TAG);
     gtk_window_set_title(GTK_WINDOW(about), title);
     g_free(title);
    
     gtk_window_stick(GTK_WINDOW(about));
     gtk_window_set_keep_above(GTK_WINDOW(about), TRUE);
     if (widgets_p->view_p->flags.type != DESKVIEW_TYPE){
	 if (rfm_global() && rfm_global()->window) {
             gtk_window_set_transient_for(GTK_WINDOW(about), GTK_WINDOW(rfm_global()->window));
         }
	 gtk_window_set_modal(GTK_WINDOW(about), TRUE);
     }
     
     
     const gchar *rfm_team="rodent-translation-team";
     gtk_about_dialog_set_translator_credits (GTK_ABOUT_DIALOG(about), _(rfm_team));
	     // const gchar *translator_credits);
	     //

     pixbuf=rfm_get_pixbuf("rodent", BIG_ICON_SIZE);

     g_object_set(G_OBJECT(about),
	"artists",		artists,
        "authors",		authors,
	"comments",           comments,
	"copyright",          copyright,
	"license",            GPLV3,
	"logo",               pixbuf,
	"version",            version,
	"website",            "http://xffm.org/",
	"program-name",       pname,
	NULL);

     if (pixbuf) g_object_unref(pixbuf);

    gtk_widget_show_all(about);
    gtk_dialog_run(GTK_DIALOG(about));

    gtk_widget_hide (about);
    gtk_widget_destroy (about);
     return NULL;
}


