#ifndef RUNBUTTON_HH
#define RUNBUTTON_HH
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
typedef struct thread_run_t {
    char *wd;
    char **argv;
} thread_run_t;
*/
namespace xf {



class RunButton {
private:
    GtkTextView *textview_;
    GtkBox *button_space_;

    pid_t pid_;
    pid_t grandchild_;
    gchar *command_;
    gchar *tip_;
    gchar *icon_id_;
    GtkButton *button_;
    //FIXME GtkMenu *menu_;
    gboolean in_shell_;
     
public:    

    RunButton(void){
      TRACE("RunButton void\n");
    }
    void init(RunButton *runButton, const gchar *command, pid_t child){
      TRACE("RunButton for %s\n", exec_command);
        gboolean shellIcon = Run::run_in_shell(command);
        setup(runButton, command, child, shellIcon);
        RunButton::reference_run_button((void *)runButton);
    }
    
    static void *reference_run_button(void *rb_p){
        TRACE("reference_run_button(%p)\n", rb_p);
        pthread_mutex_lock(&rbl_mutex);
        run_button_list = g_list_prepend(run_button_list, rb_p);
        pthread_mutex_unlock(&rbl_mutex);
        return NULL;
    }

    static void
    unreference_run_button(void *rb_p){
        TRACE("unreference_run_button(%p)\n", rb_p);
        pthread_mutex_lock(&rbl_mutex);
        void *p = g_list_find(run_button_list, rb_p);
        if (p){
            run_button_list = g_list_remove(run_button_list, rb_p);
            delete ((RunButton *)rb_p);
        }
        pthread_mutex_unlock(&rbl_mutex);
    }
    static void
    ps_signal(GtkWidget *menuitem, gpointer data){

        auto run_button_p = (RunButton *)
            g_object_get_data(G_OBJECT(menuitem), "run_button_p");

        gint signal_id =GPOINTER_TO_INT(data);
        if (signal_id ==-1) {
            run_button_p->ps_renice();
            return;
        }

        TRACE("apply data: (%p) -> %p\n", (void *)menuitem, (void *)run_button_p);
        run_button_p->sendSignal(signal_id);
    }
    void setup(void *data, const gchar * exec_command, pid_t child, gboolean shellIcon)
    {
        //parent_ = page_->parent();
        //textview_ = page_->output();
        //button_space_ = page_->parent()->vButtonBox();
        
        in_shell_ = shellIcon;
        pid_ = child;
        grandchild_ = Tubo::getChild(child);
        command_ = g_strdup (exec_command);
        icon_id_ = NULL;
        tip_ = NULL;
        TRACE ("RunButton::setup_run_button_thread: controller/process=%d/%d\n", (int)child, (gint)grandchild_);

        create_menu();
        auto text = g_strdup_printf("RunButton::setup(): %s", command_);
        new(Thread)(text, run_wait_f, (void *) this);
        g_free(text);
    }


    ~RunButton(void){
        TRACE("RunButton::~RunButton... button_ %p\n", (void *)button_);
        //FIXME gtk_widget_hide(GTK_WIDGET(menu_));
        //FIXME gtk_widget_destroy(GTK_WIDGET(menu_));
        gtk_widget_set_visible(GTK_WIDGET (button_), FALSE);
        g_object_unref(G_OBJECT (button_));
        g_free (tip_);
        g_free (command_);
        g_free (icon_id_);
    }
    // read only
    gboolean inShell(void){return in_shell_;}
    gint pid(void){ return (gint)pid_;}
    gint grandchild(void){ return (gint)grandchild_;}
    //FIXME GtkMenu *menu(void){return menu_;}
    GtkBox *button_space(void){return button_space_;}
    // read/write
    void setButton(GtkButton *button){button_ = button;}
    const gchar *icon_id(void){ return (const gchar *)icon_id_;}
    void set_icon_id(const gchar *data){
        g_free(icon_id_); 
        icon_id_ = data?g_strdup(data):NULL;
    }
    const gchar *command(void){ return (const gchar *)command_;}
    void set_command(const gchar *command){
        g_free(command_);
        command_ = g_strdup(command); 
    }
    const gchar *tip(void){ return (const gchar *)tip_;}
    void set_tip(const gchar *tip){
        g_free(tip_);
        tip_ = g_strdup(tip); 
    }


