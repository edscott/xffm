#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <pwd.h>
#include <glob.h>
#include <unistd.h>
#include <gtk/gtk.h>

#include "print_c.hpp"
#include "view_c.hpp"

enum {
    MATCH_COMMAND,
    MATCH_FILE,
    MATCH_HISTORY,
    MATCH_USER,
    MATCH_VARIABLE,
    MATCH_HOST,
    MATCH_NONE
};

/////////////////////////////////////////////////////////////////////////////
//              static thread functions                                    //
////////////////////////////////////////////////////////////////////////////
static void *print_f(void *);
static void *print_i(void *);
static void *print_d(void *);
static void *print_e(void *);
static void *print_s(void *);
static void *print_sl(void *);
static void *clear_text_buffer_f(void *);

////////////////////////////////////////////////////////////////////////////
//                       print_c class methods                            //
///////////////////////////////////////////////////////////////////////////

print_c::print_c(void *data){
    view_v = data;
    view_c *view_p = (view_c *)data;
    gtk_p = view_p->get_gtk_p();
    status = GTK_TEXT_VIEW(view_p->get_status());
    diagnostics = GTK_TEXT_VIEW(view_p->get_diagnostics());
    status_label = GTK_LABEL(view_p->get_status_label());
}

GtkTextView *
print_c::get_diagnostics(void){ 
    view_c *view_p = (view_c *)view_v;
    return GTK_TEXT_VIEW(view_p->get_diagnostics());
}
GtkTextView *
print_c::get_status(void){ 
    view_c *view_p = (view_c *)view_v;
    return GTK_TEXT_VIEW(view_p->get_status());
}
GtkLabel *
print_c::get_status_label(void){ 
    view_c *view_p = (view_c *)view_v;
    return GTK_LABEL(view_p->get_status_label());
} 


void print_c::print_error( const gchar *format, ...){
    if (!diagnostics) return;
    va_list var_args;
    va_start (var_args, format);
    gchar *string = g_strdup_vprintf(format, var_args);
    va_end (var_args);

    void *arg[]={(void *)this, (void *)"tag/bold", (void *)string};
    context_function(print_e, arg);
    g_free(string);

}

void print_c::print_debug(const gchar *format, ...){
    if (!diagnostics) return;
    va_list var_args;
    va_start (var_args, format);
    gchar *string = g_strdup_vprintf(format, var_args);
    va_end (var_args);

    void *arg[]={(void *)this, (void *)"tag/italic", (void *)string};
    context_function(print_d, arg);
    g_free(string);

}


void print_c::print(const gchar *format, ...){
    if (!diagnostics) return;
    va_list var_args;
    va_start (var_args, format);
    gchar *string = g_strdup_vprintf(format, var_args);
    va_end (var_args);

    void *arg[]={(void *)this, NULL, (void *)string};
    context_function(print_f, arg);
    g_free(string);
}

void print_c::print_tag(const gchar *tag, const gchar *format, ...){
    if (!diagnostics) return;
    va_list var_args;
    va_start (var_args, format);
    gchar *string = g_strdup_vprintf(format, var_args);
    va_end (var_args);

    void *arg[]={(void *)this, (void *)tag, (void *)string};
    context_function(print_f, arg);
    g_free(string);
}

void print_c::print_icon(const gchar *iconname, 
			     const gchar *format, ...)
{
    if (!diagnostics) return;
    va_list var_args;
    va_start (var_args, format);
    gchar *string = g_strdup_vprintf(format, var_args);
    va_end (var_args);

    GdkPixbuf *pixbuf = gtk_p->get_pixbuf(iconname, -16);
    void *arg[]={(void *)pixbuf, (void *)this, NULL, (void *)string};
    context_function(print_i, arg);
    g_free(string);
}


void print_c::print_icon_tag(const gchar *iconname, 
	                     const gchar *tag, 
			     const gchar *format, ...)
{
    if (!diagnostics) return;
    va_list var_args;
    va_start (var_args, format);
    gchar *string = g_strdup_vprintf(format, var_args);
    va_end (var_args);

    GdkPixbuf *pixbuf = gtk_p->get_pixbuf(iconname, -16);
    void *arg[]={(void *)pixbuf, (void *)this, (void *)tag, (void *)string};
    context_function(print_i, arg);
    g_free(string);
}

