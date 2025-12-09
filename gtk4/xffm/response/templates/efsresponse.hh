#ifndef EFSRESPONSE_HH
#define EFSRESPONSE_HH

#include "ecryptfs.i"
template <class Type> class EFS;
namespace xf {
  pthread_mutex_t efsMountMutex=PTHREAD_MUTEX_INITIALIZER;
 
  template <class Type>
  class EfsResponse {
   using subClass_t = EfsResponse<Type>;
   using dialog_t = DialogComplex<subClass_t>;

   GtkBox *mainBox_ = NULL;
   GtkWindow *dialog_ = NULL;
   char *title_;
   const char *iconName_;
   GtkEntry *remoteEntry_ = NULL;
   GtkEntry *mountPointEntry_ = NULL;
   GtkEntry *passEntrys_[2];
   GtkEntry *pathEntrys_[2];
   GtkBox *passphraseBox_ = NULL;
   char *folder_ = NULL;
   GtkTextView *output_;
   GList *children_ = NULL; 
   GtkButton *mountButton_ = NULL;
   GtkButton *cancelButton_ = NULL;
   GtkButton *saveButton_ = NULL;
public:
    GtkWidget *cbox(void){return NULL;}
    
    GList *children(void){return children_;}
    const char *title(void){ return title_;}
    const char *iconName(void){ return "emblem-run";}
    const char *label(void){return "xffm::efs";}
    GtkEntry *remoteEntry(void){return remoteEntry_;}
    GtkEntry *mountPointEntry(void){return mountPointEntry_;}
    char *folder(){return  folder_;}
    void folder(const char *value){folder_ = g_strdup(value);}

    ~EfsResponse (void){
      g_free(folder_);
      g_free(title_);
      TRACE("EFS destructor\n");
    }

    EfsResponse (void){
      title_ = g_strdup_printf("%s ecryptfs", _("New"));
    }

    void push(void *data){
      children_ = g_list_prepend(children_, data);
    }

     static void *asyncYes(void *data){
      auto dialogObject = (dialog_t *)data;
      DBG("%s", "hello world\n");
      return NULL;
    }

    static void *asyncNo(void *data){
      auto dialogObject = (dialog_t *)data;
      DBG("%s", "goodbye world\n");
      return NULL;
    }
#if 0
    // void *asyncCallback(void *data)
    //
    // This will be executed by the subClassObject->subClass(),
    // In other words, by the  DialogComplex<FileResponse<Type> > dialog
    // subClass object.
    //
    void *asyncCallback(void *data){
       /*auto path = (const char *)data;
       reload(path);
       return NULL;*/
       TRACE("asyncCallback(%s)...\n", (const char *)data);
       // FIXME: Final step,
       //        Set the entry text, for this, we
       //        need to know which entry is referred to...
       return (void *) "bar";
    }

    //void *asyncCallbackData(void)
    //
    // Just for completeness for now.
    // 
    void *asyncCallbackData(void){
      TRACE("asyncCallbackData...\n");
      return (void *) "foobar";
    }
#endif
    
///////////////////////////////////////////////////////////////////

    GtkBox *mainBox(void) {
      return mainBox(NULL);
    }
    
