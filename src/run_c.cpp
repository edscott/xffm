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



// This will hash commands to know what has just finished
static GHashTable *c_string_hash=NULL;
pthread_mutex_t string_hash_mutex = PTHREAD_MUTEX_INITIALIZER;

static 
gchar *pop_hash(pid_t controller){
    if (!c_string_hash) {
        return g_strdup("");
    }
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

static void
push_hash(pid_t controller, gchar *string){
    pthread_mutex_lock(&string_hash_mutex);
    if (!c_string_hash){
        c_string_hash = 
            g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, g_free);
    }
    g_hash_table_replace(c_string_hash, GINT_TO_POINTER(controller), string);
    pthread_mutex_unlock(&string_hash_mutex);
}

static
gchar *
default_shell(void){
    gchar *shell=NULL;
    if(!shell)
        shell = g_find_program_in_path ("bash");
    if(!shell)
        shell = g_find_program_in_path ("zsh");
    if(!shell)
        shell = g_find_program_in_path ("sh");
    /*if (!shell && rfm_void(PLUGIN_DIR, "ps", "module_active")) {
	shell = g_find_program_in_path ("tcsh");
	if(!shell)
	    shell = g_find_program_in_path ("csh");
    }*/
    if(!shell)
        shell = g_find_program_in_path ("ksh");
    if(!shell)
        shell = g_find_program_in_path ("sash");
    if(!shell)
        shell = g_find_program_in_path ("ash");
    if(!shell){
	g_warning("unable to find a valid shell\n");
    }

    return shell;
}

    // dash is OK now.
    // Only csh/tcsh is broken, since it will not
    // pass on SIGTERM when controler gets SIGUSR1
    // This is only a problem if rodent_ps is not 
    // loadable.
    // gchar *
static gchar *
check_shell(gchar *shell){
    if (!shell) return NULL;
    // FIXME (when ps module is incorporated)
    // This is because the stop process button will not work with csh.
    /*if (!rfm_void(PLUGIN_DIR, "ps", "module_active") && strstr(shell, "csh")) {
	g_free(shell);
	shell = NULL;
    }*/
    return shell;
}

gchar *
rfm_shell(void){
    gchar *shell=NULL;
    if(getenv ("SHELL") && strlen (getenv ("SHELL"))) {
        shell = g_find_program_in_path (getenv ("SHELL"));
    }

    if(!shell && getenv ("XTERM_SHELL") && strlen (getenv ("XTERM_SHELL"))) {
        shell = g_find_program_in_path (getenv ("XTERM_SHELL"));
    }
    shell = check_shell(shell);

    if (!shell){
	shell = default_shell();
    }
    return shell;
}

