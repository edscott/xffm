
#ifndef ECRYPTFS_HH
#define ECRYPTFS_HH

#include "ecryptfs.i"

namespace xf{
#ifndef ENABLE_EFS_MODULE 
template <class Type>class EFS{
public:
    static void doDialog(const gchar *path){return;}
    static gboolean isEFS(const gchar *path){return (strncmp(path, "efs:/", strlen("efs:/"))==0);}
    static gboolean removeItem(const gchar *path){return FALSE;}
};

#else
static pthread_mutex_t efsMountMutex=PTHREAD_MUTEX_INITIALIZER;
template <class Type>class EFS;
template <class Type>class Fstab;

template <class Type>
class Fuse  {
    GtkDialog *dialog_;
    GtkBox *vbox_;
    const gchar *urlTemplate_;
    GtkBox *mountPointBox_; // FUSE_MOUNT_POINT_BOX
    //GtkButton *loadButton_;
    GtkButton *saveButton_;
    GtkButton *cancelButton_;
    GtkButton *mountButton_;
    gint response_;
    
public:
    GtkDialog *dialog(void){ return dialog_;}
    void setUrlTemplate(const gchar *value){ urlTemplate_ = value;}
    const gchar *urlTemplate(void){ return urlTemplate_;}
    GtkBox *vbox(void){return vbox_;}
    gint response(void){return response_;}
    GtkButton *saveButton(void){return saveButton_;}
    GtkButton *mountButton(void){return mountButton_;}
    
    Fuse(const gchar *info1, const gchar *info2)
    {
        TRACE("Fuse constructor(%s, %s, %s)\n", url, info1, info2);
        dialog_ = initDialog(info1, info2);
    }

    ~Fuse(void){
        TRACE("Fuse destructor...\n");
        gtk_widget_destroy(GTK_WIDGET(dialog_));        
    }

    static gboolean isEFS(const gchar *path){return (strncmp(path, "efs:/", strlen("efs:/"))==0);}

    static gchar **
    getSavedItems(void){
        gchar *file = g_build_filename(EFS_KEY_FILE, NULL);
        GKeyFile *key_file = g_key_file_new ();
        g_key_file_load_from_file (key_file, file, (GKeyFileFlags)(G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS), NULL);
        g_free(file);
        auto retval = g_key_file_get_groups (key_file, NULL);
        g_key_file_free(key_file);
        return retval;
    }

    GtkEntry *
    addEntry(const gchar *item_string, const gchar *item_id, gboolean state=TRUE){
        auto hbox = makeEntryBox(this, item_string, item_id, state, TRUE);
        g_object_set_data(G_OBJECT(this->dialog()), item_id, hbox);
        gtk_widget_set_sensitive(GTK_WIDGET(hbox), state);
        auto entry = (GtkEntry *)g_object_get_data(G_OBJECT(hbox), "entry");
        gtk_box_pack_start (GTK_BOX (vbox_), GTK_WIDGET(hbox), FALSE, FALSE, 0);
        return entry;
    }

    void *
    addOptionPage(group_options_t *options_p, const gchar *label, gint flag_id) {
        auto key_file = (GKeyFile *)g_object_get_data(G_OBJECT(this->dialog()), 
                "key_file");
        auto url = (const gchar *)g_object_get_data(G_OBJECT(this->dialog()),
                "url");

        GtkWidget *scolledWindow = optionsBox(this->dialog(), 
                options_p, key_file, url, flag_id);
        GtkWidget *tab_label = gtk_label_new(label);
        GtkWidget *menu_label = gtk_label_new(label);
        auto notebook = (GtkNotebook *) g_object_get_data(G_OBJECT(this->dialog()), 
                "notebook");
        gtk_notebook_append_page_menu (notebook, GTK_WIDGET(scolledWindow), tab_label, 
                menu_label);  
        gtk_notebook_set_tab_reorderable (notebook, GTK_WIDGET(scolledWindow), TRUE);
        return NULL;
    }


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

