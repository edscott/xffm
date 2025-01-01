#ifndef MKDIRRESPONSE_HH
#define MKDIRRESPONSE_HH
namespace xf {

  template <class Type> class FileResponse;

  template <class Type>
  class mkdirResponse {
   const char *title_;
   const char *iconName_;
   void *parentObject_=NULL;
   char *dir_ = NULL;
public:
   void dir(char *value){ dir_ = value;}
   const char *dir(void){ return dir_;}
   void *parentObject(void){ return parentObject_;}
   void parentObject(void *value){parentObject_ = value;}

    const char *title(void){ return _("Path");}
    const char *iconName(void){ return "dialog-question";}
    const char *label(void){ return _("New Folder");}

    ~mkdirResponse(void){
      g_free(dir_);
    }

    static void *asyncNo(void *data){
    /*  auto dialogObject = (DialogEntry<mkdirResponse> *)data;
      auto dialog = dialogObject->dialog();
      auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(dialog), "entry"));
      auto path = g_object_get_data(G_OBJECT(dialog), "path");
      g_free(path);*/

      return NULL;
    }

    static void *asyncYes(void *data){
       // this dialog
       auto dialogObject = (DialogEntry<mkdirResponse> *)data;
       
       // Test mode
       //auto retval = p->asyncCallback(p->asyncCallbackData());
       //DBG("p->asyncCallback(p->asyncCallbackData) -> %s\n", (const char *)retval);
       
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
       auto dir = dialogObject->subClass()->dir();
       auto path = g_strdup(dir);
       
       if (strcmp(path, "/")) Basic::concat(&path, G_DIR_SEPARATOR_S);
       Basic::concat(&path, text);
 
       if (!g_file_test(path, G_FILE_TEST_EXISTS)){
        DBG("got mkdir operation: path=\"%s\".\n", dir);
        if(mkdir(path,0700) < 0){
          auto string = g_strdup_printf(_("Cannot create directory '%s' (%s)\n"), 
              path, strerror(errno));
          Print::printError(Child::getOutput(), g_strconcat(_("Sorry"), " ", string, NULL));
          DBG("***%s\n", string);
          g_free(string);
        }
       }
       // This sets label in parent dialog and should also update
       // the column view and selected item.
       if (g_file_test(path, G_FILE_TEST_IS_DIR)){
         auto p = (FileResponse<Type> *)dialogObject->subClass()->parentObject();
         auto retval = p->asyncCallback((void *)dir);
       }
      // cleanup
      g_free(path); 
      return NULL;
    }


};
}
#endif
