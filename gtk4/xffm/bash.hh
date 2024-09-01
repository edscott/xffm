#ifndef BASH_COMPLETION_HH
#define BASH_COMPLETION_HH

        
extern char **environ;

namespace xf {
enum {
    MATCH_COMMAND,
    MATCH_FILE,
    MATCH_HISTORY,
    MATCH_USER,
    MATCH_VARIABLE,
    MATCH_HOST,
    MATCH_NONE
};

class BaseCompletion {
public:
    static const gchar *
    get_match_type_text(gint match_type){
        switch (match_type){
            case MATCH_COMMAND:
                return _("Command");
                break;
            case MATCH_FILE:
                return _("File");
                break;
            case MATCH_HISTORY:
                return _("Command history");
                break;
            case MATCH_USER:
                return _("User");
                break;
            case MATCH_VARIABLE:
                return _("Variable");
                break;
            case MATCH_HOST:
                return _("Host");
                break;
        }
        return "WTF";
    }

    static void
    msg_show_match(GtkTextView *output, gint match_type, const gchar *match){
        if (!output) return;
        Util::showText(output);
        if (!match) {
            const gchar *option_type = get_match_type_text(match_type);
            match = _("Found no match");
            Util::print_icon(output, "dialog-warning", "red", g_strdup_printf(" %s\n", match));
        } else {
            auto p = g_find_program_in_path(match);
            if (!p && g_file_test(match, G_FILE_TEST_IS_SYMLINK)){
                auto q = realpath(match, NULL);
                p = g_strdup_printf("%s -> %s", _("Symbolic link"), q);
                g_free(q);   
            }             
            if (!p && g_file_test(match, G_FILE_TEST_IS_DIR)) p = g_strdup(_("Directory"));
            if (!p && g_file_test(match, G_FILE_TEST_IS_REGULAR)) p = g_strdup(_("File"));
            if (p) {
              Util::print(output, "blue/white_bg", g_strdup_printf(" %s (%s)\n", match,p));
              g_free(p);
            } else {
              Util::print(output, "blue/white_bg", g_strdup_printf(" %s\n", match));
            }
        }
        Util::scroll_to_bottom(output);
    }

    
    
    static gchar *
    get_token(const char *in_token){
        if (!in_token || strlen(in_token) == 0) {
            return NULL;
        }

        gchar *token=NULL;
        if (*in_token == '~' && strchr(in_token, '/')){
            if (strncmp(in_token, "~/", strlen("~/"))==0){
                token = g_strconcat(g_get_home_dir(), in_token+1, NULL);
            } else {
                gchar *dir = Util::get_tilde_dir(in_token);
                if (dir) token = g_strconcat(dir, strchr(in_token, '/')+1, NULL);
                g_free(dir);
            }
        } 
        if (!token) {
            token = g_strdup(in_token);
        }
        return token;
    }

    static GSList *
    baseFileCompletionList(const gchar *workdir, const char *in_token){
        gchar *string = base_file_completion(NULL, workdir, in_token);
        GSList *list = NULL;
        if (string) list = g_slist_prepend(list, string);
        return list;
    }

