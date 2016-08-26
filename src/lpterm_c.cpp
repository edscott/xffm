//#define DEBUG_TRACE 1

#include "view_c.hpp"
#include "lpterm_c.hpp"

#ifdef HAVE_READLINE_HISTORY_H
# include <readline/history.h>
#endif
#define LP_TERMINAL_HISTORY 	USER_RFM_CACHE_DIR,"lp_terminal_history"

// FIXME: csh completion not working. Arrows just go up and down history.

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
    // FIXME: cursor is not visible!
    //gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW(data), TRUE);
    return FALSE;
/*    window_c *window_p = (window_c *)data;
    view_c *view_p = (view_c *)(window_p->get_active_view_p());
    return view_p->window_keyboard_event(event, (void *)view_p);*/
}

lpterm_c::lpterm_c(void *data): print_c(data){
    active = FALSE;
    sh_command = NULL;
    command_history_mutex = PTHREAD_MUTEX_INITIALIZER;
    load_sh_command_history();
    
    view_c *view_p = (view_c *)data;
   // print_p = view_p->get_print_p();
    iconview = view_p->get_iconview();
    status_icon = view_p->get_status_icon();
    iconview_icon = view_p->get_iconview_icon();
    status_button = view_p->get_status_button();

    g_signal_connect (status, "key-press-event", G_CALLBACK (status_keyboard_event), data);
    g_signal_connect (status_button, "button-press-event", G_CALLBACK (on_status_button_press), (void *)this);
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
	    TRACE("key ignored\n");
            return TRUE;
        }
    }


    gboolean active = lp_get_active();
    TRACE("window_keyboard_event(0x%x): get_active_lp = %d\n", event->keyval, active);

    if (!active && is_iconview_key(event)) {
	TRACE("Sending key to iconview default handler.\n");
	return FALSE;
    }

    if (active && (event->keyval == GDK_KEY_Escape)) {
	TRACE("set_active_lp = %d\n", FALSE);
        lp_set_active(FALSE); 
	return TRUE;
    }

    if (!active) {
	TRACE("set_active_lp = %d\n", TRUE);
        lp_set_active(TRUE); 
	if (event->keyval == GDK_KEY_Tab){
	    event->keyval = GDK_KEY_Escape;
	}
	if (event->keyval == GDK_KEY_Escape){
	    return TRUE;

        } 
    }
    // By now we have a lp key to process
    TRACE("send key to status dialog for lpterm command\n");
    lpterm_keyboard_event(event, data);
    return TRUE;



    /* FIXME: callbacks...
    if (rodent_do_callback(event->keyval, event->state)) {
        TRACE("window_keyboard_event(): Tried callback with keyval!\n");
        return TRUE;
    } */
    
    /*
    if (!active){
	if (iconview_key(event)) {
            TRACE("window_keyboard_event(): This may be a callback key!\n");
            return TRUE;
        }
    }

    if ((event->state & GDK_CONTROL_MASK) && !view_p->is_lpterm_key(event)) return TRUE;
*/
    /* FIXME
    if (view_p->selection_list) {
	// selection list must be redefined...
	update_reselect_list(widgets_p);
	TRACE( "Selection---> %p\n", view_p->selection_list);	
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
	gtk_widget_grab_focus (iconview);
        //gtk_widget_hide(view_p->get_status());
    }
    g_object_set_data (G_OBJECT (status), "active", GINT_TO_POINTER(state));
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
void 
lpterm_c::run_lp_command(void){
#if 0
        if(g_object_get_data (G_OBJECT (textview), "clean") == NULL) {
            g_object_set_data(G_OBJECT(widgets_p->status), "csh_cmd_len", NULL);
            g_object_set_data(G_OBJECT(widgets_p->status), "csh_nth", NULL);
            // get the command
            gchar *command = get_current_text ((GtkTextView *) widgets_p->status);
            if(command && strlen (command)) {
                // show the lp output area        
                rfm_show_text (widgets_p);

                // first process internal commands for cd and history
		// Internal commands will return focus to paper
		// (cd is a threaded reload which will overwrite
		//  the status line when done)
                if(process_internal_commands (widgets_p, &command)){
		    // probably not the best choice to send focus to paper...
                    // lp_set_active(widgets_p, FALSE);
		    // rfm_update_status_line (widgets_p->view_p);
                    return;
		}

                // command is now external for /bin/sh
                // printstatus with the run icon run.png
                // this is already done in run.c
                // rfm_diagnostics(widgets_p, "run.png",command,"\n",NULL);

                // run the command (in a shell)
		// XXX This will block if located at a remote directory with
		// a broken network connection.
                if(widgets_p->workdir) {
                    g_free (widgets_p->workdir);
                }
                view_t *view_p = widgets_p->view_p;
                if (!view_p->en || !view_p->en->path || ! rfm_g_file_test(view_p->en->path, G_FILE_TEST_IS_DIR)){
                    widgets_p->workdir = g_strdup (g_get_home_dir());
;
                } else {
                    widgets_p->workdir = g_strdup (view_p->en->path);
                }
		// Fix any sudo commands to use the -A option
		command = sudo_fix(command);
                RFM_THREAD_RUN (widgets_p, command, FALSE);
                g_free (command);
            }
        }
        rfm_status (widgets_p, "xffm/emblem_terminal", NULL);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(status), TRUE);
        g_object_set_data (G_OBJECT (textview), "clean", NULL);
        return;
    }
#endif
}

void
lpterm_c::bash_completion(){
    gchar *head=get_text_to_cursor();
    gint head_len = strlen(head);
    g_free (head);
    gchar *token = get_current_text ();

    if (strncmp(token, "sudo", strlen("sudo"))==0 &&
	strncmp(token, "sudo -A", strlen("sudo -A"))){
	gchar *o = token;
	gchar *oo = o+strlen("sudo");
	while (*oo == ' ') oo++;
	token = g_strconcat("sudo -A ", oo, NULL);
	g_free(o);
	head_len += strlen(" -A");
    }

    gint token_len = strlen(token);
    gchar *suggest = suggest_bash_complete(token, head_len);
    g_free (token);

    if (suggest) {
        gint suggest_len = strlen(suggest);
        GtkTextIter end;
        GtkTextBuffer *buffer = 
            gtk_text_view_get_buffer (GTK_TEXT_VIEW(status));
        gint offset = -1;
        // +2 is icon and space...
        offset = head_len + (suggest_len - token_len) + 2;
        print_status(suggest);
        // FIXME
        // rfm_status (widgets_p, "xffm/emblem_terminal", suggest, NULL);
        gtk_text_buffer_get_iter_at_offset (buffer, &end, offset);
        gtk_text_buffer_place_cursor(buffer, &end);	
    }
    g_free(suggest);

    g_object_set_data (G_OBJECT(status), "clean", NULL);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(status), TRUE);
    return ;

}

gint
lpterm_c::lpterm_keyboard_event( GdkEventKey * event, gpointer data) {
    TRACE("lpterm_keyboard_event...\n");
    if(!event) {
        g_warning ("on_status_key_press(): returning on event==0\n");
        return TRUE;
    }
    view_c *view_p =(view_c *)data; 
    GtkWidget *status = view_p->get_status();
    GtkWidget *diagnostics = view_p->get_diagnostics();
    gtk_c *gtk_p = view_p->get_gtk_p();


    // On activate, run the lpcommand
    if(event->keyval == GDK_KEY_Return || event->keyval == GDK_KEY_KP_Enter) {
        run_lp_command();
        return TRUE;
    }
    // tab for bash completion
    if(event->keyval == GDK_KEY_Tab) {
        bash_completion();
        return TRUE;
    }
    if((event->keyval == GDK_KEY_Page_Up) || (event->keyval == GDK_KEY_Page_Down)) {
        gboolean retval;
        g_signal_emit_by_name ((gpointer)diagnostics, "key-press-event", event, &retval);
        
        return TRUE;
    }
    if((event->keyval == GDK_KEY_Up) || (event->keyval == GDK_KEY_Down)) {
        // csh command completion
        gint completion = (event->keyval == GDK_KEY_Up)?1:0-1;
        gint offset = (event->keyval == GDK_KEY_Up)?0-1:1; 
        if (csh_completion(completion)) return TRUE;
	// push/pop history
	offset_history(offset);
	return TRUE;
    }
    if((event->keyval >= GDK_KEY_space && event->keyval <= GDK_KEY_asciitilde)
	    || (event->keyval == GDK_KEY_Right) 
	    || (event->keyval == GDK_KEY_Left)
	    || (event->keyval == GDK_KEY_End)
	    || (event->keyval == GDK_KEY_Home)  ) {
        //gchar *command = get_current_text (GTK_TEXT_VIEW(status));
	gchar *command = get_text_to_cursor();
        gint csh_cmd_len = strlen(command);
        g_object_set_data(G_OBJECT(status), "csh_cmd_len", GINT_TO_POINTER(csh_cmd_len));
	fprintf(stderr,"csh_cmd_len = %d\n", csh_cmd_len);
    }


    // right-left-home-end for csh completion (home end right left will return false 

    gboolean retval;
    g_signal_emit_by_name ((gpointer)status, "key-press-event", event, &retval);

    return TRUE;


 


#if 0
    // FIXME
    if(event->keyval == GDK_KEY_Return || event->keyval == GDK_KEY_KP_Enter) {
        if(g_object_get_data (G_OBJECT (textview), "clean") == NULL) {
            g_object_set_data(G_OBJECT(widgets_p->status), "csh_cmd_len", NULL);
            g_object_set_data(G_OBJECT(widgets_p->status), "csh_nth", NULL);
            // get the command
            gchar *command = get_current_text ((GtkTextView *) widgets_p->status);
            if(command && strlen (command)) {
                // show the lp output area        
                rfm_show_text (widgets_p);

                // first process internal commands for cd and history
		// Internal commands will return focus to paper
		// (cd is a threaded reload which will overwrite
		//  the status line when done)
                if(process_internal_commands (widgets_p, &command)){
		    // probably not the best choice to send focus to paper...
                    // lp_set_active(widgets_p, FALSE);
		    // rfm_update_status_line (widgets_p->view_p);
                    return TRUE;
		}

                // command is now external for /bin/sh
                // printstatus with the run icon run.png
                // this is already done in run.c
                // rfm_diagnostics(widgets_p, "run.png",command,"\n",NULL);

                // run the command (in a shell)
		// XXX This will block if located at a remote directory with
		// a broken network connection.
                if(widgets_p->workdir) {
                    g_free (widgets_p->workdir);
                }
                view_t *view_p = widgets_p->view_p;
                if (!view_p->en || !view_p->en->path || ! rfm_g_file_test(view_p->en->path, G_FILE_TEST_IS_DIR)){
                    widgets_p->workdir = g_strdup (g_get_home_dir());
;
                } else {
                    widgets_p->workdir = g_strdup (view_p->en->path);
                }
		// Fix any sudo commands to use the -A option
		command = sudo_fix(command);
                RFM_THREAD_RUN (widgets_p, command, FALSE);
                g_free (command);
            }
        }
        rfm_status (widgets_p, "xffm/emblem_terminal", NULL);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(status), TRUE);
        g_object_set_data (G_OBJECT (textview), "clean", NULL);
        return TRUE;
    }
#endif
    return TRUE;
}

void 
lpterm_c::place_cursor(void){
    gint position =
	GPOINTER_TO_INT(g_object_get_data(G_OBJECT(status), "csh_cmd_len"));
    GtkTextIter iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(status));
    gtk_text_buffer_get_iter_at_offset (buffer, &iter, position+1);
    gtk_text_buffer_place_cursor (buffer, &iter);
}


gchar *
lpterm_c::get_current_text (void) {
    // get current text
    GtkTextIter start, end;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer ((GtkTextView *) status);

    gtk_text_buffer_get_bounds (buffer, &start, &end);
    gchar *t = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);
    g_strchug(t);
    return t;
}

gchar *
lpterm_c::get_text_to_cursor (void) {
    // get current text
    GtkTextIter start, end;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(status));
    gint cursor_position;
    g_object_get (G_OBJECT (buffer), "cursor-position", &cursor_position, NULL);
    
    gtk_text_buffer_get_iter_at_offset (buffer, &start, 0);
    gtk_text_buffer_get_iter_at_offset (buffer, &end, cursor_position);
    gchar *t = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);
    g_strchug(t);
    NOOP ("TO cursor position=%d %s\n", cursor_position, t);
    return t;
}

gboolean
lpterm_c::csh_completion(gint direction){
    gchar *command = get_current_text ();
    if (!command || !strlen(command)) {
	fprintf(stderr, "return on !command\n");
        g_object_set_data(G_OBJECT(status), "csh_cmd_len", NULL);
        g_object_set_data(G_OBJECT(status), "csh_nth", NULL);
	g_free(command);
        return FALSE;
    }
    g_free(command);
    command = get_text_to_cursor ();
    gint csh_cmd_len = 
        GPOINTER_TO_INT(g_object_get_data(G_OBJECT(status), "csh_cmd_len"));
    if (!csh_cmd_len) {
	fprintf(stderr, "return on !csh_cmd_len\n");
        g_object_set_data(G_OBJECT(status), "csh_nth", NULL);
	g_free(command);
        return FALSE;
    }
    gint nth = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(status), "csh_nth"));

    if (!nth) nth = g_list_length(sh_command);
    gchar *p=NULL;
    if (direction > 0){
        if (nth > 1){ 
            nth--;
	    GList *list = g_list_nth(sh_command, nth);
	    for (;list && list->data;list=list->prev, nth--){
		p = (gchar *)list->data;
		if (p && strncmp(command, p, csh_cmd_len)==0) break;
		p=NULL;
	    } 
	}
    } else {
	nth++;
	if (nth <= g_list_length(sh_command)){
            GList *list = g_list_nth(sh_command, nth);
            for (;list && list->data;list=list->next, nth++){
                p = (gchar *)list->data;
                if (p && strncmp(command, p, csh_cmd_len)==0) break;
                p=NULL;
            } 
        }
    }
    if (p){
            TRACE("gotcha (%d): %s\n", nth, p);
            g_object_set_data(G_OBJECT(status), "csh_nth", GINT_TO_POINTER(nth));
	    place_command(p);
            place_cursor();
    }
    g_free(command);
    return TRUE;    
}
void 
lpterm_c::place_command(const gchar *p){
    lp_set_active(TRUE);
    print_status ("%s", p);
    g_object_set_data (G_OBJECT (status), "clean", NULL);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW  (status), TRUE);
    gtk_widget_grab_focus (GTK_WIDGET(status));
}

gpointer
lpterm_c::load_sh_command_history (void) {
    gchar *history = g_build_filename (LP_TERMINAL_HISTORY, NULL);
    pthread_mutex_lock(&command_history_mutex);
    g_list_free (sh_command);
    GList *p;
    for(p = sh_command; p; p = p->next) {
        g_free (p->data);
    }
    sh_command = NULL;
    sh_command = g_list_append (sh_command, g_strdup (""));
    sh_command_counter = 0;

    FILE *sh_history = fopen (history, "r");
    if(sh_history) {
        gchar line[2048];
        memset (line, 0, 2048);
        while(fgets (line, 2047, sh_history) && !feof (sh_history)) {
            NOOP ("HISTORY: line %s\n", line);
            if(strchr (line, '\n')) *strchr (line, '\n') = 0;
            if(strlen (line) == 0) continue;
            // skip invalid commands (except cd):
            if(!is_valid_command (line)) {
                if(strcmp (line, "cd") != 0 && strncmp (line, "cd ", strlen ("cd ")) != 0) {
                    NOOP ("HISTORY: invalid history command in %s: %s\n", history, line);
                    continue;
                }
            }
	    gchar *newline=compact_line(line);
	    GList *element=find_in_string_list(sh_command, newline);

	    if (element) { 
		// remove old element
		gchar *data=(gchar *)element->data;
		sh_command = g_list_remove(sh_command, data);
		g_free(data);
	    }
	    // put at top of the pile
	    sh_command =
		    g_list_insert_before (sh_command, g_list_last (sh_command), newline);
        }

        NOOP ("rfm_load_sh_command_history(): readunlock for %s\n", history);
        fclose (sh_history);
        sh_command_counter = g_list_length (sh_command) - 1;
    }
    g_free (history);
	
    pthread_mutex_unlock(&command_history_mutex);
    return NULL;
}

void
lpterm_c::save_sh_command_history (const gchar * command) {
    GList *p;
	    
    pthread_mutex_lock(&command_history_mutex);
    p = g_list_previous (g_list_last (sh_command));
    if(!command || !strlen (command)){
	pthread_mutex_unlock(&command_history_mutex);
        return;
    }
    gchar *command_p = g_strdup (command);
    g_strstrip (command_p);
    // if repeat of last command, skip it.
    if(!p || strcmp (command, (gchar *)p->data)) {
        sh_command = g_list_insert_before (sh_command, g_list_last (sh_command), command_p);
        // don't save to file if invalid command 
        if(!is_valid_command (command_p)) {
            if(strcmp (command_p, "cd") != 0 && strncmp (command_p, "cd ", strlen ("cd ")) != 0) {
                DBG ("not saving %s\n", command_p);
                sh_command_counter = g_list_length (sh_command) - 1;
		pthread_mutex_unlock(&command_history_mutex);
                return;
            }
        }
        // here we will rewrite the history, removing any item
        // which duplicate the command about to be saved.
        // This effectively moves the command to the bottom
        // of the list.
        // 

        gchar *history = g_build_filename (LP_TERMINAL_HISTORY, NULL);

        // read it first to synchronize with other rodent instances
        GList *disk_history = NULL;
        
        FILE *sh_history = fopen (history, "r");

        if(sh_history) {
 
            char line[2048];
            memset (line, 0, 2048);
            while(fgets (line, 2047, sh_history) && !feof (sh_history)) {
                if(strchr (line, '\n')) {
                    *strchr (line, '\n') = 0;
                }
                if(strcmp (line, command_p) != 0) {
                    disk_history = g_list_append (disk_history, g_strdup (line));
                }
            }

            fclose (sh_history);
        }
        disk_history = g_list_append (disk_history, g_strdup (command_p));

        sh_history = fopen (history, "w");
        if(sh_history) {
            GList *p;
            for(p = g_list_first (disk_history); p && p->data; p = p->next) {
                fprintf (sh_history, "%s\n", (gchar *)p->data);
                g_free (p->data);
            }
            fclose (sh_history);
        }
        g_list_free (disk_history);
        g_free (history);
    }
    sh_command_counter = g_list_length (sh_command) - 1;
    NOOP ("rfm_save_sh_command_history(); command_counter=%d\n", sh_command_counter);
	
    pthread_mutex_unlock(&command_history_mutex);
    return;
}

gboolean
lpterm_c::is_valid_command (const gchar *cmd_fmt) {
    //return GINT_TO_POINTER(TRUE);
    NOOP ("is_valid_command(%s)\n", cmd_fmt);
    GError *error = NULL;
    gint argc;
    gchar **argv;
    if(!cmd_fmt) return FALSE;
    if(!g_shell_parse_argv (cmd_fmt, &argc, &argv, &error)) {
        gchar *msg = g_strcompress (error->message);
        DBG ("%s: %s\n", msg, cmd_fmt);
        g_error_free (error);
        g_free (msg);
        return (FALSE);
    }
    gchar **app = argv;
    if (*app==NULL) {
        errno = ENOENT;
        return (FALSE);
    }

    // assume command is correct if environment is being set
    if (strchr(*app, '=')){
        g_strfreev (argv);
        return (TRUE);
    }

    gchar *path = g_find_program_in_path (*app);
    if(!path) {
        gboolean direct_path = g_file_test (argv[0], G_FILE_TEST_EXISTS) ||
            strncmp (argv[0], "./", strlen ("./")) == 0 || strncmp (argv[0], "../", strlen ("../")) == 0;
        //DBG("argv[0]=%s\n",argv[0]);
        if(direct_path) {
            path = g_strdup (argv[0]);
        }
    }
    NOOP ("mime_is_valid_command(): g_find_program_in_path(%s)=%s\n", argv[0], path);

    if(!path) {
        g_strfreev (argv);
        errno = ENOENT;
        return (FALSE);
    }
    // here we test for execution within sudo
    // XXX we could also check for commands executed in a terminal, but not today...
    gboolean retval = TRUE;
    if (strcmp(argv[0],"sudo")==0) {
        gint i=1;
        if (strcmp(argv[i],"-A")==0) i++;
        retval=is_valid_command(argv[i]);
    }

    g_strfreev (argv);
    g_free (path);
    return retval;
}

gboolean
lpterm_c::offset_history(gint offset){
    void *p = g_list_nth_data (sh_command, sh_command_counter + offset);
    NOOP ("get nth sh_command_counter=%d\n", sh_command_counter + offset);
    if(p) {
            sh_command_counter += offset;
	    place_command((const gchar *)p);
    }
    return TRUE;
}



#if 0
void
print_suggestion (
    widgets_t * widgets_p,
    const char *data,
    int i,
    gboolean cr
) {
    gchar *element = g_strdup_printf ("%d", i);
    rfm_diagnostics (widgets_p, NULL, "[", NULL);
    rfm_diagnostics (widgets_p, "xffm_tag/green", element, NULL);
    rfm_diagnostics (widgets_p, NULL, "] ", NULL);
    rfm_diagnostics (widgets_p, "xffm_tag/blue", data, NULL);
    if(cr)
        rfm_diagnostics (widgets_p, NULL, "\n", NULL);

    g_free (element);
}

gboolean
internal_cd ( widgets_t * widgets_p, gchar ** argvp) {   
    view_t *view_p = widgets_p->view_p;
    gchar *gg=NULL;

    if(argvp[1]) {
	if (*argvp[1] == '~'){
	    if (strcmp(argvp[1], "~")==0 || 
		    strncmp(argvp[1], "~/", strlen("~/"))==0){
		gg = g_strdup_printf("%s%s", g_get_home_dir (), argvp[1]+1);
	    } else {
		//gchar *tilde_dir = rfm_get_tilde_dir(argvp[1]);
		gchar *tilde_dir = 
		    rfm_natural(RFM_MODULE_DIR, "completion", 
			    argvp[1], "rfm_get_tilde_dir");
		if (tilde_dir) gg = g_strconcat(tilde_dir, 
			strchr(argvp[1], '/')+1, NULL);
		else gg = g_strdup(argvp[1]);
		g_free(tilde_dir);	
	    }
	} else {
	    gg = g_strdup(argvp[1]);
	}

    } else {
        gg = g_strdup(g_get_home_dir ());
    }
    rfm_show_text (widgets_p);
    NOOP ("CD: gg=%s argv[1]=%s\n", gg, argvp[1]);

    // must allow relative paths too.
    if (!g_path_is_absolute(gg)){
	if(!view_p->en || ! view_p->en->path ||  
	    !rfm_g_file_test_with_wait (view_p->en->path, G_FILE_TEST_IS_DIR)) 
	{
	    rfm_diagnostics (widgets_p, "xffm/stock_dialog-error", NULL);
	    rfm_diagnostics (widgets_p, "xffm_tag/stderr", "* ", gg, ": ", strerror (ENOENT), "\n", NULL);
	    g_free (gg);
	    return TRUE;
        } 
	gchar *fullpath = g_strconcat(view_p->en->path, "/", gg, NULL);
	g_free(gg);
	gg = fullpath;
	NOOP("CD: fullpath=%s\n", fullpath);
    }

    if (gg[strlen(gg)-1]=='/' || strstr(gg, "/..")){
	gchar *rpath = realpath(gg, NULL);
	g_free(gg);
	gg=rpath;
    }
    rfm_diagnostics (widgets_p, "xffm_tag/command", "cd", " ", NULL);
    rfm_diagnostics (widgets_p, "xffm_tag/green", gg, "\n", NULL);

    gchar *rpath = realpath(gg, NULL);
    if (!rpath){
	rfm_diagnostics (widgets_p, "xffm/stock_dialog-error", NULL);
	rfm_diagnostics (widgets_p, "xffm_tag/stderr", gg, ": ", strerror (ENOENT), "\n", NULL);
	g_free (gg);
	return TRUE;
    }
    g_free (rpath);

    gtk_widget_grab_focus (widgets_p->paper);

    rodent_push_view_go_history ();
    record_entry_t *new_en = rfm_stat_entry (gg, 0);
    view_p->module = NULL;
    if (!rodent_refresh (widgets_p, new_en)){
	rfm_destroy_entry(new_en); 
    } else {
	rfm_save_to_go_history ((gchar *) gg);
	gchar *command = g_strdup_printf ("cd %s", gg);
	rfm_save_sh_command_history (view_p, command);
    }
    g_free (gg);
    return TRUE;
}

void
print_history ( widgets_t * widgets_p) {
    int i;
    view_t *view_p = widgets_p->view_p;
    GList *p;
    rfm_diagnostics (widgets_p, "xffm_tag/command", "history:", "\n", NULL);

    for(i = 1, p = g_list_first (view_p->sh_command); p && p->data; p = p->next, i++) {
        print_suggestion (widgets_p, (char *) p->data, i, TRUE);
    }

}

void
print_tab ( widgets_t * widgets_p, gchar * text, gchar *text2) {
    int tab_len = 18;
    rfm_diagnostics (widgets_p, "xffm_tag/red", text, text2, NULL);
    gint string_length = (text)?strlen(text):0 + (text2)?strlen(text2):0;
    for(tab_len = tab_len - string_length; tab_len > 0; tab_len--)
        rfm_diagnostics (widgets_p, NULL, " ", NULL);
}

void
print_history_help ( widgets_t * widgets_p) {
    rfm_diagnostics (widgets_p, "xffm_tag/command", 
	    _("History"), " (", _("Get help..."), "):\n", NULL);
    print_tab (widgets_p, "?",NULL);
    rfm_diagnostics (widgets_p, "xffm_tag/green",
	    _("Show help about options"), "\n", NULL);

    print_tab (widgets_p, "history",NULL);
    rfm_diagnostics (widgets_p, "xffm_tag/green",
	    _("Show History"), "\n", NULL);

    print_tab (widgets_p, "!","n");
    rfm_diagnostics (widgets_p, "xffm_tag/green",
	    _("Line Number"),  " (", _("Command Line"), ") ", NULL);
    rfm_diagnostics (widgets_p, "xffm_tag/red", 
	    "n", NULL);
    rfm_diagnostics (widgets_p, "xffm_tag/green",
	    ".", "\n", NULL);

    print_tab (widgets_p, "!",_("STRING"));
    rfm_diagnostics (widgets_p, "xffm_tag/green",
	    _("Complete Match"), NULL);
    rfm_diagnostics (widgets_p, "xffm_tag/red", 
	    " ", _("STRING"), NULL);
    rfm_diagnostics (widgets_p, "xffm_tag/green", ".", "\n", NULL);

    print_tab (widgets_p, "!?",_("STRING"));
    rfm_diagnostics (widgets_p, "xffm_tag/green", 
	    _("Anywhere"),  NULL);
    rfm_diagnostics (widgets_p, "xffm_tag/red", " ",
	    _("STRING"), NULL);
    rfm_diagnostics (widgets_p, "xffm_tag/green", ".", "\n", NULL);

    print_tab (widgets_p, _("STRING"),"<CTRL+TAB>");
    rfm_diagnostics (widgets_p, "xffm_tag/green", 
	    _("Completion mode:")," ", _("Command Line"), NULL);
    //rfm_diagnostics (widgets_p, "xffm_tag/red",  _("STRING"), NULL);
    rfm_diagnostics (widgets_p, "xffm_tag/green", ".", "\n", NULL);

    print_tab (widgets_p, _("STRING"),"<TAB>");
    rfm_diagnostics (widgets_p, "xffm_tag/green", 
	    _("Completion mode:")," ", "bash", NULL);
    //rfm_diagnostics (widgets_p, "xffm_tag/red",  _("STRING"), NULL);
    rfm_diagnostics (widgets_p, "xffm_tag/green", ".", "\n", NULL);
    rfm_diagnostics (widgets_p, "xffm/stock_dialog-info",NULL);
    rfm_diagnostics (widgets_p, "xffm_tag/blue", _("Full readline library history commands available"), "\n", NULL);

#if 0
    I have not used these nonconventional options, ever...
    print_tab (widgets_p, "!!", NULL);
    rfm_diagnostics (widgets_p, "xffm_tag/green", 
	    _("Clear History"), " (", _("Current"), ")", "\n", NULL);

    print_tab (widgets_p, "!!!", NULL);
    rfm_diagnostics (widgets_p, "xffm_tag/green", 
	    _("Clear History"), " (", _("Disk"), ")", "\n", NULL);
#endif
}

void
suggest_command (const char *complete, gboolean anywhere) {
    view_t *view_p = widgets_p->view_p;
    GList *p;
    char *suggest = NULL;
    for(p = g_list_last (view_p->sh_command); p && p->data; p = p->prev) {
        char *data = (char *) p->data;
        if((anywhere && strstr (data, complete)) || (!anywhere && strncmp (complete, data, strlen (complete)) == 0)) {
            suggest = g_strdup (data);
            break;
        }
        NOOP ("COMPLETE: ?? %s\n", data);
    }
    if(suggest) {
        rfm_status (widgets_p, "xffm/emblem_terminal", suggest, NULL);
        g_object_set_data (G_OBJECT (widgets_p->status), "clean", NULL);
        g_free (suggest);
    }
}

#ifndef HAVE_READLINE_HISTORY_H

gboolean
internal_history (gchar *cmd) {
    view_t *view_p = widgets_p->view_p;
    // internal fall back for when compiled without history library.
    
    // 
    const char *b = cmd + 1;
    //   errno = 0;
    long n = strtol (b, NULL, 10);
    NOOP ("COMPLETE: n=%ld\n", n);
/*    if (errno)  {
	      rfm_diagnostics (widgets_p, "xffm/stock_dialog-warning", b, ": ",
				 strerror (errno), NULL);
	      return TRUE;
    }*/
    if(n > 1 && n <= g_list_length (view_p->sh_command)) {
        GList *p = g_list_nth (view_p->sh_command, n - 1);
        if(p && p->data) {
            rfm_status (widgets_p, "xffm/emblem_terminal", (char *) p->data, NULL);
            g_object_set_data (G_OBJECT (widgets_p->status), "clean", NULL);
        }
    } else {
        if(*b != '?') {
            suggest_command (widgets_p, b, FALSE);
        } else {
            suggest_command (widgets_p, b + 1, TRUE);
        }
    }
    return TRUE;

}
#endif

