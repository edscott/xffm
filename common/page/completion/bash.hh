#ifndef BASH_COMPLETION_HH
#define BASH_COMPLETION_HH
#include "base.hh"

namespace xf {


template <class Type>
class BashCompletion: protected BaseCompletion<Type>{
    using util_c = Util<double>;
    using print_c = Print<double>;
public:
//    BashCompletion(GtkTextView *textView, GtkTextView *input){
    BashCompletion(void){
        textView_ = NULL;
        input_ = NULL;
    }
    void setCompletionTextView(GtkTextView *textView) {textView_ = textView;}
    void setCompletionInput(GtkTextView *input) {input_ = input;}
    GtkTextView *textView(void){ return textView_;}
    GtkTextView *input(void){ return input_;}

protected:
    void bash_completion(void){
        gchar *head=print_c::get_text_to_cursor(textView());
        gint head_len = strlen(head);
        g_free (head);
        gchar *token = print_c::get_current_text (textView());

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
        gchar *suggest = bash_suggestion(token, head_len);
        g_free (token);

        if (suggest) {
            gint suggest_len = strlen(suggest);
            GtkTextIter end;
            GtkTextBuffer *buffer = gtk_text_view_get_buffer (input_);
            gint offset = -1;
            // +1 is icon...
            offset = head_len + (suggest_len - token_len) + 1;
            print_c::print_status(input_, g_strdup(suggest));
            gtk_text_buffer_get_iter_at_offset (buffer, &end, offset);
            gtk_text_buffer_place_cursor(buffer, &end);	
        }
        g_free(suggest);

        gtk_text_view_set_cursor_visible(input_, TRUE);
        return ;

    }

    gchar *
    bash_suggestion(const gchar *in_token, gint token_len){
        if (!valid_token(in_token)) return NULL;
        gchar *token = g_strdup(in_token);
        gchar *tail = NULL;
        if (token_len) {
            tail = g_strdup(in_token + token_len);
            token[token_len] = 0;
        }
        gint matches;
        gchar *suggest = bash_complete_with_head(token, &matches);
        //fprintf(stderr, "completion count = %d\n", matches);
        if (suggest){
            if (matches == 1 && suggest[strlen(suggest)-1] != '/') {
                gchar *g = g_strconcat(suggest, " ", NULL);
                g_free(suggest);
                suggest = g;
            } else if (tail) {
                gchar *g = g_strconcat(suggest, tail, NULL);
                g_free(suggest);
                suggest = g;
            } 
        }
        g_free(token);
        g_free(tail);
        return suggest;
    }
private:
    GtkTextView *textView_;
    GtkTextView *input_;

 /*   gchar *
    file_completion(gchar *token){
        if (!token) {
            msg_help_text();
            return NULL;
        }
        if (token) g_strchug (token);
        if (strlen(token) == 0) {
            msg_help_text();
            return NULL;
        }
        gint match_count;
        return bash_file_completion(token, &match_count);
    }*/

    //   private completion methods

    void
    msg_too_many_matches(void){
        print_c::show_text(textView());
        gchar *message1=g_strdup_printf("%s (> %ld)",
                _("Too many matches"), this->maxOptions());
        print_c::print_icon(textView(), "dialog-info", "tag/red", g_strdup_printf("%s\n", message1));
        g_free(message1);
        print_c::scroll_to_bottom(textView());
    }

    const gchar *
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
    
    void
    msg_show_match(gint match_type, const gchar *match){
        print_c::show_text(textView());
        if (!match) {
            const gchar *option_type = get_match_type_text(match_type);
            match = _("Found no match");
            print_c::print_icon(textView(), "dialog-warning", "tag/red", g_strdup_printf(" %s\n", match));
        } else {
            print_c::print(textView(), ("tag/blue", g_strdup_printf(" %s\n", match)));
        }
        print_c::scroll_to_bottom(textView());
    }

    void
    msg_help_text(void){
        print_c::show_text(textView());

        print_c::print_icon(textView(), "dialog-info", "tag/green", g_strdup_printf("%s bash %s/%s--> ",
                _("Completion mode:"), 
                _("command"),
                _("file")));
        print_c::print(textView(),  ("tag/red", g_strdup("TAB.")));
        print_c::scroll_to_bottom(textView());
    }

