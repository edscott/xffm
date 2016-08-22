
#include "lpterm_c.hpp"

#ifdef HAVE_READLINE_HISTORY_H
# include <readline/history.h>
#endif

lpterm_c::lpterm_c(void){
    active = FALSE;
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
    // FIXME: actrivate status cursor
    active = state;
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


#ifdef OLDCODE
static void place_cursor(GtkTextView *status, gint pos){
    GtkTextIter iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (status);
    gtk_text_buffer_get_iter_at_offset (buffer, &iter, 2+pos);
    gtk_text_buffer_place_cursor (buffer, &iter);
}

static void place_command(GtkWidget *textview, widgets_t *widgets_p, const gchar *p){
    rfm_status (widgets_p, "xffm/emblem_terminal", (char *) p, NULL);
    g_object_set_data (G_OBJECT (textview), "clean", NULL);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW  (widgets_p->status), TRUE);
    gtk_widget_grab_focus (widgets_p->status);
}

static void
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

static gboolean
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

static void
print_history ( widgets_t * widgets_p) {
    int i;
    view_t *view_p = widgets_p->view_p;
    GList *p;
    rfm_diagnostics (widgets_p, "xffm_tag/command", "history:", "\n", NULL);

    for(i = 1, p = g_list_first (view_p->sh_command); p && p->data; p = p->next, i++) {
        print_suggestion (widgets_p, (char *) p->data, i, TRUE);
    }

}

static void
print_tab ( widgets_t * widgets_p, gchar * text, gchar *text2) {
    int tab_len = 18;
    rfm_diagnostics (widgets_p, "xffm_tag/red", text, text2, NULL);
    gint string_length = (text)?strlen(text):0 + (text2)?strlen(text2):0;
    for(tab_len = tab_len - string_length; tab_len > 0; tab_len--)
        rfm_diagnostics (widgets_p, NULL, " ", NULL);
}

static void
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

static void
suggest_command (
    widgets_t * widgets_p,
    const char *complete,
    gboolean anywhere
) {
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

static gboolean
internal_history ( widgets_t * widgets_p, gchar *cmd) {
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
static gchar *readline_history(widgets_t *widgets_p, gchar *cmd){
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
        place_cursor(GTK_TEXT_VIEW(widgets_p->status), strlen(expansion));
    }
    if (!expansion || !strlen(expansion) || result <= 0){
       g_free(expansion);
       expansion = NULL;
    }
    clear_history();
    return expansion;
}
#endif

static gboolean
process_internal_commands ( widgets_t * widgets_p, gchar ** command) {
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

static 
gchar *sudo_fix(gchar *command){
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

static gint
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
static gint
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

static gboolean
csh_completion(GtkWidget * textview, widgets_t *widgets_p, gint direction){
    view_t *view_p = widgets_p->view_p;
    gchar *command = get_current_text ((GtkTextView *) widgets_p->status);
    if (!command || !strlen(command)) {
        g_object_set_data(G_OBJECT(widgets_p->status), "csh_cmd_len", NULL);
        g_object_set_data(G_OBJECT(widgets_p->status), "csh_nth", NULL);
        return FALSE;
    }
    gint csh_cmd_len = 
        GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widgets_p->status), "csh_cmd_len"));
    if (!csh_cmd_len) {
        g_object_set_data(G_OBJECT(widgets_p->status), "csh_nth", NULL);
        return FALSE;
    }
    gint nth = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widgets_p->status), "csh_nth"));

    if (!nth) nth = g_list_length(view_p->sh_command);
    gchar *p=NULL;
    if (direction > 0){
        if (nth > 1){ 
            nth--;
	    GList *list = g_list_nth(view_p->sh_command, nth);
	    for (;list && list->data;list=list->prev, nth--){
		p = list->data;
		if (p && strncmp(command, p, csh_cmd_len)==0) break;
		p=NULL;
	    } 
	}
    } else {
	nth++;
	if (nth <= g_list_length(view_p->sh_command)){
            GList *list = g_list_nth(view_p->sh_command, nth);
            for (;list && list->data;list=list->next, nth++){
                p = list->data;
                if (p && strncmp(command, p, csh_cmd_len)==0) break;
                p=NULL;
            } 
        }
    }
    if (p){
            TRACE("gotcha (%d): %s\n", nth, p);
            g_object_set_data(G_OBJECT(widgets_p->status), "csh_nth", GINT_TO_POINTER(nth));
	    place_command(textview, widgets_p, p);
            place_cursor(GTK_TEXT_VIEW(widgets_p->status), csh_cmd_len);
    }
    return TRUE;    
}

