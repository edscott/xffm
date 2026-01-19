#ifndef HISTORY_HH
#define HISTORY_HH
#include <readline/readline.h>
#include  <readline/history.h>
#define XF_HISTORY g_get_user_cache_dir(),G_DIR_SEPARATOR_S,"xffm4",G_DIR_SEPARATOR_S,"xf","_history"

static const gchar *historyFile;
namespace xf {

  class History {
    public:
    static void reset(){
      history_set_pos(0);
    }
    static void
    init(void){
      historyFile = g_strconcat(XF_HISTORY, NULL);
      if (!g_file_test(historyFile, G_FILE_TEST_EXISTS)) {
        TRACE("*** reading xffm3 history...\n");
        // gtk3 history
        auto oldHistory = g_strconcat(g_get_user_cache_dir(),G_DIR_SEPARATOR_S,"lp_terminal_history",NULL);
        if (g_file_test(oldHistory, G_FILE_TEST_EXISTS)){
          FILE *in = fopen(oldHistory, "r");
          FILE *out = fopen(historyFile, "w");
          char buffer[256];
          while (fgets(buffer, 256, in) && !feof(in)){
              fputs(buffer, out);              
          }
          fclose(in);
          fclose(out);
          // this is threaded and will not be done in time:
          // pathResponse::cpmv(oldHistory, historyFile, 1);
        }
        g_free(oldHistory);
      } 

      using_history();
      read_history(historyFile);      
    }
    static void 
    down(GtkTextView *input){
      auto position = where_history();
      if (position == 0 || position == history_length) return;

      auto prefix = (char *)g_object_get_data(G_OBJECT(input), "prefix");
      if (prefix) {
          history_set_pos(position + 1);
        if (history_search_prefix(prefix, 1)==0){
          auto h = current_history();
          //TRACE("Down found %s at %d\n", h->line, where_history());
          Print::clear_text(input);
          Print::print(input, g_strdup_printf("%s", h->line));
        } else {
          history_set_pos(position);
        }
      } else {
        auto h = next_history();
        Print::clear_text(input);
        Print::print(input, g_strdup_printf("%s", h?h->line:""));
      }
    }
    static void
    up(GtkTextView *input){
      auto position = where_history();
      if (position == 0){
        history_set_pos(history_length);
        position = where_history();
        auto prefix = Print::inputText(input);
        if (strlen(prefix)) g_object_set_data(G_OBJECT(input), "prefix", prefix);
        else {
          g_free(prefix);
          auto prefix_p = g_object_get_data(G_OBJECT(input), "prefix");
          g_free(prefix_p);
          g_object_set_data(G_OBJECT(input), "prefix", NULL);
        }
      }
      //TRACE("start position=%d\n", position);
      auto prefix = (char *)g_object_get_data(G_OBJECT(input), "prefix");
      if (prefix) {
        if (where_history() != history_length){
          history_set_pos(where_history() - 1);
        }
        if (history_search_prefix(prefix, -1)==0){
          auto h = current_history();
          //TRACE("Up found %s at %d\n", h->line, where_history());
          Print::clear_text(input);
          Print::print(input, g_strdup_printf("%s", h->line));
        }
      } else {
        auto h = previous_history();
        Print::clear_text(input);
        Print::print(input, g_strdup_printf("%s", h?h->line:""));
      }
    }
    static char *
    history(void){
      char *t = NULL;
      HIST_ENTRY **history = history_list();
      int k=1;
      const char *last = "";
      char buffer[256];
      for (HIST_ENTRY **p=history; p && *p; p++, k++){
        if (strcmp(last, (*p)->line)){
          snprintf(buffer, 256, "%5d  %s\n", k, (*p)->line);
          Basic::concat(&t, buffer);
        }
        last = (*p)->line;
      }
      return t;
    }

    static bool
    add(const char *text){
        TRACE("*** add %s\n", text);
      errno=0;
      gchar *dirname = g_path_get_dirname(historyFile);
      if (!g_file_test(dirname, G_FILE_TEST_IS_DIR)){
        if (mkdir(dirname, 0700) != 0 ){
          ERROR_("addHistory(): cannot create \"%s\"\n", historyFile);
          return false;
        }
      }
      g_free(dirname);

      // remove previous history entries (if found)

      int which=-1;
      int pos = history_length - 1;
      which = history_search_pos(text, -1, pos);
      while (pos >0 && which >=0){
        TRACE("*** found %s at %d history_length=%d\n", text, which, history_length);
        HIST_ENTRY *entry = history_get(which+1);
        pos = which - 1;

        if (strcmp(entry->line, text) == 0){
          // workaround of the index vs offset mixup history bug.
          TRACE("*** which %d is %s\n", which, entry->line);
          auto h = remove_history (which);
          free_history_entry(h); 
        }
        which = history_search_pos(text, -1, pos);
      }

      // get last entry
      HIST_ENTRY *p = history_get(history_length);
      if (history_length == 0 || (p != NULL && strcmp(p->line, text))){
        add_history(text);
        write_history(historyFile);
      } else {
        //TRACE("add(\"%s\", \"%s\"): skipped\n",p->line, text);
      }
      reset();
      return true;
    }
      
    // List histories
/*    static gchar *
    showHistory (void) {
      char *t = NULL; 
      auto history = g_strconcat(XF_HISTORY, NULL);

FILE *historyFile = fopen (history, "r");
        if(historyFile) {
            gchar line[256];
            memset (line, 0, 256);
            while(fgets (line, 255, historyFile) && !feof (historyFile)) {
              if (!strchr (line, '\n')) line[255] = '\n';
              concat(&t, line);
            }
            fclose (historyFile);
        }           
        return t;
    }*/

  };
}
#endif
