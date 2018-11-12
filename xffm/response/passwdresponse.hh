#ifndef XF_PASSWDRESPONSE_HH
#define XF_PASSWDRESPONSE_HH
#include "types.h"
#include "common/gtk.hh"
#include "common/pixbuf.hh"
#include "common/util.hh"
#include "dialog/dialog.hh"

namespace xf {
template <class Type>
class Response {
    using pixbuf_c = Pixbuf<double>;
    using gtk_c = Gtk<double>;
    using util_c = Util<double>;
    
public:
    static void sendPassword(gchar **argv){
        gchar *p;

	gchar *string=NULL;
	if (argv[1]) {
	    string = g_strdup(_(argv[1]));

	} else {
	    string = g_strdup_printf("%s:", _("Enter password"));
	}

	    
	
        p = getResponse (string, NULL, TRUE);

	g_free(string);
	if (p && strlen(p)) {
	    fprintf (stdout, "%s\n", p);
	    memset(p, 0, strlen(p));
	} else {
	    // No password, either cancel or close, then
	    // send interrupt signal to parent
	    pid_t parent = getppid();
	    kill(parent, SIGINT);
	}
	g_free(p);

	exit(0);
    }

private:
    static gchar *
    getResponse (const gchar * ptext, const gchar *default_value, gboolean hidden) {
        void *arg[]={
            (void *)ptext,
            (void *)default_value,
            (void *)GINT_TO_POINTER(hidden),
            NULL
        };
        auto passphrase = (gchar *)util_c::context_function(get_response_f, arg);
        return passphrase;
    }



    static gboolean 
    response_delete(GtkDialog *dialog, GdkEvent *event, gpointer data){
        gtk_dialog_response (dialog,GTK_RESPONSE_CANCEL);
        return TRUE;
    }

    
    static void
    entry_activate (GtkWidget * entry, GdkEventKey * event, gpointer data) {
        auto dialog = GTK_DIALOG(g_object_get_data(G_OBJECT(entry), "dialog"));
        gtk_dialog_response  (dialog, GTK_RESPONSE_YES);
        return;
    }


    /*static void add_cancel_ok(GtkDialog *dialog){
        // button no
        auto button = gtk_c::dialog_button("xffm/stock_cancel", _("Cancel"));
        gtk_widget_show (GTK_WIDGET(button));
        gtk_dialog_add_action_widget (dialog, GTK_WIDGET(button), GTK_RESPONSE_NO);
        g_object_set_data (G_OBJECT (dialog), "action_false_button", (void *)button);
        // button yes
        button = gtk_c::dialog_button ("xffm/stock_ok", _("Ok"));
        gtk_widget_show (GTK_WIDGET(button));
        g_object_set_data (G_OBJECT (dialog), "action_true_button",(void *) button);
        gtk_dialog_add_action_widget (GTK_DIALOG (dialog), GTK_WIDGET(button), GTK_RESPONSE_YES);
    }*/

    static void
    cancel_entry (GtkButton *button, gpointer data) {
        auto dialog = GTK_DIALOG(data);
        gtk_dialog_response (dialog,GTK_RESPONSE_CANCEL);
    }

    static void *
    get_response_f (void *data) {
        auto arg = (void **)data;
        auto ptext=(const gchar *)arg[0];
        auto default_text = (const gchar *)arg[1];
        gboolean hidden = GPOINTER_TO_INT(arg[2]);

        auto dialog = responseDialog (ptext, default_text, hidden);
        gint response = gtk_dialog_run (dialog);
        gtk_widget_hide (GTK_WIDGET(dialog));

        gchar *response_text = NULL;

        if(response == GTK_RESPONSE_YES){
            auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(dialog), "response_text"));
            const gchar *c = gtk_entry_get_text(entry);
            if (c) response_text = g_strdup(c);
            gtk_entry_set_text(entry, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
        } 
        gtk_widget_destroy (GTK_WIDGET(dialog));
        return response_text;
    }

    static GtkDialog *
    responseDialog (const gchar * ptext, const gchar *default_value, gboolean hidden) {

        auto dialog = GTK_DIALOG(gtk_dialog_new ());
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(mainWindow));
        //gtk_window_set_type_hint(GTK_WINDOW(dialog), GDK_WINDOW_TYPE_HINT_DIALOG);

        gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
        gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);
        //gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
        gtk_window_set_title(GTK_WINDOW (dialog), _("User Input"));
        gtk_window_set_keep_above(GTK_WINDOW (dialog), TRUE);

        gtk_widget_realize (GTK_WIDGET(dialog));

        auto hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_box_set_homogeneous(hbox, FALSE);
        gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
        gtk_box_pack_start (
                GTK_BOX (gtk_dialog_get_content_area(dialog)),
                GTK_WIDGET(hbox), TRUE, TRUE, 0);
        gtk_widget_show (GTK_WIDGET(hbox));

        auto label = GTK_LABEL(gtk_label_new(""));
        if(ptext){
            gtk_label_set_markup  (label, ptext);
        } else {
            gtk_label_set_markup  (label, _("response:"));
        }

        gtk_box_pack_start (hbox, GTK_WIDGET(label), TRUE, TRUE, 0);
        gtk_widget_show (GTK_WIDGET(label));

        auto bhbox=GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_box_set_homogeneous(bhbox, FALSE);
        gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area(GTK_DIALOG(dialog))), 
                GTK_WIDGET(bhbox), TRUE, TRUE, 0);
        gtk_widget_show (GTK_WIDGET(bhbox));

        auto vbox=GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        gtk_box_set_homogeneous(vbox, FALSE);
        gtk_box_pack_start (bhbox, GTK_WIDGET(vbox), TRUE, TRUE, 0);
        // put image here
        auto pixbuf = pixbuf_c::get_pixbuf("user-info-symbolic", BIG_ICON_SIZE);
//        auto pixbuf = pixbuf_c::get_pixbuf("user-info-symbolic",SIZE_ICON);
        auto image = GTK_IMAGE(gtk_image_new_from_pixbuf(pixbuf));
        //g_object_unref(pixbuf);
        gtk_box_pack_start (vbox, GTK_WIDGET(image), TRUE, TRUE, 0);

        gtk_widget_show (GTK_WIDGET(image));
        gtk_widget_show (GTK_WIDGET(vbox));
        vbox=GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        gtk_box_set_homogeneous(vbox, FALSE);
        gtk_box_pack_start (bhbox, GTK_WIDGET(vbox), TRUE, TRUE, 0);
        gtk_widget_show (GTK_WIDGET(vbox));

        hbox=GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_box_set_homogeneous(hbox, FALSE);
        gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
        gtk_box_pack_start (vbox, GTK_WIDGET(hbox), TRUE, TRUE, 0);
        gtk_widget_show (GTK_WIDGET(hbox));


        auto response_text = GTK_ENTRY(gtk_entry_new ());
        g_object_set_data(G_OBJECT(response_text), "dialog", dialog);
        gtk_box_pack_start (hbox, GTK_WIDGET(response_text), TRUE, TRUE, 0);
        gtk_entry_set_visibility ((GtkEntry *) response_text, (hidden)?FALSE:TRUE);
        g_signal_connect (G_OBJECT (response_text), "activate", ENTRY_CALLBACK (entry_activate), NULL);
        gtk_widget_show (GTK_WIDGET(response_text));
        g_object_set_data(G_OBJECT(dialog), "response_text", (void *)response_text);
        if (default_value) gtk_entry_set_text(response_text, default_value);

#if 0
        // this button is redundant since hands are on keyboard for password
        auto button = gtk_c::dialog_button ("emblem-greenball", _("Ok"));
        gtk_widget_show (GTK_WIDGET(button));
        gtk_dialog_add_action_widget (dialog, GTK_WIDGET(button), GTK_RESPONSE_YES);
#endif
        
        auto button = gtk_c::dialog_button ("window-close-symbolic", NULL);
        gtk_widget_show (GTK_WIDGET(button));
        gtk_box_pack_start (hbox, GTK_WIDGET(button), FALSE, FALSE, 0);
        g_signal_connect (G_OBJECT (button), "clicked", BUTTON_CALLBACK (cancel_entry), (void *)dialog);
        //gtk_dialog_add_action_widget (dialog, GTK_WIDGET(button), GTK_RESPONSE_NO);

        
        gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
        gtk_widget_show (GTK_WIDGET(dialog));

        return dialog;
    }


