#ifndef SIGNALS__HH
# define SIGNALS__HH

#include "types.h"
#include "xfprint.hh"
#include "xfrun.hh"
#include "xfutil.hh"
static const gchar *ftypes[] = {
    N_("Any"),
    N_("Regular"),
    N_("Directory"),
    N_("Symbolic Link"),
    N_("Socket"),
    N_("Block device"),
    N_("Character device"),
    N_("FIFO"),
    NULL
};

typedef struct fgrData_t{
    GtkWindow *dialog;
    gint resultLimit;
    gint resultLimitCounter;
    GSList *findList;
    gchar **argument;
}fgrData_t;

static GSList *controllers = NULL;
static GSList *lastFind = NULL;

namespace xf
{


template <class Type>
class Signals: public Run<Type>{
    using gtk_c = Gtk<double>;
    using print_c = Print<double>;
    using run_c = Run<double>;
    using util_c = Util<double>;
public:
    static void 
    sensitivize (GtkToggleButton *togglebutton, gpointer data){
	GtkWidget *widget = GTK_WIDGET(data);
	gtk_widget_set_sensitive(widget, gtk_toggle_button_get_active(togglebutton));
    }

    static void 
    sensitivize_radio (GtkToggleButton *togglebutton, gpointer data){
	if (!data) return;
	radio_t *radio_p = (radio_t *)data;
	gtk_widget_set_sensitive(GTK_WIDGET(radio_p->box), FALSE);
	GtkToggleButton **tb_p = radio_p->toggle;
	for (; tb_p && *tb_p; tb_p++){
	    if (gtk_toggle_button_get_active(*tb_p)){
		gtk_widget_set_sensitive(GTK_WIDGET(radio_p->box), TRUE);
	    }
	}
    }

    static void
    command_up (GtkWidget * button, gpointer data) {
        quick_command(button, data, TRUE);
    }

    static void
    command_down (GtkWidget * button, gpointer data) {
        quick_command(button, data, FALSE);
    }

    static void
    on_buttonHelp (GtkWidget * button, gpointer data) {
	GtkWindow *dialog_=GTK_WINDOW(g_object_get_data(G_OBJECT(button), "dialog_"));
	const gchar *message = (const gchar *)data;
	std::cerr<<"fixme: signals::on_buttonHelp\n";
	gtk_c::quick_help(dialog_, message);
    }


    static void
    onClearButton (GtkWidget * button, gpointer data) {
	GtkTextView *diagnostics = GTK_TEXT_VIEW(data);
	std::cerr<<"fixme: signals::onClearButton\n";
        print_c::clear_text(diagnostics);
        print_c::hide_text(diagnostics);

    }

    static void
    onEditButton (GtkWidget * button, gpointer data) {
	GtkTextView *diagnostics = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(data), "diagnostics"));
	std::cerr<<"fixme: signals::onEditButton\n";
        print_c::print_status(diagnostics, g_strdup("fixme: signals::onEditButton testing run\n"));
        print_c::show_text(diagnostics);        
        run_c::thread_run(diagnostics, "ls -l", FALSE);
    }

    static void
    onCloseButton (GtkWidget * button, gpointer data) {
	std::cerr<<"fixme: signals::onCloseButton\n";
        GtkWidget *dialog = GTK_WIDGET(data);
        gtk_widget_hide(dialog);
        while (gtk_events_pending()) gtk_main_iteration();
        exit(1);
    }

    static void
    onCancelButton (GtkWidget * button, gpointer data) {
	std::cerr<<"fixme: signals::onCancelButton\n";
    }

    static void
    onFindButton (GtkWidget * button, gpointer dialog) {
	GtkTextView *diagnostics = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(dialog), "diagnostics"));
        updateCompletions(dialog);
        
        print_c::show_text(diagnostics);
        on_find_clicked_action (GTK_WINDOW(dialog));