#ifdef HAVE_READLINE_HISTORY_H
gchar *readline_history(gchar *cmd){
    TRACE("readline_history: \"%s\"\n", cmd);
    char *expansion;
    static gchar *history = NULL;
    if (!history) history = g_build_filename (LP_TERMINAL_HISTORY, NULL);
    
    read_history(history);
    using_history();


    int result = history_expand (cmd, &expansion);
    TRACE ("result=%d expansion=%s\n", result, expansion);
    if (result < 0){
        rfm_diagnostics(widgets_p, "xffm/stock_dialog-warning", expansion, "\n", NULL);
    }
    if (result > 0){
        place_command(widgets_p->status, widgets_p, expansion);
        place_cursor();
    }
    if (!expansion || !strlen(expansion) || result <= 0){
       g_free(expansion);
       expansion = NULL;
    }
    clear_history();
    return expansion;
}
#endif

gboolean
process_internal_commands (gchar ** command) {
    TRACE("process_internal: %s\n", *command);

    if (*command == NULL) return TRUE;
    gchar *cmd = *command;
#ifdef HAVE_READLINE_HISTORY_H
    // readline history?
    if ((*cmd == '!' && cmd[1] != ' ' && cmd[1] != '=') ||
            (*cmd =='^' && strchr(cmd+1, '^'))){
        gchar *expansion;
        if ((expansion = readline_history(widgets_p, *command)) != NULL){
            TRACE("readline expansion=%s\n", expansion);
            g_free(expansion);
            return TRUE;
        }
    }
#else
    if (*cmd == '!' && cmd[1] != ' ' && cmd[1] != '='){
            internal_history(widgets_p, cmd);
            return TRUE;
    }


#endif
    gchar *command2=NULL;
    if (strchr(*command, ';')){
	// split command in two.
	command2 = g_strdup(strchr(*command, ';') + 1);
	*strchr(*command, ';')=0;
    }
    gint argcp;
    gchar **argvp;
    GError *error = NULL;
    if(!g_shell_parse_argv (*command, &argcp, &argvp, &error)) {
        rfm_diagnostics (widgets_p, "xffm/stock_dialog-error", error->message, "\n", NULL);
        g_error_free (error);
	if (command2){
	    g_free(*command);
	    *command=command2;
	    return FALSE;
	}
        return TRUE;
    } else {
        // shortcircuit chdir and history commands 
        gboolean is_internal = FALSE;
        if(strcmp (argvp[0], "cd") == 0) {
            internal_cd (widgets_p, argvp);
            NOOP ("CD: command=%s argv[1]=%s\n", *command, argvp[1]);
            is_internal = TRUE;
        } else if(strncmp (argvp[0], "history", strlen ("history")) == 0) {
            rfm_show_text(widgets_p);
            print_history (widgets_p);
            rfm_status (widgets_p, "xffm/emblem_terminal", "", NULL);
            g_object_set_data (G_OBJECT (widgets_p->status), "clean", NULL);
            is_internal = TRUE;
        } else if(strncmp (argvp[0], "?", strlen ("?")) == 0) {
            print_history_help (widgets_p);
            rfm_status (widgets_p, "xffm/emblem_terminal", "", NULL);
            g_object_set_data (G_OBJECT (widgets_p->status), "clean", NULL);
            is_internal = TRUE;
        }

	if (command2){
	    g_free(*command);
	    *command=command2;
	    return FALSE;
	}
        g_strfreev (argvp);
        return is_internal;
    }
}

 
gchar *
sudo_fix(gchar *command){
    if (! command || !strstr(command, "sudo ")) return command; 
    if (strncmp(strstr(command, "sudo "), "sudo -A ", strlen("sudo -A "))!=0)
    {
        gchar *original_head=g_strdup(command);
        gchar *pos = strstr(original_head, "sudo ");
        if (pos){
	    *pos = 0;
	    gchar *tail=g_strdup(strstr(command, "sudo ")+strlen("sudo "));
	    tail=sudo_fix(tail);
	    gchar *new_command = g_strconcat(original_head, "sudo -A ", tail, NULL);
	    g_free(tail);
	    g_free(command);
	    command = new_command;
        }
	g_free(original_head);
    }
    return command;
}


