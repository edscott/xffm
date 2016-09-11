#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <tubo.h>
#define DEBUG_TRACE 1
#include "view_c.hpp"
#include "window_c.hpp"
#include "run_button_c.hpp"

static void *run_wait_f (void *);
static void *make_run_data_button (void *);
static void *zap_run_button(void *);
static void show_run_info (GtkButton *, gpointer);

typedef struct thread_run_t {
    char *wd;
    char **argv;
} thread_run_t;

run_button_c::~run_button_c(void){
    TRACE("run_button_c::~run_button_c... button %p\n", (void *)button);
    if(button && GTK_IS_WIDGET(button)){
        gtk_widget_hide(GTK_WIDGET (button));
        gtk_widget_destroy (GTK_WIDGET (button));
    }
    g_free (tip);
    g_free (command);
    g_free (icon_id);
    g_free (workdir);
}

run_button_c::run_button_c(void *data, const gchar * exec_command, pid_t child, gboolean shell_wrap){
    view_v = data;
    in_shell = shell_wrap;

    pid = child;
    grandchild = Tubo_child(child);
    command = g_strdup (exec_command);
    icon_id = NULL;
    workdir = NULL; // FIXME
    tip = NULL;
    TRACE ("run_button_c::setup_run_button_thread: controller/process=%d/%d\n", (int)child, (gint)grandchild);

    view_c *view_p =(view_c *)view_v;
    pthread_t *thread = view_p->thread_create("run_button_c::run_button_c: run_wait_f", 
            run_wait_f, (void *) this, FALSE);
}

gtk_c *
run_button_c::get_gtk_p(void){
    view_c *view_p =(view_c *)view_v;
    return (view_p->get_gtk_p());
}


void
run_button_c::set_icon_id(const gchar *data){
    g_free(icon_id); 
    icon_id = data?g_strdup(data):NULL;
}

const gchar *
run_button_c::get_icon_id(void){ return (const gchar *)icon_id;}

const gchar *
run_button_c::get_command(void){ return (const gchar *)command;}

const gchar *
run_button_c::get_workdir(void){ return (const gchar *)workdir;}

const gchar *
run_button_c::get_tip(void){ return (const gchar *)tip;}

gint
run_button_c::get_pid(void){ return (gint)pid;}

gint
run_button_c::get_grandchild(void){ return (gint)grandchild;}

void *
run_button_c::get_view_v(void){ return view_v;}

GtkApplication *
run_button_c::get_app(void){
    view_c *view_p =(view_c *)view_v;
    return view_p->get_app();}

void
run_button_c::run_button_setup (GtkWidget *data){
    view_c *view_p =(view_c *)view_v;
    button = data;

    // Little icon assignment
    // Shell commands come from lpterminal (with specific shell characters).
    
    if (in_shell) icon_id = g_strdup("utilities-terminal");
    else {
        command = g_strstrip(command);
        gchar **args = g_strsplit(command, " ", -1);
        if (args && args[0]) {
            icon_id = g_path_get_basename(args[0]);
            TRACE("run_button_c::run_button_setup: attempting icon for \"%s\"\n", icon_id);
        }
        else icon_id = g_strdup("emblem-run");
        g_strfreev(args);
    }


    tip = g_strdup_printf(" %s=%d\n", _("PID"), grandchild); 
    gint i=40;
    gint j=0;
    gchar buffer[2048]; memset(buffer, 0, 2048);
    do {
        if (i>strlen(command)) i=strlen(command);
        strncat(buffer, command+j, i);
        j += i;
        if (j<strlen(command))strcat(buffer, "\n");
    } while (j<strlen(command));
    gchar *g = g_strconcat(tip, buffer, NULL);
    g_free(tip);
    tip = g;
    TRACE("run_button_c::new_run_button: icon_id=%s  command=%s pid=%d grandchild=%d tip=%s\n", icon_id, command, pid, grandchild, tip);
    return ;
}

#define DO_POPOVER
#ifdef DO_POPOVER

 // for popover...
static gchar *
get_menu_xml(void){
    const gchar *items[]={N_("Renice Process"),N_("Suspend (STOP)"),N_("Continue (CONT)"),
        N_("Interrupt (INT)"),N_("Hangup (HUP)"),N_("User 1 (USR1)"),
        N_("User 2 (USR2)"),N_("Terminate (TERM)"),N_("Kill (KILL)"),
        N_("Segmentation fault"),NULL};
    gchar *menu_string = g_strdup_printf("<interface><menu id=\"signals_menu\"><section>");
    const gchar **p;
    gchar *g;
    for (p=items; p&& *p; p++){
        g = g_strdup_printf("%s\n<item>\n<attribute name=\"label\" translatable=\"yes\">%s</attribute>", 
                menu_string, *p);
        g_free(menu_string);
        menu_string = g;
        g = g_strdup_printf("%s\n<attribute name=\"action\">app.send_signal</attribute>\n</item>", menu_string);
        g_free(menu_string);
        menu_string = g;
    }
    g = g_strdup_printf("%s\n</section></menu></interface>", menu_string); 
    g_free(menu_string);
    menu_string = g;
    return menu_string;
}
#endif

