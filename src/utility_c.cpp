#include <stdlib.h>
#include <gtk/gtk.h>
#include <errno.h>
#include "utility_c.hpp"


////////////////////////////////////////////////////////////////////////////////////////////

utility_c::utility_c(void){
}

utility_c::~utility_c(void){
}

gchar * 
utility_c::get_text_editor(void){
    const gchar *value = getenv("EDITOR");
    if(!value) return NULL;
    
    gchar *editor=g_path_get_basename(value);
    // if nano or vi, then use terminal emulator
    if (editor && 
	    (strncmp(editor, "vi",strlen("vi"))==0 
	     || 
	     strncmp(editor, "nano",strlen("nano"))==0)){
	const gchar *term = what_term();
	if (term && strlen(term)) {
	    gchar *b=g_strdup_printf("%s %s %s",
		    term, term_exec_option(term), editor);
	    g_free(editor);
	    editor = b;
	}
    } else {
	g_free(editor);
	editor = g_strdup(value);
    }
    return (editor);
}

gchar *
utility_c::wrap_utf_string(const gchar *data, gint length){
    gchar *u;
    if (!g_utf8_validate (data, -1, NULL)){
        u = utf_string(data);
    } else {
        u = g_strdup(data);
    }
    gchar *uu = NULL;
    gint i;for (i=0; i<g_utf8_strlen(u, -1); i += length){
        gchar *xu = g_utf8_substring (u, i, i+length);
        if (!uu) uu = g_strdup(xu);
        else { 
            gchar *g = g_strconcat(uu,"\n",xu,NULL);
            g_free(xu);
            g_free(uu);
            uu = g;
        }
    }
    g_free(u);
    return uu;
}

static gboolean
context_function_f(gpointer data){
    void **arg = (void **)data;
    void * (*function)(gpointer) = (void* (*)(void*))arg[0];
    gpointer function_data = arg[1];
    pthread_mutex_t *mutex = (pthread_mutex_t *)arg[2];
    pthread_cond_t *signal = (pthread_cond_t *)arg[3];
    void **result_p = (void **)arg[4];
    void *result = (*function)(function_data);
    pthread_mutex_lock(mutex);
    *result_p = result;
    pthread_cond_signal(signal);
    pthread_mutex_unlock(mutex);
    return FALSE;
}

void *
utility_c::context_function(void * (*function)(gpointer), void * function_data){
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t signal = PTHREAD_COND_INITIALIZER; 
    void *result=GINT_TO_POINTER(-1);
    void *arg[] = {
        (void *)function,
        (void *)function_data,
        (void *)&mutex,
        (void *)&signal,
        (void *)&result
    };
    gboolean owner = g_main_context_is_owner(g_main_context_default());
    if (owner){
	context_function_f(arg);
    } else {
	g_main_context_invoke(NULL, context_function_f, arg);
	pthread_mutex_lock(&mutex);
	if (result == GINT_TO_POINTER(-1)) pthread_cond_wait(&signal, &mutex);
	pthread_mutex_unlock(&mutex);
    }
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&signal);
    return result;
}

gchar *
utility_c::utf_string (const gchar * t) {
    if(!t) { return g_strdup (""); }
    if(g_utf8_validate (t, -1, NULL)) { return g_strdup (t); }
    /* so we got a non-UTF-8 */
    /* but is it a valid locale string? */
    gchar *actual_tag;
    actual_tag = g_locale_to_utf8 (t, -1, NULL, NULL, NULL);
    if(actual_tag)
        return actual_tag;
    /* So it is not even a valid locale string... 
     * Let us get valid utf-8 caracters then: */
    const gchar *p;
    actual_tag = g_strdup ("");
    for(p = t; *p; p++) {
        // short circuit end of string:
        gchar *r = g_locale_to_utf8 (p, -1, NULL, NULL, NULL);
        if(g_utf8_validate (p, -1, NULL)) {
            gchar *qq = g_strconcat (actual_tag, p, NULL);
            g_free (actual_tag);
            actual_tag = qq;
            break;
        } else if(r) {
            gchar *qq = g_strconcat (actual_tag, r, NULL);
            g_free (r);
            g_free (actual_tag);
            actual_tag = qq;
            break;
        }
        // convert caracter to utf-8 valid.
        gunichar gu = g_utf8_get_char_validated (p, 2);
        if(gu == (gunichar) - 1) {
            gu = g_utf8_get_char_validated ("?", -1);
        }
        gchar outbuf[8];
        memset (outbuf, 0, 8);
        gint outbuf_len = g_unichar_to_utf8 (gu, outbuf);
        if(outbuf_len < 0) {
            //DBG ("utility_c::utf_string: unichar=%d char =%c outbuf_len=%d\n", gu, p[0], outbuf_len);
        }
        gchar *qq = g_strconcat (actual_tag, outbuf, NULL);
        g_free (actual_tag);
        actual_tag = qq;
    }
    return actual_tag;
}

