
#ifndef ECRYPTFS_HH
#define ECRYPTFS_HH

#include "ecryptfs.i"

namespace xf{


typedef struct fuse_login_t{
    guint64 flag;
    gchar *flag_key;
    gchar *computer;
    gchar *remote_path;
    gchar *login;
    gchar *empty_passphrase;
    gchar *secure_shell_key;
    gchar *url;
    gchar *mount_point;
    gchar *monitor;
    gchar *broadband;
}fuse_login_t;

template <class Type>
class Fuse  {
    GtkDialog *dialog_;
    GKeyFile *keyfile_;
    const gchar *url_;
    GtkBox *vbox_;
    GtkToggleButton *monitor_;
    GtkWidget *allowEmptyPassphrase_;
    const gchar *urlTemplate_;
    GtkBox *mountPointBox_; // FUSE_MOUNT_POINT_BOX
    GtkButton *loadButton_;
    GtkButton *saveButton_;
    GtkButton *cancelButton_;
    GtkButton *mountButton_;
    gint response_;
    
public:
    GtkToggleButton * monitor(void){return monitor_;}
    GtkWidget * allowEmptyPassphrase(void){return allowEmptyPassphrase_;}
    GtkDialog *dialog(void){ return dialog_;}
    void setUrlTemplate(const gchar *value){ urlTemplate_ = value;}
    const gchar *urlTemplate(void){ return urlTemplate_;}
    GtkBox *vbox(void){return vbox_;}
    gint response(void){return response_;}
    GtkButton *saveButton(void){return saveButton_;}
    GtkButton *mountButton(void){return mountButton_;}
    
    Fuse(const gchar *url, const gchar *info1, const gchar *info2):
        monitor_(NULL),
        allowEmptyPassphrase_(NULL)
    {
        DBG("Fuse constructor(%s, %s, %s)\n", url, info1, info2);
        
        url_ = url;
        keyfile_ = loadKeyfile(url);
        dialog_ = initDialog(info1, info2);
    }

    ~Fuse(void){
        DBG("Fuse destructor...\n");
        gtk_widget_destroy(GTK_WIDGET(dialog_));        
    }


    GtkEntry *
    addEntry(const gchar *item_string, const gchar *item_id, gboolean state=TRUE){

        auto hbox = makeEntryBox(this, item_string, item_id, state, TRUE);
        gtk_widget_set_sensitive(GTK_WIDGET(hbox), state);
        if (strcmp(item_id, "FUSE_MOUNT_POINT")==0){
            mountPointBox_ = hbox;
        }
            
            
        auto entry = (GtkEntry *)g_object_get_data(G_OBJECT(dialog_), item_id);
        if (strcmp(item_id, "FUSE_REMOTE_PATH")==0) {
            g_signal_connect(G_OBJECT(entry), "key-release-event", G_CALLBACK(make_absolute), this);
        }
        gchar *default_value=NULL;
        gchar *user=NULL;
        gchar *computer=NULL;
        gchar *remote_path=NULL;
        gchar *u = NULL;
        if (url_){
            u = g_strdup(url_);
            gchar *p = strstr(u, "://");
            if (p) {
                p = p+strlen("://");
                if (strchr(p,'/')){
                    *strchr(p,'/') = 0;
                    if (strchr(p,'@')) {
                        user = g_strdup(p);
                        *strchr(user,'@') = 0;
                        p = strchr(p,'@') + 1;
                    }
                    computer = g_strdup(p);
                    remote_path = g_strdup_printf("/%s",p + strlen(p) + 1);
                }
            }
            g_free(u);
        }

        if (strcmp(item_id, "FUSE_COMPUTER")==0) {
            default_value = g_strdup(computer);
        } else if (strcmp(item_id, "FUSE_REMOTE_PATH")==0) {
            default_value = g_strdup(remote_path);
        }
        if (strcmp(item_id, "FUSE_LOGIN")==0) {
            if (user) {
                default_value = g_strdup(user);
            } else {
                if (getenv("USER")) 
                    default_value = g_strdup(getenv("USER"));
                else if (getenv("LOGNAME")) 
                    default_value = g_strdup(getenv("LOGNAME"));
                else default_value = g_strdup(getenv("GUEST"));
            }
        }
        g_free(user);
        g_free(computer);
        g_free(remote_path);
        if (url_ && strcmp(item_id, "FUSE_MOUNT_POINT")==0) {
            default_value = defaultUrlMountPoint(url_);
        }
        
        if (!default_value) default_value=g_strdup("");
        setEntry(entry, keyfile_, url_, item_id, default_value);
        gtk_box_pack_start (GTK_BOX (vbox_), GTK_WIDGET(hbox), FALSE, FALSE, 0);
        g_free(default_value);
        return entry;
    }
    
#if 0
    GtkBox *
    addCheck(const gchar *item_string, const gchar *item_id, 
            gboolean state=TRUE,
            const gchar *extra_text=NULL){
        auto vbox = this->vbox();
        auto hbox = Gtk<Type>::hboxNew(FALSE, 0);

        DBG("addCheck...: %s %s\n", item_string,item_id);
        GtkWidget *check; 
        if (strcmp(item_id, "FUSE_BROADBAND")==0) {
            check = MakeCheckBox(this, 
                    item_string, 
                    item_id, 
                    (gpointer)toggle_broad); 
        } else if (strcmp(item_id, "FUSE_SECURE_SHELL_KEY")==0) {
            check = MakeCheckBox(this, 
                    item_string, 
                    item_id, 
                    (gpointer)toggle_ssh); 
        }else {
            check = MakeCheckBox(this, 
                    item_string, 
                    item_id); 
        }
        gtk_box_pack_start (hbox, check, FALSE, FALSE, 0);
        GtkWidget *label = gtk_label_new("");
        if (extra_text) {
            gchar *c = g_strdup_printf("<i>(%s)</i>", extra_text);
            gtk_label_set_markup(GTK_LABEL(label), c);
            g_free(c);
        } 
        gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
        //gtk_widget_show(label);
        //gtk_widget_show(GTK_WIDGET(hbox));
        auto key_file = (GKeyFile *)g_object_get_data(G_OBJECT(this->dialog()), 
                "key_file");
        auto url = (const gchar *)g_object_get_data(G_OBJECT(this->dialog()), 
                "url");
        setCheckButtonState(this->dialog(), key_file, url, 
                item_id, item_id,
                state);

        if (strcmp(item_id, "FUSE_ALLOW_EMPTY_PASSPHRASE")==0){
            auto t = (GtkWidget *)g_object_get_data(G_OBJECT(this->dialog()),
                "FUSE_SECURE_SHELL_KEY");
            if (t && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(t))){
                gtk_widget_set_sensitive(GTK_WIDGET(check), TRUE);
            } else if (t) {
                gtk_widget_set_sensitive(GTK_WIDGET(check), FALSE);
            }

        }
        gtk_box_pack_start (vbox, GTK_WIDGET(hbox), FALSE, FALSE, 0);
        return hbox;
    }
#endif

