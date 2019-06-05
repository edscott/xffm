#ifndef FINDSIGNALS__HH
# define FINDSIGNALS__HH

static const gchar *ftypes[] = {
    N_("Regular"),
    N_("Directory"),
    N_("Symbolic Link"),
    N_("Socket"),
    N_("Block device"),
    N_("Character device"),
    N_("FIFO"),
    N_("Any"),
    NULL
};
const char *ft[] = {
    "reg",
    "dir",
    "sym",
    "sock",
    "blk",
    "chr",
    "fifo",
    "any",
    NULL
};

typedef struct fgrData_t{
    GtkWindow *dialog;
    pid_t pid;
    gint resultLimit;
    gint resultLimitCounter;
    GSList *findList;
    gchar **argument;
    gboolean done;
}fgrData_t;

static GHashTable *controllerHash = NULL;
static GSList *lastFind = NULL;

namespace xf {

GtkWindow *findDialog;

template <class Type> class TreeView;
template <class Type> class LocalModel;

template <class Type>
class findSignals: public Run<Type>{
    using gtk_c = Gtk<double>;
    using print_c = Print<double>;
    using run_c = Run<double>;
    using util_c = Util<double>;
public:

    static void
    openDnDBox(GtkWindow *parent, const gchar *title, GSList *list){
        //if (g_slist_length(list) == 0) return NULL;
        
        // Create liststore for DnD
        auto dialog = GTK_WINDOW(Gtk<Type>::_quickHelp(parent, _("Results"), NULL, title));
	//gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	//gtk_window_set_transient_for (GTK_WINDOW (dialog), parent);
        
        auto vbox = GTK_BOX(g_object_get_data(G_OBJECT(dialog), "vbox"));
	auto scrolledWindow = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new (NULL, NULL));
        gtk_scrolled_window_set_policy (scrolledWindow, GTK_POLICY_NEVER,GTK_POLICY_AUTOMATIC);
        gtk_widget_set_size_request (GTK_WIDGET(scrolledWindow), 200, 200);

        gtk_box_pack_start (vbox, GTK_WIDGET(scrolledWindow), TRUE, TRUE, 0);
        
        auto model = gtk_list_store_new(2, GDK_TYPE_PIXBUF, G_TYPE_STRING);
        GtkTreeIter iter;

        for (auto l=list; l && l->data; l = l->next){
            auto path = (gchar *)l->data;
            gtk_list_store_append (model, &iter);
            gtk_list_store_set (model, &iter, 1, path, -1);

            auto pixbuf = LocalModel<Type>::getIcon(path);
            gtk_list_store_set (model, &iter, 0, pixbuf, -1);
            g_object_unref(pixbuf);
            
        }

        auto treeView = GTK_TREE_VIEW(gtk_tree_view_new());
	gtk_tree_view_set_model(treeView, GTK_TREE_MODEL(model));
        TreeView<Type>::appendColumnPixbuf(treeView, 0);
        TreeView<Type>::appendColumnText(treeView, _("Path"), 1);
       
        auto selection = gtk_tree_view_get_selection (treeView);
        gtk_tree_selection_set_mode (selection,  GTK_SELECTION_MULTIPLE);
        gtk_tree_view_set_rubber_banding (treeView, TRUE);        
        gtk_container_add (GTK_CONTAINER(scrolledWindow), GTK_WIDGET(treeView));

        gtk_widget_show(GTK_WIDGET(scrolledWindow));
        gtk_widget_show(GTK_WIDGET(treeView));

        gtk_widget_show_all (GTK_WIDGET(dialog));
        gtk_widget_hide(GTK_WIDGET(parent));
        
        
    }

