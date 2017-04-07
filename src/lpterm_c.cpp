//#define DEBUG_TRACE 1

#include "view_c.hpp"
#include "lpterm_c.hpp"
#include "run_button_c.hpp"

#ifdef HAVE_READLINE_HISTORY_H
# include <readline/history.h>
#endif

static gboolean on_status_button_press ( GtkWidget * , GdkEventButton *, gpointer);
static gboolean status_keyboard_event (GtkWidget *, GdkEventKey *, gpointer);

//////////////////////////////////////////////////////////////////

lpterm_c::lpterm_c(void *data): run_c(data){
    active = FALSE;
    view_c *view_p = (view_c *)data;
    iconview = view_p->get_iconview();
    status_icon = view_p->get_status_icon();
    iconview_icon = view_p->get_iconview_icon();
    status_button = view_p->get_status_button();
    pthread_mutexattr_t r_attr;
    pthread_mutexattr_init(&r_attr);
    pthread_mutexattr_settype(&r_attr, PTHREAD_MUTEX_RECURSIVE);
    rbl_mutex = (pthread_mutex_t *)calloc(1, sizeof(pthread_mutex_t));
    pthread_mutex_init(rbl_mutex, &r_attr);
    run_button_list = NULL;
    g_signal_connect (status, "key-press-event", G_CALLBACK (status_keyboard_event), data);
    g_signal_connect (status_button, "button-press-event", G_CALLBACK (on_status_button_press), (void *)this);
}

lpterm_c::~lpterm_c(void){
    GList *l = run_button_list;
    pthread_mutex_lock(rbl_mutex);
    for (; l && l->data; l=l->next){
        run_button_c *rb_p = (run_button_c *)l->data;
        unreference_run_button(rb_p);
    }
    g_list_free(run_button_list);
    run_button_list=NULL;
    pthread_mutex_unlock(rbl_mutex);
    pthread_mutex_destroy(rbl_mutex);
    g_free(rbl_mutex);
}

void
lpterm_c::reference_run_button(run_button_c *rb_p){
    DBG("lpterm_c::reference_run_button(%p)\n", (void *)rb_p);
    pthread_mutex_lock(rbl_mutex);
    run_button_list = g_list_prepend(run_button_list, (void *)rb_p);
    pthread_mutex_unlock(rbl_mutex);
}

void
lpterm_c::unreference_run_button(run_button_c *rb_p){
    DBG("lpterm_c::unreference_run_button(%p)\n", (void *)rb_p);
    pthread_mutex_lock(rbl_mutex);
    void *p = g_list_find(run_button_list, (void *)rb_p);
    if (p){
        run_button_list = g_list_remove(run_button_list, (void *)rb_p);
        delete (rb_p);
    }
    pthread_mutex_unlock(rbl_mutex);
}

