#ifndef BASH_COMPLETION_HH
#define BASH_COMPLETION_HH
#include <unistd.h>
#include "base.hh"
        
extern char **environ;

namespace xf {


template <class Type>
class BashCompletion{
    using util_c = Util<double>;
    using print_c = Print<double>;
    using base_c = BaseCompletion<double>;
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

public:
    static void bash_completion(GtkTextView *input, GtkTextView *output, const char *workdir){
        TRACE("bash_completion\n");
        gchar *head=print_c::get_text_to_cursor(input);
        gint head_len = strlen(head);
        g_free (head);
        gchar *token = print_c::get_current_text (input);
        gchar *newToken = tokenExpand(input, token);
        if (newToken){
            g_free(token);
            token = newToken;
            print_c::clear_text(input);
            print_c::print(input, "red", g_strdup(newToken));
            while(gtk_events_pending())gtk_main_iteration();
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
            print_c::print_status(input, g_strdup(suggest));
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
            DBG("got matchCount=%d\n", matchCount);
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
        print_c::show_text(output);

        print_c::print_icon(output, "dialog-info", "green", g_strdup_printf("%s bash %s/%s--> ",
                _("Completion mode:"), 
                _("command"),
                _("file")));
        print_c::print(output,  ("red", g_strdup("TAB.")));
        print_c::scroll_to_bottom(output);
    }

    static void
    msg_result_text(GtkTextView *output, gint match_type){
	if (!output) return;
        print_c::show_text(output);
#ifdef DEBUG_TRACE
        print_c::print_icon(output, "dialog-info", "green", g_strdup(_("Options >>")));
        const gchar *option_type = get_match_type_text(match_type);
        print_c::print(output,  ("red", g_strdup_printf("(%s)\n", option_type));
#else
        print_c::print(output,  ("green", g_strdup_printf("%s\n", _("Options >>"))));
#endif
        print_c::scroll_to_bottom(output);
    }

#if 0
    static void
    msg_too_many_matches(GtkTextView *output){
	if (!output) return;
        print_c::show_text(output);
        gchar *message1=g_strdup_printf("%s (> %ld)",
                _("Too many matches"), base_c::maxOptions());
        print_c::print_icon(output, "dialog-info", "red", g_strdup_printf("%s\n", message1));
        g_free(message1);
        print_c::scroll_to_bottom(output);
    }

    static gchar *
    msg_output(GtkTextView *output, gint *match_count_p, GSList *matches, gint match_type){
        TRACE("msg_output\n");
        if (*match_count_p <= 0) {
            switch (*match_count_p) {
                case 0:
		    DBG("No match\n");
                    //msg_show_match(output, match_type, NULL); break;
                case -1: 
                    msg_too_many_matches(output); break;
                default:
                    ;
                    // invalid token        
            }
            return NULL;
        }
        gchar *suggest = complete_it(output, &matches, match_type);
        return suggest;
    }
#endif

    static gchar *
    bash_file_completion(GtkTextView *output, const char *workdir, const char *in_file_token){
        gchar *suggest = base_c::base_file_completion(output, workdir, in_file_token);
        return suggest;
    }

    static gchar *
    bash_exec_completion(GtkTextView *output, const char *workdir, const gchar *in_token){
        DBG("bash_exec_completion for %s\n", in_token);
        gchar *suggest = base_c::base_exec_completion(output, workdir, in_token);

        DBG( "complete_it: %s,  suggest=%s\n", in_token, suggest);
        if (!suggest) {
            suggest = base_c::base_file_completion(output, workdir, in_token);
            if (suggest) {
                gchar *d = g_strdup(workdir);
                gchar *absolute_suggest = g_build_filename(d, suggest, NULL);
                g_free(d);
                g_strchomp (absolute_suggest);
                DBG("file absolute_suggest=%s (%s)\n", absolute_suggest, suggest);
                if (access(absolute_suggest, X_OK) != 0){
                    g_free(suggest);
                    suggest=NULL;
                    WARN("access \"%s\": %s\n", absolute_suggest, strerror(errno));
                } 
                g_free(absolute_suggest);
            }
        }

        DBG("suggest=%s\n", suggest);
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
	    DBG("bash_complete_with_head: token=\"%s\", suggest=\"%s\"\n",
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
}


#endif