    static gchar *
    base_file_completion(GtkTextView *output, const gchar *workdir, const char *in_file_token){
        
        gchar *file_token = get_token(in_file_token);
        if (!file_token) return NULL;
      
        GSList *matches=NULL;

        gchar *directory;
        gchar *relative_directory=NULL;
        if (g_path_is_absolute(file_token)) {
            directory = g_strdup_printf("%s*", file_token);

        } else {
            if (!workdir || !g_file_test(workdir, G_FILE_TEST_IS_DIR)){
                workdir = g_get_home_dir();
            }
            directory = g_strdup_printf("%s/%s*", workdir, file_token);
            relative_directory = g_path_get_dirname(file_token);
            if (strcmp(relative_directory, ".")==0 && 
                    strncmp(file_token, "./", strlen("./")) != 0) {
                g_free(relative_directory);
                relative_directory=NULL;
            }
        }
        TRACE("file_token=%s\ndirectory=%s\n", file_token, directory);
        glob_t stack_glob_v;
        gint flags=GLOB_NOESCAPE;

        glob(directory, flags, NULL, &stack_glob_v);
        g_free(directory);

        if (stack_glob_v.gl_pathc > maxOptions()){
            //Util::print(output, "blue/white_bg",g_strdup_printf("%s ", file_token));

            Util::print_error(output, g_strdup_printf("%s: %ld %s\n", 
                  _("Matches"), maxOptions(), _("Too many selected files")));
            globfree(&stack_glob_v);
            g_free(file_token);
            return NULL;
        } else if (stack_glob_v.gl_pathc == 0){
            TRACE("NO MATCHES\n");
            globfree(&stack_glob_v);
            g_free(file_token);
            return NULL;
        } 
        gint i;
        for (i=0; i<stack_glob_v.gl_pathc; i++){
            gboolean isdir=g_file_test(stack_glob_v.gl_pathv[i], G_FILE_TEST_IS_DIR);
            gchar *base;
            if (g_path_is_absolute(file_token)) {
                base = g_strdup(stack_glob_v.gl_pathv[i]);
            } else {
                base = g_path_get_basename(stack_glob_v.gl_pathv[i]);
                if (relative_directory) {
                    gchar *b=g_build_filename(relative_directory, base, NULL);
                    g_free(base);
                    base=b;
                }
            }
            if (isdir) {
                gchar *d=g_strconcat(base, "/", NULL);
                g_free(base);
                base = d;
            }

            matches = g_slist_append (matches, base);
        }
        g_free(relative_directory);
        globfree(&stack_glob_v);
        g_free(file_token);
        return listMatches(output, matches, MATCH_FILE);
    }

    static GSList *
    baseExecCompletionList(const gchar *workdir, const char *in_token){

        gchar *token=get_token(in_token);
        if (!token || !strlen(token)) return NULL;
            
        glob_t stack_glob_v;
        gboolean straight_path = g_path_is_absolute(token);
        if (strlen(token)>1 &&  strncmp(token, "./", strlen("./"))==0)
            straight_path = TRUE;
        if (strlen(token)>2 &&  strncmp(token, "../", strlen("../"))==0)
            straight_path = TRUE;

        if (straight_path) {
            gchar *d;
            d = g_strdup(workdir);
            if (chdir(d) < 0){ 
                TRACE("chdir %s\n",d);
            }
            g_free(d);
            
            gchar *directory = g_strdup_printf("%s*", token);
            glob(directory, 0, NULL, &stack_glob_v);
            g_free(directory);

        }     
        else if (getenv("PATH") && strlen(getenv("PATH"))){
            gchar *path_v = g_strdup(getenv("PATH"));
            gchar **path_pp = g_strsplit(path_v, ":", -1);
            gchar **pp = path_pp;
                
            for (; pp && *pp; pp++){
                if (strlen(*pp)==0) continue;

                gint flags;
                gchar *directory=g_strdup_printf("%s/%s*", *pp, token);
                if (pp==path_pp) flags = 0;
                else flags =  GLOB_APPEND;
                //gint glob_result = 
                TRACE("glob: %s\n", directory);
                glob(directory, flags, NULL, &stack_glob_v);
                g_free(directory);
                if (stack_glob_v.gl_pathc > maxOptions()) {
                    TRACE("stack_glob_v.gl_pathc > maxOptions()\n");
                    break;
                }
            }
            g_strfreev(path_pp);
            g_free(path_v);
        }

        if (stack_glob_v.gl_pathc > maxOptions()){
           // Util::print(output, "Red/white_bg",g_strdup_printf("%s ", token));
           // Util::print_error(output, g_strdup_printf(_("More than %d matches\n"), maxOptions()));

            globfree(&stack_glob_v);
            g_free(token);
            return NULL;
            //msg_too_many_matches();
            //*match_count_p = -1;
        } else if (stack_glob_v.gl_pathc == 0){
            TRACE("NO MATCHES\n");
            globfree(&stack_glob_v);
            g_free(token);
            return NULL;
            //*match_count_p = 0;
            //msg_show_match(MATCH_FILE, NULL);
        } 
        
        GSList *matches=NULL;

        struct stat st;
        gint i;
        for (i=0; i<stack_glob_v.gl_pathc; i++){
            TRACE("gl_pathv[%d] = %s\n", i, stack_glob_v.gl_pathv[i]);
            // stack_glob_v.gl_pathv is initialized in the glob() call.
            // coverity[uninit_use : FALSE]
            errno=0;
            if (stat (stack_glob_v.gl_pathv[i], &st)==0 && (S_IXOTH & st.st_mode)){
                gchar *base;
                if (straight_path) {
                    base = g_strdup(stack_glob_v.gl_pathv[i]);
                } else {
                    base = g_path_get_basename(stack_glob_v.gl_pathv[i]);
                }
                matches = g_slist_append (matches, base);
                TRACE("%d) %s\n", i, base);
            }
            if (errno){
                TRACE("base.hh::baseExecCompletionList(): stat %s (%s)\n",
                    stack_glob_v.gl_pathv[i], strerror(errno));
                errno=0;
            }
        }
        globfree(&stack_glob_v);
        g_free(token);
        return  matches;
    }

