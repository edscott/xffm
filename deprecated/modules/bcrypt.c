/*
 * Copyright (C) 2002-2013 Edscott Wilson Garcia
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "rfm.h"
/* this should be first 2 lines after headers: */
G_MODULE_EXPORT LIBRFM_MODULE 

gboolean keypress (GtkWidget *widget, GdkEvent  *event, gpointer data) {
    GtkWidget *dialog = data;
    const gchar *key = gtk_entry_get_text(GTK_ENTRY(g_object_get_data(G_OBJECT(dialog), "key")));
    const gchar *confirm = gtk_entry_get_text(GTK_ENTRY(g_object_get_data(G_OBJECT(dialog), "confirm")));
    if (strcmp(key, confirm)) gtk_widget_show(g_object_get_data(G_OBJECT(dialog), "sorry"));
    else gtk_widget_hide(g_object_get_data(G_OBJECT(dialog), "sorry"));

    if (strlen(key) < 8)  gtk_widget_show(g_object_get_data(G_OBJECT(dialog), "length"));
    else gtk_widget_hide(g_object_get_data(G_OBJECT(dialog), "length"));

    return FALSE;
}

static
GtkWidget *
confirm_dialog (widgets_t *widgets_p){
    rfm_global_t *rfm_global_p = rfm_global();

    GtkWidget *dialog;
    GtkWidget *button;
    GdkPixbuf *pixbuf=rfm_get_pixbuf("xffm/emblem_lock", 96);

    gchar *q = g_strdup_printf("<b>%s</b>: %s", _("Blowfish"),
	    _("Encryption Key Approval"));

    dialog = gtk_dialog_new_with_buttons ("rfm: bcrypt", 
            (rfm_global_p)?GTK_WINDOW(rfm_global_p->window):NULL,
            GTK_DIALOG_DESTROY_WITH_PARENT, NULL, NULL);
    gtk_window_set_type_hint(GTK_WINDOW(dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
    GtkWidget *cbox = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *box = rfm_hbox_new(FALSE, 2);
    gtk_box_pack_start(GTK_BOX(cbox),box, FALSE, FALSE,0);
    gtk_widget_show(box);
    if (pixbuf) {
        GtkWidget *image=gtk_image_new_from_pixbuf(pixbuf);
        gtk_widget_show(image);
        g_object_unref(pixbuf);
	gtk_box_pack_start(GTK_BOX(box),image, FALSE, FALSE,0);
    }
    GtkWidget *label = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(label), q);
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX(box),label, FALSE, FALSE,0);

#if 0
    // deprected message_dialog
    dialog = gtk_message_dialog_new (
	    NULL, GTK_DIALOG_DESTROY_WITH_PARENT,
	    GTK_MESSAGE_QUESTION, 
	    GTK_BUTTONS_NONE, 
	    NULL);
    gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG(dialog), q);
    g_free (q);
    if (pixbuf) {
	GtkWidget *image=gtk_image_new_from_pixbuf(pixbuf);
	g_object_unref(pixbuf);
	gtk_widget_show(image);
	gtk_message_dialog_set_image (GTK_MESSAGE_DIALOG(dialog), image);
    }
