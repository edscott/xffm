#ifndef BASE_COMPLETION_HH
#define BASE_COMPLETION_HH
#include <sys/types.h>
#include <pwd.h>
#include <glob.h>
#include <unistd.h>

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

template <class Type>
class BaseCompletion {
    using util_c = Util<double>;
    using print_c = Print<double>;
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
        print_c::show_text(output);
        if (!match) {
            const gchar *option_type = get_match_type_text(match_type);
            match = _("Found no match");
            print_c::print_icon(output, "dialog-warning", "red", g_strdup_printf(" %s\n", match));
        } else {
            print_c::print(output, ("blue/white_bg", g_strdup_printf(" %s\n", match)));
        }
        print_c::scroll_to_bottom(output);
    }

    

    /*BaseCompletion(void){
        workdir_ = NULL;
    }
    gchar *workdir(void){ return workdir_;}
    void setWorkdir(const gchar *workdir){
        g_free(workdir_);
        workdir = g_strdup(workdir);
    }
private:
    gchar *workdir_;*/
    
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
                gchar *dir = util_c::get_tilde_dir(in_token);
                if (dir) token = g_strconcat(dir, strchr(in_token, '/')+1, NULL);
                g_free(dir);
            }
        } 
        if (!token) {
            token = g_strdup(in_token);
        }
        return token;
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
        TRACE(stderr, "file_token=%s\ndirectory=%s\n", file_token, directory);
        glob_t stack_glob_v;
        gint flags=GLOB_NOESCAPE;

        glob(directory, flags, NULL, &stack_glob_v);
        g_free(directory);

        if (stack_glob_v.gl_pathc > maxOptions()){
            print_c::print(output, "blue/white_bg",g_strdup_printf("%s ", file_token));
            print_c::print_error(output, g_strdup_printf(_("More than %ld matches\n"), maxOptions()));
            globfree(&stack_glob_v);
            g_free(file_token);
            return NULL;
        } else if (stack_glob_v.gl_pathc == 0){
            TRACE(stderr, "NO MATCHES\n");
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
            
        glob_t stack_glob_v;
        gboolean straight_path = g_path_is_absolute(token) ||
            strncmp(token, "./", strlen("./"))==0 ||
            strncmp(token, "../", strlen("../"))==0;

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
           // print_c::print(output, "Red/white_bg",g_strdup_printf("%s ", token));
           // print_c::print_error(output, g_strdup_printf(_("More than %d matches\n"), maxOptions()));

            globfree(&stack_glob_v);
            g_free(token);
            return NULL;
            //msg_too_many_matches();
            //*match_count_p = -1;
        } else if (stack_glob_v.gl_pathc == 0){
            DBG("NO MATCHES\n");
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
            print_c::print(output, "blue/white_bg", g_strconcat(_("Options:"), "\n",NULL));
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
            TRACE (stderr,  "comparing a %s\n", (gchar *)a->data);
            for(b = a->next; b && b->data; b = b->next) {
                TRACE (stderr,  "comparing b %s\n", (gchar *)b->data);
                length=util_c::length_equal_string(
                        (const gchar *)(a->data), (const gchar *)(b->data));
                if(length < equal_length) {
                    equal_length = length;
                }
                TRACE (stderr, "comparing %s to %s: length=%d\n", (gchar *)a->data, (gchar *)b->data, length);
            }
        }
        suggest[equal_length]=0;
        TRACE (stderr, "string=%s, equal_length=%d, suggest=%s\n",
                (gchar *) (p->data), equal_length, suggest);
        for(a = matches; a && a->data; a = a->next) {
            g_free(a->data);
        } 
        g_slist_free(matches);
        return suggest;
    }


#if 0
    static gchar *
    base_suggestion(GtkTextView *output, gint which, const gchar *workdir, const char *in_file_token){
        gchar *suggest;
        if (which == MATCH_FILE)
            suggest = base_file_completion(output, workdir, in_file_token);
        else if (which == MATCH_COMMAND)
            suggest = base_exec_completion(output, workdir, in_file_token);
        else {
            DBG( "unknown match type: %d\n", which);
            return NULL;
        }

        if (!suggest) return NULL;
        TRACE("base:: COMPLETE: matches %d\n", g_slist_length (list));
        if(g_slist_length (list) == 1) {
            gchar *s =(gchar *)list->data;
            suggest = g_strdup (s);
        } else {
            suggest=top_match(&list);
        }
        return suggest;
    }


    static gchar *
    base_file_suggestion(GtkTextView *output, const gchar *workdir, const char *in_file_token){
        TRACE("base_file_suggestion\n");
        return base_suggestion(output, MATCH_FILE, workdir, in_file_token);
    }

    static gchar *
    base_exec_suggestion(GtkTextView *output, const gchar *workdir, const char *in_file_token){
        TRACE("base_exec_suggestion\n");
        return base_suggestion(output, MATCH_COMMAND, workdir, in_file_token);
    }

    static void
    free_match_list(GSList *matches){
        TRACE("free_match_list\n");
        GSList *p=matches;
        for (;p && p->data; p=p->next) g_free(p->data);
        g_slist_free (matches);
        return;

    }
#endif


};
}


#endif