gboolean
lp_get_active(widgets_t *widgets_p){
    if(g_object_get_data (G_OBJECT (widgets_p->status), "active")) return TRUE;
    return FALSE;
}


// Shared keybindings with either iconview or callback
typedef struct lpkey_t{
    guint key;
    guint mask;
} lpkey_t;

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
static const gchar *terminals_v[] = {
	"roxterm", 
	"sakura",
	"gnome-terminal", 
	"Eterm", 
	"konsole", 
	"Terminal", 
	"aterm", 
	"xterm", 
	"kterm", 
	"wterm", 
	"multi-aterm", 
	"evilvte",
	"mlterm",
	"xvt",
	"rxvt",
	"urxvt",
	"mrxvt",
	"tilda",
	NULL
};

static const gchar *editors_v[] = {
	"gvim -f",  
	"mousepad", 
	"gedit", 
	"kate", 
	"xemacs", 
	"nano",
	"vi",
	NULL
};

const gchar **rfm_get_terminals(void) {return terminals_v;}
const gchar **rfm_get_editors(void) {return editors_v;}

static GSList *children_list=NULL;

static pthread_mutex_t children_list_mutex = PTHREAD_MUTEX_INITIALIZER;

void rfm_remove_child(pid_t child){
    if (!children_list) return;
    pthread_mutex_lock(&(children_list_mutex));
    children_list = g_slist_remove(children_list, GINT_TO_POINTER(child));
    pthread_mutex_unlock(&(children_list_mutex));
    return;
}

