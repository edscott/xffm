
#ifndef ECRYPTFS_HH
#define ECRYPTFS_HH
#define FUSE_KEY_FILE		g_get_user_config_dir(),"xffm+","fuse.ini"

#define EFS_AUTHORIZATION _("Encryption Options")
#define EFS_TIP _("Encrypt Files")
#define ECRYPTFS_SIG _("Mount ecrypt signature")
#define ECRYPTFS_FNEK_SIG _("Filename ecrypt signature")
#define EFS_INFO1 _("Ecrypt Filesystem (EFS)")
#define EFS_INFO2 _("New EFS Link")
#define EFS_REMOTE_PATH _("Encrypted directory")
#define EPS_ENABLE_FILENAME_CRYPTO _("Encrypt filenames")
#define EPS_REQUIRES_SIGNATURE _("Requires ecryptfs signature")
#define EPS_PASSTHROUGH _("Plaintext passthrough")

#define FUSE_BROADBAND  _("local filesystem")
#define FUSE_COMPUTER _("Computer Name:")
#define FUSE_REMOTE_PATH  _("Remote Path")
#define FUSE_MOUNT_POINT  _("Mount point:")
#define FUSE_URL  _("URL")
#define FUSE_MONITOR  _("Enable file monitoring")
#define FUSE_CAUTION  _("Caution")
#define FUSE_LOGIN  _("Username:")
#define FUSE_SECURE_SHELL_KEY  _("Secure Shell Key")
#define FUSE_ALLOW_EMPTY_PASSPHRASE  _("Allow empty private key passphrase")
#define FUSE_PASSPHRASE  _("Enter your Secure Shell passphrase:")

namespace xf{


template <class Type>
class Fuse  {
public:
    GtkDialog *dialog_;
    GKeyFile *keyfile_;
    const gchar *url_;
    GtkBox *vbox_;

    Fuse(const gchar *url, const gchar *info1, const gchar *info2){
        DBG("Fuse constructor(%s, %s, %s)\n", url, info1, info2);
        
        url_ = url;
        keyfile_ = loadKeyfile(url);
        dialog_ = initDialog(info1, info2);
    }

    ~Fuse(void){
        gtk_widget_destroy(GTK_WIDGET(dialog_));        
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
        gtk_widget_show(GTK_WIDGET(hbox));

        auto pixbuf = Pixbuf<Type>::get_pixbuf("dialog-question", -24);
        auto image = gtk_image_new_from_pixbuf(pixbuf);
        g_object_unref(pixbuf);
        gtk_widget_show(image);
        gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

        auto text = g_strconcat(_("Options:"), " ", 
                info1, "\n\n", 
                info2, NULL);


        
        auto labelview = gtk_text_view_new();
        gtk_text_view_set_editable (GTK_TEXT_VIEW(labelview), FALSE);
        gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW(labelview), FALSE);
        gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW(labelview), GTK_WRAP_WORD);
        
        GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(labelview));
        GtkTextIter iter;
        gtk_text_buffer_get_start_iter (buffer, &iter);
        gtk_text_buffer_insert (buffer,&iter, text, -1);

        g_free(text);
        gtk_widget_show(labelview);
        gtk_box_pack_start (hbox, labelview, TRUE, TRUE, 0);


        auto tbox = Gtk<Type>::vboxNew(FALSE, 0);
        gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG(dialog))), GTK_WIDGET(tbox), FALSE, FALSE, 0);
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
        vbox_ = Gtk<Type>::vboxNew (FALSE, 0);

        gtk_widget_show(GTK_WIDGET(vbox_));
        
        GtkWidget *tab_label = gtk_label_new(_("Login"));
        GtkWidget *menu_label = gtk_label_new(_("Login"));

        gtk_notebook_insert_page_menu (GTK_NOTEBOOK(notebook), GTK_WIDGET(vbox_), tab_label, menu_label, 0);
        gtk_notebook_set_tab_reorderable (GTK_NOTEBOOK(notebook), GTK_WIDGET(vbox_), TRUE);

        auto action_area = Gtk<Type>::hboxNew(FALSE, 1);
        gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area(GTK_DIALOG(dialog))), 
                GTK_WIDGET(action_area), FALSE, FALSE, 0);
        gtk_widget_show(GTK_WIDGET(action_area));



        auto button = Gtk<Type>::dialog_button ("window-close", _("Cancel"));
        gtk_box_pack_start (GTK_BOX (action_area), GTK_WIDGET(button), FALSE, FALSE, 0);
        g_object_set_data (G_OBJECT (dialog), "action_FALSE_button", button);


        button = Gtk<Type>::dialog_button ("media-floppy", _("Save"));
        g_object_set_data (G_OBJECT (dialog), "action_TRUE_button", button);
        gtk_box_pack_start (GTK_BOX (action_area), GTK_WIDGET(button), FALSE, FALSE, 0);

        button = Gtk<Type>::dialog_button ("greenball", _("Mount"));
        g_object_set_data (G_OBJECT (dialog), "action_MOUNT_button", button);
        gtk_box_pack_start (GTK_BOX (action_area), GTK_WIDGET(button), FALSE, FALSE, 0);

        gtk_window_set_resizable (GTK_WINDOW(dialog), TRUE);


        return GTK_DIALOG(dialog);

    }
#if 0
    GtkBox *mountPointBox; // FUSE_MOUNT_POINT_BOX

    void
    addEntry(const gchar *item_string, const gchar *item_id){
        
        const gchar *separator=" ";
        if (item_string && !strchr(item_string, ':')){
            separator = ": ";
        }

        GtkWidget *hbox = fuse_make_entry_box(dialog_, item_string, item_id, separator, TRUE, on_key_press);
        if (strcmp(item_id, "FUSE_MOUNT_POINT")==0){
            mountPointBox = hbox;
        }
            
            
        GtkWidget *entry = g_object_get_data(G_OBJECT(dialog_), item_id);
        if (strcmp(item_id, "FUSE_REMOTE_PATH")==0) {
            g_signal_connect(G_OBJECT(entry), "key-release-event", G_CALLBACK(make_absolute), dialog_);
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
        fuse_set_entry(entry, keyfile_, url_, item_id, default_value);
        gtk_box_pack_start (GTK_BOX (vbox_), hbox, FALSE, FALSE, 0);
        g_free(default_value);
        return hbox;
    }

    gchar *
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
    GtkDialog *dialog(void){ return dialog_;}

};

template <class Type>
class EFS: public Fuse<Type>{

public:
    EFS(const gchar *url):
        Fuse<Type>(url, EFS_INFO1, EFS_INFO2)
    {
        DBG("EFS constructor\n");
        //this->addEntry(EFS_REMOTE_PATH, "FUSE_REMOTE_PATH");
        gtk_widget_show_all(GTK_WIDGET(this->dialog_));
        //this->addEntry(EFS_REMOTE_PATH, "FUSE_REMOTE_PATH");
        
    }


};

} // namespace xf
#endif