gboolean 
lpterm_c::window_keyboard_event(GdkEventKey * event, void *data)
{
    view_c *view_p = (view_c *)data;
    TRACE("lpterm_c::window_keyboard_event\n");
    /* asian Input methods */
    if(event->keyval == GDK_KEY_space && (event->state & (GDK_MOD1_MASK | GDK_SHIFT_MASK))) {
        return FALSE;
    }


    gint ignore[]={
        GDK_KEY_Control_L,
        GDK_KEY_Control_R,
        GDK_KEY_Shift_L,
        GDK_KEY_Shift_R,
        GDK_KEY_Shift_Lock,
        GDK_KEY_Caps_Lock,
        GDK_KEY_Meta_L,
        GDK_KEY_Meta_R,
        GDK_KEY_Alt_L,
        GDK_KEY_Alt_R,
        GDK_KEY_Super_L,
        GDK_KEY_Super_R,
        GDK_KEY_Hyper_L,
        GDK_KEY_Hyper_R,
	GDK_KEY_ISO_Lock,
	GDK_KEY_ISO_Level2_Latch,
	GDK_KEY_ISO_Level3_Shift,
	GDK_KEY_ISO_Level3_Latch,
	GDK_KEY_ISO_Level3_Lock,
	GDK_KEY_ISO_Level5_Shift,
	GDK_KEY_ISO_Level5_Latch,
	GDK_KEY_ISO_Level5_Lock,
        0
    };

    gint i;
    for (i=0; ignore[i]; i++) {
        if(event->keyval ==  ignore[i]) {
	    TRACE("lpterm_c::window_keyboard_event: key ignored\n");
            return TRUE;
        }
    }


    TRACE( "lpterm_c::window_keyboard_event: lpterm is active = %d\n", event->keyval, active);

    if (!active && is_iconview_key(event)) {
	TRACE("lpterm_c::window_keyboard_event: Sending key to iconview default handler.\n");
	return FALSE;
    }

    if (active && (event->keyval == GDK_KEY_Escape)) {
	TRACE("lpterm_c::window_keyboard_event: set_active_lp = %d\n", FALSE);
        lp_set_active(FALSE); 
	return TRUE;
    }

    if (!active) {
	TRACE("lpterm_c::window_keyboard_event: set_active_lp = %d\n", TRUE);
        lp_set_active(TRUE); 
	if (event->keyval == GDK_KEY_Tab){
	    event->keyval = GDK_KEY_Escape;
	}
	if (event->keyval == GDK_KEY_Escape){
	    return TRUE;

        } 
    }
    // By now we have a lp key to process
    TRACE( "lpterm_c::window_keyboard_event: send key to status dialog for lpterm command\n");
    lpterm_keyboard_event(event, data);
    return TRUE;


    /* FIXME: CTL callbacks...
    if (rodent_do_callback(event->keyval, event->state)) {
        TRACE("lpterm_c::window_keyboard_event: Tried callback with keyval!\n");
        return TRUE;
    } */
    
    /* FIXME: icon selection by keyboard
    if (view_p->selection_list) {
	// selection list must be redefined...
	update_reselect_list(widgets_p);
	TRACE( "lpterm_c::window_keyboard_event: Selection---> %p\n", view_p->selection_list);	
    } */
    
    //False defaults to status line keybinding signal callback
    return FALSE;
}
    
    
gboolean
lpterm_c::is_iconview_key(GdkEventKey *event){
    gint keys[] = {
	GDK_KEY_Right,
	GDK_KEY_Left,
        GDK_KEY_Up,
        GDK_KEY_Down,
        GDK_KEY_Page_Up,
        GDK_KEY_Page_Down,
        GDK_KEY_End,
        GDK_KEY_Home,
        GDK_KEY_Return,
        GDK_KEY_KP_Enter
    };
    gint *key=keys;
    for (;key && *key>0; key++){
	if (event->keyval == *key) {
	    return TRUE;
	}
    }
    return FALSE;
}

gboolean
lpterm_c::lp_get_active(void){return active;}

void
lpterm_c::lp_set_active(gboolean state){
    active = state;
    if (state){
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
    }
    return;
}


/*
   Ctrl-Left               Word left
   Ctrl-Right              Word right
   Ctrl-Y                  Delete line
   Ctrl-K                  Delete to end of line
   Ctrl-BS                 Delete word left
   Ctrl-Del        	   Delete word right
   Ctrl-A                  Select all text
   Ctrl-U                  Deselect block
   Ctrl-V       	   Paste block from clipboard
   Ctrl-X                  Cut block
   Ctrl-C                  Copy block to clipboard
   */


gboolean
lpterm_c::is_lpterm_key(GdkEventKey * event){
    // No mask, then it is a lpterm key:
    if (event->state == 0) return TRUE;
    // Shift page up/down, ok:
    if ((event->state & GDK_SHIFT_MASK)
	&&
	((event->keyval == GDK_KEY_Page_Up) ||
	 (event->keyval == GDK_KEY_Page_Down)) 
       ){
	return TRUE;
    }

    // Control mask exceptions
    gint keys[] = {
	GDK_KEY_Tab,
	GDK_KEY_Right,
	GDK_KEY_Left,
	GDK_KEY_y,
	GDK_KEY_Y,
	GDK_KEY_k,
	GDK_KEY_K,
	GDK_KEY_Delete,
	GDK_KEY_BackSpace,
	GDK_KEY_a,
	GDK_KEY_A,
	GDK_KEY_u,
	GDK_KEY_U,
	GDK_KEY_v,
	GDK_KEY_V,
	GDK_KEY_x,
	GDK_KEY_X,
	GDK_KEY_c,
	GDK_KEY_C,
	-1
    };
    gint *key=keys;
    for (;key && *key>0; key++){
	if (event->keyval == *key) {
	    return TRUE;
	}
    }
    return FALSE;
}

