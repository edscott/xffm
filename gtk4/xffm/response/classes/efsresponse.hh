#ifndef EFSRESPONSE_HH
#define EFSRESPONSE_HH

#include "ecryptfs.i"

namespace xf {
  class EfsResponse {
   GtkBox *mainBox_ = NULL;
   GtkWindow *dialog_ = NULL;
   char *title_;
   const char *iconName_;
public:
    const char *title(void){ return title_;}
    const char *iconName(void){ return "emblem-run";}
    const char *label(void){return "xffm::efs";}

    ~EfsResponse (void){
      g_free(title_);
        //if (bashCompletionStore_) gtk_list_store_clear(bashCompletionStore_);
        //gtk_window_destroy(response_);
    }

    EfsResponse (void){
      title_ = g_strdup_printf("%s ecryptfs", _("New"));
      /*pthread_t thread;
      int retval = pthread_create(&thread, NULL, response_f, (void *)this);
      pthread_detach(thread);*/
    }

     static void *asyncYes(void *data){
      auto dialogObject = (DialogComplex<EfsResponse> *)data;
      DBG("%s", "hello world\n");
      return NULL;
    }

    static void *asyncNo(void *data){
      auto dialogObject = (DialogComplex<EfsResponse> *)data;
      DBG("%s", "goodbye world\n");
      return NULL;
    }


    GtkBox *mainBox(void) {
        //auto dialog = gtk_dialog_new ();
        //gtk_window_set_type_hint(GTK_WINDOW(dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
        mainBox_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        gtk_widget_set_size_request(GTK_WIDGET(mainBox_), 550, 400);
        gtk_widget_set_vexpand(GTK_WIDGET(mainBox_), false);
        gtk_widget_set_hexpand(GTK_WIDGET(mainBox_), false);

        auto hbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_vexpand(GTK_WIDGET(hbox), false);
        gtk_widget_set_hexpand(GTK_WIDGET(hbox), true);
        gtk_box_append(mainBox_, GTK_WIDGET(hbox));

     /*   auto paintable = Texture<bool>::load("authentication", 24);
        
        auto image = gtk_image_new_from_paintable(paintable);
        g_object_unref(paintable);
        gtk_box_append(hbox, image);*/

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
        gtk_notebook_set_scrollable (notebook, TRUE);
        g_object_set (notebook,
                      "enable-popup", TRUE, 
                      "can-focus", FALSE,
                      "scrollable", TRUE, 
                      "show-border", FALSE,
                      "show-tabs", 
                      TRUE, "tab-pos",
                      GTK_POS_TOP, NULL);  

        gtk_box_append(mainBox_, GTK_WIDGET(notebook));

        auto child1 = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        addPage(notebook, GTK_WIDGET(child1), _("Mount"));

        // mount child
        addEntry(child1, "entry1", EFS_REMOTE_PATH, ": ");
        addEntry(child1, "entry2", FUSE_MOUNT_POINT, " ");

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

        auto cancelButton = Basic::mkButton("emblem-redball", _("Cancel"));
        gtk_box_append(action_area,  GTK_WIDGET(cancelButton));
        gtk_widget_set_vexpand(GTK_WIDGET(cancelButton), false);

        auto saveButton = Basic::mkButton ("emblem-floppy", _("Save"));
        gtk_box_append(action_area,  GTK_WIDGET(saveButton));
        gtk_widget_set_vexpand(GTK_WIDGET(saveButton), false);

        // FIXME: this no longer here
        /*mountButton_ = Gtk<Type>::dialog_button ("greenball", _("Mount"));
        compat<bool>::boxPackStart (GTK_BOX (action_area), GTK_WIDGET(mountButton_), FALSE, FALSE, 0);
        g_signal_connect (G_OBJECT (mountButton_), "clicked", G_CALLBACK (button_mount), this);
         */


        g_signal_connect (G_OBJECT (saveButton), "clicked", G_CALLBACK (button_save), this);
        g_signal_connect (G_OBJECT (cancelButton), "clicked", G_CALLBACK (button_cancel), this);

        // FIXME: 
/*
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

        this->getScrolledWindow(mount_options, _("Options"), 6 );
        this->getScrolledWindow(efs_options, _("Advanced"), 12);

        if (path) setOptions(path);
 */       
        /*
        g_signal_connect (G_OBJECT (dialog), "delete-event", G_CALLBACK (response_delete), this);
        gtk_window_set_resizable (GTK_WINDOW(dialog), TRUE);

        response_ = GTK_RESPONSE_CANCEL;

        MainDialog = GTK_WINDOW(dialog);
        DBG("efs main dialog = %p.\n", MainDialog);
        */
        return mainBox_;
    }