    void create_menu(void){
      /* FIXME
        const gchar *items[]={N_("Renice Process"),N_("Suspend"),N_("Continue"),
            N_("Interrupt"),N_("Hangup"),N_("User 1 (USR1)"),
            N_("User 2 (USR2)"),N_("Terminate Task"),N_("Abort"),
            N_("Kill"),
            N_("Segmentation fault"),NULL};
        gint signals[] = {
            -1,
            SIGSTOP, SIGCONT, SIGINT, SIGHUP, SIGUSR1, 
            SIGUSR2, SIGTERM, SIGABRT, SIGKILL, SIGSEGV};

        
        menu_ = GTK_MENU(gtk_menu_new());
        GtkMenuItem *title = GTK_MENU_ITEM(gtk_c::menu_item_new(NULL, "")); 
        gtk_widget_set_sensitive(GTK_WIDGET(title), FALSE);
        gtk_widget_show (GTK_WIDGET(title));
        gtk_container_add (GTK_CONTAINER (menu_), GTK_WIDGET(title));
        // No good for commands with && or >
        gchar *markup1 = g_strdup( command_);
        for (auto p = markup1; p && *p; p++){
            if (*p == '&') *p='^';
            if (*p == '>') *p='!';
            if (*p == '<') *p='!';
        }
        
        gchar *markup = g_strdup_printf("<span color=\"blue\" size=\"larger\">%s\n</span><span color=\"red\" size=\"larger\">pid: %d</span>", markup1, grandchild_);
        g_free(markup1);        
        gtk_c::menu_item_content(title, NULL, markup, -48);
        g_free(markup);

        const gchar **p = items;
        gint i;
        for (i=0;p && *p; p++,i++){
            GtkWidget *v = gtk_menu_item_new_with_label (_(*p));
            g_object_set_data(G_OBJECT(v), "run_button_p", (void *)this);
            TRACE("set data: (%p) -> %p\n", (void *)v, (void *)this);
            gtk_container_add (GTK_CONTAINER (menu_), v);
            g_signal_connect ((gpointer) v, "activate", MENUITEM_CALLBACK (ps_signal), GINT_TO_POINTER(signals[i]));
            gtk_widget_show (v);
        }
        gtk_widget_show (GTK_WIDGET(menu_));
        */
        return ;
    }

    static void
    run_button_setup (void *data){
        auto run_button_p = (RunButton *)data;

        // Little icon assignment
        // Shell commands come from lpterminal (with specific shell characters).
        gchar *command = g_strdup(run_button_p->command());
        
        /*if (run_button_p->inShell()) {
            TRACE("run_button_p->inShell\n");
            run_button_p->set_icon_id("utilities-terminal");
        } else */
        {
            command = g_strstrip(command);
            gchar **args = g_strsplit(command, " ", -1);
            gchar *icon_id = NULL;
            if (args && args[0]) {
                icon_id = g_path_get_basename(args[0]);
                TRACE("RunButton::run_button_setup: attempting icon for \"%s\"\n", icon_id);
                // xterm exception
                if (strcmp(icon_id, "xterm")==0){
                    g_free(icon_id);
                    icon_id = g_strdup("utilities-terminal");
                } 
                /* FIXME (enable)
                else if (!Pixbuf<Type>::iconThemeHasIcon(icon_id)){
                    g_free(icon_id);
                    icon_id = NULL;
                }*/
            }
            g_strfreev(args);
            if (!icon_id) {
                //icon_id = g_strdup("run");
                icon_id = g_strdup("system-run");
            }
            run_button_p->set_icon_id(icon_id);
        }


        gchar *tip = g_strdup_printf(" %s=%d\n", _("PID"), run_button_p->grandchild()); 
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
        run_button_p->set_tip(tip);
        g_free(tip);
        g_free(command);
        TRACE("RunButton::new_run_button: icon_id_=%s  command_=%s pid_=%d grandchild_=%d tip_=%s\n", run_button_p->icon_id(), run_button_p->command(), run_button_p->pid(), run_button_p->grandchild(), run_button_p->tip());
        return ;
    }
////////////////////////////////////////////////////////////////////

    static void *
    make_run_data_button (void *data) {
        auto run_button_p = (RunButton *)data;
        //auto button = GTK_MENU_BUTTON(gtk_menu_button_new ());
        auto button = Util::newButton("avatar-default", "tooltip here" );
        run_button_p->setButton(button);
        TRACE("make_run_data_button... \n");
        // FIXME gtk_menu_button_set_popup (button,  GTK_WIDGET(run_button_p->menu()));
            
        // static:
        run_button_setup(data);
        
        TRACE("make_run_data_button: icon_id_=\"%s\" tip_=\"%s\"\n", run_button_p->icon_id(), run_button_p->tip());

        //gtk_c::setup_image_button(GTK_BUTTON(button), run_button_p->icon_id(), run_button_p->tip());
        gtk_widget_set_can_focus (GTK_WIDGET(button), FALSE);
        //gtk_button_set_relief (GTK_BUTTON(button), GTK_RELIEF_NONE);
        
        //g_signal_connect(button, "toggled", G_CALLBACK (run_button_toggled), data);
        Util::boxPack0 (Util::vButtonBox(), GTK_WIDGET(button), FALSE, FALSE, 0);
       // Util::boxPack0 (run_button_p->button_space(), GTK_WIDGET(button), FALSE, FALSE, 0);

        gtk_widget_set_visible(GTK_WIDGET(button), TRUE);
        // flush gtk
        Util::flushGTK();
        TRACE ("make_run_data_button: button made for grandchildPID=%d\n", (int)run_button_p->pid());
        return NULL;
    }


