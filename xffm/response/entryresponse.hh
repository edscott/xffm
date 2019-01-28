#ifndef XF_ENTRYRESPONSE_HH
# define XF_ENTRYRESPONSE_HH
#include "completion/completion.hh"
#include "entryfileresponse.hh"
namespace xf
{
template <class Type> class Response;
template <class Type> class EntryResponse;
template <class Type>
class EntryFolderResponse: public EntryResponse<Type> {
    using gtk_c = Gtk<Type>;
public:

    EntryFolderResponse(GtkWindow *parent, const gchar *windowTitle, const gchar *icon):
        EntryResponse<Type>(parent, windowTitle, icon)
    {
	auto button = gtk_c::dialog_button ("folder-symbolic", NULL);
	auto vbox = gtk_c::vboxNew (FALSE, 6);
	gtk_box_pack_start (this->hbox_, GTK_WIDGET(button), FALSE, FALSE, 0);
	gtk_widget_show (GTK_WIDGET(button));
        g_signal_connect (G_OBJECT(button), 
                        "clicked", BUTTON_CALLBACK (folderChooser), 
                        (gpointer) this->entry());
    }
    
    static void
    folderChooser (GtkButton * button, gpointer data) {
        GtkEntry *entry = GTK_ENTRY(data);
        const gchar *text = _("Choose directory");
        EntryFileResponse<Type>::folderChooser(entry, text);
    }

};
template <class Type>
class EntryResponse {
    
    using gtk_c = Gtk<Type>;
    using pixbuf_c = Pixbuf<Type>;
    using util_c = Util<Type>;
    using pixbuf_icons_c = Icons<Type>;
    using page_c = Page<Type>;


    GtkLabel *responseLabel_;
    GtkLabel *entryLabel_;
    GtkLabel *checkboxLabel_;
    GtkEntry *entry_;
    GtkCheckButton *checkbutton_;
    GtkEntryCompletion *bashCompletion_;
    GtkButton *no_;

protected:
    GtkButton *yes_;
    GtkDialog *response_;
    GtkBox *hbox_;
    GtkListStore *bashCompletionStore_;

    GtkLabel *comboLabel(void) {return entryLabel_;}
        
    void connectBashCompletion(const gchar *wd, GtkEntry *entry){
        g_object_set_data(G_OBJECT(entry), "workDir", (void *)wd);
	gtk_entry_set_completion (entry, bashCompletion_);
	g_signal_connect (G_OBJECT(entry),
			      "key_release_event", 
			      KEY_EVENT_CALLBACK(EntryResponse<Type>::on_completion), 
			      (void *)bashCompletionStore_);
    }

    void setLabel(GtkLabel *label, const gchar *value){
        gtk_label_set_markup(label, value);
        gtk_widget_show(GTK_WIDGET(label));
    }

public:
    ~EntryResponse (void){
        gtk_widget_destroy(GTK_WIDGET(response_));
    }

    EntryResponse (GtkWindow *parent, const gchar *windowTitle, const gchar *icon){
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

	responseLabel_ = GTK_LABEL(gtk_label_new (""));
	gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET(responseLabel_), FALSE, FALSE, 0);

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
	entryLabel_ = GTK_LABEL(gtk_label_new (""));
	gtk_box_pack_start (GTK_BOX (hbox_), GTK_WIDGET(entryLabel_), FALSE, TRUE, 0);
	
        entry_ = GTK_ENTRY(gtk_entry_new ());
        gtk_box_pack_start (GTK_BOX (hbox_), GTK_WIDGET(entry_), TRUE, TRUE, 0);
        g_object_set_data(G_OBJECT(entry_),"response", response_);
        g_signal_connect (G_OBJECT (entry_), "activate", 
                ENTRY_CALLBACK (EntryResponse<Type>::activate_entry), (void *)response_);
        gtk_widget_show(GTK_WIDGET(hbox_));