    void *
    addOptionPage(group_options_t *options_p, const gchar *label, gint flag_id) {
        auto key_file = (GKeyFile *)g_object_get_data(G_OBJECT(this->dialog()), 
                "key_file");
        auto url = (const gchar *)g_object_get_data(G_OBJECT(this->dialog()),
                "url");

        GtkWidget *vbox = optionsBox(this->dialog(), 
                options_p, key_file, url, flag_id);
        //gtk_widget_show(GTK_WIDGET(vbox));
        GtkWidget *tab_label = gtk_label_new(label);
        GtkWidget *menu_label = gtk_label_new(label);
        auto notebook = (GtkNotebook *) g_object_get_data(G_OBJECT(this->dialog()), 
                "notebook");
        gtk_notebook_append_page_menu (notebook, GTK_WIDGET(vbox), tab_label, 
                menu_label);  
        gtk_notebook_set_tab_reorderable (notebook, GTK_WIDGET(vbox), TRUE);
        return NULL;
    }

#if 0
    //this is next, partly called by Fuse constructor 
    //and partly called by EFS constructor
    void *
    confirmHost (gpointer data){
        gint response = GTK_RESPONSE_CANCEL;
        gtk_main();
        gtk_widget_hide (fuse_data_p->dialog);

        gboolean retval;
        if(response == GTK_RESPONSE_YES || response == GTK_RESPONSE_APPLY){
            retval = TRUE;
            gchar *new_url = rfm_rational(PLUGIN_DIR, module_name, fuse_data_p, (void*) url_, "accept");
            if (!new_url) {
                goto retry;
            }
            if(response == GTK_RESPONSE_YES) {
                TRACE( "%s: mount_url\n", module_name);
                retval = GPOINTER_TO_INT(rfm_rational(PLUGIN_DIR, module_name, widgets_p, new_url, "mount_url"));
            }
            view_t *view_p = widgets_p->view_p;
            record_entry_t *t_en = rfm_copy_entry(view_p->en);
            if(!rodent_refresh (widgets_p, t_en)) {
                rfm_destroy_entry(t_en);
            }
            g_free(new_url);
        } else {
            retval = FALSE;
        }
            gtk_widget_destroy (fuse_data_p->dialog);
        g_cond_signal(signal);
        return GINT_TO_POINTER(retval);
    }
#endif

private:
    static GtkTextView *
    mkTextView (const gchar *text){
        auto labelview = GTK_TEXT_VIEW(gtk_text_view_new());
        gtk_text_view_set_editable (labelview, FALSE);
        gtk_text_view_set_cursor_visible (labelview, FALSE);
        gtk_text_view_set_wrap_mode (labelview, GTK_WRAP_WORD);
        
        auto buffer = gtk_text_view_get_buffer (labelview);
        GtkTextIter iter;
        gtk_text_buffer_get_start_iter (buffer, &iter);
        gtk_text_buffer_insert (buffer,&iter, text, -1);
        return labelview;
    }


