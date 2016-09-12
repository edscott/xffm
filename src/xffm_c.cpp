#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "xffm_c.hpp"
#include "window_c.hpp"
static void activate(GtkApplication *, void *);
static void startup(GtkApplication *, void *);  
static void shutdown(GtkApplication *, void *);
static void open (GtkApplication *, gpointer, gint, gchar *, gpointer);

xffm_c::xffm_c(gint in_argc, gchar **in_argv){
    gtk_p = NULL;
    argc = in_argc;
    argv = in_argv;
    app = gtk_application_new (NULL, G_APPLICATION_HANDLES_OPEN);

    window_p_list = NULL;
    g_signal_connect (app, "activate", G_CALLBACK (activate), (void *)this);
    g_signal_connect (app, "open", G_CALLBACK (open), (void *)this);
    g_signal_connect (app, "shutdown", G_CALLBACK (shutdown), (void *)this);
    g_signal_connect (app, "startup", G_CALLBACK (startup), (void *)this);
}

void
xffm_c::set_signal_menu_model(GMenuModel *data){
    signal_menu_model = data;
}

GMenuModel *
xffm_c::get_signal_menu_model(void){ return signal_menu_model;}

gint 
xffm_c::run(void){
    return g_application_run (G_APPLICATION (app), argc, argv);
};

    
xffm_c::~xffm_c(void){
    DBG("xffm_c::~xffm_c\n");
    GList *l = window_p_list;
    for (;l && l->data; l=l->next){
	window_c *window_p = (window_c *)l->data;
	delete window_p;
    }
    if (gtk_p) delete gtk_p;
}  

window_c * 
xffm_c::add_window_p(void){
    window_c *window_p = new window_c(app, gtk_p);
    if (!gtk_p) gtk_p = window_p->get_gtk_p();
    
    // initial view...
    window_p->create_new_page(g_get_home_dir());
    gtk_application_add_window (app, window_p->get_window());
    window_p_list = g_list_append(window_p_list, (void *)window_p);
    g_object_set_data(G_OBJECT(window_p->get_window()), "signal_menu_model", (void *)signal_menu_model);
    return window_p;
}

window_c * 
xffm_c::add_window_p(const gchar *data){
    window_c *window_p = new window_c(app, gtk_p);
    if (!gtk_p) gtk_p = window_p->get_gtk_p();
    // initial view...
    window_p->create_new_page(data);
    gtk_application_add_window (app, window_p->get_window());
    window_p_list = g_list_append(window_p_list, (void *)window_p);
    g_object_set_data(G_OBJECT(window_p->get_window()), "signal_menu_model", (void *)signal_menu_model);
    return window_p;
}

static void
activate(GtkApplication *app, void *data){
    DBG("activate(GtkApplication *app, void *data)\n");
    xffm_c *xffm_p = (xffm_c *)data;
    xffm_p->add_window_p();
}



static void
send_signal (GSimpleAction *action,
                       GVariant      *parameter,
                       gpointer       app)
{
    gint pid = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(app), "signal_pid"));
    DBG("send_signal to %d\n", pid);
}

static void
process_info (GSimpleAction *action,
                       GVariant      *parameter,
                       gpointer       app)
{
    DBG("process_info\n");
}
// FIXME: one callback for each signal. Yeah, that is not quite
//        efficient, but what the heck...
static GActionEntry app_entries[] =
{
  { "send_signal", send_signal, NULL, NULL, NULL },
  { "process_info", process_info, NULL, NULL, NULL }
};