        bashCompletion_ = gtk_entry_completion_new();
        gtk_entry_completion_set_popup_completion(bashCompletion_, TRUE);
        gtk_entry_completion_set_text_column (bashCompletion_, 0);
        gtk_entry_completion_set_minimum_key_length (bashCompletion_, 2);
        bashCompletionStore_ = gtk_list_store_new(1, G_TYPE_STRING);
        gtk_entry_completion_set_model (bashCompletion_, GTK_TREE_MODEL(bashCompletionStore_));

	checkbutton_ = GTK_CHECK_BUTTON(gtk_check_button_new_with_label(""));
	gtk_box_pack_start (GTK_BOX (vbox),GTK_WIDGET(checkbutton_), FALSE, FALSE, 0);
        // not needed yet:
        // g_object_set_data(G_OBJECT(checkbutton), "entryResponse", this);
	add_cancel_ok(GTK_DIALOG (response_));

	gtk_widget_realize (GTK_WIDGET(response_));
	if(windowTitle){
	    gtk_window_set_title (GTK_WINDOW (response_), windowTitle);
	} else {
	    gdk_window_set_decorations (
                    gtk_widget_get_window(GTK_WIDGET(response_)), GDK_DECOR_BORDER);
	}

	g_signal_connect (G_OBJECT (response_), "delete-event", G_CALLBACK (response_delete), response_);
        gtk_widget_grab_focus(GTK_WIDGET(entry_));
        gtk_widget_set_can_default (GTK_WIDGET(yes_), TRUE);
        gtk_widget_grab_default(GTK_WIDGET(yes_));
        return;
    }

    void setResponseLabel(const gchar *value){
        setLabel(responseLabel_, value);
    }
    void setEntryLabel(const gchar *value){
        setLabel(entryLabel_, value);
        gtk_widget_show(GTK_WIDGET(entry_));
    }

    GtkEntry *entry(void){
        return entry_;
    }

    void setEntryDefault(const gchar *value){
        if (!value) return;
        gtk_entry_set_text(entry_, value);
        gtk_editable_select_region (GTK_EDITABLE(entry_), 0, strlen(value));
        gtk_widget_show(GTK_WIDGET(entry_));
    }
			      
    void setEntryBashCompletion(const gchar *wd){
        connectBashCompletion(wd, entry_);
    }


    GtkCheckButton *checkButton(void){ return checkbutton_;}

    gboolean checkButtonState(void){
        return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbutton_));
    }

    void setCheckButton(const gchar *text){
        auto child = gtk_bin_get_child(GTK_BIN(checkbutton_));
        auto label = GTK_LABEL(gtk_label_new(""));
        gtk_label_set_markup(label, text);
        gtk_container_remove(GTK_CONTAINER(checkbutton_), child);
        gtk_container_add(GTK_CONTAINER(checkbutton_), GTK_WIDGET(label));
        gtk_widget_show(GTK_WIDGET(label));
        gtk_widget_show(GTK_WIDGET(checkbutton_));
    }

   
    void setCheckButton(gboolean state){
        // Set the toggle state.
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton_), state);
    }
    void setCheckButtonEntryCallback(void *clickAction){
        // Set the toggle action.
        g_signal_connect (G_OBJECT (checkbutton_), "clicked", 
		    BUTTON_CALLBACK(clickAction), 
                    (void *)entry_);
        gtk_widget_show(GTK_WIDGET(checkbutton_));
    }

    void setCheckButtonEntryCallback(void *clickAction, void * data){
        // Set the toggle action.
        g_signal_connect (G_OBJECT (checkbutton_), "clicked", 
		    BUTTON_CALLBACK(clickAction), 
                    data);
        gtk_widget_show(GTK_WIDGET(checkbutton_));
    }
    void setEntryCallback(void *changeAction){
        // Set the toggle action.
        g_signal_connect (G_OBJECT (entry_), "key-release-event", 
		    BUTTON_EVENT_CALLBACK(changeAction), this);
        gtk_widget_show(GTK_WIDGET(entry_));
    }

    gchar *getResponse(void){
        return g_strdup(gtk_entry_get_text (entry_));
    }
    gchar * 
    runResponse(void){
        /* show response_ and return */
	gtk_window_set_position(GTK_WINDOW(response_), GTK_WIN_POS_CENTER);
	gtk_widget_show (GTK_WIDGET(response_));
        gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), FALSE);
	gint response  = gtk_dialog_run(GTK_DIALOG(response_));
        gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), TRUE);
	//if (checkboxText) g_free(g_object_get_data(G_OBJECT(checkButton), "app"));
        gchar *responseTxt = NULL;
	if(response == GTK_RESPONSE_YES) {
            responseTxt = getResponse();
	}
	gtk_widget_hide (GTK_WIDGET(response_));
	if(responseTxt != NULL){
	    g_strstrip (responseTxt);
	}
        if (bashCompletionStore_) gtk_list_store_clear(bashCompletionStore_);
        
	return responseTxt;
    }

