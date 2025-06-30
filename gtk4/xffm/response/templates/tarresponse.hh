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
   const char *formats_[5]={"GZip","BZip2","XZ",NULL};
//   const char *formats_[5]={"GZip","BZip2","XZ","Zip",NULL};
   GtkEntry *targetEntry_ = NULL;
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
   
    const char *title(void){ return _("Compressed file...");}
    const char *label(void){return _("Create a compressed archive with the selected objects");}
    void dialog(GtkWindow *value){ dialog_ = value; }
    GtkWindow *dialog(void){return dialog_;}

     static void *asyncYes(void *data){
      auto dialogObject = (DialogEntry<tarResponse<Type>> *)data;
      TRACE("%s", "Tar hello world\n");
      auto mainBox = dialogObject->subClass()->getMainBox();
      auto targetEntry = dialogObject->subClass()->targetEntry();
      auto buffer = gtk_entry_get_buffer(targetEntry);
      auto target = gtk_entry_buffer_get_text(buffer);
      const char *choice = "";
      for (auto p=dialogObject->subClass()->formats(); p && *p; p++){
          auto r = GTK_CHECK_BUTTON(g_object_get_data(G_OBJECT(mainBox), *p));
          if (gtk_check_button_get_active(r)) {
            choice = *p;
            break;
          }
      }
      const char *option = "-czf";
      const char *ext = ".tgz";
      if (strcmp(choice, "BZip2")==0){ 
        option = "-cjf";
        ext = ".bz2";
      }
      if (strcmp(choice, "XZ")==0){ 
        option = "-cJf";
        ext = ".xz";
      }
      auto tar = g_find_program_in_path("tar");
      if (tar){
        auto path = dialogObject->subClass()->path();
        auto base = g_path_get_basename(path);       
        auto output = g_strconcat(target, G_DIR_SEPARATOR_S, base, ext, NULL);
        auto command = g_strdup_printf("%s %s %s \"%s\"", tar, option, output, base);

        auto childWidget =Child::getChild();
        auto buttonSpace = GTK_BOX(g_object_get_data(G_OBJECT(childWidget), "buttonSpace"));
        auto workDir = Child::getWorkdir(childWidget);
        TRACE("command = \"%s\"\n", command);

        pid_t childPid = Run<Type>::shell_command(Child::getOutput(), command, false, false);
        auto runButton = new RunButton<Type>(EMBLEM_PACKAGE, NULL);
        runButton->init(command, childPid, Child::getOutput(), workDir, buttonSpace);
        
        //Run<Type>::thread_run(Child::getOutput(), command, false);
        g_free(tar);
        g_free(base);
        g_free(output);
        g_free(command);
        Settings::setString("Tarballs", "Default", target);
      }

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
        for (auto p=formats_; p && *p; p++){
          auto r = GTK_CHECK_BUTTON(gtk_check_button_new_with_label(*p));
          g_object_set_data(G_OBJECT(mainBox_), *p, r);
          gtk_box_append(hbox, GTK_WIDGET(r));
          if (!firstCheck) {
            gtk_check_button_set_group (r, NULL);
            firstCheck = r;
            gtk_check_button_set_active(r, true);
          }
          else gtk_check_button_set_group (r, firstCheck);
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

        auto action_area = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_vexpand(GTK_WIDGET(action_area), false);
        gtk_widget_set_hexpand(GTK_WIDGET(action_area), false);
        gtk_box_append(mainBox_, GTK_WIDGET(action_area));

        auto cancelButton = UtilBasic::mkButton("emblem-redball", _("Cancel"));
        gtk_box_append(action_area,  GTK_WIDGET(cancelButton));
        gtk_widget_set_vexpand(GTK_WIDGET(cancelButton), false);

        auto saveButton = UtilBasic::mkButton ("emblem-floppy", _("Save"));
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