    static void onSizeAllocate (GtkWidget    *widget,
		   GdkRectangle *allocation,
		   gpointer      data){
        static gint lastX=-1;
        static gint lastY=-1;
        if (allocation->width == lastX && allocation->height == lastY) return;
	TRACE("dialog.hh::onSizeAllocate():SIZE allocate\n");
        lastX = allocation->width;
        lastY = allocation->height;

        // Save selection width and height to .ini
	Settings<Type>::setSettingInteger( "xffind", "width", lastX);
	Settings<Type>::setSettingInteger( "xffind", "height", lastY);

    }

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
	TRACE("fixme: signals::on_buttonHelp\n");
	gtk_c::quick_help(dialog_, message);
    }


    static void
    onClearButton (GtkWidget * button, gpointer data) {
	GtkTextView *diagnostics = GTK_TEXT_VIEW(data);
	TRACE("fixme: signals::onClearButton\n");
        print_c::clear_text(diagnostics);
        print_c::hide_text(diagnostics);

    }

    static void
    onEditButton (GtkWidget * button, gpointer data) {
	GtkTextView *diagnostics = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(data), "diagnostics"));
	TRACE("fixme: signals::onEditButton\n");
        //print_c::print_status(diagnostics, g_strdup("fixme: signals::onEditButton testing run\n"));
        print_c::showTextSmall(diagnostics);        
        edit_command(data);
    }

    static void
    onCloseButton (GtkWidget * button, gpointer data) {
	TRACE("fixme: signals::onCloseButton\n");
        GtkWidget *dialog = GTK_WIDGET(data);
        gtk_widget_hide(dialog);
        while (gtk_events_pending()) gtk_main_iteration();
        gtk_main_quit();
        exit(1);
    }

    static void
    onCloseDetails (GtkWidget * button, gpointer data) {
        auto advancedButton = GTK_TOGGLE_BUTTON(g_object_get_data(G_OBJECT(data), "advancedButton"));
        auto advancedDialog = GTK_WIDGET(g_object_get_data(G_OBJECT(data), "advancedDialog"));
        gtk_widget_hide(advancedDialog);
        gtk_toggle_button_set_active(advancedButton, FALSE);
    }

    static void
    onDetails (GtkWidget * button, gpointer data) {
        auto advancedDialog = GTK_WIDGET(g_object_get_data(G_OBJECT(data), "advancedDialog"));
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button))){
            gtk_widget_show_all(advancedDialog);
        } else {
            gtk_widget_hide(advancedDialog);
        }
    }

    static void
    onCancelButton (GtkWidget * button, gpointer data) {
	TRACE("fixme: signals::onCancelButton\n");
        cancel_all(data);
    }

    static void
    onFindButton (GtkWidget * button, gpointer dialog) {
        if (!controllerHash){
            controllerHash = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, g_free);
        }
	GtkTextView *diagnostics = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(dialog), "diagnostics"));
        updateCompletions(dialog);
        
        print_c::showTextSmall(diagnostics);
        on_find_clicked_action (GTK_WINDOW(dialog));
    }

    static gint
    onCloseEvent (GtkWidget * widget, GdkEventKey * event, gpointer data) {
        onCloseButton (NULL, data);
        return FALSE;
    }

    static void grepOptions (GtkEntry *widget, gpointer data) {
        gboolean active = FALSE;
	gchar *text = util_c::compact_line(gtk_entry_get_text(GTK_ENTRY(widget)));
        if (text && strlen(text)) active = TRUE;
        g_free(text);
        if (data){
            gtk_widget_set_sensitive(GTK_WIDGET(data), active);
        }        
    }    
    static gint  on_key_release (GtkWidget * widget, GdkEventKey * event, gpointer data) {
        grepOptions(GTK_ENTRY(widget), data);
        return FALSE;
    }
 
    static void onSelectionReceived (GtkWidget        *widget,
               GtkSelectionData *data,
               guint             time,
               gpointer          user_data){
        grepOptions(GTK_ENTRY(widget), data);
    }
  
    static gint
    on_completion (GtkWidget * widget, GdkEventKey * event, gpointer data) {
        GtkEntryCompletion *completion = gtk_entry_get_completion(GTK_ENTRY(widget));
        gtk_entry_completion_complete (completion);
        return FALSE;
    }