    static gchar *
    base_exec_completion(GtkTextView *output, const gchar *workdir, const char *in_token){
        GSList *matches = baseExecCompletionList(workdir, in_token);
        return  listMatches(output, matches, MATCH_COMMAND);
    }

    static gchar *
    listMatches (GtkTextView *output, GSList *matches, gint match_type){
        if (!matches || !g_slist_length(matches)) return NULL;
        matches = g_slist_sort (matches, ya_strcmp);
        GSList *a;
        GSList *b;
        int length;
        GSList *p = matches;
        char *suggest = g_strdup ((gchar *) (p->data));
        int equal_length=strlen(suggest);
        for(a = matches; a && a->data; a = a->next) {
            TRACE("comparing a %s\n", (gchar *)a->data);
            for(b = a->next; b && b->data; b = b->next) {
                TRACE("comparing b %s\n", (gchar *)b->data);
                length=Util::length_equal_string(
                        (const gchar *)(a->data), (const gchar *)(b->data));
                if(length < equal_length) {
                    equal_length = length;
                }
                TRACE("comparing %s to %s: length=%d\n", (gchar *)a->data, (gchar *)b->data, length);
            }
        }
        suggest[equal_length]=0;
        TRACE("string=%s, equal_length=%d, suggest=%s\n",
                (gchar *) (p->data), equal_length, suggest);

        a = NULL;
        if (match_type == MATCH_COMMAND){
            for(p = matches; p && p->data; p = p->next) {
                gboolean OK = FALSE;
                if (g_file_test((gchar *)p->data, G_FILE_TEST_IS_EXECUTABLE)) {
                    OK=TRUE;
                } else {
                    if (g_file_test((gchar *)p->data, G_FILE_TEST_IS_DIR)) {
                        OK=TRUE;
                    } else {
                        gchar *u = g_find_program_in_path((gchar *)p->data);
                        if (u){ g_free(u); OK= TRUE;}
                    }
                }
                if (OK) a = g_slist_prepend(a, p->data);
                else  g_free(p->data);
            }
            g_slist_free(matches);
            matches = a;
        }

        int i;
        if (output){
            g_object_set_data(G_OBJECT(output), "matchCount", 
                    GINT_TO_POINTER(g_slist_length(matches)));
            TRACE("matchCount = %d\n", g_slist_length(matches));
        }
        if (output && g_slist_length(matches) > 1) {
            Util::print(output, "blue/white_bg", g_strconcat(_("Options:"), "\n",NULL));
            for(p = matches; p && p->data; p = p->next) {
                TRACE("msg_show_match call 1\n");
                msg_show_match(output, match_type, (const gchar *)p->data);
                g_free(p->data);
            }
        }
        g_slist_free(matches);
        return suggest;
    }

    static gint
    ya_strcmp ( gconstpointer a, gconstpointer b) {
        return strcmp ((char *) a, (char *) b);
    }
    