    GtkBox *mainBox(const char *folder) {
      DBG("efsresponse.hh: mainBox(%s)\n", folder);
        folder_ = folder? g_strdup(folder) : g_strdup("/");
        //auto dialog = gtk_dialog_new ();
        //gtk_window_set_type_hint(GTK_WINDOW(dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
        mainBox_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        //gtk_widget_set_size_request(GTK_WIDGET(mainBox_), 550, 400);
        gtk_widget_set_vexpand(GTK_WIDGET(mainBox_), false);
        gtk_widget_set_hexpand(GTK_WIDGET(mainBox_), false);

        auto hbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_vexpand(GTK_WIDGET(hbox), false);
        gtk_widget_set_hexpand(GTK_WIDGET(hbox), true);
        gtk_box_append(mainBox_, GTK_WIDGET(hbox));

     /*   auto paintable = Texture<bool>::load("authentication", 24);
        
        auto picture = gtk_picture_new_for_paintable(paintable);
        g_object_unref(paintable);
        gtk_box_append(hbox, picture);*/

        auto text = g_strconcat(_("Options:"), " Ecrypt file system", NULL);        
        auto label = gtk_label_new(text);
        g_free(text);
        gtk_box_append(hbox, GTK_WIDGET(label));

        //auto tbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        //gtk_box_append(mainBox_, GTK_WIDGET(tbox));

        auto notebook = GTK_NOTEBOOK(gtk_notebook_new ());
        gtk_widget_set_vexpand(GTK_WIDGET(notebook), true);
        gtk_widget_set_hexpand(GTK_WIDGET(notebook), true);
        g_object_set_data(G_OBJECT(mainBox_), "notebook", notebook);
        gtk_notebook_popup_enable (notebook);
#if 0
        gtk_notebook_set_scrollable (notebook, TRUE);
#else
        gtk_notebook_set_scrollable (notebook, false);
#endif
        g_object_set (notebook,
                      "enable-popup", FALSE, 
                      "can-focus", TRUE,
                      "scrollable", TRUE, 
                      "show-border", FALSE,
                      "show-tabs", 
                      TRUE, "tab-pos",
                      GTK_POS_TOP, NULL);  

        gtk_box_append(mainBox_, GTK_WIDGET(notebook));

        auto child1 = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        addPage(notebook, GTK_WIDGET(child1), _("Mount"));

        // mount child
        auto encrypted = g_strconcat(_("Mount Point"), " (", _("Encrypted"), "): ",NULL);
        pathEntrys_[0] = remoteEntry_ = FileResponse<Type, subClass_t>::addEntry(child1, "FUSE_REMOTE_PATH", encrypted, this);
        g_free(encrypted);

        auto unencrypted = g_strconcat(_("Mount Point"), " (", _("Unencrypted"), "): ",NULL);
        pathEntrys_[1] = mountPointEntry_ = FileResponse<Type, subClass_t>::addEntry(child1, "FUSE_MOUNT_POINT", unencrypted, this);
//        mountPointEntry_ = addEntry(child1, "entry2", unencrypted);
        g_free(unencrypted);
        
        for (int i=0; i<2; i++){
          auto keyController = gtk_event_controller_key_new();
          gtk_event_controller_set_propagation_phase(keyController, GTK_PHASE_BUBBLE);
          gtk_widget_add_controller(GTK_WIDGET(pathEntrys_[i]), keyController);
          g_signal_connect (G_OBJECT (keyController), "key-released", 
          G_CALLBACK (on_keypress0), (void *)this);
        }

        passphraseBox_ = addEntryPass(child1);
        gtk_widget_set_visible(GTK_WIDGET(passphraseBox_), false);

        auto sw = gtk_scrolled_window_new();
        gtk_box_append(child1, GTK_WIDGET(sw));
        output_ = GTK_TEXT_VIEW(gtk_text_view_new());
        gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(sw), GTK_WIDGET(output_));
        gtk_widget_set_size_request(GTK_WIDGET(sw), -1, 200);

        // Options child
        auto child2 = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        addPage(notebook, GTK_WIDGET(child2), _("Options"));
        auto sw2 = getScrolledWindow(mount_options);
        gtk_box_append(child2, sw2);

        // Advanced child
        auto child3 = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        addPage(notebook, GTK_WIDGET(child3), _("Advanced"));
        auto sw3 = getScrolledWindow(efs_options);
        gtk_box_append(child3, sw3);

     

        auto action_area = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_vexpand(GTK_WIDGET(action_area), false);
        gtk_widget_set_hexpand(GTK_WIDGET(action_area), false);
        gtk_box_append(mainBox_, GTK_WIDGET(action_area));

        cancelButton_ = UtilBasic::mkButton("emblem-redball", _("Cancel"));
        gtk_box_append(action_area,  GTK_WIDGET(cancelButton_));
        gtk_widget_set_vexpand(GTK_WIDGET(cancelButton_), false);

        saveButton_ = UtilBasic::mkButton ("emblem-floppy", _("Save"));
        gtk_box_append(action_area,  GTK_WIDGET(saveButton_));
        gtk_widget_set_vexpand(GTK_WIDGET(saveButton_), false);