    GtkDialog *
    initDialog(const gchar *info1, const gchar *info2){
        auto dialog = gtk_dialog_new ();
        if (mainWindow) {
            gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
            gtk_window_set_transient_for (GTK_WINDOW (dialog), 
                    GTK_WINDOW (mainWindow));
        } else {
            gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
        }

        gtk_window_set_type_hint(GTK_WINDOW(dialog), GDK_WINDOW_TYPE_HINT_DIALOG);

        auto hbox = Gtk<Type>::hboxNew (FALSE, 2);
        gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area(GTK_DIALOG (dialog))), GTK_WIDGET(hbox), FALSE, FALSE, 0);

        auto pixbuf = Pixbuf<Type>::get_pixbuf("dialog-question", -24);
        auto image = gtk_image_new_from_pixbuf(pixbuf);
        g_object_unref(pixbuf);
        gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

        auto text = g_strconcat(_("Options:"), " ", 
                info1, "\n\n", 
                info2, NULL);        
        auto labelview = mkTextView(text);
        g_free(text);
        gtk_box_pack_start (hbox, GTK_WIDGET(labelview), TRUE, TRUE, 0);


        auto tbox = Gtk<Type>::vboxNew(TRUE, 0);
        gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG(dialog))), GTK_WIDGET(tbox), TRUE, TRUE, 0);
        GtkWidget *notebook = gtk_notebook_new ();
        g_object_set_data(G_OBJECT(dialog), "notebook", notebook);
        gtk_notebook_popup_enable (GTK_NOTEBOOK(notebook));
        gtk_notebook_set_scrollable (GTK_NOTEBOOK(notebook), TRUE);
        g_object_set (notebook,
                      "enable-popup", TRUE, 
                      "can-focus", FALSE,
                      "scrollable", TRUE, 
                      "show-border", FALSE,
                      "show-tabs", 
                      TRUE, "tab-pos",
                      GTK_POS_TOP, NULL);  


        gtk_box_pack_start (tbox, notebook, TRUE, TRUE, 0);
        vbox_ = Gtk<Type>::vboxNew (TRUE, 0);

        //gtk_widget_show(GTK_WIDGET(vbox_));
        
        GtkWidget *tab_label = gtk_label_new(_("Mount"));
        GtkWidget *menu_label = gtk_label_new(_("Mount"));

        gtk_notebook_insert_page_menu (GTK_NOTEBOOK(notebook), GTK_WIDGET(vbox_), tab_label, menu_label, 0);
        gtk_notebook_set_tab_reorderable (GTK_NOTEBOOK(notebook), GTK_WIDGET(vbox_), TRUE);

        auto action_area = Gtk<Type>::hboxNew(FALSE, 1);
        gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area(GTK_DIALOG(dialog))), 
                GTK_WIDGET(action_area), FALSE, FALSE, 0);
        //gtk_widget_show(GTK_WIDGET(action_area));



        cancelButton_ = Gtk<Type>::dialog_button ("window-close", _("Cancel"));
        gtk_box_pack_start (GTK_BOX (action_area), GTK_WIDGET(cancelButton_), FALSE, FALSE, 0);


        saveButton_ = Gtk<Type>::dialog_button ("media-floppy", _("Save"));
        gtk_box_pack_start (GTK_BOX (action_area), GTK_WIDGET(saveButton_), FALSE, FALSE, 0);

        loadButton_ = Gtk<Type>::dialog_button ("document-open", _("Load"));
        gtk_box_pack_start (GTK_BOX (action_area), GTK_WIDGET(loadButton_), FALSE, FALSE, 0);

        mountButton_ = Gtk<Type>::dialog_button ("greenball", _("Mount"));
        gtk_box_pack_start (GTK_BOX (action_area), GTK_WIDGET(mountButton_), FALSE, FALSE, 0);


        g_signal_connect (G_OBJECT (loadButton_), "clicked", G_CALLBACK (button_load), this);
        g_signal_connect (G_OBJECT (saveButton_), "clicked", G_CALLBACK (button_save), this);
        g_signal_connect (G_OBJECT (cancelButton_), "clicked", G_CALLBACK (button_cancel), this);
        g_signal_connect (G_OBJECT (mountButton_), "clicked", G_CALLBACK (button_mount), this);

        g_signal_connect (G_OBJECT (dialog), "delete-event", G_CALLBACK (response_delete), this);
        gtk_window_set_resizable (GTK_WINDOW(dialog), TRUE);

        response_ = GTK_RESPONSE_CANCEL;

        return GTK_DIALOG(dialog);

    }

    static GKeyFile *
    loadKeyfile(const gchar *url){
        if (!url) return NULL;
        GKeyFile *key_file=NULL;
        key_file = g_key_file_new ();
        gchar *file = g_build_filename(FUSE_KEY_FILE, NULL);
        if (!g_key_file_load_from_file (key_file, file, (GKeyFileFlags)0, NULL)){
            g_key_file_free(key_file);
            key_file=NULL;
        }
        g_free(file);
        return key_file;
    }

    static GtkBox *
    makeEntryBox(Fuse<Type> *fuse, 
            const gchar *item_string, 
            const gchar *id, 
            gboolean withSelector,
            gboolean visibility)
    {
        const gchar *separator=" ";
        if (item_string && !strchr(item_string, ':')){
            separator = ": ";
        }
        auto hbox = Gtk<Type>::hboxNew(FALSE, 0);
        auto full_text = g_strconcat(item_string, separator, NULL);
        auto label = gtk_label_new(full_text);
        g_free(full_text);
        auto entry = gtk_entry_new();
        gtk_entry_set_visibility (GTK_ENTRY(entry), visibility);
        g_object_set_data(G_OBJECT(fuse->dialog()), id, entry);
        gtk_box_pack_start (hbox, label, FALSE, FALSE, 0);
        gtk_box_pack_start (hbox, entry, TRUE, TRUE, 0);

        if (withSelector){
            auto button = Gtk<Type>::dialog_button ("document-open", NULL);
            g_signal_connect (G_OBJECT (button), "clicked", 
                    G_CALLBACK (ChooserResponse<Type>::folderChooser), entry);
            gtk_box_pack_start (hbox, GTK_WIDGET(button), FALSE, FALSE, 0);
        }
            

        return hbox;
    }

    static void
    setEntry(GtkEntry *entry, 
            GKeyFile *key_file, 
            const gchar *group,
            const gchar *key, 
            const gchar *default_value){
        gchar *pre_set=NULL;
        if (key_file && group) {
            pre_set = g_key_file_get_value(key_file, group, key, NULL);
        }

                        DBG("1.2 value=%s\n", (pre_set)?pre_set:default_value);
                        DBG("1.2 pre_set=\"%s\" \"%s\"\n", pre_set, default_value);
        gtk_entry_set_text(GTK_ENTRY(entry),(pre_set)?pre_set:default_value); 
        g_free(pre_set); 
    }


    static gchar *
    defaultUrlMountPoint(const gchar *url){
        gchar *computer=NULL;
        gchar *remote_path=NULL;
        gchar *u = NULL;
        if (url){
            u = g_strdup(url);
            gchar *p = strstr(u, "://");
            if (p) {
                p = p+strlen("://");
                if (strchr(p,'/')){
                    *strchr(p,'/') = 0;
                    computer = g_strdup(p);
                    remote_path = g_strdup_printf("/%s",p + strlen(p) + 1);
                }
            }
            g_free(u);
        }

        auto user = g_path_get_basename(g_get_home_dir ());
        gchar *dir;
        gchar *device = NULL;
        if (remote_path) {
            device = g_path_is_absolute(remote_path)? remote_path+1: remote_path;
        }
        if (computer && remote_path) {
            dir = g_strdup_printf("%s-%s", computer, device);
        } else {
            dir = g_strdup((computer)? computer : device);
        }
        auto default_value = 
            g_build_filename (g_get_tmp_dir (), user, "mnt", dir, NULL);
        g_free(user);
        g_free(dir);
        g_free(computer);
        g_free(remote_path);
        return default_value;
    }
    
    static gint
    on_key_press (GtkWidget * in_entry, GdkEventKey * event, gpointer data){
        auto fuse = (Fuse<Type> *)data;
        auto entry = (GtkEntry *)g_object_get_data(G_OBJECT(fuse->dialog()), "FUSE_COMPUTER");
        gchar *host;
        if (entry) {
            const gchar *c =  gtk_entry_get_text (GTK_ENTRY(entry));
            if (!c || !strlen(c)){
                host = g_strdup("");
            } else {
                host = g_strdup(c);
            }
        } else {
            DBG("No host name defined in function fuse_common.c:on_key_press()\n");
            host = g_strdup("");
        }


        gchar *rpath=NULL;
        entry = (GtkEntry *)g_object_get_data(G_OBJECT(fuse->dialog()), "FUSE_REMOTE_PATH");
        if (entry) {
            const gchar *c =  gtk_entry_get_text (GTK_ENTRY(entry));
            if (!c || !strlen(c)){
                rpath = g_strdup("/");
            } else {
                if (c[0] != '/') rpath = g_strconcat("/", c, NULL);
                else rpath = g_strdup(c);
            }
        }

        gchar *login=NULL;
        entry = (GtkEntry *)g_object_get_data(G_OBJECT(fuse->dialog()), "FUSE_LOGIN");
        if (entry) {
            const gchar *c =  gtk_entry_get_text (GTK_ENTRY(entry));
            if (c && strlen(c)){
                login = g_strdup(c);
            }
        }

        gchar *url;
        if (login) { 
            url = g_strdup_printf("%s://%s@%s%s", fuse->urlTemplate(), login, host, 
                    (rpath)?rpath:"");
        } else {
            url = g_strdup_printf("%s://%s%s", fuse->urlTemplate(), host, 
                    (rpath)?rpath:"");
        }
        entry = (GtkEntry *)g_object_get_data(G_OBJECT(fuse->dialog()), "FUSE_URL");

                        DBG("1.4 value=%s\n", url);
        gtk_entry_set_text(GTK_ENTRY(entry), url);
        g_free(host);
        g_free(rpath);
        g_free(url);
        return FALSE;
    }