    void
    msg_result_text(gint match_type){
        print_c::show_text(textView());
#ifdef DEBUG_TRACE
        print_c::print_icon(textView(), "dialog-info", "tag/green", g_strdup(_("Options >>")));
        const gchar *option_type = get_match_type_text(match_type);
        print_c::print(textView(),  ("tag/red", g_strdup_printf("(%s)\n", option_type));
#else
        print_c::print(textView(),  ("tag/green", g_strdup_printf("%s\n", _("Options >>"))));
#endif
        print_c::scroll_to_bottom(textView());
    }

    gchar *
    list_matches (GSList **matches_p, gint match_type){
        *matches_p = g_slist_sort (*matches_p, this->ya_strcmp);
        GSList *a;
        GSList *b;
        int length;
        GSList *p = *matches_p;
        char *suggest = g_strdup ((gchar *) (p->data));
        int equal_length=strlen(suggest);
        for(a = *matches_p; a && a->data; a = a->next) {
            TRACE(stderr, "comparing a %s\n", (gchar *)a->data);
            for(b = a->next; b && b->data; b = b->next) {
                TRACE(stderr, "comparing b %s\n", (gchar *)b->data);
                length=util_c::length_equal_string(
                        (const gchar *)(a->data), (const gchar *)(b->data));
                if(length < equal_length) {
                    equal_length = length;
                }
                TRACE(stderr, "comparing %s to %s: length=%d\n", (gchar *)a->data, (gchar *)b->data, length);
            }
        }
        suggest[equal_length]=0;
        TRACE(stderr, "string=%s, equal_length=%d, suggest=%s\n",
                (gchar *) (p->data), equal_length, suggest);


        int i;
        for(i = 1, p = *matches_p; p && p->data; p = p->next, i++) {
            msg_show_match(match_type, (const gchar *)p->data);
        }
        return suggest;
    }

    gchar *
    complete_it(GSList **matches_p, gint match_type){
        if (!matches_p || !(*matches_p)) return NULL;
        gchar *suggest = NULL;
        TRACE( "COMPLETE: matches %d\n", g_slist_length (*matches_p));
        if(g_slist_length (*matches_p) == 1) {
            gchar *s =(gchar *)(*matches_p)->data;
            suggest = g_strdup (s);
        } else {
            print_c::show_text(textView());
            msg_result_text(match_type);
            suggest=this->top_match(matches_p);
            gint i;
            GSList *p;
            for(i = 1, p = *matches_p; p && p->data; p = p->next, i++) {
                msg_show_match(match_type, (const gchar *)p->data);
            }
        }
        return suggest;
    }

    gchar *
    msg_output(gint *match_count_p, GSList *matches, gint match_type){
        if (*match_count_p <= 0) {
            switch (*match_count_p) {
                case 0:
                    msg_show_match(match_type, NULL); break;
                case -1: 
                    msg_too_many_matches(); break;
                default:
                    ;
                    // invalid token        
            }
            return NULL;
        }
        gchar *suggest = complete_it(&matches, match_type);
        return suggest;
    }

    gchar *
    bash_file_completion(const char *in_file_token, gint *match_count_p){
        GSList *matches = this->base_file_completion(this->workdir(), in_file_token, match_count_p);
        gchar *suggest = msg_output(match_count_p, matches, MATCH_FILE);
        this->free_match_list(matches);
        return suggest;
    }

    gchar *
    bash_exec_completion(const gchar *in_token, gint *match_count_p){
        fprintf(stderr, "bash_exec_completion for %s\n", in_token);
        GSList *matches = this->base_exec_completion(this->workdir(), in_token, match_count_p);
        fprintf(stderr, "match count = %d\n", *match_count_p);
        gchar *suggest = msg_output(match_count_p, matches, MATCH_COMMAND);

        TRACE( "complete_it: MATCH_COMMAND=%d suggest=%s, matches=%d\n",
                MATCH_COMMAND, suggest, g_slist_length(matches));
        if (!suggest) {
            suggest = this->base_file_suggestion(this->workdir(), in_token, match_count_p);
            if (suggest) {

                gchar *d = g_strdup(this->workdir());
                gchar *absolute_suggest = g_build_filename(d, suggest, NULL);
                g_free(d);
                g_strchomp (absolute_suggest);
                TRACE(stderr, "file absolute_suggest=%s (%s)\n", absolute_suggest, suggest);
                if (access(absolute_suggest, X_OK) != 0){
                    g_free(suggest);
                    suggest=NULL;
                    TRACE(stderr, "access \"%s\": %s\n", absolute_suggest, strerror(errno));
                } 
                g_free(absolute_suggest);
            }
        }

        TRACE(stderr, "suggest=%s\n", suggest);
        if (*match_count_p==1 && g_file_test(suggest, G_FILE_TEST_IS_DIR)){
            gchar *s=g_strconcat(suggest, "/", NULL);
            g_free(suggest);
            suggest = s;
        }
        this->free_match_list(matches);
        return suggest;
    }



    gchar *
    variable_complete(const gchar *token, gint *match_p){
        // variable, starts with a $ (pending)
        extern char **environ;
        gchar **env = environ;
        GSList *envlist = NULL;
        for (; env && *env; env++){
            gchar *e = g_strdup_printf("$%s", *env);
            if (strchr(e, '=')) {
                *(strchr(e, '=')) = 0;
                envlist = g_slist_prepend (envlist, e);
            }
        }
        GSList *matches = NULL;
        GSList *list;
        for (list=envlist; list && list->data; list = list->next){
            if (strncmp(token, (gchar *)list->data, strlen(token))==0){
                matches = g_slist_prepend (matches, g_strdup((gchar *)list->data));
            } 
        }
        if (matches){
            *match_p = g_slist_length(matches);
        } else {
            *match_p = 0;
        }
        for (list=envlist; list && list->data; list = list->next){ g_free(list->data);}
        g_slist_free(envlist);
        gchar *suggest = complete_it(&matches, MATCH_VARIABLE);
        for (list=matches; list && list->data; list = list->next){ g_free(list->data);}
        g_slist_free(matches);
        return suggest;
    }


    gchar *
    userdir_complete(const gchar *token, gint *match_p){
        // username, starts with a ~ (pending)
        // Get username from passwd. If user has a shell, append a /
        // otherwise, append a space.
        // Function: 
        struct passwd *pw;
        GSList *pwlist = NULL;
        while((pw = getpwent ()) != NULL) {
            gboolean shell_ok = strcmp(pw->pw_shell, "/bin/false") && 
                g_file_test(pw->pw_shell, G_FILE_TEST_IS_EXECUTABLE);
            pwlist = g_slist_prepend (pwlist, g_strdup_printf ("~%s%s", 
                    pw->pw_name,
                    (shell_ok)?"/":""));
    // bug: space will be escaped on 2+ completion:	(shell_ok)?"/":" "));
        }
        endpwent ();
        GSList *matches = NULL;
        GSList *list;
        for (list=pwlist; list && list->data; list = list->next){
            if (strncmp(token, (gchar *)list->data, strlen(token))==0){
                matches = g_slist_prepend (matches, g_strdup((gchar *)list->data));
            } 
        }
        if (matches){
            *match_p = g_slist_length(matches);
        } else {
            *match_p = 0;
        }
        for (list=pwlist; list && list->data; list = list->next){ g_free(list->data);}
        g_slist_free(pwlist);
        gchar *suggest = complete_it(&matches, MATCH_USER);
        for (list=matches; list && list->data; list = list->next){ g_free(list->data);}
        g_slist_free(matches);
        return suggest;
    }

    gchar *
    hostname_complete(const gchar *token, gint *match_p){
        // hostname, starts with a @ (pending)
        // Get host from /etc/hosts
        GSList *hostlist = NULL;
        FILE *hosts=fopen("/etc/hosts", "r");
        if (hosts){
            gchar buffer[1024];
            memset (buffer, 0, 1024);
            while(fgets(buffer, 1023, hosts) && !feof(hosts)){
                g_strchug(buffer);
                if (*buffer == '#') continue;
                gchar *t[2];
                t[0] = strchr(buffer, '\t');
                t[1] = strchr(buffer, ' ');
                gchar *p=NULL;
                if (t[0] && t[1]) {
                    if (t[0] < t[1]) p = t[0];
                    else p = t[1];
                } else if (t[0]) p = t[0];
                else p = t[1];
                // chop
                if (p) {
                    if (strchr(p, '\n')) *strchr(p, '\n')=0;
                    g_strstrip(p);
                    hostlist = g_slist_prepend(hostlist, g_strdup_printf("@%s", p));
                }
            }
            fclose(hosts);
        }
        GSList *matches = NULL;
        GSList *list;
        for (list=hostlist; list && list->data; list = list->next){
            if (strncmp(token, (gchar *)list->data, strlen(token))==0){
                matches = g_slist_prepend (matches, g_strdup((gchar *)list->data));
            } 
        }
        if (matches){
            *match_p = g_slist_length(matches);
        } else {
            *match_p = 0;
        }
        for (list=hostlist; list && list->data; list = list->next){ g_free(list->data);}
        g_slist_free(hostlist);
        gchar *suggest = complete_it(&matches, MATCH_HOST);
        for (list=matches; list && list->data; list = list->next){ g_free(list->data);}
        g_slist_free(matches);
        return suggest;

        return NULL;
    }

    gchar *
    extra_completion(const gchar *token, gint *matches_p){
       if (*token == '$' || *token == '@' || *token == '~'){
            gchar *suggest = NULL;
            switch (*token){
                case '$':
                    suggest = variable_complete(token, matches_p);
                    break;
                case '~':
                    suggest = userdir_complete(token, matches_p);
                    break;
                case '@':
                    suggest = hostname_complete(token, matches_p);
                    break;
            }
            return suggest;
        }
       return NULL;
    }

    gchar *
    extra_space(gchar *suggest, gint *matches_p){
        gint loc = strlen(suggest) -1;
        if (loc < 0) loc = 0;
        if (*matches_p == 1 && suggest[loc] != '/'){
            gchar *g = g_strconcat(suggest, " ", NULL);
            g_free(suggest);
            suggest = g;
        }
        return suggest;
    }

#define ALT_CHAR 13
    gchar *
    bash_complete(const gchar *token, gint *matches_p){
        //
        // If we are dealing with file completion, then we have a head.

        gchar *active_token = g_strdup(token);
        g_strchug(active_token);
        gchar *file_token = NULL;
        gchar *command_token = NULL;
        const gchar *head = NULL;

        gchar *suggest = extra_completion(active_token, matches_p);
        if (suggest) {
            suggest = extra_space(suggest, matches_p);
            if (strcmp(suggest, active_token)==0){
                g_free(suggest);
                suggest = util_c::get_tilde_dir(active_token);
            }
            g_free(active_token);
            return suggest;
        }
     
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

        gboolean extra_completed = FALSE;
        if (command_token) {
            suggest = extra_completion(command_token, matches_p);
            TRACE( "extra_completion ...%s -> %s\n", command_token, suggest);
            if (suggest && strcmp(suggest, command_token)==0){
                g_free(suggest);
                suggest = util_c::get_tilde_dir(command_token);
                TRACE( "extra_completion ...%s -> %s\n", command_token, suggest);
            }
            if (!suggest) {
                suggest = bash_exec_completion(command_token, matches_p);	
                TRACE( "bash_exec_completion ...%s -> %s\n", command_token, suggest);
            } else extra_completed = TRUE;
        }
        else if (file_token) {
            TRACE( "bash_file_completion ...%s\n", file_token);
            suggest = extra_completion(file_token, matches_p);
            if (suggest && strcmp(suggest, file_token)==0){
                g_free(suggest);
                suggest = util_c::get_tilde_dir(file_token);
            }
            if (!suggest) {
                suggest = bash_file_completion(file_token, matches_p);
            }  else extra_completed = TRUE;
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
        // add final space for $, @ and ~ completion.
        if (extra_completed){
            suggest = extra_space(suggest, matches_p);
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


    gchar *
    bash_complete_with_head(const gchar *in_token, gint *matches_p){
        // Do we have a command separator?
        gboolean multiple_command = (strrchr(in_token, ';') || strrchr(in_token, '&') ||  strrchr(in_token, '|'));

        if (!multiple_command){
            return bash_complete(in_token, matches_p);
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
        gchar *suggest = bash_complete(g_strchug(p), matches_p);
        g_free(p);
        if (suggest) {
            gchar *g = g_strconcat(token, suggest, NULL);
            g_free(suggest);
            suggest=g;
        }
        g_free(token);
        return suggest;
    }

    gboolean 
    valid_token(const gchar *token){
        if (!token) {
            msg_help_text();
            return FALSE;
        }
        gchar *t = g_strdup(token);
        g_strchug (t);
        if (strlen(t) == 0) {
            g_free(t);
            msg_help_text();
            return FALSE;
        }
        g_free(t);
        return TRUE;
    }



};
}


#endif

