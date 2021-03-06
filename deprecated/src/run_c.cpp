#ifdef HAVE_WINDOWS_H
# include <windows.h>
// build libtubo with:
// ./configure --host=x86_64-w64-mingw32 --prefix=/usr/x86_64-w64-mingw32
#else
#include <tubo.h>
#endif


#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include <unistd.h>
#include <gtk/gtk.h>
#include "util_c.hpp"

#include "window_c.hpp"
#include "view_c.hpp"
#include "run_c.hpp"


/*********************************************************************************/
////////////////////////////////////////////////////////////////////////////////////
//                             run_c class methods                                //
////////////////////////////////////////////////////////////////////////////////////

static void *thread_f(void *data){
    return NULL;
}
static void *wait_f(void *data){
    return NULL;
}




static void
rfm_operate_stdout (void *data, void *stream, int childFD) {
    view_c *view_p = (view_c *)data;
    window_c *window_p = (window_c *)(view_p->get_window_v());
    if (!window_p->is_view_in_list(data)) return;
    
    lpterm_c *lpterm_p = view_p->get_lpterm_p();

    // FIXME exit status... necessary XXX test if so.
    //  (we had exit status in window and in each view...
    //gint status = rfm_global_p->status;
    //if (status == STATUS_EXIT) return;

    char *line;
    line = (char *)stream;
    if(line[0] == '\n') return;
    //TRACE("* %s", line);
    const gchar *exit_token = "Tubo-id exit:";
    gchar *recur = NULL;
    if (strstr(line, exit_token) && strstr(line, exit_token) != line){
        recur = g_strdup(strstr(line, exit_token));
        strcpy(strstr(line, exit_token), "\n");
    }

    //int bell=0x07; // bell
    int bs=0x08;   // backspace
    //int ht=0x09;   // horizontal tab
    //int lf=0x0a;   // linefeed
    //int vt=0x0b;   // vertical tab
    //int ff=0x0c;   // formfeed
    //int cr=0x0d;   // carriage return

    // apply all ^H (bs) found (as in rar output)
    int i, j;
    gchar *outline = g_strdup (line);

    for(i = 0, j = 0; line[i]; i++) {
	if(line[i] == bs && j > 0) j--;
	else {
            outline[j] = line[i];
            j++;
        }
    }
    outline[j] = 0;

    if(strncmp (line, exit_token, strlen (exit_token)) == 0) {
        gchar *string = lpterm_p->exit_string(line);
        lpterm_p->print_icon("emblem-redball", g_strdup(string));
        g_free(string);
    } else {
	lpterm_p->print(g_strdup(outline));
    }
    g_free(outline);


    // With this, this thread will not do a DOS attack
    // on the gtk event loop.
    static gint count = 1;
    if (count % 20 == 0){
        usleep(10000);
    } 
    if (recur) {
        rfm_operate_stdout (data, recur, childFD);
        g_free(recur);
    }
    return;
}

void
rfm_operate_stderr (void *data, void *stream, int childFD) {
    view_c *view_p = (view_c *)data;
    window_c *window_p = (window_c *)(view_p->get_window_v());
    if (!window_p->is_view_in_list(data)) return;
    lpterm_c *lpterm_p = view_p->get_lpterm_p();

    // FIXME exit status... necessary XXX test if so.
    //  (we had exit status in window and in each view...
    //gint status = rfm_global_p->status;
    //if (status == STATUS_EXIT) return;

    char *line;
    line = (char *)stream;


    if(line[0] != '\n') {
	lpterm_p->print_tag("tag/red", g_strdup(line));
    }

    // With this, this thread will not do a DOS attack
    // on the gtk event loop.
    static gint count = 1;
    if (count % 20 == 0){
        usleep(10000);
    } 
   return;
}





static void
operate_stdout (void *data, void *stream, gint childFD) {
    run_c *run_p = (run_c *)data;
    gchar *line = (gchar *)stream;

    run_p->print_tag("tag/green",  g_strdup(line));
    // This is a bit hacky, to keep runaway output from hogging
    // up the gtk event loop.
    static gint count = 1;
    if (count % 20 == 0){
        usleep(100000);
    } else {
        usleep(1000);
    }
    // reap child here.
   return;
}
static void
operate_stderr (void *data, void *stream, gint childFD) {
    run_c *run_p = (run_c *)data;
    gchar *line = (gchar *)stream;

    run_p->print_tag("tag/red",  g_strdup(line));
    // This is a bit hacky, to keep runaway output from hogging
    // up the gtk event loop.
    static gint count = 1;
    if (count % 20 == 0){
        usleep(100000);
    } else {
        usleep(1000);
    }
   return;
}

gboolean done_f(void *data) {
    view_c *view_p = (view_c *)data;
    //view_p->get_lpterm_p()->print_tag("tag/bold", g_strdup_printf("%s\n", "run complete."));
    return FALSE;
}

static
void
fork_finished_function (void *data) {
    g_timeout_add(1, done_f, data);                                                
}

static void
threadwait (void) {
    struct timespec thread_wait = {
        0, 100000000
    };
    nanosleep (&thread_wait, NULL);
}

#define MAX_COMMAND_ARGS 2048

