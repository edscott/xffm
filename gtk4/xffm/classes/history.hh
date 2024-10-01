#ifndef HISTORY_HH
#define HISTORY_HH
#include <readline/readline.h>
#include  <readline/history.h>
#define XF_HISTORY g_get_user_cache_dir(),G_DIR_SEPARATOR_S,"xffm+",G_DIR_SEPARATOR_S,"xf","_history"

static const gchar *historyFile;
namespace xf {

  class History {
    public:
    static void reset(){
      history_set_pos(0);
    }
    static void
    init(void){
      using_history();
      historyFile = g_strconcat(XF_HISTORY, NULL);
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
          //DBG("Down found %s at %d\n", h->line, where_history());
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
      //DBG("start position=%d\n", position);
      auto prefix = (char *)g_object_get_data(G_OBJECT(input), "prefix");
      if (prefix) {
        if (where_history() != history_length){
          history_set_pos(where_history() - 1);
        }
        if (history_search_prefix(prefix, -1)==0){
          auto h = current_history();
          //DBG("Up found %s at %d\n", h->line, where_history());
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
      errno=0;
      gchar *dirname = g_path_get_dirname(historyFile);
      if (!g_file_test(dirname, G_FILE_TEST_IS_DIR)){
        if (mkdir(dirname, 0700) != 0 ){
          DBG("addHistory(): cannot create \"%s\"\n", historyFile);
          return false;
        }
      }
      g_free(dirname);

      if (!g_file_test(historyFile, G_FILE_TEST_EXISTS)) {
        // gtk3 history
        auto oldHistory = g_strconcat(g_get_user_cache_dir(),G_DIR_SEPARATOR_S,"lp_terminal_history",NULL);
        if (g_file_test(oldHistory, G_FILE_TEST_EXISTS)){
          read_history(oldHistory);
          g_free(oldHistory);
        }
        if (write_history(historyFile) != 0){
          DBG("failed write_history to \"%s\": %s\n", historyFile, strerror(errno));
          return false;
        }
      } 
      // get last entry
      HIST_ENTRY *p = history_get(history_length);
      if (history_length == 0 || (p != NULL && strcmp(p->line, text))){
        add_history(text);
        write_history(historyFile);
      } else {
        //DBG("add(\"%s\", \"%s\"): skipped\n",p->line, text);
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
