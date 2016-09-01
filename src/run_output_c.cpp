#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <tubo.h>
#include "run_output_c.hpp"

#include "window_c.hpp"
#include "view_c.hpp"

run_output_c::run_output_c(void *data): csh_completion_c(data){
    // This will hash commands to know what has just finished
    c_string_hash = 
            g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, g_free);
    string_hash_mutex = PTHREAD_MUTEX_INITIALIZER;
}

run_output_c::~run_output_c(void){
    pthread_mutex_destroy(&string_hash_mutex);
    g_hash_table_destroy(c_string_hash);
}

gchar *
run_output_c::start_string_argv(gchar **argv, pid_t controller){
    pid_t grandchild=Tubo_child (controller);
    

    gchar *g = g_strdup_printf ("%c[34m<%d>", 27, grandchild);
    gchar *gg = arg_string(argv);
    push_hash(controller, g_strdup(gg));
    gg = arg_string_format(argv);

    gchar *ggg = g_strconcat(g, " ", gg, "\n", NULL);
    g_free(g);
    g_free(gg);
    return (ggg);
}

gchar *
run_output_c::exit_string(gchar *tubo_string){
    gchar *string = NULL;
    if(strchr (tubo_string, '\n')) *strchr (tubo_string, '\n') = 0;
    gchar *s = strchr (tubo_string, '(');
    int pid = -1;
    long id = 0;
    if (s) {
        s++;
        if(strchr (s, ')')) *strchr (s, ')') = 0;
        errno = 0;
        id = strtol(s, NULL, 10);
        if (!errno){
            pid = Tubo_child((pid_t) id);
        }
    }
    gchar *g = g_strdup_printf("%c[31m<%d>", 27, pid);
    //gchar *c_string = pop_hash((pid_t)id);
    gchar *c_string = pop_hash((pid_t)pid);
    string = g_strconcat(g, c_string, "\n", NULL);
    g_free(c_string);
    g_free(g);
    return string;
}


gchar *
run_output_c::start_string(gchar *command, pid_t controller, gboolean with_shell){
    pid_t grandchild=Tubo_child (controller);
    push_hash(controller, g_strdup(command));
    gchar *g = g_strdup_printf ("%c[34m<%d>", 27, grandchild);
    gchar *gg;

    
    const gchar bold[]={27, '[', '1', 'm', 0};
    if (with_shell) {
        gg = g_strconcat (g, " ", u_shell(), " ", bold, command, "\n", NULL);
        g_free (g);
        return gg;
    }
    if (!strchr(command, '*') && !strchr(command,'?')){
        gg = g_strconcat (g, " ", bold, command, "\n", NULL);
        g_free (g);
        return gg;
    }

    gchar **ap;
    gint ac;
    if (g_shell_parse_argv (command, &ac, &ap, NULL)){
        gg = arg_string_format(ap);
        g_strfreev(ap);
        gchar *ggg = g_strconcat(g, " ", gg, "\n", NULL);
        g_free(g);
        g_free(gg);
        return ggg;
    }
    return g_strdup(g);
}

void
run_output_c::push_hash(pid_t controller, gchar *string){
    pthread_mutex_lock(&string_hash_mutex);
    g_hash_table_replace(c_string_hash, GINT_TO_POINTER(controller), string);
    pthread_mutex_unlock(&string_hash_mutex);
}

gchar *
run_output_c::pop_hash(pid_t controller){
    pthread_mutex_lock(&string_hash_mutex);
    gchar *string = (gchar *)g_hash_table_lookup (c_string_hash, GINT_TO_POINTER(controller));
    if (!string){
        pthread_mutex_unlock(&string_hash_mutex);
        DBG("controller %d not found in hashtable\n", controller);
        return g_strdup("");
    }
    g_hash_table_steal(c_string_hash, GINT_TO_POINTER(controller));
    pthread_mutex_unlock(&string_hash_mutex);
    gchar bold[]={27, '[', '1', 'm',0};
    gchar *dbg_string = g_strconcat(bold, string, NULL);
    g_free(string);
    return dbg_string;
}

gchar *
run_output_c::arg_string_format(char **arg){
    const gchar quote[]={27, '[', '3', '1', 'm', '"', 0};
    const gchar bold[]={27, '[', '1', 'm', 0};
    gchar *g = g_strdup("");
    gchar **a = arg;
    for (;a && *a; a++){
        const gchar *q;
        if (strchr(*a, '*') || strchr(*a, '?') || strchr(*a, ' ')) q = quote;
        else q = "";
        gchar *gg = g_strconcat(g, " ", q, bold, *a, q, NULL);
        g_free(g); g=gg;
    }
    return g;
}

gchar *
run_output_c::arg_string(char **arg){
    const gchar quote[]={'"', 0};
    gchar *g = g_strdup("");
    gchar **a = arg;
    for (;a && *a; a++){
        const gchar *q;
        if (strchr(*a, '*') || strchr(*a, '?') || strchr(*a, ' ')) q = quote;
        else q = "";
        gchar *gg = g_strconcat(g, " ", q, *a, q, NULL);
        g_free(g); g=gg;
    }
    return g;
}