#endif
    
    if(widgets_p) {
	view_t *view_p=widgets_p->view_p;
        if(view_p && view_p->flags.type == DESKVIEW_TYPE) {
	    gtk_window_set_keep_above (GTK_WINDOW(dialog), TRUE);
	    gtk_window_stick (GTK_WINDOW(dialog));
	} else {   
            gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
            gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (rfm_global_p->window));
        } 
    } else {
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
    }
    GtkWidget *area = gtk_dialog_get_content_area (GTK_DIALOG(dialog));
    button =   gtk_check_button_new_with_label (_("-o     print output to standard out. Implies -r."));
    g_object_set_data(G_OBJECT(dialog), "o-option", button);
    gtk_widget_show(button);
    gtk_box_pack_start (GTK_BOX (area), button, FALSE, FALSE, 0);
    button =   gtk_check_button_new_with_label (_("-c     DO NOT compress files before encryption."));
    gtk_widget_show(button);
    gtk_box_pack_start (GTK_BOX (area), button, FALSE, FALSE, 0);
    g_object_set_data(G_OBJECT(dialog), "c-option", button);
    button =   gtk_check_button_new_with_label (_("-r     DO NOT remove input files after processing"));
    gtk_widget_show(button);
    gtk_box_pack_start (GTK_BOX (area), button, FALSE, FALSE, 0);
    g_object_set_data(G_OBJECT(dialog), "r-option", button);

    {
	GtkWidget *hbox = rfm_hbox_new(FALSE, 1);
	GtkWidget *label = gtk_label_new(_("Encryption key: "));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	GtkWidget *entry = gtk_entry_new ();
	g_object_set_data(G_OBJECT(dialog), "key", entry);
	gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, FALSE, 0);
	gtk_entry_set_visibility ((GtkEntry *) entry, FALSE);
	gtk_widget_show_all(hbox);
	gtk_box_pack_start (GTK_BOX (area), hbox, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT (entry), "key-release-event", G_CALLBACK (keypress),  dialog);
    }
    {
	GtkWidget *hbox = rfm_hbox_new(FALSE, 1);
	GtkWidget *label = gtk_label_new(_("Confirm:"));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	GtkWidget *entry = gtk_entry_new ();
	g_object_set_data(G_OBJECT(dialog), "confirm", entry);
	gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, FALSE, 0);
	gtk_entry_set_visibility ((GtkEntry *) entry, FALSE);
	gtk_widget_show_all(hbox);
	gtk_box_pack_start (GTK_BOX (area), hbox, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT (entry), "key-release-event", G_CALLBACK (keypress),  dialog);
    }
    {
	GtkWidget *label = gtk_label_new("");
	gtk_box_pack_start (GTK_BOX (area), label, FALSE, FALSE, 0);
	gchar *text = g_strdup_printf("<span foreground=\"red\" style=\"italic\">%s</span>", _("Sorry, passwords do not match"));
	gtk_label_set_markup(GTK_LABEL(label), text);
	g_free(text);
	g_object_set_data(G_OBJECT(dialog), "sorry", label);
    }
    {
	GtkWidget *label = gtk_label_new("");
	gtk_box_pack_start (GTK_BOX (area), label, FALSE, FALSE, 0);
	gchar *text = g_strdup_printf("<span foreground=\"red\" style=\"italic\">%s 8</span>", _("Minimum length:"));
	gtk_label_set_markup(GTK_LABEL(label), text);
	g_free(text);
	g_object_set_data(G_OBJECT(dialog), "length", label);
        gtk_widget_show(label);
    }


    gtk_dialog_add_button(GTK_DIALOG(dialog), _("Cancel"), GTK_RESPONSE_CANCEL);
    gtk_dialog_add_button(GTK_DIALOG(dialog), _("Ok"), GTK_RESPONSE_ACCEPT);
    return dialog;
}

static void *
confirm_f (gpointer data) {

    GSList *list = data;
    widgets_t *widgets_p = rfm_get_widget("widgets_p");
    gchar *bcrypt = g_find_program_in_path("bcrypt");
    if (!bcrypt){
        gchar *text = g_strdup_printf("%s bcrypt", _("Install"));
        gchar *text2 = g_strdup_printf(_("%s does not exist."), "bcrypt");
        gchar *fulltext = g_strdup_printf("%s\n\n%s\n", text2, text);
        rfm_confirm(widgets_p, GTK_MESSAGE_ERROR, fulltext, NULL,NULL);
        g_free(text);
        g_free(text2);
        g_free(fulltext);

        return NULL;
    }

    GtkWidget *dialog = confirm_dialog (widgets_p);
    if(!dialog) return NULL;
   
loop:    
    gtk_widget_show (dialog);
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));   
    if (response == GTK_RESPONSE_ACCEPT){
	const gchar *key = gtk_entry_get_text(GTK_ENTRY(g_object_get_data(G_OBJECT(dialog), "key")));
	const gchar *confirm = gtk_entry_get_text(GTK_ENTRY(g_object_get_data(G_OBJECT(dialog), "confirm")));
	if (strcmp(key, confirm)) goto loop;
	if (strlen(key) < 8) goto loop;
	gchar *argv[MAX_COMMAND_ARGS];
	gint i=0;
	argv[i++] = "bcrypt";
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_object_get_data(G_OBJECT(dialog), "o-option")))) argv[i++] = "-o";
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_object_get_data(G_OBJECT(dialog), "c-option")))) argv[i++] = "-c";
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_object_get_data(G_OBJECT(dialog), "r-option")))) argv[i++] = "-r";
	GSList *tmp = list;
	for (;tmp && tmp->data; tmp = tmp->next){
	    argv[i++] = tmp->data;
	}
	argv[i] = NULL;
	//gchar **p = argv;for (;p && *p; p++) TRACE("%s ", *p); TRACE( "\n");
	// encrypt/decrypt all paths in selection list.
	gint fd;
	rfm_thread_run_argv_with_stdin (widgets_p, argv, FALSE, &fd);	
	rfm_threadwait();
	if (write(fd, key, strlen(key)))
	    ; 
	if (write(fd, "\n", strlen("\n")))
	    ;
	rfm_threadwait();
	if (write(fd, key, strlen(key)))
	    ;
        if (write(fd, "\n", strlen("\n")))
	    ;
	close(fd);
    }
	    

    gtk_widget_hide (dialog);
    gtk_widget_destroy (dialog);


    return NULL;
}

G_MODULE_EXPORT
void *
bcrypt_dialog (void *data){
           //GSList *paths = data;
    rfm_context_function(confirm_f, data);
    return GINT_TO_POINTER(1);
}


