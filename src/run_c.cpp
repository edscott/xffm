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

#include "window_c.hpp"
#include "view_c.hpp"
#include "run_c.hpp"


/*********************************************************************************/
////////////////////////////////////////////////////////////////////////////////////
//                             run_c class methods                                //
////////////////////////////////////////////////////////////////////////////////////

static void *thread_f(void *data){
}
static void *wait_f(void *data){
}




static void
rfm_operate_stdout (void *data, void *stream, int childFD) {
    view_c *view_p = (view_c *)data;
    // FIXME: this will croak!
    window_c *window_p = (window_c *)(view_p->window_v);
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
        lpterm_p->print_icon("emblem-redball", string);
        g_free(string);
    } else {
	lpterm_p->print(outline);
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
    window_c *window_p = (window_c *)(view_p->window_v);
    if (!window_p->is_view_in_list(data)) return;
    lpterm_c *lpterm_p = view_p->get_lpterm_p();

    // FIXME exit status... necessary XXX test if so.
    //  (we had exit status in window and in each view...
    //gint status = rfm_global_p->status;
    //if (status == STATUS_EXIT) return;

    char *line;
    line = (char *)stream;


    if(line[0] != '\n') {
	lpterm_p->print_tag("tag/red", line);
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

    run_p->print_tag("tag/green", "%s", line);
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

    run_p->print_tag("tag/red", "%s", line);
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
    //view_p->get_lpterm_p()->print_tag("tag/bold", "%s\n", "run complete.");
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

// FIXME: this mutex must live across objects
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

GPid run_c::thread_run(const gchar **arguments){

    gchar *command = g_strdup("");
    const gchar **p = arguments;
    for (;p && *p; p++){
        gchar *g =  g_strdup_printf("%s %s", command, *p);
        g_free(command);
        command = g;
    }
                  
    int flags = TUBO_EXIT_TEXT|TUBO_VALID_ANSI|TUBO_CONTROLLER_PID;

    pid_t pid = Tubo_fork (fork_function,(gchar **)arguments,
                                NULL, // stdin
                                rfm_operate_stdout, //stdout_f,
                                rfm_operate_stderr, //stderr_f
                                fork_finished_function,
                                view_v,
                                flags);
    //print_tag("tag/bold", "Started process with pid:%d\n", pid);
    pid_t grandchild=Tubo_child (pid);
    print_icon_tag("emblem-greenball", "tag/blue", "<%d>", grandchild);
    print_tag("tag/bold", "%s\n", command);
    push_hash(grandchild, g_strdup(command));
    g_free(command);
}


GPid run_c::thread_run(const gchar *command){
    GError *error = NULL;
    gint argc;
    gchar **argv;
    gboolean with_shell = FALSE;
    const gchar *special = "\'*?<>|&";
    if (strchr(command, '?')) with_shell = TRUE;
    if (strchr(command, '*')) with_shell = TRUE;
    if (strchr(command, '<')) with_shell = TRUE; 
    if (strchr(command, '>')) with_shell = TRUE; 
    if (strchr(command, '|')) with_shell = TRUE; 
    if (strchr(command, '&')) with_shell = TRUE; 
    if (strchr(command, '\'')) with_shell = TRUE;

    gchar *ncommand;
    if (with_shell) ncommand = g_strdup_printf("%s -c \"%s\"", u_shell(), command);
    else ncommand = g_strdup(command);
    if(!g_shell_parse_argv (ncommand, &argc, &argv, &error)) {
        gchar *msg = g_strcompress (error->message);
        print_error("%s: %s\n", msg, ncommand);
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

#if 0


/***************  threaded ****************/


// This is a thread function...

static pid_t
thread_run (
	widgets_t * widgets_p, 
	gchar ** argv, 
	gint *stdin_fd,
	void (*stdout_f) (void *stdout_data,
                      void *stream,
                      int childFD),
	void (*stderr_f) (void *stderr_data,
                      void *stream,
                      int childFD),
	void (*tubo_done_f) (void *data)
	) {

    if (!widgets_p) {
	DBG("thread_run() is being called with NULL widgets_p\n");
    }
    NOOP(stderr, "At thread_run(), widgets_p=0x%x\n",
	    GPOINTER_TO_INT(widgets_p));


    if(widgets_p && widgets_p->workdir && strcmp(g_get_home_dir(),widgets_p->workdir)){
	NOOP( "At chdir(%s)\n", widgets_p->workdir);
	if (chdir (widgets_p->workdir) < 0) {
	    DBG ("chdir(%s): %s\n", widgets_p->workdir, strerror (errno));
	}
    } else {
	if (chdir (g_get_home_dir()) < 0) {
	    DBG ("chdir(%s): %s\n", g_get_home_dir(), strerror (errno));
	}
    }
    NOOP( "At g_find_program_in_path()\n");
    gchar *g = g_find_program_in_path (argv[0]);
    if(g) g_free (g);
    else if(widgets_p) {
	rfm_threaded_show_text(widgets_p);
	rfm_threaded_diagnostics (widgets_p, "xffm/stock_dialog-error", NULL);
	rfm_threaded_diagnostics (widgets_p, "xffm_tag/stderr", g_strconcat(argv[0], ": ", strerror (ENOENT), "\n", NULL));      
	return -1;
    }


#if DEBUG_TRACE
    TRACE( "exec Tubo() now: %s\n", (rfm_get_gtk_thread() != g_thread_self())?"CHILD fork":"MAIN fork" );
#endif

    pid_t child = Tubo_fork (fork_function,
		  (void *)argv,
		  stdin_fd,
		  (stdout_f)?stdout_f:rfm_operate_stdout,
		  (stderr_f)?stderr_f:rfm_operate_stderr,
		  (tubo_done_f)?tubo_done_f:run_fork_finished_function,
		  (void *)widgets_p,
                  TUBO_EXIT_TEXT|TUBO_VALID_ANSI|TUBO_CONTROLLER_PID
		  );
/*    pid_t child = Tubo_threads (fork_function,
		  (void *)argv,
		  stdin_fd,
		  (stdout_f)?stdout_f:rfm_operate_stdout,
		  (stderr_f)?stderr_f:rfm_operate_stderr,
		  (tubo_done_f)?tubo_done_f:run_fork_finished_function,
		  (void *)widgets_p,
		  FALSE,
		  TRUE);*/

    NOOP(stderr, "exec Tubo():%d\n", child);

    if (rfm_global()){
	rfm_add_child(child);
    }	
      
    if (widgets_p){
    //g_free (widgets_p->workdir);
    NOOP(stderr, "exec Tubo():GETWD\n");
	widgets_p->workdir = g_strdup (GETWD);
	NOOP(stderr, "exec Tubo():SETWD\n");
	SETWD();
	// Trigger a monitor reload condition now.
	// This is to get any initial changes to the view before the command completes

	TRACE("xfdir_monitor_control_greenlight now\n");
	xfdir_monitor_control_greenlight(widgets_p);
    }
    
    return child;

}

static
pid_t
private_rfm_thread_run_argv (
	widgets_t * widgets_p, 
	gchar ** argv, gboolean interm, 
	gint *stdin_fd,
	void (*stdout_f) (void *stdout_data,
                      void *stream,
                      int childFD),
	void (*stderr_f) (void *stderr_data,
                      void *stream,
                      int childFD),
	void (*tubo_done_f) (void *data)
	) {
    int i = 0;
    gchar *v_argv[MAX_COMMAND_ARGS];
    const gchar *term = NULL;
    gchar **termv = NULL;
    if(interm) {
	term = rfm_what_term ();
	// do a strsplit here. terminal command is already validated.
	if (strchr(term, ' ')){
	    termv = g_strsplit(term, " ", -1);
	    gchar **g=termv;
	    for (;g && *g; g++){
		v_argv[i++] = *g;
	    }
	} else {
	    v_argv[i++] = (gchar *)term;
	}
	const gchar *exec_option = rfm_term_exec_option(term);
        v_argv[i++] = (gchar *)exec_option;
	//TRACE( "At private_rfm_thread_run_argv(): term= %s\n",term);
    }

 
    gchar **p;
    for(p = argv; p && *p && i < MAX_COMMAND_ARGS - 2; p++) {
        v_argv[i++] = *p;
    }
    v_argv[i] = NULL;
    
    if (i==MAX_COMMAND_ARGS - 1) {
	rfm_threaded_show_text(widgets_p);
    	rfm_threaded_diagnostics(widgets_p,"xffm/stock_dialog-error",NULL);
        gchar *max=g_strdup_printf("%d",MAX_COMMAND_ARGS);
        rfm_threaded_diagnostics (widgets_p, "xffm_tag/stderr", g_strconcat(strerror(E2BIG)," (> ",max,")","\n", NULL));
        g_free(max);
    }
    gchar *command = g_strdup (v_argv[0]);
    gchar *qq;
    for(i = 1; v_argv[i]; i++) {
            qq = command;
            command = g_strconcat (qq, " ", v_argv[i], NULL);
            g_free (qq);
    }
    gchar *s = strstr(command, "password=");
    if (s) {
	s = s + strlen("password=");
	for (; s && *s && *s !=' ' && *s !=','; s++) *s = '*';
    }
    gboolean visible;
    rfm_global_t *rfm_global_p = rfm_global();
    if (rfm_global_p) {
	visible = rfm_threaded_diagnostics_is_visible (widgets_p);
    } else {
	visible = rfm_diagnostics_is_visible (widgets_p);
    }
    NOOP (stderr, "private_rfm_thread_run_argv(): thread_run\n");
    pid_t controller = thread_run (widgets_p, v_argv, stdin_fd, stdout_f, stderr_f, tubo_done_f);
    if(visible) {
        gchar *g = lpterm_p->start_string_argv(v_argv, controller);
        rfm_threaded_diagnostics (widgets_p, "xffm/emblem_greenball", g);   
    }
    if(widgets_p && controller > 0) {
        setup_run_button_thread (widgets_p, command, controller);
    }

    g_free (termv);
    g_free (command);
    return controller;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////


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

const gchar **rfm_get_terminals(void) {return terminals_v;}
const gchar **rfm_get_editors(void) {return editors_v;}

static GSList *children_list=NULL;

static pthread_mutex_t children_list_mutex = PTHREAD_MUTEX_INITIALIZER;

void rfm_remove_child(pid_t child){
    if (!children_list) return;
    pthread_mutex_lock(&(children_list_mutex));
    children_list = g_slist_remove(children_list, GINT_TO_POINTER(child));
    pthread_mutex_unlock(&(children_list_mutex));
    return;
}

void rfm_killall_children(void){
    TRACE("rfm_killall_children(): signalling controller children...\n");
    pthread_mutex_lock(&(children_list_mutex));

    GSList *list = children_list;
    for (;list && list->data; list = list->next){
	pid_t child = GPOINTER_TO_INT(list->data);
	TRACE( "ZZZZZZZ--- killing %d\n", child);
	kill(child, SIGTERM);

    }
    g_slist_free(children_list);
    children_list = NULL;
    pthread_mutex_unlock(&(children_list_mutex));

}

void rfm_add_child(pid_t child){ 
    pthread_mutex_lock(&(children_list_mutex));
    NOOP(stderr, "adding %d to children_list\n", child);
    children_list = g_slist_prepend(children_list, GINT_TO_POINTER(child));
    pthread_mutex_unlock(&(children_list_mutex));

    return;
}

gchar * 
rfm_get_text_editor_envar(const gchar *value){
    if(!value) return NULL;
    
    gchar *editor=g_path_get_basename(value);
    // if nano or vi, then use terminal emulator
    if (editor && 
	    (strncmp(editor, "vi",strlen("vi"))==0 
	     || 
	     strncmp(editor, "nano",strlen("nano"))==0)){
	const gchar *t=getenv("TERMINAL_CMD");
	gchar *term = g_find_program_in_path(t);
	if (term) g_free(term);
	else {
	    t=NULL;
	    gint i;
	    for (i=0; terminals_v[i]; i++){
		// sakura is broken... 
		if (strstr(terminals_v[i], "sakura")) continue;
		term = g_find_program_in_path(terminals_v[i]);
		if (term){
		    t=terminals_v[i];
		    g_free(term);
		    break;
		}
	    }
	}
	if (t && strlen(t)) {
	    gchar *b=g_strdup_printf("%s %s %s",
		    t, rfm_term_exec_option(t), editor);
	    g_free(editor);
	    editor = b;
	}
    } else {
	g_free(editor);
	editor = g_strdup(value);
    }
    return (editor);
}

///////
static pthread_mutex_t *
get_command_history_mutex(void){
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    return &mutex;
}
//////////////// Module wraparounds ////////////////////////////
//
// Use rfm_thread_run_argv or rfm_thread_run?
// rfm_thread_run_argv will execute directly, not via a shell, and will
// not be saved in the lpterm history

// rfm_thread_run_argv:
// This modality will execute the command without shell


// rfm_thread_run:
// This modality will execute the command via "sh -c" and allow pipes and 
// redirection. and will be saved in the lpterm history file
    
pid_t
rfm_thread_run_argv (
	widgets_t * widgets_p, 
	gchar ** argv, 
	gboolean interm
	){
    const void *vector[]={widgets_p, argv, GINT_TO_POINTER(interm), NULL,  NULL, NULL, NULL, NULL};
    return GPOINTER_TO_INT(rfm_vector_run(RFM_MODULE_DIR, "run", GINT_TO_POINTER(7),
		    vector, "m_thread_run_argv"));
}

pid_t
rfm_thread_run_argv_full (
	widgets_t * widgets_p, 
	gchar ** argv, 
	gboolean interm,
	gint *stdin_fd,
	void (*stdout_f) (void *stdout_data,
                      void *stream,
                      int childFD),
	void (*stderr_f) (void *stdout_data,
                      void *stream,
                      int childFD),
	void (*tubo_done_f) (void *data)
	){
    if (!argv || !argv[0]) return 0;
    const void *vector[]={widgets_p, argv, GINT_TO_POINTER(interm), stdin_fd,  stdout_f, stderr_f, tubo_done_f};
    return GPOINTER_TO_INT(rfm_vector_run(RFM_MODULE_DIR, "run", GINT_TO_POINTER(7),
		    vector, "m_thread_run_argv"));

 }

pid_t
rfm_thread_run_argv_with_stdin (
	widgets_t * widgets_p, 
	gchar ** argv, 
	gboolean interm, 
	gint *stdin_fd
    	){
    const void *vector[]={widgets_p, argv, GINT_TO_POINTER(interm), stdin_fd,  NULL, NULL, NULL};
    return GPOINTER_TO_INT(rfm_vector_run(RFM_MODULE_DIR, "run", GINT_TO_POINTER(7),
		    vector, "m_thread_run_argv"));

 
}

pid_t
rfm_thread_run_argv_with_stdout (
	widgets_t * widgets_p, 
	gchar ** argv, 
	gboolean interm, 
	void (*stdout_f) (void *stdout_data,
                      void *stream,
                      int childFD)
	){
    const void *vector[]={widgets_p, argv, GINT_TO_POINTER(interm), NULL,  stdout_f, NULL, NULL};
    return GPOINTER_TO_INT(rfm_vector_run(RFM_MODULE_DIR, "run", GINT_TO_POINTER(7),
		    vector, "m_thread_run_argv"));

    
}

pid_t
rfm_thread_run_argv_with_stderr (
	widgets_t * widgets_p, 
	gchar ** argv, 
	gboolean interm, 
	void (*stderr_f) (void *stderr_data,
                      void *stream,
                      int childFD)
	){
    const void *vector[]={widgets_p, argv, GINT_TO_POINTER(interm), NULL,  NULL, stderr_f, NULL};
    return GPOINTER_TO_INT(rfm_vector_run(RFM_MODULE_DIR, "run", GINT_TO_POINTER(7),
		    vector, "m_thread_run_argv"));

}
////////////////////////////////////////////////////////////////////////////

void
rfm_recover_flags (gchar * in_cmd, gboolean * interm, gboolean * hold) {
    DBHashTable *runflags;
    GString *gs;
    int *flags;
    gchar *g = g_build_filename ( RUN_FLAG_FILE, NULL);
    TRACE("opening %s...\n",g); 
    if((runflags = dbh_new (g, NULL, DBH_READ_ONLY|DBH_PARALLEL_SAFE)) == NULL) {
        TRACE ("Cannot open %s\n", g);
        *interm = 0;
        *hold = 0;
        return;
    }
    TRACE("opened %s.\n",g); 
    dbh_set_parallel_lock_timeout(runflags, 3);
    gs = g_string_new (in_cmd);
    sprintf ((char *)DBH_KEY (runflags), "%10u", g_string_hash (gs));
    g_string_free (gs, TRUE);
    flags = (int *)runflags->data;
    dbh_load (runflags);
    *interm = flags[0];
    *hold = flags[1];
    dbh_close (runflags);

    NOOP ("flags recovered from dbh file for %s, interm=%d hold=%d\n", in_cmd, *interm, *hold);
}

 
const gchar * 
rfm_term_exec_option(const gchar *terminal) {
    const gchar *exec_option = "-e";
    gchar *t = g_path_get_basename (terminal);
    if(strcmp (t, "gnome-terminal") == 0 || strcmp (t, "Terminal") == 0)
            exec_option = "-x";
    g_free(t);
    return exec_option;
}

const gchar *
rfm_what_term (void) {
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
	    const gchar **p=terminals_v;
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

////////////////////////////////////////////////////////////////////////////////////////////////

G_MODULE_EXPORT
void *
rfm_thread_run (widgets_t * widgets_p, const gchar * command, void *interm) {

    pid_t controller;
    gchar *exec_command;
    if(interm) {
        const gchar *term = rfm_what_term ();
        const char *exec_option = rfm_term_exec_option(term);
        exec_command = g_strconcat (term, " ", exec_option, " ", command, NULL);
	//fprintf(stderr, "exec command= %s\n", exec_command);
    } else
        exec_command = g_strdup (command);
    gchar *save_command = g_strdup (exec_command);
    g_strstrip (exec_command);
    if(strncmp (exec_command, "sudo", strlen ("sudo")) == 0 && strncmp (exec_command, "sudo -A", strlen ("sudo -A")) != 0) {
        check_sudo ();
        gchar *nc = g_strdup_printf ("sudo -A %s", exec_command + strlen ("sudo"));
        g_free (exec_command);
        exec_command = nc;
    }

    gchar *argv[5];
    gint i=0;

    argv[i++] = u_shell();
    argv[i++] = "-c";
    argv[i++] = exec_command;
    argv[i++] = NULL;

    controller = thread_run (widgets_p, argv, NULL, NULL, NULL, NULL);
    gboolean visible;
    rfm_global_t *rfm_global_p = rfm_global();
    if (rfm_global_p ) {
	visible = rfm_threaded_diagnostics_is_visible (widgets_p);
    } else {
	visible = rfm_diagnostics_is_visible (widgets_p);
    }
    if(visible) {
        gchar *g = rfm_diagnostics_start_string(exec_command, controller, TRUE);
        if (rfm_global_p ) {
            rfm_diagnostics (widgets_p, "xffm/emblem_greenball", g, NULL);
            g_free(g);
        } else {
            rfm_threaded_diagnostics (widgets_p, "xffm/emblem_greenball", g);
        }
    }
     if(controller > 0) {
	gchar *run_button_text = g_strdup_printf("%s -c \"%s\"", shell, exec_command);
        setup_run_button_thread (widgets_p, run_button_text, controller);
	g_free(run_button_text);
    }
    // instead of disposing of exec_command, we will save command 
    // in the sh_command history 
    rfm_save_sh_command_history (widgets_p->view_p, save_command);
    // do not do this: g_free(save_command); 
    // because save_command gets put into the history GList.
    g_free (exec_command);
    return  GINT_TO_POINTER(controller);
}


// Use rfm_thread_run_argv or rfm_thread_run?
// rfm_thread_run_argv will execute directly, not via a shell, and will
// not be saved in the lpterm history

// rfm_thread_run_argv:
// This modality will execute the command without shell
G_MODULE_EXPORT
void *
rfm_thread_run2argv (widgets_t * widgets_p, const gchar * in_command, void *interm) {
    // do the call with argv so that command not saved in lpterm history
    gint argcp;
    gchar **argvp;
    pid_t child;
    gchar *command = g_strdup(in_command);
    g_strstrip (command);
    if(strncmp (command, "sudo", strlen ("sudo")) == 0 && strncmp (command, "sudo -A", strlen ("sudo -A")) != 0) {
	gchar *nc = NULL;
        check_sudo ();
        nc = g_strdup_printf ("sudo -A %s", command + strlen ("sudo"));
	g_free(command);
	command = nc;
    }
    if(g_shell_parse_argv (command, &argcp, &argvp, NULL)) {
        NOOP ("OPEN: rfm_thread_run_argv\n");
        // this should always work
        child = rfm_thread_run_argv (widgets_p, argvp, GPOINTER_TO_INT(interm));
        g_strfreev (argvp);
    } else {
        DBG ("failed to parse command with g_shell_parse_argv() at run.c\n");
        child = GPOINTER_TO_INT(rfm_thread_run (widgets_p, command, interm));
    }
    g_free(command);
    return GINT_TO_POINTER(child);
}


G_MODULE_EXPORT
void *
rfm_try_sudo (widgets_t * widgets_p, gchar ** argv, void *interm) {
    
    gint i;
    gint j=0;
    gchar *exec_argv[MAX_COMMAND_ARGS];

    check_sudo ();
    exec_argv[j++] = "sudo";
    exec_argv[j++] = "-A";
    for(i = 0; argv[i] != NULL && j < MAX_COMMAND_ARGS - 2; i++) {
        NOOP (" %s", argv[i]);
        exec_argv[j++] = argv[i];
    }



    rfm_threaded_show_text(widgets_p);
    if (j==MAX_COMMAND_ARGS - 1) {
    	rfm_threaded_diagnostics(widgets_p,"xffm/stock_dialog-warning",NULL);
        gchar *max=g_strdup_printf("%d",MAX_COMMAND_ARGS);
        rfm_threaded_diagnostics (widgets_p, "xffm_tag/stderr", g_strconcat(strerror(E2BIG)," (> ",max,")","\n", NULL));
        g_free(max);
    }
    

    exec_argv[j++] = NULL;
    NOOP (" \n");

    pid_t child =
	private_rfm_thread_run_argv(widgets_p, exec_argv, 
		GPOINTER_TO_INT(interm), 
		NULL, NULL, NULL, NULL);
    return GINT_TO_POINTER(child);
}

///   vector function... 
G_MODULE_EXPORT
void *
m_thread_run_argv (void *p) {
    void **arg = p;
    widgets_t * widgets_p = arg[0]; 
    //view_t *view_p = widgets_p->view_p;
    gchar ** argv = arg[1];
    gboolean interm = GPOINTER_TO_INT(arg[2]);
    gint *stdin_fd = arg[3];
    void (*stdout_f) (void *stdout_data,
		  void *stream,
		  int childFD) = arg[4];
    void (*stderr_f) (void *stdout_data,
		  void *stream,
		  int childFD) = arg[5];
    void (*tubo_done_f) (void *data) = arg[6];

    NOOP( "At m_thread_run_argv()\n");
    if (widgets_p->workdir
	    &&
	!rfm_g_file_test_with_wait(widgets_p->workdir, G_FILE_TEST_IS_DIR)){
	 gchar *workdir = g_strconcat("workdir = ", 
		 (widgets_p->workdir)?widgets_p->workdir: "NULL", NULL); 
	 rfm_time_out(widgets_p, workdir);
	 g_free(workdir);
	 return NULL; 
     }

    if (widgets_p->workdir && access(widgets_p->workdir, R_OK|X_OK) != 0){
	rfm_threaded_show_text(widgets_p);
	rfm_threaded_diagnostics(widgets_p, "xffm/stock_dialog-error", NULL);
	rfm_threaded_diagnostics(widgets_p, "xffm_tag/stderr",
		g_strconcat(strerror(EACCES), ": '", widgets_p->workdir, "'\n", NULL));
	 return NULL; 
    }

	

    pid_t child =
	private_rfm_thread_run_argv(widgets_p, argv, interm, stdin_fd, stdout_f, stderr_f, tubo_done_f);

    g_free(widgets_p->workdir);
    widgets_p->workdir = g_strdup(g_get_home_dir());

    g_free(p);
    return GINT_TO_POINTER(child);
}



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
		    gchar *space = (strlen(*p))?" ":"";
		    gchar *amp = (*(p+1))?"&amp;":"";
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
    rfm_threadwait ();
    _exit (123);
}
/*******************************************/
#endif
