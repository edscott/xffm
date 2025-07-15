#ifndef FFUNCTIONS_HH
# define FFUNCTIONS_HH
namespace xf {
  class Ffunctions {
    public:
      
     static const char **ftypes(void){
       static const char *ftypes_[] = {
         N_("Regular"),
         N_("Directory"),
         N_("Symbolic Link"),
         N_("Socket"),
         N_("Block device"),
         N_("Character device"),
         N_("FIFO"),
         N_("Any"),
         NULL
       };
       return ftypes_;
     }
     static const char **ft(void) {
       static const char *ft_[] = {
          "reg",
          "dir",
          "sym",
          "sock",
          "blk",
          "chr",
          "fifo",
          "any",
          NULL
       };
       return ft_;
     }


    public:
      
    protected:

    static void 
    sensitivizeSpin (GtkCheckButton *check, gpointer data){
        TRACE("*** sensitivizeSpin, widget = %p\n", data);
        GtkWidget *widget = GTK_WIDGET(data);
        gtk_widget_set_sensitive(widget, gtk_check_button_get_active(check));
    }

    static void
    sensitivize ( GtkEntryBuffer* self, guint position, gchar* chars, guint n_chars, void *data){
      auto box = GTK_WIDGET(data);
      auto text = gtk_entry_buffer_get_text(self);
      gtk_widget_set_sensitive(box, strlen(text)>0);
    }


    static void 
    saveHistory (GtkEntry *entry, const gchar *history, const gchar *text) {
        char *historyDir = g_path_get_dirname(history);
        TRACE("history dir = %s\n", historyDir);
        if (!g_file_test(historyDir,G_FILE_TEST_IS_DIR)){
            g_mkdir_with_parents (historyDir, 0770);
        }
        g_free(historyDir);

        auto list = (GList *)g_object_get_data(G_OBJECT(entry), "list");

        // if item is already in history, bring it to the front
        // else, prepend item.
       
        bool found = false;
        for (auto l=list; l && l->data; l=l->next){
          auto data = (char *)l->data;
          if (strcmp(data, text)==0){
            list = g_list_remove(list, data);
            list = g_list_prepend(list,data);
            found = true;
            break;
          }
        }
        if (!found){
            list = g_list_prepend(list,g_strdup(text));
        }
        g_object_set_data(G_OBJECT(entry), "list", list);
        // rewrite history file
        FILE *historyFile = fopen (history, "w");
        if(!historyFile) {
            ERROR_("saveHistory(): unable to write to file: \"%s\"\n", history);
            return;
        }

        for (auto l=list; l && l->data; l=l->next){
            fprintf (historyFile, "%s\n", (char *)l->data);
        }
        fclose (historyFile);
        return;
    }



  };
  
}
#endif