    static glong 
    maxOptions(void){
        const gchar *env_maximum=getenv("XFFM_MAXIMUM_COMPLETION_OPTIONS");
        if (!env_maximum || !strlen(env_maximum)) return 104;
        errno=0;
        glong amount = strtol(env_maximum, (char **) NULL, 10);
        if (errno) return 104;
        return amount;
    }

    static gchar *
    top_match (GSList *matches){
        if (!matches || !g_slist_length(matches)) return NULL;
        char *suggest;
        if(g_slist_length (matches) == 1) {
            suggest = (gchar *) (matches->data);
            g_slist_free(matches);
            return suggest;
        }
        matches = g_slist_sort (matches, ya_strcmp);
        GSList *a;
        GSList *b;
        int length;
        suggest = g_strdup ((gchar *) (matches->data));
        int equal_length=strlen(suggest);
        for(a = matches; a && a->data; a = a->next) {
            TRACE ( "comparing a %s\n", (gchar *)a->data);
            for(b = a->next; b && b->data; b = b->next) {
                TRACE ( "comparing b %s\n", (gchar *)b->data);
                length=Util::length_equal_string(
                        (const gchar *)(a->data), (const gchar *)(b->data));
                if(length < equal_length) {
                    equal_length = length;
                }
                TRACE ("comparing %s to %s: length=%d\n", (gchar *)a->data, (gchar *)b->data, length);
            }
        }
        suggest[equal_length]=0;
        //TRACE ("string=%s, equal_length=%d, suggest=%s\n",
          //      (gchar *) (p->data), equal_length, suggest);
        for(a = matches; a && a->data; a = a->next) {
            g_free(a->data);
        } 
        g_slist_free(matches);
        return suggest;
    }


};
#if 10
class Bash{
public:
//    BashCompletion(GtkTextView *textView, GtkTextView *input){
/*    BashCompletion(void){
        textView_ = NULL;
        input_ = NULL;
    }
    void setCompletionTextView(GtkTextView *textView) {textView_ = textView;}
    void setCompletionInput(GtkTextView *input) {input_ = input;}
    GtkTextView *textView(void){ return textView_;}
    GtkTextView *input(void){ return input_;} */
    static gchar *
    get_text_to_cursor (GtkTextView *textview) {
        // get current text
        GtkTextIter start, end;
        auto buffer = gtk_text_view_get_buffer (textview);
        gint cursor_position;
        // cursor_position is a GtkTextBuffer internal property (read only)
        g_object_get (G_OBJECT (buffer), "cursor-position", &cursor_position, NULL);
        
        gtk_text_buffer_get_iter_at_offset (buffer, &start, 0);
        gtk_text_buffer_get_iter_at_offset (buffer, &end, cursor_position);
        auto t = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);
        g_strchug(t);
        TRACE ("lpterm_c::get_text_to_cursor: to cursor position=%d %s\n", cursor_position, t);
        return t;
    }

public:
    static void complete(GtkTextView *input, GtkTextView *output){
        auto child = GTK_WIDGET(g_object_get_data(G_OBJECT(input), "child"));

        auto workdir = Child::getWorkdir(child);
        gchar *head=get_text_to_cursor(input);
        g_strstrip(head);   
        if (head[0] == '$') { // Eliminate any preceeding $
          head[0] = ' ';
          g_strstrip(head);   
        }

        TRACE("bash_completion: %s\n", head);
        gint head_len = strlen(head);
        g_free (head);
        gchar *token = Util::inputText (input);
//        gchar *token = Util::get_current_text (input);
//
//        newToken will expand ~ and $HOME
        gchar *newToken = tokenExpand(input, token);
        if (newToken){
            g_free(token);
            token = newToken;
            Util::clear_text(input);
            Util::print(input, g_strdup_printf("%s",newToken));

            GtkTextIter end;
            GtkTextBuffer *buffer = gtk_text_view_get_buffer (input);
            gtk_text_buffer_get_end_iter(buffer, &end);
            gtk_text_buffer_place_cursor(buffer, &end);
            
            Util::flushGTK();
            return;
        }

        // Replace "sudo" with "sudo -A" to enable password dialog.
        if (strncmp(token, "sudo", strlen("sudo"))==0 &&
            strncmp(token, "sudo -A", strlen("sudo -A"))){
            gchar *o = token;
            gchar *oo = o+strlen("sudo");
            while (*oo == ' ') oo++;
            token = g_strconcat("sudo -A ", oo, NULL);
            g_free(o);
            head_len += strlen(" -A");
        }

        gint token_len = strlen(token);
        gchar *suggest = bash_suggestion(output, workdir, token, head_len);
        g_free (token);
        TRACE("bash_completion suggest= %s\n", suggest);
        if (suggest) {
            gint suggest_len = strlen(suggest);
            GtkTextIter end;
            GtkTextBuffer *buffer = gtk_text_view_get_buffer (input);
            gint offset = head_len + (suggest_len - token_len);
//            gint offset = head_len + (suggest_len - token_len) + 1;
            Util::clear_text(input);
            Util::print(input, g_strdup_printf("%s", suggest));
//            Util::print_status(input, g_strdup(suggest));
            gtk_text_buffer_get_iter_at_offset (buffer, &end, offset);
            gtk_text_buffer_place_cursor(buffer, &end);
        }
        g_free(suggest);

        gtk_text_view_set_cursor_visible(input, TRUE);
        return ;

    }

