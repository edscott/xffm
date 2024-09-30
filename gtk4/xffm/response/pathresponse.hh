#ifndef XF_PATHRESPONSE_HH
#define XF_PATHRESPONSE_HH

namespace xf {
class pathDialog {
public:

    static void *asyncYesArg(void *data, const char *op){
      auto dialogObject = (DialogTimeout<pathDialog> *)data;
      auto dialog = dialogObject->dialog();
      
      auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(dialog), "entry"));
      auto path = (char *)g_object_get_data(G_OBJECT(entry), "path");
      auto buffer = gtk_entry_get_buffer(entry);
      auto text = gtk_entry_buffer_get_text(buffer);

      auto dir = g_path_get_dirname(path);
      auto newFile = g_strconcat(dir, G_DIR_SEPARATOR_S, text, NULL);
      auto output = Child::getOutput();

      auto op_f = g_find_program_in_path(op);
      if (!op_f) DBG("*** Error: %s not found\n", op);
      if (op) {
        if (g_file_test(newFile, G_FILE_TEST_EXISTS)){
          auto string = g_strconcat(_("Sorry"), " \"", text, "\" ", _("exists"), "\n", NULL);
          Print::printError(output, string);
        } else {
          if (strcmp(op, "cp") == 0 || strcmp(op, "mv") == 0) {          
            if (g_file_test(path, G_FILE_TEST_IS_DIR)){
              char *arg[]={(char *)op, (char *)"-a", (char *)"-v", path, newFile, NULL};
              Run<bool>::thread_run(output, (const char **)arg, false);
            } else {
              char *arg[]={(char *)op, (char *)"-v", path, newFile, NULL};
              Run<bool>::thread_run(output, (const char **)arg, false);
            }
          } else if (strcmp(op, "ln") == 0) {          
            char *arg[]={(char *)op, (char *)"-s", (char *)"-v", path, newFile, NULL};
            Run<bool>::thread_run(output, (const char **)arg, false);
          }
        }
      }      
     
done:
      g_free(op_f);
      g_free(path);
      g_free(dir);
      g_free(newFile);
      return NULL;
    }

    static void *asyncNo(void *data){
      auto dialogObject = (DialogEntry<pathDialog> *)data;
      auto dialog = dialogObject->dialog();
      auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(dialog), "entry"));
      auto path = g_object_get_data(G_OBJECT(entry), "path");
      g_free(path);

      return NULL;
    }
};

class cpDialog: public pathDialog {
   const char *title_;
   const char *iconName_;
public:
    const char *title(void){ return _("Path");}
    const char *iconName(void){ return "dialog-question";}
    const char *label(void){ return _("Duplicate");}

    static void setDefaults(GtkWindow *dialog, GtkLabel *label){
      auto entry = GTK_ENTRY( g_object_get_data(G_OBJECT(dialog),"entry"));
      auto path = (const char *)g_object_get_data(G_OBJECT(entry), "path");
      auto base = g_path_get_basename(path);
      //auto name = g_strconcat(_("Copy of"), " ", base, NULL);
      auto buffer = gtk_entry_get_buffer(entry);
      //gtk_entry_buffer_set_text(buffer, name, -1);
      gtk_entry_buffer_set_text(buffer, base, -1);
      //g_free(name);
      auto string = g_strconcat("<span color=\"blue\"><b>",_("Duplicate"), " ", base, ":</b></span>", NULL);
      gtk_label_set_markup(label, string);
      g_free(base);
      g_free(string);
    }
    
    
    static void *asyncYes(void *data){
       asyncYesArg(data, "cp");      
       return NULL;
    }
};

class mvDialog: public pathDialog {
   const char *title_;
   const char *iconName_;
public:
    const char *title(void){ return _("Path");}
    const char *iconName(void){ return "dialog-question";}
    const char *label(void){ return _("Rename");}

    static void setDefaults(GtkWindow *dialog, GtkLabel *label){
      auto entry = GTK_ENTRY( g_object_get_data(G_OBJECT(dialog),"entry"));
      auto path = (const char *)g_object_get_data(G_OBJECT(dialog), "path");
      auto base = g_path_get_basename(path);
      //auto name = g_strconcat(_("Rename"), " ", base, NULL);
      auto buffer = gtk_entry_get_buffer(entry);
      gtk_entry_buffer_set_text(buffer, base, -1);
      //g_free(name);
      auto string = g_strconcat("<span color=\"blue\"><b>",_("Rename"), " ", base, ":</b></span>", NULL);
      gtk_label_set_markup(label, string);
      g_free(base);
      g_free(string);
    }

