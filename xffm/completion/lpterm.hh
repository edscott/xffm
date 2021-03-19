#ifndef LPTERM_HH
#define LPTERM_HH

// TODO, if readline/history.h is available in BSD, simplify with this
//#warning "add cmake test for HAVE_READLINE_HISTORY_H"
#ifdef HAVE_READLINE_HISTORY_H
# include <readline/history.h>
#endif

namespace xf {
template <class Type> class LpTerm;
template <class Type> class Page;

template <class Type>
class lptermSignals {
    static gboolean
    on_status_button_press ( GtkWidget *w , GdkEventButton * event, gpointer data) {
        auto lpterm_p = (LpTerm<Type> *)data;
        lpterm_p->lp_set_active(TRUE);
        return TRUE;
    }

    static gboolean
    status_keyboard_event (GtkWidget * window, GdkEventKey * event, gpointer data)
    {
        TRACE("status_keyboard_event\n");
        return FALSE;
    }

};


//////////////////////////////////////////////////////////////////
// some static, some instanciated (for mutex)



template <class Type>
class LpTerm {
    using util_c = Util<Type>;
    using print_c = Print<Type>;
    using run_c = Run<Type>;
private:
    GtkTextView *textview_;
    Page<Type> *page_;

    gboolean active_;
    GtkIconView *iconview_;
    GtkWidget *status_button_;
    GtkWidget *status_icon_;
    GtkWidget *iconview_icon_;

public:
    void setLPTermPage(Page<Type> *page){page_ = page;}
    void setTextview(GtkTextView *textview){textview_ = textview;}
    void setIconview(GtkIconView *iconview){iconview_ = iconview;}
    void setStatusButton(GtkWidget *status_button){status_button_ = status_button;}
    void setStatusIcon(GtkWidget *status_icon){status_icon_ = status_icon;}
    void setIconviewIcon(GtkWidget *iconview_icon){iconview_icon_ = iconview_icon;}

    LpTerm(void){
        active_ = FALSE;
        // FIXME: these callback functions and connection should be connected 
        // in pagechild
         /*
        g_signal_connect (status, "key-press-event", 
                KEY_EVENT_CALLBACK (status_keyboard_event), data);
        g_signal_connect (status_button, "button-press-event", 
                BUTTON_CALLBACK (on_status_button_press), (void *)this);
                */
    }

    gboolean
    lp_get_active(void){return active_;}

    void
    lp_set_active(gboolean state){
        active_ = state;
/*        if (state){
            gtk_widget_hide(GTK_WIDGET(status_button));
            gtk_widget_show(GTK_WIDGET(status));
            gtk_widget_show(status_icon);
            gtk_widget_hide(iconview_icon);
            gtk_widget_grab_focus (GTK_WIDGET(status));
        } else {
            gtk_widget_hide(GTK_WIDGET(status));
            gtk_widget_show(GTK_WIDGET(status_button));
            gtk_widget_show(iconview_icon);
            gtk_widget_hide(status_icon);
            gtk_widget_grab_focus (GTK_WIDGET(iconview));
        }*/
        return;
    }

    pid_t 
    run_lp_command(GtkTextView *output, const gchar *workdir, const gchar *command, gboolean withRunButton){
        pid_t child = 0;
        // On empty string, do a simple pwd
        if (!command || !strlen(command)) command = "pwd";
        // escape all quotes
        gchar *ncommand;
        if (strchr (command, '\"') || strchr(command,'\'')){
            gchar **g;
            if (strchr (command, '\"')) {
                g = g_strsplit(command, "\"", -1);
                ncommand = g_strjoinv ("\\\"", g);
                g_strfreev(g);
            } else {
                g = g_strsplit(command, "\'", -1);
                ncommand = g_strjoinv ("\\\'", g);
                g_strfreev(g);
            }
            TRACE("ncommand is %s\n", ncommand);
        } else ncommand = g_strdup(command);
        command = ncommand;

        gchar *newWorkdir =NULL;
        gchar ** commands = NULL;
        if (strchr(command, ';')) commands = g_strsplit(command, ";", -1);
        if (!commands) {
            commands = (gchar **) calloc(2, sizeof(gchar *));
            commands[0] = g_strdup(command); 
        }
        gchar **c;
        for (c=commands; c && *c; c++){
            if (process_internal_command (output, workdir, *c)) {
                TRACE("internal command=%s\n", command);
                continue;
            }
            // automatic shell determination:
            if (strcmp(workdir, "xffm:root")==0) {
                if (chdir(g_get_home_dir()) < 0){
                    ERROR("Cannot chdir to %s\n", g_get_home_dir());
                    ERROR("aborting command: \"%s\"\n", command);
                    continue;
                }
            } else {
                if (chdir(workdir) < 0){
                    ERROR("Cannot chdir to %s\n", workdir);
                    ERROR("aborting command: \"%s\"\n", command);
                    continue;
                }
            }
            gboolean scrollup = FALSE;
            if (strncmp(command, "man", strlen("man"))==0) {
                scrollup = TRUE;
                print_c::clear_text(output);
            }

            child = run_c::shell_command(output, *c, scrollup);
            if (withRunButton) page_->newRunButton(*c, child);
            // forced shell to command:
            //run_c::shell_command(output, *c, FALSE);
            /*
            run_button_c *run_button_p = NULL;
            // XXX runbutton constructor will need textview and runbutton box
            run_button_p = new run_button_c(view_v, c, pid, run_in_shell(c));
            // run_button_p will run alone and call its own destructor.
*/
            // Here we save to csh history.
            // We save the original sudo command,
            //   not the one modified with "-A".
        }
        g_strfreev(commands);
        g_free(ncommand); 
        return child;
    }