static void
startup(GtkApplication *app, void *data){
  xffm_c *xffm_p = (xffm_c *)data;  
  DBG("startup(GtkApplication *app, void *data)\n");
  GtkBuilder *builder;

  //G_APPLICATION_CLASS (menu_button_parent_class)->startup (application);

  g_action_map_add_action_entries (G_ACTION_MAP (app), app_entries, G_N_ELEMENTS (app_entries), app);
 
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
"        <attribute name='action'>app.send_signal</attribute>"
"      </item>"
"      <item>"
"        <attribute name='label' translatable='yes'>Suspend (STOP)</attribute>"
"        <attribute name='action'>app.send_signal</attribute>"
"      </item>"
"      <item>"
"        <attribute name='label' translatable='yes'>Continue (CONT)</attribute>"
"        <attribute name='action'>app.send_signal</attribute>"
"      </item>"
"      <item>"
"        <attribute name='label' translatable='yes'>Interrupt (INT)</attribute>"
"        <attribute name='action'>app.send_signal</attribute>"
"      </item>"
"      <item>"
"        <attribute name='label' translatable='yes'>Hangup (HUP)</attribute>"
"        <attribute name='action'>app.send_signal</attribute>"
"      </item>"
"      <item>"
"        <attribute name='label' translatable='yes'>User 1 (USR1)</attribute>"
"        <attribute name='action'>app.send_signal</attribute>"
"      </item>"
"      <item>"
"        <attribute name='label' translatable='yes'>User 2 (USR2)</attribute>"
"        <attribute name='action'>app.send_signal</attribute>"
"      </item>"
"      <item>"
"        <attribute name='label' translatable='yes'>Terminate (TERM)</attribute>"
"        <attribute name='action'>app.send_signal</attribute>"
"      </item>"
"      <item>"
"        <attribute name='label' translatable='yes'>Kill (KILL)</attribute>"
"        <attribute name='action'>app.send_signal</attribute>"
"      </item>"
"      <item>"
"        <attribute name='label' translatable='yes'>Segmentation fault</attribute>"
"        <attribute name='action'>app.send_signal</attribute>"
"      </item>"
"    </section>"
"  </menu>"
"</interface>", -1, NULL);
  GMenuModel *signal_menu_model = G_MENU_MODEL (gtk_builder_get_object (builder, "signal-menu"));
    xffm_p->set_signal_menu_model(signal_menu_model);

  gtk_application_set_app_menu (GTK_APPLICATION (app), G_MENU_MODEL (gtk_builder_get_object (builder, "signal-menu")));
  g_object_unref (builder);
}


static void
shutdown(GtkApplication *app, void *data){
  DBG("shutdown(GtkApplication *app, void *data)\n");
}

static gchar *
get_dirname(GFile *file){
    gchar *path = g_file_get_path(file);
    if (!g_file_test(path, G_FILE_TEST_IS_DIR)){
        gchar *g = g_path_get_dirname(path);
        g_free(path);
        path = g;
    }
    return path;
}

static void
open (GtkApplication *app,
               gpointer      files,
               gint          n_files,
               gchar        *hint,
               gpointer      data){
//    open(GtkApplication *app, void *data){
    DBG("open(GtkApplication *app, void *data)\n");
    xffm_c *xffm_p = (xffm_c *)data;
    if (!n_files){
	xffm_p->add_window_p();	
        return;
    }    
    GFile **array =(GFile **)files;
    gchar *path = get_dirname(array[0]);
    window_c *window_p = xffm_p->add_window_p(path);	
    g_free(path);
    gint i;  
    for (i=1; i<n_files; i++){
        // add extra notebook page
        path = get_dirname(array[i]);
        window_p->create_new_page(path);
        g_free(path);
    }
}

void
xffm_c::remove_window_p_from_list(void *data){
    window_p_list = g_list_remove(window_p_list, data);
}

static 
void
ps_signal(gint pid, gint signal_id){
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
    gint sig=signal_id;
    if (sudoize && sudo) {
        DBG("ps_signal by means of sudo...\n");
	// FIXME
        /*widgets_t *widgets_p = rfm_get_widget ("widgets_p");
        gchar *signal_number = g_strdup_printf("-%0d", sig);
        gchar *argv[]={sudo, "-A", "kill", signal_number, spid, NULL};
        rfm_thread_run_argv (widgets_p, argv, FALSE);
        g_free(signal_number);*/
    } else {
        if (!sudo) g_warning("sudo command not found to signal non-owned process\n");
        DBG("normal ps_signal to %d...\n", (int)pid);
        kill(pid, sig);
    }
    //rfm_rational(RFM_MODULE_DIR, "callbacks", GINT_TO_POINTER(REFRESH_ACTIVATE), NULL, "callback");
    g_free(sudo);
    g_free(spid);
    return;
}


