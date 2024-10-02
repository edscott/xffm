#ifndef XF_ENTRYRESPONSE_HH
# define XF_ENTRYRESPONSE_HH
namespace xf
{


template <class Type>
class EntryResponse {
    
    using gtk_c = Gtk<Type>;
    using pixbuf_c = Pixbuf<Type>;
    using util_c = Util<Type>;
    using page_c = Page<Type>;

    GtkLabel *responseLabel_;
    GtkLabel *entryLabel_;
    GtkLabel *checkboxLabel_;
    GtkEntry *entry_;
    GtkCheckButton *checkbutton_;
    GtkEntryCompletion *bashCompletion_;

protected:
    GtkButton *yes_;
    GtkButton *no_;
    GtkWindow *response_;
    GtkBox *hbox_;
    GtkBox *vbox2_;
    GtkListStore *bashCompletionStore_;

    GtkLabel *comboLabel(void) {return entryLabel_;}
        
    void connectBashCompletion(const gchar *wd, GtkEntry *entry){
        g_object_set_data(G_OBJECT(entry), "workDir", (void *)wd);
        gtk_entry_set_completion (entry, bashCompletion_);
        g_signal_connect (G_OBJECT(entry),
                              "key_release_event", 
                              KEY_EVENT_CALLBACK(EntryResponse<Type>::onExecCompletion), 
                              (void *)bashCompletionStore_);
    }
        
    void connectBashFileCompletion(const gchar *wd, GtkEntry *entry){
        TRACE("connectBashFileCompletion(%s, %p)\n", wd, entry);
        g_object_set_data(G_OBJECT(entry), "workDir", (void *)wd);
        gtk_entry_set_completion (entry, bashCompletion_);
        g_signal_connect (G_OBJECT(entry),
                              "key_release_event", 
                              KEY_EVENT_CALLBACK(EntryResponse<Type>::onFileCompletion), 
                              (void *)bashCompletionStore_);
    }

    void setLabel(GtkLabel *label, const gchar *value){
        gtk_label_set_markup(label, value);
        gtk_widget_show(GTK_WIDGET(label));
    }

public:
    GtkWindow *dialog(void){return response_;}
    GtkBox *vbox2(void) {return vbox2_;}
    
    
    ~EntryResponse (void){
        if (bashCompletionStore_) gtk_list_store_clear(bashCompletionStore_);
        MainDialog = NULL;
        
        gtk_widget_destroy(GTK_WIDGET(response_));
    }



