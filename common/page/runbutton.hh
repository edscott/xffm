#ifndef RUNBUTTON_HH
#define RUNBUTTON_HH
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "common/tubo.hh"
#include "common/gtk.hh"
#include "common/util.hh"
#include "common/run.hh"

/*
typedef struct thread_run_t {
    char *wd;
    char **argv;
} thread_run_t;
*/

namespace xf {
template <class Type> class PageChild;
template <class Type> class RunButton;
template <class Type> class RunButtonSignals {
    static void
    ps_signal(GtkWidget *menuitem, gpointer data){

	auto run_button_p = (RunButton<Type> *)
	    g_object_get_data(G_OBJECT(menuitem), "run_button_p");

	gint signal_id =GPOINTER_TO_INT(data);
	if (signal_id ==-1) {
	    run_button_p->ps_renice();
	    return;
	}

        DBG("apply data: (%p) -> %p\n", (void *)menuitem, (void *)run_button_p);
	run_button_p->sendSignal(signal_id);
    }
	
/*
    static void
    run_button_toggled(GtkWidget *button, void *data){
	auto run_button_p = (RunButton<Type> *)data;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button))){
	
	}
    }
*/

};

template <class Type>
class RunButton {
    using pixbuf_c = Pixbuf<double>;
    using print_c = Print<double>;
    using gtk_c = Gtk<double>;
    using util_c = Util<double>;
    using run_c = Run<double>;
private:
    PageChild<Type> *pageChild_p;
    GtkTextView *textview;
    GtkBox *button_space;

    pid_t pid_;
    pid_t grandchild_;
    gchar *command_;
    gchar *tip_;
    gchar *icon_id_;
    GtkWidget *button_;
    GtkWidget *menu_;
public:    
        gboolean in_shell;
        GtkWidget *get_menu(void){return menu_;}

    RunButton(void *data, const gchar * exec_command, pid_t child, gboolean shell_wrap){
	pageChild_p = (PageChild<Type> *)data;
	textview = pageChild_p->diagnostics();
	button_space = pageChild_p->button_space();
	
	in_shell = shell_wrap;
	pid_ = child;
	grandchild_ = Tubo<Type>::Tubo_child(child);
	command_ = g_strdup (exec_command);
	icon_id_ = NULL;
	tip_ = NULL;
	TRACE ("RunButton::setup_run_button_thread: controller/process=%d/%d\n", (int)child, (gint)grandchild_);

	create_menu();
	pageChild_p->thread_create("RunButton::RunButton: run_wait_f", 
		run_wait_f, (void *) this, FALSE);
    }


    ~RunButton(void){
	TRACE("RunButton::~RunButton... button_ %p\n", (void *)button_);
	if(button_ && GTK_IS_WIDGET(button_)){
	    gtk_widget_hide(GTK_WIDGET (button_));
	    gtk_widget_destroy (GTK_WIDGET (button_));
	}
	g_free (tip_);
	g_free (command_);
	g_free (icon_id_);
    }


    void
    set_icon_id(const gchar *data){
	g_free(icon_id_); 
	icon_id_ = data?g_strdup(data):NULL;
    }

    const gchar *
    get_icon_id(void){ return (const gchar *)icon_id_;}

    const gchar *
    get_command(void){ return (const gchar *)command_;}

    const gchar *
    get_tip(void){ return (const gchar *)tip_;}

    gint
    get_pid(void){ return (gint)pid_;}

    gint
    get_grandchild(void){ return (gint)grandchild_;}

    void
    create_menu(void){
	const gchar *items[]={N_("Renice Process"),N_("Suspend (STOP)"),N_("Continue (CONT)"),
	    N_("Interrupt (INT)"),N_("Hangup (HUP)"),N_("User 1 (USR1)"),
	    N_("User 2 (USR2)"),N_("Terminate (TERM)"),N_("Kill (KILL)"),
	    N_("Segmentation fault"),NULL};
	gint signals[] = {
	    -1,
	    SIGSTOP, SIGCONT, SIGINT, SIGHUP, SIGUSR1, 
	    SIGUSR2, SIGTERM, SIGKILL, SIGSEGV};


	
	menu_ = gtk_menu_new();
	const gchar **p = items;
	gint i;
	for (i=0;p && *p; p++,i++){
	    GtkWidget *v = gtk_menu_item_new_with_label (_(*p));
	    g_object_set_data(G_OBJECT(v), "run_button_p", (void *)this);
	    fprintf(stderr, "set data: (%p) -> %p\n", (void *)v, (void *)this);
	    gtk_container_add (GTK_CONTAINER (menu_), v);
	    g_signal_connect ((gpointer) v, "activate", MENUITEM_CALLBACK (RunButtonSignals<Type>::ps_signal), GINT_TO_POINTER(signals[i]));
	    gtk_widget_show (v);
	}
	gtk_widget_show (menu_);
	return ;
    }