#if 0
    static gchar *
    setKeyValue(const gchar *group, const gchar *key, const gchar *value){
        gchar *file = g_build_filename(FUSE_KEY_FILE, NULL);
        GKeyFile *key_file = g_key_file_new ();
        const gchar *retval=value;
        if (g_key_file_load_from_file (key_file, file, G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS, NULL)){
            g_key_file_set_value (key_file, group, key, value);
            writeKeyfile(key_file);    
        } else retval = NULL;
        g_free(file);
        g_key_file_free(key_file);
        return value;
    }

    static void
    writeKeyfile(GKeyFile *key_file){
        gchar *file = g_build_filename(FUSE_KEY_FILE, NULL);
        TRACE( "writeKeyfile: %s\n", file);
        // Write out key_file:
        gsize file_length;
        gchar *file_string = g_key_file_to_data (key_file, &file_length, NULL);
        gchar *config_directory = g_path_get_dirname(file);
        if (!g_file_test(config_directory, G_FILE_TEST_IS_DIR)){
            TRACE( "creating directory %s\n", config_directory);
            g_mkdir_with_parents(config_directory, 0700);
        }

        g_free(config_directory);
        gint fd = creat(file, O_WRONLY | S_IRWXU);

        if (fd >= 0){
            // lock file for write
            if (write(fd, file_string, file_length) < 0){
                DBG("write_keyfile(): cannot write to %s: %s\n", file, strerror(errno));
            }
            close(fd);
        } else {
            DBG("write_keyfile(): cannot open %s for write: %s\n", file, strerror(errno));
        }
        g_free(file);
    }
    static void
    save_extra_option(GtkWidget *dialog, const gchar *url,
            const gchar *option_id, gchar *extra_option_id){ 
        gchar *entry_id = g_strconcat(option_id, "Entry", NULL);
        GtkWidget *entry;
        GtkToggleButton *button = g_object_get_data(G_OBJECT(dialog), option_id);
        if (gtk_toggle_button_get_active(button)){
            // override last set value
            entry = g_object_get_data(G_OBJECT(dialog), entry_id);
        } else {
            entry = g_object_get_data(G_OBJECT(dialog), extra_option_id);
        }
        const gchar *c = gtk_entry_get_text (GTK_ENTRY(entry));
        if (c && strlen(c)){
              setKeyValue (url, extra_option_id, (gchar *)c);
        }
        return;
    }
    static void 
    togglebutton_f(GtkToggleButton *togglebutton , gpointer data){
        GtkWidget *dialog = data;
        GtkToggleButton *in_b;
        GtkToggleButton *out_b;
        GtkEntry *out_e;
        gchar *entry_txt;

        in_b = g_object_get_data(G_OBJECT(dialog), "EPS_ENABLE_FILENAME_CRYPTO");
        out_b = g_object_get_data(G_OBJECT(dialog), "_oecryptfs_enable_filename_crypto_");
        out_e = g_object_get_data(G_OBJECT(dialog), "_oecryptfs_enable_filename_crypto_Entry");
        entry_txt = (gtk_toggle_button_get_active(in_b))? "yes": "no";
        gtk_entry_set_text(out_e, entry_txt);
        gtk_toggle_button_set_active(out_b, TRUE);
        // Requires fnek signature:
        out_b = g_object_get_data(G_OBJECT(dialog), "_oecryptfs_fnek_sig_");
        gtk_toggle_button_set_active(out_b, gtk_toggle_button_get_active(in_b));

        in_b = g_object_get_data(G_OBJECT(dialog), "EPS_PASSTHROUGH");
        out_b = g_object_get_data(G_OBJECT(dialog), "_oecryptfs_passthrough_");
        out_e = g_object_get_data(G_OBJECT(dialog), "_oecryptfs_passthrough_Entry");
        entry_txt = (gtk_toggle_button_get_active(in_b))? "yes": "no";
        gtk_entry_set_text(out_e, entry_txt);
        gtk_toggle_button_set_active(out_b, TRUE);

    }
