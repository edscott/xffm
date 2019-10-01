#ifndef XF_USERRESPONSE_HH
# define XF_USERRESPONSE_HH
#include "chooserresponse.hh"

namespace xf
{
template <class Type> class MenuPopoverSignals;
template <class Type>
class CustomResponse: public ComboFileResponse<Type>{
    using gtk_c = Gtk<Type>;
    gchar *exec_;
    gchar *workdir_;
    gint terminal_;
    GList *optionsList;
public:
    ~CustomResponse(void){
        g_free(exec_);
        g_free(workdir_);
        g_list_free(optionsList);
        customDialogs = g_list_remove(customDialogs, this->response_);
    }

    CustomResponse(GtkWindow *parent, const gchar *file):
     ComboFileResponse<Type>(parent, "title", "run")
    {
        auto showEntry = getInt(file, "custombutton", "entry");
        if (!showEntry){
            gtk_widget_hide(GTK_WIDGET(this->hbox_));
            auto tooltip = getString(file, "custombutton", "tooltip");
            if (tooltip){
                auto label = GTK_LABEL(gtk_label_new(""));
                auto markup = g_strdup_printf("<span color=\"blue\" size=\"larger\">%s</span>", tooltip);
                gtk_label_set_markup(label, markup);
                g_free(markup);
	        gtk_box_pack_start (GTK_BOX (this->vbox2_), GTK_WIDGET(label), FALSE, FALSE, 0);
                gtk_widget_show(GTK_WIDGET(label));
                g_free(tooltip);
            }
        }
        customDialogs = g_list_prepend(customDialogs, this->response_);
        exec_ = getString(file, "custombutton", "exec");
        workdir_ = getString(file, "custombutton", "workdir");
        terminal_ = getInt(file, "custombutton", "terminal");
        optionsList = NULL;
        if (!exec_) {
            throw(1);
        }
        if (!g_path_is_absolute(exec_)){
                auto p = g_find_program_in_path(exec_);
                if (p) {
                    g_free(exec_);
                    exec_=p;
                }
        }
        auto title = g_strdup_printf("%s: %s", _("Execute command"), exec_);
        this->setTitle(exec_);
        g_free(title);
        g_object_set_data(G_OBJECT(this->yes_), "exec", exec_);
        g_object_set_data(G_OBJECT(this->yes_), "workdir", workdir_);
        if (terminal_ == 1) g_object_set_data(G_OBJECT(this->yes_), "terminal", GINT_TO_POINTER(1));
        else g_object_set_data(G_OBJECT(this->yes_), "terminal", NULL);
	g_signal_connect (G_OBJECT (this->no_), "clicked", G_CALLBACK (cancel), this);
	g_signal_connect (G_OBJECT (this->yes_), "clicked", G_CALLBACK (run), this);
	g_signal_connect (G_OBJECT (this->dialog()), "delete-event", G_CALLBACK (response_delete), this);

        // get selection list paths in (gchar **)
        auto view = Fm<Type>::getCurrentView();
        GList *selectionList;
        if (isTreeView){
            auto treeModel = view->treeModel();
            auto selection = gtk_tree_view_get_selection (view->treeView());
            selectionList = gtk_tree_selection_get_selected_rows (selection, &treeModel);
        } else {
            selectionList = gtk_icon_view_get_selected_items (view->iconView());
        }
        view->setSelectionList(selectionList);

        DBG("selection list %p length = %d\n", selectionList, g_list_length(selectionList));
        GtkTreeIter iter;
        if (selectionList && g_list_length(selectionList)){
            int i=0;
            auto paths = (gchar **)calloc(g_list_length(selectionList)+1, sizeof(gchar *));
            for (auto l = selectionList; l && l->data; l=l->next){
                if (!gtk_tree_model_get_iter (view->treeModel(), &iter, (GtkTreePath *)l->data)){
                    continue;
                }
                gchar *path;
        	gtk_tree_model_get (view->treeModel(), &iter, PATH, &path, -1);
                paths[i++] = g_path_get_basename(path);
                if (!workdir_) {
                    workdir_ = g_path_get_dirname(path);
                    g_object_set_data(G_OBJECT(this->yes_), "workdir", workdir_);
                }
                g_free(path);
            }
            this->setComboOptions((const gchar **)paths);
            g_strfreev(paths);
        }
            


        this->setComboBashCompletion("/");// XXX folder or file entry should be determined on .ini file (probably just inherit from comboresponse for multiple files options (selected files)...
        this->setComboLabel(_("Source File"));
	auto options = getKeys(file, "options");
        for (auto p=options; p && *p; p++){
            auto itemValue = getString(file, "options", *p);
	    auto hbox = gtk_c::hboxNew (FALSE, 6);
	    auto innerbox = gtk_c::hboxNew (FALSE, 6);
            auto check = gtk_check_button_new();
            g_object_set_data(G_OBJECT(hbox), "check", check);
            g_object_set_data(G_OBJECT(hbox), "innerbox", innerbox);
	    auto option = g_strdup_printf("--%s", *p);  
	    auto label = gtk_label_new(option);
            g_free(option);
            g_object_set_data(G_OBJECT(hbox), "label", label);

            optionsList = g_list_append(optionsList, (void *)hbox);
	    gtk_box_pack_start (GTK_BOX (this->vbox2_), GTK_WIDGET(hbox), FALSE, FALSE, 0);
	    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET(check), FALSE, FALSE, 0);
	    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET(innerbox), FALSE, FALSE, 0);
	    gtk_box_pack_start (GTK_BOX (innerbox), GTK_WIDGET(label), FALSE, FALSE, 0);
	    if (strcmp(itemValue, "file")==0 || strcmp(itemValue, "folder")==0
                    || strcmp(itemValue, "text")==0) {
		auto entry = GTK_ENTRY(gtk_entry_new ());
                g_object_set_data(G_OBJECT(hbox), "entry", entry);
                    g_object_set_data(G_OBJECT(entry), "workdir", workdir_);

		gtk_box_pack_start (GTK_BOX (innerbox), GTK_WIDGET(entry), FALSE, FALSE, 0);
		if (strcmp(itemValue, "file")==0 || strcmp(itemValue, "folder")==0){

		    auto button = gtk_c::dialog_button ((strcmp(itemValue, "file")==0)?
			    "document-new-symbolic":"folder-symbolic", NULL);
		    gtk_box_pack_start (innerbox, GTK_WIDGET(button), FALSE, FALSE, 0);
		    if (strcmp(itemValue, "folder")==0){
			DBG("setting up exec completion\n");
			g_signal_connect (G_OBJECT(button), 
				"clicked", BUTTON_CALLBACK (ChooserResponse<Type>::folderChooser), 
				(gpointer) entry);
		       this->setComboBashCompletion("/");
		    } else if (strcmp(itemValue, "file")==0) {
			DBG("setting up file completion\n");
			g_signal_connect (G_OBJECT(button), 
				"clicked", BUTTON_CALLBACK (ChooserResponse<Type>::fileChooser), 
				(gpointer) entry);
		       this->setComboBashFileCompletion(g_get_home_dir());
		    } 
		    this->setInLineCompletion(0);
		}
	    }
	    else if (strcmp(itemValue, "on")==0) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), TRUE);
	    }
            g_signal_connect(G_OBJECT(check), 
			    "clicked", BUTTON_CALLBACK (toggleBox), 
			    (gpointer) innerbox);

            gtk_widget_set_sensitive(GTK_WIDGET(innerbox), 
                        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check)));

            gtk_widget_show_all(GTK_WIDGET(hbox));
            
            g_free(itemValue);
        }
        g_object_set_data(G_OBJECT(this->yes_), "optionsList", optionsList);
        g_object_set_data(G_OBJECT(this->yes_), "pageWorkdir", (void *)Fm<Type>::getCurrentPage()->workDir());

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
	    //ERROR("custombutton():: %s\n", error->message);
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
	    DBG("custombutton():: %s\n", error->message);
            g_key_file_free(keyFile);
            g_error_free(error);
	    return NULL;
	}
        g_key_file_free(keyFile);
        return retval;
    }

    static gchar *getIcon(const gchar *file){
        auto icon = getString(file, "custombutton", "icon");
        if (!icon) icon = g_strdup("system-run");
        return icon;
    }

    static gchar *getTooltip(const gchar *file){
        auto tooltip = getString(file, "custombutton", "tooltip");
        if (!tooltip) tooltip = g_strdup("Custom button");
        return tooltip;
    }

    static GtkButton *custombutton(const gchar *file){
        auto icon = getIcon(file);
        auto tooltip = getTooltip(file);
	auto button = Gtk<Type>::newButton(icon, tooltip);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(openCustomDialog), g_strdup(file));
	g_free(icon);
	
	return button;
    }