    static gchar *
    bash_suggestion(GtkTextView *output, const char *workdir, const gchar *in_token, gint token_len){
        TRACE("bash_suggestion\n");
        if (!valid_token(output, in_token)) return NULL;
        gchar *token = g_strdup(in_token);
        gchar *tail = NULL;
        if (strlen(in_token+token_len)) {
            tail = g_strdup(in_token + token_len);
            token[token_len] = 0;
        }
        gchar *suggest = bash_complete_with_head(output, workdir, token);
        //fprintf(stderr, "completion count = %d\n", matches);
        if (suggest){
            gint matchCount = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(output), "matchCount"));
            TRACE("got matchCount=%d\n", matchCount);
            if (suggest[strlen(suggest)-1] != '/' && !tail) {
                if (matchCount == 1){
                    tail = g_strdup(" ");
                }
            } 
            if (tail && strlen(tail)) {
                TRACE("bash_suggestion: suggest=\"%s\" tail=\"%s\"\n", suggest, tail);
                gchar *g = g_strconcat(suggest, tail, NULL);
                g_free(suggest);
                suggest = g;
            } 
        }
        g_free(token);
        g_free(tail);
        TRACE("retval, bash_suggestion: suggest=\"%s\"\n", suggest);
        return suggest;
    }


        static void bashCompletionDir(GtkTextView *input, GtkTextView *output, const char *workdir){
        TRACE("bashCompletionDir\n");
        gchar *head=get_text_to_cursor(input);
        gint head_len = strlen(head);
        g_free (head);
        gchar *token = Util::get_current_text (input);
        gchar *newToken = tokenExpand(input, token);
        if (newToken){
            g_free(token);
            token = newToken;
            Util::clear_text(input);
            Util::print(input, g_strdup(newToken));
            Util::flushGTK();
            return;
        }

        gint token_len = strlen(token);
        gchar *suggest = bash_suggestion(output, workdir, token, head_len);
        g_free (token);
        TRACE("bashCompletionDir; %s\n", suggest);
        if (suggest) {
            gint suggest_len = strlen(suggest);
            GtkTextIter end;
            GtkTextBuffer *buffer = gtk_text_view_get_buffer (input);
            gint offset = -1;
            // +1 is icon...
            offset = head_len + (suggest_len - token_len) + 1;
            Util::print_status(input, g_strdup(suggest));
            gtk_text_buffer_get_iter_at_offset (buffer, &end, offset);
            gtk_text_buffer_place_cursor(buffer, &end);
        }
        g_free(suggest);

        gtk_text_view_set_cursor_visible(input, TRUE);
        return ;

    }

        static void bash_completion(GtkTextView *input, GtkTextView *output, const char *workdir){
        TRACE("bash_completion\n");
        gchar *head=get_text_to_cursor(input);
        gint head_len = strlen(head);
        g_free (head);
        gchar *token = Util::get_current_text (input);
        gchar *newToken = tokenExpand(input, token);
        if (newToken){
            g_free(token);
            token = newToken;
            Util::clear_text(input);
            Util::print(input, g_strdup(newToken));
            Util::flushGTK();
            return;
        }

        if (strncmp(token, "sudo", strlen("sudo"))==0 &&
            strncmp(token, "sudo -A", strlen("sudo -A"))){
            gchar *o = token;
            gchar *oo = o+strlen("sudo");
            while (*oo == ' ') oo++;
            token = g_strconcat("sudo -A ", oo, NULL);
            g_free(o);
            head_len += strlen(" -A");
        }

        gint token_len = strlen(token);
        gchar *suggest = bash_suggestion(output, workdir, token, head_len);
        g_free (token);
        TRACE("bash_completion; %s\n", suggest);
        if (suggest) {
            gint suggest_len = strlen(suggest);
            GtkTextIter end;
            GtkTextBuffer *buffer = gtk_text_view_get_buffer (input);
            gint offset = -1;
            // +1 is icon...
            offset = head_len + (suggest_len - token_len) + 1;
            Util::print_status(input, g_strdup(suggest));
            gtk_text_buffer_get_iter_at_offset (buffer, &end, offset);
            gtk_text_buffer_place_cursor(buffer, &end);
        }
        g_free(suggest);

        gtk_text_view_set_cursor_visible(input, TRUE);
        return ;

    }

