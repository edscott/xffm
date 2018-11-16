#ifndef CSHCOMPLETION_HH
#define CSHCOMPLETION_HH
#include "bash.hh"
#define CSH_HISTORY 	g_get_user_cache_dir(),"lp_terminal_history"

// We keep this non static so different pages of the notebook
// will have different csh histories. 
// In the end, startup history file will join all histories.
namespace xf {
template <class Type>

class CshCompletion{
    using print_c = Print<double>;
    using util_c = Util<double>;
    
public:
    //CshCompletion(GtkTextView *textView, GtkTextView *input): BashCompletion<Type>(textView, input){

    CshCompletion(void){
        csh_cmd_save_ = NULL;
        csh_history_list_ = NULL;
        csh_load_history();
        csh_clean_start();
    }

    static gboolean
    csh_is_valid_command (const gchar *cmd_fmt) {
        //return GINT_TO_POINTER(TRUE);
        TRACE ("csh_is_valid_command(%s)\n", cmd_fmt);
        GError *error = NULL;
        gint argc;
        gchar **argv;
        if(!cmd_fmt) return FALSE;
        if(!g_shell_parse_argv (cmd_fmt, &argc, &argv, &error)) {
            gchar *msg = g_strcompress (error->message);
            DBG ("csh.hh:: %s: %s\n", msg, cmd_fmt);
            g_error_free (error);
            g_free (msg);
            return (FALSE);
        }
        gchar **ap = argv;
        if (*ap==NULL) {
            errno = ENOENT;
            return (FALSE);
        }

        // assume command is correct if environment is being set
        if (strchr(*ap, '=')){
            g_strfreev (argv);
            return (TRUE);
        }

        gchar *path = g_find_program_in_path (*ap);
        if(!path) {
            gboolean direct_path = g_file_test (argv[0], G_FILE_TEST_EXISTS) ||
                strncmp (argv[0], "./", strlen ("./")) == 0 || strncmp (argv[0], "../", strlen ("../")) == 0;
            TRACE("argv[0]=%s\n",argv[0]);
            if(direct_path) {
                path = g_strdup (argv[0]);
            }
        }

        if(!path) {
            g_strfreev (argv);
            errno = ENOENT;
            return (FALSE);
        }
        // here we test for execution within sudo
        // this is to add the -A option, which is necesary for
        // password dialog to pop up, when password is required.
        gboolean retval = TRUE;
        if (strcmp(argv[0],"sudo")==0) {
            gint i=1;
            if (strcmp(argv[i],"-A")==0) i++;
            retval=csh_is_valid_command(argv[i]);
        }

        g_strfreev (argv);
        g_free (path);
        return retval;
    }

protected:
    void 
    csh_dirty_start(GtkTextView *input){ 
        TRACE( "csh_dirty_start\n");
        //start from current position.
        g_free(csh_cmd_save_);
        while (gtk_events_pending()) gtk_main_iteration();
        csh_cmd_save_ = print_c::get_text_to_cursor(input);
        TRACE( "csh_dirty_start: %s\n", csh_cmd_save_);
        if (csh_cmd_save_ && strlen(csh_cmd_save_)){
            csh_nth_ = 0;
            csh_completing_ = TRUE;
        } else {
            csh_clean_start();
        }

    }


    void 
    csh_clean_start(void){
        TRACE( "csh_clean_start\n");
        //start from bottom.
        g_free(csh_cmd_save_);
        csh_cmd_save_ = NULL;
        csh_nth_ = 0;
        csh_completing_ = FALSE;
        csh_history_counter_ = -1; // non completion counter.
        
    }


    gboolean
    is_completing(void){
        return csh_completing_;
    }