        auto pixbuf = Pixbuf<Type>::getPixbuf("dialog-question", -24);
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

        //loadButton_ = Gtk<Type>::dialog_button ("document-open", _("Load"));
        //gtk_box_pack_start (GTK_BOX (action_area), GTK_WIDGET(loadButton_), FALSE, FALSE, 0);

        mountButton_ = Gtk<Type>::dialog_button ("greenball", _("Mount"));
        gtk_box_pack_start (GTK_BOX (action_area), GTK_WIDGET(mountButton_), FALSE, FALSE, 0);


        //g_signal_connect (G_OBJECT (loadButton_), "clicked", G_CALLBACK (button_load), this);
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
        gchar *file = g_build_filename(EFS_KEY_FILE, NULL);
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
        g_object_set_data(G_OBJECT(hbox), "entry", entry);
        g_object_set_data(G_OBJECT(hbox), "label", label);
        gtk_box_pack_start (hbox, label, FALSE, FALSE, 0);
        gtk_box_pack_start (hbox, entry, TRUE, TRUE, 0);
        if (withSelector){
            auto button = Gtk<Type>::dialog_button ("document-open", NULL);
            g_object_set_data(G_OBJECT(hbox), "button", button);
            g_signal_connect (G_OBJECT (button), "clicked", 
                    G_CALLBACK (ChooserResponse<Type>::folderChooser), entry);
            gtk_box_pack_start (hbox, GTK_WIDGET(button), FALSE, FALSE, 0);
        }
        return hbox;
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
        
          const gchar *labelColor = (options_p->sensitive ==0)?"gray":"red";

          auto checkMarkup = g_strdup_printf("<span color=\"%s\">%s</span>", 
                  (options_p->entry)?labelColor:"blue",
                  options_p->id);

          auto label = gtk_label_new("");
          gtk_label_set_markup(GTK_LABEL(label), checkMarkup);
          g_free(checkMarkup);

          auto check = gtk_check_button_new_with_label(options_p->flag);
          gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), 
                  options_p->sensitive > 1 || options_p->sensitive < 0);

          g_object_set_data(G_OBJECT(hbox),"check", check);
          g_object_set_data(G_OBJECT(hbox),"label", label);

          gtk_box_pack_start (GTK_BOX (vbox2), GTK_WIDGET(hbox), FALSE, FALSE, 0);
          gtk_box_pack_start (GTK_BOX (vbox2), GTK_WIDGET(hbox2), TRUE, TRUE, 0);
          gtk_box_pack_start (GTK_BOX (hbox), check, FALSE, FALSE, 0);
          gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
          gtk_widget_set_sensitive(GTK_WIDGET(label), (options_p->sensitive > 0) && options_p->sensitive != 3);

          if (options_p->entry)  {
            auto entry = GTK_ENTRY(gtk_entry_new());
            g_object_set_data(G_OBJECT(hbox),"entry", entry);
            gtk_entry_set_text(entry, options_p->entry);
            gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET(entry), FALSE, FALSE, 0);
            gtk_widget_set_sensitive(GTK_WIDGET(entry), (options_p->sensitive > 0));
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

          gtk_widget_set_sensitive(GTK_WIDGET(check), (options_p->sensitive > 0 && options_p->sensitive != 3));
          gtk_widget_set_sensitive(GTK_WIDGET(check), (options_p->sensitive > 0 && options_p->sensitive != 3));
        

          i++;

          gtk_box_pack_start (vbox, GTK_WIDGET(vbox2), FALSE, FALSE, 0);      
        }
        gtk_widget_set_size_request(sw, 400, -1);
        return sw;
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


};


template <class Type>
class EFS: public Fuse<Type>{
    GtkEntry *remoteEntry_;
    GtkEntry *mountPointEntry_;
    GtkEntry *urlEntry_;
public:
    GtkEntry *mountPointEntry(void){return mountPointEntry_;}
    GtkEntry *remoteEntry(void){return remoteEntry_;}
    GtkEntry *urlEntry(void){return urlEntry_;}