    EntryResponse (GtkWindow *parent, const gchar *windowTitle, const gchar *icon){
        bashCompletionStore_ = NULL;
        response_ = GTK_WINDOW(gtk_window_new (GTK_WINDOW_TOPLEVEL));
//        response_ = GTK_DIALOG(gtk_dialog_new ());
        MainDialog = GTK_WINDOW(response_);
        gtk_window_set_type_hint(GTK_WINDOW(response_), GDK_WINDOW_TYPE_HINT_DIALOG);
        //gtk_window_set_modal (GTK_WINDOW (response_), TRUE);
        gtk_window_set_transient_for (GTK_WINDOW (response_), GTK_WINDOW (parent));
        gtk_window_set_resizable (GTK_WINDOW (response_), TRUE);
        gtk_container_set_border_width (GTK_CONTAINER (response_), 6);

        auto vbox = gtk_c::vboxNew (FALSE, 6);
        gtk_widget_show(GTK_WIDGET(vbox));
        compat<Type>::boxPackStart (GTK_BOX (gtk_dialog_get_content_area(GTK_DIALOG (response_))), GTK_WIDGET(vbox), FALSE, FALSE, 0);

        auto hbox = gtk_c::hboxNew (FALSE, 6);

        responseLabel_ = GTK_LABEL(gtk_label_new (""));
        compat<Type>::boxPackStart (GTK_BOX (vbox), GTK_WIDGET(responseLabel_), FALSE, FALSE, 0);

        compat<Type>::boxPackStart (GTK_BOX (vbox), GTK_WIDGET(hbox), FALSE, FALSE, 0);


        if (icon){
            GdkPixbuf *p = Pixbuf<Type>::getPixbuf(icon, -48);
            if (p){
                auto image = GTK_IMAGE(gtk_image_new_from_pixbuf(p));
                compat<Type>::boxPackStart (GTK_BOX (hbox), GTK_WIDGET(image), FALSE, FALSE, 0);
                gtk_widget_show(GTK_WIDGET(image));
                TRACE("Loaded icon %s\n", icon);
            } else {
                TRACE("Cannot load icon %s\n", icon);
            }
        }

        vbox2_ = gtk_c::vboxNew (FALSE, 6);
        gtk_widget_show(GTK_WIDGET(vbox2_));
        compat<Type>::boxPackStart (GTK_BOX (hbox), GTK_WIDGET(vbox2_), FALSE, FALSE, 0);

        hbox_ = gtk_c::hboxNew (FALSE, 6);
        gtk_widget_show(GTK_WIDGET(hbox));
        compat<Type>::boxPackStart (GTK_BOX (vbox2_), GTK_WIDGET(hbox_), FALSE, FALSE, 0);
        
        entryLabel_ = GTK_LABEL(gtk_label_new (""));
        compat<Type>::boxPackStart (GTK_BOX (hbox_), GTK_WIDGET(entryLabel_), FALSE, TRUE, 0);
        
        entry_ = GTK_ENTRY(gtk_entry_new ());
        compat<Type>::boxPackStart (GTK_BOX (hbox_), GTK_WIDGET(entry_), TRUE, TRUE, 0);
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
        compat<Type>::boxPackStart (GTK_BOX (vbox),GTK_WIDGET(checkbutton_), FALSE, FALSE, 0);

        auto buttonBox = gtk_c::hboxNew (FALSE, 6);
        gtk_widget_show(GTK_WIDGET(buttonBox));
        compat<Type>::boxPackStart (GTK_BOX (vbox2_), GTK_WIDGET(buttonBox), FALSE, FALSE, 0);

        // not needed yet:
        // g_object_set_data(G_OBJECT(checkbutton), "entryResponse", this);
        add_cancel_ok(GTK_DIALOG (response_), GTK_BOX(buttonBox));

        gtk_widget_realize (GTK_WIDGET(response_));
        setTitle(windowTitle);

        g_signal_connect (G_OBJECT (response_), "delete-event", 
            G_CALLBACK (ResponseClass<Type>::on_destroy_event), GINT_TO_POINTER(-1));
        //g_signal_connect (G_OBJECT (response_), "delete-event", G_CALLBACK (response_delete), response_);
        //g_signal_connect (G_OBJECT (entry_), "key-press-event", G_CALLBACK (progressReset), timeoutProgress_);
        gtk_widget_grab_focus(GTK_WIDGET(entry_));
        gtk_widget_set_can_default (GTK_WIDGET(yes_), TRUE);
        gtk_widget_grab_default(GTK_WIDGET(yes_));
        return;
    }

    void setTitle(const gchar *windowTitle){
        if(windowTitle){
            gtk_window_set_title (GTK_WINDOW (response_), windowTitle);
        } else {
            gdk_window_set_decorations (
                    gtk_widget_get_window(GTK_WIDGET(response_)), GDK_DECOR_BORDER);
        }
    }