private:

    static gint
    on_completion (GtkWidget * widget, GdkEventKey * event, gpointer data) {
	auto store = (GtkListStore *)data;
	// get entry text
	auto entry = GTK_ENTRY(widget);
	const gchar *text = gtk_entry_get_text(entry);
	if (!text || strlen(text)<2) return FALSE;

	// Determine if Terminal check button should be depressed
	auto checkButton = GTK_TOGGLE_BUTTON(g_object_get_data(G_OBJECT(entry), "checkButton"));
	// FIXME gtk_toggle_button_set_active(checkButton, Mime<Type>::runInTerminal(text));
	// Hard coded exceptions:
	// nano vi and others...
	if (Mime<Type>::fixedInTerminal(text)){
	    gchar *a = Mime<Type>::baseCommand(text);
	    gtk_toggle_button_set_active(checkButton, TRUE);
	    Settings<Type>::setSettingInteger("Terminal", a, 1);
	    g_free(a);
	}

	// Get GSlist of bash completion

	auto wd = (const gchar *)g_object_get_data(G_OBJECT(entry), "workDir");
        if (!wd) wd = g_get_home_dir();
	
	auto slist = BaseCompletion<Type>::baseExecCompletionList(wd, text);
	// remove all old model entries
	gtk_list_store_clear(store);
	// add new entries from GSList
	GSList *p;
	GtkTreeIter iter;
	for (p=slist; p && p->data; p=p->next){
	    TRACE("completion list: %s\n", (const gchar *)p->data);
	    gtk_list_store_append (store, &iter);
	    gtk_list_store_set(store, &iter, 0, (const gchar *)p->data, -1);
	    g_free(p->data);
	}
	g_slist_free(slist);

        auto completion = gtk_entry_get_completion(GTK_ENTRY(widget));
        gtk_entry_completion_complete (completion);
        return FALSE;
    }

    static void
    activate_entry (GtkEntry * entry, gpointer data) {
	auto dialog = GTK_DIALOG(data);
	gtk_dialog_response (dialog,GTK_RESPONSE_YES);
    }

    static void
    cancel_entry (GtkEntry * entry, gpointer data) {
	auto dialog = GTK_DIALOG(g_object_get_data(G_OBJECT(entry), "dialog"));
	gtk_dialog_response (dialog,GTK_RESPONSE_CANCEL);
    }

    static gboolean 
    response_delete(GtkWidget *dialog, GdkEvent *event, gpointer data){
	gtk_dialog_response (GTK_DIALOG(dialog),GTK_RESPONSE_CANCEL);
	return TRUE;
    }

    void add_cancel_ok(GtkDialog *dialog){
	// button no
	no_ = gtk_c::dialog_button ("window-close-symbolic", _("Cancel"));
	gtk_widget_show (GTK_WIDGET(no_));
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), GTK_WIDGET(no_), GTK_RESPONSE_NO);
	g_object_set_data (G_OBJECT (dialog), "action_false_button", no_);
	yes_ = gtk_c::dialog_button ("system-run-symbolic", _("Ok"));
	gtk_widget_show (GTK_WIDGET(yes_));
	g_object_set_data (G_OBJECT (dialog), "action_true_button", yes_);
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), GTK_WIDGET(yes_), GTK_RESPONSE_YES);
    }
  
};
}
#endif