    void
    run_button_setup (void){

	// Little icon assignment
	// Shell commands come from lpterminal (with specific shell characters).
	
	if (in_shell) icon_id_ = g_strdup("utilities-terminal");
	else {
	    command_ = g_strstrip(command_);
	    gchar **args = g_strsplit(command_, " ", -1);
	    if (args && args[0]) {
		icon_id_ = g_path_get_basename(args[0]);
		TRACE("RunButton::run_button_setup: attempting icon for \"%s\"\n", icon_id_);
	    }
	    else icon_id_ = g_strdup("emblem-run");
	    g_strfreev(args);
	}


	tip_ = g_strdup_printf(" %s=%d\n", _("PID"), grandchild_); 
	gint i=40;
	gint j=0;
	gchar buffer[2048]; memset(buffer, 0, 2048);
	do {
	    if (i>strlen(command_)) i=strlen(command_);
	    strncat(buffer, command_+j, i);
	    j += i;
	    if (j<strlen(command_))strcat(buffer, "\n");
	} while (j<strlen(command_));
	gchar *g = g_strconcat(tip_, buffer, NULL);
	g_free(tip_);
	tip_ = g;
	TRACE("RunButton::new_run_button: icon_id_=%s  command_=%s pid_=%d grandchild_=%d tip_=%s\n", icon_id_, command_, pid_, grandchild_, tip_);
	return ;
    }
////////////////////////////////////////////////////////////////////

    void *
    make_run_data_button (void *data) {
	auto run_button_p = (RunButton<Type> *)data;
	GtkWidget *button_ = gtk_menu_button_new ();

	//GMenuModel *menu_ = run_button_p->get_signal_menu_model();
	//fprintf(stderr, "make_run_data_button: menu_ model is %p\n", menu_);
	//gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (button_), menu_);
	gtk_menu_button_set_popup (GTK_MENU_BUTTON (button_),  run_button_p->get_menu());


	const gchar *icon = run_button_p->get_icon_id();

	// Test for validity of icon
	if (!icon || !pixbuf_c::find_pixbuf(icon, -16)){
	    run_button_p->set_icon_id("emblem-run");
	} 
	run_button_setup();
	
	TRACE("make_run_data_button: icon_id_=\"%s\" tip_=\"%s\"\n", run_button_p->get_icon_id(), run_button_p->get_tip());

	gtk_c::setup_image_button(button_, run_button_p->get_icon_id(), run_button_p->get_tip());
	//g_signal_connect(button_, "toggled", G_CALLBACK (run_button_toggled), data);
	gtk_box_pack_end (GTK_BOX (pageChild_p->get_button_space()), button_, FALSE, FALSE, 0);
	gtk_widget_show (button_);
	// flush gtk
	while (gtk_events_pending()) gtk_main_iteration();
	TRACE ("make_run_data_button: button_ made for grandchildPID=%d\n", (int)run_button_p->get_pid());
	return NULL;
    }


    void *
    run_wait_f (void *data) {
	auto run_button_p = (RunButton<Type> *) data;
	/* create little button (note: thread protected within subroutine) */
	// The following function will not return until button is created and duly
	// processed by the gtk loop. This to avoid a race with the command_ completing
	// before gtk has fully created the little run button.
	//
	util_c::context_function(make_run_data_button, data);
	TRACE("run_wait_f: thread waitpid for %d on (%s/%s)\n", 
		run_button_p->get_pid(), 
		run_button_p->get_command(), 
		run_button_p->get_workdir());

	pageChild_p->reference_run_button(run_button_p);
	
	gint status;
	waitpid (run_button_p->get_pid(), &status, 0);

	TRACE("run_wait_f: thread waitpid for %d complete!\n", run_button_p->get_pid());
	/* remove little button */
	
#ifdef DEBUG_TRACE    
	// This is out of sync here (grayball), so only in debug mode.
	print_c::print_icon(textview, "emblem-grayball", g_strdup_printf("%s %d/%d\n", run_button_p->get_command(),
		run_button_p->get_pid(), run_button_p->get_grandchild()));
#endif
	
	/// FIXME: the following code is to signal the background monitor
	//         currently no monitor is enabled.
	// Run has completed.  
	// no use for controller process anymore....
	TRACE("run has completed: %d\n", run_button_p->get_pid());
	// FIXME: what does rfm_remove_child do??
	//        remove controller!
	// FIXME: enable
	// rfm_remove_child(run_data_p->pid);
	// Flush pipes.
	fflush(NULL);  
	// Destroy little button (if exists) and free run_data_p 
	// associated memory. Done in main thread for gtk instruction set.
	util_c::context_function(zap_run_button, data);
	return NULL;
    }