#define MAX_PATH_LABEL 40
#define MAX_PATH_START_LABEL 18
const gchar *
utility_c::chop_excess (gchar * b) {
    // chop excess length...

    const gchar *home = g_get_home_dir();
    gchar *bb;
    if (strncmp(home, b, strlen(home))==0){
        if (strlen(home) == strlen(b)) return b;
        bb = g_strconcat("~/", b + strlen(home)+1, NULL);
    } else {
        bb = g_strdup(b);
    }
    
    int len = strlen (bb);

    if(len < MAX_PATH_LABEL) {
        strcpy(b, bb);
        g_free(bb);
        return (b);
    }
        
    bb[MAX_PATH_START_LABEL - 3] = 0;

    gchar *g = g_strconcat(bb, "...", b + (len - MAX_PATH_LABEL + MAX_PATH_START_LABEL), NULL);
    strcpy (b, g);
    g_free(bb);
    g_free(g);

    return b;
}

gchar *
utility_c::compact_line(const gchar *line){
    //1. Remove leading and trailing whitespace
    //2. Compact intermediate whitespace

    gchar *newline= g_strdup(line); 
    g_strstrip(newline);
    gchar *p = newline;
    for(;p && *p; p++){
        if (*p ==' ') g_strchug(p+1);
    }
    return newline;
}

GList *
utility_c::find_in_string_list(GList *list, const gchar *string){
    GList *tmp=g_list_first(list);
    for (;tmp && tmp->data; tmp=tmp->next){
	if (strcmp((gchar *)tmp->data, string)==0) {
	    return tmp;
	}
    }
    return NULL;
}

gboolean
utility_c::program_in_path(const gchar *program){
    gchar *s = g_find_program_in_path (program);
    if (!s) return FALSE;
    g_free(s);
    return TRUE;
}

const gchar *
utility_c::default_shell(void){
    const gchar *shells[]={"bash","tcsh","csh","dash","zsh","ksh","sash","ash","sh",NULL};
    const gchar **shell;
    for (shell = shells; shell && *shell; shell++){
        if (program_in_path(*shell)) return *shell;
    }
    g_warning("unable to find a valid shell\n");
    return "/bin/sh";
}

const gchar *
utility_c::u_shell(void){
    if(getenv ("SHELL") && strlen (getenv ("SHELL"))) {
        if (program_in_path(getenv ("SHELL"))) return getenv ("SHELL");
    }

    if(getenv ("XTERM_SHELL") && strlen (getenv ("XTERM_SHELL"))) {
        if (program_in_path(getenv ("XTERM_SHELL"))) return getenv ("XTERM_SHELL");
    }
    return default_shell();
}


gchar *
utility_c::esc_string (const gchar * string) {
    gint i, j, k;
    const gchar *charset = "\\\"\' ()|<>";

    for(j = 0, i = 0; i < strlen (string); i++) {
        for(k = 0; k < strlen (charset); k++) {
            if(string[i] == charset[k])
                j++;
        }
    }
    gchar *estring = (gchar *) calloc (1, strlen (string) + j + 1);
    for(j = 0, i = 0; i < strlen (string); i++, j++) {
        for(k = 0; k < strlen (charset); k++) {
            if(string[i] == charset[k])
                estring[j++] = '\\';
        }
        estring[j] = string[i];
    }
    NOOP ("ESC:estring=%s\n", estring);
    return estring;
}


const gchar *
utility_c::what_term (void) {
    const gchar *term=getenv ("TERMINAL_CMD");
    gchar *t=NULL;
    if(term && strlen (term)) {
	if (strchr(term, ' ')){
	    gchar **g = g_strsplit(term, " ", -1);
	    t = g_find_program_in_path (g[0]);
	    g_strfreev(g);
	} else {
	    t = g_find_program_in_path (term);
	}
    }
    if(!t) {
	    const gchar **p=get_terminals();
	    for (;p && *p; p++){
		t = g_find_program_in_path (*p);
		if (t) {
		    term=*p;
		    break;  
		}  
	    }
    }
    if (t) {
	g_free(t);
	return term;
    }
    DBG ("TERMINAL_CMD=%s: %s\n", getenv ("TERMINAL_CMD"), strerror (ENOENT));

    return NULL;
}
 
const gchar * 
utility_c::term_exec_option(const gchar *terminal) {
    const gchar *exec_option = "-e";
    gchar *t = g_path_get_basename (terminal);
    if(strcmp (t, "gnome-terminal") == 0 || strcmp (t, "Terminal") == 0)
            exec_option = "-x";
    g_free(t);
    return exec_option;
}




const gchar **
utility_c::get_terminals(void) {
    static const gchar *terminals_v[] = {
	"roxterm", 
	"sakura",
	"gnome-terminal", 
	"Eterm", 
	"konsole", 
	"Terminal", 
	"aterm", 
	"xterm", 
	"kterm", 
	"wterm", 
	"multi-aterm", 
	"evilvte",
	"mlterm",
	"xvt",
	"rxvt",
	"urxvt",
	"mrxvt",
	"tilda",
	NULL
    };
    return terminals_v;
}

const gchar **
utility_c::get_editors(void) {
    static const gchar *editors_v[] = {
	"gvim -f",  
	"mousepad", 
	"gedit", 
	"kate", 
	"xemacs", 
	"nano",
	"vi",
	NULL
    }; 
    return editors_v;
}