private:

 /*   gchar *
    file_completion(gchar *token){
        if (!token) {
            msg_help_text(output);
            return NULL;
        }
        if (token) g_strchug (token);
        if (strlen(token) == 0) {
            msg_help_text(output);
            return NULL;
        }
        gint match_count;
        return bash_file_completion(token, &match_count);
    }*/

    //   private completion methods
    
    static void
    msg_help_text(GtkTextView *output){
        if (!output) return;
        Util::showText(output);

        Util::print_icon(output, "dialog-info", "green", g_strdup_printf("%s bash %s/%s--> ",
                _("Completion mode:"), 
                _("command"),
                _("file")));
        Util::print(output,  "red", g_strdup("TAB.\n"));
        Util::scroll_to_bottom(output);
    }

    static void
    msg_result_text(GtkTextView *output, gint match_type){
        if (!output) return;
        Util::showText(output);
#ifdef DEBUG_TRACE
        Util::print_icon(output, "dialog-info", "green", g_strdup(_("Options >>")));
        const gchar *option_type = get_match_type_text(match_type);
        Util::print(output,  "red", g_strdup_printf("(%s)\n", option_type));
#else
        Util::print(output,  "green", g_strdup_printf("%s\n", _("Options >>")));
#endif
        Util::scroll_to_bottom(output);
    }

    static gchar *
    bash_file_completion(GtkTextView *output, const char *workdir, const char *in_file_token){
        gchar *suggest = BaseCompletion::base_file_completion(output, workdir, in_file_token);
        return suggest;
    }
public:
    static gchar *
    bash_exec_completion(GtkTextView *output, const char *workdir, const gchar *in_token){
        TRACE("bash_exec_completion for %s\n", in_token);
        gchar *suggest = BaseCompletion::base_exec_completion(output, workdir, in_token);

        TRACE( "complete_it: %s,  suggest=%s\n", in_token, suggest);
        if (!suggest) {
            suggest = BaseCompletion::base_file_completion(output, workdir, in_token);
            if (suggest) {
                gchar *d = g_strdup(workdir);
                gchar *absolute_suggest = g_build_filename(d, suggest, NULL);
                g_free(d);
                g_strchomp (absolute_suggest);
                TRACE("file absolute_suggest=%s (%s)\n", absolute_suggest, suggest);
                if (access(absolute_suggest, X_OK) != 0){
                    g_free(suggest);
                    suggest=NULL;
                    ERROR("access \"%s\": %s\n", absolute_suggest, strerror(errno));
                } 
                g_free(absolute_suggest);
            }
        }

        TRACE("suggest=%s\n", suggest);
        if (suggest && g_file_test(suggest, G_FILE_TEST_IS_DIR)){
            // only add slash if not already in suggest string...
            if (suggest[strlen(suggest)-1] != G_DIR_SEPARATOR) {
                gchar *s=g_strconcat(suggest, G_DIR_SEPARATOR_S, NULL);
                g_free(suggest);
                suggest = s;
            }
        }
        return suggest;
    }