    void setSubClassDialog(GtkWindow *dialog){
      dialog_ = dialog;
    }

    GtkWindow *dialog(void){return dialog_;}

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
              DBG("optionsBox(): options id cannot be null.\n");
              continue;
          }
          if (g_object_get_data(G_OBJECT(mainBox), options_p->id)) {
              DBG("optionsBox(): Duplicate entry: %s\n", options_p->id);
              continue;
          }
          g_object_set_data(G_OBJECT(mainBox),options_p->id, hbox);
        
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

       void addEntry(GtkBox *child, const char *id, const char *text, const char *semicolon){
          auto hbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
          gtk_widget_set_vexpand(GTK_WIDGET(hbox), false);
          gtk_widget_set_hexpand(GTK_WIDGET(hbox), true);
          auto label = gtk_label_new(text);
          gtk_widget_set_hexpand(GTK_WIDGET(label), false);
          auto semicolonL = gtk_label_new(semicolon);
          gtk_widget_set_hexpand(GTK_WIDGET(semicolonL), false);
          auto entry = gtk_entry_new();
          gtk_widget_set_hexpand(GTK_WIDGET(entry), true);
          g_object_set_data(G_OBJECT(child), id, entry);
          auto button = Basic::mkButton("document-open", NULL);
          g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(getDirectory), child);

          gtk_box_append(hbox, label);
          gtk_box_append(hbox, semicolonL);
          gtk_box_append(hbox, entry);
          gtk_box_append(hbox, GTK_WIDGET(button));
          gtk_box_append(child, GTK_WIDGET(hbox));
        }

      static void fileOK( GObject* source_object, GAsyncResult* result,  gpointer data ){
        DBG("fileOK: \n");
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

    static void getDirectory(GtkButton *button, void *data){
      DBG("getDirectory\n");
      auto fileDialog = gtk_file_dialog_new();
      gtk_file_dialog_set_title (fileDialog, "foo bar");
      gtk_file_dialog_set_modal (fileDialog, true);
      GFile *file = g_file_new_for_path ("/tmp");
      gtk_file_dialog_set_initial_folder (fileDialog, file);
      g_object_unref (file);
   
      gtk_file_dialog_set_accept_label (fileDialog, _("Accept"));

      gtk_file_dialog_select_folder (fileDialog, GTK_WINDOW(MainWidget), NULL, fileOK, data);

    }

    static void
    button_save (GtkButton * button, gpointer data) {
      auto subClass = (EfsResponse *)data;
      g_object_set_data(G_OBJECT(subClass->dialog()), "response", GINT_TO_POINTER(1));
    }

    static void
    button_cancel (GtkButton * button, gpointer data) {
      auto subClass = (EfsResponse *)data;
      g_object_set_data(G_OBJECT(subClass->dialog()), "response", GINT_TO_POINTER(-1));
    }

  };

  class EFS {
    static char *efsKeyFile(void){
      return  g_strconcat(g_get_user_config_dir(),G_DIR_SEPARATOR_S, "xffm+",G_DIR_SEPARATOR_S, "efs.ini", NULL);}

    public:

    static gchar **
    getSavedItems(void){
        gchar *file = g_build_filename(efsKeyFile(), NULL);
        GKeyFile *key_file = g_key_file_new ();
        g_key_file_load_from_file (key_file, file, (GKeyFileFlags)(G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS), NULL);
        g_free(file);
        auto retval = g_key_file_get_groups (key_file, NULL);
        g_key_file_free(key_file);
        return retval;
    }

    static void newEfs(void){
      auto dialogObject = new DialogComplex<EfsResponse>;
      dialogObject->setParent(GTK_WINDOW(MainWidget));
      auto dialog = dialogObject->dialog();
      gtk_window_set_decorated(dialog, true);
      dialogObject->setSubClassDialog();

      gtk_widget_realize(GTK_WIDGET(dialog));
      Basic::setAsDialog(GTK_WIDGET(dialog), "dialog", "Dialog");
      gtk_window_present(dialog);

      dialogObject->run();
      

    }



  };


}
#endif