    EFS(const gchar *path):
        Fuse<Type>(EFS_INFO1, EFS_INFO2)
    {
        this->setUrlTemplate("efs");
        DBG("EFS constructor entries\n");
        remoteEntry_ = this->addEntry(EFS_REMOTE_PATH, "FUSE_REMOTE_PATH");
        mountPointEntry_ = this->addEntry(FUSE_MOUNT_POINT, "FUSE_MOUNT_POINT");
        //this->addEntry(ECRYPTFS_SIG, "ECRYPTFS_SIG", FALSE);
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

        if (path) setOptions(path);
        
        gtk_widget_show_all(GTK_WIDGET(this->dialog()));
    }
    ~EFS(void){
        DBG("efs destructor\n");
    }
    
    static gboolean
    removeItem(const gchar *group){
        GKeyFile *key_file = g_key_file_new ();
        gchar *file = g_build_filename(EFS_KEY_FILE, NULL);
        auto retval = FALSE;
        if (g_key_file_load_from_file (key_file, file, (GKeyFileFlags)(G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS), NULL)){
            retval = g_key_file_remove_group (key_file, group, NULL);
            if (retval) g_key_file_save_to_file(key_file, file, NULL);
        }
       g_key_file_free(key_file);
       g_free(file);
       return retval;
    }

    static void
    doDialog( const gchar *path, void *data){
        auto view =  (View<Type> *)g_object_get_data(G_OBJECT(data), "view");
        doDialog(path, view);
    }

    static void
    doDialog( const gchar *path, View<Type> *view){
        auto efs = new(EFS<Type>)(path);
        gint response  = gtk_dialog_run(efs->dialog());
        DBG("menuAddEFS(): efs response=%d (%d,%d,%d)\n", 
                response,GTK_RESPONSE_YES,GTK_RESPONSE_APPLY,GTK_RESPONSE_CANCEL);
        gchar *efsmount=NULL;
        switch (response){
            case GTK_RESPONSE_YES: 
             // mount
                efsmount = g_find_program_in_path("mount.ecryptfs");
                if (not efsmount){
                    auto message = g_strdup_printf("%s: mount.ecryptfs\n", strerror(ENOENT));
                    auto cancel = Dialogs<int>::quickDialog(mainWindow, message, "dialog-error", "mount");
                    gtk_widget_show_all (GTK_WIDGET(cancel));
                    gtk_dialog_run(GTK_DIALOG(cancel));
                    break;
                }
                g_free(efsmount);
                // This here will ask for passphrase before mounting:
                efs->mountUrl(view);
                // 
                break;
            case GTK_RESPONSE_APPLY: // Save
                efs->save();
                break;
            default:
            case GTK_RESPONSE_CANCEL:
                break;
        }
        delete(efs);
    }
    