    void
    open_terminal(GtkTextView *output){
        const gchar *terminal = util_c::what_term();
        run_c::shell_command(output, terminal, FALSE);
/*        run_button_c *run_button_p = NULL;
        // XXX runbutton constructor will need textview and runbutton box
        run_button_p = new run_button_c(view_v, c, pid, run_in_shell(c));*/
    // This is not configured to save to csh history.
    }

    // FIXME: we should allow expansion of environment variables like $HOME
    //        and whatever else is in the environment.
    gchar *
    internal_cd (GtkTextView *output, const gchar *workdir, gchar ** argvp) {   
        gchar *gg=NULL;

        if (argvp[1] == NULL){
            print_c::showTextSmall(output);
            //Output is taken care of in page.hh:setPageWorkdir().
            //print_c::print(output, "green", g_strdup_printf("cd %s\n", g_get_home_dir()));
            return g_strdup(g_get_home_dir());
        }

        if(argvp[1]) {
            if (*argvp[1] == '~'){
                if (strcmp(argvp[1], "~")==0 || 
                        strncmp(argvp[1], "~/", strlen("~/"))==0){
                    gg = g_strdup_printf("%s%s", g_get_home_dir (), argvp[1]+1);
                } else {
                    gchar *tilde_dir = util_c::get_tilde_dir(argvp[1]);
                    if (tilde_dir) gg = g_strconcat(tilde_dir, strchr(argvp[1], '/')+1, NULL);
                    else gg = g_strdup(argvp[1]);
                    g_free(tilde_dir);        
                }
            } else {
                gg = g_strdup(argvp[1]);
            }

        } 
        print_c::showTextSmall(output);

        // must allow relative paths too.
        if (!g_path_is_absolute(gg)){
            if(!g_file_test (workdir, G_FILE_TEST_IS_DIR)) 
            {
                print_c::print_error(output, g_strdup_printf("%s: %s\n", gg, strerror (ENOENT)));
                g_free (gg);
                return NULL;
            } 
            gchar *fullpath;
            if (strcmp(workdir, G_DIR_SEPARATOR_S)==0)
                fullpath = g_strconcat(G_DIR_SEPARATOR_S, gg, NULL);
            else
                fullpath = g_strconcat(workdir, G_DIR_SEPARATOR_S, gg, NULL);
            g_free(gg);
            gg = fullpath;
        }

        gchar *rpath = realpath(gg, NULL);
        if (!rpath){
            print_c::print_error(output, g_strdup_printf("%s: %s\n", gg, strerror (ENOENT)));
            g_free (gg);
            return NULL;
        }

        if (gg[strlen(gg)-1]==G_DIR_SEPARATOR || strstr(gg, "/..")){
            g_free(gg);
            gg=rpath;
        } else {
            g_free (rpath);
        }

        //taken care of in taskbar object
        //print_c::print(output, "green", g_strdup_printf("cd %s\n", gg));
        if (chdir(gg) < 0) {
            print_c::print_error(output, g_strdup_printf("%s\n", strerror(errno)));
            return NULL;
        }
        //print_c::clear_text();

        // FIXME: here we must signal a reload to the iconview...
        //view_p->reload(gg);

        return gg;
    }

    // FIXME: we need to add history as an internal command with csh history.
    gboolean
    process_internal_command (GtkTextView *output, const gchar *workdir, const gchar *command) {
        gint argcp;
        gchar **argvp;
        GError *error = NULL;
        if(!g_shell_parse_argv (command, &argcp, &argvp, &error)) {
            print_c::print_error(output, g_strdup_printf("%s\n", error->message));
            return FALSE;
        } else if(strcmp (argvp[0], "cd")==0) {
            // shortcircuit chdir
            gchar *gg = internal_cd (output, workdir, argvp);
            g_strfreev (argvp);
            if (gg) {
                TRACE("newWorkdir-gg = %s\n", gg);
                TRACE("page_ = %p\n", (void *)page_);
                if (page_) {
                    page_->setPageWorkdir(gg);
                auto view = (View<Type> *)
                    g_object_get_data(G_OBJECT(page_->topScrolledWindow()), "baseView");
                view->loadModel(gg, page_->view());
                    g_free(gg);
                }
                return TRUE;
            }

            return TRUE;
        }
        g_strfreev (argvp);
        return  FALSE;
    }
 
};

#if 0
// work in progress: run command and open with dialog
// see src/lpterm.cpp
#endif


///////////////////////   static GTK callbacks... ////////////////////////////////////////


} // namespace
#endif 

