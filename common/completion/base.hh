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

public:
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
protected:
    static gchar *
    get_token(const char *in_token, gint *match_count_p){
        if (!in_token) {
            *match_count_p = -2; // invalid token
            return NULL;
        }
        if (strlen(in_token) == 0) {
            *match_count_p = -2; // invalid token
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

    static GSList *
    base_file_completion(const gchar *workdir, const char *in_file_token, gint *match_count_p){
        
        gchar *file_token = get_token(in_file_token, match_count_p);
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

        *match_count_p = 0;
        if (stack_glob_v.gl_pathc > maxOptions()){
            //msg_too_many_matches();
            *match_count_p = -1;
        } else if (stack_glob_v.gl_pathc == 0){
            TRACE(stderr, "NO MATCHES\n");
                //msg_show_match(MATCH_FILE, NULL);
        } else {
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
                (*match_count_p)++;
            }
        } 
        g_free(relative_directory);
        globfree(&stack_glob_v);
        g_free(file_token);
       
        // Quick exit:
        if (*match_count_p <= 0){
            return NULL;
        }  
        return matches;
    }

    static GSList *
    base_exec_completion(const gchar *workdir, const char *in_token, gint *match_count_p){
        GSList *matches=NULL;

        gchar *token=get_token(in_token, match_count_p);
            
        glob_t stack_glob_v;
        gboolean straight_path = g_path_is_absolute(token) ||
            strncmp(token, "./", strlen("./"))==0 ||
            strncmp(token, "../", strlen("../"))==0;

        if (straight_path) {
            gchar *d;
            d = g_strdup(workdir);
            if (chdir(d) < 0){ 
                DBG("chdir %s\n",d);
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
                glob(directory, flags, NULL, &stack_glob_v);
                g_free(directory);
                if (stack_glob_v.gl_pathc > maxOptions()) break;
            }
            g_strfreev(path_pp);
            g_free(path_v);
        }

        if (stack_glob_v.gl_pathc > maxOptions()){
            //msg_too_many_matches();
            *match_count_p = -1;
        } else if (stack_glob_v.gl_pathc == 0){
            TRACE(stderr, "NO MATCHES\n");
            *match_count_p = 0;

                //msg_show_match(MATCH_FILE, NULL);
        } else {
            struct stat st;
            gint i;
            for (i=0; i<stack_glob_v.gl_pathc; i++){
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
                }
            }
            *match_count_p = g_slist_length(matches);
        } 

        globfree(&stack_glob_v);
        g_free(token);
        // Quick exit:
        if (*match_count_p <= 0){
            return NULL;
        }  
        return matches;
    }


    static gchar *
    base_suggestion(gint which, const gchar *workdir, const char *in_file_token, gint *match_count_p){
        GSList *list;
        if (which == MATCH_FILE)
            list = base_file_completion(workdir, in_file_token, match_count_p);
        else if (which == MATCH_COMMAND)
            list = base_exec_completion(workdir, in_file_token, match_count_p);
        else {
            fprintf(stderr, "unknown match type: %d\n", which);
            return NULL;
        }

        if (!list) return NULL;
        gchar *suggest = NULL;
        fprintf(stderr, "COMPLETE: matches %d\n", g_slist_length (list));
        if(g_slist_length (list) == 1) {
            gchar *s =(gchar *)list->data;
            suggest = g_strdup (s);
        } else {
            suggest=top_match(&list);
        }

        GSList *p = list;
        for (;p && p->data; p= p->next) g_free(p->data);
        g_slist_free(list);
        return suggest;
    }


    static gchar *
    base_file_suggestion(const gchar *workdir, const char *in_file_token, gint *match_count_p){
        return base_suggestion(MATCH_FILE, workdir, in_file_token, match_count_p);
    }

    // XXX FIXME: this basically duplicates the above...
    static gchar *
    base_exec_suggestion(const gchar *workdir, const char *in_file_token, gint *match_count_p){
        return base_suggestion(MATCH_COMMAND, workdir, in_file_token, match_count_p);
    }

    static void
    free_match_list(GSList *matches){
        GSList *p=matches;
        for (;p && p->data; p=p->next) g_free(p->data);
        g_slist_free (matches);
        return;

    }

    static gint
    ya_strcmp ( gconstpointer a, gconstpointer b) {
        return strcmp ((char *) a, (char *) b);
    }

    static gchar *
    top_match (GSList **matches_p){
        *matches_p = g_slist_sort (*matches_p, ya_strcmp);
        GSList *a;
        GSList *b;
        int length;
        GSList *p = *matches_p;
        char *suggest = g_strdup ((gchar *) (p->data));
        int equal_length=strlen(suggest);
        for(a = *matches_p; a && a->data; a = a->next) {
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
     /*   int i;
        for(i = 1, p = *matches_p; p && p->data; p = p->next, i++) {
            msg_show_match(match_type, (const gchar *)p->data);
        }*/
        return suggest;
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


};
}


#endif
