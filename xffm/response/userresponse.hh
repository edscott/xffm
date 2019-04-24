#ifndef XF_USERRESPONSE_HH
# define XF_USERRESPONSE_HH
#include "entryfolderresponse.hh"

namespace xf
{
template <class Type>
class UserResponse: public EntryFolderResponse<Type>{
    using gtk_c = Gtk<Type>;

public:


    UserResponse(GtkWindow *parent, const gchar *file):
     EntryFolderResponse<Type>(parent, "title", "run")
    {

	g_signal_connect (G_OBJECT (this->no_), "clicked", G_CALLBACK (cancel), this);
	g_signal_connect (G_OBJECT (this->dialog()), "delete-event", G_CALLBACK (response_delete), this);

        this->setEntryBashCompletion("/");
        this->setEntryLabel(_("Workdir:"));
        this->setEntryDefault(g_get_home_dir());
        auto checkboxes = getInt(file, "checkbox", "items");
        for (int i=0; i<checkboxes; i++){
            auto item = g_strdup_printf("item%d",i);
            auto itemValue = getString(file, "checkbox", item);
            DBG("checkbox %d: %s --> %s\n", i, item, itemValue);
            auto check = gtk_check_button_new_with_label(itemValue);
            g_free(item);
            item = g_strdup_printf("item%dDefault",i);
            auto active = (getInt(file, "checkbox", item) == 1);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), active);

            g_object_set_data(G_OBJECT(check), "option", itemValue);
	    gtk_box_pack_start (GTK_BOX (this->vbox2_), GTK_WIDGET(check), FALSE, FALSE, 0);
            gtk_widget_show(GTK_WIDGET(check));
            
            g_free(item);
        }


#if 0
        gchar *icon = getIcon(file);
        gchar *title = getTooltip(file);

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
	    GdkPixbuf *p = Icons<Type>::get_theme_pixbuf(icon, 48);
	    if (p){
		auto image = GTK_IMAGE(gtk_image_new_from_pixbuf(p));
		gtk_box_pack_start (GTK_BOX (hbox_), GTK_WIDGET(image), FALSE, FALSE, 0);
		gtk_widget_show(GTK_WIDGET(image));
		TRACE("Loaded icon %s\n", icon);
	    } else {
		TRACE("Cannot load icon %s\n", icon);
	    }
	}
	auto vbox2 = gtk_c::vboxNew (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (hbox_), GTK_WIDGET(vbox2), FALSE, FALSE, 0);
	gtk_widget_show (GTK_WIDGET(vbox2));
	auto hbox = gtk_c::hboxNew (FALSE, 1);
	gtk_box_pack_start (GTK_BOX (vbox2), GTK_WIDGET(hbox), FALSE, FALSE, 0);
	gtk_widget_show (GTK_WIDGET(hbox));
        
	entryLabel_ = GTK_LABEL(gtk_label_new (""));
	gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET(entryLabel_), FALSE, TRUE, 0);
	
        entry_ = GTK_ENTRY(gtk_entry_new ());
        gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET(entry_), TRUE, TRUE, 0);
	gtk_widget_show (GTK_WIDGET(entry_));
        g_object_set_data(G_OBJECT(entry_),"response", response_);

	auto button = gtk_c::dialog_button ("folder-symbolic", NULL);
	gtk_box_pack_start (hbox, GTK_WIDGET(button), FALSE, FALSE, 0);
	gtk_widget_show (GTK_WIDGET(button));
        g_signal_connect (G_OBJECT(button), 
                        "clicked", BUTTON_CALLBACK (folderChooser), 
                        (gpointer) entry_);

        
        /*g_signal_connect (G_OBJECT (entry_), "activate", 
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
	if(title){
	    gtk_window_set_title (GTK_WINDOW (response_), title);
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
#endif   
    }

    static gint getInt(const gchar *file, const gchar *group, const gchar *item){
        auto keyFile = g_key_file_new();
        gboolean loaded = g_key_file_load_from_file(keyFile, file,(GKeyFileFlags) (0),NULL);
        if (!loaded) {
            ERROR("Cannot load %s\n", file);
            g_key_file_free(keyFile);
	    return -1;
        }
	GError *error = NULL;
	auto retval = g_key_file_get_integer (keyFile, group, item, &error);
	if (error){
	    //ERROR("userbutton():: %s\n", error->message);
            g_key_file_free(keyFile);
            g_error_free(error);
	    return -1;
	}
        g_key_file_free(keyFile);
        return retval;
    }

    static gchar *getString(const gchar *file, const gchar *group, const gchar *item){
        auto keyFile = g_key_file_new();
        gboolean loaded = g_key_file_load_from_file(keyFile, file,(GKeyFileFlags) (0),NULL);
        if (!loaded) {
            ERROR("Cannot load %s\n", file);
            g_key_file_free(keyFile);
	    return NULL;
        }
	GError *error = NULL;
	auto retval = g_key_file_get_string (keyFile, group, item, &error);
	if (error){
	    DBG("userbutton():: %s\n", error->message);
            g_key_file_free(keyFile);
            g_error_free(error);
	    return NULL;
	}
        g_key_file_free(keyFile);
        return retval;
    }

    static gchar *getIcon(const gchar *file){
        auto icon = getString(file, "userbutton", "icon");
        if (!icon) icon = g_strdup("system-run");
        return icon;
    }

    static gchar *getTooltip(const gchar *file){
        auto tooltip = getString(file, "userbutton", "tooltip");
        if (!tooltip) tooltip = g_strdup("User button");
        return tooltip;
    }

    static GtkButton *userbutton(const gchar *file){
        auto icon = getIcon(file);
        auto tooltip = getTooltip(file);
	auto button = Gtk<Type>::newButton(icon, tooltip);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(openUserDialog), g_strdup(file));
	g_free(icon);
	
	return button;
    }
private:
    static void
    folderChooser (GtkButton * button, gpointer data) {
        GtkEntry *entry = GTK_ENTRY(data);
        const gchar *text = _("Choose directory");
        EntryFileResponse<Type>::folderChooser(entry, text);
    }
        
    static void openUserDialog(GtkButton *button, void *data){
        auto file = (const gchar *) data;
        auto userResponse = new(UserResponse<Type>)(mainWindow, file);
        userResponse->runResponse();
    }

    static void
    cancel (GtkButton *button, gpointer data) {
        auto object = (UserResponse<Type> *)data;
        /* //done automatically when object is destroyed:
        auto dialog = GTK_WIDGET(object->dialog());
        gtk_widget_hide(dialog);
        gtk_widget_destroy(dialog);
        while(gtk_events_pending()) gtk_main_iteration();*/
        delete(object);
    }
/*
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
  */  
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
        //gtk_main();
	return;
    }

};
}


#endif