    void setOptions(const gchar *url){
        DBG("efs setOptions %s\n", url);
        GKeyFile *key_file = g_key_file_new ();
        gchar *file = g_build_filename(EFS_KEY_FILE, NULL);
        if (!g_key_file_load_from_file (key_file, file, (GKeyFileFlags)(G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS), NULL)){
            DBG("g_key_file_load_from_file(%s) failed.\n", file);
        }
        if (!g_key_file_has_group (key_file, url)){
            DBG("setOptions(): failed g_key_file_has_group(%s)\n", url);
            g_key_file_free(key_file);
            return;
        }
        auto mountPoint = g_key_file_get_string (key_file, url, "mountPoint", NULL);
        if (!mountPoint) {
            DBG("setOptions(): failed mountPoint\n");
            g_key_file_free(key_file);
            return;
        }
        auto mountOptions = g_key_file_get_string (key_file, url, "mountOptions", NULL);
        if (!mountOptions) {
            DBG("setOptions(): failed mountOptions\n");
            g_key_file_free(key_file);
            return;
        }
        auto efsOptions = g_key_file_get_string (key_file, url, "efsOptions", NULL);
        if (!efsOptions){
            DBG("setOptions(): failed efsOptions\n");
            g_key_file_free(key_file);
            return;
        }
        gtk_entry_set_text(this->remoteEntry(), url);
        gtk_entry_set_text(this->mountPointEntry(), mountPoint);

        // clear all checks...
        for (auto p=mount_options; p->id; p++) {
            auto box = G_OBJECT(g_object_get_data(G_OBJECT(this->dialog()), p->id));
            auto check = GTK_TOGGLE_BUTTON(g_object_get_data(box, "check"));
            gtk_toggle_button_set_active(check, FALSE);

        }
        // set saved checks...
        auto mOptions = g_strsplit(mountOptions, ",", -1);
        g_free(mountOptions);
        for (auto o= mOptions; o && *o; o++){
            for (auto p=mount_options; p->id; p++) {
                if (strcmp(*o, p->flag)==0){
                    auto box = G_OBJECT(g_object_get_data(G_OBJECT(this->dialog()), p->id));
                    auto check = GTK_TOGGLE_BUTTON(g_object_get_data(box, "check"));
                    gtk_toggle_button_set_active(check, TRUE);
                    break;
                }
            }
        }
        g_strfreev(mOptions);

        // clear all checks...
        for (auto p=efs_options; p->id; p++) {
            auto box = G_OBJECT(g_object_get_data(G_OBJECT(this->dialog()), p->id));
            auto check = GTK_TOGGLE_BUTTON(g_object_get_data(box, "check"));
            gtk_toggle_button_set_active(check, FALSE);
        }
        // set saved checks...
        auto eOptions = g_strsplit(efsOptions, ",", -1);
        g_free(efsOptions);
        for (auto o= eOptions; o && *o; o++){
            if (strchr(*o, '=')){
                auto parts = g_strsplit(*o,"=", 2);
                auto key = g_strconcat(parts[0], "=", NULL);
                auto box = G_OBJECT(g_object_get_data(G_OBJECT(this->dialog()), key));
                auto check = GTK_TOGGLE_BUTTON(g_object_get_data(box, "check"));
                auto entry = GTK_ENTRY(g_object_get_data(box, "entry"));
                gtk_toggle_button_set_active(check, TRUE);
                gtk_entry_set_text(entry, parts[1]);
                g_strfreev(parts);
                g_free(key);
            } else {
                auto box = G_OBJECT(g_object_get_data(G_OBJECT(this->dialog()), *o));
                auto check = GTK_TOGGLE_BUTTON(g_object_get_data(box, "check"));
                gtk_toggle_button_set_active(check, TRUE);
            }
        }

        g_strfreev(eOptions);

        
   
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

    gboolean save(void){
        auto path = gtk_entry_get_text(this->remoteEntry());
        auto mountPoint = gtk_entry_get_text(this->mountPointEntry());
        auto mountOptions = getMountOptions();
        auto efsOptions = getEFSOptions();
        TRACE("[%s]\n", path);
        TRACE(" mountPoint=\"%s\"\n", mountPoint);
        TRACE(" mountOptions=\"%s\"\n", mountOptions);
        TRACE(" efsOptions=\"%s\"\n", efsOptions);

        gchar *file = g_build_filename(EFS_KEY_FILE, NULL);
        GKeyFile *key_file = g_key_file_new ();
        g_key_file_load_from_file (key_file, file, (GKeyFileFlags)(G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS), NULL);

        if (mountPoint) g_key_file_set_value (key_file, path, "mountPoint", mountPoint);
        if (mountOptions) g_key_file_set_value (key_file, path, "mountOptions", mountOptions);
        if (efsOptions) g_key_file_set_value (key_file, path, "efsOptions", efsOptions);
        auto retval = g_key_file_save_to_file (key_file,file,NULL);
        g_free(file);

        g_key_file_free(key_file);
        return retval;
        
    }

    gchar *getMountOptions(void){
        // Mount options
        gchar *retval = NULL;
        gint i=0;
        for (auto p=mount_options; p->id && i+1 < MAX_COMMAND_ARGS; p++,i++) {
            auto box = GTK_BOX(g_object_get_data(G_OBJECT(this->dialog()), p->id));
            if (!box) {
                DBG("getOptions(): cannot find item \"%s\"\n", p->id);
                continue;
            }
            auto check = GTK_TOGGLE_BUTTON(g_object_get_data(G_OBJECT(box), "check")); 
            if (gtk_toggle_button_get_active(check)) {
                TRACE("Option %s --> %s\n", p->id, p->flag); 
                auto g = g_strconcat((retval)?retval:"",(retval)?",":"", p->flag, NULL);
                g_free(retval);
                retval=g;
            }        
        }
        return retval;
    }


    gchar *getEFSOptions(void){
        // EFS options
        gchar *optionsOn = NULL;
        gint i=0;
        for (auto p=efs_options; p->id && i+1 < MAX_COMMAND_ARGS; p++, i++) {
            auto box = GTK_BOX(g_object_get_data(G_OBJECT(this->dialog()), p->id));
            if (!box) {
                DBG("getOptions(): cannot find item \"%s\"\n", p->id);
                continue;
            }
            auto check = GTK_TOGGLE_BUTTON(g_object_get_data(G_OBJECT(box), "check")); 
            auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(box), "entry")); 
            if (gtk_toggle_button_get_active(check)) {
                if (!optionsOn) {
                    optionsOn = g_strdup("");
                } else {
                    auto g = g_strconcat(optionsOn,",",NULL);
                    g_free(optionsOn);
                    optionsOn = g;
                }
                auto g = g_strconcat(optionsOn, p->id, 
                        (entry)?gtk_entry_get_text(entry):"", NULL);
                g_free(optionsOn);
                optionsOn = g;
                TRACE("Option %s --> %s\n", p->id, optionsOn);
            }
            else TRACE("no check:  %s\n", p->id);
        } 
        return optionsOn;
    }
    void mountUrl(View<Type> *view){

        auto path = gtk_entry_get_text(this->remoteEntry());
        auto mountPoint = gtk_entry_get_text(this->mountPointEntry());
        DBG("mountUrl: %s -> %s\n", path, mountPoint);
        const gchar *argv[MAX_COMMAND_ARGS];
        memset((void *)argv, 0, MAX_COMMAND_ARGS*sizeof(const gchar *));

        gint i=0;
        if (geteuid() != 0) {
            argv[i++] = "sudo";
            argv[i++] = "-A";
        }
        
        argv[i++] = "mount";
        // Mount options
        auto mountOptions = getMountOptions();
        if (mountOptions) {
            auto optionsM = g_strsplit(mountOptions, ",", -1);
            for (auto o=optionsM; o && *o; o++){
                argv[i++] = *o;
            }
            g_free(mountOptions);
        }

        argv[i++] = "-t";
        argv[i++] = "ecryptfs";
        
        auto optionsOn = getEFSOptions();
        
        gchar *passphraseFile = NULL;
        gboolean insecurePassphraseFile = FALSE;
        if (optionsOn && strstr(optionsOn, "passphrase_passwd_file=")){
            insecurePassphraseFile = TRUE;
        }

        // Get passphrase option
        if (!insecurePassphraseFile) {
            passphraseFile = get_passfile(path);
            if (!passphraseFile) {
                DBG("No passphrase file...\n");
                g_free(optionsOn);
                return;
            }
            auto g = g_strconcat(optionsOn, ",passphrase_passwd_file=", passphraseFile, NULL);
            g_free(optionsOn);
            optionsOn = g;
        }
        if (optionsOn){
            argv[i++] = "-o";
            argv[i++] = optionsOn;
        }
        argv[i++] = path;
        argv[i++] = mountPoint;

        auto textview = Fm<Type>::getCurrentTextview();
        Print<Type>::showTextSmall(textview);
        gchar *command = g_strdup_printf(_("Mounting %s"), path);
 
        pthread_mutex_lock(&efsMountMutex);
        new (CommandResponse<Type>)(command,"system-run", argv, cleanupGo, (void *)view);


        // cleanup
        memset(optionsOn, 0, strlen(optionsOn));
        g_free(optionsOn);
        new(Thread<Type>)("EFS::mountUrl(): cleanup_passfile", cleanup_passfile, (void *) passphraseFile);

   }

    static gboolean changeEfsItem(void *data){
        auto view = (View<Type> *)data;
        view->reloadModel();        
        return G_SOURCE_REMOVE;
    }
   
    static void 
    cleanupGo(void * data){
        pthread_mutex_unlock(&efsMountMutex);
        // update icon emblem:
        g_timeout_add(1000, changeEfsItem, data);
    }

    static void *
    cleanup_passfile(void * data){
        auto passfile = (gchar *)data;
        if (!passfile) return NULL;
        struct stat st;
        pthread_mutex_lock(&efsMountMutex);

        gint fd = open(passfile, O_RDWR);
        if (fd < 0){
            DBG("Cannot open password file %s to wipeout\n", passfile);
        } else {
            gint i;
            // wipeout
            for (i=0; i<2048; i++){
                const gchar *null="";
                if (write(fd, null, 1) < 0){
                    break;
                }
            }
            close(fd);
            if (unlink(passfile)<0) {
                    DBG("Cannot unlink password file %s\n", passfile);
            }
        }
        memset(passfile, 0, strlen(passfile));
        g_free(passfile);
        pthread_mutex_unlock(&efsMountMutex);
        return NULL;
    }

    static gchar *
    get_passfile(const gchar *passphrase_text){
        gchar *ptext = g_strdup_printf(_("Enter Passphrase for %s"), passphrase_text);
        gchar *passphrase = PasswordResponse<Type>::getPassword (ptext);
        g_free(ptext);

        gint fd = -1;
        gchar *passfile = NULL;
        if (passphrase && strlen(passphrase)){
            time_t seconds;
            time (&seconds);
            gint tried=0;
    retry:
            srand ((unsigned)seconds);
            gint divide = RAND_MAX / 10000;
            
            if((seconds = rand () / divide) > 100000L){
                seconds = 50001;
            }
            passfile = g_strdup_printf("%s/.efs-%ld", g_get_home_dir(), (long)seconds);
            // if the file exists, retry with a different seudo-random number...
            if (g_file_test(passfile, G_FILE_TEST_EXISTS)){
                if (seconds > 0) seconds--;
                else seconds++;
                if (tried++ < 300) {
                    g_free(passfile);
                    goto retry;
                } else {
                    g_error("This is a what some people call \"a chickpea that weighs a pound\"\n");
                }
            }
            TRACE("passfile=%s on try %d\n", passfile, try);

            fd = open (passfile, O_CREAT|O_TRUNC|O_RDWR, 0600);
    //        fd = open (passfile, O_CREAT|O_TRUNC|O_RDWR|O_SYNC|O_DIRECT, 0600);
            if (fd >= 0) {
                if (write(fd, (void *)"passwd=", strlen("passwd=")) < 0){
                    DBG("write %s: %s\n", passfile, strerror(errno));
                }
                if (write(fd, (void *)passphrase, strlen(passphrase)) < 0){
                    DBG("write %s: %s\n", passfile, strerror(errno));
                }
                memset(passphrase, 0, strlen(passphrase));
                close(fd);
            } else {
                DBG("cannot open %s: %s\n", passfile, strerror(errno));
            }

        }
        return passfile;
    }

};
#endif

} // namespace xf
#endif
