//#define DEBUG_TRACE 1
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include "signal_action_c.hpp"
#include "view_c.hpp"
#include "run_button_c.hpp"
static void  renice_signal(GSimpleAction *, GVariant *, gpointer);
static void  stop_signal(GSimpleAction *, GVariant *, gpointer);
static void  cont_signal(GSimpleAction *, GVariant *, gpointer);
static void  int_signal(GSimpleAction *, GVariant *, gpointer);
static void  hup_signal(GSimpleAction *, GVariant *, gpointer);
static void  usr1_signal(GSimpleAction *, GVariant *, gpointer);
static void  usr2_signal(GSimpleAction *, GVariant *, gpointer);
static void  term_signal(GSimpleAction *, GVariant *, gpointer);
static void  kill_signal(GSimpleAction *, GVariant *, gpointer);
static void  segv_signal(GSimpleAction *, GVariant *, gpointer);
static void process_info(GSimpleAction *, GVariant *, gpointer);

signal_action_c::signal_action_c(GtkApplication *data){
    app = data;
    add_actions(app);
    create_menu_model(app);
    TRACE("signal_action_c::signal_action_c: menu model is %p\n", get_signal_menu_model());
}

GtkApplication *
signal_action_c::get_app(void){return app;}

void
signal_action_c::create_menu_model(GtkApplication *app){
    GtkBuilder *builder;
    builder = gtk_builder_new ();
    const gchar *items[]={N_("Renice Process"),N_("Suspend (STOP)"),N_("Continue (CONT)"),
        N_("Interrupt (INT)"),N_("Hangup (HUP)"),N_("User 1 (USR1)"),
        N_("User 2 (USR2)"),N_("Terminate (TERM)"),N_("Kill (KILL)"),
        N_("Segmentation fault"),NULL};
    gtk_builder_add_from_string (builder,
"<interface>"
"  <menu id='signal-menu'>"
"    <section>"
"      <item>"
"        <attribute name='label' translatable='yes'>Information</attribute>"
"        <attribute name='action'>app.process_info</attribute>"
"      </item>"
"      <item>"
"        <attribute name='label' translatable='yes'>Renice Process</attribute>"
"        <attribute name='action'>app.renice_signal</attribute>"
"      </item>"
"      <item>"
"        <attribute name='label' translatable='yes'>Suspend (STOP)</attribute>"
"        <attribute name='action'>app.stop_signal</attribute>"
"      </item>"
"      <item>"
"        <attribute name='label' translatable='yes'>Continue (CONT)</attribute>"
"        <attribute name='action'>app.cont_signal</attribute>"
"      </item>"
"      <item>"
"        <attribute name='label' translatable='yes'>Interrupt (INT)</attribute>"
"        <attribute name='action'>app.int_signal</attribute>"
"      </item>"
"      <item>"
"        <attribute name='label' translatable='yes'>Hangup (HUP)</attribute>"
"        <attribute name='action'>app.hup_signal</attribute>"
"      </item>"
"      <item>"
"        <attribute name='label' translatable='yes'>User 1 (USR1)</attribute>"
"        <attribute name='action'>app.usr1_signal</attribute>"
"      </item>"
"      <item>"
"        <attribute name='label' translatable='yes'>User 2 (USR2)</attribute>"
"        <attribute name='action'>app.usr2_signal</attribute>"
"      </item>"
"      <item>"
"        <attribute name='label' translatable='yes'>Terminate (TERM)</attribute>"
"        <attribute name='action'>app.term_signal</attribute>"
"      </item>"
"      <item>"
"        <attribute name='label' translatable='yes'>Kill (KILL)</attribute>"
"        <attribute name='action'>app.kill_signal</attribute>"
"      </item>"
"      <item>"
"        <attribute name='label' translatable='yes'>Segmentation fault</attribute>"
"        <attribute name='action'>app.segv_signal</attribute>"
"      </item>"
"    </section>"
"  </menu>"
"</interface>", -1, NULL);
    signal_menu_model = G_MENU_MODEL (gtk_builder_get_object (builder, "signal-menu"));
    
    gtk_application_set_app_menu (GTK_APPLICATION (app), G_MENU_MODEL (gtk_builder_get_object (builder, "signal-menu")));
    g_object_unref (builder);
}

void 
signal_action_c::set_signal_action_parameter(void *data){
    g_object_set_data(G_OBJECT(app), "run_button_p", data);
}    

void
signal_action_c::add_actions(GtkApplication *app){
    GActionEntry app_entries[] =
    {
      { "process_info", process_info, NULL, NULL, NULL },
      { "renice_signal", renice_signal, NULL, NULL, NULL },
      { "stop_signal", stop_signal, NULL, NULL, NULL },
      { "cont_signal", cont_signal, NULL, NULL, NULL },
      { "int_signal",  int_signal, NULL, NULL, NULL },
      { "hup_signal",  hup_signal, NULL, NULL, NULL },
      { "usr1_signal", usr1_signal, NULL, NULL, NULL },
      { "usr2_signal", usr2_signal, NULL, NULL, NULL },
      { "term_signal", term_signal, NULL, NULL, NULL },
      { "kill_signal", kill_signal, NULL, NULL, NULL },
      { "segv_signal", segv_signal, NULL, NULL, NULL }
    };
    g_action_map_add_action_entries (G_ACTION_MAP (app), app_entries, 
            G_N_ELEMENTS (app_entries), app);

}