gboolean
lpterm_c::internal_cd (gchar ** argvp) {   
    gchar *gg=NULL;

    if(argvp[1]) {
	if (*argvp[1] == '~'){
	    if (strcmp(argvp[1], "~")==0 || 
		    strncmp(argvp[1], "~/", strlen("~/"))==0){
		gg = g_strdup_printf("%s%s", g_get_home_dir (), argvp[1]+1);
	    } else {
		//gchar *tilde_dir = rfm_get_tilde_dir(argvp[1]);
		gchar *tilde_dir = get_tilde_dir(argvp[1]);
		if (tilde_dir) gg = g_strconcat(tilde_dir, strchr(argvp[1], '/')+1, NULL);
		else gg = g_strdup(argvp[1]);
		g_free(tilde_dir);	
	    }
	} else {
	    gg = g_strdup(argvp[1]);
	}

    } else {
        gg = g_strdup(g_get_home_dir ());
    }
    show_text();

    // must allow relative paths too.
    if (!g_path_is_absolute(gg)){
	if(!g_file_test (get_workdir(), G_FILE_TEST_IS_DIR)) 
	{
	    print_error(g_strdup_printf("%s: %s\n", gg, strerror (ENOENT)));
	    g_free (gg);
	    return TRUE;
        } 
	gchar *fullpath = g_strconcat(get_workdir(), G_DIR_SEPARATOR_S, gg, NULL);
	g_free(gg);
	gg = fullpath;
    }

    gchar *rpath = realpath(gg, NULL);
    if (!rpath){
	print_error(g_strdup_printf("%s: %s\n", gg, strerror (ENOENT)));
	g_free (gg);
	return TRUE;
    }

    if (gg[strlen(gg)-1]==G_DIR_SEPARATOR || strstr(gg, "/..")){
	g_free(gg);
	gg=rpath;
    } else {
        g_free (rpath);
    }

    print_tag("tag/green", g_strdup_printf("cd %s\n", gg));
    clear_status();

    view_c *view_p =(view_c *)view_v;
    view_p->reload(gg);

    g_free (gg);
    return TRUE;
}


gboolean
lpterm_c::process_internal_command (const gchar *command) {
    gint argcp;
    gchar **argvp;
    GError *error = NULL;
    if(!g_shell_parse_argv (command, &argcp, &argvp, &error)) {
        print_error(g_strdup_printf("%s\n", error->message));
        return TRUE;
    } else if(strcmp (argvp[0], "cd")==0) {
        // shortcircuit chdir
        internal_cd (argvp);
        g_strfreev (argvp);
        return TRUE;
    }
    g_strfreev (argvp);
    return FALSE;
}
 
gchar *
lpterm_c::sudo_fix(const gchar *command){
    if (!strstr(command, "sudo ")) return NULL; 
    gchar *new_command = NULL;
    if (strncmp(strstr(command, "sudo "), "sudo -A ", strlen("sudo -A "))!=0)
    {
        gchar *original_head=g_strdup(command);
        gchar *pos = strstr(original_head, "sudo ");
        if (pos){
	    *pos = 0;
	    gchar *tail=g_strdup(strstr(command, "sudo ")+strlen("sudo "));
	    new_command = g_strconcat(original_head, "sudo -A ", tail, NULL);
	    g_free(tail);
        }
	g_free(original_head);
    }
    return new_command;
}

void *
lpterm_c::shell_command(const gchar *c, gboolean save){
    // Make sure any sudo command has the "-A" option
    gchar *command = sudo_fix(c);
    pid_t pid = thread_run(command?command:c);
    if (!pid) return NULL; 
    run_button_c *run_button_p = NULL;
    run_button_p = new run_button_c(view_v, c, pid, run_in_shell(c));
    // We save the original sudo command, not the one modified with "-A"
    if (save) csh_save_history(c);
    g_free (command);
    return (void *)run_button_p;
}

void *
lpterm_c::shell_command(const gchar *c){
    return shell_command(c, TRUE);
}

