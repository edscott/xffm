#ifndef TARRESPONSE_HH
#define TARRESPONSE_HH

namespace xf {
template <class Type>
class tarResponse {
   using subClass_t = tarResponse;
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
   ~tarResponse(void){
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
    const char *label(void){return _("Create a compressed archive with the selected objects");}
    void dialog(GtkWindow *value){ dialog_ = value; }
    GtkWindow *dialog(void){return dialog_;}

     static void *asyncYes(void *data){
      auto dialogObject = (DialogEntry<tarResponse<Type>> *)data;
      TRACE("%s", "Tar hello world\n");
      auto mainBox = dialogObject->subClass()->getMainBox();
      auto targetEntry = dialogObject->subClass()->targetEntry();
      auto nameEntry = dialogObject->subClass()->nameEntry();
      auto buffer = gtk_entry_get_buffer(targetEntry);
      auto target = gtk_entry_buffer_get_text(buffer);
      buffer = gtk_entry_get_buffer(nameEntry);
      auto name = gtk_entry_buffer_get_text(buffer);
      const char *choice = "";
      for (auto p=dialogObject->subClass()->formats(); p && *p; p++){
          auto r = GTK_CHECK_BUTTON(g_object_get_data(G_OBJECT(mainBox), *p));
          if (gtk_check_button_get_active(r)) {
            choice = *p;
            break;
          }
      }
      char *tar = g_find_program_in_path("tar");
      char *zip = g_find_program_in_path("zip");
      int id = 0;
      const char *option = "-czf";
      const char *ext = ".tgz";
      char *cmd = tar;
      if (strcmp(choice, "BZip2")==0){ 
        option = "-cjf";
        ext = ".bz2";
        id = 1;
      }
      if (strcmp(choice, "XZ")==0){ 
        option = "-cJf";
        ext = ".xz";
        id = 2;
      }
      if (strcmp(choice, "Zip")==0){ 
        cmd = zip;
        option = "-vr";
        ext = ".zip";
        id = 3;
        g_free(tar);
        tar = NULL;
      }

      auto path = dialogObject->subClass()->path();
      auto base = g_path_get_basename(path);       
      auto output = g_strconcat(target, G_DIR_SEPARATOR_S, name, ext, NULL);
      auto childWidget =Child::getChild();
      auto workDir = Child::getWorkdir(childWidget);
      auto buttonSpace = GTK_BOX(g_object_get_data(G_OBJECT(childWidget), "buttonSpace"));

      if (cmd) {
        char *command = g_strdup_printf("%s %s \"%s\" \"%s\"", cmd, option, output, base);
        TRACE("command = \"%s\"\n", command);
        pid_t childPid = Run<Type>::shell_command(Child::getOutput(NULL), command, false, false);
        auto runButton = new RunButton<Type>(EMBLEM_PACKAGE, NULL);
        runButton->init(command, childPid, Child::getOutput(NULL), workDir, buttonSpace);
        g_free(command);
      }

      Settings::setString("Tarballs", "Default", target);
      Settings::setInteger("Tarballs", "Ext", id);
      g_free(zip);
      g_free(tar);
      g_free(base);
      g_free(output);

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

        GtkCheckButton *firstCheck = NULL;
        auto q = ext_;
        
        GtkCheckButton *r[4];
        int k=0;
        for (auto p=formats_; p && *p && q && *q; p++, q++, k++){
          if (!zip_ && k==3) continue;
          r[k] = GTK_CHECK_BUTTON(gtk_check_button_new_with_label(*p));
          g_object_set_data(G_OBJECT(mainBox_), *p, r[k]);
          gtk_box_append(hbox, GTK_WIDGET(r[k]));
          if (!firstCheck) {
            gtk_check_button_set_group (r[k], NULL);
            firstCheck = r[k];
          }
          else gtk_check_button_set_group (r[k], firstCheck);
          g_signal_connect(G_OBJECT(r[k]), "toggled", G_CALLBACK(toggle), this);
          g_object_set_data(G_OBJECT(r[k]), "ext", (void *)*q);
        }

        hbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_box_append(mainBox_, GTK_WIDGET(hbox));
        auto entryLabelText = _("Target directory:"); 
        targetEntry_ =
          FileResponse<Type, subClass_t>::addEntry(hbox, "targetEntry", entryLabelText, this);
        
        auto buffer = gtk_entry_get_buffer(targetEntry_);
        folder_ = Settings::getString("Tarballs", "Default");
        if (!folder_) folder_ = g_strdup(g_get_home_dir());
        gtk_entry_buffer_set_text(buffer, folder_, -1);

        auto nameText = _("Name:"); 
        auto nameLabel = gtk_label_new(nameText);
        nameExt_ = GTK_LABEL(gtk_label_new("")); 
        nameEntry_= GTK_ENTRY(gtk_entry_new());
        buffer = gtk_entry_get_buffer(nameEntry_);
        auto basename = g_path_get_basename(path);
        gtk_entry_buffer_set_text(buffer, basename, -1);
        g_free(basename);
        hbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_box_append(mainBox_, GTK_WIDGET(hbox));
        gtk_box_append(hbox, GTK_WIDGET(nameLabel));
        gtk_box_append(hbox, GTK_WIDGET(nameEntry_));
        gtk_box_append(hbox, GTK_WIDGET(nameExt_));

        // XXX recall last selection 
        auto ext = Settings::getInteger("Tarballs", "Ext");
        if (ext >= 0 && ext <= 4){
          gtk_check_button_set_active(r[ext], true);
        } else {
          gtk_check_button_set_active(r[0], true);
        }

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