        mountButton_ = UtilBasic::mkButton ("emblem-greenball", _("Mount"));
        gtk_box_append (GTK_BOX (action_area), GTK_WIDGET(mountButton_));
        g_signal_connect (G_OBJECT (mountButton_), "clicked", G_CALLBACK (button_mount), this);
        gtk_widget_set_sensitive(GTK_WIDGET(mountButton_), false);
         


        g_signal_connect (G_OBJECT (saveButton_), "clicked", G_CALLBACK (button_save), this);
        g_signal_connect (G_OBJECT (cancelButton_), "clicked", G_CALLBACK (button_cancel), this);

         return mainBox_;
    }
  
    GtkBox *addEntryPass(GtkBox *child){
      TRACE("***subClassObject-Folder=%s\n", folder());
        auto vbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        gtk_widget_set_vexpand(GTK_WIDGET(vbox), false);
        gtk_widget_set_hexpand(GTK_WIDGET(vbox), true);

        GtkBox *hbox[2];
        GtkWidget *label[2];
        GtkImage *red[2];
        GtkImage *green[2];
        const char *text[]={_("Passphrase"), _("Confirm"), NULL};
        for (int i=0; i<2; i++){
          hbox[i] = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
          gtk_widget_set_vexpand(GTK_WIDGET(hbox[i]), false);
          gtk_widget_set_hexpand(GTK_WIDGET(hbox[i]), true);
          label[i] = gtk_label_new(text[i]);
          gtk_widget_set_hexpand(GTK_WIDGET(label[i]), false);
          passEntrys_[i] = GTK_ENTRY(gtk_entry_new());
          gtk_widget_set_hexpand(GTK_WIDGET(passEntrys_[i]), true);

          red[i] = Texture<bool>::getImage(EMBLEM_RED_BALL, 16);
          green[i] = Texture<bool>::getImage(EMBLEM_GREEN_BALL, 16);       
          g_object_set_data(G_OBJECT(passEntrys_[i]), "red", red[i]);
          g_object_set_data(G_OBJECT(passEntrys_[i]), "green", green[i]);
          gtk_box_append(hbox[i], label[i]);
          gtk_box_append(hbox[i], GTK_WIDGET(passEntrys_[i]));
          gtk_box_append(hbox[i], GTK_WIDGET(red[i]));
          gtk_box_append(hbox[i], GTK_WIDGET(green[i]));
          gtk_box_append(vbox, GTK_WIDGET(hbox[i]));
          gtk_widget_set_visible(GTK_WIDGET(red[i]), true);
          gtk_widget_set_visible(GTK_WIDGET(green[i]), false);
          gtk_entry_set_visibility (GTK_ENTRY(passEntrys_[i]), false);
          gtk_box_append(vbox, GTK_WIDGET(hbox[i]));

          auto keyController = gtk_event_controller_key_new();
          gtk_event_controller_set_propagation_phase(keyController, GTK_PHASE_BUBBLE);
          gtk_widget_add_controller(GTK_WIDGET(passEntrys_[i]), keyController);
          g_signal_connect (G_OBJECT (keyController), "key-released", 
          G_CALLBACK (this->on_keypress), (void *)this);
        }
        gtk_box_append(child, GTK_WIDGET(vbox));
        return vbox;
    }
    
    static gboolean
    on_keypress (GtkEventControllerKey* self,
          guint keyval,
          guint keycode,
          GdkModifierType state,
          gpointer data){
      auto object = (EfsResponse *)data;
      object->checkPass();
      return FALSE;
    }
    
    static gboolean
    on_keypress0 (GtkEventControllerKey* self,
          guint keyval,
          guint keycode,
          GdkModifierType state,
          gpointer data){
      auto object = (EfsResponse *)data;
      object->checkPaths();
      return FALSE;
    }