void send_signal(GtkWidget *w, void *data){
    
    run_button_c *run_button_p = (run_button_c *)data;
    DBG("send_signal %d to %d\n", 
            GPOINTER_TO_INT(g_object_get_data(G_OBJECT(w),"signal_id")), 
            run_button_p->get_grandchild());

#if 0

static 
void
ps_signal(GtkMenuItem *m, gpointer data){
    pid_t pid =  GPOINTER_TO_INT(rfm_get_widget("ps_uid"));
    if (!pid) return;
    // Is the effective runtime uid the same as that for the process to signal?
    
    // 1. Are we executing the command in a shell chain?
    gchar *pcommand = g_strdup_printf("ps ax -o ppid,pid");
    FILE *p = popen(pcommand, "r");
    if (!p){
        g_warning("pipe creation failed for %s\n", pcommand);
        g_free(pcommand);
        return;
    } 
    g_free(pcommand);
    gint cpid = -1;
    gchar *spid = g_strdup_printf("%0d", pid);
    gchar buffer[64];
    memset(buffer, 0, 64);
    while (fgets(buffer, 63, p) && !feof(p)){
        if (strncmp(buffer, spid, strlen(spid))==0){
            DBG("gotcha: %s", buffer);
            gchar **gg=g_strsplit(buffer, " ", -1);
            errno = 0;
            long l = strtol(gg[1], NULL, 10);
            if (errno) {
                g_warning("cannot parse to long: %s\n", gg[1]);
                pclose(p);
                g_free(spid);
                g_strfreev(gg);
                return;
            }
            cpid = l;
            g_strfreev(gg);
            break;
        }
    }
    pclose(p);
    // If cpid turns out > 0, then we are in a chained command and pid must change
    if (cpid > 0) {
        pid = cpid;
        g_free(spid);
        spid = g_strdup_printf("%0d", pid);
    }

    // 2. Does pid (either direct or from chained command) belong to us?
    pcommand = g_strdup_printf("ps -p %d -o uid", (int)pid);
    gboolean sudoize = FALSE;
    uid_t uid = geteuid();
    p = popen(pcommand, "r");
    long luid = geteuid();
    if (!p){
        g_warning("pipe creation failed for %s\n", pcommand);
    } else {
        gchar buffer[64];
        memset(buffer, 0, 64);
        while (fgets(buffer, 63, p) && !feof(p)){
	    if (strstr(buffer, "UID")) continue;
            errno=0;
            luid = strtol(buffer, NULL, 10);
            if (!errno){
		DBG("line: %s gotcha: %ld\n", buffer, luid);
                if (luid != uid) sudoize = TRUE;
                break;
            }
        }
        pclose(p);
    }
    g_free(pcommand);
    gchar *sudo = g_find_program_in_path("sudo");
    DBG("signal to pid: %s (owned by %d, sudo=%d)\n", spid, (int)luid, sudoize);
    gint sig=GPOINTER_TO_INT(data);
    if (sudoize && sudo) {
        DBG("ps_signal by means of sudo...\n");
        widgets_t *widgets_p = rfm_get_widget ("widgets_p");
        gchar *signal_number = g_strdup_printf("-%0d", sig);
        gchar *argv[]={sudo, "-A", "kill", signal_number, spid, NULL};
        rfm_thread_run_argv (widgets_p, argv, FALSE);
        g_free(signal_number);
    } else {
        if (!sudo) g_warning("sudo command not found to signal non-owned process\n");
        DBG("normal ps_signal to %d...\n", (int)pid);
        kill(pid, sig);
    }
    rfm_rational(RFM_MODULE_DIR, "callbacks", GINT_TO_POINTER(REFRESH_ACTIVATE), NULL, "callback");
    g_free(sudo);
    g_free(spid);
    return;
}



#endif

}
static void
run_button_toggled(GtkWidget *button, void *data){
    run_button_c *run_button_p = (run_button_c *)data;
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button))){
	// here we set the parameter for the signal actions.
	TRACE("run button toggled: pid=%d\n", run_button_p->get_grandchild());
	g_object_set_data(G_OBJECT(run_button_p->get_app()), 
		"signal_pid", 
		GINT_TO_POINTER(run_button_p->get_grandchild())
		);
    }
}

static void *
make_run_data_button (void *data) {
    run_button_c *run_button_p = (run_button_c *)data;
    GtkWidget *button = gtk_menu_button_new ();
    //GtkWidget *button = gtk_button_new ();
    
#ifdef DO_POPOVER
    // popover is nice. but freaking problem to associate actions and parameters.
/*    gchar *signals_xml = get_menu_xml();
    GtkBuilder *builder = gtk_builder_new_from_string (signals_xml,-1);
    g_free(signals_xml);
    GMenuModel *menu = G_MENU_MODEL (gtk_builder_get_object (builder, "signals_menu"));
    g_object_unref(builder);
    gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (button), menu);
 
 */
    view_c *view_p = (view_c *)run_button_p->get_view_v();
    GMenuModel *menu = view_p->get_signal_menu_model();
    gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (button), menu);
