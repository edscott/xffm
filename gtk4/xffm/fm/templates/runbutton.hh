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

pthread_mutex_t rbl_mutex = PTHREAD_MUTEX_INITIALIZER; // run button list mutex


template <class Type> class RunButton {
private:
    GtkTextView *textview_ = NULL;
    GtkBox *buttonSpace_ = NULL;

    pid_t pid_ = 0;
    pid_t grandchild_ = 0;
    gchar *command_ = NULL;
    gchar *tip_ = NULL;
    gchar *icon_id_ = NULL;
    GtkMenuButton *button_ = NULL;
    gboolean in_shell_ = FALSE;
    gchar *workdir_ = NULL;
     
public:    
    GtkMenuButton *button(void){ return button_;}
    gboolean inShell(void){return in_shell_;}
    gint pid(void){ return (gint)pid_;}
    gint grandchild(void){ return (gint)grandchild_;}
    GtkBox *buttonSpace(void){return buttonSpace_;}
    gchar *workdir(void){return workdir_;}

    RunButton(void){
      TRACE("RunButton void\n");
    }
    ~RunButton(void){
        TRACE("RunButton::~RunButton... button_ %p\n", (void *)button_);
        auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button_), "menu"));
        gtk_popover_popdown(menu);

        gtk_widget_unrealize(GTK_WIDGET(menu));
        gtk_widget_unparent(GTK_WIDGET(menu));

        gtk_widget_set_visible(GTK_WIDGET (button_), FALSE);
        gtk_widget_unrealize(GTK_WIDGET (button_));
        gtk_widget_unparent(GTK_WIDGET (button_));
        g_free (tip_);
        g_free (command_);
        g_free (icon_id_);
        g_free (workdir_);
    }
    
    void init(RunButton *runButton, const gchar *command, pid_t child, GtkTextView *output, const gchar *workdir, GtkBox *buttonBox){
        TRACE("RunButton for %s\n", command);
        gboolean shellIcon = Run<bool>::run_in_shell(command);

        in_shell_ = shellIcon;
        pid_ = child;
        grandchild_ = Tubo::getChild(child);
        command_ = g_strdup (command);
        icon_id_ = NULL;
        tip_ = g_strdup (command);
        workdir_ = g_strdup(workdir);
        textview_ = output; 
        buttonSpace_ = buttonBox;
        TRACE ("RunButton::setup_run_button_thread: controller/process=%d/%d\n", (int)child, (gint)grandchild_);
        TRACE ("RunButton::buttonSpace = %p\n", buttonSpace_);
//        
        auto text = g_strdup_printf("RunButton::setup(): %s", command_);
        new(Thread)(text, run_wait_f, (void *) this);
        g_free(text);
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
    ps_signal(GtkWidget *button, gpointer data){
        auto run_button_p = (RunButton *)
            g_object_get_data(G_OBJECT(button), "run_button_p");

        gint signal_id =GPOINTER_TO_INT(data);

        TRACE("apply data: (%p) -> %p\n", (void *)button, (void *)run_button_p);
        auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu"));
        gtk_popover_popdown(menu);

        if (signal_id ==-1) {
            run_button_p->ps_renice();
            return;
        }
        run_button_p->sendSignal(signal_id);
        return;
    }



    // read only

    // read/write
    void setButton(GtkMenuButton *button){
      button_ = button;
    }
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
        Basic::setTooltip(GTK_WIDGET(button_), tip_);
    }

