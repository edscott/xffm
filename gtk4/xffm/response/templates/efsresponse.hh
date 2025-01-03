#ifndef EFSRESPONSE_HH
#define EFSRESPONSE_HH

#include "ecryptfs.i"

namespace xf {
  template <class Type>
  class EfsResponse {
   using dialog_t = DialogComplex<EfsResponse<Type> >;
   using subClass_t = EfsResponse<Type>;
   
   GtkBox *mainBox_ = NULL;
   GtkWindow *dialog_ = NULL;
   char *title_;
   const char *iconName_;
   GtkEntry *remoteEntry_ = NULL;
   GtkEntry *mountPointEntry_ = NULL;
   char *folder_ = NULL;
   GtkTextView *output_;
   GList *children_ = NULL; 
public:
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
      cleanup();
        //if (bashCompletionStore_) gtk_list_store_clear(bashCompletionStore_);
        //gtk_window_destroy(response_);
    }

    EfsResponse (void){
      title_ = g_strdup_printf("%s ecryptfs", _("New"));
      /*pthread_t thread;
      int retval = pthread_create(&thread, NULL, response_f, (void *)this);
      pthread_detach(thread);*/
    }

    void push(void *data){
      children_ = g_list_prepend(children_, data);
    }

    void cleanup(void){// FIXME: this is subject to race 
                       //GTK_IS_WINDOW(l->data) may cause crash
                       //probably should do with controls...
      for (auto l=children_; l && l->data; l=l->next){
          DBG("destroy dialog GTK_IS_WINDOW %p\n", l->data);
        if (GTK_IS_WINDOW(l->data)) {
          DBG("destroy dialog %p\n", l->data);
          gtk_widget_set_visible(GTK_WIDGET(l->data), false);
          // set to self destruct with cancelation:
          g_object_set_data(G_OBJECT(l->data), "response", GINT_TO_POINTER(-1)); 
          //gtk_window_destroy(GTK_WINDOW(l->data)); // destroy here causes gtk to crash
        } else {
          DBG("%p is not a dialog\n", l->data);
        }
      }      
      DBG("cleanup done\n");
      g_list_free(children_);
      children_ = NULL;
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
       DBG("asyncCallback(%s)...\n", (const char *)data);
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
      DBG("asyncCallbackData...\n");
      return (void *) "foobar";
    }
#endif
    
///////////////////////////////////////////////////////////////////

    GtkBox *mainBox(void) {
      return mainBox(NULL);
    }

    GtkBox *mainBox(const char *folder) {
        folder_ = folder? g_strdup(folder) : g_strdup("/");
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
        remoteEntry_ = FileResponse<Type, subClass_t>::addEntry(child1, "entry1", encrypted, this);
        g_free(encrypted);
        //gtk_widget_set_sensitive(GTK_WIDGET(remoteEntry_), true); // FIXME: put to false 

        auto unencrypted = g_strconcat(_("Mount Point"), " (", _("Unencrypted"), "): ",NULL);
        mountPointEntry_ = FileResponse<Type, subClass_t>::addEntry(child1, "entry2", unencrypted, this);
//        mountPointEntry_ = addEntry(child1, "entry2", unencrypted);
        g_free(unencrypted);
        //gtk_widget_set_sensitive(GTK_WIDGET(mountPointEntry_), true); // FIXME: put to false 

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


    // void dialog(GtkWindow *value)
    // Set a pointer to the GtkWindow in the FileResponse
    // object so that it can be referred to in the
    // async main context thread callbacks.
    // 
    void dialog(GtkWindow *value){
      dialog_ = value;
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
                DBG("getOptions(): cannot find item \"%s\"\n", p->id);
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
                DBG("getOptions(): cannot find item \"%s\"\n", p->id);
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
        DBG("path=\"%s\"\n", path);
        DBG(" mountPoint=\"%s\"\n", mountPoint);
        DBG(" mountOptions=\"%s\"\n", mountOptions);
        DBG(" efsOptions=\"%s\"\n", efsOptions);

        if (ok) {
          gchar *file = g_build_filename(EFS_KEY_FILE, NULL);
          GKeyFile *key_file = g_key_file_new ();
          g_key_file_load_from_file (key_file, file, (GKeyFileFlags)(G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS), NULL);

          g_key_file_set_value (key_file, path, "mountPoint", mountPoint);
          g_key_file_set_value (key_file, path, "mountOptions", mountOptions);
          g_key_file_set_value (key_file, path, "efsOptions", efsOptions);
          auto retval = g_key_file_save_to_file (key_file,file,NULL);
          g_key_file_free(key_file);
          if (!retval){
            DBG("EfsResponse:: save(): Error writing %s\n", file);
          }
          g_free(file);
        }
        
        return ok;
        
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

  };


}
#endif