/*	std::cerr<<"fixme: signals::onFindButton\n";
        print_c::print(diagnostics, g_strdup("1.fixme: signals::\n"));
        print_c::print(diagnostics, "tag/green", g_strdup("2.fixme: signals::\n"));
        print_c::print_debug(diagnostics, g_strdup("3.fixme: signals::\n"));
        print_c::print_error(diagnostics, g_strdup("4.fixme: signals::\n"));
        print_c::print_icon(diagnostics, "edit-find",  g_strdup("5.fixme: signals::\n"));
        print_c::print_icon_tag(diagnostics, "edit-find",  "tag/red",g_strdup("fixme: signals::\n"));*/
    }

    static gint
    on_key_release (GtkWidget * widget, GdkEventKey * event, gpointer data) {
        const gchar *text = gtk_entry_get_text(GTK_ENTRY(widget));
        gboolean active = FALSE;
        if (text && strlen(text)) active = TRUE;
        //std::cerr<<"on_key_release: "<< text << " active: " << active << "\n";
        if (data){
            gtk_widget_set_sensitive(GTK_WIDGET(data), active);
        }
        return FALSE;
    }
    static gint
    on_completion (GtkWidget * widget, GdkEventKey * event, gpointer data) {
        GtkEntryCompletion *completion = gtk_entry_get_completion(GTK_ENTRY(widget));
        gtk_entry_completion_complete (completion);
        return FALSE;
    }
    static void
    folderChooser (GtkButton * button, gpointer data) {
        GtkEntry *entry = GTK_ENTRY(data);
        const gchar *text = _("Select folder to search in");
        //const gchar *text = _("Select Files...");
         GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
        // GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
        GtkDialog *dialog = GTK_DIALOG(gtk_file_chooser_dialog_new (text,
                                                         GTK_WINDOW (gtk_widget_get_toplevel(GTK_WIDGET(entry))),
                                                         action,
                                                         _("Cancel"),
                                                         GTK_RESPONSE_CANCEL,
                                                         _("Open"),
                                                         GTK_RESPONSE_ACCEPT,
                                                         NULL));
        gtk_file_chooser_set_action ((GtkFileChooser *) dialog, action);
        gchar *current_folder = g_get_current_dir();
        gtk_file_chooser_set_current_folder ((GtkFileChooser *) dialog, current_folder);

        gint response = gtk_dialog_run(dialog);

        if(response == GTK_RESPONSE_ACCEPT) {
            gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
            gtk_entry_set_text (entry, filename);
            NOOP ("Got %s\n", filename);
            g_free (filename);
        }
        gtk_widget_hide (GTK_WIDGET(dialog));
        gtk_widget_destroy (GTK_WIDGET(dialog));

    }