void 
lpterm_c::run_lp_command(void){
    gchar *command = get_current_text();
    gchar ** commands = NULL;
    if (strchr(command, ';')) commands = g_strsplit(command, ";", -1);
    if (!commands) {
        commands = (gchar **) calloc(2, sizeof(gchar *));
        commands[0] = g_strdup(command); 
    }
    gchar **c;
    for (c=commands; c && *c; c++){
        if(process_internal_command (*c)) continue;
        // shell to command
        shell_command(*c);
        clear_status();
    }
    g_strfreev(commands); 
    g_free(command);
}

void
lpterm_c::open_terminal(void){
    const gchar *terminal = what_term();
    shell_command(terminal, FALSE);
}

gint
lpterm_c::lpterm_keyboard_event( GdkEventKey * event, gpointer data) {
    TRACE( "lpterm_c::lpterm_keyboard_event...\n");
    if(!event) {
        g_warning ("on_status_key_press(): returning on event==0\n");
        return TRUE;
    }
    view_c *view_p =(view_c *)data; 
    gtk_widget_grab_focus (GTK_WIDGET(status));

    
    if(event->keyval == GDK_KEY_Up || event->keyval == GDK_KEY_KP_Up) {
        if (is_completing()) csh_completion(1);
        else csh_history(1);
        return TRUE;
    }
    if(event->keyval == GDK_KEY_Down || event->keyval == GDK_KEY_KP_Down) {
        if (is_completing()) csh_completion(-1);
        else csh_history(-1);

        return TRUE;
    }
    // On activate, run the lpcommand.
    if((event->keyval == GDK_KEY_Return) || (event->keyval == GDK_KEY_KP_Enter)) {
        csh_clean_start();
        run_lp_command();
        return TRUE;
    }
    if((event->keyval == GDK_KEY_Page_Up) || (event->keyval == GDK_KEY_Page_Down)) {
        gboolean retval;
        g_signal_emit_by_name ((gpointer)diagnostics, "key-press-event", event, &retval);
        
        return TRUE;
    }
    // tab for bash completion.
    if(event->keyval == GDK_KEY_Tab) {
        bash_completion();
        return TRUE;
    }

    // Let the internal callback do its business first.
    TRACE("Let the internal callback do its business first.\n");
    gboolean retval;
    g_signal_emit_by_name ((gpointer)status, "key-press-event", event, &retval);
    while (gtk_events_pending())gtk_main_iteration();
    TRACE("Now do our stuff.\n");

    // Now do our stuff.
    
    // If cursor is back at position 0, then reset.
    query_cursor_position();

    // On right or left, change the csh completion token.
    if((event->keyval == GDK_KEY_Right) || (event->keyval == GDK_KEY_KP_Right)) {
        csh_dirty_start();
    }
    // On right or left, change the csh completion token.
    if((event->keyval == GDK_KEY_Left) || (event->keyval == GDK_KEY_KP_Left)) {
        csh_dirty_start();
    }


    if (event->keyval >= GDK_KEY_space && event->keyval <= GDK_KEY_asciitilde){
        csh_dirty_start();
    }
    return retval;
}

#if 0
// work in progress: run command and open with dialog

// foward declaration (temporary)
gchar *
get_response_history (const gchar * title_txt,
                             const gchar * label_txt,
                             const gchar * extra_txt,
                             gchar * history_file,
                             const gchar * path,
                             const gchar * entry_text,
                             gchar * flagfile, 
			     const gchar * check_label, 
			     gint filechooser_action, 
			     const gchar * folder,
			     gint completion_type) ;