private:
         
    static void toggleBox(GtkToggleButton *button, void *data){
        auto box = GTK_WIDGET(data);
        gtk_widget_set_sensitive(box, gtk_toggle_button_get_active(button));
    }
       
    static void openCustomDialog(GtkButton *button, void *data){
        auto file = (const gchar *) data;
        try {
            auto customResponse = new(CustomResponse<Type>)(mainWindow, file);
            customResponse->runResponse();
        } catch (int e) {
            gchar *message;
            switch (e){
                case 1:
                    message = g_strdup_printf("\"<b>exec</b>\" key not found in file: \n<i>%s</i>\n", file);
                    break;
                default:
                    message = g_strdup_printf("undefined error in file: %s\n", file);
                    break;
            }
            Dialogs<Type>::quickHelp(mainWindow, message, "dialog-error");
            g_free(message);
            return;
        }
    }

    static void
    run (GtkButton *button, gpointer data) {
        auto object = (CustomResponse<Type> *)data;
        auto exec = (const gchar *)g_object_get_data(G_OBJECT(button), "exec");
        auto workdir = (const gchar *)g_object_get_data(G_OBJECT(button), "workdir");
        auto terminal = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button), "terminal"));

        auto entry = object->comboEntry();
        auto file = gtk_entry_get_text(entry);
        auto command = g_strdup_printf("%s ", exec);
        if (terminal) {
            auto g = g_strconcat(getenv("TERMINAL_EXEC"), " ", command, NULL);
            g_free(command);
            command = g;
        }
        auto oList = (GList *)g_object_get_data(G_OBJECT(button), "optionsList");
        for (auto p=oList; p && p->data; p=p->next){
            auto hbox = p->data;
            auto check = g_object_get_data(G_OBJECT(hbox), "check");
            if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check))) continue;
            auto label = g_object_get_data(G_OBJECT(hbox), "label");
            auto option = gtk_label_get_text(GTK_LABEL(label));
            auto g = g_strconcat(command," ",option,NULL);
            g_free(command);
            command = g;

            auto entry =  g_object_get_data(G_OBJECT(hbox), "entry");
            if (entry) {
                auto text = gtk_entry_get_text(GTK_ENTRY(entry));
                if (text && strlen(text)){
                    auto g = g_strconcat(command,"=",text,NULL);
                    g_free(command);
                    command = g;
                }
            }
        }
        
        // XXX Hack: do not use combo entry if in terminal XXX
        if (!terminal) {
            gchar *path;
            if (g_path_is_absolute(file)) path = g_strdup(file);
            else if (workdir) path = g_strconcat(workdir,G_DIR_SEPARATOR_S, file, NULL);
            else {
                auto w = (const gchar *)g_object_get_data(G_OBJECT(button), "pageWorkdir");
                path = g_strconcat(w,G_DIR_SEPARATOR_S, file, NULL); 
            }
            auto g = g_strconcat(command," \"", path, "\"", NULL);
            g_free(command);
            g_free(path);
            command = g;
        }
        

        DBG("run \'%s\'\n", command);
        //MenuPopoverSignals<Type>::plainRun(NULL, command);
        MenuPopoverSignals<Type>::runWd(workdir, command);
        g_free(command);
    }
    static void
    cancel (GtkButton *button, gpointer data) {
        auto object = (CustomResponse<Type> *)data;
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
        Dialogs<Type>::placeDialog(GTK_WINDOW(this->response_));
        //gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), FALSE);
        //gtk_main();
	return;
    }

};
}


#endif