    static void *asyncYes(void *data){
       asyncYesArg(data, "mv");      
       return NULL;
    }
};

class lnDialog: public pathDialog {
   const char *title_;
   const char *iconName_;
public:
    const char *title(void){ return _("Path");}
    const char *iconName(void){ return "dialog-question";}
    const char *label(void){ return _("symlink");}


    static void setDefaults(GtkWindow *dialog, GtkLabel *label){
      auto entry = GTK_ENTRY( g_object_get_data(G_OBJECT(dialog),"entry"));
      auto path = (const char *)g_object_get_data(G_OBJECT(entry), "path");
      auto base = g_path_get_basename(path);
      //auto name = g_strconcat(_("Link"), " ", base, NULL);
      auto buffer = gtk_entry_get_buffer(entry);
      gtk_entry_buffer_set_text(buffer, base, -1);
      //g_free(name);
      auto string = g_strconcat("<span color=\"blue\"><b>",_("symlink"), " ", base, ":</b></span>", NULL);
      gtk_label_set_markup(label, string);
      g_free(base);
      g_free(string);
    }
    static void *asyncYes(void *data){
       asyncYesArg(data, "ln");      
       return NULL;
    }
};

template <class pathClass>
class pathResponse {
    
public:
    static void action(const char *path){    

      auto dialogObject = new DialogEntry<pathClass>;
      dialogObject->setParent(GTK_WINDOW(MainWidget));
      auto dialog = dialogObject->dialog();
      auto entry = GTK_ENTRY( g_object_get_data(G_OBJECT(dialog),"entry"));
      g_object_set_data(G_OBJECT(entry), "path", g_strdup(path));

      dialogObject->subClass()->setDefaults(dialog, dialogObject->label());
      
      dialogObject->run();
    }

 };

class rmDialog {
  private:
    GtkButton *button_[4];
public:
    const char *title(void){ return _("Delete");}
    const char *iconName(void){ return "dialog-question";}
    const char *label(void){ return _("Delete");
    }

    ~rmDialog(void){
    }
    rmDialog(void){
      button_[0] = Basic::mkButton("dialog-warning", _("Shred")); //1
      button_[1] = Basic::mkButton("delete", _("Delete")); //2
      button_[2] = Basic::mkButton("user-trash", _("Trash")); //3
      button_[3] = NULL;
    }
    GtkButton **getButtons(void){ return button_;}
    
    static void setDefaults(GtkWindow *dialog, GtkLabel *label){
      auto path = (const char *)g_object_get_data(G_OBJECT(dialog), "path");
      auto base = g_path_get_basename(path);
      auto string = g_strconcat("<span color=\"blue\"><b>",_("Delete"), ":\n", base, "</b></span>", NULL);
      gtk_label_set_markup(label, string);
      g_free(base);
      g_free(string);
    }
    
    static void *asyncYes(void *data){
      auto dialogObject = (DialogTimeout<rmDialog> *)data;
      auto dialog = dialogObject->dialog();
      auto path = (char *)g_object_get_data(G_OBJECT(dialog), "path");
      int response = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(dialog), "response"));
      GError *error_ = NULL;
      auto info = G_FILE_INFO(g_object_get_data(G_OBJECT(dialog), "info"));
      auto file = G_FILE(g_file_info_get_attribute_object (info, "standard::file"));
      auto output = Child::getOutput();
      switch (response){
        case 1:
          {
              char *arg[]={(char *)"shred", (char *)"-fzuv", path, NULL};
              Run<bool>::thread_run(output, (const char **)arg, false);
          }
          break;
        case 2:
          {
              char *arg[]={(char *)"rm", (char *)"-frv", (char *)"--preserve-root=all", (char *)"--one-file-system",path, NULL};
              Run<bool>::thread_run(output, (const char **)arg, false);
          }
          break;
        case 3:
          if (!g_file_trash(file, NULL, &error_)){
            DBG("*** Error:: %s: %s\n", path, error_->message);
          }
          break;
      }

      g_free(path);
      return NULL;
    }
    static void *asyncNo(void *data){
      auto dialogObject = (DialogTimeout<rmDialog> *)data;
      auto dialog = dialogObject->dialog();
      auto path = (char *)g_object_get_data(G_OBJECT(dialog), "path");
      g_free(path);
      return NULL;
    }
};

}
#endif