#endif

    static GtkWidget *
    MakeCheckBox(Fuse<Type> *fuse, 
            const gchar *text,
            const gchar *id,
            gpointer callback=NULL)
    {
        GtkWidget *check;
        check = gtk_check_button_new_with_label (text);
        g_object_set_data(G_OBJECT(fuse->dialog()), id, check);
        //gtk_widget_show(check);
        if (callback){
            g_signal_connect (check, "toggled", G_CALLBACK (callback), fuse);
        }
        return check;
    }
    
    static void
    setCheckButtonState(GtkDialog *dialog,
            GKeyFile *key_file, 
            const gchar *group,
            const gchar *key,
            const gchar *id,
            gboolean default_state){
        auto check = (GtkWidget *)g_object_get_data(G_OBJECT(dialog), id);
        if (!check) return;
        if (strstr(id, "FTP_PASSIVE")){
            TRACE("FTP_passive = %d\n",  
                    g_key_file_get_boolean(key_file, group, key, NULL));
        }
        if (key_file){
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), 
                        g_key_file_get_boolean(key_file, group, key, NULL));
        } else {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), default_state);
        }
    }

    static GtkWidget *
    optionsBox(GtkDialog *dialog, group_options_t *options_p,
            GKeyFile *key_file, const gchar *group, gint flag_id) {
        guint64 ui=0x01;
        auto  vbox = Gtk<Type>::vboxNew(0, FALSE);
        GtkWidget *sw = gtk_scrolled_window_new (NULL, NULL);
        gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), 
                GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_container_add(GTK_CONTAINER(sw), GTK_WIDGET(vbox));