// FIXME: use Menu class

    static GtkPopover *
    mkPsMenu(GtkLabel *title, RunButton *run_button_p, const gchar **items, void **callback, void **data){
      auto vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
        gtk_widget_add_css_class (GTK_WIDGET(vbox), "inquireBox" );
        gtk_widget_set_hexpand(GTK_WIDGET(vbox), FALSE);
        gtk_widget_set_vexpand(GTK_WIDGET(vbox), FALSE);
     auto titleBox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_add_css_class (GTK_WIDGET(titleBox), "inquireBox" );
      gtk_box_append (GTK_BOX (vbox), GTK_WIDGET(titleBox));
      gtk_box_append (GTK_BOX (titleBox), GTK_WIDGET(title));

      GtkWidget *menu = gtk_popover_new ();
        gtk_widget_set_vexpand(GTK_WIDGET(vbox), FALSE);       
        gtk_widget_add_css_class (GTK_WIDGET(menu), "inquireBox" );
      gtk_popover_set_autohide(GTK_POPOVER(menu), TRUE);
      gtk_popover_set_has_arrow(GTK_POPOVER(menu), FALSE);
      gtk_widget_add_css_class (GTK_WIDGET(menu), "inquire" );

      void **q = callback;
      void **r = data;
      for (const gchar **p=items; p && *p && *q; p++){
        GtkWidget *item = gtk_button_new_with_label(*p);
        gtk_widget_add_css_class (GTK_WIDGET(item), "inquireBox" );
        gchar *t = g_strdup_printf("%s: %d", _("Signal to emit"), GPOINTER_TO_INT(*r));
        Basic::setTooltip(item, t);
        g_free(t);
        g_object_set_data(G_OBJECT(item), "run_button_p", run_button_p);
        gtk_button_set_has_frame(GTK_BUTTON(item), FALSE);
        gtk_box_append (GTK_BOX (vbox), item);
        if (*q) {
          g_signal_connect (G_OBJECT (item), "clicked", G_CALLBACK(*q), *r);
          q++;
        }
        g_object_set_data(G_OBJECT(item), "menu", menu);
        if (*r) r++;
      }
          
      gtk_popover_set_child (GTK_POPOVER (menu), vbox);
      return GTK_POPOVER(menu);
    }

    static GtkWidget *
    create_menu(RunButton *run_button_p){      
      const gchar *items[]={
          N_("Renice Process"), //1
          N_("Suspend"),        //2 
          N_("Continue"),       //3
          N_("Interrupt"),      //4
          N_("Hangup"),         //5 
          N_("User 1 (USR1)"),  //6
          N_("User 2 (USR2)"),  //7
          N_("Terminate Task"), //8
          N_("Abort"),          //9
          N_("Kill"),           //10
          N_("Segmentation fault"),//11
          NULL}; 
        
      void *signals[] = {
            GINT_TO_POINTER(-1),      //1
            GINT_TO_POINTER(SIGSTOP), //2
            GINT_TO_POINTER(SIGCONT), //3
            GINT_TO_POINTER(SIGINT),  //4
            GINT_TO_POINTER(SIGHUP),  //5
            GINT_TO_POINTER(SIGUSR1), //6
            GINT_TO_POINTER(SIGUSR2), //7
            GINT_TO_POINTER(SIGTERM), //8
            GINT_TO_POINTER(SIGABRT), //9
            GINT_TO_POINTER(SIGKILL), //10
            GINT_TO_POINTER(SIGSEGV), //11
            NULL};


      void *callbacks[]={
        (void *)ps_signal, //1
        (void *)ps_signal, //2
        (void *)ps_signal, //3
        (void *)ps_signal, //4
        (void *)ps_signal, //5
        (void *)ps_signal, //6
        (void *)ps_signal, //7
        (void *)ps_signal, //8
        (void *)ps_signal, //9
        (void *)ps_signal, //10
        (void *)ps_signal, //11
        NULL};
        GtkLabel *title = GTK_LABEL(gtk_label_new(""));

        gchar *commandS = g_strdup(run_button_p->command());
        for (auto p = commandS; p && *p; p++){
            if (*p == '&') *p='^';
            if (*p == '>') *p='!';
            if (*p == '<') *p='!';
        }

        gchar *process = g_strdup_printf("Process: %s\n", commandS);
        gchar *pidS = g_strdup_printf("<span color=\"red\" size=\"larger\">pid: %d</span>", run_button_p->grandchild());
        gchar *markup = g_strdup_printf("<span color=\"blue\" size=\"larger\">%s</span>%s", process, pidS);
        Basic::concat(&markup, "\n<span color=\"green\" size=\"larger\">Send signal:</span>");
        g_free(commandS);
        g_free(process);
        g_free(pidS);

        gtk_label_set_markup(title, markup);
        g_free(markup);
             
        return GTK_WIDGET(mkPsMenu(title, run_button_p, items, callbacks, signals)); 
    }

    static void
    run_button_setup (void *data){
        auto run_button_p = (RunButton *)data;

        // Little icon assignment
        // Shell commands come from lpterminal (with specific shell characters).
        gchar *command = g_strdup(run_button_p->command());
        
        if (run_button_p->inShell()) {
            TRACE("run_button_p->inShell\n");
            run_button_p->set_icon_id("utilities-terminal");
        } else 
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
                    icon_id = g_strdup("video-display");
                } 
                if (gtk_icon_theme_has_icon (iconTheme, icon_id)){
                  auto button = run_button_p->button();
                  gtk_menu_button_set_icon_name(button, icon_id);
                }

            }
            g_strfreev(args);
           /* if (!icon_id) {
                icon_id = g_strdup("network-workgroup");
            }*/
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
        auto button = GTK_MENU_BUTTON(gtk_menu_button_new());

        gtk_menu_button_set_icon_name(button, EMBLEM_RUN);
        run_button_p->setButton(button);

        auto menu = create_menu(run_button_p);
        gtk_menu_button_set_popover (GTK_MENU_BUTTON (button), GTK_WIDGET(menu));  
        g_object_set_data(G_OBJECT(button), "menu", menu);

        TRACE("make_run_data_button... \n");
            
        // static:
        run_button_setup(data); // Here we modify the icon on the button...
        
        TRACE("make_run_data_button: icon_id_=\"%s\" tip_=\"%s\"\n", run_button_p->icon_id(), run_button_p->tip());

        //gtk_c::setup_image_button(GTK_BUTTON(button), run_button_p->icon_id(), run_button_p->tip());
        gtk_widget_set_can_focus (GTK_WIDGET(button), FALSE);
        //gtk_button_set_relief (GTK_BUTTON(button), GTK_RELIEF_NONE);
        
        //g_signal_connect(button, "toggled", G_CALLBACK (run_button_toggled), data);
        //Util::boxPack0 (Util::vButtonBox(), GTK_WIDGET(button), FALSE, FALSE, 0);
        Basic::boxPack0 (run_button_p->buttonSpace(), GTK_WIDGET(button), FALSE, FALSE, 0);

        gtk_widget_set_visible(GTK_WIDGET(button), TRUE);
        // flush gtk
        //Basic::flushGTK();
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
        Basic::context_function(make_run_data_button, data);
        TRACE("run_wait_f: thread waitpid for %d on (%s/%s)\n", 
                run_button_p->pid(), 
                run_button_p->command(), 
                run_button_p->workdir());

        // referenced already in pagechild.hh:
        //page()->reference_run_button(run_button_p);
        
        gint status;
        waitpid (run_button_p->pid(), &status, 0);

        TRACE("run_wait_f: thread waitpid for %d complete!\n", run_button_p->pid());
        /* remove little button */
        
