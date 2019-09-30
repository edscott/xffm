#ifndef XF_PASSWDRESPONSE_HH
#define XF_PASSWDRESPONSE_HH

namespace xf {

template <class Type>
class PasswordResponse {
    using pixbuf_c = Pixbuf<double>;
    using gtk_c = Gtk<double>;
    using util_c = Util<double>;
    
public:
    static void sendPassword(gchar **argv){
        gchar *p;

	gchar *string=NULL;
	if (argv[1]) {
            if (strncmp(argv[1], "Password", strlen("Password"))==0) 
                string = g_strdup_printf("%s:", _("Enter password"));
            else{
	        string = g_strdup(_(argv[1]));
            }

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
        auto title = getenv("RFM_ASKPASS_COMMAND");
        if (title && strlen(title)){
            gtk_window_set_title(GTK_WINDOW(dialog), title);
            setenv("RFM_ASKPASS_COMMAND", "", 1);            
        } 
        // No main window...
        //gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), FALSE);
        gint response = gtk_dialog_run (dialog);
        //gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), TRUE);
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
        //gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(mainWindow));
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
            gtk_label_set_markup  (label, _("Response Requested"));
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

        
        // Not honored by fvwm:
        //gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);
        gtk_widget_show (GTK_WIDGET(dialog));
        // Instead:
        Dialogs<Type>::placeDialog(GTK_WINDOW(dialog));

        return dialog;
    }


};

}
#endif