    void checkPass(void){
      bool result = false;
      const char *text[2];
      for (int i=0; i<2; i++) {
        auto buffer = gtk_entry_get_buffer(passEntrys_[i]);
        text[i] = gtk_entry_buffer_get_text(buffer);
      }
      if (strlen(text[0]) && strcmp(text[0], text[1]) == 0) result = true;
      if (result){
        gtk_widget_set_sensitive(GTK_WIDGET(mountButton_), true);
        for (int i=0; i<2; i++){
          gtk_widget_set_visible(GTK_WIDGET(g_object_get_data(G_OBJECT(passEntrys_[i]),"red")), false);
          gtk_widget_set_visible(GTK_WIDGET(g_object_get_data(G_OBJECT(passEntrys_[i]),"green")), true);
        }

      } else {
        gtk_widget_set_sensitive(GTK_WIDGET(mountButton_), false);
        for (int i=0; i<2; i++){
          gtk_widget_set_visible(GTK_WIDGET(g_object_get_data(G_OBJECT(passEntrys_[i]),"red")), true);
          gtk_widget_set_visible(GTK_WIDGET(g_object_get_data(G_OBJECT(passEntrys_[i]),"green")), false);
        }
      }
    }

    void checkPaths(void){
      bool result = false;
      const char *text[2];
      for (int i=0; i<2; i++) {
        auto buffer = gtk_entry_get_buffer(pathEntrys_[i]);
        text[i] = gtk_entry_buffer_get_text(buffer);
      }
      if (g_file_test(text[0], G_FILE_TEST_IS_DIR) && 
          g_file_test(text[1], G_FILE_TEST_IS_DIR)) result = true;
      if (result){
        gtk_widget_set_visible(GTK_WIDGET(passphraseBox_), true);
      } else {
        gtk_widget_set_visible(GTK_WIDGET(passphraseBox_), false);
      }
    }

