#ifndef UNTARRESPONSE_HH
#define UNTARRESPONSE_HH

namespace xf {
template <class Type>
class untarResponse {
   using subClass_t = untarResponse;
   using dialog_t = DialogComplex<subClass_t>;
   GtkBox *mainBox_;
   GtkWindow *dialog_ = NULL;
   char *tar_ = NULL;
   char *zip_ = NULL;
   char *path_ = NULL;
   char *folder_ = NULL;
   const char *ext_[5]={"tgz","bz2","xz","zip",NULL};
   const char *formats_[5]={"GZip","BZip2","XZ","Zip",NULL};
   GtkEntry *targetEntry_ = NULL;
   GtkEntry *nameEntry_ = NULL;
   GtkLabel *nameExt_ = NULL; 
   public:
   ~untarResponse(void){
     g_free(folder_);
     g_free(tar_);
     g_free(zip_);
     g_free(path_);
   }
   const char **formats(void){ return formats_;}
   const char *path(void){ return (const char *)path_;}
   GtkEntry *targetEntry(void){ return targetEntry_;}
   GtkBox *getMainBox(void){ return mainBox_;}
   GtkEntry *nameEntry(void){return nameEntry_;}
   GtkLabel *nameExt(void){return nameExt_;}
   
    const char *title(void){ return _("Compressed file...");}
    const char *label(void){return _("Extract files from the archive");}
    void dialog(GtkWindow *value){ dialog_ = value; }
    GtkWindow *dialog(void){return dialog_;}

     static void *asyncYes(void *data){
      auto dialogObject = (DialogEntry<untarResponse<Type>> *)data;
      TRACE("%s", "Tar hello world\n");
      auto mainBox = dialogObject->subClass()->getMainBox();
      auto targetEntry = dialogObject->subClass()->targetEntry();
      auto buffer = gtk_entry_get_buffer(targetEntry);
      auto target = gtk_entry_buffer_get_text(buffer);

      char *tar = g_find_program_in_path("tar");
      char *zip = g_find_program_in_path("zip");

      auto path = dialogObject->subClass()->path();
      const char *extension = strrchr(path, '.');
      if (!extension){
        DBG("extension is null. Should not happen.\n");
        return NULL;
      }

      auto base = g_path_get_basename(path);       
      auto childWidget =Child::getChild();
      auto workDir = Child::getWorkdir(childWidget);
      auto buttonSpace = GTK_BOX(g_object_get_data(G_OBJECT(childWidget), "buttonSpace"));

      char *cmd = NULL;
      if (strcmp(extension, ".tgz") == 0) cmd = g_strconcat(tar, " ", "-xzf", NULL);
      else if (strcmp(extension, ".bz2") == 0) cmd = g_strconcat(tar, " ", "-xjf", NULL);
      else if (strcmp(extension, ".xz") == 0) cmd = g_strconcat(tar, " ", "-xJf", NULL);
      else if (strcmp(extension, ".zip") == 0) cmd = g_strconcat(zip, " ", "-vr", NULL);

      if (cmd) {
        char *command = g_strdup_printf("cd \"%s\" && %s \"%s\" ", target, cmd, path);
        TRACE("command = %s\n", command);
        pid_t childPid = Run<Type>::shell_command(Child::getOutput(NULL), command, false, false);
        auto runButton = new RunButton<Type>(EMBLEM_PACKAGE, NULL);
        runButton->init(command, childPid, Child::getOutput(NULL), workDir, buttonSpace);
        g_free(command);
      }

      Settings::setString("Tarballs", "Default", target);
      g_free(zip);
      g_free(tar);
      g_free(base);

      return NULL;
    }

    static void *asyncNo(void *data){
      TRACE("%s", "Tar goodbye world\n");
      return NULL;
    }
 