void rfm_killall_children(void){
    TRACE("rfm_killall_children(): signalling controller children...\n");
    pthread_mutex_lock(&(children_list_mutex));

    GSList *list = children_list;
    for (;list && list->data; list = list->next){
	pid_t child = GPOINTER_TO_INT(list->data);
	TRACE( "ZZZZZZZ--- killing %d\n", child);
	kill(child, SIGTERM);

    }
    g_slist_free(children_list);
    children_list = NULL;
    pthread_mutex_unlock(&(children_list_mutex));

}

void rfm_add_child(pid_t child){ 
    pthread_mutex_lock(&(children_list_mutex));
    NOOP(stderr, "adding %d to children_list\n", child);
    children_list = g_slist_prepend(children_list, GINT_TO_POINTER(child));
    pthread_mutex_unlock(&(children_list_mutex));

    return;
}

gchar * 
rfm_get_text_editor_envar(const gchar *value){
    if(!value) return NULL;
    
    gchar *editor=g_path_get_basename(value);
    // if nano or vi, then use terminal emulator
    if (editor && 
	    (strncmp(editor, "vi",strlen("vi"))==0 
	     || 
	     strncmp(editor, "nano",strlen("nano"))==0)){
	const gchar *t=getenv("TERMINAL_CMD");
	gchar *term = g_find_program_in_path(t);
	if (term) g_free(term);
	else {
	    t=NULL;
	    gint i;
	    for (i=0; terminals_v[i]; i++){
		// sakura is broken... 
		if (strstr(terminals_v[i], "sakura")) continue;
		term = g_find_program_in_path(terminals_v[i]);
		if (term){
		    t=terminals_v[i];
		    g_free(term);
		    break;
		}
	    }
	}
	if (t && strlen(t)) {
	    gchar *b=g_strdup_printf("%s %s %s",
		    t, rfm_term_exec_option(t), editor);
	    g_free(editor);
	    editor = b;
	}
    } else {
	g_free(editor);
	editor = g_strdup(value);
    }
    return (editor);
}