    gboolean
    query_cursor_position(GtkTextView *input){
        GtkTextBuffer *buffer = gtk_text_view_get_buffer (input);
        gint position = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(buffer), "cursor-position"));
        TRACE("cursor position=%d\n", position);
        if (!position) csh_completing_ = FALSE;
        return csh_completing_;
    }

    gboolean
    csh_completion(GtkTextView *input, GtkTextView *output,  gint direction){

        // Completing?
        const gchar *suggest = csh_find(input, output, csh_cmd_save_, direction);
        if (suggest){
            TRACE("csh_completion: suggest=%s at csh_nth_=%d\n", suggest, csh_nth_); 
            csh_place_command(input, output, suggest);
            return TRUE;
        }
        TRACE("csh_completion: no match to %s\n", csh_cmd_save_);
        csh_place_command(input, output, csh_cmd_save_);
        
        return FALSE;
    }


    void
    csh_save_history (const gchar * data) {
        GList *p;
        gchar *command_p = g_strdup(data);
        g_strstrip (command_p);
        // Get last registered command
        void *last = g_list_nth_data (csh_history_list_, 0);
        if (last && strcmp((gchar *)last, command_p) == 0) {
            g_free(command_p);
            // repeat of last command. nothing to do here.
            return;
        }
        // don't save to file if invalid command (cd counts as invalid).
        if(!csh_is_valid_command (command_p)) {
            if(strcmp (command_p, "cd") != 0 && strncmp (command_p, "cd ", strlen ("cd ")) != 0) {
                DBG ("not saving %s\n", command_p);
                g_free(command_p);
                return;
            }
        }

        // if command is already in history, bring it to the front
        GList *item = util_c::find_in_string_list(csh_history_list_, command_p);
        if (item){
            void *data = item->data;
            // remove old position
            csh_history_list_=g_list_remove(csh_history_list_, data);
            // insert at top of list (item 0 is empty string)
            csh_history_list_ = g_list_insert(csh_history_list_, data, 0);
            goto save_to_disk;
        }

        // so the item was not found. proceed to insert
        csh_history_list_ = g_list_insert(csh_history_list_, command_p, 0);

    save_to_disk:
        // rewrite history file
        gchar *history = g_build_filename (CSH_HISTORY, NULL);
        // read it first to synchronize with other xffm+ instances
        GList *disk_history = NULL;       
        FILE *sh_history = fopen (history, "r");
        if(sh_history) {
            char line[2048];
            memset (line, 0, 2048);
            while(fgets (line, 2047, sh_history) && !feof (sh_history)) {
                if(strchr (line, '\n')) *strchr (line, '\n') = 0;
                if(strcmp (line, command_p) != 0) {
                    disk_history = g_list_prepend (disk_history, g_strdup (line));
                }
            }
            fclose (sh_history);
        }
        disk_history = g_list_prepend (disk_history, g_strdup (command_p));
        disk_history = g_list_reverse(disk_history);

        sh_history = fopen (history, "w");
        if(sh_history) {
            GList *p;
            for(p = g_list_first (disk_history); p && p->data; p = p->next) {
                fprintf (sh_history, "%s\n", (gchar *)p->data);
                g_free (p->data);
            }
            fclose (sh_history);
        }
        g_list_free (disk_history);
        g_free (history);
        return;
    }

    gboolean
    csh_history(GtkTextView *input, GtkTextView *output,  gint offset){
        csh_history_counter_ += offset;
        if (csh_history_counter_ < 0)  {
            csh_history_counter_ = -1;
            csh_place_command(input, output, "");
            return TRUE;
        }
        if (csh_history_counter_ > g_list_length(csh_history_list_)-2) 
            csh_history_counter_ = g_list_length(csh_history_list_) - 2;

        const gchar *p = (const gchar *)g_list_nth_data (csh_history_list_, csh_history_counter_);
        
        TRACE( "csh_history: get csh_history_counter_=%d offset=%d \n", csh_history_counter_, offset);
        
        if(p) {
            csh_place_command(input, output, p);
        }
        return TRUE;
    }

private:
        GList *csh_history_list_;

	gchar *csh_cmd_save_;
	gint csh_nth_;
	gint csh_history_counter_;
        gboolean csh_completing_;

    const gchar *
    csh_find(GtkTextView *input, GtkTextView *output, const gchar *token, gint direction){
        if (csh_nth_) csh_nth_ += direction;
        if (csh_nth_ >= g_list_length(csh_history_list_)){
            csh_nth_ = g_list_length(csh_history_list_)-1;
            return NULL;
        }
        GList *list = g_list_nth(csh_history_list_, csh_nth_);
        for (;list && list->data; (direction>0)?list=list->next:list=list->prev, csh_nth_+=direction){
            if (list->data && strncmp(token, (gchar *)list->data, strlen(token))==0) {
                return (const gchar *)list->data;
            }
        }
        // When you reach the bottom, put command back.
        if (direction < 0) {
            csh_nth_ = 0;
            csh_place_command(input, output, token);
        }
        return NULL;
    }
    void 
    csh_place_command(GtkTextView *input, GtkTextView *output,  const gchar *data){
        print_c::print_status (input, g_strdup(data));
        place_cursor(input, output);
    }

    gpointer
    csh_load_history (void) {
        gchar *history = g_build_filename (CSH_HISTORY, NULL);
        GList *p;
        // clean out history list before loading a new one.
        for(p = csh_history_list_; p; p = p->next) {
            g_free (p->data);
        }
        g_list_free (csh_history_list_);
        csh_history_list_ = NULL;

        FILE *sh_history = fopen (history, "r");
        if(sh_history) {
            gchar line[2048];
            memset (line, 0, 2048);
            while(fgets (line, 2047, sh_history) && !feof (sh_history)) {
                if(strchr (line, '\n')) *strchr (line, '\n') = 0;
                if(strlen (line) == 0) continue;
                // skip invalid commands (except cd):
                if(!csh_is_valid_command (line)) {
                    if(strcmp (line, "cd") != 0 && strncmp (line, "cd ", strlen ("cd ")) != 0) {
                        TRACE ("lpterm_c::csh_load_history: HISTORY: invalid history command in %s: %s\n", history, line);
                        continue;
                    }
                }
                gchar *newline = util_c::compact_line(line);
                GList *element = util_c::find_in_string_list(csh_history_list_, newline);

                if (element) { 
                    // remove old element
                    gchar *data=(gchar *)element->data;
                    csh_history_list_ = g_list_remove(csh_history_list_, data);
                    g_free(data);
                }
                // put element at top of the pile
                csh_history_list_ = g_list_prepend(csh_history_list_, newline);
            }
            fclose (sh_history);
        }
        g_free (history);
            
        return NULL;
    }

    void 
    place_cursor(GtkTextView *input, GtkTextView *output){
        GtkTextIter iter;
        GtkTextBuffer *buffer = gtk_text_view_get_buffer (input);
        if (csh_completing_ && csh_cmd_save_) {
            gtk_text_buffer_get_iter_at_offset (buffer, &iter, strlen(csh_cmd_save_));
        } else {
            gchar *text = print_c::get_current_text (input);
//            gchar *text = print_c::get_current_text (output);
            gtk_text_buffer_get_iter_at_offset (buffer, &iter, strlen(text));
            g_free(text);
        }
        gtk_text_buffer_place_cursor (buffer, &iter);
    }





};
}


#endif