// Used by run and open-with callbacks
// This opens the confirmation or user input dialog.
gboolean
lpterm_c::execute (const gchar *work_dir, GList *selection_list) {
// "execute() is a thread function\n"

    //GSList *selection_list = execute_p->list;
    gboolean retval = TRUE;
    
    gchar *command_fmt=NULL;
    /* set the working directory */
    if (!g_file_test(work_dir, G_FILE_TEST_IS_DIR)) {
	work_dir = g_get_home_dir();
    }

    TRACE ("execute()...\n");
    gchar *files = NULL;
    gchar *files_txt;

    gchar *first_path=NULL;
    if(selection_list) {
#if 0
	// list of ACTUAL_NAME/DISPLAY_NAMEs
	files_txt = g_strdup_printf (_("Open with %s"),":   \n\n");
        if(g_list_length (selection_list)) {
            gchar *tt;
            gchar *ttt;
            files = g_strdup (" ");
            GSList *tmp = selection_list;
            for(; tmp; tmp = tmp->next) {
                record_entry_t *en = (record_entry_t *) tmp->data;
                char *b = g_path_get_basename (en->path);
                gchar *q = rfm_utf_string (rfm_chop_excess (b));
		// get DISPLAY_NAME
                tt = g_strconcat (files_txt, display_name, "\n", NULL);
                gchar *esc_path = rfm_esc_string (en->path);
		if (!first_path){
		    first_path=g_strdup(esc_path);
		    command_fmt=get_command_fmt(en);
		}
                ttt = g_strconcat (files, tt, " ", NULL);
                g_free (esc_path);
                g_free (files_txt);
                g_free (q);
                g_free (b);
                g_free (files);
                files_txt = tt;
                files = ttt;
            }
        }
#endif
    } else {
	// no selection
	files_txt = g_strdup_printf ("%s \n\n", _("Command:"));
    }
    NOOP ("OPEN: files=%s\n", files);
    NOOP ("OPEN: first_path=%s\n", first_path);
    gboolean interm = FALSE;
    gchar *g=NULL;
    {
        gchar *f = g_build_filename (RUN_DBH_FILE, NULL);
        gchar *ff = g_build_filename (RUN_FLAG_FILE, NULL);
        NOOP (stderr, "RUN_DBH_FILE=%s RUN_FLAG_FILE=%s\n", f, ff);
	
	gchar *title;
	if (selection_list) {
	    title=g_strdup_printf(_("Open with %s"),"");
	} else {
	    title=g_strdup(_("Execute Shell Command"));
	}
        g = get_response_history (title, 
		files_txt,
		_("Console: quickly run single commands -- write a command here and press enter."),
		f, 
		first_path,
		command_fmt, //NULL, // entry text
		ff,
		_("Run in Terminal"),
		GTK_FILE_CHOOSER_ACTION_OPEN,
		"/usr/bin",
		MATCH_COMMAND); 
       g_free (f);
        g_free (ff);
	NOOP(stderr, "got: \"%s\"\n", g);
    }
    g_free (first_path);
    g_free (command_fmt);

    g_free (files_txt);
    if(!g) {
        NOOP ("on_open_with_activate... !g\n");
	retval=FALSE;
	goto cleanup;
    }
    if(g[strlen (g) + 1])
        interm = TRUE;
#if 0
    if(selection_list) {
        /* if only one file selected, associate to mimetype... 
	 * but not default unless no other command available 
	 * (i.e., append, not prepend)*/
        if(g_slist_length (selection_list) == 1) {
	    record_entry_t *en = selection_list->data;
	    if(!en->mimetype || strcmp(en->mimetype, _("unknown"))==0) {
		if (IS_LOCAL_TYPE(en->type) && !en->mimemagic) {
		    en->mimemagic = rfm_rational(RFM_MODULE_DIR, "mime", en, "mime_magic", "mime_function");
		    if (!en->mimemagic) en->mimemagic = g_strdup(_("unknown"));
		} 
	    }   
	    const gchar *type = en->mimetype;
	    if (!type || strcmp(type, _("unknown"))==0) type = en->mimemagic;
            if(type) {
                gchar *command_fmt = g_strdup (g);
                if(interm) {
                    gchar *term_command = MIME_mk_terminal_line (command_fmt);
                    g_free (command_fmt);
                    command_fmt = term_command;
                }
		command_fmt = strip_path(command_fmt,en->path);
                NOOP(stderr, "OPEN: adding %s --> %s (from %s)\n", type, command_fmt, g);
		rfm_rational(RFM_MODULE_DIR,"mime", 
			(void *)(en->mimetype),
			(void *)command_fmt, "mime_append");
		// MIME_add would prepend, which is now deprecated:
		// MIME_add (view_p->mouse_event.selected_p->en->mimetype, command_fmt);
		g_free(command_fmt);
            }
        }
    }
#endif
    gchar *command;
    if(strstr (g, "%s")) {
        command = g_strdup_printf (g, (files)?files:"");
    } else {
        command = g_strdup_printf ("%s %s", g, (files)?files:"");
    }
    g_strstrip(command);
    g_free(g);
    NOOP (stderr,"OPEN: command = \"%s\"\n", command);
    g_free (files);
/*
    if(widgets_p->diagnostics_window){
	if (!rfm_threaded_get_visible(widgets_p->diagnostics_window)){
	    rfm_threaded_show_text(widgets_p);
	}
    } else {
	rfm_threaded_show_text(widgets_p);
    }
*/
#if 0
    // do the call with argv so that command not saved in lpterm history
    // (but this is broken when we have pipes or redirection)
    gboolean shell_it = FALSE;
    gchar *tokens="|><;&";
    gchar *c=tokens;
    for (c=tokens; *c; c++){
	if (strchr(command, *c)){
	    shell_it=TRUE;
	    break;
	} 
    }
    NOOP(stderr, "command = %s (shell=%d)\n", command, shell_it);
    if (shell_it){
	RFM_THREAD_RUN (widgets_p, command, interm);
    } else {
	RFM_THREAD_RUN2ARGV (widgets_p, command, interm);
    }
#endif
    //shell(command);
    g_free (command);
    // Cleanup
cleanup:;
#if 0
    GSList *list = selection_list;
    for (; list && list->data; list = list->next){
	record_entry_t *en = list->data;
	rfm_destroy_entry(en);
    }
    if (selection_list) g_slist_free(selection_list);
#endif
    TRACE("execute done...\n");
    return retval;
}