//////////////// Module wraparounds ////////////////////////////
//
// Use rfm_thread_run_argv or rfm_thread_run?
// rfm_thread_run_argv will execute directly, not via a shell, and will
// not be saved in the lpterm history

// rfm_thread_run_argv:
// This modality will execute the command without shell


// rfm_thread_run:
// This modality will execute the command via "sh -c" and allow pipes and 
// redirection. and will be saved in the lpterm history file
    
pid_t
rfm_thread_run_argv (
	widgets_t * widgets_p, 
	gchar ** argv, 
	gboolean interm
	){
    const void *vector[]={widgets_p, argv, GINT_TO_POINTER(interm), NULL,  NULL, NULL, NULL, NULL};
    return GPOINTER_TO_INT(rfm_vector_run(RFM_MODULE_DIR, "run", GINT_TO_POINTER(7),
		    vector, "m_thread_run_argv"));
}

pid_t
rfm_thread_run_argv_full (
	widgets_t * widgets_p, 
	gchar ** argv, 
	gboolean interm,
	gint *stdin_fd,
	void (*stdout_f) (void *stdout_data,
                      void *stream,
                      int childFD),
	void (*stderr_f) (void *stdout_data,
                      void *stream,
                      int childFD),
	void (*tubo_done_f) (void *data)
	){
    if (!argv || !argv[0]) return 0;
    const void *vector[]={widgets_p, argv, GINT_TO_POINTER(interm), stdin_fd,  stdout_f, stderr_f, tubo_done_f};
    return GPOINTER_TO_INT(rfm_vector_run(RFM_MODULE_DIR, "run", GINT_TO_POINTER(7),
		    vector, "m_thread_run_argv"));

 }