// FIXME: this mutex must live across objects (why?)
static pthread_mutex_t fork_mutex=PTHREAD_MUTEX_INITIALIZER;
static void
fork_function (void *data) {
    gchar **argv = (char **)data;

    gint i = 0;
    static gchar *sudo_cmd=NULL;
    pthread_mutex_lock(&fork_mutex);
    g_free(sudo_cmd);
    sudo_cmd=NULL;
    for(i=0; argv && argv[i] && i < 5; i++) {
	if(!sudo_cmd && 
		(strstr (argv[i], "sudo") ||
		 strstr (argv[i], "ssh")  ||
		 strstr (argv[i], "rsync")  ||
		 strstr (argv[i], "scp"))) {
	    sudo_cmd=g_strdup_printf("<b>%s</b> ", argv[i]);
	    continue;
	} 	
	if (sudo_cmd){
	    if (strchr(argv[i], '&')){
	        gchar **a = g_strsplit(argv[i], "&", -1);
		gchar **p=a;
		for (;p && *p; p++){
		    const gchar *space = (strlen(*p))?" ":"";
		    const gchar *amp = (*(p+1))?"&amp;":"";
		    gchar *g = g_strconcat(sudo_cmd,  space, "<i>",*p, amp, "</i>", NULL);
		    g_free(sudo_cmd);
		    sudo_cmd=g;
		}
		g_strfreev(a);
	    } else {
		gchar *a = g_strdup(argv[i]);
		if (strlen(a) >13) {
		    a[12] = 0;
		    a[11] = '.';
		    a[10] = '.';
		    a[9] = '.';
		}
		gchar *g = g_strconcat(sudo_cmd,  " <i>",a, "</i>", NULL);
		g_free(a);
		g_free(sudo_cmd);
		sudo_cmd=g;
	    }
	}
    }


    if (i>=MAX_COMMAND_ARGS - 1) {
    	NOOP("%s: (> %d)\n", strerror(E2BIG), MAX_COMMAND_ARGS);
	argv[MAX_COMMAND_ARGS - 1]=NULL;
    }

    if (sudo_cmd) {
	gchar *g = g_strconcat(sudo_cmd,  "\n", NULL);
	g_free(sudo_cmd);
	sudo_cmd = g;
	// This  function makes copies of the strings pointed to by name and value
        // (by contrast with putenv(3))
	setenv("RFM_ASKPASS_COMMAND", sudo_cmd, 1);
	g_free(sudo_cmd);
    } else {
	setenv("RFM_ASKPASS_COMMAND", "", 1);
    }
    pthread_mutex_unlock(&fork_mutex);
    execvp (argv[0], argv);
    g_warning ("CHILD could not execvp: this should not happen\n");
    g_warning ("Do you have %s in your path?\n", argv[0]);
    threadwait ();
    _exit (123);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

run_c::run_c(void *data): run_output_c(data) {}

pid_t run_c::thread_run(const gchar **arguments){

    gchar *command = g_strdup("");
    const gchar **p = arguments;
    for (;p && *p; p++){
        gchar *g =  g_strdup_printf("%s %s", command, *p);
        g_free(command);
        command = g;
    }
                  
    int flags = TUBO_EXIT_TEXT|TUBO_VALID_ANSI|TUBO_CONTROLLER_PID;

    if (chdir(get_workdir())<0){
        print_error(g_strdup_printf("chdir(%s): %s\n", get_workdir(), strerror(errno)));
        return 0;
    }
    pid_t pid = Tubo_fork (fork_function,(gchar **)arguments,
                                NULL, // stdin
                                rfm_operate_stdout, //stdout_f,
                                rfm_operate_stderr, //stderr_f
                                fork_finished_function,
                                view_v,
                                flags);
    pid_t grandchild=Tubo_child (pid);
#ifdef DEBUG_TRACE        
    print_icon_tag("emblem-greenball", "tag/green", g_strdup_printf("<%d> %s\n", grandchild, command));
#else
    print_icon_tag("emblem-greenball", "tag/bold", g_strdup_printf("%s\n", command));
#endif
    push_hash(grandchild, g_strdup(command));
    g_free(command);
    return pid;
}

gboolean
run_c::run_in_shell(const gchar *command){
    const gchar *special = "\'*?<>|&";
    if (strchr(command, '`')) return TRUE;
    if (strchr(command, '?')) return TRUE;
    if (strchr(command, '*')) return TRUE;
    if (strchr(command, '<')) return TRUE; 
    if (strchr(command, '>')) return TRUE; 
    if (strchr(command, '|')) return TRUE; 
    if (strchr(command, '&')) return TRUE; 
    if (strchr(command, '\'')) return TRUE;
    if (strchr(command, '"')) return TRUE;
    // Are we defining an environment variable or something else?
    gint count;
    gchar **g;
    gboolean retval = FALSE;
    if (!g_shell_parse_argv (command, &count, &g, NULL)) return TRUE;
    if (!g || !g[0]) {
        retval = TRUE;
    }
    else {
        gchar *p = g_find_program_in_path(g[0]);
        if (!p) retval = TRUE;
        g_free(p);
    }
    g_strfreev(g);
    return retval;
}

pid_t run_c::thread_run(const gchar *command){
    GError *error = NULL;
    gint argc;
    gchar **argv;

    gchar *ncommand;
    if (run_in_shell(command)){
       	ncommand = g_strdup_printf("%s -c \"%s\"", util_c::u_shell(), command);
    } else ncommand = g_strdup(command);
    if(!g_shell_parse_argv (ncommand, &argc, &argv, &error)) {
        gchar *msg = g_strcompress (error->message);
        print_error(g_strdup_printf("%s: %s\n", msg, ncommand));
        g_free(ncommand);
        g_error_free (error);
        g_free (msg);
        return 0;
    }
    g_free(ncommand);
    gchar *full_path = g_find_program_in_path(argv[0]);
    if (full_path){
        g_free(argv[0]);
        argv[0] = full_path;
    }
    pid_t pid = thread_run((const gchar **)argv);
    
    g_strfreev(argv);
    return pid;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////

