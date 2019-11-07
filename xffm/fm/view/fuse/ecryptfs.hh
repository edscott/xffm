
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


#define FLAG_KEY(x)  g_strdup_printf("FLAG_%d", x)

namespace xf{
//#define GroupOptions group_options_t
typedef struct group_options_t {
    gint sensitive;
    const gchar *flag;
    const gchar *id;
    const gchar *text;
    const gchar *entry;
    const gchar *tip;
}group_options_t;

static gchar **mount_option_keys = NULL;
static  group_options_t mount_options[]={
//linux options:
    {
	.sensitive = 1, .flag = "-r", .id = NULL, 
        .text = N_(" Mount file system readonly."), 
	.entry = NULL, .tip = NULL
    },
    {
	.sensitive = 1, .flag = "-v", .id = NULL, 
        .text = N_("Be verbose."), 
	.entry = NULL, .tip = NULL
    },
    {
	.sensitive = 1, .flag = "-V", .id = NULL, 
        .text = N_("Print version."), 
	.entry = NULL, .tip = NULL
    },
    {
	.sensitive = 2, .flag = "-w", .id = NULL, 
        .text = N_("Mount file system read-write."), 
	.entry = NULL, .tip = NULL
    },
//#ifdef THIS_IS_LINUX
    {
	.sensitive = 1, .flag = "-f", .id = NULL, 
        .text = N_("Fake mount."),  
	.entry = NULL, 
        .tip = N_("Don't actually call the mount system call.")
    },
    {
	.sensitive = 1, .flag = "-n", .id = NULL, 
        .text = N_("Do  not  update  /etc/mtab."), 
	.entry = NULL,
	.tip = 
N_("By default, an entry is created in/etc/mtab for every mounted file system.\n Use this option to skip making an entry.")
    },
    {
	.sensitive = 1, .flag = "-s", .id = NULL, 
	.text = N_("Tolerate sloppy options."),
	.entry = NULL,
        .tip = N_("Tolerate sloppy mount options rather than fail.") 
    },
//#endif
    {
	.sensitive = 1, .flag = "-h", .id = NULL, 
        .text = N_("Print help message."), 
	.entry = NULL, .tip = NULL
    },
    
    {.sensitive = 1, NULL, NULL, NULL, NULL, NULL}
};

static gchar **efs_option_keys = NULL;
static  group_options_t efs_options[]={
    {
	.sensitive = 1, .flag = "-o", .id = "ecryptfs_sig=", 
        .text = NULL, 
	.entry = N_("(fekek_sig)"),
        .tip = N_("Specify  the  signature  of the mount wide authentication token. The authentication token must be in the  kernel  keyring  before the  mount  is performed. ecryptfs-manager or the eCryptfs mount helper can be used to construct the authentication token and add it to the keyring prior to mounting.") 
    },
    
    {
	.sensitive = 1, .flag = "-o", .id = "ecryptfs_fnek_sig=", 
        .text = NULL, 
	.entry = N_("(fnek_sig)"),
        .tip = N_("Specify  the  signature  of  the mount wide authentication token used for filename crypto. The authentication must be in the kernel keyring before mounting.")
    },
    
    {
	.sensitive = 2, .flag = "-o", .id = "ecryptfs_cipher=", 
	.text = "aes | blowfish | des3_ede | cast6 | cast5",
	.entry = "aes",
        .tip = N_("Specify the symmetric cipher to be used on a per file basis. ")
    },
    
    {
	.sensitive = 2, .flag = "-o", .id = "ecryptfs_key_bytes=", 
        .text = NULL, 
	.entry = "16",
        .tip = N_("Specify  the keysize to be used with the selected cipher. If the cipher only has one keysize the keysize  does  not  need  to  be specified.\n\n\taes: blocksize = 16; min keysize = 16; max keysize = 32\n\tblowfish: blocksize = 16; min keysize = 16; max keysize = 56\n\tdes3_ede: blocksize = 8; min keysize = 24; max keysize = 24\n\tcast6: blocksize = 16; min keysize = 16; max keysize = 32\n\tcast5: blocksize = 8; min keysize = 5; max keysize = 16\n\t")
    },
    
    {
	.sensitive = 0, .flag = "-o", .id = "ecryptfs_passthrough=", 
        .text = NULL, 
	.entry = "no",
        .tip = N_("Allows for non-eCryptfs files to be read and written from within an eCryptfs mount. This option is turned off by default.")
    },
    
    {
	.sensitive = -1, .flag = "-o", .id = "no_sig_cache", 
        .text = NULL, 
	.entry = NULL,
        .tip = N_("Do not check the mount key signature against the values  in  the user's  ~/.ecryptfs/sig-cache.txt  file. This is useful for such things as non-interactive  setup  scripts,  so  that  the  mount helper  does  not stop and prompt the user in the event that the key sig is not in the cache.")
    },
    
    {
	.sensitive = 1, .flag = "-o", .id = "ecryptfs_encrypted_view", 
        .text = NULL, 
	.entry = NULL,
        .tip = N_("This option provides a unified  encrypted  file  format  of  the eCryptfs  files in the lower mount point.  Currently, it is only useful if the lower mount point contains files with the metadata stored in the extended attribute.  Upon a file read in the upper mount point, the encrypted version of the file will be presented with  the  metadata  in  the  file  header instead of the xattr.  Files cannot be opened for writing when this option is enabled.")
    },
    
    {
	.sensitive = 1, .flag = "-o", .id = "ecryptfs_xattr", 
        .text = NULL, 
	.entry = NULL,
        .tip = N_("Store the metadata in the extended attribute of the lower  files rather than the header region of the lower files.")
    },
    
    {
	.sensitive = 1, .flag = "-o", .id = "verbose", 
        .text = NULL, 
	.entry = NULL,
        .tip = N_("Log  ecryptfs  information  to  /var/log/messages.   Do  not run eCryptfs in verbose-mode unless you are doing so  for  the  sole purpose  of development, since secret values will be written out to the system log in that case.")
    },
        
    {
	.sensitive = 0, .flag = "-o", .id = "ecryptfs_enable_filename_crypto=", 
        .text = NULL, 
	.entry = "no",
        .tip = N_("Specify whether filename encryption should be enabled.  If  not, the  mount  helper  will  not  prompt  the user for the filename encryption key signature.")
    },
    
    {
	.sensitive = -1, .flag = "-o", .id = "verbosity=", 
        .text = NULL, 
	.entry = "1",
        .tip = N_("If verbosity=1, the mount helper will ask you for missing values (default).  Otherwise, if verbosity=0, it will not ask for missing values and will fail if required values are omitted.  ")
    },
    
    {
	.sensitive = 1, .flag = "-o", .id = "passphrase_passwd=", 
        .text = NULL, 
	.entry = N_("(passphrase)"),
        .tip = N_("The actual password is passphrase. Since the password is visible to utilities (like ps under Unix) this form should only be  used where security is not important.")
    },
    
    {
	.sensitive = 1, .flag = "-o", .id = "passphrase_passwd_file=", 
        .text = NULL, 
	.entry = N_("(filename)"),
        .tip = N_("The    password   should   be   specified   in   a   file   with passwd=(passphrase). It is highly reccomended that the  file  be stored on a secure medium such as a personal usb key.")
    },
    
    {
	.sensitive = 1, .flag = "-o", .id = "passphrase_passwd_fd=", 
        .text = NULL, 
	.entry = N_("(file descriptor)"),
        .tip = N_("The password is specified through the specified file descriptor.")
    },
    
    {
	.sensitive = 1, .flag = "-o", .id = "passphrase_salt=", 
        .text = NULL, 
	.entry = N_("(hex value)"),
        .tip = N_("The salt should be specified as a 16 digit hex value.  ")
    },
    
    {
	.sensitive = 1, .flag = "-o", .id = "openssl_keyfile=", 
        .text = NULL, 
	.entry = N_("(filename)"),
        .tip = N_("The   password   should   be   specified   in   a   file    with openssl_passwd=(openssl-password). It is highly reccomended that the file be stored on a secure medium such  as  a  personal  usb key.")
    },
    
    
    
    {
	.sensitive = 1, .flag = "-o", .id = "openssl_passwd_file=", 
        .text = NULL, 
	.entry = N_("(filename)"),
        .tip = N_("The   password   should   be   specified   in   a   file    with openssl_passwd=(openssl-password). It is highly reccomended that the file be stored on a secure medium such  as  a  personal  usb key.")
    },
    {
	.sensitive = 1, .flag = "-o", .id = "openssl_passwd_fd=", 
        .text = NULL, 
	.entry = N_("(file descriptor)"),
        .tip = N_("The password is specified through the specified file descriptor.")
    },
    {
	.sensitive = 1, .flag = "-o", .id = "openssl_passwd=", 
        .text = NULL, 
	.entry = N_("(password)"),
        .tip = N_("The  password  can  be  specified on the command line. Since the password is visible in the process list,  it  is  highly  recommended to use this option only for testing purposes.")
    },
    
    {.sensitive = 1, NULL, NULL, NULL, NULL, NULL}
};



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
public:
    GtkToggleButton * monitor(void){return monitor_;}
    GtkWidget * allowEmptyPassphrase(void){return allowEmptyPassphrase_;}
    GtkDialog *dialog(void){ return dialog_;}
    void setUrlTemplate(const gchar *value){ urlTemplate_ = value;}
    const gchar *urlTemplate(void){ return urlTemplate_;}
    GtkBox *vbox(void){return vbox_;}
    
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
        gtk_widget_destroy(GTK_WIDGET(dialog_));        
    }

    GtkBox *
    addEntry(const gchar *item_string, const gchar *item_id, gboolean state=TRUE){
        
        const gchar *separator=" ";
        if (item_string && !strchr(item_string, ':')){
            separator = ": ";
        }

        auto hbox = makeEntryBox(this, item_string, item_id, separator, TRUE, (gpointer)on_key_press);
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
        return hbox;
    }
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
        gtk_widget_show(label);
        gtk_widget_show(GTK_WIDGET(hbox));
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

    void *
    addOptionPage(group_options_t *options_p, const gchar *label, gint flag_id) {
        auto key_file = (GKeyFile *)g_object_get_data(G_OBJECT(this->dialog()), 
                "key_file");
        auto url = (const gchar *)g_object_get_data(G_OBJECT(this->dialog()),
                "url");

        GtkWidget *vbox = optionsBox(this->dialog(), 
                options_p, key_file, url, flag_id);
        gtk_widget_show(GTK_WIDGET(vbox));
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
        void **arg = data;
        fuse_data_t *(*dialog_f)(const gchar *url) = arg[0];
        const gchar *module_name = arg[2];
        g_free(arg);
        widgets_t * widgets_p = rfm_get_widget("widgets_p");
        GCond *signal = fuse_hold_monitor();
        fuse_data_t *fuse_data_p = (*dialog_f) (url_);
        if(!fuse_data_p || !fuse_data_p->dialog){
            return GINT_TO_POINTER(FALSE);
        }
        gint response = GTK_RESPONSE_CANCEL;
        GtkNotebook *notebook = 
            g_object_get_data(G_OBJECT(fuse_data_p->dialog), "notebook");
        gtk_notebook_set_current_page(notebook, 0);

        GtkWidget *button;

        button = g_object_get_data(G_OBJECT(fuse_data_p->dialog), "action_TRUE_button");
        g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (button_ok), &response);
        button = g_object_get_data(G_OBJECT(fuse_data_p->dialog), "action_FALSE_button");
        g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (button_cancel), &response);
        button = g_object_get_data(G_OBJECT(fuse_data_p->dialog), "action_MOUNT_button");
        g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (button_mount), &response);

        g_signal_connect (G_OBJECT (fuse_data_p->dialog), "delete-event", G_CALLBACK (response_delete), &response);
    retry:;
        
        gtk_widget_show_all (fuse_data_p->dialog);
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