pid_t
rfm_thread_run_argv_with_stdin (
	widgets_t * widgets_p, 
	gchar ** argv, 
	gboolean interm, 
	gint *stdin_fd
    	){
    const void *vector[]={widgets_p, argv, GINT_TO_POINTER(interm), stdin_fd,  NULL, NULL, NULL};
    return GPOINTER_TO_INT(rfm_vector_run(RFM_MODULE_DIR, "run", GINT_TO_POINTER(7),
		    vector, "m_thread_run_argv"));

 
}

pid_t
rfm_thread_run_argv_with_stdout (
	widgets_t * widgets_p, 
	gchar ** argv, 
	gboolean interm, 
	void (*stdout_f) (void *stdout_data,
                      void *stream,
                      int childFD)
	){
    const void *vector[]={widgets_p, argv, GINT_TO_POINTER(interm), NULL,  stdout_f, NULL, NULL};
    return GPOINTER_TO_INT(rfm_vector_run(RFM_MODULE_DIR, "run", GINT_TO_POINTER(7),
		    vector, "m_thread_run_argv"));

    
}

pid_t
rfm_thread_run_argv_with_stderr (
	widgets_t * widgets_p, 
	gchar ** argv, 
	gboolean interm, 
	void (*stderr_f) (void *stderr_data,
                      void *stream,
                      int childFD)
	){
    const void *vector[]={widgets_p, argv, GINT_TO_POINTER(interm), NULL,  NULL, stderr_f, NULL};
    return GPOINTER_TO_INT(rfm_vector_run(RFM_MODULE_DIR, "run", GINT_TO_POINTER(7),
		    vector, "m_thread_run_argv"));

}
////////////////////////////////////////////////////////////////////////////