GMenuModel *
signal_action_c::get_signal_menu_model(void){ return signal_menu_model;}


////////////////////////  app action callbacks ///////////////////////////////////

//    DBG("send_signal to %d signal %d\n", pid, g_variant_get_int32(parameter));

static void ps_signal(gpointer, gint);
static void ps_renice(gpointer);

static void
renice_signal(GSimpleAction *action, GVariant *parameter, gpointer app)
{
    ps_renice(app);
}

static void
stop_signal(GSimpleAction *action, GVariant *parameter, gpointer app)
{
    ps_signal(app, SIGSTOP);
}

static void
cont_signal(GSimpleAction *action, GVariant *parameter, gpointer app)
{
    ps_signal(app, SIGCONT);
}

static void
int_signal(GSimpleAction *action, GVariant *parameter, gpointer app)
{
    ps_signal(app, SIGINT);
}

static void
hup_signal(GSimpleAction *action, GVariant *parameter, gpointer app)
{
    ps_signal(app, SIGHUP);
}

static void
usr1_signal(GSimpleAction *action, GVariant *parameter, gpointer app)
{
    ps_signal(app, SIGUSR1);
}

static void
usr2_signal(GSimpleAction *action, GVariant *parameter, gpointer app)
{
    ps_signal(app, SIGUSR2);
}

static void
term_signal(GSimpleAction *action, GVariant *parameter, gpointer app)
{
    ps_signal(app, SIGTERM);
}

static void
kill_signal(GSimpleAction *action, GVariant *parameter, gpointer app)
{
    ps_signal(app, SIGKILL);
}

static void
segv_signal(GSimpleAction *action, GVariant *parameter, gpointer app)
{
    ps_signal(app, SIGSEGV);
}

static void
process_info (GSimpleAction *action,
                       GVariant      *parameter,
                       gpointer       app)
{
    DBG("process_info\n");
}

static gchar **
non_empty_strsplit(const gchar *input, const gchar *token){
    if (!strstr(input, token)) return NULL;        
    gchar *c = g_strdup(input);
    if (strchr(c, '\n')) *strchr(c, '\n') = 0;
    gchar **src = g_strsplit(c, token, -1);
    g_free(c);
    if (!src) return NULL;
    gchar **r= (gchar **)malloc((g_strv_length(src)+1)*sizeof(gchar *)); 
    if (!r) return NULL;
    memset (r, 0, (g_strv_length(src)+1)*sizeof(gchar *));
    gchar **p, **tgt;
    for (p=src, tgt=r; p && *p; p++) {
        if (strlen(*p)){
            *tgt = g_strdup(*p);
            tgt++;
        }
    }
    g_strfreev(src);
    return r;   
}


static gint
shell_child_pid(gint pid){
    gchar *pcommand = g_strdup_printf("ps ax -o ppid,pid");
    FILE *p = popen(pcommand, "r");
    if (!p){
        g_warning("pipe creation failed for %s\n", pcommand);
        g_free(pcommand);
        return pid;
    } 
    g_free(pcommand);

    DBG("** looking for shell child of %d\n", pid);
    gint cpid = -1;
    gchar *spid = g_strdup_printf("%d", pid);
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
            long l = strtol(buffer, NULL, 10);
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
    // If cpid turns out > 0, then we are in a chained command and pid must change
    if (cpid > 0) return cpid;
    return pid;
}


static void
ps_signal(gpointer app, gint signal_id){
    run_button_c *run_button_p = (run_button_c *)g_object_get_data(G_OBJECT(app), "run_button_p");
    gint pid = run_button_p->get_grandchild();
    if (!pid) return;
    // Do we need sudo?
    gboolean sudoize = FALSE;
    if (strncmp(run_button_p->get_command(), "sudo", strlen("sudo"))==0) sudoize = TRUE;
    // Are we running in a shell?
    if (run_button_p->in_shell || sudoize){
        DBG("shell child pid required...\n");
        pid = shell_child_pid(pid);
    }
        
    view_c *view_p = (view_c *) run_button_p->get_view_v();

    DBG("signal to pid: %d (in_shell=%d sudo=%d)\n", pid, run_button_p->in_shell, sudoize);
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
            command =  g_strdup_printf("%s -A kill -%d %d %d", sudo, signal_id, pid, run_button_p->get_grandchild());
        } else {
            command =  g_strdup_printf("%s -A kill -%d %d", sudo, signal_id, pid);
        }
        view_p->get_lpterm_p()->shell_command(command);
        g_free(command);
        g_free(sudo);
    } else {
        DBG("normal ps_signal to %d...\n", (int)pid);
        view_p->get_lpterm_p()->print_icon_tag("emblem-important", "tag/blue", "kill -%d %d\n",
                signal_id, pid);
        kill(pid, signal_id);
    }
    // FIXME: is this necessary?
    // rfm_rational(RFM_MODULE_DIR, "callbacks", GINT_TO_POINTER(REFRESH_ACTIVATE), NULL, "callback");
}

static void
ps_renice(gpointer app){
    run_button_c *run_button_p = (run_button_c *)g_object_get_data(G_OBJECT(app), "run_button_p");

    gint pid = run_button_p->get_grandchild();
    if (run_button_p->in_shell) pid = shell_child_pid(pid);

    gchar *command = g_strdup_printf("renice +1 -p %d", pid);
    view_c *view_p = (view_c *) run_button_p->get_view_v();
    view_p->get_lpterm_p()->shell_command(command);
    g_free(command);

    return;
}