    void *
    zap_run_button(void * data){
	TRACE("zap_run_button...\n");
	auto run_button_p = (RunButton<Type> *)data;
	pageChild_p->unreference_run_button(run_button_p);
	return NULL;
    }

    ////////////////////////  app action callbacks ///////////////////////////////////

    static glong
    shell_child_pid(glong pid){
	gchar *pcommand = g_strdup_printf("ps ax -o ppid,pid");
	FILE *p = popen(pcommand, "r");
	if (!p){
	    g_warning("pipe creation failed for %s\n", pcommand);
	    g_free(pcommand);
	    return pid;
	} 
	g_free(pcommand);

	DBG("** looking for shell child of %ld\n", pid);
	glong cpid = -1;
	gchar *spid = g_strdup_printf("%ld", pid);
	gchar buffer[64];
	memset(buffer, 0, 64);

	while (fgets(buffer, 64, p) && !feof(p)){
	    if (!strstr(buffer, spid)) continue;
	    g_strstrip(buffer);
	    if (strncmp(buffer, spid, strlen(spid))==0){
		DBG("** shell_child_pid(): gotcha shell pid: \"%s\"\n", buffer);
		memset(buffer, ' ', strlen(spid));
		g_strstrip(buffer);
		errno = 0;
		glong l = strtol(buffer, NULL, 10);
		if (errno) {
		    g_warning("strtol() cannot parse: %s\n", buffer);
		    pclose(p);
		    g_free(spid);
		    return pid;
		}
		cpid = l;
		break;
	    }
	}
	pclose(p);
	g_free(spid);
	// If cpid turns out > 0, then we are in a chained command_ and pid must change
	if (cpid > 0) return cpid;
	return pid;
    }

    void
    ps_renice(void){

	glong pid = get_grandchild();
	if (in_shell) pid = shell_child_pid(pid);

	gchar *command = g_strdup_printf("renice +1 -p %ld", pid);
	run_c::shell_command(textview, command, FALSE);
	// Here we do not need to save "$command" to history...
	g_free(command);

	return;
    }
////////////////////////////////////////////////////////////////////

    static void send_signal(GtkWidget *w, void *data){
	auto run_button_p = (RunButton<Type> *)data;
	DBG("send_signal %d to %d\n", 
		GPOINTER_TO_INT(g_object_get_data(G_OBJECT(w),"signal_id")), 
		run_button_p->get_grandchild());


    }

    static void
    show_run_info (GtkButton * button, gpointer data) {
	auto run_button_p = (RunButton<Type> *)data;
	//FIXME: we also need to be able to signal to child of shell, if launched from shell
	TRACE("FIXME: popup signal menu or dialog here.\n");

	
    }
    
public:
    void sendSignal(gint signal_id){
	glong pid = get_grandchild();
	if (!pid) return;
	// Do we need sudo?
	gboolean sudoize = FALSE;
	if (strncmp(get_command(), "sudo", strlen("sudo"))==0) sudoize = TRUE;
	// Are we running in a shell?
	if (in_shell || sudoize){
	    DBG("shell child pid required...\n");
	    pid = shell_child_pid(pid);
	}
	    
	fprintf(stderr, "signal to pid: %ld (in_shell=%d sudo=%d)\n", pid, in_shell, sudoize);
	if (sudoize) {
	    //        1.undetached child will remain as zombie
	    //        2.sudo will remain in wait state and button will not disappear
	    // hack: if signal is kill, kill sudo in the same command
	    //       eliminate zombie...
	    gchar *sudo = g_find_program_in_path("sudo");
	    if (!sudo){
		g_warning("sudo not found in path\n");
		return;
	    }
	    gchar *command;
	    if (signal_id == SIGKILL) {
		command =  g_strdup_printf("%s -A kill -%d %ld %d", sudo, signal_id, pid, get_grandchild());
	    } else {
		command =  g_strdup_printf("%s -A kill -%d %ld", sudo, signal_id, pid);
	    }
	    Run<Type>::shell_command(textview, command, FALSE);
	    // Again, when we signal process, there is no need to save command
	    // in the csh history file.
	    g_free(command);
	    g_free(sudo);
	} else {
	    DBG("normal ps_signal to %d...\n", (int)pid);
	    print_c::print_icon(textview, "emblem-important", "tag/blue", g_strdup_printf("kill -%d %ld\n",
		    signal_id, pid));
	    kill((pid_t)pid, signal_id);
	}
    }



};
}

#endif


