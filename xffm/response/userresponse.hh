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

        this->setEntryBashCompletion("/");// XXX folder or file entry should be determined on .ini file (probably just inherit from comboresponse for multiple files options (selected files)...
        this->setEntryLabel(_("FIXME:"));
        this->setEntryDefault(g_get_home_dir());
	auto options = getKeys(file, "options");
        for (auto p=options; p && *p; p++){
            auto itemValue = getString(file, "options", *p);
            auto check = gtk_check_button_new();
	    auto option = g_strdup_printf("--%s", *p);
	    auto label = gtk_label_new(option);
	    auto hbox = gtk_c::hboxNew (FALSE, 6);
	    gtk_box_pack_start (GTK_BOX (this->vbox2_), GTK_WIDGET(hbox), FALSE, FALSE, 0);
	    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET(check), FALSE, FALSE, 0);
	    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET(label), FALSE, FALSE, 0);
	    if (strcmp(itemValue, "file")==0 || strcmp(itemValue, "folder")==0) {
		auto entry = GTK_ENTRY(gtk_entry_new ());
		gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET(entry), FALSE, FALSE, 0);
		auto button = gtk_c::dialog_button ("folder-symbolic", NULL);
		gtk_box_pack_start (hbox, GTK_WIDGET(button), FALSE, FALSE, 0);
		if (strcmp(itemValue, "folder")==0){
		    g_signal_connect (G_OBJECT(button), 
			    "clicked", BUTTON_CALLBACK (EntryFolderResponse<Type>::folderChooser), 
			    (gpointer) entry);
		    // FIXME bash folder completion
		} else {
		    g_signal_connect (G_OBJECT(button), 
			    "clicked", BUTTON_CALLBACK (EntryFileResponse<Type>::fileChooser), 
			    (gpointer) entry);
		    // FIXME bash file completion
		}
	    }
	    else if (strcmp(itemValue, "on")==0) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), TRUE);
	    }

            g_object_set_data(G_OBJECT(check), "option", option);
            gtk_widget_show_all(GTK_WIDGET(hbox));
            
            g_free(itemValue);
        }

    }
    static gchar **getKeys(const gchar *file, const gchar *group){
        auto keyFile = g_key_file_new();
        gboolean loaded = g_key_file_load_from_file(keyFile, file,(GKeyFileFlags) (0),NULL);
        if (!loaded) {
            ERROR("Cannot load %s\n", file);
            g_key_file_free(keyFile);
	    return NULL;
        }
	auto retval = g_key_file_get_keys (keyFile, group, NULL, NULL);
        g_key_file_free(keyFile);
        return retval;

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