void print_c::print_status(const gchar *format, ...){
    va_list var_args;
    va_start (var_args, format);
    gchar *string = g_strdup_vprintf(format, var_args);
    va_end (var_args);

    void *arg[]={(void *)this, (void *)string};
    context_function(print_s, arg);
    g_free(string);
}


void print_c::print_status_label(const gchar *format, ...){
    va_list var_args;
    va_start (var_args, format);
    gchar *string = g_strdup_vprintf(format, var_args);
    va_end (var_args);
    if (strstr(string, "\n")) *(strstr(string, "\n")) = 0;

    void *arg[]={(void *)status_label, (void *)string};
    context_function(print_sl, arg);
    g_free(string);
}


void print_c::show_text(void){
    view_c *view_p = (view_c *)view_v;
    view_p->show_diagnostics();
}

void print_c::clear_text(void){
    view_c *view_p = (view_c *)view_v;
    view_p->clear_diagnostics();
}


void *
print_c::scroll_to_top(void){
    // make sure all text is written before attempting scroll
    while (gtk_events_pending()) gtk_main_iteration();
    GtkTextMark *mark;
    GtkTextIter start, end;
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer (diagnostics);
    gtk_text_buffer_get_bounds (buffer, &start, &end);
    mark = gtk_text_buffer_create_mark (buffer, "scrollmark", &start, FALSE);
    gtk_text_view_scroll_to_mark (diagnostics, mark, 0.2,    /*gdouble within_margin, */
                                  TRUE, 0.0, 0.0);
    gtk_text_buffer_delete_mark (buffer, mark);
    return NULL;
}

void *
print_c::scroll_to_bottom(void){
    // make sure all text is written before attempting scroll
    while (gtk_events_pending()) gtk_main_iteration();
    GtkTextIter start, end;
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer (diagnostics);
    gtk_text_buffer_get_bounds (buffer, &start, &end);
    GtkTextMark *mark = gtk_text_buffer_create_mark (buffer, "scrolldown", &end, FALSE);
    gtk_text_view_scroll_to_mark (diagnostics, mark, 0.2,    /*gdouble within_margin, */
                                  TRUE, 1.0, 1.0);
    //gtk_text_view_scroll_mark_onscreen (diagnostics, mark);
    gtk_text_buffer_delete_mark(buffer, mark);
    return NULL;
}


////////////////////////////////////////////////////////////////////
////////////////////// completion ///////////////////
///////////////////////////////////////////////////////////////////


static gint
ya_strcmp (
    gconstpointer a,
    gconstpointer b
) {
    return strcmp ((char *) a, (char *) b);
}


static int
length_equal_string(const gchar *a, const gchar *b){
    int length=0;
    int i;
    for (i = 0; i < strlen(a) && i < strlen(b); i++){
	if (strncmp(a,b,i+1)) {
	    length=i;
	    break;
	} else {
	    length=i+1;
	}
    }
     NOOP(stderr, "%s --- %s differ at length =%d\n", a,b,length);
   return length;
}

gchar *
print_c::suggest_bash_complete(const gchar *in_token, gint token_len){
    if (!valid_token(in_token)) return NULL;
    gchar *token = g_strdup(in_token);
    gchar *tail = NULL;
    if (token_len) {
	tail = g_strdup(in_token + token_len);
	token[token_len] = 0;
    }
    gint matches;
    gchar *suggest = bash_complete_with_head(token, &matches);
    if (suggest){
	if (tail) {
	    gchar *g = g_strconcat(suggest, tail, NULL);
	    g_free(suggest);
	    suggest = g;
	} else if (matches == 1 && suggest[strlen(suggest)-1] != '/') {
	    gchar *g = g_strconcat(suggest, " ", NULL);
	    g_free(suggest);
	    suggest = g;
	}
    }
    g_free(token);
    g_free(tail);
    return suggest;
}

gchar *
print_c::file_completion(gchar *token){
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
}

gchar *
print_c::get_tilde_dir(const gchar *token){
    struct passwd *pw;
    gchar *tilde_dir = NULL;
    while((pw = getpwent ()) != NULL) {
	gchar *id = g_strdup_printf("~%s/", pw->pw_name);
	if (strncmp(token, id, strlen(id))==0){
	    tilde_dir = g_strdup_printf("%s/", pw->pw_dir);
	    g_free(id);
	    break;
	}
	g_free(id);
    }
    endpwent ();
    return tilde_dir;
}

//   private completion methods