    void setup(const char *path, 
               const char *mountPoint, 
               const char *mountOptions, 
               const char *efsOptions)
    {
      TRACE("efsresponse.hh: setup() foo\n");
      auto buffer = gtk_entry_get_buffer(remoteEntry_);
      gtk_entry_buffer_set_text(buffer, path, -1);
      buffer = gtk_entry_get_buffer(mountPointEntry_);
      gtk_entry_buffer_set_text(buffer, mountPoint, -1);

      // Mount options. Turn all off.
      for (auto p=mount_options; p && p->flag != NULL; p++){
        auto hbox = g_object_get_data(G_OBJECT(mainBox_), p->id);
        auto check = GTK_CHECK_BUTTON(g_object_get_data(G_OBJECT(hbox), "check"));
        if (check) gtk_check_button_set_active(check, false);
      }
      // Now turn on necessary options.
      char **v=NULL;
      if (mountOptions){
        if (strchr(mountOptions, ',')) {
          v = g_strsplit(mountOptions, ",", -1);
        } else if (strlen(mountOptions)) {
          v = (char **)calloc(2, sizeof(char *));
          v[0] = g_strdup(mountOptions);
        }
      }
      for (auto p=v; p && *p; p++){
        auto hbox = g_object_get_data(G_OBJECT(mainBox_), *p);
        auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(hbox), "entry"));
        auto check = GTK_CHECK_BUTTON(g_object_get_data(G_OBJECT(hbox), "check"));
        if (check) gtk_check_button_set_active(check, true);
        TRACE("efsresponse.hh: hbox=%p, option %s  entry --> %p, check --> %p\n",
            hbox, *p, entry, check);
      }
      if (v) g_strfreev(v);
      // Mount options done.

      // Efs options. Turn all off.
      for (auto p=efs_options; p && p->id != NULL; p++){
        auto hbox = g_object_get_data(G_OBJECT(mainBox_), p->id);
        auto check = GTK_CHECK_BUTTON(g_object_get_data(G_OBJECT(hbox), "check"));
        if (check) gtk_check_button_set_active(check, false);
      }
      // Now turn on necessary options.
      v=NULL;
      if (efsOptions){
        if (strchr(efsOptions, ',')) {
          v = g_strsplit(efsOptions, ",", -1);
        } else if (strlen(efsOptions)) {
          v = (char **)calloc(2, sizeof(char *));
          v[0] = g_strdup(efsOptions);
        }
      }
      DBG("now at options '%s'\n", efsOptions);
      for (auto p=v; p && *p; p++){
        char **w = NULL;
        char *key = NULL;
        if (strchr(*p, '=')){
          w = g_strsplit(*p, "=", -1);
          key = g_strconcat(w[0], "=", NULL);
        }
        auto hbox =  g_object_get_data(G_OBJECT(mainBox_), key? key : *p);
        DBG("p=%s hbox=%p\n", w? w[0] : *p, hbox); 
        auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(hbox), "entry"));
        auto check = GTK_CHECK_BUTTON(g_object_get_data(G_OBJECT(hbox), "check"));
          DBG("efsresponse.hh: hbox=%p, option %s  entry --> %p, check --> %p\n",
            hbox, *p, entry, check);
        if (check) {
          gtk_check_button_set_active(check, true);
          if (w){
            auto buffer = gtk_entry_get_buffer(entry);
            gtk_entry_buffer_set_text(buffer, w[1], -1);
          }
        }
        if (w) g_strfreev(w);
        if (key) g_free(key);
      }
      if (v) g_strfreev(v);
      gtk_widget_set_sensitive(GTK_WIDGET(mountButton_), false);
      gtk_widget_set_visible(GTK_WIDGET(passphraseBox_), true);
      
      return;
    }

    // void dialog(GtkWindow *value)
    // Set a pointer to the GtkWindow in the FileResponse
    // object so that it can be referred to in the
    // async main context thread callbacks.
    // 
    void dialog(GtkWindow *value){
      dialog_ = value;
    }
    GtkWindow *dialog(void){return dialog_;}
    
    static bool isEfsMount(const char *path){
      bool retval = false;
      auto key_file = getKeyFile();
      if (g_key_file_has_key(key_file, path, "efsOptions", NULL)) retval = true;
      return retval;
    }

    static GKeyFile *getKeyFile(void){
        gchar *file = efsKeyFile();
        GKeyFile *key_file = g_key_file_new ();
        g_key_file_load_from_file (key_file, file, (GKeyFileFlags)(G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS), NULL);
        g_free(file);
        return key_file;
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
    
    GtkWidget *
    getScrolledWindow(group_options_t *options_p) {
        GtkWidget *mainBox = GTK_WIDGET(mainBox_);
        guint64 ui=0x01;
        auto  vbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        auto sw = GTK_SCROLLED_WINDOW (gtk_scrolled_window_new ());
        gtk_widget_set_vexpand(GTK_WIDGET(sw), true);
        gtk_widget_set_hexpand(GTK_WIDGET(sw), true);
        //gtk_scrolled_window_set_shadow_type (sw, GTK_SHADOW_ETCHED_IN);
        gtk_scrolled_window_set_policy (sw, 
                GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_child(sw, GTK_WIDGET(vbox));

        gint i=0;
        for (; options_p && options_p->id; options_p++){
          auto vbox2 = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
          auto hbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
          auto hbox2 = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
          if (!options_p->id){
              ERROR_("optionsBox(): options id cannot be null.\n");
              continue;
          }
          if (g_object_get_data(G_OBJECT(mainBox), options_p->id)) {
              ERROR_("optionsBox(): Duplicate entry: %s\n", options_p->id);
              continue;
          }
          g_object_set_data(G_OBJECT(mainBox),options_p->id, hbox);
          if (strlen(options_p->flag)){
              g_object_set_data(G_OBJECT(mainBox),options_p->flag, hbox);
              DBG("Set hbox %s --> %p\n", options_p->flag, hbox);
          }
        
          const gchar *labelColor = (options_p->sensitive ==0)?"gray":"red";

          auto checkMarkup = g_strdup_printf("<span color=\"%s\">%s</span>", 
                  (options_p->entry)?labelColor:"blue",
                  options_p->id);

          auto label = gtk_label_new("");
          gtk_label_set_markup(GTK_LABEL(label), checkMarkup);
          g_free(checkMarkup);

          auto check = gtk_check_button_new_with_label(options_p->flag);
          gtk_check_button_set_active(GTK_CHECK_BUTTON(check), 
                  options_p->sensitive > 1 || options_p->sensitive < 0);

          g_object_set_data(G_OBJECT(hbox),"check", check);
          g_object_set_data(G_OBJECT(hbox),"label", label);

          gtk_box_append(vbox2,  GTK_WIDGET(hbox));
          gtk_box_append(vbox2, GTK_WIDGET(hbox2));
          gtk_box_append(hbox, check);
          gtk_box_append(hbox, label);
          gtk_widget_set_sensitive(GTK_WIDGET(label), (options_p->sensitive > 0) && options_p->sensitive != 3);

          if (options_p->entry)  {
            auto entry = GTK_ENTRY(gtk_entry_new());
            g_object_set_data(G_OBJECT(hbox),"entry", entry);
            auto buffer = gtk_entry_get_buffer(entry);
            gtk_entry_buffer_set_text(buffer, options_p->entry, -1);
            gtk_box_append(hbox,  GTK_WIDGET(entry));
            gtk_widget_set_sensitive(GTK_WIDGET(entry), (options_p->sensitive > 0));
          }
          if (options_p->text || options_p->tip) {
                //auto text = g_strdup_printf("<span color=\"gray\">(<span color=\"blue\">%s</span> %s)</span>", 
                     //   options_p->flag, (options_p->tip)?options_p->tip:options_p->text);
                auto text = g_strdup_printf("%s %s", 
                        options_p->flag, (options_p->tip)?options_p->tip:options_p->text);

                auto lines = g_strsplit(text, "\n", -1);
                GtkLabel *label;
                if (!strstr(text, "\n")) {
                  label = GTK_LABEL(gtk_label_new(""));
                  auto line = g_strdup_printf("\t<span color=\"#445566\">%s</span>", text);
                  gtk_label_set_markup(label, line);
                  g_free(line);
                  gtk_widget_set_halign(GTK_WIDGET(label), GTK_ALIGN_START);
                  gtk_box_append(hbox2, GTK_WIDGET(label));
                } else {
                  auto lines = g_strsplit(text, "\n", -1);
                  auto vbox3 = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
                  gtk_widget_set_halign(GTK_WIDGET(vbox3), GTK_ALIGN_START);
                  for (auto p=lines; p && *p; p++){
                    label = GTK_LABEL(gtk_label_new(""));
                    gtk_widget_set_halign(GTK_WIDGET(label), GTK_ALIGN_START);
                    auto line = g_strdup_printf("\t<span color=\"#445566\">%s</span>", *p);
                    gtk_label_set_markup(label, line);
                    g_free(line);
                    gtk_box_append(vbox3, GTK_WIDGET(label));
                  }
                  gtk_box_append(hbox2, GTK_WIDGET(vbox3));

                  g_strfreev(lines);
                }
                //auto labelview = GTK_LABEL(gtk_label_new(""));
                //auto labelview = GTK_LABEL(gtk_label_new(text));
                //gtk_label_set_markup(labelview, text);
                //auto labelview = mkTextView(text);
                //gtk_box_append(hbox2, GTK_WIDGET(labelview));
                g_free(text);
          }

          gtk_widget_set_sensitive(GTK_WIDGET(check), (options_p->sensitive > 0 && options_p->sensitive != 3));
          gtk_widget_set_sensitive(GTK_WIDGET(check), (options_p->sensitive > 0 && options_p->sensitive != 3));
        

          i++;
        
          gtk_box_append(vbox, GTK_WIDGET(vbox2));
        }
        gtk_widget_set_size_request(GTK_WIDGET(sw), 400, -1);
        return GTK_WIDGET(sw);
    }

      void addPage(GtkNotebook *notebook, GtkWidget *child, const char *text){
        gtk_widget_set_vexpand(GTK_WIDGET(child), true);
        gtk_widget_set_hexpand(GTK_WIDGET(child), true);

        gtk_notebook_append_page (notebook, GTK_WIDGET(child), 
            gtk_label_new(text));
        gtk_notebook_set_tab_reorderable (notebook, GTK_WIDGET(child), TRUE);
      }

      
      static void fileOK( GObject* source_object, GAsyncResult* result,  gpointer data ){
        TRACE("fileOK: \n");
        GtkFileDialog *dialog = GTK_FILE_DIALOG (source_object);
        GFile *file;
        GError *error = NULL;

        file = gtk_file_dialog_select_folder_finish (dialog, result, &error);
        if (!file)
          {
            g_print ("Error: %s %d %s\n", g_quark_to_string (error->domain), error->code, error->message);
            g_error_free (error);
          }
        else
          {
            g_print ("%s\n", g_file_peek_path (file));
            g_object_unref (file);
          }

        //done = TRUE;
      }

    char *getMountOptions(void){
        // Mount options
        gchar *retval = NULL;
        gint i=0;
        for (auto p=mount_options; p->id && i+1 < MAX_COMMAND_ARGS; p++,i++) {
            auto box = GTK_BOX(g_object_get_data(G_OBJECT(this->mainBox_), p->id));
            if (!box) {
                ERROR_("getOptions(): cannot find item \"%s\"\n", p->id);
                continue;
            }
            auto check = GTK_CHECK_BUTTON(g_object_get_data(G_OBJECT(box), "check")); 
            if (gtk_check_button_get_active(check)) {
                TRACE("Option %s --> %s\n", p->id, p->flag); 
                auto g = g_strconcat((retval)?retval:"",(retval)?",":"", p->flag, NULL);
                g_free(retval);
                retval=g;
            }        
        }
        return retval;
    }

    char *getEFSOptions(void){
        // EFS options
        char *optionsOn = NULL;
        gint i=0;
        for (auto p=efs_options; p->id && i+1 < MAX_COMMAND_ARGS; p++, i++) {
            auto box = GTK_BOX(g_object_get_data(G_OBJECT(this->mainBox_), p->id));
            if (!box) {
                ERROR_("getOptions(): cannot find item \"%s\"\n", p->id);
                continue;
            }
            auto check = GTK_CHECK_BUTTON(g_object_get_data(G_OBJECT(box), "check")); 
            auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(box), "entry")); 
            if (gtk_check_button_get_active(check)) {
                if (!optionsOn) {
                    optionsOn = g_strdup("");
                } else {
                    auto g = g_strconcat(optionsOn,",",NULL);
                    g_free(optionsOn);
                    optionsOn = g;
                }
                const char *g = "";
                if (entry){
                  auto buffer = gtk_entry_get_buffer(entry);
                  g = gtk_entry_buffer_get_text(buffer);
                }
                auto gg = g_strconcat(optionsOn, p->id, g, NULL);
                g_free(optionsOn);
                optionsOn = gg;
                TRACE("Option %s --> %s\n", p->id, optionsOn);
            }
            else TRACE("no check:  %s\n", p->id);
        } 
        return optionsOn;
    }

    gboolean save(void){
        auto buffer = gtk_entry_get_buffer(this->remoteEntry());
        auto path = gtk_entry_buffer_get_text(buffer);
        buffer = gtk_entry_get_buffer(this->mountPointEntry());
        auto mountPoint = gtk_entry_buffer_get_text(buffer);
        auto mountOptions = getMountOptions();
        auto efsOptions = getEFSOptions();
        bool ok = true;
        if (!g_file_test(path, G_FILE_TEST_IS_DIR)){
          Print::printWarning(output_,
              g_strconcat(_("Mount Point"), " (", _("Encrypted"), ") ", " \"",path, "\" : ",
                _("Folder does not exist"), "\n", NULL));
          ok = false;
        }
        if (!g_file_test(mountPoint, G_FILE_TEST_IS_DIR)){
          Print::printWarning(output_,
              g_strconcat(_("Mount Point"), " (", _("Unencrypted"), ") ", " \"",mountPoint, "\" : ",
                _("Folder does not exist"), "\n", NULL));
          ok = false;
        }
        TRACE("path=\"%s\"\n", path);
        TRACE(" mountPoint=\"%s\"\n", mountPoint);
        TRACE(" mountOptions=\"%s\"\n", mountOptions);
        TRACE(" efsOptions=\"%s\"\n", efsOptions);

        if (ok) {
          auto key_file = getKeyFile();
          
          g_key_file_set_value (key_file, path, "mountPoint", mountPoint);
          g_key_file_set_value (key_file, path, "mountOptions", mountOptions);
          g_key_file_set_value (key_file, path, "efsOptions", efsOptions);
          auto file = efsKeyFile();
          auto retval = g_key_file_save_to_file (key_file,file,NULL);
          g_key_file_free(key_file);
          if (!retval){
            ERROR_("EfsResponse:: save(): Error writing %s\n", file);
          }
          g_free(file);
        }
        
        return ok;
        
    }
    static char *efsKeyFile(void){
      return  g_strconcat(g_get_user_config_dir(),
          G_DIR_SEPARATOR_S, "xffm4",G_DIR_SEPARATOR_S, "efs.ini", NULL);
    }


    static void
    button_save (GtkButton * button, gpointer data) {
      auto subClass = (subClass_t *)data;
      if (subClass->save()){
        g_object_set_data(G_OBJECT(subClass->dialog()), "response", GINT_TO_POINTER(1));
      }
    }

    static void
    button_cancel (GtkButton * button, gpointer data) {
      auto subClass = (subClass_t *)data;
      g_object_set_data(G_OBJECT(subClass->dialog()), "response", GINT_TO_POINTER(-1));
    }

    static void
    button_mount (GtkButton * button, gpointer data) {
      auto subClass = (subClass_t *)data;
      g_object_set_data(G_OBJECT(subClass->dialog()), "response", GINT_TO_POINTER(2));
      DBG("button_mount...\n");
      subClass->mount();
      
    }

    void mount(void){
       // mount
      DBG("mount...\n");
      auto efsmount = g_find_program_in_path("mount.ecryptfs");
      if (not efsmount){
          ERROR_(g_strdup_printf("%s: mount.ecryptfs\n", strerror(ENOENT)));
          g_free(efsmount);
          return;
      }
      // This here will ask for passphrase before mounting:
      mountUrl();
    }

    void mountUrl(void){
      DBG("mountUrl...\n");
      
      auto buffer = gtk_entry_get_buffer(this->remoteEntry());
      auto path = gtk_entry_buffer_get_text(buffer);

      buffer = gtk_entry_get_buffer(this->mountPointEntry());
      auto mountPoint = gtk_entry_buffer_get_text(buffer);

      buffer = gtk_entry_get_buffer(passEntrys_[0]);
      auto passphase = g_strdup(gtk_entry_buffer_get_text(buffer));

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
      auto buffer2 = gtk_entry_get_buffer(passEntrys_[0]);
      auto pass = gtk_entry_buffer_get_text(buffer2);
      auto keyOption = g_strconcat(optionsOn,",key=passphrase:passphrase_passwd=",pass, NULL);
      

      argv[i++] = "-o";
      argv[i++] = keyOption;
      

      argv[i++] = path;
      argv[i++] = mountPoint;
      argv[i] = NULL;   

      Print::print(output_, g_strdup_printf(_("Mounting %s"), path));

      for (auto p=argv; p && *p; p++)fprintf(stderr, "%s ", *p); fprintf(stderr, "\n");

// FIXME: run a "sudo -A modprobe ecryptfs" first and test for confirm ok, or do not activate
//                                          the mount button until both greens are set 
//                                          with no empty string passphrase.
/* 
      pthread_mutex_lock(&efsMountMutex);
      new Thread("EFS::mountUrl(): cleanup_passfile", cleanup_passfile, (void *) passphraseFile);

      auto pid = fork();
      if (pid){
        int status;
        waitpid(pid, &status, 0);
        pthread_mutex_unlock(&efsMountMutex);
      } else {
        execvp(argv[0], (char* const*)argv);
      }
      //new (CommandResponse<Type>)(command,"system-run", argv, cleanupGo, (void *)view);
      //Run<bool>::thread_run(output_, (const char **)argv, true);
*/

      // cleanup
      g_free(optionsOn);
      memset(keyOption, 0, strlen(keyOption));
      g_free(keyOption);
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
    get_passfile(char *passphrase){
      gchar *passfile = NULL;

      gint fd = -1;
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
              g_error("This is a \"a chickpea that weighs a pound\"\n");
            }
          }
         // TRACE("passfile=%s on try %d\n", passfile, try);

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


}
#endif