private:
    
    static void
    quick_command (GtkWidget * button, gpointer data, gboolean scrollUp) {
	GtkWindow *dialog_=GTK_WINDOW(g_object_get_data(G_OBJECT(button), "dialog_"));
	GtkTextView *diagnostics = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(dialog_), "diagnostics"));
	std::cerr<<"fixme: signals::quick_command\n";
        print_c::clear_text(diagnostics);
        //run_c::thread_run(diagnostics, (const gchar *)data);
        run_c::thread_run(diagnostics, (const gchar *)data, scrollUp);
    }

    static void 
    updateCompletions(gpointer dialog){ 
        gchar *history;
        GtkEntry *entry;
        GtkTreeModel *model;

        history = g_build_filename (FILTER_HISTORY);
        entry = GTK_ENTRY(g_object_get_data(G_OBJECT(dialog), "filter_entry"));
        model = GTK_TREE_MODEL(g_object_get_data(G_OBJECT(entry), "model"));
        util_c::saveHistory(history, model, gtk_entry_get_text(entry));
        g_free(history);

        history = g_build_filename (GREP_HISTORY);
        entry = GTK_ENTRY(g_object_get_data(G_OBJECT(dialog), "grep_entry"));
        model = GTK_TREE_MODEL(g_object_get_data(G_OBJECT(entry), "model"));
        util_c::saveHistory(history, model, gtk_entry_get_text(entry));
        g_free(history);
    }

    static const gchar *
    get_time_type(GtkWindow *dialog){
        if (gtk_toggle_button_get_active ((GtkToggleButton *) g_object_get_data(G_OBJECT(dialog), "radio1"))){
            return "-M";
        }
        if (gtk_toggle_button_get_active ((GtkToggleButton *) g_object_get_data(G_OBJECT(dialog), "radio2"))){
            return "-C";
        }
        return "-A";
    }

    static void
    stdout_f (void *data, void *stream, int childFD) {
        fgrData_t *Data = (fgrData_t *)data;
        char *line;
        line = (char *)stream;
        NOOP ("FORK stdout: %s\n", line);

        if(line[0] == '\n') return;
        GtkTextView *diagnostics = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(Data->dialog), "diagnostics"));

        if (Data->resultLimit > 0 && Data->resultLimit==Data->resultLimitCounter) {
            gchar *g=g_strdup_printf("%s. %s %d", _("Results"), _("Upper limit:"), Data->resultLimit);
            print_c::print_icon_tag(diagnostics, "dialog-warning", "tag/green", g_strconcat(g, "\n", NULL));
            print_c::print_icon_tag(diagnostics, "dialog-info", "tag/blue",  g_strconcat(_("Counting files..."), "\n", NULL));
            g_free(g);
        }

        if(strncmp (line, "Tubo-id exit:", strlen ("Tubo-id exit:")) == 0) {
                if(strchr (line, '\n')) *strchr (line, '\n') = 0;
                
                /*FIXME this wont work if another fgr is running
                 * must check for any running fgr subprocess
                 * GtkWidget *cancel = widgets_p->data; 
                if (cancel && GTK_IS_WIDGET(cancel)) {
                    rfm_context_function(hide_cancel_button, cancel);
                }*/
                gchar *plural_text = 
                    g_strdup_printf (
                            ngettext ("Found %d match", "Found %d matches", 
                                Data->resultLimitCounter), Data->resultLimitCounter);
                gchar *fgr = g_find_program_in_path("fgr");
                gchar *m = g_strdup_printf(_("%s Finished : %s"), fgr, plural_text);
                g_free(fgr);
                g_free(plural_text);
                gchar *g = g_strdup_printf("%c[31m%s\n",27, m);
                print_c::print_icon (diagnostics, "emblem_redball", g);
                g_free(m);
                // Free last find results
                GSList *list = lastFind;
                for(;list; list=list->next) g_free(list->data);
                g_slist_free(lastFind);
                // assign new find results
                lastFind = Data->findList;
                // Free fgrData_t *Data
                freeFgrData(Data);
                // FIXME: remove controller pid from controller list
        } else {
            Data->resultLimitCounter++; 
            if (Data->resultLimit ==0 ||
                (Data->resultLimit > 0 && Data->resultLimit > Data->resultLimitCounter) ) {
                print_c::print(diagnostics, g_strdup(line));
                //fprintf(stderr, "resultLimitCounter:%d %s\n", Data->resultLimitCounter, line);
                gchar *file = g_strdup(line);
                if (strchr(file, '\n')) *strchr(file, '\n') = 0;
                Data->findList = g_slist_prepend(Data->findList, file);
            } 
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
    cancel_all (fgrData_t *Data) {
        // Send a KILL to each fgr that is still running. This is done
        // by sending a SIGUSR2 to the controller (SIGUSR1 would pass on a TERM).
        GSList *tmp = controllers;
        for (; tmp && tmp->data; tmp=tmp->next){
            kill(GPOINTER_TO_INT(tmp->data), SIGUSR2);
        }
        g_slist_free(controllers);
        controllers=NULL;
    }
    
    static void
    freeFgrData(fgrData_t *Data){
        gchar **a=Data->argument;
        for (;*a; a++) g_free(*a);
        g_free(Data->argument);
        // We keep Data around to avoid any potential race
        // between stderr_f and stdout_f. Just a small calculated leak.
        //g_free(Data);
    }

    static gboolean
    on_find_clicked_action (GtkWindow *dialog) {
        GtkTextView *diagnostics = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(dialog), "diagnostics"));
        // Get the search path.
        GtkEntry *entry = GTK_ENTRY(g_object_get_data(G_OBJECT(dialog), "path_entry"));
        gchar *path = g_strdup(gtk_entry_get_text(entry));
        /* tilde expansion */
        if(path[0] == '~'){
            gchar *t = g_strconcat(g_get_home_dir(),"/", path+1);
            g_free(path);
            path = t;
        }
        /* environment variables */
        else if(path[0] == '$') {
            const gchar *p = getenv (path + 1);
            if(p){
                g_free(path);
                path = g_strdup(p);
            }
        }
        if(!path || strlen (path) == 0 || !g_file_test (path, G_FILE_TEST_EXISTS)) {
            gchar *message = g_strconcat(strerror (ENOENT), ": ", path, "\n", NULL);
            print_c::print_error(diagnostics, message);
            g_free(path);
            return FALSE;
        }
        fgrData_t *Data = (fgrData_t *)calloc(1,sizeof(fgrData_t));
        Data->dialog = dialog;


        GtkWidget *cancel = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "cancel_button"));
        gtk_widget_show(cancel);
        gtk_widget_set_sensitive(cancel, TRUE);

        /* get the parameters set by the user... *****/
        get_arguments(path, Data);


        gchar *message = g_strconcat( _("Searching..."), "\n", NULL);
        print_c::print_icon_tag(diagnostics, "emblem_find", "tag/green", message);

        /*
    // not here: FIXME    
        if (Data->find_list) {
            GSList *list = find_list;
            for (;list && list->data; list=list->next){
                g_free(list->data);
            }
            g_slist_free(find_list);
            find_list = NULL;
        }
    */

        int flags = TUBO_EXIT_TEXT|TUBO_VALID_ANSI|TUBO_CONTROLLER_PID;
        pid_t controller = run_c::thread_run(Data, (const gchar **)Data->argument, stdout_f);

        // When the dialog is destroyed on action, then output is going to a
        // widgets_p away from dialog window, so we let the process run on and on.
        // FIXME: enable cancel button with controllers.
	controllers = 
	    g_slist_prepend(controllers, GINT_TO_POINTER(controller));

    }

    static void
    get_arguments(gchar *path, fgrData_t *Data){
        gint i=0;
        GtkWindow *dialog = Data->dialog;

        /* limit */
        Data->resultLimit = gtk_spin_button_get_value_as_int (
                GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(dialog), "upper_limit_spin")));
        Data->resultLimitCounter = -1; // fgr ends with "fgr search complete"
        Data->argument = (gchar **)calloc(MAX_COMMAND_ARGS, sizeof(gchar *));
        if (!Data->argument){
            std::cerr<<"calloc error at get_arguments()\n";
            exit(1);
        }
        
        /* the rest */
        Data->argument[i++] = g_strdup("fgr");
        Data->argument[i++] = g_strdup("-v"); // (verbose output) 
        if(gtk_toggle_button_get_active ((GtkToggleButton *) 
                    g_object_get_data(G_OBJECT(dialog), "recursive")))
        {
            if(gtk_toggle_button_get_active ((GtkToggleButton *)
                        g_object_get_data(G_OBJECT(dialog), "recursiveH"))) 
            {
                Data->argument[i++] = g_strdup("-r");
            } else {
                Data->argument[i++] = g_strdup("-R");
            }
        } 

        if(gtk_toggle_button_get_active ((GtkToggleButton *) 
                    g_object_get_data(G_OBJECT(dialog), "size_greater"))) 
        {
             gint size_greater = gtk_spin_button_get_value_as_int (
                GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(dialog), "size_greater_spin")));	    
             Data->argument[i++] = g_strdup("-s");
             Data->argument[i++] = g_strdup_printf("+%d", size_greater);
        }
        if(gtk_toggle_button_get_active ((GtkToggleButton *) 
                    g_object_get_data(G_OBJECT(dialog), "size_smaller"))) 
        {
             gint size_smaller = gtk_spin_button_get_value_as_int (
                GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(dialog), "size_smaller_spin")));	    
             Data->argument[i++] = g_strdup("-s");
             Data->argument[i++] = g_strdup_printf("-%d", size_smaller);
        }
        if(gtk_toggle_button_get_active ((GtkToggleButton *) 
                    g_object_get_data(G_OBJECT(dialog), "last_months"))) 
        {
            Data->argument[i++] = g_strdup((gchar *)get_time_type(dialog));
            gint last_months = gtk_spin_button_get_value_as_int (
                GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(dialog), "last_months_spin")));	    
            Data->argument[i++] = g_strdup("-m");
            Data->argument[i++] = g_strdup_printf("%d", last_months);
        }
        else if(gtk_toggle_button_get_active ((GtkToggleButton *)
                    g_object_get_data(G_OBJECT(dialog), "last_days"))) 
        {
            Data->argument[i++] = (gchar *)get_time_type(dialog);
            gint last_days = gtk_spin_button_get_value_as_int (
                GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(dialog), "last_days_spin")));	    
            Data->argument[i++] = g_strdup("-d");
            Data->argument[i++] = g_strdup_printf("%d", last_days);
        }
        else if(gtk_toggle_button_get_active ((GtkToggleButton *) 
                    g_object_get_data(G_OBJECT(dialog), "last_hours"))) 
        {
            Data->argument[i++] = (gchar *)get_time_type(dialog);
            gint last_hours = gtk_spin_button_get_value_as_int (
                GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(dialog), "last_hours_spin")));	    
            Data->argument[i++] = g_strdup("-h");
            Data->argument[i++] = g_strdup_printf("%d", last_hours);
        }
        else if(gtk_toggle_button_get_active ((GtkToggleButton *) 
                    g_object_get_data(G_OBJECT(dialog), "last_minutes"))) 
        {
            Data->argument[i++] = g_strdup((gchar *)get_time_type(dialog));
            gint last_minutes = gtk_spin_button_get_value_as_int (
                GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(dialog), "last_minutes_spin")));	    
            Data->argument[i++] = g_strdup("-k");
            Data->argument[i++] = g_strdup_printf("%d", last_minutes);
        }
        if(gtk_toggle_button_get_active ((GtkToggleButton *) 
                    g_object_get_data(G_OBJECT(dialog), "suidexe")))
        {
            Data->argument[i++] = g_strdup("-p");
            if(gtk_toggle_button_get_active ((GtkToggleButton *) 
                        g_object_get_data(G_OBJECT(dialog), "suid_radio")))
            {
                Data->argument[i++] = g_strdup("suid");
            } else {
                Data->argument[i++] = g_strdup("exe");
            }
        } 
        if(gtk_toggle_button_get_active ((GtkToggleButton *) 
                    g_object_get_data(G_OBJECT(dialog), "octal_p")))
        {
            Data->argument[i++] = g_strdup("-o");
            const gchar *c = gtk_entry_get_text ((GtkEntry *)
                    g_object_get_data(G_OBJECT(dialog), "permissions_entry"));
            if (c && strlen(c)) Data->argument[i++] = g_strdup((gchar *)c);
            else Data->argument[i++] = g_strdup("0666");
        } 

        if(gtk_toggle_button_get_active ((GtkToggleButton *) 
                    g_object_get_data(G_OBJECT(dialog), "uid")))
        {
            GtkWidget *entry = gtk_bin_get_child(GTK_BIN(g_object_get_data(G_OBJECT(dialog), "uid_combo")));
            const gchar *val = gtk_entry_get_text (GTK_ENTRY (entry));
            if(val && strlen(val)) {
                Data->argument[i++] = g_strdup("-u");
                struct passwd *pw = getpwnam (val);
                if(pw) {
                    Data->argument[i++] = g_strdup_printf("%d", pw->pw_uid);
                } else {
                    Data->argument[i++] = g_strdup_printf("%d",atoi(val));
                }

            }
        }
        if(gtk_toggle_button_get_active ((GtkToggleButton *) 
                    g_object_get_data(G_OBJECT(dialog), "gid")))
        {
            GtkWidget *entry = gtk_bin_get_child(GTK_BIN(g_object_get_data(G_OBJECT(dialog), "gid_combo")));
            const gchar *val = gtk_entry_get_text (GTK_ENTRY (entry));
            if(val && strlen(val)) {
                Data->argument[i++] = g_strdup("-g");
                struct group *gr = getgrnam (val);
                if(gr) {
                    Data->argument[i++] = g_strdup_printf("%d", gr->gr_gid);
                } else {
                    Data->argument[i++] = g_strdup_printf("%d",atoi(val));
                }

            }
        }
        if(!gtk_toggle_button_get_active ((GtkToggleButton *) 
                    g_object_get_data(G_OBJECT(dialog), "case_sensitive")))
        {
            Data->argument[i++] = g_strdup("-i");
        } 
        if(gtk_toggle_button_get_active ((GtkToggleButton *)
                    g_object_get_data(G_OBJECT(dialog), "line_count")))
        {
            Data->argument[i++] = g_strdup("-c");
        } 
        if(gtk_toggle_button_get_active ((GtkToggleButton *) 
                    g_object_get_data(G_OBJECT(dialog), "xdev")))
        {
            Data->argument[i++] = g_strdup("-a");
        } 
        const gchar *token = gtk_entry_get_text(GTK_ENTRY(g_object_get_data(G_OBJECT(dialog), "grep_entry")));
        if(token) {
              if(gtk_toggle_button_get_active ((GtkToggleButton *)
                        g_object_get_data(G_OBJECT(dialog), "ext_regexp")))
            {
                Data->argument[i++] = g_strdup("-E");
            } else {
                Data->argument[i++] = g_strdup("-e");
            }
            Data->argument[i++] = g_strdup(token);

            /* options for grep: ***** */
            if(!gtk_toggle_button_get_active ((GtkToggleButton *) 
                        g_object_get_data(G_OBJECT(dialog), "look_in_binaries")))
            {
                Data->argument[i++] = g_strdup("-I");
            }
            if(gtk_toggle_button_get_active ((GtkToggleButton *) 
                        g_object_get_data(G_OBJECT(dialog), "match_words")))
            {
                Data->argument[i++] = g_strdup("-w");
            }
            else if(gtk_toggle_button_get_active ((GtkToggleButton *) 
                        g_object_get_data(G_OBJECT(dialog), "match_lines")))
            {
                Data->argument[i++] = g_strdup("-x");
            }
            else if(gtk_toggle_button_get_active ((GtkToggleButton *) 
                        g_object_get_data(G_OBJECT(dialog), "match_no_match")))
            {
                Data->argument[i++] = g_strdup("-L");
            }
        }

        /* select list */
        const char *ft[] = {
            "any",
            "reg",
            "dir",
            "sym",
            "sock",
            "blk",
            "chr",
            "fifo",
            NULL
        };
        const gchar *ftype = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT(
                g_object_get_data(G_OBJECT(dialog), "file_type_om")));
        for(int j = 0; ftypes[j] != NULL; j++) {
            if(ftype && strcmp (ftype, _(ftypes[j])) == 0) {
                Data->argument[i++] = g_strdup("-t");
                Data->argument[i++] = g_strdup(ft[j]);
                break;
            }
        }
        const gchar *filter = gtk_entry_get_text(GTK_ENTRY(g_object_get_data(G_OBJECT(dialog), "filter_entry")));
        /* apply wildcard filter if not specified (with dotfiles option) */
        Data->argument[i++] = g_strdup("-f");
        if(filter && strlen (filter)) {
            Data->argument[i++] = g_strdup(filter);
        } else {
            Data->argument[i++] = g_strdup("*");
            Data->argument[i++] = g_strdup("-D");
        }

        /* last Data->argument is the path */
        Data->argument[i++] = path;
        Data->argument[i] = (char *)0;
    }
};

}
#endif