#if 0
        guint64 flag = 0;
        if (key_file && group && strlen(group)) {
            // Get the flag set in the file...
            TRACE( "check options from keyfile for group=%s...\n", group);
            gchar *flag_key = FLAG_KEY(flag_id);
            flag = g_key_file_get_uint64(key_file, group, flag_key, NULL);
            g_free(flag_key);
        } else {
            // Set a new default flag...
            gint i=0;
            group_options_t *p;
            for (p=options_p; p && p->flag; p++,i++){
                // Set checkbox on...
                if (p->sensitive > 1 || p->sensitive < 0) {
                    TRACE( "setting check on for %s\n", p->id);
                    flag |= (ui<<i);
                } else {
                    TRACE( "check off for %s\n", p->id);
                }
                // Special case, uid and gid, default values
                if (p->id && strcmp(p->id,"uid=")==0){
                    uid_t uid=geteuid();
                    TRACE("%d: uid is now %d\n",i, uid);
                    p->entry = g_strdup_printf("%d", (gint)uid);
                    continue;
                }
                if (p->id && strcmp(p->id,"gid=")==0){
                    gid_t gid=getegid();
                    p->entry = g_strdup_printf("%d", (gint)gid);
                }
            }
        }
#endif
            


        gint i=0;
        for (; options_p && options_p->id; options_p++){
          auto vbox2 = Gtk<Type>::vboxNew(0, FALSE);
          auto hbox = Gtk<Type>::hboxNew(0, FALSE);
          auto hbox2 = Gtk<Type>::hboxNew(0, FALSE);
          if (!options_p->id){
              DBG("optionsBox(): options id cannot be null.\n");
              continue;
          }
          if (g_object_get_data(G_OBJECT(dialog), options_p->id)) {
              DBG("optionsBox(): Duplicate entry: %s\n", options_p->id);
              continue;
          }
          g_object_set_data(G_OBJECT(dialog),options_p->id, hbox);


          auto checkMarkup = g_strdup_printf("<span color=\"%s\">%s</span>", 
                  (options_p->entry)?"red":"blue",
                  options_p->id);

          auto label = gtk_label_new("");
          gtk_label_set_markup(GTK_LABEL(label), checkMarkup);
          g_free(checkMarkup);

          auto check = gtk_check_button_new_with_label(options_p->flag);
          gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), 
                  options_p->sensitive > 1 || options_p->sensitive < 0);

          g_object_set_data(G_OBJECT(hbox),"check", check);

          gtk_box_pack_start (GTK_BOX (vbox2), GTK_WIDGET(hbox), FALSE, FALSE, 0);
          gtk_box_pack_start (GTK_BOX (vbox2), GTK_WIDGET(hbox2), TRUE, TRUE, 0);
          gtk_box_pack_start (GTK_BOX (hbox), check, FALSE, FALSE, 0);
          gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

          if (options_p->entry)  {
            auto entry = GTK_ENTRY(gtk_entry_new());
            g_object_set_data(G_OBJECT(hbox),"entry", entry);
            gtk_entry_set_text(entry, options_p->entry);
            gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET(entry), FALSE, FALSE, 0);
          }
          if (options_p->text || options_p->tip) {
                auto text = g_strconcat(
                        (options_p->tip)?options_p->tip:options_p->text, " ",
                        (options_p->tip)?options_p->text:NULL,
                        NULL);
                auto labelview = mkTextView(text);
                gtk_box_pack_start (GTK_BOX (hbox2), GTK_WIDGET(labelview), TRUE, TRUE, 0);
                g_free(text);
          }

          gtk_widget_set_sensitive(GTK_WIDGET(hbox), (options_p->sensitive > 0));
        
          /*if (key_file) {
            if (entry) {
                    gchar *value = g_key_file_get_value(key_file, group, id, NULL);
                    if (value) {
                        DBG("1 value=%s\n", value);
                        gtk_entry_set_text(GTK_ENTRY(entry), value);
                        g_free(value);
                    } else {
                        // Value is not found in key file (as for a new group)
                        DBG("2 value=%s\n", value);
                        gtk_entry_set_text(GTK_ENTRY(entry), options_p->entry);
                    }
                } 
          } else if (entry) {
                        DBG("3 value=%s\n", options_p->entry);
                gtk_entry_set_text(GTK_ENTRY(entry), options_p->entry);
          }*/
          /*if (flag & (ui << i)){
               gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), TRUE);
          }*/
          g_signal_connect (check, "toggled", G_CALLBACK (option_toggle), 
                   GINT_TO_POINTER(i));
          i++;

          gtk_box_pack_start (vbox, GTK_WIDGET(vbox2), FALSE, FALSE, 0);      
        }
        gtk_widget_set_size_request(sw, 400, -1);
        return sw;
    }

    static gchar *
    getOptionId(gchar **argv){
        gchar **p;
        gchar *id=g_strdup("");
        for (p=argv; p && *p; p++){
            gchar *q=id;
            id = g_strconcat(q, *p, NULL);
            g_free(q);
        }
        while (strchr(id, '=')) *strchr(id, '=') = '_';
        while (strchr(id, '-')) *strchr(id, '-') = '_';
        return id;
    }