private:
#define ALT_CHAR 13
    static gchar *
    bash_complete(GtkTextView *output, const char *workdir, const gchar *token){
        //
        // If we are dealing with file completion, then we have a head.
        TRACE("bash_complete\n");
        gchar *active_token = g_strdup(token);
        g_strchug(active_token);
        gchar *file_token = NULL;
        gchar *command_token = NULL;
        const gchar *head = NULL;

        gchar *suggest = NULL;

        // Obtain the file token separation, if any.
        gboolean esc_space;
        // Test for use of escaped spaces.
        if (strstr(token, "\\ ")){
            esc_space=TRUE;
            gchar *tmp_token = g_strdup(token);
            gint i,j; for(i=0,j=0; j<strlen(token); i++,j++){
                if (strncmp(token+j, "\\ ", 2) == 0){
                    tmp_token[i] = ALT_CHAR;
                    j++;
                } else {
                    tmp_token[i] = token[j];
                }
            }
            tmp_token[i] = 0;

            g_free(active_token);
            active_token = tmp_token;
        }



        {
            gchar *t[5];
            const gchar *token_char = {"=\t ><"};
            gint i;
            gboolean file_completion=FALSE;
            for (i=0; i < 5; i++) {
                t[i] = strrchr(active_token, token_char[i]);
                if (t[i]) file_completion = TRUE;
            }
            if (file_completion){
                if (t[0] > t[1] && t[0] > t[2] && t[0] > t[3] && t[0] > t[4]) i = 0;
                else if (t[1] > t[0] && t[1] > t[2] && t[1] > t[3] && t[1] > t[4]) i = 1;
                else if (t[2] > t[0] && t[2] > t[1] && t[2] > t[3] && t[2] > t[4]) i = 2;
                else if (t[3] > t[0] && t[3] > t[1] && t[3] > t[2] && t[3] > t[4]) i = 3;
                else if (t[4] > t[0] && t[4] > t[1] && t[4] > t[2] && t[4] > t[3]) i = 4;
                else g_error("should never happen");
                file_token = g_strdup(t[i]+1);
                *(t[i]+1) = 0;
                head = active_token;
            }
        } 

        if (!file_token) command_token = g_strdup(active_token);
        
        
        // Now that we have our token defined, we need to check
        // if the user is using quotes or not.
        const gchar *quote=NULL;
        const gchar *r = (file_token)?file_token:command_token;
        if (*r == '\'') quote="\'";
        if (*r == '\"') quote="\"";
        if (quote) {
            gchar *g = g_strdup(r+1);
            if (file_token) {
                g_free(file_token);
                file_token=g;
            } else {
                g_free(command_token);
                command_token=g;
            }
        }

        if (esc_space) {
            gchar *s = (file_token)?file_token:command_token;
            gint j; for(j=0; s && j<strlen(s); j++){
                if (s[j] == ALT_CHAR){
                    s[j] = ' ';
                } 
            }
        }
        if (head && esc_space) {
            gchar *g = (gchar *) malloc(2*strlen(head) + 1);
            if (!g) g_error("malloc: %s", strerror(errno));
            memset(g, 0, 2*strlen(head) + 1);
            gint i;
            gchar *p = active_token;
            for (i=0; p && *p; p++,i++){
                if (*p == ALT_CHAR) {
                    g[i++] = '\\';
                    g[i] = ' ';
                } else g[i] = *p;
            }
            g_free(active_token);
            active_token = g_strdup(g);
            g_free(g);
            head = active_token;
        }

        if (command_token) {
                suggest = bash_exec_completion(output, workdir, command_token);        
        }
        else if (file_token) {
                suggest = bash_file_completion(output, workdir, file_token);
        }

        if (suggest) {
          if (quote) {
            gchar *s;
            /*if (*matches_p==1 && suggest[strlen(suggest)-1] != '/') {
                s = g_strconcat (quote, suggest, quote, " ", NULL);
            } else */
            {
                s = g_strconcat (quote, suggest, NULL);
            }
            g_free(suggest);
            suggest=s;

          } else {
            if (strchr(suggest, ' ')){
                gchar **split = g_strsplit (suggest, " ", -1);
                gchar *s = g_strjoinv ("\\ ", split);
                g_free(suggest);
                suggest=s;
                g_strfreev(split);
            }
          }
        }
        if (head && suggest) {
            gchar *g = g_strconcat(head, suggest, NULL);
            g_free(suggest);
            suggest = g;
        }
        g_free(active_token);
        g_free(command_token);
        g_free(file_token);

        return suggest;
    }


    static gchar *
    bash_complete_with_head(GtkTextView *output, const char *workdir, const gchar *in_token){
        // Do we have a command separator?
        gboolean multiple_command = (strrchr(in_token, ';') || strrchr(in_token, '&') ||  strrchr(in_token, '|'));

        if (!multiple_command){
            TRACE("bash_complete_with_head: !multiple_command\n");
            return bash_complete(output, workdir, in_token);
        }
        gchar *token = g_strdup(in_token);
        gchar *t[3];
        const gchar *token_char={";&|"};
        gint i;
        for (i=0; i < 3; i++) t[i] = strrchr(token, token_char[i]);
        if (t[0] > t[1] && t[0] > t[2]) i = 0;
        else if (t[1] > t[0] && t[1] > t[2]) i = 1;
        else if (t[2] > t[0] && t[2] > t[1]) i = 2;
        else g_error("should never happen");
        gchar *p = g_strdup(t[i]+1);
        *(t[i]+1) = 0;
        gchar *suggest = bash_complete(output, workdir, g_strchug(p));
        g_free(p);
        if (suggest) {
            TRACE("bash_complete_with_head: token=\"%s\", suggest=\"%s\"\n",
                    token, suggest);
            gchar *g = g_strconcat(token, suggest, NULL);
            g_free(suggest);
            suggest=g;
        }
        g_free(token);
        return suggest;
    }

    static gboolean 
    valid_token(GtkTextView *output, const gchar *token){
        if (!token) {
            msg_help_text(output);
            return FALSE;
        }
        gchar *t = g_strdup(token);
        g_strchug (t);
        if (strlen(t) == 0) {
            g_free(t);
            msg_help_text(output);
            return FALSE;
        }
        g_free(t);
        return TRUE;
    }


    static gchar *tokenExpand(GtkTextView *input, const gchar *token){
       gchar *newToken = NULL;
       if (strchr(token,'~')){
           gchar *g = g_strdup(token);
           *(strchr(g, '~')) = 0;
           gchar *gg = g_strdup(strchr(token, '~') +1);
           newToken = g_strconcat(g, g_get_home_dir(), gg, NULL);
           g_free(g);
           g_free(gg);

       }
       else if (strchr(token,'$')){
           gchar *g = g_strdup(token);
           *(strchr(g, '$')) = 0;
            gchar *t = g_strdup(strchr(token, '$'));
            gint len = 0;
            for (gchar *tt = t; tt && *tt; tt++, len++){
                if (*tt == G_DIR_SEPARATOR || *tt == ' ') {
                    *tt = 0;
                    break;
                }
            }
            const gchar *e = getenv(t+1);
            if (e && strlen(e)){
                newToken = g_strconcat(g, e, token+len+strlen(g), NULL);
            }
            g_free(t);
            g_free(g);
       } 
       return newToken;
    }    



};

#endif
}


#endif

