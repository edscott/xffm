#ifndef XF_USERRESPONSE_HH
# define XF_USERRESPONSE_HH

namespace xf
{
template <class Type>
class UserResponse {
    using gtk_c = Gtk<Type>;
    GtkListStore *bashCompletionStore_;
    GtkBox *hbox_;
    GtkButton *yes_;
    GtkButton *no_;
    GtkDialog *response_;

public:
    UserResponse(GtkWindow *parent, const gchar *windowTitle, const gchar *icon) {
        bashCompletionStore_ = NULL;
	response_ = GTK_DIALOG(gtk_dialog_new ());
	gtk_window_set_type_hint(GTK_WINDOW(response_), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_window_set_modal (GTK_WINDOW (response_), TRUE);
	gtk_window_set_transient_for (GTK_WINDOW (response_), GTK_WINDOW (parent));
	gtk_window_set_resizable (GTK_WINDOW (response_), TRUE);
	gtk_container_set_border_width (GTK_CONTAINER (response_), 6);

	auto vbox = gtk_c::vboxNew (FALSE, 6);
        gtk_widget_show(GTK_WIDGET(vbox));
	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area(GTK_DIALOG (response_))), GTK_WIDGET(vbox), FALSE, FALSE, 0);

	hbox_ = gtk_c::hboxNew (FALSE, 6);

	//responseLabel_ = GTK_LABEL(gtk_label_new (""));
	//gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET(responseLabel_), FALSE, FALSE, 0);

	gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET(hbox_), FALSE, FALSE, 0);


	if (icon){
	    GdkPixbuf *p = Icons<Type>::get_theme_pixbuf(icon, -48);
	    if (p){
		auto image = GTK_IMAGE(gtk_image_new_from_pixbuf(p));
		gtk_box_pack_start (GTK_BOX (hbox_), GTK_WIDGET(image), FALSE, FALSE, 0);
		gtk_widget_show(GTK_WIDGET(image));
		TRACE("Loaded icon %s\n", icon);
	    } else {
		TRACE("Cannot load icon %s\n", icon);
	    }
	}
	/*entryLabel_ = GTK_LABEL(gtk_label_new (""));
	gtk_box_pack_start (GTK_BOX (hbox_), GTK_WIDGET(entryLabel_), FALSE, TRUE, 0);
	
        entry_ = GTK_ENTRY(gtk_entry_new ());
        gtk_box_pack_start (GTK_BOX (hbox_), GTK_WIDGET(entry_), TRUE, TRUE, 0);
        g_object_set_data(G_OBJECT(entry_),"response", response_);
        g_signal_connect (G_OBJECT (entry_), "activate", 
                ENTRY_CALLBACK (EntryResponse<Type>::activate_entry), (void *)response_);
        */
        gtk_widget_show(GTK_WIDGET(hbox_));

        /*bashCompletion_ = gtk_entry_completion_new();
        gtk_entry_completion_set_popup_completion(bashCompletion_, TRUE);
        gtk_entry_completion_set_text_column (bashCompletion_, 0);
        gtk_entry_completion_set_minimum_key_length (bashCompletion_, 2);
        bashCompletionStore_ = gtk_list_store_new(1, G_TYPE_STRING);
        gtk_entry_completion_set_model (bashCompletion_, GTK_TREE_MODEL(bashCompletionStore_));

	checkbutton_ = GTK_CHECK_BUTTON(gtk_check_button_new_with_label(""));
	gtk_box_pack_start (GTK_BOX (vbox),GTK_WIDGET(checkbutton_), FALSE, FALSE, 0);

        timeoutProgress_ = GTK_PROGRESS_BAR(gtk_progress_bar_new());
	gtk_box_pack_end (GTK_BOX (vbox),GTK_WIDGET(timeoutProgress_), FALSE, FALSE, 0);
*/
	add_cancel_ok(GTK_DIALOG (response_));

	gtk_widget_realize (GTK_WIDGET(response_));
	if(windowTitle){
	    gtk_window_set_title (GTK_WINDOW (response_), windowTitle);
	} else {
	    gdk_window_set_decorations (
                    gtk_widget_get_window(GTK_WIDGET(response_)), GDK_DECOR_BORDER);
	}

	g_signal_connect (G_OBJECT (response_), "delete-event", G_CALLBACK (response_delete), this);
	//g_signal_connect (G_OBJECT (entry_), "key-press-event", G_CALLBACK (progressReset), timeoutProgress_);
        //gtk_widget_grab_focus(GTK_WIDGET(entry_));
        gtk_widget_set_can_default (GTK_WIDGET(yes_), TRUE);
        gtk_widget_grab_default(GTK_WIDGET(yes_));
        return;
        
    }

    static GtkButton *userbutton(const gchar *file){
        auto keyFile = g_key_file_new();
        gboolean loaded = g_key_file_load_from_file(keyFile, file,(GKeyFileFlags) (0),NULL);
        if (!loaded) {
            ERROR("Cannot load %s\n", file);
	    return NULL;
        }
	GError *error = NULL;
	auto icon = g_key_file_get_string (keyFile, "userbutton", "icon", &error);
	if (error){
	    ERROR("userbutton():: %s\n", error->message);
	    icon = g_strdup("system-run");
	}
	auto tooltip = g_key_file_get_string (keyFile, "userbutton", "tooltip", &error);
	if (error){
	    ERROR("userbutton():: %s\n", error->message);
	    tooltip = g_strdup("User button");
	}
	auto button = Gtk<Type>::newButton(icon, tooltip);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(openUserDialog), g_strdup(tooltip));
	g_free(icon);
	
	return button;
    }
private:
        
    static void openUserDialog(GtkButton *button, void *data){
        auto title = (const gchar *) data;
        auto userResponse = new(UserResponse<Type>)(mainWindow, title, "run");
        userResponse->runResponse();
        delete(userResponse);
    }

    static void
    cancel (GtkButton *button, gpointer data) {
        auto dialog = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "dialog"));
        gtk_widget_hide(dialog);
        gtk_widget_destroy(dialog);
	// FIXME: cleanup here causes crash on double free or corruption.
        //delete((UserResponse<Type> *)data);
    }

    void add_cancel_ok(GtkDialog *dialog){
	// button no
	no_ = gtk_c::dialog_button ("window-close-symbolic", _("Cancel"));
        g_object_set_data(G_OBJECT(no_), "dialog", dialog);
	gtk_widget_show (GTK_WIDGET(no_));
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), GTK_WIDGET(no_), GTK_RESPONSE_NO);
	g_signal_connect (G_OBJECT (no_), "clicked", G_CALLBACK (cancel), this);
	yes_ = gtk_c::dialog_button ("system-run-symbolic", _("Ok"));
	gtk_widget_show (GTK_WIDGET(yes_));
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), GTK_WIDGET(yes_), GTK_RESPONSE_YES);
    }
    
    static gboolean 
    response_delete(GtkWidget *dialog, GdkEvent *event, gpointer data){
        cancel(NULL, data);
	return TRUE;
    }

    void 
    runResponse(void){
        /* show response_ and return */
	gtk_window_set_type_hint(GTK_WINDOW(this->response_), GDK_WINDOW_TYPE_HINT_NORMAL);
        //GDK_WINDOW_TYPE_HINT_DIALOG
	gtk_window_set_modal (GTK_WINDOW (this->response_), FALSE);
	gtk_window_set_position(GTK_WINDOW(this->response_), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_widget_show (GTK_WIDGET(this->response_));
        //gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), FALSE);

	return;
    }

};
}


#endif