private:
    
    static void
    quick_command (GtkWidget * button, gpointer data, gboolean scrollUp) {
	GtkWindow *dialog_=GTK_WINDOW(g_object_get_data(G_OBJECT(button), "dialog_"));
	GtkTextView *diagnostics = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(dialog_), "diagnostics"));
	TRACE("fixme: signals::quick_command\n");
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
 
        history = g_build_filename (PATH_HISTORY);
        entry = GTK_ENTRY(g_object_get_data(G_OBJECT(dialog), "path_entry"));
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

    static gboolean removeFunc(gpointer key, gpointer value, gpointer data){
        GtkTextView *diagnostics = GTK_TEXT_VIEW(data);
        print_c::print_icon(diagnostics, "edit-delete", "bold", 
                g_strdup_printf("%s: \"%s\"\n",_("Cancel"), (gchar *) value));
        //Signal process controller to kill child process.
        kill(GPOINTER_TO_INT(key), SIGUSR2);
        return FALSE;
    }
    static void
    cancel_all(void * dialog){
        void *diagnostics = g_object_get_data(G_OBJECT(dialog), "diagnostics");
        g_hash_table_foreach_remove (controllerHash, removeFunc, diagnostics);
        GtkWidget *cancel = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "cancel_button"));
        gtk_widget_set_sensitive(cancel, FALSE);
    }

    static gboolean
    Cleanup (void *data) {
       fgrData_t *Data = (fgrData_t *)data;
       GtkWindow *dialog = Data->dialog;
       if (g_hash_table_size(controllerHash) == 0){
            GtkWidget *cancel = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "cancel_button"));
            gtk_widget_set_sensitive(cancel, FALSE);
       }

       GtkWidget *edit_button = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "edit_button"));
       if (g_slist_length(lastFind)){
            const gchar *editor = getenv("EDITOR");
            if (!editor || strlen(editor)==0){
                GtkTextView *diagnostics = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(dialog), "diagnostics"));
                print_c::print_icon(diagnostics, "dialog-warning", 
                        g_strdup_printf("%s (EDITOR=\"\")\n", _("No editor for current action.")));
                return FALSE;
            }
	    gtk_widget_set_sensitive(GTK_WIDGET(edit_button), TRUE);
       } else {
	    gtk_widget_set_sensitive(GTK_WIDGET(edit_button), FALSE);
       }
       if (Data->done) {
           freeFgrData(Data);
           return FALSE;
       }
       return TRUE;
    }

    static void
    forkCleanup (void *data) {
       TRACE("forkCleanup\n");
        fgrData_t *Data = (fgrData_t *)data;
        g_hash_table_remove(controllerHash, GINT_TO_POINTER(Data->pid));
        g_timeout_add_seconds(1, Cleanup, (void *)Data);
        
    }

    static void
    stderr_f (void *data, void *stream, int childFD) {
        fgrData_t *Data = (fgrData_t *)data;
        GtkTextView *diagnostics = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(Data->dialog), "diagnostics"));
        if (!gtk_widget_is_visible(GTK_WIDGET(diagnostics))) return;

        char *line;
        line = (char *)stream;


        if(line[0] != '\n') {
            // FIXME use print_icon()
            print_c::print(diagnostics, "red", g_strdup(line));
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
    stdout_f (void *data, void *stream, int childFD) {
        fgrData_t *Data = (fgrData_t *)data;
        char *line;
        line = (char *)stream;
        TRACE("FORK stdout: %s\n", line);

        if(line[0] == '\n') return;
        GtkTextView *diagnostics = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(Data->dialog), "diagnostics"));

        if (Data->resultLimit > 0 && Data->resultLimit==Data->resultLimitCounter) {
            gchar *g=g_strdup_printf("%s. %s %d", _("Results"), _("Upper limit:"), Data->resultLimit);
            print_c::print_icon(diagnostics, "dialog-warning", "green", g_strconcat(g, "\n", NULL));
            print_c::print_icon(diagnostics, "dialog-info", "blue",  g_strconcat(_("Counting files..."), "\n", NULL));
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
                gchar *message = g_strdup_printf(_("%s Finished : %s"), xffindProgram, plural_text);
                gchar *g = g_strdup_printf("%c[31m%s\n",27, message);
                print_c::print_icon (diagnostics, "process-stop", g);
                // Free last find results
                GSList *list = lastFind;
                for(;list; list=list->next) g_free(list->data);
                g_slist_free(lastFind);
                // assign new find results
                lastFind = Data->findList;
                Data->done = TRUE;
                list = lastFind;
                for (;list && list->data; list=list->next){
                    TRACE("last find: %s\n", (gchar *)list->data);
                }
                openDnDBox(findDialog, plural_text, lastFind);
                

                // cleanupmake
                //
                g_free(plural_text);
                g_free(message);
        } else {
            if (!gtk_widget_is_visible(GTK_WIDGET(diagnostics))) return;
            gchar *file = g_strdup(line);
            if (strchr(file, '\n')) *strchr(file, '\n') = 0;
            if (g_file_test(file, G_FILE_TEST_EXISTS)) {
                Data->resultLimitCounter++; 
                if (Data->resultLimit ==0 ||
                    (Data->resultLimit > 0 && Data->resultLimit > Data->resultLimitCounter) ) {
                    print_c::print(diagnostics, g_strdup_printf("%s\n", file));
                    TRACE("--> %s\n",file);
                    //TRACE("resultLimitCounter:%d %s\n", Data->resultLimitCounter, line);
                    Data->findList = g_slist_prepend(Data->findList, file);
                }
            } else {
               TRACE("ignoring: \"%s\"\n", file);
               g_free(file);
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

    static gboolean
    freeFgrData(void *data){
        fgrData_t *Data = (fgrData_t *)data;
        gchar **a=Data->argument;
        for (;*a; a++) g_free(*a);
        g_free(Data->argument);
        g_free(Data);
        return FALSE;
    }

    static gboolean
    on_find_clicked_action (GtkWindow *dialog) {
        GtkTextView *diagnostics = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(dialog), "diagnostics"));
        // Get the search path.
        GtkEntry *entry = GTK_ENTRY(g_object_get_data(G_OBJECT(dialog), "path_entry"));
        gchar *path = g_strdup(gtk_entry_get_text(entry));
        /* tilde expansion */
        if(path[0] == '~'){
            gchar *t = g_strconcat(g_get_home_dir(),"/", path+1, NULL);
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
        Data->done = FALSE; // (This is redundant with calloc, here just for clarity).


        GtkWidget *cancel = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "cancel_button"));
        //gtk_widget_show(cancel);
        gtk_widget_set_sensitive(cancel, TRUE);

        /* get the parameters set by the user... *****/
        get_arguments(path, Data);


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
        Data->pid = run_c::thread_run(Data, (const gchar **)Data->argument, stdout_f, stderr_f, forkCleanup);

        // When the dialog is destroyed on action, then output is going to a
        // widgets_p away from dialog window, so we let the process run on and on.
        gchar *command = g_strdup("");
        gchar **p=Data->argument;
        for (;p && *p; p++){
            gchar *g = g_strconcat(command, " ", *p, NULL);
            g_free(command);
            command = g;
        }
        g_hash_table_replace(controllerHash, GINT_TO_POINTER(Data->pid), command);
        print_c::print_icon(diagnostics, "system-search", "green", g_strconcat( _("Searching..."), "\n", NULL));
        print_c::print_icon(diagnostics, "system-run", "bold", 
                g_strdup_printf("%s: \"%s\"\n",_("Searching..."), (gchar *) command));
	return FALSE;
    }

    static void
    get_arguments(gchar *path, fgrData_t *Data){
        gint i=0;
        GtkWindow *dialog = Data->dialog;

        /* limit */
        Data->resultLimit = gtk_spin_button_get_value_as_int (
                GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(dialog), "upper_limit_spin")));
        Data->resultLimitCounter = 0; // fgr ends with "fgr search complete"
        Data->argument = (gchar **)calloc(MAX_COMMAND_ARGS, sizeof(gchar *));
        if (!Data->argument){
            std::cerr<<"calloc error at get_arguments()\n";
            exit(1);
        }
        
        /* the rest */
        Data->argument[i++] = g_strdup(xffindProgram);
        Data->argument[i++] = g_strdup("--fgr");
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
        if(token && strlen(token)) {
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
        //Data->argument[i++] = g_strdup_printf("\"%s\"", path); g_free(path);
        Data->argument[i++] = path;
        Data->argument[i] = (char *)0;
    }

    static void
    edit_command (gpointer data) {
        GtkWindow *dialog = GTK_WINDOW(data);
        GtkTextView *diagnostics = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(dialog), "diagnostics"));
        GSList *list = lastFind;

        /*if (!list || g_slist_length(list) < 1) {
            rfm_diagnostics(widgets_p, "xffm/stock_dialog-warning",NULL);
            rfm_diagnostics(widgets_p, "xffm_stderr", _("Search returned no results"), "\n", NULL);
            return;
        }*/

        const gchar *editor = getenv("EDITOR");
        if (!editor || strlen(editor)==0){
            print_c::print_error(diagnostics, g_strdup_printf("%s (EDITOR=\"%s\")\n",
                        _("No editor for current action."), editor));
            return;
        }
        TRACE("editor = %s\n", editor);
        gchar *command;
	if (Mime<Type>::runInTerminal(editor)){
	    command = Mime<Type>::mkTerminalLine(editor, "");
	} else {
	    command = g_strdup(editor);
	}
      
        TRACE("command = %s\n", command);

        for (; list && list->data; list=list->next){
            gchar *g = g_strconcat(command, " \"", (gchar *)list->data, "\"", NULL);
            g_free(command);
            command = g;
        }
        TRACE("command args = %s\n", command);

        // Hack: for nano or vi, run in terminal
        gboolean in_terminal = FALSE;
        if (strstr(command, "nano") || 
                (strstr(command, "vi") && !strstr(command, "gvim")))
        {
            in_terminal = TRUE;
        }

        TRACE("thread_run %s\n", command);
        run_c::thread_run(diagnostics, command, FALSE);
        //RFM_THREAD_RUN2ARGV(widgets_p, command, in_terminal);
        
        //widgets_p->workdir = g_strdup(g_get_home_dir());
    }


};

} // namespace xf
#endif