void
rfm_recover_flags (gchar * in_cmd, gboolean * interm, gboolean * hold) {
    DBHashTable *runflags;
    GString *gs;
    int *flags;
    gchar *g = g_build_filename ( RUN_FLAG_FILE, NULL);
    TRACE("opening %s...\n",g); 
    if((runflags = dbh_new (g, NULL, DBH_READ_ONLY|DBH_PARALLEL_SAFE)) == NULL) {
        TRACE ("Cannot open %s\n", g);
        *interm = 0;
        *hold = 0;
        return;
    }
    TRACE("opened %s.\n",g); 
    dbh_set_parallel_lock_timeout(runflags, 3);
    gs = g_string_new (in_cmd);
    sprintf ((char *)DBH_KEY (runflags), "%10u", g_string_hash (gs));
    g_string_free (gs, TRUE);
    flags = (int *)runflags->data;
    dbh_load (runflags);
    *interm = flags[0];
    *hold = flags[1];
    dbh_close (runflags);

    NOOP ("flags recovered from dbh file for %s, interm=%d hold=%d\n", in_cmd, *interm, *hold);
}


 
const gchar * 
rfm_term_exec_option(const gchar *terminal) {
    const gchar *exec_option = "-e";
    gchar *t = g_path_get_basename (terminal);
    if(strcmp (t, "gnome-terminal") == 0 || strcmp (t, "Terminal") == 0)
            exec_option = "-x";
    g_free(t);
    return exec_option;
}