#define BASH_COMPLETION_OPTIONS maximum_completion_options()
glong 
print_c::maximum_completion_options(void){
    const gchar *env_maximum=getenv("RFM_MAXIMUM_COMPLETION_OPTIONS");
    if (!env_maximum || !strlen(env_maximum)) return 104;
    errno=0;
    glong amount = strtol(env_maximum, (char **) NULL, 10);
    if (errno) return 104;
    return amount;
}


void
print_c::msg_too_many_matches(void){
    show_text();
    gchar *message1=g_strdup_printf("%s (> %ld)",
	    _("Too many matches"), BASH_COMPLETION_OPTIONS);
    gchar *message2=g_strdup_printf("%s %s", _("Options:"), message1);
    print_icon_tag("dialog-info", "tag/blue", "[%s]", _("Text Completion"));
    print_tag("tag/red", "%s\n", message2);
    g_free(message1);
    g_free(message2);
    scroll_to_bottom();
}

const gchar *
print_c::get_match_type_text(gint match_type){
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
print_c::msg_show_match(gint match_type, const gchar *match){
    if (!match) {
	const gchar *option_type = get_match_type_text(match_type);
	print_icon_tag ("dialog-warning", "xffm_tag/red", "(%s)", option_type);
	match = _("Found no match");
    } 
    print_tag("tag/blue", " %s\n", match);
    scroll_to_bottom();
}

void
print_c::msg_help_text(void){
    show_text();

    print_icon_tag ("dialog-info", "tag/green", "%s bash %s/%s--> ",
	    _("Completion mode:"), 
	    _("command"),
            _("file"));
    print_tag ("tag/red", "TAB.");
    scroll_to_bottom();
}

void
print_c::msg_result_text(gint match_type){
    show_text();
    print_icon_tag ("dialog-info", "tag/green", _("Options >>"));
    const gchar *option_type = get_match_type_text(match_type);
    print_tag ("tag/red", "(%s)\n", option_type);
    scroll_to_bottom();
}

const gchar *
print_c::get_workdir(void){
    view_c *view_p = (view_c *)view_v;
    return view_p->get_path();
}

gchar *
print_c::list_matches (GSList **matches_p, gint match_type){
    *matches_p = g_slist_sort (*matches_p, ya_strcmp);
    GSList *a;
    GSList *b;
    int length;
    GSList *p = *matches_p;
    char *suggest = g_strdup ((gchar *) (p->data));
    int equal_length=strlen(suggest);
    for(a = *matches_p; a && a->data; a = a->next) {
	NOOP(stderr, "comparing a %s\n", (gchar *)a->data);
	for(b = a->next; b && b->data; b = b->next) {
	    NOOP(stderr, "comparing b %s\n", (gchar *)b->data);
	    length=length_equal_string(
		    (const gchar *)(a->data), (const gchar *)(b->data));
	    if(length < equal_length) {
		equal_length = length;
	    }
	    NOOP(stderr, "comparing %s to %s: length=%d\n", (gchar *)a->data, (gchar *)b->data, length);
	}
    }
    suggest[equal_length]=0;
    NOOP(stderr, "string=%s, equal_length=%d, suggest=%s\n",
	    (gchar *) (p->data), equal_length, suggest);


    int i;
    for(i = 1, p = *matches_p; p && p->data; p = p->next, i++) {
	msg_show_match(match_type, (const gchar *)p->data);
    }
    return suggest;
}

gchar *
print_c::complete_it(GSList **matches_p, gint match_type){
        gchar *suggest = NULL;
        if(*matches_p) {
            NOOP( "COMPLETE: matches %d\n", g_slist_length (*matches_p));
            if(g_slist_length (*matches_p) == 1) {
                gchar *s =(gchar *)(*matches_p)->data;
                suggest = g_strdup (s);
            } else {
		show_text();
		msg_result_text(match_type);
		suggest=list_matches(matches_p, match_type);
            }
        }
	return suggest;
}

gchar *
print_c::bash_file_completion(const char *in_file_token, gint *match_count_p){
    if (!in_file_token) return NULL;
    if (strlen(in_file_token) == 0) return NULL;

    gchar *file_token=NULL;
    if (*in_file_token == '~' && strchr(in_file_token, '/')){
	if (strncmp(in_file_token, "~/", strlen("~/"))==0){
	    file_token = g_strconcat(g_get_home_dir(), in_file_token+1, NULL);
	} else {
	    gchar *dir = get_tilde_dir(in_file_token);
	    if (dir) file_token = g_strconcat(dir, strchr(in_file_token, '/')+1, NULL);
	    g_free(dir);
	}
    } 
    if (!file_token) {
	file_token = g_strdup(in_file_token);
    }
    
  
    GSList *matches=NULL;


    gchar *directory;
    gchar *relative_directory=NULL;
    if (g_path_is_absolute(file_token)) {
	directory = g_strdup_printf("%s*", file_token);

    } else {
	directory = g_strdup_printf("%s/%s*", get_workdir(), file_token);
	relative_directory = g_path_get_dirname(file_token);
	if (strcmp(relative_directory, ".")==0 && 
		strncmp(file_token, "./", strlen("./")) != 0) {
	    g_free(relative_directory);
	    relative_directory=NULL;
	}
    }
    
    NOOP(stderr, "file_token=%s\ndirectory=%s\n", file_token, directory);

    
    
    glob_t stack_glob_v;
    gint flags=GLOB_NOESCAPE;

    glob(directory, flags, NULL, &stack_glob_v);
    g_free(directory);

    if (stack_glob_v.gl_pathc == 0){
	NOOP(stderr, "NO MATCHES\n");
	    msg_show_match(MATCH_FILE, NULL);
    } else if (stack_glob_v.gl_pathc <= BASH_COMPLETION_OPTIONS){
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
    } else {
	msg_too_many_matches();
    }

	    
    g_free(relative_directory);
    globfree(&stack_glob_v);
    gchar *suggest = complete_it(&matches, MATCH_FILE);
    NOOP(stderr, "match = \"%s\"\n",suggest? suggest : "no match!");


    if (suggest) {
	*match_count_p = g_slist_length(matches);
    } else *match_count_p = 0;

    GSList *p=matches;
    for (;p && p->data; p=p->next){
	g_free(p->data);
    }
    g_slist_free (matches);

    g_free(file_token);
    return suggest;
}


gchar *
print_c::bash_exec_completion(const char *in_token, gint *match_count_p){
    GSList *matches=NULL;

    gchar *token=NULL;
    if (*in_token == '~' && strchr(in_token, '/')){
	if (strncmp(in_token, "~/", strlen("~/"))==0){
	    token = g_strconcat(g_get_home_dir(), in_token+1, NULL);
	} else {
	    gchar *dir = get_tilde_dir(in_token);
	    if (dir) token = g_strconcat(dir, strchr(in_token, '/')+1, NULL);
	    g_free(dir);
	}
    } 
    if (!token) {
	token = g_strdup(in_token);
    }
	
    glob_t stack_glob_v;
    gboolean straight_path = g_path_is_absolute(token) ||
	strncmp(token, "./", strlen("./"))==0 ||
	strncmp(token, "../", strlen("../"))==0;

    if (straight_path) {
	gchar *d;
	d = g_strdup(get_workdir());
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
	//gchar **path_pp = rfm_split(path_v,':');
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
	    if (stack_glob_v.gl_pathc > BASH_COMPLETION_OPTIONS) break;
	}
	g_strfreev(path_pp);
	g_free(path_v);
    }

    if (stack_glob_v.gl_pathc == 0){
	    msg_show_match(MATCH_COMMAND, NULL);
    } else if (stack_glob_v.gl_pathc <= BASH_COMPLETION_OPTIONS){
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
    } else {
	msg_too_many_matches();
    }

    if (chdir(g_get_home_dir()) < 0){ DBG("chdir %s\n",g_get_home_dir());}

    globfree(&stack_glob_v);


    gchar *suggest = complete_it(&matches, MATCH_COMMAND);
    NOOP( "complete_it: MATCH_COMMAND=%d suggest=%s, matches=%d\n",
	    MATCH_COMMAND, suggest, g_slist_length(matches));
    if (!suggest) {
        suggest = bash_file_completion(token, match_count_p);
	if (suggest) {

	    gchar *d = g_strdup(get_workdir());
	    gchar *absolute_suggest = g_build_filename(d, suggest, NULL);
	    g_free(d);
	    g_strchomp (absolute_suggest);
	    NOOP(stderr, "file absolute_suggest=%s (%s)\n", absolute_suggest, suggest);
	    if (access(absolute_suggest, X_OK) != 0){
		g_free(suggest);
		suggest=NULL;
		NOOP(stderr, "access \"%s\": %s\n", absolute_suggest, strerror(errno));
	    } 
	    g_free(absolute_suggest);
	}
    }
    NOOP(stderr, "suggest=%s\n", suggest);
    *match_count_p = g_slist_length(matches);
   if (g_slist_length(matches)==1 && g_file_test(suggest, G_FILE_TEST_IS_DIR)){
	gchar *s=g_strconcat(suggest, "/", NULL);
	g_free(suggest);
	suggest = s;
    }

    GSList *p=matches;
    for (;p && p->data; p=p->next){
	g_free(p->data);
    }
    g_slist_free (matches);

    g_free(token);
    return suggest;

}