private:

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
            const gchar *text, 
            const gchar *id, 
            const gchar *colon, 
            gboolean visibility, 
            gpointer callback){
        auto hbox = Gtk<Type>::hboxNew(FALSE, 0);
        auto full_text = g_strconcat(text, colon, NULL);
        auto label = gtk_label_new(full_text);
        g_free(full_text);
        auto entry = gtk_entry_new();
        gtk_entry_set_visibility (GTK_ENTRY(entry), visibility);
        g_object_set_data(G_OBJECT(fuse->dialog()), id, entry);
        gtk_box_pack_start (hbox, label, FALSE, FALSE, 0);
        gtk_box_pack_start (hbox, entry, TRUE, TRUE, 0);
        gtk_widget_show(GTK_WIDGET(hbox));
        gtk_widget_show(label);
        gtk_widget_show(entry);
        if (callback) {
            g_signal_connect (G_OBJECT (entry), "key-release-event", G_CALLBACK (callback), fuse);
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
        gtk_widget_show(check);
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

            
        gtk_container_add(GTK_CONTAINER(sw), GTK_WIDGET(vbox));


        gint i=0;
        for (; options_p && options_p->flag; options_p++){

          auto hbox = Gtk<Type>::hboxNew(0, FALSE);
          gtk_widget_show(GTK_WIDGET(hbox));
          GtkWidget *check;
          GtkWidget *entry=NULL;
          gchar *check_text;
          if (options_p->entry){
              check_text = 
                g_strdup_printf("%s %s",  options_p->flag,
                        options_p->id);
          } else {
              check_text = g_strdup_printf("%s %s",  options_p->flag,
                        (options_p->id)?options_p->id:"");
          }

            check = 
                gtk_check_button_new_with_label(check_text);
            /*if (options_p->tip){
                rfm_add_custom_tooltip(check, NULL, options_p->tip);
            } else if (options_p->text && options_p->entry){
                rfm_add_custom_tooltip(check, NULL, options_p->text);
            }*/
            g_free(check_text);
            gtk_widget_show(check);
            gtk_box_pack_start (GTK_BOX (hbox), check, FALSE, FALSE, 0);

          GtkWidget *label=NULL;
          if (options_p->entry)  {
            entry = gtk_entry_new();
            gtk_widget_show(entry);
            gtk_box_pack_start (GTK_BOX (hbox), entry, FALSE, FALSE, 0);
          }
          else if (options_p->text) {
              // not for entries...
            gchar *g = g_strdup_printf(" <i>(%s)</i>", _(options_p->text));
            label=gtk_label_new("");
            gtk_label_set_markup(GTK_LABEL(label), g);
            g_free(g);
            gtk_widget_show(label);
            gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
          }

            gtk_widget_set_sensitive(GTK_WIDGET(hbox), (options_p->sensitive > 0));
            gchar *id_a[]={(gchar *)options_p->flag, (gchar *)options_p->id, NULL};
            gchar *id = getOptionId(id_a);
            if (g_object_get_data(G_OBJECT(dialog), id)) {
                DBG("Duplicate option: %s\n", id);
            }
           g_object_set_data(G_OBJECT(dialog), id, check);
           if (entry){
                gchar *idd_a[]={(gchar *)options_p->flag, (gchar *)options_p->id, (gchar *)"Entry", NULL};
                gchar *idd = getOptionId(idd_a);
                TRACE( "++ setting entry idd to %s\n", idd);
                if (g_object_get_data(G_OBJECT(dialog), idd)) {
                    DBG("Duplicate entry: %s\n", idd);
                }
                g_object_set_data(G_OBJECT(dialog), idd, entry);
                g_free(idd);
           }
        
           if (key_file) {
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
           }
           if (flag & (ui << i)){
               gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), TRUE);
           }
           g_signal_connect (check, "toggled", G_CALLBACK (option_toggle), 
                   GINT_TO_POINTER(i));
           g_free(id);
           i++;

          gtk_box_pack_start (vbox, GTK_WIDGET(hbox), FALSE, FALSE, 0);      
        }
        gtk_widget_show(GTK_WIDGET(vbox));
        gtk_widget_set_size_request(sw, -1, 300);
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
    button_ok (GtkButton * button, gpointer data) {
        gint *response = data;
        *response = GTK_RESPONSE_APPLY;
        gtk_main_quit();
    }

    static void
    button_mount (GtkEntry * entry, gpointer data) {
        gint *response = data;
        *response = GTK_RESPONSE_YES;
        gtk_main_quit();
    }

    static void
    button_cancel (GtkButton * button, gpointer data) {
        gint *response = data;
        *response = GTK_RESPONSE_CANCEL;
        gtk_main_quit();
    }


    static gboolean 
    response_delete(GtkWidget *dialog, GdkEvent *event, gpointer data){
        gint *response = data;
        *response = GTK_RESPONSE_CANCEL;
        gtk_main_quit();
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

public:
    EFS(const gchar *url):
        Fuse<Type>(url, EFS_INFO1, EFS_INFO2)
    {
        this->setUrlTemplate("efs");
        DBG("EFS constructor entries\n");
        this->addEntry(EFS_REMOTE_PATH, "FUSE_REMOTE_PATH");
        this->addEntry(FUSE_MOUNT_POINT, "FUSE_MOUNT_POINT", FALSE);
        this->addEntry(ECRYPTFS_SIG, "ECRYPTFS_SIG", FALSE);
        this->addEntry(ECRYPTFS_FNEK_SIG, "ECRYPTFS_FNEK_SIG", FALSE);
        this->addEntry(FUSE_URL, "FUSE_URL");
        auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(this->dialog()), "FUSE_URL"));
        auto u =g_strconcat(this->urlTemplate(), "://", NULL);
                        DBG("1.1 value=%s\n", u);
        gtk_entry_set_text(entry, u);
        g_free(u);
        gtk_widget_set_sensitive(GTK_WIDGET(entry), FALSE);

        DBG("EFS constructor checkboxes\n");

        this->addCheck(EPS_ENABLE_FILENAME_CRYPTO, "EPS_ENABLE_FILENAME_CRYPTO", TRUE, EPS_REQUIRES_SIGNATURE);
        this->addCheck(EPS_PASSTHROUGH, "EPS_PASSTHROUGH", FALSE );


        this->addOptionPage(mount_options, "Mount", 6 );
        this->addOptionPage(efs_options, "EcryptFS", 12);

        
        gtk_widget_show_all(GTK_WIDGET(GTK_WIDGET(this->dialog())));
    }


};

} // namespace xf
#endif