const gchar *
rfm_what_term (void) {
    const gchar *term=getenv ("TERMINAL_CMD");
    gchar *t=NULL;
    if(term && strlen (term)) {
	if (strchr(term, ' ')){
	    gchar **g = g_strsplit(term, " ", -1);
	    t = g_find_program_in_path (g[0]);
	    g_strfreev(g);
	} else {
	    t = g_find_program_in_path (term);
	}
    }
    if(!t) {
	    const gchar **p=terminals_v;
	    for (;p && *p; p++){
		t = g_find_program_in_path (*p);
		if (t) {
		    term=*p;
		    break;  
		}  
	    }
    }
    if (t) {
	g_free(t);
	return term;
    }
    DBG ("TERMINAL_CMD=%s: %s\n", getenv ("TERMINAL_CMD"), strerror (ENOENT));

    return NULL;
}

#endif

#if 0
gint
on_button_press (
    GtkWidget * textview,
     GdkEventButton * event,
    gpointer data
) {
    widgets_t *widgets_p = (widgets_t *) data;
    //view_t *view_p = widgets_p->view_p;
    NOOP(stderr,"on_button_press...%lf\n", event->x);
    if (lp_get_active(widgets_p)){
	if (event->x < TINY_ICON_SIZE)event->x = TINY_ICON_SIZE;
    }
    return FALSE;
}
static gint
on_button_release (
    GtkWidget * textview,
     GdkEventButton * event,
    gpointer data
) {
    widgets_t *widgets_p = (widgets_t *) data;
    view_t *view_p = widgets_p->view_p;
    NOOP("on_button_release...%lf\n", event->x);
    if (lp_get_active(widgets_p)){
	if (event->x < TINY_ICON_SIZE)event->x = TINY_ICON_SIZE;
	return FALSE;
    }


    if(!rfm_population_try_read_lock (view_p, "on_button_release")) return FALSE;
    rodent_unselect_all_pixbuf (view_p);
    rodent_unsaturate_icon (view_p);
    status_grab_focus (widgets_p->view_p, 0);
    if (!view_p->lp_command) view_p->lp_command = g_strdup("");
    rfm_status (widgets_p, "xffm/emblem_terminal", view_p->lp_command, NULL);
    g_object_set_data (G_OBJECT (widgets_p->status), "clean", NULL);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW  (widgets_p->status), TRUE);
    lp_set_active(widgets_p, TRUE);
    
    rfm_population_read_unlock (view_p, "on_button_release");

    return FALSE;
}

gint
on_motion_event (
    GtkWidget * textview,
    GdkEventMotion* event,
    gpointer data
) {
    //widgets_t *widgets_p = (widgets_t *) data;
    //view_t *view_p = widgets_p->view_p;
    NOOP(stderr,"GdkEventMotion...\n");
    if (event->x < TINY_ICON_SIZE) return TRUE;
    return FALSE;
}

#endif


