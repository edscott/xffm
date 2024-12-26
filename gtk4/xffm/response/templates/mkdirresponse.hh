#ifndef MKDIRRESPONSE_HH
#define MKDIRRESPONSE_HH
namespace xf {

  template <class Type> class FileResponse;

  template <class Type>
  class mkdirResponse {
   const char *title_;
   const char *iconName_;
   void *parentObject_=NULL;
public:
   void *parentObject(void){ return parentObject_;}
   void parentObject(void *value){parentObject_ = value;}

    const char *title(void){ return _("Path");}
    const char *iconName(void){ return "dialog-question";}
    const char *label(void){ return _("New Folder");}


    static void setDefaults(GtkWindow *dialog, GtkLabel *label){
      auto entry = GTK_ENTRY( g_object_get_data(G_OBJECT(dialog),"entry"));
      auto path = (const char *)g_object_get_data(G_OBJECT(entry), "path");
      auto base = g_path_get_basename(path);
      auto dir = g_path_get_dirname(path);
      auto buffer = gtk_entry_get_buffer(entry);
      gtk_entry_buffer_set_text(buffer, base, -1);

      //auto string = g_strconcat("<span color=\"green\"><b>",_("New Folder"), ":\n</b></span><span color=\"blue\"><b>", dir, "</b></span>", NULL);
      auto string = g_strconcat("<span color=\"blue\"><b>",dir, "\n</b></span><span color=\"green\"><b>", _("New folder name:"), "</b></span>", NULL);
      gtk_label_set_markup(label, string);
      g_free(base);
      g_free(dir);
      g_free(string);
    }

    static void *asyncNo(void *data){
      auto dialogObject = (DialogEntry<mkdirResponse> *)data;
      auto dialog = dialogObject->dialog();
      auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(dialog), "entry"));
      auto path = g_object_get_data(G_OBJECT(entry), "path");
      g_free(path);

      return NULL;
    }

    static void *asyncYes(void *data){
       // this dialog
       auto dialogObject = (DialogEntry<mkdirResponse> *)data;
       
       // Test mode
       //auto retval = p->asyncCallback(p->asyncCallbackData());
       //DBG("p->asyncCallback(p->asyncCallbackData) -> %s\n", (const char *)retval);
       
       // send to action (asyncYesArg) where entry property "path" is g_free'd
       asyncYesArg(data, "mkdir");      
       return NULL;
    }

private:


    static void *asyncYesArg(void *data, const char *op){
       if (!op) return NULL;
       auto dialogObject = (DialogEntry<mkdirResponse> *)data;
       auto dialog = dialogObject->dialog();
       auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(dialog), "entry"));
       auto buffer = gtk_entry_get_buffer(entry);
       const char *text = gtk_entry_buffer_get_text(buffer);
       auto path = (char *)g_object_get_data(G_OBJECT(entry), "path");
       auto dir = g_path_get_dirname(path);
       Basic::concat(&dir, G_DIR_SEPARATOR_S);
       Basic::concat(&dir, text);
 
       if (!g_file_test(dir, G_FILE_TEST_EXISTS)){
        DBG("got mkdir operation: path=\"%s\".\n", dir);
        if(mkdir(dir,0700) < 0){
          auto string = g_strdup_printf(_("Cannot create directory '%s' (%s)\n"), 
              dir, strerror(errno));
          Print::printError(Child::getOutput(), g_strconcat(_("Sorry"), " ", string, NULL));
          DBG("***%s\n", string);
          g_free(string);
        }
       }
       // This sets label in parent dialog and should also update
       // the column view and selected item.
       if (g_file_test(dir, G_FILE_TEST_IS_DIR)){
         auto p = (FileResponse<Type> *)dialogObject->subClass()->parentObject();
         auto retval = p->asyncCallback((void *)dir);
       }
      // cleanup
      g_free(path); 
      g_free(dir); 
      return NULL;
    }


};
}
#endif