private: // gtk callbacks

    static void
    button_load (GtkButton * button, gpointer data) {
        auto fuse = (Fuse<Type> *)data;
        gtk_dialog_response(fuse->dialog(), GTK_RESPONSE_APPLY);
        gtk_widget_hide(GTK_WIDGET(fuse->dialog()));
    }

    static void
    button_save (GtkButton * button, gpointer data) {
        auto fuse = (Fuse<Type> *)data;
        gtk_dialog_response(fuse->dialog(), GTK_RESPONSE_APPLY);
        gtk_widget_hide(GTK_WIDGET(fuse->dialog()));
    }

    static void
    button_mount (GtkEntry * entry, gpointer data) {
        auto fuse = (Fuse<Type> *)data;
        gtk_dialog_response(fuse->dialog(), GTK_RESPONSE_YES);
        gtk_widget_hide(GTK_WIDGET(fuse->dialog()));
    }

    static void
    button_cancel (GtkButton * button, gpointer data) {
        auto fuse = (Fuse<Type> *)data;
        gtk_dialog_response(fuse->dialog(), GTK_RESPONSE_CANCEL);
        gtk_widget_hide(GTK_WIDGET(fuse->dialog()));
    }


    static gboolean 
    response_delete(GtkWidget *dialog, GdkEvent *event, gpointer data){
        button_cancel(NULL, data);
        return TRUE;
    }


    static gint
    make_absolute (GtkWidget * in_entry, GdkEventKey * event, gpointer data){
        const gchar *c =  gtk_entry_get_text (GTK_ENTRY(in_entry));
        if (c && g_path_is_absolute(c)) return FALSE;
        gchar *a = g_strconcat("/", c, NULL);
                        DBG("1.3 value=%s\n", a);
        gtk_entry_set_text(GTK_ENTRY(in_entry), a);

        g_free(a);
        gtk_editable_set_position (GTK_EDITABLE(in_entry), strlen(a));
        return FALSE;
    }
    static void 
    option_toggle(GtkToggleButton *togglebutton , gpointer data){
#if 0
        gint i = GPOINTER_TO_INT(data);
        gboolean active = gtk_toggle_button_get_active(togglebutton);

        guint64 flags=0;
        if (active){
            flags |= ((guint64)0x01 << i); 
        } else {
            flags &= ~((guint64)0x01 << i); 
        }
#endif
        return;
    }
    static void 
    toggle_broad(GtkToggleButton *togglebutton , gpointer data){
        auto fuse = (Fuse<Type> *)data;
        auto b = (GtkToggleButton *)fuse->monitor();
        //auto b = (GtkToggleButton *)g_object_get_data(G_OBJECT(dialog), "FUSE_MONITOR");
        if (!b) return;
        if (gtk_toggle_button_get_active(togglebutton)){
            gtk_widget_set_sensitive(GTK_WIDGET(b), FALSE);
            gtk_toggle_button_set_active(b, FALSE);
        } else {
            gtk_widget_set_sensitive(GTK_WIDGET(b), TRUE);
        }
    }

    static void 
    toggle_ssh(GtkToggleButton *togglebutton , gpointer data){
        auto fuse = (Fuse<Type> *)data;
        auto b = (GtkWidget *)fuse->allowEmptyPassphrase();
        //auto b = (GtkWidget *)g_object_get_data(G_OBJECT(dialog), "FUSE_ALLOW_EMPTY_PASSPHRASE");
        if (!b){
            return;
        }
        if (gtk_toggle_button_get_active(togglebutton)){
            gtk_widget_set_sensitive(b, TRUE);
        } else {
            gtk_widget_set_sensitive(b, FALSE );
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(b), FALSE);
        }
    }
    


};


template <class Type>
class EFS: public Fuse<Type>{
/*    gchar *remotePath_;
    gchar *mountPoint_;
    gchar *ecryptfsSig_;
    gchar *ecryptfsFnekSig_;
    gboolean epsEnableFilenameCrypto_;
    gboolean epsPassthrough_;

    void getEfsOptions(void){
        remotePath_ = getOption("FUSE_REMOTE_PATH");
        mountPoint_ = getOption("FUSE_MOUNT_POINT");
        ecryptfsSig_ = getOption("ECRYPTFS_SIG");
        ecryptfsFnekSig_ = getOption("ECRYPTFS_FNEK_SIG");
        efsEnableFilenameCrypto_ = getCheck("EFS_ENABLE_FILENAME_CRYPTO");
        efsPassthrough_ = getCheck("EfS_PASSTHROUGH");

    }*/
    GtkEntry *remoteEntry_;
    GtkEntry *mountPointEntry_;
    GtkEntry *urlEntry_;
public:
    GtkEntry *mountPointEntry(void){return mountPointEntry_;}
    GtkEntry *remoteEntry(void){return remoteEntry_;}
    GtkEntry *urlEntry(void){return urlEntry_;}