    static void *
    run_wait_f (void *data) {
        auto run_button_p = (RunButton *) data;
        /* create little button (note: thread protected within subroutine) */
        // The following function will not return until button is created and duly
        // processed by the gtk loop. This to avoid a race with the command_ completing
        // before gtk has fully created the little run button.
        //
        Util::context_function(make_run_data_button, data);
        TRACE("run_wait_f: thread waitpid for %d on (%s/%s)\n", 
                run_button_p->pid(), 
                run_button_p->command(), 
                run_button_p->page()->pageWorkdir());

        // referenced already in pagechild.hh:
        //page()->reference_run_button(run_button_p);
        
        gint status;
        waitpid (run_button_p->pid(), &status, 0);

        TRACE("run_wait_f: thread waitpid for %d complete!\n", run_button_p->pid());
        /* remove little button */
        
#ifdef DEBUG_TRACE    
        // This is out of sync here (grayball), so only in debug mode.
        print_c::print_icon(run_button_p->textview_, "emblem-grayball", g_strdup_printf("%s %d/%d\n", run_button_p->command(),
                run_button_p->pid(), run_button_p->grandchild()));
#endif
        
        /// FIXME: the following code is to signal the background monitor
        //         currently no monitor is enabled.
        // Run has completed.  
        // no use for controller process anymore....
        TRACE("run has completed: %d\n", run_button_p->pid());
        // FIXME: what does rfm_remove_child do??
        //        remove controller!
        // FIXME: enable
        // rfm_remove_child(run_data_p->pid);
        // Flush pipes.
        fflush(NULL);  
        // Destroy little button (if exists) and free run_data_p 
        // associated memory. Done in main thread for gtk instruction set.
        Util::context_function(zap_run_button, data);
        return NULL;
    }

    static void *
    zap_run_button(void * data){
        TRACE("zap_run_button...\n");
        auto run_button_p = (RunButton *)data;
        //run_button_p->page()->unreference_run_button(run_button_p);
        RunButton::unreference_run_button(run_button_p);
        return NULL;
    }

    GtkTextView *textview(void){return textview_;}

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

        TRACE("** looking for shell child of %ld\n", pid);
        glong cpid = -1;
        gchar *spid = g_strdup_printf("%ld", pid);
        gchar buffer[64];
        memset(buffer, 0, 64);

        while (fgets(buffer, 64, p) && !feof(p)){
            if (!strstr(buffer, spid)) continue;
            g_strstrip(buffer);
            if (strncmp(buffer, spid, strlen(spid))==0){
                TRACE("** shell_child_pid(): gotcha shell pid: \"%s\"\n", buffer);
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

        glong pid = grandchild();
        if (inShell()) pid = shell_child_pid(pid);

        gchar *command = g_strdup_printf("renice +1 -p %ld", pid);
        Run::shell_command(textview_, command, false, true);
        // Here we do not need to save "$command" to history...
        g_free(command);

        return;
    }
////////////////////////////////////////////////////////////////////

    static void send_signal(GtkWidget *w, void *data){
        auto run_button_p = (RunButton *)data;
        TRACE("send_signal %d to %d\n", 
                GPOINTER_TO_INT(g_object_get_data(G_OBJECT(w),"signal_id")), 
                run_button_p->grandchild());


    }

    static void
    show_run_info (GtkButton * button, gpointer data) {
        auto run_button_p = (RunButton *)data;
        //FIXME: we also need to be able to signal to child of shell, if launched from shell
        TRACE("FIXME: popup signal menu or dialog here.\n");

        
    }
    
public:
    void sendSignal(gint signal_id){
        glong pid = grandchild();
        if (!pid) return;
        // Do we need sudo?
        gboolean sudoize = FALSE;
        if (strncmp(command(), "sudo", strlen("sudo"))==0) sudoize = TRUE;
        // Are we running in a shell?
        if (inShell() || sudoize){
            TRACE("shell child pid required...\n");
            pid = shell_child_pid(pid);
        }
            
        TRACE("signal to pid: %ld (inShell()=%d sudo=%d) \"%s\"\n", pid, inShell(), sudoize, command());
            //        1.undetached child will remain as zombie
            //        2.sudo will remain in wait state and button will not disappear
            // hack: if signal is kill, kill sudo in the same command
            //       eliminate zombie...
        gchar *sudo = g_find_program_in_path("sudo");
        if (sudoize && !sudo){
            g_warning("sudo not found in path\n");
            return;
        }
        gchar *kill;
        if (sudoize) {
            kill = g_strdup_printf("%s -A kill", sudo);
        } else {
            kill = g_find_program_in_path("kill");
        }

        gchar *command;
        if (signal_id == SIGKILL) {
            command =  g_strdup_printf("%s -%d -%ld -%d", kill, signal_id, pid, grandchild());
        } else {
            command =  g_strdup_printf("%s -%d -%ld", kill, signal_id, pid);
        }
        WARN("signalling with %s\n", command);
        Run::shell_command(textview_, command, FALSE, TRUE);
        // Again, when we signal process, there is no need to save command
        // in the csh history file.
        g_free(command);
        g_free(sudo);
        g_free(kill);
    }



};
}

#endif