/////////////////////////////////////////  dialogs....

static void
toggle_activate (GtkToggleButton * togglebutton, gpointer user_data){
    extra_key_t *extra_key_p = (extra_key_t *) user_data;
    if(gtk_toggle_button_get_active (togglebutton))
        extra_key_p->flag1 = TRUE;
    else
        extra_key_p->flag1 = FALSE;
    rodent_save_flags (extra_key_p);
}


gchar *
get_response_history (const gchar * title_txt,
                             const gchar * label_txt,
                             const gchar * extra_txt,
                             gchar * history_file,
                             const gchar * path,
                             const gchar * entry_text,
                             gchar * flagfile, 
			     const gchar * check_label, 
			     gint filechooser_action, 
			     const gchar * folder,
			     gint completion_type) {


    extra_key_t extra_key;
    memset(&extra_key, 0, sizeof(extra_key_t));
    gint response = GTK_RESPONSE_NONE;
    GtkWidget *hbox, *label, *button, *dialog;
    GtkWidget *combo = NULL;
    void *combo_info = NULL;
    filechooser_t filechooser_v;
    filechooser_v.entry = NULL;

    if (folder && chdir(folder) < 0){
	DBG("cannot chdir(%s)\n", folder);
    }
    dialog = gtk_dialog_new ();
    gtk_window_set_type_hint(GTK_WINDOW(dialog), GDK_WINDOW_TYPE_HINT_DIALOG);

    if(widgets_p) {
	view_t *view_p=widgets_p->view_p;
        if(view_p && view_p->flags.type == DESKVIEW_TYPE) {
	    gtk_window_set_keep_above (GTK_WINDOW(dialog), TRUE);
	    gtk_window_stick (GTK_WINDOW(dialog));
	} else {   
            gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	    if (parent_window){
		gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (parent_window));
	    }
        } 
    } else {
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
    }


    gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);
    gtk_container_set_border_width (GTK_CONTAINER (dialog), 6);

    combo =  rfm_combo_box_new_with_entry ();
    gtk_widget_set_size_request (GTK_WIDGET (combo), 350, -1);

    if (extra_txt) {
	gchar *markup;
	label=gtk_label_new ("");
	markup = g_markup_printf_escaped ("<span style=\"italic\">%s</span>\n", extra_txt);
	gtk_label_set_markup (GTK_LABEL (label), markup);
	g_free (markup);	
        hbox = rfm_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area(GTK_DIALOG (dialog))), hbox, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	gtk_widget_show_all(hbox);
    }

    if(label_txt)
        label = gtk_label_new (label_txt);
    else
        label = gtk_label_new (_("Preparing"));
    hbox = rfm_hbox_new (FALSE, 6);
    gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area(GTK_DIALOG (dialog))), hbox, FALSE, FALSE, 0);
    GtkWidget *vbox = rfm_vbox_new (FALSE, 6);
    gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
    vbox = rfm_vbox_new (FALSE, 6);
    gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);

    gtk_box_pack_start (GTK_BOX (vbox), (GtkWidget *) combo, FALSE, FALSE, 0);


    gboolean combobox_active =GPOINTER_TO_INT(rfm_void(RFM_MODULE_DIR, "combobox", "module_active"));
    if(combo_info == NULL) {
	    combo_info = COMBOBOX_init_combo (combo, completion_type);
    } else {
	DBG("This is not happening... (get_response_history_f)\n");
	COMBOBOX_clear_history (combo_info);
    }
    COMBOBOX_set_quick_activate(combo_info, GINT_TO_POINTER(TRUE));
    GtkEntry *entry = COMBOBOX_get_entry_widget(combo_info);
    g_object_set_data(G_OBJECT(entry), "dialog", dialog);
    COMBOBOX_set_activate_function(combo_info, activate_entry);
    COMBOBOX_set_cancel_function(combo_info, cancel_entry);
    COMBOBOX_set_activate_user_data(combo_info, &response);
    COMBOBOX_set_cancel_user_data(combo_info, dialog);
    COMBOBOX_set_extra_key_completion_function(combo_info, extra_key_completionR);
    COMBOBOX_set_extra_key_completion_data(combo_info, &extra_key);


    COMBOBOX_read_history (combo_info, history_file);
    COMBOBOX_set_combo (combo_info);

    
    if(filechooser_action == GTK_FILE_CHOOSER_ACTION_OPEN || filechooser_action == GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER) {
        filechooser_v.combo_info = combo_info;
        filechooser_v.parent = dialog;
        filechooser_v.folder = folder;
        filechooser_v.title = title_txt;
	if (combobox_active) {
	    filechooser_v.entry = GTK_ENTRY(COMBOBOX_get_entry_widget (combo_info));
	} else {
	    filechooser_v.entry = GTK_ENTRY(gtk_bin_get_child (GTK_BIN(combo)));

	}
        filechooser_v.activate_func = activate_entry;
        filechooser_v.activate_user_data = &response;

        filechooser_v.filechooser_action = filechooser_action;
        preload (filechooser_v.folder);

        button = gtk_button_new ();
	GdkPixbuf *pixbuf=rfm_get_pixbuf("xffm/stock_directory", SIZE_BUTTON);
        GtkWidget *image;
	if (pixbuf) {
	    image = gtk_image_new_from_pixbuf (pixbuf);
	} else {
	    image = gtk_image_new_from_icon_name ("folder", GTK_ICON_SIZE_BUTTON);
	    //image = gtk_image_new_from_icon_name (GTK_STOCK_DIRECTORY, GTK_ICON_SIZE_BUTTON);
	}
	g_object_unref(pixbuf);
        gtk_button_set_image ((GtkButton *) button, image);
        vbox = rfm_vbox_new (FALSE, 6);
        gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);
        gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
        g_signal_connect (button, "clicked", G_CALLBACK (filechooser), (gpointer) (&filechooser_v));
        gtk_widget_show (button);
    }


    gtk_widget_show_all (hbox);
    if(path) {
        gchar *type = MIME_type ((void *)path, NULL);
        gchar *p = NULL;
	//   This may be borked if path is a non local file (i.e., obexfs)
	//   so we verify on the parent folder.
        if(!type){
	    if (widgets_p && widgets_p->view_p && widgets_p->view_p->en &&
		    IS_LOCAL_TYPE(widgets_p->view_p->en->type)){
		type = MIME_magic (path);
	    }
	} 
	if (!type) type = g_strdup(_("unknown"));
        p = MIME_command (type);
        g_free (type);
        if(p){
	    NOOP("COMBO: setting entry to %s\n", p);
	    if (combobox_active) {
		COMBOBOX_set_entry (combo_info, p);
	    } else {
		if (filechooser_v.entry) gtk_entry_set_text (filechooser_v.entry,p);
	    }
	    g_free(p);
	}

    }

    if (entry_text) {
	NOOP ("COMBO: setting combo to %s\n", entry_text);
	if (combobox_active) {
	    COMBOBOX_set_entry (combo_info, entry_text);
 	} else {
	    if (filechooser_v.entry) gtk_entry_set_text (filechooser_v.entry,entry_text);
	}
   }
    if (!entry_text && ! path) {
	COMBOBOX_set_default (combo_info);
	NOOP ("COMBO: setting combo to default (!entry_text && ! path)\n");
    }

    // Check button
    // This demands deprecated gtk_dialog_get_action_area since 
    // gtk_dialog_add_action_widget is only for widgets which emit 
    // response. Bleak workaround, use content area...
    //GtkWidget *action_area = gtk_dialog_get_action_area (GTK_DIALOG(dialog));
    GtkWidget *action_area = gtk_dialog_get_content_area (GTK_DIALOG(dialog));
    if(check_label && flagfile) {
        extra_key.check1 = (GtkWidget *) gtk_check_button_new_with_mnemonic (check_label);
        g_signal_connect (extra_key.check1, "toggled", G_CALLBACK (toggle_activate), (gpointer) & extra_key);
        extra_key.entry = filechooser_v.entry;
        gtk_box_pack_start (GTK_BOX (action_area), GTK_WIDGET (extra_key.check1), FALSE, FALSE, 0);
        gtk_widget_show (extra_key.check1);
    }
    add_cancel_ok(widgets_p, GTK_DIALOG (dialog));
    gtk_widget_realize (dialog);
    if(flagfile) {
        extra_key.flagfile = flagfile;
        extra_key_completionR (&extra_key);
    } else
        extra_key.flagfile = NULL;

    if(title_txt)
        gtk_window_set_title (GTK_WINDOW (dialog), title_txt);
    else
        gdk_window_set_decorations (gtk_widget_get_window(dialog), GDK_DECOR_BORDER);

    g_signal_connect (G_OBJECT (dialog), "delete-event", G_CALLBACK (response_delete), dialog);
    /* show dialog and return */
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
    gtk_widget_show_all (dialog);
    response  = gtk_dialog_run(GTK_DIALOG(dialog));
    //    gtk_main();
    gchar *response_txt = NULL;
    if(response == GTK_RESPONSE_YES) {
        const gchar *et = NULL;
        if (combobox_active){
	    et = COMBOBOX_get_entry (combo_info);
        } else if (filechooser_v.entry) {
	    et = gtk_entry_get_text (filechooser_v.entry);
        }
        if(et && strlen (et)) {
            response_txt = (gchar *) malloc ((strlen (et) + 3) * sizeof (gchar));
	    if (!response_txt) {
		DBG("malloc: %s\n", strerror(errno));
		return NULL;
	    }
            memset (response_txt, 0, (strlen (et) + 3) * sizeof (gchar));
            strcpy (response_txt, et);
	    if(response_txt) g_strstrip (response_txt);
            COMBOBOX_save_to_history (history_file, (char *)response_txt);
            if(flagfile) rodent_save_flags (&extra_key);
        
	    if(flagfile && extra_key.check1 && GTK_IS_TOGGLE_BUTTON(extra_key.check1)) {
		gboolean active = gtk_toggle_button_get_active ((GtkToggleButton *)
                                                            extra_key.check1);
		//NOOP("active=%d\n",active);
		if(active)
		    response_txt[strlen (response_txt) + 1] = 1;
	    }
        }
    }
    gtk_widget_hide (dialog);
    // cleanup combobox module objects:
    COMBOBOX_destroy_combo(combo_info);
    
    gtk_widget_destroy (dialog);
    if (chdir(g_get_home_dir()) < 0){
	DBG("cannot chdir(g_get_home_dir())\n");
    }


    return response_txt;
}

#endif


///////////////////////   static GTK callbacks... ////////////////////////////////////////


static gboolean
on_status_button_press ( GtkWidget *w , GdkEventButton * event, gpointer data) {
    lpterm_c *lpterm_p = (lpterm_c *)data;
    lpterm_p->lp_set_active(TRUE);
    return TRUE;
}

static gboolean
status_keyboard_event (GtkWidget * window, GdkEventKey * event, gpointer data)
{
    TRACE("status_keyboard_event\n");
    return FALSE;
}