    void setInLineCompletion(gboolean state){
        gtk_entry_completion_set_popup_completion(bashCompletion_, !state);
        gtk_entry_completion_set_inline_completion(bashCompletion_, state);
        gtk_entry_completion_set_inline_selection(bashCompletion_, state);
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
                              
    void setEntryBashFileCompletion(const gchar *wd){
        TRACE("setEntryBashFileCompletion(%s)\n", wd);
        connectBashFileCompletion(wd, entry_);
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


    gint 
    runResponseSetup(gint timeout){
        /* show response_ and return */
        gtk_window_set_position(GTK_WINDOW(response_), GTK_WIN_POS_CENTER_ON_PARENT);
        gtk_widget_show (GTK_WIDGET(response_));
        //if (mainWindow) gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), FALSE);
        //Dialogs<Type>::placeDialog(GTK_WINDOW(response_));        
            
        gint response  = gtk_dialog_run(GTK_DIALOG(response_));

        
        gtk_widget_hide(GTK_WIDGET(response_));
        //if (mainWindow) gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), TRUE);
        //while (gtk_events_pending())gtk_main_iteration();
        return response;
    }
 

private:

    static gint
    onExecCompletion (GtkWidget * widget, GdkEventKey * event, gpointer data) {
        return on_completion (widget, event, data, FALSE) ;
    }
    static gint
    onFileCompletion (GtkWidget * widget, GdkEventKey * event, gpointer data) {
        return on_completion (widget, event, data, TRUE);
    }
    static gint
    on_completion (GtkWidget * widget, GdkEventKey * event, gpointer data, gboolean fileCompletion) 
    {
        TRACE("on_completion()\n");
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
        if (Run<Type>::fixedInTerminal(text)){
            gchar *a = Run<Type>::baseCommand(text);
            gtk_toggle_button_set_active(checkButton, TRUE);
            Settings<Type>::setInteger("Terminal", a, 1);
            g_free(a);
        }

        // Get GSlist of bash completion

        auto wd = (const gchar *)g_object_get_data(G_OBJECT(entry), "workDir");
        if (!wd) wd = g_get_home_dir();
        
        GSList *slist;
        if (fileCompletion) {
            slist = BaseCompletion<Type>::baseFileCompletionList(wd, text);
        } else {
            slist = BaseCompletion<Type>::baseExecCompletionList(wd, text);
        }
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

 /*   static void
    activate_entry (GtkEntry * entry, gpointer data) {
        auto dialog = GTK_DIALOG(data);
        gtk_dialog_response (dialog,GTK_RESPONSE_YES);
    }

    static void
    cancel_entry (GtkEntry * entry, gpointer data) {
        auto dialog = GTK_DIALOG(g_object_get_data(G_OBJECT(entry), "dialog"));
        gtk_dialog_response (dialog,GTK_RESPONSE_CANCEL);
    }*/

    /*static gboolean 
    response_delete(GtkWidget *dialog, GdkEvent *event, gpointer data){
        gtk_dialog_response (GTK_DIALOG(dialog),GTK_RESPONSE_CANCEL);
        return TRUE;
    }*/

    void add_cancel_ok(GtkDialog *dialog, GtkBox *buttonBox){
        // button no
        no_ = gtk_c::dialog_button ("window-close-symbolic", _("Cancel"));
        gtk_widget_show (GTK_WIDGET(no_));
        g_signal_connect (G_OBJECT (no_), "clicked", 
                G_CALLBACK (ResponseClass<Type>::responseAction), GINT_TO_POINTER(-1));
        compat<Type>::boxPackStart (buttonBox, GTK_WIDGET(no_), FALSE, FALSE, 0);
        
        //gtk_dialog_add_action_widget (GTK_DIALOG (dialog), GTK_WIDGET(no_), GTK_RESPONSE_NO);
        //g_object_set_data (G_OBJECT (dialog), "action_false_button", no_);

        yes_ = gtk_c::dialog_button ("system-run-symbolic", _("Ok"));
        gtk_widget_show (GTK_WIDGET(yes_));
        g_signal_connect (G_OBJECT (yes_), "clicked", 
                G_CALLBACK (ResponseClass<Type>::responseAction), GINT_TO_POINTER(1));
        compat<Type>::boxPackStart (buttonBox, GTK_WIDGET(yes_), FALSE, FALSE, 0);
        //g_object_set_data (G_OBJECT (dialog), "action_true_button", yes_);
        //gtk_dialog_add_action_widget (GTK_DIALOG (dialog), GTK_WIDGET(yes_), GTK_RESPONSE_YES);
    }
  
};
}
#endif