static gboolean
offset_history(GtkWidget *textview, widgets_t *widgets_p, gint offset){
    view_t *view_p = widgets_p->view_p;
    void *p = g_list_nth_data (view_p->sh_command, view_p->sh_command_counter + offset);
    NOOP ("get nth sh_command_counter=%d\n", view_p->sh_command_counter + offset);
    if(p) {
            view_p->sh_command_counter += offset;
	    place_command(textview, widgets_p, p);
    }
    return TRUE;
}

static gint
on_status_key_press (
    GtkWidget * textview,
    GdkEventKey * event,
    gpointer data
) {
    widgets_t *widgets_p = (widgets_t *) data;
    view_t *view_p = widgets_p->view_p;
    if(!event) {
        DBG ("on_status_key_press(): returning on event==0\n");
        //status_grab_focus (widgets_p->view_p, 0);
        return TRUE;
    }

    // backspace must not erase icon. Place_cursor is offset by 2 for this purpose.
    TRACE ("on_status_key_press(): * on_status_key_press: got key= 0x%x\n", event->keyval);
    if(event->keyval == GDK_KEY_Home) {
        place_cursor(GTK_TEXT_VIEW(widgets_p->status), 0);
        return TRUE;
    }
    if(event->keyval == GDK_KEY_BackSpace || event->keyval == GDK_KEY_Left) {
        gint intval;
        GtkTextBuffer *buffer = gtk_text_view_get_buffer ((GtkTextView *) widgets_p->status);
        g_object_get (G_OBJECT (buffer), "cursor-position", &intval, NULL);
        NOOP ("on_status_key_press(): cursor-position =%d\n", intval);
        if(intval > 2) {
            // offset==1 is the icon and offset==2 is the space after icon
            return FALSE;
        }
        return TRUE;
    }

    // CTRL-TAB for command history completion
    if(event->keyval == GDK_KEY_Tab && event->state & GDK_CONTROL_MASK) {
        // do history completion ...
        gchar *complete = get_current_text ((GtkTextView *) widgets_p->status);
	if (strncmp(complete, "sudo", strlen("sudo"))==0 &&
	    strncmp(complete, "sudo -A", strlen("sudo -A"))){
	    gchar *o = complete;
	    gchar *oo = o+strlen("sudo");
	    while (*oo == ' ') oo++;
	    complete = g_strconcat("sudo -A ", oo, NULL);
	    g_free(o);
	}
#if 0
	// too nerdy?
	// "&&" --> " && "
	// "||" --> " || "
	// ";" --> "; "
	// "|" --> " | "
	if (command && strlen(command)){
	    command = subst(complete, "&&", " && ");
	    command = subst(complete, "||", " || ");
	    command = subst(complete, ">>", " >> ");
	    command = subst(complete, ";", "; ");	
	    while (strchr(complete, "  ")) command = subst(complete, "  ", " ");
	}
#endif
        gchar *suggest = rfm_rational(RFM_MODULE_DIR,
		"completion", widgets_p, complete,
		"rfm_history_completion");
        g_free (complete);
        if(suggest) {
            rfm_status (widgets_p, "xffm/emblem_terminal", suggest, NULL);
            g_free (suggest);
        }
        g_object_set_data (G_OBJECT (widgets_p->status), "clean", NULL);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW  (widgets_p->status), TRUE);
        return TRUE;
    }
    // tab for bash completion
    if(event->keyval == GDK_KEY_Tab) {
	gchar *head=get_text_to_cursor(GTK_TEXT_VIEW(widgets_p->status));
	gint head_len = strlen(head);
        gchar *token = get_current_text (GTK_TEXT_VIEW(widgets_p->status));
	gint token_len = strlen(token);

	//gchar *tail=get_text_from_cursor(GTK_TEXT_VIEW(widgets_p->status));
        g_free (head);
	//gchar *suggest = rfm_bash_complete(widgets_p, token, head_len);
	gchar *suggest = rfm_complex(RFM_MODULE_DIR, "completion",
		widgets_p, token, GINT_TO_POINTER(head_len),
		"rfm_bash_complete");
        g_free (token);

	if (suggest) {
	    gint suggest_len = strlen(suggest);
	    GtkTextIter end;
	    GtkTextBuffer *buffer = 
		gtk_text_view_get_buffer (GTK_TEXT_VIEW(widgets_p->status));
	    gint offset = -1;
	    // +2 is icon and space...
	    offset = head_len + (suggest_len - token_len) + 2;
            rfm_status (widgets_p, "xffm/emblem_terminal", suggest, NULL);
	    gtk_text_buffer_get_iter_at_offset (buffer, &end, offset);
	    gtk_text_buffer_place_cursor(buffer, &end);	
	}
	g_free(suggest);

        g_object_set_data (G_OBJECT (widgets_p->status), "clean", NULL);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW  (widgets_p->status), TRUE);
        return TRUE;
    }
    if ((event->state & GDK_SHIFT_MASK)
	    &&
	    ((event->keyval == GDK_KEY_Page_Up) || 
	    (event->keyval == GDK_KEY_Page_Down) ||
	    (event->keyval == GDK_KEY_Up) || 
	    (event->keyval == GDK_KEY_Down)) ) 
    {
	// Show text.
        rfm_show_text (widgets_p);
	// Get diagnostics area height.
	// Scroll up or down.
	gboolean page = (event->keyval == GDK_KEY_Page_Up || event->keyval == GDK_KEY_Page_Down); 
	gboolean up = (event->keyval == GDK_KEY_Page_Up || event->keyval == GDK_KEY_Up); 
	NOOP(stderr, "scroll \n");
	rfm_scroll(widgets_p, up, page);

        return TRUE;
    }
    if(event->keyval == GDK_KEY_Up) {
        // csh command completion
        if (csh_completion(textview, widgets_p, 1)) return TRUE;
	// push/pop history
	offset_history(textview, widgets_p, -1);
	return TRUE;
    }


    if(event->keyval == GDK_KEY_Down) {
        // csh command completion
        if (csh_completion(textview, widgets_p, -1)) return TRUE;
	// push/pop history
	offset_history(textview, widgets_p, 1);
        return TRUE;
    }
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
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW  (widgets_p->status), TRUE);
        g_object_set_data (G_OBJECT (textview), "clean", NULL);
        return TRUE;
    }

    if(g_object_get_data (G_OBJECT (textview), "clean")) {
        rfm_status (widgets_p, "xffm/emblem_terminal", NULL);
    }
    // set unclean status...
    g_object_set_data (G_OBJECT (textview), "clean", NULL);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW  (widgets_p->status), TRUE);
    // Allow default handler for text buffer
    if (event->keyval >= GDK_KEY_space && event->keyval <= GDK_KEY_asciitilde){
        gchar *command = get_current_text ((GtkTextView *) widgets_p->status);
        gint csh_cmd_len = strlen(command)+1;
        g_object_set_data(G_OBJECT(widgets_p->status), "csh_cmd_len", GINT_TO_POINTER(csh_cmd_len));
    }

    return FALSE;
}


static gboolean
lp_get_active(widgets_t *widgets_p){
    if(g_object_get_data (G_OBJECT (widgets_p->status), "active")) return TRUE;
    return FALSE;
}

static void lp_set_active(widgets_t *widgets_p, gboolean state){
    if (state) g_object_set_data (G_OBJECT (widgets_p->status), "active", GINT_TO_POINTER(1));
    else g_object_set_data (G_OBJECT (widgets_p->status), "active", NULL);
    return;
}

// Shared keybindings with either iconview or callback
typedef struct lpkey_t{
    guint key;
    guint mask;
} lpkey_t;


#endif


