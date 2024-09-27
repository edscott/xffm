#ifndef XF_PATHRESPONSE_HH
#define XF_PATHRESPONSE_HH

namespace xf {
class pathDialog {
public:

    static void *asyncYesArg(void *data, const char *op){
      auto dialogObject = (Dialog<pathDialog> *)data;
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
      auto name = g_strconcat(_("Copy of"), " ", base, NULL);
      auto buffer = gtk_entry_get_buffer(entry);
      gtk_entry_buffer_set_text(buffer, name, -1);
      g_free(name);
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
      auto path = (const char *)g_object_get_data(G_OBJECT(entry), "path");
      auto base = g_path_get_basename(path);
      auto name = g_strconcat(_("Rename"), " ", base, NULL);
      auto buffer = gtk_entry_get_buffer(entry);
      gtk_entry_buffer_set_text(buffer, name, -1);
      g_free(name);
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
      auto name = g_strconcat(_("Link"), " ", base, NULL);
      auto buffer = gtk_entry_get_buffer(entry);
      gtk_entry_buffer_set_text(buffer, name, -1);
      g_free(name);
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
}
#endif