    GtkBox *mainBox(const char *folder, const char *path) {
      tar_ = g_find_program_in_path("tar");
      zip_ = g_find_program_in_path("zip");
      path_ = g_strdup(path);

        mainBox_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        //gtk_widget_set_size_request(GTK_WIDGET(mainBox_), 550, 400);
        gtk_widget_set_vexpand(GTK_WIDGET(mainBox_), false);
        gtk_widget_set_hexpand(GTK_WIDGET(mainBox_), true);

        auto hbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_halign(GTK_WIDGET(hbox), GTK_ALIGN_CENTER);
        gtk_widget_set_vexpand(GTK_WIDGET(hbox), false);
        gtk_widget_set_hexpand(GTK_WIDGET(hbox), true);
        gtk_box_append(mainBox_, GTK_WIDGET(hbox));
        
        auto label0 = gtk_label_new(label());
        gtk_widget_set_halign(label0, GTK_ALIGN_CENTER);
        gtk_box_append(hbox, GTK_WIDGET(label0));
        
        hbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_halign(GTK_WIDGET(hbox), GTK_ALIGN_CENTER);
        gtk_widget_set_vexpand(GTK_WIDGET(hbox), false);
        gtk_widget_set_hexpand(GTK_WIDGET(hbox), true);
        gtk_box_append(mainBox_, GTK_WIDGET(hbox));

        auto pathLabel = gtk_label_new("");
        auto markup = g_strconcat("<span color=\"blue\"><b>",path,"</b></span>",NULL);
        gtk_label_set_markup(GTK_LABEL(pathLabel), markup);
        g_free(markup);
        gtk_box_append(hbox, GTK_WIDGET(pathLabel));
        
        hbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_vexpand(GTK_WIDGET(hbox), false);
        gtk_widget_set_hexpand(GTK_WIDGET(hbox), true);
        gtk_box_append(mainBox_, GTK_WIDGET(hbox));

        hbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_box_append(mainBox_, GTK_WIDGET(hbox));
        auto entryLabelText = _("Target directory:"); 
        targetEntry_ =
          FileResponse<Type, subClass_t>::addEntry(hbox, "targetEntry", entryLabelText, this);
        
        auto buffer = gtk_entry_get_buffer(targetEntry_);
        folder_ = Settings::getString("Tarballs", "Default");
        if (!folder_) folder_ = g_strdup(g_get_home_dir());
        gtk_entry_buffer_set_text(buffer, folder_, -1);

        auto action_area = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_vexpand(GTK_WIDGET(action_area), false);
        gtk_widget_set_hexpand(GTK_WIDGET(action_area), false);
        gtk_box_append(mainBox_, GTK_WIDGET(action_area));
        gtk_widget_set_halign(GTK_WIDGET(action_area), GTK_ALIGN_END);

        auto cancelButton = UtilBasic::mkButton(EMBLEM_RED_BALL, _("Cancel"));
        gtk_box_append(action_area,  GTK_WIDGET(cancelButton));
        gtk_widget_set_vexpand(GTK_WIDGET(cancelButton), false);

        auto saveButton = UtilBasic::mkButton (EMBLEM_GREEN_BALL, _("Apply"));
        gtk_box_append(action_area,  GTK_WIDGET(saveButton));
        gtk_widget_set_vexpand(GTK_WIDGET(saveButton), false);

        g_signal_connect (G_OBJECT (saveButton), "clicked", G_CALLBACK (button_save), this);
        g_signal_connect (G_OBJECT (cancelButton), "clicked", G_CALLBACK (button_cancel), this);

        return mainBox_;
    }

    const char *folder(void){ return (const char *)folder_;}
    void folder(const char *value){ folder_ = g_strdup(value);}
   
   private:

    static void
    toggle(GtkCheckButton* self, void *data){
      auto subClass = (subClass_t *)data;
      auto ext = (const char *)g_object_get_data(G_OBJECT(self), "ext");
      auto label = subClass->nameExt();
      auto markup = g_strconcat("<span color=\"blue\">.",ext,"</span>", NULL);
      gtk_label_set_markup(label, markup);
      g_free(markup);
    }
    
    static void
    button_save (GtkButton * button, gpointer data) {
      auto subClass = (subClass_t *)data;
      g_object_set_data(G_OBJECT(subClass->dialog()), "response", GINT_TO_POINTER(1));
    }

    static void
    button_cancel (GtkButton * button, gpointer data) {
      auto subClass = (subClass_t *)data;
      g_object_set_data(G_OBJECT(subClass->dialog()), "response", GINT_TO_POINTER(-1));
    }


};

}
#endif

