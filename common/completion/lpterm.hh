#ifndef LPTERM_HH
#define LPTERM_HH
//#include "run_button_c.hpp"
#include "common/run.hh"
#include "common/util.hh"

//#ifdef HAVE_READLINE_HISTORY_H
# include <readline/history.h>
//#endif
namespace xf {
template <class Type> class LpTerm;
template <class Type> class PageChild;

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
#if 0
    gboolean 
    window_keyboard_event(GdkEventKey * event, void *data)
    {
	TRACE("window_keyboard_event\n");
	/* asian Input methods */
	if(event->keyval == GDK_KEY_space && (event->state & (GDK_MOD1_MASK | GDK_SHIFT_MASK))) {
	    return FALSE;
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
		TRACE("window_keyboard_event: key ignored\n");
		return TRUE;
	    }
	}


	TRACE( "window_keyboard_event: lpterm is active_ = %d\n", event->keyval, active_);

	if (!active_ && is_iconview_key(event)) {
	    TRACE("window_keyboard_event: Sending key to iconview default handler.\n");
	    return FALSE;
	}

	if (active_ && (event->keyval == GDK_KEY_Escape)) {
	    TRACE("window_keyboard_event: set_active_lp = %d\n", FALSE);
	    lp_set_active(FALSE); 
	    return TRUE;
	}

	if (!active_) {
	    TRACE("window_keyboard_event: set_active_lp = %d\n", TRUE);
	    lp_set_active(TRUE); 
	    if (event->keyval == GDK_KEY_Tab){
		event->keyval = GDK_KEY_Escape;
	    }
	    if (event->keyval == GDK_KEY_Escape){
		return TRUE;

	    } 
	}

	// XXXXXXXX
	//
	//
	// By now we have a lp key to process
	TRACE( "window_keyboard_event: send key to status dialog for lpterm command\n");
	lpterm_keyboard_event(event, data);
	return TRUE;


	/* FIXME: CTL callbacks...
	if (rodent_do_callback(event->keyval, event->state)) {
	    TRACE("window_keyboard_event: Tried callback with keyval!\n");
	    return TRUE;
	} */
	
	/* FIXME: icon selection by keyboard
	if (view_p->selection_list) {
	    // selection list must be redefined...
	    update_reselect_list(widgets_p);
	    TRACE( "window_keyboard_event: Selection---> %p\n", view_p->selection_list);	
	} */
	
	//False defaults to status line keybinding signal callback
	return FALSE;
    }
	
	
    static gboolean
    is_iconview_key(GdkEventKey *event){
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

    static gboolean
    is_lpterm_key(GdkEventKey * event){
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
#endif

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
    PageChild<Type> *page_;

    gboolean active_;
    GtkIconView *iconview_;
    GtkWidget *status_button_;
    GtkWidget *status_icon_;
    GtkWidget *iconview_icon_;

public:
    void setPage(PageChild<Type> *page){page_ = page;}
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
		EVENT_CALLBACK (status_keyboard_event), data);
	g_signal_connect (status_button, "button-press-event", 
		BUTTON_CALLBACK (on_status_button_press), (void *)this);
		*/
    }

    gboolean
    lp_get_active(void){return active_;}

    void
    lp_set_active(gboolean state){
	active_ = state;
/*	if (state){
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

    gchar * 
    run_lp_command(GtkTextView *output, const gchar *workdir, const gchar *command){
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
		DBG("internal command=%s\n", command);
		continue;
	    }
	    // automatic shell determination:
            if (strcmp(workdir, "xffm:root")==0) {
                if (chdir(g_get_home_dir()) < 0){
                    DBG("Cannot chdir to %s\n", g_get_home_dir());
                    DBG("aborting command: \"%s\"\n", command);
                    continue;
                }
            } else {
                if (chdir(workdir) < 0){
                    DBG("Cannot chdir to %s\n", workdir);
                    DBG("aborting command: \"%s\"\n", command);
                    continue;
                }
            }
	    pid_t child = run_c::thread_run(output, *c, FALSE);
	    page_->newRunButton(*c, child);
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
	return newWorkdir;
    }

    void
    open_terminal(GtkTextView *output){
	const gchar *terminal = util_c::what_term();
	run_c::shell_command(output, terminal, FALSE);
/*	run_button_c *run_button_p = NULL;
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
	    print_c::show_text(output);
	    print_c::print(output, "tag/green", g_strdup_printf("cd %s\n", g_get_home_dir()));
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
	print_c::show_text(output);

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

	print_c::print(output, "tag/green", g_strdup_printf("cd %s\n", gg));
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
		DBG("newWorkdir-gg = %s\n", gg);
		DBG("page_ = %p\n", (void *)page_);
		if (page_) {
		    page_->setPageWorkdir(gg);
		    g_free(gg);
		}
		return TRUE;
	    }

	    return TRUE;
	}
	g_strfreev (argvp);
	return  FALSE;
    }
     

#if 0
    // This is now at completion class
    gint
    lpterm_keyboard_event( GdkEventKey * event, gpointer data) {
	TRACE( "lpterm_keyboard_event...\n");
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
#endif

};

#if 0
// work in progress: run command and open with dialog
// see src/lpterm.cpp
#endif


///////////////////////   static GTK callbacks... ////////////////////////////////////////


} // namespace
#endif 