static gchar *
arg_string(char **arg){
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


static gchar *
arg_string_format(char **arg){
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

gchar *rfm_diagnostics_start_string(gchar *command, pid_t controller, gboolean with_shell){
    pid_t grandchild=Tubo_child (controller);
    push_hash(controller, g_strdup(command));
    gchar *g = g_strdup_printf ("%c[34m<%d>", 27, grandchild);
    gchar *gg;

    
    const gchar bold[]={27, '[', '1', 'm', 0};
    if (with_shell) {
        gchar *shell = rfm_shell();
        gg = g_strconcat (g, " ", shell, " ", bold, command, "\n", NULL);
        g_free (g);
        g_free(shell);
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

gchar *rfm_diagnostics_start_string_argv(gchar **argv, pid_t controller){
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

gchar *rfm_diagnostics_exit_string(gchar *tubo_string){
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


void
rfm_operate_stdout (void *data, void *stream, int childFD) {
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
        gchar *string = rfm_diagnostics_exit_string(line);
        lpterm_p->print_icon("emblem-redball", string);
    } else {
	lpterm_p->print(outline);
    }
    g_free(outline);


    // With this, this thread will not do a DOS attack
    // on the gtk event loop.
    static gint count = 1;
    if (count % 20 == 0){
        usleep(100000);
    } else {
        usleep(1000);
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
        usleep(100000);
    } else {
        usleep(1000);
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

run_c::run_c(void *data): csh_completion_c(data) {}

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
    if (with_shell) ncommand = g_strdup_printf("%s -c \"%s\"", rfm_shell(), command);
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

typedef struct thread_run_t {
    char *wd;
    char **argv;
} thread_run_t;

typedef struct run_data_t {
    widgets_t *widgets_p;
    union {
        pid_t controller;
        pid_t pid;
    };
    pid_t grandchild;
    gchar *command;
    gchar *workdir;
    gchar *icon;
    GtkWidget *button;
} run_data_t;

/***************  threaded ****************/


static
  gboolean
check_sudo (void) {
    return TRUE;
}


// This is a thread function, must have GDK mutex set for gtk commands...
static
void
run_fork_finished_function (void *user_data) {
    //widgets_t *widgets_p = user_data;
    // decrement thread count here

#if 0
    gchar *g = g_strdup_printf ("Cleanup: %d> pid=%d", Tubo_id () - 1, getpid());
    if(rfm_threaded_diagnostics_is_visible (widgets_p)) {
        rfm_threaded_diagnostics (widgets_p, "xffm_tag/command_id", g);
        rfm_threaded_diagnostics (widgets_p, "xffm/stock_no", g_strdup("\n"));
    }
#endif

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

// This is main thread callback
static void
show_run_info (GtkButton * button, gpointer data) {
    if (g_thread_self() != rfm_get_gtk_thread()){
	g_error("show_run_info() is a main thread function\n");
    }
    run_data_t *run_data_p = data;
    guint button_id = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(button), "button_id"));
    if (rfm_void(PLUGIN_DIR, "ps", "module_active")){
	if (button_id == 3){
	    NOOP(stderr, "ps popup now... \n");
	    record_entry_t *en = rfm_mk_entry(0);
	    en->type=0; /* remove local-type attributes */
	    en->st = (struct stat *)malloc(sizeof(struct stat));
	    if (!en->st) g_error("malloc: %s\n", strerror(errno));
	    memset(en->st,0,sizeof(struct stat));

	    pid_t child = Tubo_child(run_data_p->pid);
	    en->path = g_strdup_printf("%d:%s", child, run_data_p->command);
	    en->st->st_uid = child;
	    
	    rfm_rational(PLUGIN_DIR, "ps", NULL, en, "private_popup");
	    NOOP(stderr, "popup mapped...\n");
	    return;
	}
	gchar *ps_module = g_find_program_in_path("rodent-plug");
	if (ps_module) {
	    gchar *command=g_strdup_printf("%s ps %d",
		    ps_module, (gint) run_data_p->pid);
	    GError *error=NULL;
	    if (!g_spawn_command_line_async (command, &error)){
		g_warning("%s: %s\n", ps_module, error->message);
		g_error_free(error);
		g_free(command);
		// here we default to the terminate dialog...
	    } else {
		g_free(command);
		return;
	    }
	}
    } else { // no rodent-ps
	if (button_id == 3) return;
	
	gchar *text2 =g_strdup_printf ("%s %s: %s\n\n%s %s (%d)?",
		_("Kill (KILL)"), run_data_p->command, 
		strerror(ETIMEDOUT), 
		_("Kill"), run_data_p->command, run_data_p->pid);
	if(rfm_confirm (run_data_p->widgets_p, GTK_MESSAGE_QUESTION, text2, _("No"), _("Yes"))) {
	    // SIGUSR2 will pass on a SIGKILL signal
	    gchar *gg = g_strdup_printf ("%d", (int)(run_data_p->pid));
	    rfm_diagnostics (run_data_p->widgets_p, "xffm/stock_dialog-warning", NULL);
	    rfm_diagnostics (run_data_p->widgets_p, "xffm_tag/command_id",
		    _("Kill (KILL)"), " ", gg, "\n", NULL);
	    g_free (gg);
	    kill (run_data_p->pid, SIGUSR2);
	}
	g_free (text2);
    }
}

static void *
make_run_data_button (gpointer data) {
    run_data_t * run_data_p = data;
    if(run_data_p->widgets_p->button_space) {
	pid_t pid = Tubo_child(run_data_p->pid);
	if (pid < 0) {
	    TRACE("Tubo_child  < 0\n");
	    return NULL;
	}
	gchar *text = g_strdup(_("Left click once to follow this link.\nMiddle click once to select this cell"));
	if (strstr(text, "\n")) *strstr(text, "\n") = 0;
	gchar *short_command = g_strdup(run_data_p->command);
	if (strlen(short_command) > 80){
	    short_command[76] = ' ';
	    short_command[77] = '.';
	    short_command[78] = '.';
	    short_command[79] = '.';
	}
	gchar *tip=g_strdup_printf("%s\n(%s=%d)\n%s\n%s",  
		short_command, _("PID"), pid,
	      _("Right clicking pops context menu immediately"),
	      text);
	g_free(short_command);
	g_free(text);
	const gchar *icon_id = run_data_p->icon;
	if (!icon_id || !rfm_get_pixbuf(icon_id, SIZE_BUTTON)){
	    icon_id=(rfm_void(PLUGIN_DIR, "ps", "module_active"))?
		"xffm/stock_execute": "xffm/stock_stop";
	}
	
        run_data_p->button =
            rfm_mk_little_button (icon_id,
                                  (gpointer) show_run_info, run_data_p,
				  tip);
	g_free(tip);
        gtk_box_pack_end (GTK_BOX (run_data_p->widgets_p->button_space), run_data_p->button, FALSE, FALSE, 0);
        gtk_widget_show (run_data_p->button);
        // flush gtk
        while (gtk_events_pending()) gtk_main_iteration();
        NOOP ("DIAGNOSTICS:srun_button made for grandchildPID=%d\n", (int)run_data_p->pid);
    }
    return NULL;
}

static void *
zap_run_button(gpointer data){
	    NOOP(stderr, "zap_run_button...\n");
    run_data_t *run_data_p = data;
    if(run_data_p->button && GTK_IS_WIDGET(run_data_p->button)){
        gtk_widget_hide(GTK_WIDGET (run_data_p->button));
        gtk_widget_destroy (GTK_WIDGET (run_data_p->button));
    }
    g_free (run_data_p->command);
    g_free (run_data_p->icon);
    g_free (run_data_p->workdir);
    g_free (run_data_p);
    return NULL;
}

// This is a thread function...
static gpointer
thread_run_f (gpointer data) {
    run_data_t *run_data_p = (run_data_t *) data;

    /* create little button (note: thread protected within subroutine) */
    // The following function will not return until button is created and duly
    // processed by the gtk loop. This to avoid a race with the command completing
    // before gtk has fully created the little run button.
    rfm_context_function(make_run_data_button, run_data_p);
    NOOP ("grand---thread waitpid for %d on (%s/%s)\n", run_data_p->pid, run_data_p->command, run_data_p->workdir);

    gint status;
    waitpid (run_data_p->pid, &status, 0);

    NOOP (stderr, "grand---thread waitpid for %d complete!\n", run_data_p->pid);
    /* remove little button (thread protect gtk here) */
    
#ifdef DEBUG_TRACE    
    // This is out of sync here (grayball), so only in debug mode.
    gchar *t = rfm_diagnostics_start_string(run_data_p->command, run_data_p->controller, FALSE);
    rfm_threaded_diagnostics (run_data_p->widgets_p, "xffm/emblem_grayball", t);
#endif
    
    rfm_global_t *rfm_global_p = rfm_global();
    if (rfm_global_p) {
        TRACE("thread_run_f(): trigger reload...\n");
        // Trigger reload to all tabbed views.
        GSList **list_p = rfm_view_list_lock(NULL, "thread_run_f");
        if (!list_p) return NULL;
        g_mutex_lock(rfm_global_p->status_mutex);
        gint status = rfm_global_p->status;
        g_mutex_unlock(rfm_global_p->status_mutex);
        if (status == STATUS_EXIT) {
            rfm_view_list_unlock("thread_run_f");
            return NULL;
        }
    
        GSList *list = *list_p;
        for (; list && list->data; list = list->next){
              view_t *view_p = list-> data;
              // Skip modules...
              // (otherwise smb module goes berserk)
              if (view_p->module && !strstr(view_p->module, "fstab") ) continue;
              if (!xfdir_monitor_control_greenlight(&(view_p->widgets))) {
                  TRACE("++ run.i triggered reload...\n");
                  // XXX: disabling automatic refresh to test if
                  // it is responsible for iconview_key/update_f deadlock...
                    //rodent_trigger_reload(view_p);
              }
        }
        rfm_view_list_unlock("2 thread_run_f");
    }


    // Run has completed.  
    // no use for controller process anymore....
    NOOP(stderr, "run has completed: %d\n", run_data_p->pid);
    rfm_remove_child(run_data_p->pid);
    // Flush pipes.
    fflush(NULL);  
    // Destroy little button (if exists) and free run_data_p 
    // associated memory. Done in main thread for gtk instruction set.
    rfm_context_function(zap_run_button, run_data_p);
    return NULL;
}


static void
setup_run_button_thread (widgets_t * widgets_p, const gchar * exec_command, pid_t child) {
    NOOP ("setup_run_button_thread(), grandchildPID=%d\n", (int)child);
    run_data_t *run_data_p = (run_data_t *) malloc (sizeof (run_data_t));
    if (!run_data_p) g_error("malloc: %s", strerror(errno));
    memset (run_data_p, 0, sizeof (run_data_t));


    run_data_p->pid = child;
    run_data_p->grandchild = Tubo_child(child);
    run_data_p->command = g_strdup (exec_command);
    // Little icon assignment
    // Shell commands come from lpterminal.
    gchar **args = g_strsplit(exec_command, " ", -1);
    if (args && args[0]) {
	gchar *shell = rfm_shell();
	if (strcmp(shell, args[0])==0) run_data_p->icon = g_strdup("xffm/emblem_terminal");
	else {
	    run_data_p->icon = g_path_get_basename(args[0]);
	    NOOP(stderr, ".... %s : %s\n", run_data_p->icon, exec_command);
	}
	g_free(shell);
    }
    g_strfreev(args);

    if (widgets_p->workdir) {
	run_data_p->workdir = g_strdup (widgets_p->workdir);
    } else {
	run_data_p->workdir = g_strdup (g_get_home_dir());
    }
    run_data_p->widgets_p = widgets_p;

    view_t *view_p = widgets_p->view_p;
    rfm_view_thread_create(view_p, thread_run_f, (gpointer) run_data_p, "thread_run_f");
}

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
        gchar *g = rfm_diagnostics_start_string_argv(v_argv, controller);
        rfm_threaded_diagnostics (widgets_p, "xffm/emblem_greenball", g);   
    }
    if(widgets_p && controller > 0) {
        setup_run_button_thread (widgets_p, command, controller);
    }

    g_free (termv);
    g_free (command);
    return controller;
}



/*******************************************/
#endif