#ifdef DEBUG_TRACE    
        // This is out of sync here (grayball), so only in debug mode.
        Print::print_icon(run_button_p->textview_, EMBLEM_GRAY_BALL, g_strdup_printf("%s %d/%d\n", run_button_p->command(),
                run_button_p->pid(), run_button_p->grandchild()));
#endif
        
        // Flush pipes.
        fflush(NULL);  
        // Destroy little button (if exists) and free run_data_p 
        // associated memory. Done in main thread for gtk instruction set.
        Basic::context_function(zap_run_button, data);
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
        Run<bool>::shell_command(textview_, command, false, true); // yes output to textview
        //Run::shell_command(textview_, command, false, false); // no output to textview
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
        TRACE("popup signal menu or dialog here.\n");
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
            command =  g_strdup_printf("%s -%d %ld %d", kill, signal_id, pid, grandchild());
        } else {
            command =  g_strdup_printf("%s -%d %ld", kill, signal_id, pid);
        }
        TRACE("signalling with %s\n", command);
        //Run::shell_command(textview_, command, FALSE, TRUE); // yes output to textview
        Run<bool>::shell_command(textview_, command, FALSE, FALSE); // no output to textview
        // Again, when we signal process, there is no need to save command
        // in the csh history file.
        g_free(command);
        g_free(sudo);
        g_free(kill);
    }



};
}

#endif