    EFS(const gchar *url):
        Fuse<Type>(url, EFS_INFO1, EFS_INFO2)
    {
        this->setUrlTemplate("efs");
        DBG("EFS constructor entries\n");
        remoteEntry_ = this->addEntry(EFS_REMOTE_PATH, "FUSE_REMOTE_PATH");
        mountPointEntry_ = this->addEntry(FUSE_MOUNT_POINT, "FUSE_MOUNT_POINT");
        this->addEntry(ECRYPTFS_SIG, "ECRYPTFS_SIG", FALSE);
        urlEntry_ = this->addEntry(FUSE_URL, "FUSE_URL", FALSE);

        auto entryBuffer = gtk_entry_get_buffer (remoteEntry_);
        g_signal_connect(G_OBJECT(entryBuffer), "inserted-text", G_CALLBACK(updateUrl), this);
        g_signal_connect(G_OBJECT(entryBuffer), "inserted-text", G_CALLBACK(activateButtons), this);
        entryBuffer = gtk_entry_get_buffer (mountPointEntry_);
        g_signal_connect(G_OBJECT(entryBuffer), "inserted-text", G_CALLBACK(activateButtons), this);

        gtk_widget_set_sensitive(GTK_WIDGET(this->saveButton()), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(this->mountButton()), FALSE);

        DBG("EFS constructor checkboxes\n");

        this->addOptionPage(mount_options, _("Options"), 6 );
        this->addOptionPage(efs_options, _("Advanced"), 12);
        
        gtk_widget_show_all(GTK_WIDGET(this->dialog()));
    }
    ~EFS(void){
        DBG("efs destructor\n");
    }
    
    static void
    activateButtons (GtkEntryBuffer *buffer,
               guint           position,
               gchar          *chars,
               guint           n_chars,
               gpointer        data){
        auto efs = (EFS<Type> *)data;
        const gchar *path1 = gtk_entry_get_text(efs->remoteEntry());
        const gchar *path2 = gtk_entry_get_text(efs->mountPointEntry());
        auto condition = (g_file_test(path1, G_FILE_TEST_IS_DIR) && g_file_test(path2, G_FILE_TEST_IS_DIR));
        gtk_widget_set_sensitive(GTK_WIDGET(efs->saveButton()), condition);
        gtk_widget_set_sensitive(GTK_WIDGET(efs->mountButton()), condition);
    }
    
    static void
    updateUrl (GtkEntryBuffer *buffer,
               guint           position,
               gchar          *chars,
               guint           n_chars,
               gpointer        data){
        auto efs = (EFS<Type> *)data;
        const gchar *path = gtk_entry_get_text(efs->remoteEntry());
        auto u =g_strconcat(efs->urlTemplate(), "://", path, NULL);
        gtk_entry_set_text(efs->urlEntry(), u);
        g_free(u);
        gtk_entry_set_text(efs->mountPointEntry(), path);
    }

    static gboolean 
    checkPath(GtkWindow *parent, const gchar *path){
        if (!path) return FALSE;
        auto retval = TRUE;
        if (!strlen(path)) retval = FALSE;
        if (!g_file_test(path, G_FILE_TEST_EXISTS)) retval = FALSE;
        if (!g_file_test(path, G_FILE_TEST_IS_DIR)) retval = FALSE;
        if (!retval){
            auto message = g_strdup_printf("%s: \"%s\"\n", strerror(ENOENT), path);
            DBG("%s", message);
            //Dialogs<Type>::quickHelp(parent, message, "dialog-warning", _("Error"));
            g_free(message);
        }
        return retval;
    }

    gboolean save(void){
        auto dialog = G_OBJECT(this->dialog());
        auto entryPath = GTK_ENTRY(g_object_get_data(dialog, "FUSE_REMOTE_PATH"));
        auto entryMountPoint = GTK_ENTRY(g_object_get_data(dialog, "FUSE_MOUNT_POINT"));
        auto path = gtk_entry_get_text(entryPath);
        auto mountPoint = gtk_entry_get_text(entryMountPoint);
        if (!this->checkPath(GTK_WINDOW(dialog), path)) return FALSE;
        if (!this->checkPath(GTK_WINDOW(dialog), mountPoint)) return FALSE;
        getOptions();
        return TRUE;

    }

    void getOptions(void){
        group_options_t *options_p = efs_options;
        for (auto p=options_p; p && p->flag; p++){
            auto box = GTK_BOX(g_object_get_data(G_OBJECT(this->dialog()), p->id));
            if (!box) {
                DBG("getOptions(): cannot find item \"%s\"\n", p->id);
                continue;
            }
            auto check = GTK_TOGGLE_BUTTON(g_object_get_data(G_OBJECT(box), "check")); 
            auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(box), "entry")); 
            if (gtk_toggle_button_get_active(check)) {
                DBG("Option --> %s%s\n", p->id, 
                        (entry)?gtk_entry_get_text(entry):"");
            }
            else TRACE("no check:  %s\n", p->id);
        }    
    }
};

} // namespace xf
#endif