gchar *
print_c::variable_complete(const gchar *token, gint *match_p){
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
print_c::userdir_complete(const gchar *token, gint *match_p){
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
print_c::hostname_complete(const gchar *token, gint *match_p){
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
print_c::extra_completion(const gchar *token, gint *matches_p){
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
print_c::extra_space(gchar *suggest, gint *matches_p){
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
print_c::bash_complete(const gchar *token, gint *matches_p){
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
	    suggest = get_tilde_dir(active_token);
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
	NOOP( "extra_completion ...%s -> %s\n", command_token, suggest);
	if (suggest && strcmp(suggest, command_token)==0){
	    g_free(suggest);
	    suggest = get_tilde_dir(command_token);
	    NOOP( "extra_completion ...%s -> %s\n", command_token, suggest);
	}
	if (!suggest) {
	    suggest = bash_exec_completion(command_token, matches_p);	
	    NOOP( "bash_exec_completion ...%s -> %s\n", command_token, suggest);
	} else extra_completed = TRUE;
    }
    else if (file_token) {
	NOOP( "bash_file_completion ...%s\n", file_token);
	suggest = extra_completion(file_token, matches_p);
	if (suggest && strcmp(suggest, file_token)==0){
	    g_free(suggest);
	    suggest = get_tilde_dir(file_token);
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
print_c::bash_complete_with_head(const gchar *in_token, gint *matches_p){
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
print_c::valid_token(const gchar *token){
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


//////////////////////////////////////////////////////////////////////
//                             print.i                              //
//////////////////////////////////////////////////////////////////////



typedef struct sequence_t {
    const gchar *id;
    const gchar *sequence;
} sequence_t;


static sequence_t sequence_v[] = {
        {"tag/black", "30"},
        {"tag/black_bg", "40"},
        {"tag/red", "31"},
        {"tag/red_bg", "41"},
        {"tag/green", "32"},
        {"tag/green_bg", "42"},
        {"tag/yellow", "33"},
        {"tag/yellow_bg", "43"},
        {"tag/blue", "34"},
        {"tag/blue_bg", "44"},
        {"tag/magenta", "35"},
        {"tag/magenta_bg", "45"},
        {"tag/cyan", "36"},
        {"tag/cyan_bg", "46"},
        {"tag/white", "37"},
        {"tag/white_bg", "47"},

        {"tag/bold", "1"},
        {"tag/bold", "01"},
        {"tag/italic", "4"},
        {"tag/italic", "04"},
        {"tag/blink", "5"},
        {"tag/blink", "05"},
        {NULL, ""},
        {NULL, "0"},
        {NULL, "00"},
        {NULL, "22"},
        {NULL, "24"},
        {NULL, NULL}            // this marks the end of sequences.
};


static const gchar *
get_ansi_tag(const gchar *code){
    sequence_t *p;
    for(p = sequence_v; p && p->sequence; p++) {
	if(strcmp (code, p->sequence) == 0) {
	    return p->id;
	}
    }
    return NULL;

}


typedef struct lpterm_colors_t {
    const gchar *id;
    GdkColor color;
} lpterm_colors_t;


static
GtkTextTag *
resolve_tag (GtkTextBuffer * buffer, const gchar * id) {
    if (!id) return NULL;

    GtkTextTag *tag = NULL;
    lpterm_colors_t lpterm_colors_v[] = {
        {"tag/command", {101, 0x5858, 0x3434, 0xcfcf}},
        {"tag/stderr", {102, 0xcccc, 0, 0}},
        {"tag/command_id", {103, 0x0000, 0x0000, 0xffff}},
        {"tag/green", {104, 0x0000, 0xaaaa, 0x0000}},
        {"tag/red", {105, 0xcdcd, 0x0000, 0x0000}},
        {"tag/blue", {106, 0x0000, 0x0000, 0xcdcd}},
        {"tag/yellow", {107, 0xcdcd, 0xcdcd, 0x0000}},
        {"tag/magenta", {108, 0xcdcd, 0x0000, 0xcdcd}},
        {"tag/cyan", {109, 0x0000, 0xcdcd, 0xcdcd}},
        {"tag/black", {1, 0x0000, 0x0000, 0x0000}},
        {"tag/white", {0, 0xffff, 0xffff, 0xffff}},
        {"tag/grey", {110, 0x8888, 0x8888, 0x8888}},
        {NULL, {0, 0, 0, 0}}

    };
    void *initialized = g_object_get_data(G_OBJECT(buffer), "text_tag_initialized");
    if (!initialized) {
	lpterm_colors_t *p = lpterm_colors_v;
        for(; p && p->id; p++) {
	    gtk_text_buffer_create_tag (buffer, p->id, 
		    "foreground_gdk", &(p->color), 
		    NULL);
	    gchar *bg_id = g_strconcat(p->id, "_bg", NULL);
 	    gtk_text_buffer_create_tag (buffer, bg_id, 
		    "background_gdk", &(p->color), 
		    NULL);
	    g_free(bg_id);
       }
        gtk_text_buffer_create_tag (buffer, 
                "tag/bold", "weight", PANGO_WEIGHT_BOLD, "foreground_gdk", NULL, NULL);
        gtk_text_buffer_create_tag (buffer, 
                "tag/italic", "style", PANGO_STYLE_ITALIC, "foreground_gdk", NULL, NULL);
	g_object_set_data(G_OBJECT(buffer), "text_tag_initialized", GINT_TO_POINTER(1));
    } 

    tag = gtk_text_tag_table_lookup (gtk_text_buffer_get_tag_table (buffer), id);
    if (!tag) fprintf(stderr, "No GtkTextTag for %s\n", id);
    return tag;
}



static GtkTextTag **
resolve_tags(GtkTextBuffer * buffer, const gchar *tag){
    GtkTextTag **tags = NULL;
    if(tag) {
        if(strncmp (tag, "tag/", strlen ("tag/")) == 0) {
	    tags = (GtkTextTag **)malloc(2*sizeof(GtkTextTag *));
	    if (!tags) g_error("malloc\n");
	    tags[0] = resolve_tag (buffer, tag);
	    tags[1] = NULL;
        } else if(strncmp (tag, "icon/", strlen ("icon/")) == 0){
            //icon = rfm_get_pixbuf (id, SIZE_BUTTON);
        }
    }
    return tags;
}

static void
insert_string (print_c *print_p, GtkTextBuffer * buffer, const gchar * s, GtkTextTag **tags) {
    if(!s) return;
    GtkTextIter start, end, real_end;
    gint line = gtk_text_buffer_get_line_count (buffer);
    gchar *a = NULL;
   

    gchar esc[]={0x1B, '[', 0};
    gboolean head_section = strncmp(s, esc, strlen(esc));
    const char *esc_location = strstr (s, esc);
    if(esc_location) {      //vt escape sequence
	// do a split
	    NOOP(stderr, "0.splitting %s\n", s);
	gchar **split = g_strsplit(s, esc, -1);
	if (!split) {
	    DBG("insert_string(): split_esc barfed\n");
	    return;
	}

	gchar **pp=split;
	gint count = 0;
	for (;pp && *pp; pp++, count++){
	    if (strlen(*pp) == 0) continue;
	    NOOP(stderr, "split %d: %s\n", count, *pp);
	    if (count==0 && head_section){
		insert_string (print_p, buffer, *pp, NULL);
		continue;
	    }
	    gchar *code = *pp;
	    if (*code == 'K'){
		insert_string (print_p, buffer, "\n", NULL);
		continue;
	    }
	    NOOP(stderr, "1.splitting %s\n", *pp);
	    
	    gchar **ss = g_strsplit(*pp, "m", 2);

	    // Insert tags
	    gchar **tags = NULL;
	    if (strchr(ss[0],';')) {
		NOOP(stderr, "2.splitting %s\n", ss[0]);
		tags = g_strsplit(ss[0], ";", -1);
	    } else {
		tags = (gchar **)malloc(sizeof(gchar *) * 2 );
		if (!tags) g_error("malloc: %s\n", "no memory");
		tags[0] = g_strdup(ss[0]);
		tags[1] = 0;
	    }
	    gchar **t = tags;
	    gint tag_count = 0;
	    for (;t && *t; t++)tag_count++;
	    GtkTextTag **gtags = (GtkTextTag **)malloc((tag_count+1)*sizeof(GtkTextTag *));
	    if (!gtags) g_error("malloc: %s\n", "no memory");
	    memset(gtags, 0, (tag_count+1)*sizeof(GtkTextTag *));


	    gint i;
	    for (i=0,t = tags;t && *t; t++) {
		if (!strcmp(*t,"01") || !strcmp(*t,"1")
		    || !strcmp(*t,"05") || !strcmp(*t,"5"))
		{
		    gtags[i++] = resolve_tag(buffer, "tag/bold");
		    continue;
		}
		const gchar *tag = get_ansi_tag(*t);
		if (!tag) {
		    NOOP(stderr, "no ansi tag for %s\n", *t);
		    continue;
		}
		gtags[i++] = resolve_tag(buffer, tag);
		NOOP(stderr, "ansi_tag=%s\n", tag);
	    }


	    // Insert string
	    insert_string (print_p, buffer, ss[1], gtags);
	    g_free(gtags);

	    g_strfreev(ss);
	    
	}
	g_strfreev(split);
	// done
	return;
    }

    GtkTextMark *mark = gtk_text_buffer_get_mark (buffer, "rfm-ow");
    if(strchr (s, 0x0D)) {      //CR
        gchar *aa = g_strdup (s);
        *strchr (aa, 0x0D) = 0;
        insert_string (print_p, buffer, aa, tags);
        g_free (aa);
        const char *caa = strchr (s, 0x0D) + 1;
        if(mark) {
            gtk_text_buffer_get_iter_at_line (buffer, &start, line);
            gtk_text_buffer_move_mark (buffer, mark, &start);
        }
        insert_string (print_p, buffer, caa, tags);
        g_free (a);
        // we're done
        return;
    }

    gchar *q = print_p->utf_string (s);
    /// this should be mutex protected since this function is being called
    //  from threads all over the place.
    static pthread_mutex_t insert_mutex =  PTHREAD_MUTEX_INITIALIZER;

    pthread_mutex_lock(&insert_mutex);
    if(mark == NULL) {
        gtk_text_buffer_get_end_iter (buffer, &end);
        gtk_text_buffer_create_mark (buffer, "rfm-ow", &end, FALSE);
    } else {
        gtk_text_buffer_get_iter_at_mark (buffer, &end, mark);
    }

    gtk_text_buffer_get_iter_at_line (buffer, &start, line);
    gtk_text_buffer_get_end_iter (buffer, &real_end);

    // overwrite section
    gchar *pretext = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);
    gchar *posttext = gtk_text_buffer_get_text (buffer, &end, &real_end, TRUE);
    long prechars = g_utf8_strlen (pretext, -1);
    long postchars = g_utf8_strlen (posttext, -1);
    long qchars = g_utf8_strlen (q, -1);
    g_free (pretext);
    g_free (posttext);
    if(qchars < postchars) {
        gtk_text_buffer_get_iter_at_line_offset (buffer, &real_end, line, prechars + qchars);
    }
    /////
    gtk_text_buffer_delete (buffer, &end, &real_end);
    if(tags) {
	gint tag_count = 0;
	GtkTextTag **tt = tags;
	for (;tt && *tt; tt++)tag_count++;
	switch (tag_count) { // This is hacky
	    case 1:
		gtk_text_buffer_insert_with_tags (buffer, &end, q, -1, 
			tags[0], NULL);
		break;
	    case 2:
		gtk_text_buffer_insert_with_tags (buffer, &end, q, -1,
			tags[0],tags[1], NULL);
		break;
	    default: // Max. 3 tags...
		gtk_text_buffer_insert_with_tags (buffer, &end, q, -1,
			tags[0],tags[1],tags[1], NULL);
		break;
	}
		
    } else {
	NOOP(stderr, "gtk_text_buffer_insert %s\n", q);
        gtk_text_buffer_insert (buffer, &end, q, -1);
    }
    pthread_mutex_unlock(&insert_mutex);
    g_free (q);
    g_free (a);
    return;
}

#define MAX_LINES_IN_BUFFER 1000    
static void
scroll_to_mark(GtkTextView *textview){
    GtkTextIter start, end;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (textview);
    
    gtk_text_buffer_get_bounds (buffer, &start, &end);
    GtkTextMark *mark = gtk_text_buffer_get_mark (buffer, "scrollmark");
    if (mark == NULL){
	mark = gtk_text_buffer_create_mark (buffer, "scrollmark", &end, FALSE);
    } else {
	gtk_text_buffer_move_mark   (buffer,  mark  ,&end);
    }
    gtk_text_view_scroll_mark_onscreen (textview, mark);
    gint line_count = gtk_text_buffer_get_line_count (buffer);
    if (line_count > MAX_LINES_IN_BUFFER) {
	gtk_text_buffer_get_iter_at_line (buffer, &start, 0);
	gtk_text_buffer_get_iter_at_line (buffer, &end, line_count - MAX_LINES_IN_BUFFER);
	gtk_text_buffer_delete (buffer, &start, &end);
    }
    return;
}


static void *
print_f(void *data){
    if (!data) return GINT_TO_POINTER(-1);
    void **arg = (void **)data;
    print_c *print_p = (print_c *) arg[0];
    GtkTextView *textview = print_p->get_diagnostics();
    if (!GTK_IS_TEXT_VIEW(textview)) return GINT_TO_POINTER(-1);
    const gchar *tag = (const gchar *)arg[1];
    const gchar *string = (const gchar *)arg[2];

    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
    GtkTextTag **tags = resolve_tags(buffer, tag);
    if(string && strlen (string)) {
        insert_string (print_p, buffer, string, tags);
    }
    g_free(tags);
    scroll_to_mark(textview);
    return NULL;
}

#if 0   
static void *
print_op(void *data){
    if (!data) return GINT_TO_POINTER(-1);
    void **arg=(void **)data;
    void *arg1[]={arg[0],(void *)"tag/green", arg[1]};
    print_f((void *)arg1);
    void *arg2[]={arg[0], (void *)"tag/bold", arg[2]};
    return print_f((void *)arg2);
}
#endif

static void *
print_i(void *data){
    if (!data) return GINT_TO_POINTER(-1);
    void **arg=(void **)data;
    GdkPixbuf *pixbuf = (GdkPixbuf *)arg[0];
    print_c *print_p = (print_c *) arg[1];
    GtkTextView *textview = print_p->get_diagnostics();
    if (!GTK_IS_TEXT_VIEW(textview)) return GINT_TO_POINTER(-1);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (textview);
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds (buffer, &start, &end);
    gtk_text_buffer_insert_pixbuf (buffer, &end, pixbuf);
    return print_f(arg+1);
}
    

static void *
print_d(void *data){
    if (!data) return GINT_TO_POINTER(-1);
    void **arg=(void **)data;
    void *d_arg[]={arg[0],(void *)"tag/italic", (void *)"DBG> "};
    print_f((void *)d_arg);
    return print_f(data);
}
    
    
static void *
print_e(void *data){
    if (!data) return GINT_TO_POINTER(-1);
    void **arg=(void **)data;
    void *e_arg1[]={arg[0],(void *)"tag/red", (void *)"*** "};
    print_f((void *)e_arg1);
    return print_f(data);
}


static void *
print_s(void *data){
    if (!data) return GINT_TO_POINTER(-1);
    void **arg = (void **)data;
    print_c *print_p = (print_c *) arg[0];
    GtkTextView *textview = print_p->get_status();
    if (!GTK_IS_TEXT_VIEW(textview)) return GINT_TO_POINTER(-1);
    const gchar *string = (const gchar *)arg[1];

    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds (buffer, &start, &end);
    gtk_text_buffer_delete (buffer, &start, &end);
    if(string && strlen (string)) {
        insert_string (print_p, buffer, string, NULL);
    }
    return NULL;
}

static void *
print_sl(void *data){
    if (!data) return GINT_TO_POINTER(-1);
    void **arg = (void **)data;
    GtkLabel *status_label = (GtkLabel *)arg[0];
    const gchar *text = (const gchar *)arg[1];
    if (!GTK_IS_LABEL(status_label)) return GINT_TO_POINTER(-1);
    gtk_label_set_markup(status_label, text);
    return NULL;
}

static void *
clear_text_buffer_f(void *data){
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (data));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds (buffer, &start, &end);
    gtk_text_buffer_delete (buffer, &start, &end);
    return NULL;
}


