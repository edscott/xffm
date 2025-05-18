#ifndef XF_PASSWDRESPONSE_HH
#define XF_PASSWDRESPONSE_HH

namespace xf {
class PasswordDialog {
   const char *title_;
   const char *iconName_;
public:
    const char *title(void){ return _("Security");}
    const char *iconName(void){ return "dialog-authentication";}
    const char *label(void){ return _("Enter password");}

    static void *asyncYes(void *data){
      auto dialogObject = (DialogBasic<PasswordDialog> *)data;
      auto dialog = dialogObject->dialog();
      
      auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(dialog), "entry"));
      auto buffer = gtk_entry_get_buffer(entry);
      auto text = g_strdup(gtk_entry_buffer_get_text(buffer));
      gtk_entry_buffer_set_text(buffer, "alkdflakj lkjasdlfkj qwlekr sadlfk lsdflkasjdf ", -1);
      gtk_entry_buffer_set_text(buffer, "", -1);
        

      if (text && strlen(text)) {
          fprintf (stdout, "%s\n", text);
          memset(text, 0, strlen(text));
      } else {
          // No password, then
          // send interrupt signal to parent
          pid_t parent = getppid();
          kill(parent, SIGINT);

          //kill(currentProcess, SIGINT);
          //kill(parentProcess, SIGINT);
      }
      g_free(text);
      
      TRACE("%s", "hello world\n");
      return NULL;
    }

    static void *asyncNo(void *data){
      auto dialogObject = (DialogBasic<PasswordDialog> *)data;
      auto dialog = dialogObject->dialog();
      // Cancel
      // send interrupt signal to parent
      pid_t parent = getppid();
      kill(parent, SIGINT);
      return NULL;
    }
 
private:

    static void activate(GtkEntry *entry, void *dialog){
      g_object_set_data(G_OBJECT(dialog), "response", GINT_TO_POINTER(2));
    }



    static gboolean 
    response_delete(GtkWidget *dialog, GdkEvent *event, gpointer data){
        return TRUE;
    }

};

class PasswordResponse {
// This runs in its own process, called by sudo/ssh ASK_PASSWORD.    
// We need another class to run inside app's process:
//    basically just a dialog entry set to password mode.
public:
    static void sendPassword(const char **argv){
      gtk_init ();
      IconTheme::init();

      char *string=NULL;
      if (argv[1]) {
          if (strncmp(argv[1], "Password", strlen("Password"))==0) 
              string = g_strdup_printf("%s:", _("Enter password"));
          else{
              string = g_strdup(_(argv[1]));
          }
      } 
      
      auto dialogObject = new DialogPasswd<PasswordDialog>;
      dialogObject->setParent(GTK_WINDOW(MainWidget));
      auto dialog = dialogObject->dialog();
      auto entry = GTK_ENTRY( g_object_get_data(G_OBJECT(dialog),"entry"));
      gtk_entry_set_visibility (entry, false);

      
      TRACE("create dialogObject=%p\n", dialogObject); 
      dialogObject->setLabelText(string);
      
      dialogObject->run();
      while (g_list_model_get_n_items (gtk_window_get_toplevels ()) > 0)
        g_main_context_iteration (NULL, TRUE);
        exit(0);
    }

 };

}
#endif
