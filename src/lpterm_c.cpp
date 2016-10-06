//#define DEBUG_TRACE 1

#include "view_c.hpp"
#include "lpterm_c.hpp"
#include "run_button_c.hpp"

#ifdef HAVE_READLINE_HISTORY_H
# include <readline/history.h>
#endif


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
	gtk_widget_grab_focus (iconview);
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

gint
lpterm_c::lpterm_keyboard_event( GdkEventKey * event, gpointer data) {
    TRACE( "lpterm_c::lpterm_keyboard_event...\n");
    if(!event) {
        g_warning ("on_status_key_press(): returning on event==0\n");
        return TRUE;
    }
    view_c *view_p =(view_c *)data; 
    gtk_widget_grab_focus (GTK_WIDGET(status));
    gtk_c *gtk_p = view_p->get_gtk_p();

    
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