#else
   
    GtkWidget *menu = run_button_p->make_menu();
    gtk_menu_button_set_popup (GTK_MENU_BUTTON(button),menu);
#endif

    run_button_p->run_button_setup(button);


    const gchar *icon = run_button_p->get_icon_id();

    // Test for validity of icon
    if (!icon || !view_p->get_gtk_p()->find_pixbuf(icon, -16)){
        run_button_p->set_icon_id("emblem-run");
    } 
    
    DBG("*** icon_id=%s tip=%s\n", run_button_p->get_icon_id(), run_button_p->get_tip());

    view_p->get_gtk_p()->setup_image_button(button, run_button_p->get_icon_id(), run_button_p->get_tip());
    g_signal_connect(button, "toggled", G_CALLBACK (run_button_toggled), data);
    gtk_box_pack_end (GTK_BOX (view_p->get_button_space()), button, FALSE, FALSE, 0);
    gtk_widget_show (button);
    // flush gtk
    while (gtk_events_pending()) gtk_main_iteration();
    TRACE ("DIAGNOSTICS:srun_button made for grandchildPID=%d\n", (int)run_button_p->get_pid());


    return NULL;
}


static void *
run_wait_f (void *data) {
    run_button_c *run_button_p = (run_button_c *) data;

    /* create little button (note: thread protected within subroutine) */
    // The following function will not return until button is created and duly
    // processed by the gtk loop. This to avoid a race with the command completing
    // before gtk has fully created the little run button.

    
    
    run_button_p->context_function(make_run_data_button, data);
    TRACE("run_wait_f: thread waitpid for %d on (%s/%s)\n", 
            run_button_p->get_pid(), 
            run_button_p->get_command(), 
            run_button_p->get_workdir());

    view_c *view_p = (view_c *)run_button_p->get_view_v();
    view_p->get_lpterm_p()->reference_run_button(run_button_p);
    
    gint status;
    waitpid (run_button_p->get_pid(), &status, 0);

    TRACE("run_wait_f: thread waitpid for %d complete!\n", run_button_p->get_pid());
    /* remove little button */
    
#ifdef DEBUG_TRACE    
    // This is out of sync here (grayball), so only in debug mode.
    view_p->get_lpterm_p()->print_icon("emblem-grayball", "%s %d/%d\n", run_button_p->get_command(),
            run_button_p->get_pid(), run_button_p->get_grandchild());
#endif
    
    /// FIXME: the following code is to signal the background monitor
    //         currently no monitor is enabled.
#if 0
    /// FIXME
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

#endif
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
    run_button_p->context_function(zap_run_button, data);
    return NULL;
}


static void *
zap_run_button(void * data){
    TRACE("zap_run_button...\n");
    run_button_c *run_button_p = (run_button_c *)data;
    view_c *view_p = (view_c *)run_button_p->get_view_v();
    view_p->get_lpterm_p()->unreference_run_button(run_button_p);
    return NULL;
}

static void
show_run_info (GtkButton * button, gpointer data) {
    run_button_c *run_button_p = (run_button_c *)data;
    //guint button_id = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(button), "button_id"));
    //
    //FIXME: popup signal menu or dialog here.
    //FIXME: we also need to be able to signal to child of shell, if launched from shell
    TRACE("FIXME: popup signal menu or dialog here.\n");
//	if (button_id == 3) return;
	
/*	gchar *text2 =g_strdup_printf ("%s %s: %s\n\n%s %s (%d)?",
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
	g_free (text2);*/
    
}

#if 0

GtkWidget *
run_button_c::make_menu(void){
    const gchar *items[]={N_("Renice Process"),N_("Suspend (STOP)"),N_("Continue (CONT)"),
        N_("Interrupt (INT)"),N_("Hangup (HUP)"),N_("User 1 (USR1)"),
        N_("User 2 (USR2)"),N_("Terminate (TERM)"),N_("Kill (KILL)"),
        N_("Segmentation fault"),NULL};
    const gchar *icons[]={"emblem-wait", "emblem-grayball","emblem-greenball",
        "emblem-exit","view-refresh","emblem-user",
        "emblem-user","emblem-cancel","emblem-redball",
        "emblem-core",NULL};

    GtkWidget *menu = gtk_menu_new();
    const gchar **i;
    const gchar **p;
    gint j;
    for (j=0,p=items, i=icons; p&& *p; p++,i++,j++){
        GtkWidget *menuitem = get_gtk_p()->menu_item_new(*i, *p);
        g_object_set_data(G_OBJECT(menuitem), "signal_id", GINT_TO_POINTER(j));
        g_signal_connect(menuitem, "activate", G_CALLBACK(send_signal), (void *)this);
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
        gtk_widget_show(menuitem);
    }

    gtk_widget_show(menu);
    return menu;
}



static void
setup_run_button_thread (widgets_t * widgets_p, const gchar * exec_command, pid_t child) {
    view_t *view_p = widgets_p->view_p;
    rfm_view_thread_create(view_p, run_wait_f, (gpointer) run_data_p, "run_wait_f");
}


static pthread_mutex_t fork_mutex=PTHREAD_MUTEX_INITIALIZER;

// This is main thread callback

#endif

#if 0


#endif