#if 0
    // FIXME: this is a different response function...

    static void
    activate_entry (GtkEntry * entry, gpointer data) {
        auto dialog = GTK_DIALOG(g_object_get_data(G_OBJECT(entry), "dialog"));
        gtk_dialog_response (dialog,GTK_RESPONSE_YES);
    }
    static gchar *
    getResponse(const gchar * title_txt, const gchar * label_txt, const gchar * default_txt) {
        auto rh_p = (response_t *)calloc(1, sizeof(response_t));
        if (!rh_p) g_error("calloc: %s\n", strerror(errno));
        rh_p->title_txt = title_txt;
        rh_p->label_txt = label_txt;
        rh_p->default_txt = default_txt;
        /*if (rfm_global()){
            rh_p->parent_window = GTK_WINDOW(rfm_global()->window);
        } else {
            rh_p->parent_window = NULL;
        }*/n
        gchar *result = util_c::context_function(get_response_f, rh_p);
        g_free(rh_p);
        return result;

        
    }
    static void *
    get_response_f (gpointer data) {
        response_t *rh_p = data;
        gchar *response_txt = NULL;
        gint response = GTK_RESPONSE_NONE;
        if(!rh_p->default_txt) rh_p->default_txt = "";
        auto dialog = GTK_DIALOG(gtk_dialog_new ());
        gtk_window_set_type_hint(GTK_WINDOW(dialog), GDK_WINDOW_TYPE_HINT_DIALOG);

        response_txt = NULL;
     
        gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
        if (rh_p->parent_window){
            gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (rh_p->parent_window));
        }

        gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);
        gtk_container_set_border_width (GTK_CONTAINER (dialog), 6);

        GtkLabel *title_label=NULL;
        if (rh_p->title_txt) {
            title_label = GTK_LABEL(gtk_label_new (""));
            gchar *markup = g_strdup_printf("<b>%s</b>", rh_p->title_txt);
            gtk_label_set_markup(title_label, markup);
            g_free(markup);
        }
        GtkLabel *label;
        if(rh_p->label_txt)
            label = GTK_LABEL(gtk_label_new (rh_p->label_txt));
        else
            label = GTK_LABEL(gtk_label_new (_("Preparing")));

        auto hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        auto vbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area(dialog)), 
                GTK_WIDGET(vbox), FALSE, FALSE, 0);

        auto entry = GTK_ENTRY(gtk_entry_new ());

        if (title_label){
            gtk_box_pack_start (vbox, GTK_WIDGET(title_label), TRUE, TRUE, 0);
        }
        gtk_box_pack_start (vbox, GTK_WIDGET(hbox), FALSE, FALSE, 0);
        gtk_box_pack_start (hbox, GTK_WIDGET(label), TRUE, TRUE, 0);
        gtk_box_pack_start (hbox, GTK_WIDGET(entry), TRUE, TRUE, 0);
        gtk_widget_show_all (GTK_WIDGET(hbox));


        gtk_entry_set_text (entry, rh_p->default_txt);

        g_object_set_data(G_OBJECT(entry),"dialog", (void *)dialog);
        g_signal_connect (G_OBJECT (entry), "activate", ENTRY_CALLBACK (activate_entry), (void *)dialog);

        add_cancel_ok(dialog);

        gtk_widget_realize (dialog);
        if(rh_p->title_txt){
            // This may or may not work, depending on the window manager.
            // That is why we duplicate above with markup.
            gtk_window_set_title (GTK_WINDOW (dialog), rh_p->title_txt);
        } else {
            gdk_window_set_decorations (gtk_widget_get_window(dialog), GDK_DECOR_BORDER);
        }

        g_signal_connect (G_OBJECT (dialog), "delete-event", RESPONSE_EVENT_CALLBACK (response_delete), dialog);
        /* show dialog and return */
        gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
        gtk_widget_show_all (dialog);
        response  = gtk_dialog_run(dialog);


        if(response == GTK_RESPONSE_YES) {
            const gchar *et = gtk_entry_get_text (entry);
            if(et && strlen (et)) {
                response_txt = g_strdup (et);
            }
        }
        gtk_widget_hide (GTK_WIDGET(dialog));
        gtk_widget_destroy (GTK_WIDGET(dialog));
        if(response_txt != NULL){
            g_strstrip (response_txt);
        }

        return response_txt;
    }




    /*
    static void *
    dialog_run_response_f(void *data){
        GtkDialog *dialog = data;
        gint response = gtk_dialog_run (dialog);
        return GINT_TO_POINTER(response);
    }

    // Thread OK:
    gint
    rfm_dialog_run_response(GtkWidget *dialog){
        return GPOINTER_TO_INT(
                rfm_context_function(dialog_run_response_f, dialog));
    }
    */
#endif

///////////////////////////////////////////////////////////////////////////
#if 0
    // FIXME: this is for a confirmSudo operation retry
    //        please note that the confirm_f is NOT the same
    //        as that which is detailed for bcrypt confirm.
    // Thread OK:
    gboolean confirmSudo(widgets_t *widgets_p,
           const gchar *tgt, 
           const gchar *failed, 
           const gchar *operation){

            gchar *altoperation=
                g_strconcat(_("sudo"), " ", operation, NULL);
            gchar *text=g_strconcat(
                _("Command:"), " \"", operation, "\"",
                "\n\n",
                failed,"\n",
                _("Permission denied"), ": ", tgt, "\n\n",
                 _("Try to approach a problem from different angles."), "\n\n",
                _("Do you want to retry?"), "\n",
                        _("Alternate:"), " \"", altoperation, "\"",
                "\n",
                NULL
                    );
            gboolean retval= confirm(widgets_p, GTK_MESSAGE_QUESTION,
                    text, _("No"), altoperation);
            g_free(altoperation);
            g_free(text);
            return retval;
    }
#endif
/////////////////////////////////////////////////////////////
#if 0
// FIXME: add this to a bcrypt template
// For bcrypt dialog
    // Thread OK:
    static gboolean confirm ( 
            // type: GTK_MESSAGE_INFO, GTK_MESSAGE_WARNING, GTK_MESSAGE_QUESTION, GTK_MESSAGE_ERROR
            gint type,
    //	GtkMessageType type,
            const gchar * text, 
            const gchar * action_false,  // if NULL, button not shown
            const gchar * action_true   // if NULL, "Ok" button shown
            ) {
        void *result;
        void *arg[] = {
            GINT_TO_POINTER(type),
            (void *)text,
            (void *)action_false,
            (void *)action_true
        };
            result = util_c::context_function(confirm_f, arg);
        return GPOINTER_TO_INT(result);	
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
#endif




};

}
#endif
