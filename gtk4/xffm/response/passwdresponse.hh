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

    static void *asyncStart(void *data){
      auto dialogObject = (Dialog<PasswordDialog> *)data;
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


    static void *asyncEnd(void *dialog){
      TRACE("%s", "goodbye world\n");
      return NULL;
    }


    void content(GtkWindow *dialog, GtkBox *contentArea){

       auto entryBox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
       gtk_widget_set_hexpand(GTK_WIDGET(entryBox), false);
       gtk_widget_set_halign (GTK_WIDGET(entryBox),GTK_ALIGN_CENTER);
       gtk_box_append(GTK_BOX (contentArea), GTK_WIDGET(entryBox));

        
       auto entry = GTK_ENTRY(gtk_entry_new ());
       gtk_box_append(GTK_BOX (entryBox), GTK_WIDGET(entry));
       gtk_widget_set_halign (GTK_WIDGET(entry),GTK_ALIGN_START);
       g_object_set_data(G_OBJECT(dialog),"entry", entry);
       gtk_entry_set_visibility (entry, false);
       
       
       g_object_set_data(G_OBJECT(entry),"dialog", dialog);
       g_signal_connect (G_OBJECT (entry), "activate", 
                ENTRY_CALLBACK (activate), (void *)dialog);

    }

    void action(GtkWindow *dialog, GtkBox *actionArea){
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
    
public:
    static void sendPassword(gchar **argv){
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
      
      auto dialogObject = new Dialog<PasswordDialog>(NULL);
      TRACE("create dialogObject=%p\n", dialogObject); 
      dialogObject->setLabelText(string);
      auto dialog = dialogObject->dialog();
      auto no = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "no"));
      gtk_widget_set_visible(no, false);
      
      dialogObject->run();
      while (g_list_model_get_n_items (gtk_window_get_toplevels ()) > 0)
        g_main_context_iteration (NULL, TRUE);
        exit(0);
    }

 };

}
#endif
