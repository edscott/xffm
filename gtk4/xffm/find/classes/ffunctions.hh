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


    protected:

      
    const char *
    filter_text_help(void) {
      return _("Basic rules:\n" "\n"
                "*  Will match any character zero or more times.\n"
                "?  Will match any character exactly one time\n"
                "[] Match any character within the [] \n"
                "^  Match at beginning of string\n" 
                "$  Match at end of string \n");
    }
    const char *
    grep_text_help(void) {
      return _("Reserved characters for extended regexp are . ^ $ [ ] ? * + { } | \\ "
        "( ) : \n"
        "In  basic regular expressions the metacharacters ?, +, {, |, (, "
        "and ) \n"
        "  lose their special meaning.\n"
        "\n"
        "The  period . matches  any  single  character.\n"
        "The caret ^ matches at the start of line.\n"
        "The dollar $ matches at the end of line.\n"
        "\n"
        "Characters within [ ] matches any single character in the list.\n"
        "Characters within [^ ] matches any single character *not* in the "
        "list.\n"
        "Characters inside [ - ] matches a range of characters (ie [0-9] or "
        "[a-z]).\n"
        "\n"
        "A regular expression may be followed by one of several repetition "
        "operators:\n"
        "?      The preceding item is optional and matched\n"
        "       at most once.\n"
        "*      The preceding item will be matched zero\n"
        "       or more times.\n"
        "+      The preceding item will be matched one or\n"
        "       more times.\n"
        "{n}    The preceding item is matched exactly n times.\n"
        "{n,}   The preceding item is matched n or more times.\n"
        "{n,m}  The preceding item is matched at least n times,\n"
        "       but not more than m times.\n"
        "\n"
        "To match any reserved character, precede it with \\. \n"
        "\n"
        "Two regular expressions may be joined by the logical or operator |.\n"
        "Two regular expressions may be concatenated.\n"
        "\n"
        "More information is available by typing \"man grep\"\n"
);
    }

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
