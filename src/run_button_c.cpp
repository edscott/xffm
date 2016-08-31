#include <tubo.h>
#include "view_c.hpp"
#include "window_c.hpp"
#include "run_button_c.hpp"


typedef struct thread_run_t {
    char *wd;
    char **argv;
} thread_run_t;

run_button_c::~run_button_c(void){
   g_free(command);
   g_free(icon);
}

run_button_c::run_button_c(void *data, const gchar * exec_command, pid_t child){
    view_v = data;
    TRACE ("run_button_c::setup_run_button_thread: grandchildPID=%d\n", (int)child);
    pid = child;
    grandchild = Tubo_child(child);
    command = g_strdup (exec_command);
    // Little icon assignment
    // Shell commands come from lpterminal.
    gchar **args = g_strsplit(exec_command, " ", -1);
    if (args && args[0]) {
    // FIXME: rfm_shell should be an xffm method
	const gchar *shell = rfm_shell(); 
	if (strcmp(shell, args[0])==0) icon = g_strdup("utilities-terminal");
	else {
	    icon = g_path_get_basename(args[0]);
	    TRACE("run_button_c::setup_run_button_thread: %s : %s\n", icon, exec_command);
            // FIXME: if icon cannot be created from name, then use "emblem-run"
	}
    }
    g_strfreev(args);
    // FIXME: this should be a view method
    //rfm_view_thread_create(view_p, run_wait_f, (void *) this, "run_wait_f");
}

const gchar *
run_button_c::rfm_shell(void){
    view_c *view_p = (view_c *)view_v;
    window_c *window_p = (window_c *)(view_p->window_v);
    return window_p->xffm_shell();
}

#if 0


static void
setup_run_button_thread (widgets_t * widgets_p, const gchar * exec_command, pid_t child) {
    view_t *view_p = widgets_p->view_p;
    rfm_view_thread_create(view_p, run_wait_f, (gpointer) run_data_p, "run_wait_f");
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

static void
run_wait_f (void *data) {
    run_button_c *run_button_p = (run_button_c *) data;

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
    gchar *t = run_start_string(run_data_p->command, run_data_p->controller, FALSE);
    print_icon("emblem-grayball", t);
#endif
    
    rfm_global_t *rfm_global_p = rfm_global();
    if (rfm_global_p) {
        TRACE("run_wait_f(): trigger reload...\n");
        // Trigger reload to all tabbed views.
        GSList **list_p = rfm_view_list_lock(NULL, "run_wait_f");
        if (!list_p) return NULL;
        g_mutex_lock(rfm_global_p->status_mutex);
        gint status = rfm_global_p->status;
        g_mutex_unlock(rfm_global_p->status_mutex);
        if (status == STATUS_EXIT) {
            rfm_view_list_unlock("run_wait_f");
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
        rfm_view_list_unlock("2 run_wait_f");
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

static pthread_mutex_t fork_mutex=PTHREAD_MUTEX_INITIALIZER;

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

#endif
