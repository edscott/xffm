#ifdef COPYRIGHT_INFORMATION
#include "gplv3.h"
#endif
#include "primary-internal.h"
/*
 * Copyright (C) 2002-2012 Edscott Wilson Garcia
 * EMail: edscott@users.sf.net
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; 
 */

// All these functions are thread called. GDK mutex must be
// locked if instruction set requires this mutex.


// XXX duplicate and symlink could be multiple selection



// These callbacks will be processed by keybindings and showed in the
// keybindings listing. If a menu option is not listed here, then it will
// not be subject to a keybinding, nor a keybinding modification.
static 
RodentCallback menu_callback_v[]={
    // F keys
 HELP_CALLBACK,
 SETTINGS_CALLBACK,
 MENU_CALLBACK,
 TERMINAL_CALLBACK,
 RELOAD_CALLBACK,
 RUN_CALLBACK,
 SEARCH_CALLBACK,
 DIFFERENCES_CALLBACK,
// ADD_BOOKMARK_CALLBACK,
// REMOVE_BOOKMARK_CALLBACK,
 CASE_SORT_CALLBACK,
 TOGGLE_BOOKMARK_CALLBACK,
 // ctrl F keys
 SORT_ASCENDING_CALLBACK,
 SORT_DESCENDING_CALLBACK,
 NAME_SORT_CALLBACK,
 TYPE_SORT_CALLBACK,
 DATE_SORT_CALLBACK,
 SIZE_SORT_CALLBACK,
 OWNER_SORT_CALLBACK,
 GROUP_SORT_CALLBACK,
 MODE_SORT_CALLBACK,
 KEYBINDINGS_CALLBACK,
// 
 NEW_FILE_CALLBACK,
 NEWDIR_CALLBACK,
 BCRYPT_CALLBACK,
 RENAME_CALLBACK,
 DUPLICATE_CALLBACK,
 SYMLINK_CALLBACK,
 TOUCH_CALLBACK,
 PROPERTIES_CALLBACK,
 SELECT_ALL_CALLBACK,
 SELECT_INVERT_CALLBACK,
 SELECT_BYFILTER_CALLBACK,
 UNSELECT_BYFILTER_CALLBACK,
 UNSELECT_ALL_CALLBACK,
 NEW_TAB_CALLBACK,
 NEW_WINDOW_CALLBACK,
 SHOW_HIDDEN_CALLBACK,
 SHOW_BACKUP_CALLBACK,
 PREVIEW_IMAGES_CALLBACK,
 COMPACT_ICONSIZE_CALLBACK,
 TINY_ICONSIZE_CALLBACK,
 NORMAL_ICONSIZE_CALLBACK,
 BIG_ICONSIZE_CALLBACK,
 HUGE_ICONSIZE_CALLBACK,
 PLUS_ICONSIZE_CALLBACK,
 MINUS_ICONSIZE_CALLBACK,
 ABOUT_CALLBACK,
 JUMP_TO_CALLBACK,
 GOTO_HOST_CALLBACK,
 GOTO_HOME_CALLBACK,
 GO_UP_CALLBACK,
 GO_BACK_CALLBACK,
 GO_FORWARD_CALLBACK,
 CUT_CALLBACK,
 COPY_CALLBACK,
 PASTE_CALLBACK,
 REMOVE_CALLBACK,
// AUTOTYPE_CALLBACK,
// AUTOTYPE_R_CALLBACK,
 OPEN_WITH_CALLBACK,
 FIND_CALLBACK,
 MOUNT_CALLBACK,
 UNMOUNT_CALLBACK,
 CLOSE_CALLBACK,
 LS_CALLBACK,
 NULL_CALLBACK
};

static RodentCallback*
get_menu_callback_p(gint menu_enum){
    gint i;
    // Check if menu_enum is defined in menu_callback_v
    for (i=0; menu_callback_v[i].function_id >= 0; i++){
	if (menu_enum==menu_callback_v[i].function_id) {
            // Now check whether function_id is valid
            if (menu_callback_v[i].function_id >= ENUM_CALLBACKS) return NULL;
	    return &(menu_callback_v[i]);
	}
    }
    return NULL;
}

static gboolean
is_valid_view_entry(widgets_t *widgets_p, gint menu_enum){
    view_t *view_p = widgets_p->view_p;
    if (!rfm_entry_available(widgets_p, view_p->en)) {
	RodentCallback *menu_callback_p = get_menu_callback_p(menu_enum);
	rfm_show_text(widgets_p);
	rfm_threaded_diagnostics(widgets_p, "xffm/stock_dialog-warning", NULL);
	rfm_threaded_diagnostics(widgets_p, "xffm_tag/blue",
		g_strconcat(
		    (menu_callback_p->string)?_(menu_callback_p->string):".",
		": ", _( "Could not validate the transaction"), "\n",NULL));
	NOOP("callback_enter(): view entry is no longer valid\n");
	return FALSE; 
    }
    return TRUE;
}

static gboolean
is_single_selection(widgets_t *widgets_p, gint menu_enum){
    view_t *view_p = widgets_p->view_p;
    if (g_slist_length(view_p->selection_list) != 1){
	RodentCallback *menu_callback_p = get_menu_callback_p(menu_enum);
	rfm_threaded_show_text(widgets_p);
	rfm_threaded_diagnostics(widgets_p, "xffm/stock_dialog-warning",NULL);
	rfm_threaded_diagnostics(widgets_p, "xffm_tag/blue",
		g_strconcat((menu_callback_p->string)?_(menu_callback_p->string):".",
		": ", _( "No selection available"), "\n",NULL));
	return FALSE;
    }
    return TRUE;
}


static gboolean
is_multiple_selection(widgets_t *widgets_p, gint menu_enum){
    view_t *view_p = widgets_p->view_p;
    if (g_slist_length(view_p->selection_list) == 0){
	RodentCallback *menu_callback_p = get_menu_callback_p(menu_enum);
	rfm_threaded_show_text(widgets_p);
	rfm_threaded_diagnostics(widgets_p, "xffm/stock_dialog-warning",NULL);
	rfm_threaded_diagnostics(widgets_p, "xffm_tag/blue",
		g_strconcat(
		(menu_callback_p->string)?_(menu_callback_p->string):".",
		": ", _( "No group selected"), "\n",NULL));
	return FALSE;
    }
    return TRUE;
}
static void
time_out_message(widgets_t *widgets_p, const gchar *path){
    rfm_threaded_show_text(widgets_p);
    rfm_threaded_diagnostics(widgets_p, "xffm/stock_dialog-error",  g_strconcat(path, ": ", NULL));
    rfm_threaded_diagnostics(widgets_p, "xffm_tag/stderr", g_strconcat(_(strerror(ETIMEDOUT)), "\n", NULL));
    rfm_global_t *rfm_global_p = rfm_global();
    rfm_threaded_cursor_reset(rfm_global_p->window);
    return;
}
///////////////////////////////////////////////////////////////////////////////////
///******************************************************************************/
//////////////////////////////////////////////////////////////////////////////////
static void jump_to (widgets_t *widgets_p);
static void open_x(widgets_t *widgets_p);
static void
open_with (widgets_t *widgets_p, record_entry_t * en) {
    /* open with */
    gchar *command=NULL;
    gchar *command_fmt=NULL;
    NOOP ("open_with()... \n");

    if(!en || !en->path) {
        NOOP ("OPEN: open_with !en || !en->path\n");
        return;
    }

    gchar *wd = g_path_get_dirname (en->path);
    if (!rfm_g_file_test_with_wait (wd, G_FILE_TEST_EXISTS)){
	 time_out_message(widgets_p, wd);
	 g_free(wd);
	 wd = g_strdup(g_get_home_dir());
     }
	
    g_free (widgets_p->workdir);
    widgets_p->workdir = wd;

    // Here we take special consideration for shell scripts.
    // Shell scripts will be editable files, therefore will
    // have an associated mime_command to open the editor.
    // tests 

    if (!en->mimetype) en->mimetype=MIME_type (en->path, en->st);
    if(!en->mimemagic){
	if (IS_LOCAL_TYPE(en->type) && !en->mimemagic) {
	    en->mimemagic = rfm_rational(RFM_MODULE_DIR, "mime", en, "mime_magic", "mime_function");
	    if (!en->mimemagic) en->mimemagic = g_strdup(_("unknown"));
	}
	else en->mimemagic = g_strdup(_("unknown"));
    }

    if(!en->filetype) {
	if (IS_LOCAL_TYPE(en->type)) {
	    en->filetype = rfm_rational(RFM_MODULE_DIR, "mime", en, "mime_file", "mime_function"); 
	    if (!en->filetype) en->filetype = g_strdup(_("unknown"));
	}
	else en->filetype = g_strdup(_("unknown"));
    }

    command_fmt = MIME_command (en->mimetype);
    NOOP ("OPEN: command_fmt(%s) = %s\n", en->mimetype, command_fmt);
    if(!command_fmt) {
        command_fmt = MIME_command (en->mimemagic);
    }

    gboolean is_script= ((en->mimetype && strstr (en->mimetype, "/x-sh")) ||
			 (en->mimemagic && strstr (en->mimemagic, "/x-sh")) ||
		 (en->mimetype && strstr (en->mimetype, "/x-shellscript")) ||   
		 (en->mimemagic && strstr (en->mimemagic, "/x-shellscript")) ||
			 (en->mimetype && strstr (en->mimetype, "/x-csh")) ||   
			 (en->mimemagic && strstr (en->mimemagic, "/x-csh")) ||
			 (en->mimetype && strstr (en->mimetype, "/x-perl")) ||   
			 (en->mimemagic && strstr (en->mimemagic, "/x-perl")) 			 );
    if (is_script && !IS_EXE_TYPE(en->type)){
	g_free(command_fmt);
	command_fmt = NULL;
    }

    // for default editor...
    gchar *text_editor = NULL;
    if(!command_fmt) {
	text_editor = rodent_get_text_editor(en);
	NOOP ("OPEN: text_editor = %s\n", text_editor);
	if(text_editor) {
	    /* OK to apply an editor */
	    command_fmt = g_strconcat(text_editor, " ", NULL);
	}
    }

    //command_fmt=get_command_fmt(en);
    if (is_script) {
	rfm_threaded_show_text(widgets_p);
	if (!IS_EXE_TYPE(en->type)){
	    rfm_threaded_diagnostics(widgets_p, "xffm/stock_dialog-warning", 
		    g_strconcat(en->path, "\n", NULL));
	    rfm_threaded_diagnostics(widgets_p, "xffm_tag/stderr",
		    g_strconcat(_("The program exists, but is not executable.\nPlease check your installation and/or install the binary properly."), 
		    "\n", NULL));
	    rfm_threaded_diagnostics(widgets_p, "xffm/stock_dialog-info", NULL);
	    gchar *text=g_strdup_printf (_("Open with %s"), _("Text Editor"));
	    gchar *base=g_path_get_basename(en->path);
	    rfm_threaded_diagnostics(widgets_p, "xffm_tag/green", g_strconcat(text, ": ", base, "\n", NULL));
	    g_free(base);
	    g_free(text);
	}
    }

    NOOP ("open_with(): magic=%s, mime=%s, command_fmt=%s, editor=%s\n",
	    en->mimemagic, en->mimetype, command_fmt, text_editor);
    g_free(text_editor);

    if(command_fmt) {
        command = MIME_mk_command_line (command_fmt, en->path);
        TRACE( "OPEN: command = %s\n", command);

        RFM_THREAD_RUN2ARGV (widgets_p, command, FALSE);
        g_free (command);
        g_free (command_fmt);
    } else {
	open_x(widgets_p);
        //rodent_open_with_activate (NULL, (gpointer) widgets_p);
    }
    return;
}


// This is used by the go back callback
static void
pop_view_go_history (view_t * view_p) {
    GList *last;
    if(!view_p->go_list)
        return;
    last = g_list_last (view_p->go_list);
    if(!last) {
        g_list_free (view_p->go_list);
        view_p->go_list = NULL;
        return;
    }
    view_p->go_list = g_list_remove (view_p->go_list, last->data);
    if(!g_list_length (view_p->go_list)) {
        g_list_free (view_p->go_list);
        view_p->go_list = NULL;
        return;
    }
    return;
}

// This is used by the goto callback
static gchar *get_jumpto_dir(widgets_t *widgets_p){
   if (g_thread_self() == rfm_get_gtk_thread()){
	g_error("get_jumpto_dir() is a thread function\n");
    }
    view_t *view_p = widgets_p->view_p;
    gchar *f = g_build_filename (GOTO_DBH_FILE, NULL);
    gchar *response = get_response_history ( _("Go To"), _("Path"), 
	    NULL, // extra_text
	    f,
	    NULL, NULL, NULL, NULL,
            GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
            g_get_home_dir (),
	    MATCH_FILE);
    g_free (f);

    NOOP("dialog returned with %s\n", response);
    if (!response) return NULL;
    gchar *choice = response;

    g_strstrip(choice);
    if (strlen(choice)==0) {
	g_free(choice);
	return NULL;
    }
    if (g_path_is_absolute(choice)){
	if (!rfm_g_file_test_with_wait (choice, G_FILE_TEST_EXISTS)){
	   time_out_message(widgets_p, choice);
	   g_free(choice);
	   return NULL;
	}
    } else {
	// make path absolute
	const gchar *dirname;
	if (view_p->en && view_p->en->path) dirname = view_p->en->path;
	else dirname = g_get_home_dir();
	gchar *fullpath = g_build_filename(dirname, choice, NULL);
	if (!rfm_g_file_test_with_wait (fullpath, G_FILE_TEST_EXISTS)){
	   time_out_message(widgets_p, fullpath);
	   g_free(fullpath);
	   g_free(choice);
	   return NULL;
	}
	g_free(fullpath);
    }


    if(!rfm_g_file_test_with_wait (choice, G_FILE_TEST_IS_DIR)) {
	gchar *text=g_strdup_printf(_("%s does not exist."), choice);
	rfm_confirm (widgets_p,GTK_MESSAGE_ERROR, text, NULL, NULL);
	g_free(choice);
	g_free(text);
        return NULL;
    }
    else if(chdir (choice) < 0) {
        rfm_threaded_show_text(widgets_p);
        rfm_threaded_diagnostics (widgets_p, "xffm/stock_dialog-error", NULL);
        rfm_threaded_diagnostics (widgets_p, "xffm_tag/stderr", g_strconcat(choice, ": ", strerror (errno), "\n", NULL));
	g_free(choice);
	return NULL;
    } 
    g_free(choice);
    gchar *g = g_get_current_dir ();
    SETWD();
    return g;
}

/*****************************************************************************/

// Used by execute()
static gchar *
get_command_fmt(record_entry_t *en) {
    gchar *command_fmt = MIME_command (en->mimetype);
    if(!command_fmt) {
        command_fmt = MIME_command (en->mimemagic);
    }
    gboolean is_text = ((en->mimetype && strstr (en->mimetype, "text/")) ||
                        (en->mimemagic && strstr (en->mimemagic, "text/")) || 
                        (en->filetype && strstr (en->filetype, "text")));
    if(!command_fmt && is_text && getenv ("EDITOR_CMD") && strlen (getenv ("EDITOR_CMD"))) {
        command_fmt = g_strdup (getenv ("EDITOR_CMD"));
    }
    return command_fmt;
}

// Used by execute()
static gchar *
strip_path(gchar *command_fmt, const gchar *path){
    if (!path) return command_fmt;
    NOOP("stipping %s\n", command_fmt);
    if (strstr(command_fmt, path)){
        gchar *end = strstr(command_fmt,path) + strlen(path);
        *strstr(command_fmt, path) = 0;
        gchar *g = g_strconcat(command_fmt, "%","s",end, NULL);
        g_free(command_fmt);
        command_fmt=g;
        NOOP("stipped %s\n", command_fmt);
        return command_fmt;
    }
    // What if the path is quoted or escaped or both
    gchar *esc_path = rfm_esc_string (path);
    if (strstr(command_fmt, esc_path)){
        command_fmt = strip_path(command_fmt, esc_path);
    }
    g_free(esc_path);
    return command_fmt;
}

typedef struct execute_t{
    widgets_t *widgets_p;
    GSList *list;
} execute_t;

// Used by run and open-with callbacks
// This opens the confirmation or user input dialog.
static void *
execute (widgets_t *widgets_p, GSList *selection_list) {
  if (g_thread_self() == rfm_get_gtk_thread()){
	g_error("execute() is a thread function\n");
    }

    view_t *view_p = widgets_p->view_p;
    //GSList *selection_list = execute_p->list;
    gpointer retval=GINT_TO_POINTER(1);
    
    gchar *command_fmt=NULL;
    /* set the working directory */
    g_free (widgets_p->workdir);
    if(view_p->en && IS_LOCAL_TYPE(view_p->en->type) && view_p->en->path)
        widgets_p->workdir = g_strdup (view_p->en->path);
    else
        widgets_p->workdir = g_strdup (g_get_home_dir ());

    TRACE ("execute()...\n");
    gchar *files = NULL;
    gchar *files_txt;

    gchar *first_path=NULL;
    if(selection_list) {
	files_txt = g_strdup_printf (_("Open with %s"),":   \n\n");
        if(g_slist_length (selection_list)) {
            gchar *tt,
             *ttt;
            files = g_strdup (" ");
            GSList *tmp = selection_list;
            for(; tmp; tmp = tmp->next) {
                record_entry_t *en = (record_entry_t *) tmp->data;
                char *b = g_path_get_basename (en->path);
                gchar *q = rfm_utf_string (rfm_chop_excess (b));
                tt = g_strconcat (files_txt, q, "\n", NULL);
                gchar *esc_path = rfm_esc_string (en->path);
		if (!first_path){
		    first_path=g_strdup(esc_path);
		    command_fmt=get_command_fmt(en);
		}
                ttt = g_strconcat (files, esc_path, " ", NULL);
                g_free (esc_path);
                g_free (files_txt);
                g_free (q);
                g_free (b);
                g_free (files);
                files_txt = tt;
                files = ttt;
            }
        }
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
	retval=NULL;
	goto cleanup;
    }
    if(g[strlen (g) + 1])
        interm = TRUE;

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

    if(widgets_p->diagnostics_window){
	if (!rfm_threaded_get_visible(widgets_p->diagnostics_window)){
	    rfm_threaded_show_text(widgets_p);
	}
    } else {
	rfm_threaded_show_text(widgets_p);
    }

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

    g_free (command);
    // Cleanup
cleanup:;
    GSList *list = selection_list;
    for (; list && list->data; list = list->next){
	record_entry_t *en = list->data;
	rfm_destroy_entry(en);
    }
    if (selection_list) g_slist_free(selection_list);
    TRACE("execute done...\n");
    return retval;
}


static void
rodent_save_workdir_history (char *p) {
    gchar *f = g_build_filename (WORKDIR_DBH_FILE, NULL);
    COMBOBOX_save_to_history (f, p);
    g_free (f);
}
// Determines the work directory of the command to be executed
// to do things like extract a tar file to a selected diretory
static gchar * 
autofunction_workdir (widgets_t *widgets_p, const gchar * querypath) {
    if (g_thread_self() == rfm_get_gtk_thread()){
	g_error("autofunction_workdir() is a thread function\n");
    }
    view_t *view_p = widgets_p->view_p;
    static gchar *last_dir = NULL;
    gchar *g=NULL;

    if (!view_p->en) return NULL;
    if(querypath) {
        gchar *f = g_build_filename (WORKDIR_DBH_FILE, NULL);
        const gchar *folder;
        if(view_p->en && view_p->en->path && IS_SDIR(view_p->en->type)) {
            folder = view_p->en->path;
        } else {
            folder = g_get_home_dir ();
        }
        NOOP ("COMBO: last_dir is %s\n", last_dir);
	//const gchar *default_dir=last_dir;
	const gchar *default_dir=NULL;
	if (!default_dir) default_dir=view_p->en->path;
	if (!rfm_entry_available(widgets_p, view_p->en)) {
	    default_dir=g_get_home_dir();
	} 
	g = get_response_history ( _(querypath), 
			    _("Path"), _("Select directory"),
			    f, NULL,
                            default_dir,
                            NULL, NULL,
                            GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                            folder,
			    MATCH_FILE);
        g_free (f);

        if(!g) return NULL;

	gboolean exists = g_file_test (g, G_FILE_TEST_EXISTS);
	gboolean isdir = g_file_test (g, G_FILE_TEST_IS_DIR);

	
	if(exists && !isdir) {
            rfm_threaded_status (widgets_p, "xffm/stock_dialog-warning", g_strdup(strerror (ENOTDIR)));
	    g_free(g);	    
            return NULL;
        }
        if(!exists) {
            gchar *b = g_strdup_printf ("%s: %s\n%s...", g,
                                        strerror (ENOENT),_("Create New Folder"));
            if(!rfm_confirm (widgets_p, GTK_MESSAGE_QUESTION, b, _("Cancel"), NULL)) {
		g_free(g);	    
                g_free (b);
                return NULL;
            }
            g_free (b);
            if(g_mkdir_with_parents (g, 0750) < 0) {
		rfm_threaded_show_text(widgets_p);
		rfm_threaded_diagnostics (widgets_p, "xffm/stock_dialog-error", NULL);
		rfm_threaded_status (widgets_p, "xffm_tag/stderr", g_strconcat(g, ": ", strerror (errno), "\n", NULL));
		g_free(g);	    
                return NULL;
            }
        }

    
        g_free (last_dir);
        last_dir = g_strdup (g);
        NOOP ("COMBO: setting last_dir to %s\n", last_dir);
        if(isdir)
            rodent_save_workdir_history (widgets_p->workdir);
        else {
            rfm_threaded_show_text(widgets_p);
            rfm_threaded_diagnostics (widgets_p, "xffm/stock_dialog-error", NULL);
            rfm_threaded_status (widgets_p, "xffm_tag/stderr", g_strconcat(g, ": ", strerror (errno), "\n", NULL));
	    g_free(g);	    
            return NULL;
        }
        return g;
    }

    g_free(g);	    
    return NULL;
}

static int
select_all_view (view_t * view_p, gboolean invert) {
    rfm_global_t *rfm_global_p = rfm_global();
    population_t **population_pp;
    int items = 0;
    if(!view_p || !view_p->en)
        return items;
    rfm_cursor_wait (rfm_global_p->window);
    population_pp = view_p->population_pp;
    for(; population_pp && *population_pp; population_pp++) {
        if((*population_pp)->en == NULL)
            continue;
        if(IS_DUMMY_TYPE ((*population_pp)->en->type))
            continue;
        items++;
        NOOP ("selected item %d", items);
        //

	if (invert) {
	    if ((*population_pp)->flags & POPULATION_SELECTED) {
		//no gtk in this function:
		rfm_unselect_pixbuf (view_p, *population_pp);
	    } else {
		//no gtk in this function:
		rfm_select_pixbuf (view_p, *population_pp);
	    }
	    rfm_expose_item (view_p, *population_pp);
	} else {
	    if (!((*population_pp)->flags & POPULATION_SELECTED)){
		//no gtk in this function:
		rfm_select_pixbuf (view_p, *population_pp);
		//all expose issues are invoked in main context:
		rfm_expose_item (view_p, *population_pp);
	    }
	}

    }
    rfm_threaded_cursor_reset (rfm_global_p->window);
    return items;
}

enum {
    END_MATCH,
    START_MATCH,
    TOMATO_IN_SANDWICH_MATCH,
    BREAD_ON_SANDWICH_MATCH,
    EXACT_MATCH
};

static gint
select_byfilter_view (widgets_t *widgets_p, const gchar * select_filter, gboolean select_it) {
    rfm_global_t *rfm_global_p = rfm_global();

    view_t *view_p = widgets_p->view_p;
    population_t **population_pp;
    gint items = 0;
    if(!view_p || !view_p->en) return items;
    if(!select_filter || !strlen (select_filter)) return items;
    if(strcmp (select_filter, "*") == 0) return select_all_view (view_p, FALSE);
    rfm_threaded_cursor_wait (rfm_global_p->window);

    gchar *filter = g_strdup (select_filter);
    memset (filter, 0, strlen (select_filter));
    int j = 0,
        i = 0,
        caso;
    for(; i < strlen (select_filter); i++) {
        if(select_filter[i] != '*')
            filter[j++] = select_filter[i];
    }

    if(select_filter[0] == '*' && select_filter[strlen (select_filter) - 1] == '*') {
        caso = TOMATO_IN_SANDWICH_MATCH;
    } else if(select_filter[0] == '*') {
        caso = END_MATCH;
    } else if(select_filter[strlen (select_filter) - 1] == '*') {
        caso = START_MATCH;
    } else if(strchr (select_filter, (int)'*')) {
        caso = BREAD_ON_SANDWICH_MATCH;
    } else {
        caso = EXACT_MATCH;
    }

    //rodent_unselect_all_pixbuf (view_p);
    population_pp = view_p->population_pp;
    for(; population_pp && *population_pp; population_pp++) {
        gchar *f;
        gboolean match = FALSE;
        if((*population_pp)->en == NULL)
            continue;
        if((*population_pp)->en->path == NULL)
            continue;
        if(IS_DUMMY_TYPE ((*population_pp)->en->type))
            continue;
        f = g_path_get_basename ((*population_pp)->en->path);
        gchar *p = strcasestr (f, filter);

        if(p && caso == END_MATCH) {
            gchar *pp;
            do {
                pp = p;
                p = strcasestr (p + 1, filter);
            }
            while(p);
            p = pp;
        }

        if(p) {
            NOOP ("p=%s, filter=%s\n", p, filter);
            switch (caso) {
            case TOMATO_IN_SANDWICH_MATCH:
                match = TRUE;
                break;
            case END_MATCH:
                if(p[strlen (filter)] == 0)
                    match = TRUE;
                break;
            case START_MATCH:
                if(p == f)
                    match = TRUE;
                break;
            case EXACT_MATCH:
                if(strlen (p) == strlen (filter))
                    match = TRUE;
                break;
            case BREAD_ON_SANDWICH_MATCH:
                //not implemented. Need to do 2 matches.
                //break;
            default:           //START_END_MATCH
                match = TRUE;
                break;

            }
        }
        if(match) {
            items++;
            if (select_it) rfm_select_pixbuf (view_p, *population_pp);
	    else rfm_unselect_pixbuf (view_p, *population_pp);
	    rfm_expose_item (view_p, *population_pp);
        }
        g_free (f);
    }
    rfm_threaded_cursor_reset (rfm_global_p->window);
    gchar *plural_text=g_strdup_printf (ngettext ("%'u item", "%'u items",
                                               items),items);
    gchar *g = g_strdup_printf ("%s: %s", _("Selection"), plural_text);
    g_free(plural_text);

    rfm_threaded_status (&(view_p->widgets), "xffm/stock_dialog-info", g );
    g_free (filter);

    return items;
}

static void
unselect_all (widgets_t *widgets_p) {
    view_t *view_p = widgets_p->view_p;
    rodent_unselect_all_pixbuf (view_p);
}


static void
private_mount (widgets_t *widgets_p, gint mounted) {
    rfm_global_t *rfm_global_p = rfm_global();
    view_t *view_p = widgets_p->view_p;
    record_entry_t *en = view_p->selection_list->data;
    NOOP ("MOUNT: private_mount %d (0x%x)\n", mounted, GPOINTER_TO_INT(view_p));
    if(!en || !en->path) {
        DBG ("no entry to mount\n");
        return;
    }
    if(mounted){
        SET_MOUNTED_TYPE (en->type);
    } else{
        UNSET_MOUNTED_TYPE (en->type);
    }
    rfm_threaded_show_text(widgets_p);
    gchar *text;
    const gchar *icon;
    if (mounted){
	icon = "xffm/emblem_redball/compositeC/stock_go-up";
	text = g_strdup_printf(_("Unmounting %s"), en->path);
    } else {
	icon = "xffm/emblem_greenball/compositeC/stock_go-up";
	text = g_strdup_printf(_("Mounting %s"), en->path);
    }
    rfm_threaded_diagnostics(widgets_p, icon, g_strconcat(text, "\n", NULL));
    FSTAB_fstab_mount (widgets_p, en);
    g_free(text);


    unselect_all(widgets_p);
    rfm_threaded_cursor_reset (rfm_global_p->window);

    NOOP ("MOUNT: private_mount done\n");
    return;
}


static gint
gui_pasteboard_list (GList ** list_p) {

    // client side pasteboard, via MIT-shm
    gchar *b=rfm_get_paste_buffer();
    if((!b) || (!strlen (b))) {
      no_pasteboard:
	g_free(b);
	return 0;
    }
    gboolean cut;
    gchar *word;
    if((word = strtok (b, ":")) == NULL)
        goto no_pasteboard;
    if(!strstr (word, "#xfvalid_buffer"))
        goto no_pasteboard;
    if((word = strtok (NULL, ":")) == NULL)
        goto no_pasteboard;
    if(strstr (word, "cut"))
        cut = TRUE;
    else
        cut = FALSE;
    if((word = strtok (NULL, ":")) == NULL)
        goto no_pasteboard;
    //src_host = g_strdup(word);

    word = word + strlen (word) + 1;
    if(word[0] == '\n') {
        word++;
        if(word[0] == 0)
            goto no_pasteboard;
    } else {
        if((word = strtok (NULL, "\n")) == NULL)
            goto no_pasteboard;
        word = word + strlen (word) + 1;
    }

    /* create list to send to CreateTmpList */
    gint retval = rfm_uri_parse_list (word, list_p);
 
    g_free (b);
    if (retval) {
	if(cut) retval=1;
	else retval= 2;
    }
    return retval;
}

static gint
gui_pasteboard_transfer (widgets_t * widgets_p,
                         record_entry_t * t_en, 
			 GList * list, 
			 gboolean cut,
			 gboolean symlink) {
    if (!list){
        g_warning("gui_pasteboard_transfer() list is null\n");
        return 0;
    }
    gchar *url = (gchar *) list->data;
    if(!url){
	DBG("gui_pasteboard_transfer: !url\n");
        return 0;
    }
 
    if(t_en->module) {
        NOOP ("PASTE: en->module=%s\n", t_en->module);
        if(rfm_natural (PLUGIN_DIR, t_en->module, t_en, "valid_drop_site")) {
            NOOP ("module: valid_drop_site for %s\n", t_en->module);
            rfm_natural (PLUGIN_DIR, t_en->module, t_en, "set_drop_entry");
            if(rfm_rational (PLUGIN_DIR, t_en->module, list, widgets_p, "process_drop")) {
                NOOP ("module: process_drop ok\n");
                /*result=TRUE; */
            }
            rfm_void (PLUGIN_DIR, t_en->module, "clear_drop_entry");
            return 1;
        }
    }
    NOOP ("PASTE: must be local then\n");
    int mode;
    if(cut) {
        mode = TR_MOVE;
    } else {
        mode = TR_COPY;
    }
    if(symlink) {
        mode = TR_LINK;
    }
       
    // Only the uploading notice works here...
    gchar *text=NULL;
    const gchar *icon=NULL;
    //gint type=0;
    gboolean local_target = TRUE;
    //gboolean local_source = TRUE;
    //if (!IS_LOCAL_TYPE(type))local_source = FALSE;
    if (!IS_LOCAL_TYPE(t_en->type))local_target = FALSE;
    if (!local_target){
      switch (mode){
        case TR_COPY:
	case TR_MOVE:
        case TR_COPY_REMOTE:
	case TR_MOVE_REMOTE:
	    icon = "xffm/emblem_network/compositeSE/stock_go-forward";
	    text = g_strdup_printf(_("Uploading file %s"), "...");
	    break;
	default:
	    break;
      }
    } else {
	// Local target. Test first item of list for remote host.
	const gchar *path = list->data;
	record_entry_t *en = rfm_stat_entry(path, 0);
	if (!IS_LOCAL_TYPE(en->type)) {
	    icon = "xffm/emblem_network/compositeSE/stock_go-back";
	    text = g_strdup_printf(_("Downloading file %s..."), "");
	}
	rfm_destroy_entry(en);
   }


    
    if (text) {
	rfm_threaded_diagnostics(widgets_p, "xffm/emblem_network/compositeSE/stock_go-forward", NULL);
	rfm_threaded_diagnostics(widgets_p, icon, NULL);
	rfm_threaded_diagnostics(widgets_p, "xffm_tag/red", g_strconcat(text, "\n", NULL));
	g_free(text);
    }
    // Here we are already in a threaded environment, so plain cp
    // would create an unnecessary extra thread.
    // cp (mode, list, t_en->path);
    plain_cp(widgets_p, mode, list, t_en->path, FALSE);
    
    return 1;
}


static void
private_paste (widgets_t * widgets_p, view_t * view_p, gboolean symlink) {
    gboolean cut;
    GList *list = NULL;
    record_entry_t *t_en = NULL;
    int i;


    
    if((t_en = view_p->en) == NULL){
	DBG("(t_en = view_p->en) == NULL\n");
        return;
    }


    if((i = gui_pasteboard_list (&list)) == 0){
	DBG("(i = gui_pasteboard_list (&list)) == 0\n");
        return;
    }
    if(i == 1)
        cut = TRUE;
    else
        cut = FALSE;
    i = gui_pasteboard_transfer (widgets_p, t_en, list, cut, symlink);

    list = rfm_uri_free_list (list);

    if(i) {
        if(cut){
	    rfm_clear_paste_buffer();
	}	    
    }
    return;
}

static void
gui_pasteboard_copy_cut (widgets_t * widgets_p, gboolean cut, GSList ** paste_list) {
    gint len;
    gchar *buffer;
    //gchar *files;
    view_t *view_p = widgets_p->view_p;
    // clear any previous icon emblems
    rodent_clear_cut_icons (view_p);
    
    if(*paste_list == NULL){
	DBG("*paste_list == NULL\n");
        return;
    }

    if(cut)
        rfm_threaded_status (widgets_p, "xffm/stock_dialog-info", g_strdup(_("Cut")));
    else
        rfm_threaded_status (widgets_p, "xffm/stock_dialog-info", g_strdup(_("Copy")));

    rfm_clear_paste_buffer();

    len = 1 + strlen ("#xfvalid_buffer:copy:%%:\n");
    len += strlen (g_get_host_name ());
    GSList *tmp = *paste_list;
    for(; tmp; tmp = tmp->next) {
        gint addlen;
        record_entry_t *en = (record_entry_t *) tmp->data;
        addlen = 0;
        len += (1 + strlen (en->path) + addlen);
    }
    buffer = (gchar *)malloc (len * sizeof (char) + 1);
    if(!buffer) {
        DBG ("rfm: unable to allocate paste buffer\n");
        return;
    }
    sprintf (buffer, "#xfvalid_buffer:%s:%s:\n", (cut) ? "cut" : "copy", g_get_host_name ());
    //files = buffer + strlen (buffer);

    for(tmp = *paste_list; tmp; tmp = tmp->next) {
        record_entry_t *en = (record_entry_t *) tmp->data;
        {
            strcat (buffer, en->path);
            strcat (buffer, "\n");
        }
    }
    NOOP("dbg:len=%d,strlen=%d,data=%s\n",len,strlen(buffer),buffer); 
    rfm_store_paste_buffer(buffer, len);
    g_free (buffer);
    buffer = NULL;

    if(cut)
        rfm_threaded_status (widgets_p, "xffm/stock_dialog-info", g_strconcat(_("Cut"), NULL));
    else
        rfm_threaded_status (widgets_p, "xffm/stock_dialog-info", g_strconcat(_("Copy"), NULL));
    //view_p->flags.pasteboard_serial++;
    NOOP ("PASTE: view_p->flags.pasteboard_serial=%d\n", view_p->flags.pasteboard_serial);
    gchar *value = g_strdup_printf ("%d", view_p->flags.pasteboard_serial + 1);
    if (rfm_rational (RFM_MODULE_DIR, "settings", (void *)"RFM_PASTEBOARD_SERIAL", (void *)value, "mcs_set_var") == NULL){
        rfm_setenv ("RFM_PASTEBOARD_SERIAL", value, TRUE);
        NOOP("cannot set RFM_PASTEBOARD_SERIAL");
    }
    g_free (value);
    rodent_update_cut_icons(view_p);
    TRACE("copy-cut calling rodent_expose_all()\n");
    //rodent_expose_all (view_p);
    rodent_redraw_items (view_p);
}

static void
copy_cut_callback (widgets_t *widgets_p, gboolean cut) {

    view_t *view_p = widgets_p->view_p;
    if (!rfm_entry_available(widgets_p, view_p->en)) return; 
    if(!view_p->selection_list) return;

    gui_pasteboard_copy_cut (widgets_p, cut, &(view_p->selection_list));
    // unselect stuff if it is cut or copied.
    GSList *tmp=view_p->selection_list;
    for (;tmp && tmp->data; tmp=tmp->next){
	record_entry_t *en=tmp->data;
	rfm_destroy_entry(en);
    }
    g_slist_free (view_p->selection_list);
    view_p->selection_list = NULL;

    // Mark all population items as unselected.
    if(rfm_population_read_lock (view_p, "copy_cut_callback")) {
        population_t **pp;
        for(pp = view_p->population_pp; pp && *pp; pp++) {
            population_t *population_p = *pp;
            population_p->flags  &= (POPULATION_SELECTED ^ 0xffffffff);
        }
        rfm_population_read_unlock (view_p, "copy_cut_callback");
    }

}
static void
paste_callback (widgets_t *widgets_p, gboolean symlink) {
    view_t *view_p = widgets_p->view_p;
    if(!view_p->en)
        return;
    if (!rfm_entry_available(widgets_p, view_p->en)) return; 
    //rfm_details->pastepath = view_p->en->path;
    private_paste (widgets_p, view_p, symlink);
}


///////////////////////////////////////////////////////////////////////////
///******************************************************************///
//////////////////////////////////////////////////////////////////////////

static void
glob_x (widgets_t *widgets_p){
    NOOP( "glob_x...\n");
    gchar *fgr = g_find_program_in_path("rodent-fgr");
    if (!fgr) {
	DBG("fgr not found.\n");
        return;
    }
    
    view_t *view_p = widgets_p->view_p;
    gchar *path = NULL;
    
    // on keybind call, mouse_event is ignored, selection_p is
    // set to NULL in keybind.
    if(view_p->mouse_event.selected_p && view_p->mouse_event.selected_p->en 
	    && IS_SDIR(view_p->mouse_event.selected_p->en->type)
       && g_slist_length (view_p->selection_list) == 1) {
	path = g_strdup(view_p->mouse_event.selected_p->en->path);
    } else if(view_p->en) {
	path = g_strdup(view_p->en->path);
    }
    gchar *argv[]={fgr, path, NULL};
    rfm_threaded_show_text(widgets_p);
    rfm_thread_run_argv(widgets_p, argv, FALSE);
    g_free(fgr);
    g_free(path);
}

static void
help (widgets_t *widgets_p){
    //view_t *view_p = widgets_p->view_p;
    // XXX: should define PDF_DIR and use ./configure --pdfdir=xxx
    gchar *help_file=g_strdup_printf("%s/doc/rfm/RTFM.pdf", 
	    PACKAGE_DATA_DIR);
    // This is a local and absolute path.
    if (!g_file_test(help_file, G_FILE_TEST_EXISTS)){
	DBG("%s: %s\n", help_file, strerror(ENOENT));
    } else {
	record_entry_t *en=rfm_stat_entry(help_file, 0);
	en->mimetype=MIME_type(en->path, NULL);
	open_with (widgets_p, en);
	rfm_destroy_entry(en);
    }
    g_free(help_file);
}
static void
remove_x (widgets_t *widgets_p){
    view_t *view_p = widgets_p->view_p;
    if (view_p && g_slist_length(view_p->selection_list) >= 1){
	GSList *list = NULL;
	GSList *tmp=view_p->selection_list;
	for (;tmp && tmp->data; tmp=tmp->next) {
	    record_entry_t *en=tmp->data;
	    list = g_slist_prepend(list, g_strdup (en->path));
	    list = g_slist_reverse(list);
	}
	rm(widgets_p, list);
	// unselect all items now so they won't remain in selection
	// list when the thread is done.
        TRACE("** rm call has returned\n");
	rodent_unselect_all_pixbuf (view_p);	  
    }
}

static void
refresh (widgets_t *widgets_p){
    NOOP(stderr, "refresh...\n");
    view_t *view_p = widgets_p->view_p;
    if(view_p->en){
	if (view_p->en->path) {
	    gchar *path = g_strdup( view_p->en->path);
	    rfm_cleanup_thumbnails(path);
	}
	if (view_p->en->module &&
	    rfm_natural(PLUGIN_DIR, view_p->en->module, view_p, "reload")) {
		return;
	}
    }
    record_entry_t *en = rfm_copy_entry (view_p->en);
    if (!rodent_refresh (widgets_p, en)){ rfm_destroy_entry(en); }
}

static void
host (widgets_t *widgets_p) {
    view_t *view_p = widgets_p->view_p;
    if(view_p->child_constructor) {
        (*view_p->child_constructor) (widgets_p, NULL);
    } else {
        rodent_push_view_go_history ();
        if (!rodent_refresh (widgets_p, NULL)){
            g_warning("!rodent_refresh (widgets_p, NULL)\n");
        }
    }
}

static void
goup (widgets_t *widgets_p) {
    view_t *view_p = widgets_p->view_p;
    record_entry_t *en = NULL;
    if (view_p->population_pp && view_p->population_pp[0]){
	if (view_p->population_pp[0]->en && 
		view_p->population_pp[0]->en->module) {
	    en =rfm_copy_entry(view_p->population_pp[0]->en);
	} else if (view_p->population_pp[0]->en) {
	    gchar *path=g_strdup(view_p->population_pp[0]->en->path);
	    if (g_path_is_absolute(path)){
		while (!rfm_g_file_test_with_wait(path, G_FILE_TEST_IS_DIR)){
		    gchar *d = g_path_get_dirname(path);
		    g_free(path);
		    path = d;
		}
	    } else {
		g_error("goup_activate(): this should not happen\n");
	    }
	    en =rfm_stat_entry(path, 0);
	    g_free(path);
	}
    } 
    if (!en) {
	host (widgets_p);
	return;
    }
    if (IS_SDIR(en->type)){
        rodent_push_view_go_history ();
    }
    if(view_p->child_constructor) {
	if (en->module) {
	  gchar *m=g_strdup_printf("rodent-plug %s", en->module);
          (*view_p->child_constructor) (widgets_p, (char *)m);
	  g_free(m);
	} else {
          (*view_p->child_constructor) (widgets_p, en->path);
        }
        rfm_destroy_entry(en);
    } else {
	if (IS_SDIR(en->type)){
	    rodent_push_view_go_history ();
	}
	if (!rodent_refresh (widgets_p, en)){ 
            rfm_destroy_entry(en);
        }
    }
}


static void
forward (widgets_t *widgets_p){
    view_t *view_p = widgets_p->view_p;
    if (!view_p->f_list) {
	rfm_threaded_show_text(widgets_p);
	rfm_threaded_diagnostics(widgets_p, "xffm/stock_dialog-info", NULL);
	rfm_threaded_diagnostics(widgets_p, "xffm_tag/blue",
		g_strconcat(_("Empty history"), "\n", NULL)); 
	jump_to(widgets_p);
	return;
    }
    record_entry_t *en = view_p->f_list->data;
    view_p->f_list = g_slist_remove(view_p->f_list, en);
    // if we reach the end, push history (hack)
    rodent_push_view_go_history();
    if(en) view_p->module = en->module;
    else view_p->module = NULL;
    TRACE( "forward() calling rodent_full_reload_view\n");
    rodent_full_reload_view ((gpointer) view_p, en);
}

static void
back (widgets_t *widgets_p){
    view_t *view_p = widgets_p->view_p;
    GList *last = g_list_last (view_p->go_list);
    if(!last) {
	// This will only occur on keybinding callback.
	rfm_threaded_show_text(widgets_p);
	rfm_threaded_diagnostics(widgets_p, "xffm/stock_dialog-info", NULL);
	rfm_threaded_diagnostics(widgets_p, "xffm_tag/blue",
		g_strconcat(_("Initial Point"),
		": ", (view_p->en)?view_p->en->path:_("Plugins"), "\n", NULL)); 
	return;
    }
    record_entry_t *en = last->data;
    // Failure warning instead of going to previous back.
    if (!rfm_entry_available(widgets_p, en)) {
	pop_view_go_history (view_p);
	return; 
    }
    view_p->f_list = g_slist_prepend(view_p->f_list, rfm_copy_entry(view_p->en));
    if(en) view_p->module = en->module;
    else view_p->module = NULL;
    pop_view_go_history (view_p);
    TRACE( "back() calling rodent_full_reload_view\n");
    rodent_full_reload_view ((gpointer) view_p, en);
}

static void
home (widgets_t *widgets_p){
    view_t *view_p = widgets_p->view_p;
    if (!rfm_g_file_test_with_wait (g_get_home_dir(), G_FILE_TEST_IS_DIR)){
	 time_out_message(widgets_p, g_get_home_dir());
	 return; 
    }
    if(view_p->child_constructor) {
	(*view_p->child_constructor) (widgets_p, (char *)g_get_home_dir ());
    } else {
	rodent_push_view_go_history ();
	record_entry_t *en = rfm_stat_entry ((gchar *) g_get_home_dir (), 0);
	if (!rodent_refresh (widgets_p, en)){ rfm_destroy_entry(en); }
    }	
}

static void
jump_to (widgets_t *widgets_p){
    view_t *view_p = widgets_p->view_p;
    gchar *g = get_jumpto_dir(widgets_p);
    if (!g) return;
    if(view_p->child_constructor) {
	(*view_p->child_constructor) (widgets_p, g);
    } else {
	rodent_push_view_go_history ();
	rfm_save_to_go_history (g);
	record_entry_t *en = rfm_stat_entry (g, 0);
	if (!rodent_refresh (widgets_p, en)){ rfm_destroy_entry(en); }
    }
    g_free (g);
}

static void
plugin_goto (widgets_t *widgets_p, gpointer data) {
    view_t *view_p = widgets_p->view_p;
    GtkWidget *menu_item = data;
    const gchar *module_name = g_object_get_data(G_OBJECT(menu_item), "module_name");
    if(module_name) {
      if (view_p->child_constructor) {
	gchar *m = g_strconcat("rodent-plug"," ", module_name, NULL);
        (*view_p->child_constructor) (widgets_p, (char *)m);
	g_free(m);
      } else {
        rodent_push_view_go_history ();
        record_entry_t *new_en = rfm_mk_entry (0);
        SET_ROOT_TYPE (new_en->type);
        new_en->module = module_name;
        new_en->path = rfm_void (PLUGIN_DIR, module_name, "module_label");
	if (!rodent_refresh (widgets_p, new_en)){ rfm_destroy_entry(new_en); }
      }
    } else {
        DBG ("cannot get module_name from module-goto menu\n");
    }
    return;
}

// This function uses menuitem and does not accept keybinding...
static void
level_goto (widgets_t *widgets_p, gpointer data) {
    view_t *view_p = widgets_p->view_p;
    GtkWidget *menu_item = data;
    const gchar *path = g_object_get_data(G_OBJECT(menu_item), "path");
    NOOP("path is %s\n", path);
    if(path) {
        if(view_p->child_constructor) {
            (*view_p->child_constructor) (widgets_p, (char *)path);
        } else {
            rodent_push_view_go_history ();
	    record_entry_t *new_en;
	    if (rfm_g_file_test_with_wait(path, G_FILE_TEST_EXISTS)){
		     new_en = rfm_stat_entry (path, 0);
	    } else {
		rfm_confirm (widgets_p, GTK_MESSAGE_ERROR, strerror (EEXIST), NULL, NULL);
		return;
	    }   
	    if (new_en && !new_en->module) {
		rfm_save_to_go_history ((gchar *) path);
	    }
	    if (!rodent_refresh (widgets_p, new_en)){ 
		rfm_destroy_entry(new_en); 
	    }
	    
        }
    } else {
        DBG ("cannot get path from level-goto menu\n");
    }
    return;
}


static void
ascending_descending (widgets_t *widgets_p, enum menu_enum_t menu_enum){
    view_t *view_p = widgets_p->view_p;
	
    if (view_p->flags.preferences & SORT_ASCENDING) {
	view_p->flags.preferences &= (SORT_ASCENDING ^ 0xffffffff);
    } else {	
	view_p->flags.preferences |= SORT_ASCENDING;
    }
#if 0
    if (menu_enum == ASCENDING_ACTIVATE) {
	if (view_p->flags.preferences & SORT_ASCENDING) return;
	view_p->flags.preferences |= SORT_ASCENDING;
    } else {
	if ((view_p->flags.preferences & SORT_ASCENDING) == 0) return;
	view_p->flags.preferences &= (SORT_ASCENDING ^ 0xffffffff);
    }
#endif
    rfm_save_view_preferences (view_p, view_p->en);
    record_entry_t *en = rfm_copy_entry (view_p->en);
    if (!rodent_refresh (widgets_p, en)){ rfm_destroy_entry(en); }
}

static void
about(widgets_t *widgets_p){
    gchar *argv[3];
    argv[0] = "rodent";
    argv[1] = "--version";
    argv[2] = NULL;


    rfm_threaded_show_text (widgets_p);
    rfm_thread_run_argv (widgets_p, argv, FALSE);
    // pipe output of current app...
    gchar *app = g_strdup("rodent-app");
    rfm_global_t *rfm_global_p = rfm_global();
    if (rfm_global_p){
	gchar *c = g_strdup_printf("%s --version", rfm_global_p->argv[0]);
	FILE *p = popen(c, "r");
	gchar buffer[256];
	memset(buffer, 0, 256);
	if (p && fgets(buffer, 255, p)){
	    g_free(app);
	    if (strchr(buffer,'\n')) *strchr(buffer,'\n') = 0;
	    app = g_strdup(buffer);
	}
	if (p) pclose(p);
    } 

    void *arg[]={widgets_p, app};
    rfm_context_function(about_dialog_f, arg);
    g_free(app);
}

static void
newdir (widgets_t *widgets_p) {
    view_t *view_p = widgets_p->view_p;
    record_entry_t *en =  view_p->en;
    gchar *label = g_strdup_printf ("%s/", en->path);
    gchar *q = rfm_utf_string (rfm_chop_excess (label));
    gchar *g = get_response (_("Create New Folder"), q,
                                          _("Name"));
    g_free (label);
    g_free (q);

    if(g && strlen (g)) {
      gchar *p = g_build_filename (en->path, g, NULL);
      // 
      if(!rfm_locate_path(view_p, p))
      {
        gchar *argv[4];
        int j=0;
        argv[j++]="mkdir";
        argv[j++]=p;
        argv[j++]=NULL;

	if (en->st->st_mode == 0) {
	    DBG("This should not happen: en->st->st_mode == 0\n");
	    g_free(g);
	    return;
	}	

	if (rfm_write_ok_path(en->path)){
	    rfm_thread_run_argv (widgets_p, argv, FALSE);
	} else {
	    gchar *base=g_path_get_basename(p);
	    gchar *operation=g_strconcat("mkdir ", base, NULL);
	    g_free(base);
	    if (confirm_sudo(widgets_p, en->path, 
		    _("write failed"), operation)){
		RFM_TRY_SUDO (widgets_p, argv, FALSE);
		rfm_threaded_show_text(widgets_p);
		rfm_threaded_diagnostics(widgets_p, "xffm/stock_properties",NULL);
		rfm_threaded_diagnostics(widgets_p, "xffm_tag/green",
			g_strconcat(_("Don't forget"), " ", NULL));
		rfm_threaded_diagnostics(widgets_p, "xffm_tag/magenta",
		    g_strconcat("chown pid:gid", " ", p, "\n", NULL));
	    }
	    g_free(operation);
	}
      }
      else {
            rfm_confirm (widgets_p, GTK_MESSAGE_ERROR, strerror (EEXIST), NULL, NULL);
      }
      g_free(p);
    }
    g_free(g);
}

static void
newfile (widgets_t *widgets_p) {
    view_t *view_p = widgets_p->view_p;
    record_entry_t *en = view_p->en;
    gchar *label = g_strdup_printf ("%s/", en->path);
    gchar *q = rfm_utf_string (rfm_chop_excess (label));
    gchar *g = get_response ( _("New file"), q, _("Name"));
    g_free (q);
    g_free (label);
    if(g && strlen (g)) {
        gchar *p = g_build_filename (en->path, g, NULL);
        if(!rfm_locate_path(view_p, p)) {
	    gchar *argv[3];
	    int j=0;
	    argv[j++]="touch";
	    argv[j++]=p;
	    argv[j++]=NULL;
	    

	    if (rfm_write_ok_path(en->path)){
		rfm_thread_run_argv (widgets_p, argv, FALSE);
	    } else {
		gchar *base=g_path_get_basename(p);
		gchar *operation=g_strconcat("touch ", base, NULL);
		g_free(base);
		if (confirm_sudo(widgets_p, en->path, 
			_("write failed"), operation)){
		    RFM_TRY_SUDO (widgets_p, argv, FALSE);
		    rfm_threaded_show_text(widgets_p);
		    rfm_threaded_diagnostics(widgets_p, "xffm/stock_properties",NULL);
		    rfm_threaded_diagnostics(widgets_p, "xffm_tag/green",
			    g_strconcat(_("Don't forget"), " ", NULL));
		    rfm_threaded_diagnostics(widgets_p, "xffm_tag/magenta",
			g_strconcat("chown pid:gid", " ", p, "\n", NULL));
		}
		g_free(operation);
	    }
         } else {
            rfm_confirm (widgets_p, GTK_MESSAGE_ERROR, strerror (EEXIST), NULL, NULL);
        }
        g_free (p);
    }
    g_free(g);
}
 
static void 
run(widgets_t *widgets_p){
    gboolean visible=rfm_threaded_diagnostics_is_visible(widgets_p);
    if (!visible) rfm_threaded_show_text(widgets_p);
    if (!execute (widgets_p, NULL)){
	if (!visible && widgets_p->diagnostics_window==NULL){
	    rfm_threaded_hide_text(widgets_p);
	}
    }
}

static void 
bcrypt(widgets_t *widgets_p){
    // this will be an internal function call, rodent-bcrypt
    view_t *view_p = widgets_p->view_p;
    GSList *list=NULL;
    GSList *s_list = view_p->selection_list;
    if (g_slist_length(s_list) > MAX_COMMAND_ARGS - 5){
	DBG("Argument list too long (%d > %d)\n", 
	    g_slist_length(s_list), MAX_COMMAND_ARGS - 5);
	return;
    }
    for (; s_list && s_list->data; s_list = s_list->next){
	record_entry_t *en = s_list->data;
	if (en && g_path_is_absolute(en->path))
	    list = g_slist_append(list, g_strdup(en->path)); 
    }
    if (!rfm_natural(RFM_MODULE_DIR, "bcrypt", list, "bcrypt_dialog")){
	DBG("cannot load bcrypt plugin\n");
    }
    for (s_list = list; s_list && s_list->data; s_list = s_list->next){
	g_free(s_list->data);
    }
    g_slist_free(list);
}

static void 
open_x(widgets_t *widgets_p){
    view_t *view_p = widgets_p->view_p;
    GSList *list=NULL;
    GSList *s_list = view_p->selection_list;
    for (; s_list && s_list->data; s_list = s_list->next){
	record_entry_t *in_en = s_list->data;
	record_entry_t *out_en = rfm_copy_entry(in_en);
	list = g_slist_append(list, out_en); 
    }
    execute(widgets_p, list);
}

static void 
new_window(widgets_t *widgets_p){
    view_t *view_p = widgets_p->view_p;
    gchar *new_path=NULL;
    if (view_p->module) {
	new_path=g_strconcat("rodent-plug", " ", view_p->module, NULL);
    } else if (view_p->en) {
	 if (!rfm_g_file_test_with_wait(view_p->en->path, G_FILE_TEST_IS_DIR)){
	     time_out_message(widgets_p, view_p->en->path);
	     return; 
	 }
	new_path=g_strdup(view_p->en->path);
    }	
    rodent_new_gridview(widgets_p, new_path);
    g_free(new_path);
}
static void 
open_in_terminal(widgets_t *widgets_p){
    view_t *view_p = widgets_p->view_p;
    gchar *shell = rfm_xterm_shell();
    if (!shell) return;
    if(!view_p->en 
	    || !view_p->en->path 
	    || !g_path_is_absolute(view_p->en->path)){
	g_free (widgets_p->workdir);
	widgets_p->workdir = g_strdup (g_get_home_dir ());
    } else { 
	g_free (widgets_p->workdir);
	widgets_p->workdir = g_strdup (view_p->en->path);
    }

     gchar *argv[] = { shell, NULL };
    // use argv version so that it won't go into history file...
    rfm_thread_run_argv (widgets_p, argv, TRUE);
    g_free(shell);
}
static void 
differences(widgets_t *widgets_p){
    view_t *view_p = widgets_p->view_p;
    gchar *file1 = NULL;
    gchar *file2 = NULL;
    if(view_p->selection_list) {
	file1 = ((record_entry_t *) (view_p->selection_list->data))->path;
	if(g_slist_length (view_p->selection_list) > 1){
	    file2 = ((record_entry_t *) (view_p->selection_list->next->data))->path;
	}
    }
    if (file1 && !rfm_g_file_test_with_wait(file1, G_FILE_TEST_EXISTS)) {
	 time_out_message(widgets_p, file1);
	 return;
    }
    if (file2 && !rfm_g_file_test_with_wait(file2, G_FILE_TEST_EXISTS)) {
	 time_out_message(widgets_p, file2);
	 return;
    }
    g_free (widgets_p->workdir);
    widgets_p->workdir = g_strdup (g_get_home_dir ());
    
    gchar *argv[4];
    int i = 0;
    argv[i++] = "rodent-diff";
    if(file1)
        argv[i++] = file1;
    if(file2)
        argv[i++] = file2;
    argv[i] = NULL;

    rfm_thread_run_argv (widgets_p, argv, FALSE);
}
static void 
settings(widgets_t *widgets_p){
    if(rfm_void (RFM_MODULE_DIR, "settings", "module_active")) {
	rfm_void (RFM_MODULE_DIR, "settings", "run_rfm_settings_dialog");
    } else {
	DBG ("rodent_settings_activate(): cannot load libsettings\n");
    }
}

static void *
new_tab(gpointer data){
    void **arg = data;
    view_t *view_p = arg[0];
    gchar *argv = arg[1];
    g_free(arg);
    widgets_t *widgets_p = &(view_p->widgets);
    if (view_p->tab_constructor) (*(view_p->tab_constructor))(widgets_p, argv);
    g_free(argv);
    return NULL;
}

static void *
new_tab_f(void *data){
    if (rfm_get_gtk_thread() != g_thread_self()){
	rfm_context_function(new_tab_f, data);
	return NULL;
    }
    widgets_t *widgets_p = data;
    view_t *view_p = widgets_p->view_p;
    if (!view_p->tab_constructor) return NULL;
    gchar *path = NULL;
    if (view_p->en && !view_p->en->module) path = view_p->en->path;
    (*(view_p->tab_constructor))(widgets_p, path);
    return NULL;
}


    


// This function uses menuitem and does not accept keybinding...
// autotype C is "open with" operator which is determined by
// means of the command associated data item of the widget
static void
autotype (widgets_t *widgets_p, gpointer menuitem) {
    if (!menuitem) return;
    view_t *view_p = widgets_p->view_p;
    gchar *new_command=NULL;
    record_entry_t *en = view_p->selection_list->data;
    if (!en->path || !strlen(en->path))return;

    const gchar *output_arg = g_object_get_data (G_OBJECT (menuitem), "output_arg");
    const gchar *command = g_object_get_data (G_OBJECT (menuitem), "command");
    const gchar *dirname = g_object_get_data (G_OBJECT (menuitem), "workdir");
    const gchar *querypath = g_object_get_data (G_OBJECT (menuitem), "querypath");
    const gchar *output_ext = g_object_get_data (G_OBJECT (menuitem), "output_ext");

    // Assign command as default command here, if applicable.
    if (g_object_get_data(G_OBJECT(menuitem), "CTL_SET")) {
	NOOP("CTL is set\n");
	// must substitute path in command line for %s
	gchar *command_fmt=g_strdup(command);
	command_fmt=strip_path(command_fmt,en->path);

	if(!en->mimetype || strcmp(en->mimetype, _("unknown"))==0) {
	    if (IS_LOCAL_TYPE(en->type) && !en->mimemagic) {
		en->mimemagic = rfm_rational(RFM_MODULE_DIR, "mime", en, "mime_magic", "mime_function");
		if (!en->mimemagic) en->mimemagic = g_strdup(_("unknown"));
	    } else {
		en->mimemagic = g_strdup(_("unknown"));
	    }
	}    
	if(en->mimetype && strcmp(en->mimetype, _("unknown"))!= 0) {
	    NOOP (stderr, "OPEN: adding %s --> %s \n", view_p->mouse_event.selected_p->en->mimetype, command_fmt);
	    // This sets it as default command for the specified mimetype
	    MIME_add (en->mimetype, command_fmt);
	}
	g_free(command_fmt);
    }




    gint argc;
    gchar **argv;
    GError *error=NULL;

    if(!g_shell_parse_argv (command, &argc, &argv, &error)) {
        DBG("autotype_activate(%s): %s\n", command, error->message);
        g_error_free (error);
        g_strfreev (argv);
        NOOP ("!g_shell_parse_argv at popup_autostuff\n");
        return;
    }

// internal commands:
    if (strcmp(argv[0], "rodent-newtab")==0){
        if(view_p->tab_constructor) {
	    void **arg = (void **)malloc(2*sizeof(void*));
	    if (!arg) g_error("malloc: %s\n", strerror(errno));
	    arg[0] = view_p;
	    arg[1] = g_strdup(argv[1]);
	    // Wait for this:
	    rfm_context_function(new_tab, arg);
        }
        return;
    }
    if (strcmp(argv[0], "rodent-bcrypt")==0){
	bcrypt(widgets_p);
        return;
    }
    if (strcmp(argv[0], "rodent-newwin")==0){
	rodent_new_gridview(widgets_p, argv[1]);
        return;
    }
    // We don't use this any more:
    g_strfreev (argv);

    gchar *workdir=NULL;
    if(querypath) {
        workdir=autofunction_workdir (widgets_p, querypath);
        if (!workdir) return;
    }

    if (dirname) {// XXX I wonder what this is for?
        g_free (widgets_p->workdir);
        widgets_p->workdir = g_strdup (dirname);
    }
    if(output_ext) {
        gchar *basename = g_path_get_basename (output_arg);
        gchar *output_path=g_strconcat("\"",workdir, G_DIR_SEPARATOR_S, basename, output_ext,"\"",NULL);
        gchar *esc_path = g_strconcat("\"",basename,"\"",NULL);
        g_free(basename);
        basename=esc_path;

        gchar *s=strstr(command,"%s");
        if (s) {
        // XXX this should have at least two %s tokens...
            new_command = g_strdup_printf(command, output_path, basename);
        } else {
                NOOP("no %%s in command");
            new_command = g_strconcat (command, " ", output_path, " ", basename, NULL);
        }
        command = (const gchar *)new_command;
        g_free (output_path);
        g_free (basename);
    } else if (workdir) {
        g_free (widgets_p->workdir);
        widgets_p->workdir = g_strdup (workdir); 
    }
    NOOP ("output_arg=%s, command=%s, querypath(string)=%s, output_ext=%s, dirname=%s workdir=%s\n", output_arg, command, querypath,
           output_ext, dirname, widgets_p->workdir);

    rfm_threaded_show_text(widgets_p);
    RFM_THREAD_RUN2ARGV (widgets_p, (void *)command, FALSE);
        
    g_free (new_command);
    g_free (workdir);


}

// This function is for executables and dotdesktop types. 
static void
autotype_r (widgets_t *widgets_p, gpointer menuitem) {
    if (!menuitem) return;
    record_entry_t *en = g_object_get_data(G_OBJECT(menuitem), "record_entry");	    
    if (en) {
	if (en->mimetype && 
	    strcmp(en->mimetype, "application/x-desktop")==0 && 
	    rfm_rational(PLUGIN_DIR, "dotdesktop", widgets_p, en,  "double_click")) 
	{
	    return;
	}

	if (en->path && IS_EXE_TYPE(en->type))
	{
	    NOOP("executing %s\n", en->path);
	    g_free(widgets_p->workdir);
	    widgets_p->workdir=g_path_get_dirname(en->path);
	    if (!rfm_g_file_test_with_wait(widgets_p->workdir, G_FILE_TEST_EXISTS)){
		time_out_message(widgets_p, widgets_p->workdir);
		g_free(widgets_p->workdir);
		widgets_p->workdir = g_strdup(g_get_home_dir());
	    } else {
		gchar *argv[2]={en->path, NULL};
		rfm_thread_run_argv(widgets_p, argv, TRUE);
	    }
	}
    } 
    return;
}

static void
rename_x (widgets_t *widgets_p) {
    view_t *view_p = widgets_p->view_p;
    void **arg = (void **) malloc(3*sizeof(void *));
    if (!arg) g_error("malloc: %s\n", strerror(errno));
    arg[0] = view_p;
    arg[1] = (void *)(view_p->mouse_event.selected_p);
    arg[2] = GINT_TO_POINTER(RENAME_CASO);
    rfm_context_function(mk_rename_entry, arg); 
}

static void
duplicate (widgets_t *widgets_p) {
    view_t *view_p = widgets_p->view_p;
    void **arg = (void **) malloc(3*sizeof(void *));
    if (!arg) g_error("malloc: %s\n", strerror(errno));
    arg[0] = view_p;
    arg[1] = (void *)(view_p->mouse_event.selected_p);
    arg[2] = GINT_TO_POINTER(DUPLICATE_CASO);
    rfm_context_function(mk_rename_entry, arg); 
}

static void
symlink_x (widgets_t *widgets_p) {
    view_t *view_p = widgets_p->view_p;
    void **arg = (void **) malloc(3*sizeof(void *));
    if (!arg) g_error("malloc: %s\n", strerror(errno));
    arg[0] = view_p;
    arg[1] = (void *)(view_p->mouse_event.selected_p);
    arg[2] = GINT_TO_POINTER(SYMLINK_CASO);
    rfm_context_function(mk_rename_entry, arg); 
}


static void
touch (widgets_t *widgets_p) {
    view_t *view_p = widgets_p->view_p;
    GSList *list = NULL;
    GSList *tmp = view_p->selection_list;
    for (;tmp && tmp->data; tmp=tmp->next) {
	record_entry_t *en=tmp->data;
	list = g_slist_append(list, g_strdup (en->path));
    }
    void **argv = (void **)malloc(2*sizeof(void *));
    argv[0] = widgets_p;
    argv[1] = list;
    rfm_context_function(touch_dialog, argv);
    return;
}

static void
properties (widgets_t *widgets_p) {
    view_t *view_p = widgets_p->view_p;
    if(view_p->module && 
	    rfm_rational (PLUGIN_DIR, view_p->module,
		widgets_p,  view_p->selection_list, "do_properties")
	    ) {
	// module has done the properties thing here;
    } else {
	rfm_natural(RFM_MODULE_DIR, "properties", widgets_p, "do_prop");
    }
}


static void
select_all (widgets_t *widgets_p, gpointer user_data) {
    view_t *view_p = widgets_p->view_p;
    rfm_status (&(view_p->widgets), "xffm/stock_dialog-info", _("Select All"), "...", NULL);
    select_all_view (view_p, GPOINTER_TO_INT(user_data));
    gchar *plural_text=g_strdup_printf (
	ngettext ("%'u item", "%'u items", g_slist_length(view_p->selection_list)),
	g_slist_length(view_p->selection_list));
    gchar *g = g_strdup_printf ("%s: %s", _("Selection"), plural_text);
    g_free(plural_text);
    rfm_threaded_status (&(view_p->widgets), "xffm/stock_dialog-info", g);
}

static void
select_by_filter (widgets_t *widgets_p, gpointer user_data) {
    gboolean select_it = GPOINTER_TO_INT(user_data);
    gchar *filter_s;
    view_t *view_p = widgets_p->view_p;
    gchar *b = g_strdup_printf ("%s/", view_p->en->path);
    gchar *label = rfm_utf_string (rfm_chop_excess (b));
    filter_s = get_response ( (select_it)?_("Select Items Matching..."):
	    _("Unselect Items Matching..."), label, "*");
    g_free (label);
    g_free (b);
    if(filter_s && strlen (filter_s)) {
        select_byfilter_view (widgets_p, filter_s, select_it);
    }
    g_free(filter_s);
}


static void
mount (widgets_t *widgets_p) {
    private_mount (widgets_p, FALSE);
}

static void
unmount (widgets_t *widgets_p) {
    private_mount (widgets_p, TRUE);
}

static void *close_x (gpointer data);


static void *
quit (void *data) {
    //return NULL;
    rfm_global_t *rfm_global_p = rfm_global();
    g_mutex_lock(rfm_global_p->status_mutex);
    //g_error("wuit...\n");
    TRACE("quit(): setting status to STATUS_EXIT\n");
    rfm_global_p->status = STATUS_EXIT;
    g_mutex_unlock(rfm_global_p->status_mutex);

#ifndef VERIFY_LEAKS
    TRACE("ZZZ quit(): killing all controller children now...\n");
    rfm_killall_children();
    // This is quick exit, without memory leak check cleanup
    TRACE("ZZZ quit(): exit now.\n");
    exit(1);
#else
    TRACE("VERIFY_LEAKS is set on exit...\n");
    // This way the janitor exits the main loop and the main program
    // will do a full cleanup to verify memory leaks. 
    widgets_t *widgets_p = data;
    TRACE( "ZZZ quit callback... thread 0x%x\n",GPOINTER_TO_INT(g_thread_self()));
    if (rfm_get_gtk_thread() != g_thread_self()){
        rfm_context_function(quit, widgets_p);
	return NULL;
    }
    TRACE( "ZZZ quit callback proceed ... thread 0x%x\n",GPOINTER_TO_INT(g_thread_self()));

    g_cond_signal(rfm_global_p->janitor_signal);
    gtk_widget_hide(rfm_global_p->window);
    gdk_flush();
#endif
    return NULL;
}

static void *
destructor(void *data){
    view_t *view_p = data;
    (*(view_p->tab_destructor))(view_p);
    return NULL;
}

static void *
close_x (gpointer data) {
    TRACE("ZZZ close_x callback...\n");
    widgets_t *widgets_p = data; 
    rfm_global_t *rfm_global_p = rfm_global();
    if (!widgets_p){
	DBG("close_x(): widgets_p==NULL\n");
	return NULL;
    }

    if (strstr(rfm_global_p->argv[0], "rodent-desk")){
	quit(widgets_p);
	return NULL;
    }
    
    GSList **list_p = rfm_view_list_lock(NULL, "close_x");
    gint active_views = g_slist_length(*list_p);
    rfm_view_list_unlock("close_x");
    if(active_views == 1) quit(widgets_p);
    // Gridview...
    view_t *view_p = widgets_p->view_p;
    if (!view_p) return FALSE;
    TRACE("ZZZ calling destructor...\n");
    if (view_p->tab_destructor) destructor(view_p);
    return NULL;
}


// This function uses menuitem and does not accept keybinding...
static void
bookmark_add (widgets_t *widgets_p, gpointer menuitem) {
    view_t * view_p = widgets_p->view_p;
    const gchar *path = NULL;
    if (menuitem) path = g_object_get_data(G_OBJECT(menuitem), "path");
    else if (view_p->en) path = view_p->en->path;
    if (!path) return;
    if (!g_path_is_absolute(path)){
     return; 
    }
    if (rodent_bookmarks_add(path)) {
	unselect_all (widgets_p);
	if (menuitem){
	    GdkRectangle *rect=g_object_get_data(G_OBJECT(menuitem), "rect");
	    if (rect) rfm_expose_rect (view_p, rect);
	}
	rodent_set_view_icon (view_p); // this is sent to main context.
	rfm_threaded_diagnostics(widgets_p, "xffm/stock_dialog-info",NULL);
	rfm_threaded_diagnostics(widgets_p, "xffm_tag/blue", g_strconcat(_("Bookmark added"),"\n",NULL));   
    } else {
	rfm_threaded_diagnostics(widgets_p, "xffm/stock_dialog-warning",NULL);
	rfm_threaded_diagnostics(widgets_p, "xffm_tag/stderr", g_strconcat(_("The operation was cancelled."),"\n",NULL));
    }
}

// This function uses menuitem and does not accept keybinding...
static void
bookmark_remove (widgets_t *widgets_p, gpointer menuitem) {
    view_t * view_p = widgets_p->view_p;
    const gchar *path = NULL;
    if (menuitem) path = g_object_get_data(G_OBJECT(menuitem), "path");
    else if (view_p->en) path = view_p->en->path;
    if (!path) return;
    
    if (rodent_bookmarks_remove(path)) {
	unselect_all (widgets_p);
	if (menuitem){
	    GdkRectangle *rect=g_object_get_data(G_OBJECT(menuitem), "rect");
	    if (rect) rfm_expose_rect (view_p, rect);
	}
	rodent_set_view_icon (view_p); // this is sent to main context.
    } else {
	rfm_threaded_diagnostics(widgets_p, "xffm/stock_dialog-warning",NULL);
	rfm_threaded_diagnostics(widgets_p, "xffm_tag/stderr", g_strconcat(_("The operation was cancelled."),"\n",NULL));
    }
}

static void
toggle_bookmark (widgets_t *widgets_p) {
    view_t * view_p = widgets_p->view_p;
    const gchar *path = NULL;
    if (view_p->en) path = view_p->en->path;
    if (!path) return;
    if (rodent_path_has_bookmark(path)) bookmark_remove(widgets_p, NULL);
    else bookmark_add(widgets_p, NULL);
}

static gint
compar(const void *a_p, const void *b_p){
    RodentCallback *a = (RodentCallback *)a_p; 
    RodentCallback *b = (RodentCallback *)b_p; 
    if (!a && !b) return 0;
    if (!a) return -1;
    if (!b) return -1;
    if (a->key - b->key) return (a->key - b->key);
    return (a->mask - b->mask);
}

static void
keybindings (widgets_t *widgets_p){
    RodentCallback *p = menu_callback_v;
    size_t nmemb = sizeof(menu_callback_v) / sizeof(RodentCallback) - 1;
    // quick sort
    qsort(p, nmemb, sizeof(RodentCallback), compar);
    //void (*callback)(GtkMenuItem * menuitem, gpointer user_data);
    rfm_threaded_show_text(widgets_p);
    for (; p && p->function_id > 0; p++){
	if (!p->key) continue;
	gchar *mod=g_strdup("");
	gchar *key=NULL;
	if (p->mask & GDK_SHIFT_MASK) {
	    gchar *g = g_strconcat (mod,_("Shift"), "+", NULL);
	    g_free(mod);
	    mod = g;
	}
	if (p->mask & GDK_CONTROL_MASK) {
	    gchar *g = g_strconcat (mod,_("Control"), "+", NULL);
	    g_free(mod);
	    mod = g;
	}
	if (p->mask & GDK_MOD1_MASK)  {
	     gchar *g = g_strconcat (mod,_("Alt"), "+", NULL);
	    g_free(mod);
	    mod = g;
	}
	if ((p->key > 0x40 && p->key < 0x5b) ||(p->key >0x02f  && p->key < 0x03a)) {
	    key = g_strdup_printf("%c", p->key);
	}
	else if (p->key > 0x60 && p->key < 0x7b) {
	    key = g_strdup_printf("%c", toupper(p->key));
	}
	else if (p->key > 0xffbd && p->key < 0xffca) { // function keys f1-f12
	    key = g_strdup_printf("F%d", p->key-0xffbd);
	}
	else { // other keys 
	    switch (p->key){
		case GDK_KEY_Home: key = g_strdup(_("Home")); break;
		case GDK_KEY_Left: key = g_strdup(_("Left")); break;
		case GDK_KEY_Up: key = g_strdup(_("Up")); break;
		case GDK_KEY_Right: key = g_strdup(_("Right")); break;
		case GDK_KEY_Down: key = g_strdup(_("Down")); break;
		case GDK_KEY_Page_Up: key = g_strdup(_("Page up")); break;
		case GDK_KEY_Page_Down: key = g_strdup(_("Page down")); break;
		case GDK_KEY_End: key = g_strdup(_("End")); break;
		case GDK_KEY_Begin: key = g_strdup(_("Begin")); break;
		case GDK_KEY_Delete: key = g_strdup(_("Delete")); break;
		case GDK_KEY_Insert: key = g_strdup(_("Insert")); break;
		case GDK_KEY_equal: key = g_strdup(_("Equal")); break;
		case GDK_KEY_plus: key = g_strdup(_("Plus")); break;
		case GDK_KEY_minus: key = g_strdup(_("Minus")); break;
		case GDK_KEY_KP_Add: key = g_strdup(_("Add")); break;
		case GDK_KEY_KP_Subtract: key = g_strdup(_("Subtract")); break;
	    } 
	}
	if (!key) key = g_strdup_printf("0x%x", p->key);
	const gchar *icon = p->icon;
	if (!icon && p->type == CHECKITEM_TYPE)icon = "xffm/emblem_synchronized"; 
	else if (!icon && p->type == RADIOITEM_TYPE)icon = "xffm/emblem_favorite"; 
	rfm_threaded_diagnostics(widgets_p, icon, NULL);
	rfm_threaded_diagnostics(widgets_p, "xffm_tag/red", g_strconcat(mod, key, " ",NULL));
	rfm_threaded_diagnostics(widgets_p, "xffm_tag/green", g_strconcat(_(p->string), "\n",NULL));
	g_free(key);
	g_free(mod);
    }
}

static gboolean
do_item_click(widgets_t *widgets_p, record_entry_t *en){
    TRACE ("do_item_click: %s\n", en->path);
    if (en && !IS_LOCAL_TYPE(en->type) && 
	    !rfm_entry_available(widgets_p, en))  return TRUE;
    if ((!en->mimetype  || strcmp(en->mimetype, _("unknown"))==0) && !en->mimemagic){
	if (IS_LOCAL_TYPE(en->type) && !en->mimemagic) {
	    // avoid magic on remote fs...
	    en->mimemagic = rfm_rational(RFM_MODULE_DIR, "mime", en, "mime_magic", "mime_function");
	    if (!en->mimemagic) en->mimemagic = g_strdup(_("unknown"));
	}
	if (!en->mimemagic) en->mimemagic = g_strdup(_("unknown"));
    }
    const gchar *mimetype = en->mimetype;
    if (!mimetype || strcmp(mimetype, _("unknown"))==0){
	mimetype = en->mimemagic;
    }

    if (strstr(mimetype, "application/x-desktop") &&
	rfm_void(PLUGIN_DIR, "dotdesktop", "module_active")) {
	rfm_rational(PLUGIN_DIR, "dotdesktop", widgets_p, en, "double_click");
	return TRUE;
    }
    gchar *prg = MIME_command (mimetype);
    NOOP("prg=%s, mimetype=%s\n", prg, mimetype);
    if (prg) {
	// This open_with, since prg != NULL, will
	// open the associated application.
	NOOP (stderr, "OPEN: calling rodent_open_with()\n");
	open_with (widgets_p, en);
	g_free (prg);
	return TRUE;

    } else	if (IS_EXE_TYPE (en->type)) { 
	// Since there is no associated application, then try
	// to execute the program if it has the exec bit on.
	NOOP("rodent_mouse: using RFM_THREAD_RUN2ARGV\n");
	view_t *view_p = widgets_p->view_p;
	if (view_p->en && IS_LOCAL_TYPE(view_p->en->type)){
	    g_free(widgets_p->workdir);
	    widgets_p->workdir=g_strdup(view_p->en->path);
	}
	/*  assume in_term to be safe */
	RFM_THREAD_RUN2ARGV (widgets_p, en->path, TRUE);
	return TRUE;

    } else {
	// this open_with, when prg==NULL will open the dialog, or 
	// default to default editor if file is editable. 
	open_with (widgets_p, en);
	return TRUE;

    }
    DBG("do_item_click(): nothing programed here with double click: %s\n", en->path);
    return FALSE;
}
#define MODULE_EN(x) ((view_p->module)?view_p->module:(x)->module)

static gboolean
do_folder_click(widgets_t *widgets_p, record_entry_t *en){
    TRACE( " do_folder_click...\n");

    if (en && !IS_LOCAL_TYPE(en->type) && 
	    !rfm_entry_available(widgets_p, en))  {
	return TRUE;
    }
	
    view_t *view_p = widgets_p->view_p;
    view_t *view_target = view_p;

    // Action will proceed. Clear out selection list.
    // XXX not working....
    {
	// XXX Mutex protection is missing here.
	GSList *list = view_p->selection_list;
	for (;list && list->data; list=list->next){
	    rfm_destroy_entry((record_entry_t *)list->data);
	}
	g_slist_free(view_p->selection_list);
	view_p->selection_list = NULL;
    }

    // Special treatment when item vanishes from system 
    // (only local types to avoid blockage on network error).
    if (en && !en->module &&
	    IS_SDIR(en->type) &&
	    IS_LOCAL_TYPE(en->type) &&
	    !g_file_test(en->path, G_FILE_TEST_IS_DIR)) {
	NOOP("Element is not a directory (may no longer exist)\n");
	// if go up icon selected, then go up...
	if(IS_UP_TYPE(en->type))
	{
	    gchar *path=g_strdup(en->path);
	    while (!rfm_g_file_test(path, G_FILE_TEST_IS_DIR)){
		gchar *d = g_path_get_dirname(path);
		g_free(path);
		path = d;
	    }
	    record_entry_t *new_en =rfm_stat_entry(path, 0);
	    if (!rodent_refresh (&(view_target->widgets), new_en)) rfm_destroy_entry(new_en);
	    g_free(path);
	} else {
	    rfm_confirm(widgets_p, GTK_MESSAGE_WARNING, _("The location does not exist."), NULL, _("Accept")); 
	}
	return TRUE;


    }

    // Default folder click action: jump to.
    if(en) UNSET_DUMMY_TYPE(en->type);
    if(en && IS_SDIR(en->type)){
	// en != NULL implies that path is directory
	rfm_save_to_go_history (en->path);
    }

    if(view_p->flags.type == DESKVIEW_TYPE && 
	    (!getenv("RFM_NAVIGATE_DESKTOP") || 
	     strlen(getenv("RFM_NAVIGATE_DESKTOP"))==0)){
	// This will open in a new window.
	// Basically for deskview type. 
	gchar *path=NULL;
	if (en && MODULE_EN(en)) {
	    path=g_strdup_printf("rodent-plug %s", MODULE_EN(en));
	} else if (en){
	    path=g_strdup(en->path);
	}

	NOOP ("DOUBLE_CLICK: en && view_p->child_constructor: %s (%s)\n", path, MODULE_EN(en));
	rodent_new_gridview(widgets_p, path);
	g_free(path);
	return TRUE;

    }
    NOOP(stderr, "folder click... doing refresh...\n");
    // This is for the navigation history
    rodent_push_view_go_history ();
    // Quick visual response to the user:
     rodent_threaded_clean_paper(&(view_target->widgets));
    record_entry_t *new_en = rfm_copy_entry(en);
    // Deskview and iconview navigation:
    if(rodent_refresh (&(view_target->widgets), new_en)) {
	if(en && en->path && 
		IS_SDIR (en->type)){
	    // This is for the combobox history
	    rfm_save_to_go_history (en->path);
	}
    } else {
	rfm_destroy_entry(new_en);
	rodent_expose_all (view_target);
    }
    return TRUE;
}

static void *
double_click(widgets_t *widgets_p){
     rfm_global_t *rfm_global_p = rfm_global();
   view_t *view_p = widgets_p->view_p;
    record_entry_t *en = NULL;
    g_mutex_lock(view_p->mutexes.status_mutex);
    gboolean status = view_p->flags.status;
    g_mutex_unlock(view_p->mutexes.status_mutex);
    if(status == STATUS_EXIT) return NULL;

    if (view_p->selection_list) 
	en = rfm_copy_entry((record_entry_t *)view_p->selection_list->data);   

    NOOP( "**** double_click\n");
    //hide_tip (view_p);


    if(en && !en->module && IS_NOACCESS_TYPE (en->type) && !IS_PARTITION_TYPE(en->type)) {
	rfm_threaded_show_text(widgets_p);
	rfm_threaded_diagnostics (widgets_p, "xffm/stock_dialog-error", NULL);
	rfm_threaded_diagnostics (widgets_p, "xffm_tag/stderr", 
		g_strconcat(strerror(EACCES), ": '", en->path, "'\n", NULL));
	rfm_threaded_cursor_reset(rfm_global_p->window);
	rfm_destroy_entry(en);
	return NULL;
    }

    /* does the POPULATION_MODULE have something else in mind for the
       double click on the particular entry ? */

    gboolean item_click=FALSE;
    gboolean folder_click=FALSE;
    gboolean module_click=FALSE;
    if (en){
	if (en && MODULE_EN(en)) {
	    module_click = TRUE;
	}
	if (IS_SDIR (en->type)){
	    folder_click = TRUE;
	} else if (g_path_is_absolute (en->path)){
	    item_click = TRUE;
	} else if (module_click) {
	    folder_click = TRUE;
	}
    } else {
	folder_click = TRUE;
    }
    TRACE( "Mouse: module_click=%d, item_click=%d, folder_click=%d\n", 
	    module_click,item_click, folder_click);
    // if the POPULATION_MODULE's double_click function does not exist or
    // returns NULL, callback proceeds with either item or folder behaviour.
    if (module_click && 
	rfm_rational(PLUGIN_DIR, MODULE_EN(en), widgets_p, en,  "double_click"));
    else if (item_click) do_item_click(widgets_p, en);
    else if (folder_click) do_folder_click(widgets_p, en);
    else DBG("double_click(): should not reach this point (harmless though)\n");
    
    rfm_threaded_cursor_reset(rfm_global_p->window);
    rfm_destroy_entry(en);
    return NULL;
}

/////////////////////////////////////
// Thread pool refactoring ready...//
/////////////////////////////////////

static void
apply_new_icon_size (view_t *view_p){
    static gboolean running = FALSE;
    if (running){
        DBG("Quietely ignoring apply_new_icon_size until current execution is done.\n");
        return;
    } else running = TRUE;
    TRACE("apply_new_icon_size: id=%d size=%d\n", ICON_SIZE_ID(view_p) ,ICON_SIZE(view_p));
    if (!rfm_population_read_lock (view_p, "apply_new_icon_size")) {
        running = FALSE;
	return ;
    }  
    if (view_p->module){
	// Set the iconsize in the module for the view_p.
	gint size = ICON_SIZE_ID(view_p);
	if (!size) size = -1;
	rfm_rational(PLUGIN_DIR,view_p->module, view_p, 
		GINT_TO_POINTER(size), "module_icon_size");
    }
    //record_entry_t *en = rfm_copy_entry (view_p->en);
    rfm_save_view_preferences (view_p, view_p->en); 

    view_p->flags.thumbnailer_active=TRUE;
    population_t **p = view_p->population_pp;
    gint element=0;
    for (;p && *p; p++, element++){
        population_t *population_p = *p;
        // FIXME: here we should resolve with the same procedure as used
        //        when initially loaded. This here is borked (using icontheme though not selected)
        //        but this *only* happens when monitor is disabled! 
        //        Remove iconsize changes from monitor... Everything should be here.
        if (population_p->layout) g_object_unref(population_p->layout);
        population_p->layout=NULL;
        if (population_p->layout2) g_object_unref(population_p->layout2);
        population_p->layout2=NULL;
        if (population_p->layout_full) g_object_unref(population_p->layout_full);
        population_p->layout_full=NULL;

        if (population_p->pixbuf) g_object_unref(population_p->pixbuf);
        population_p->pixbuf = NULL;

        population_p->icon_size=ICON_SIZE(view_p);
        population_p->icon_size_id = ICON_SIZE_ID(view_p);


        // No monitor, then get new icons here...
        if (10) {
            if (population_p->flags & POPULATION_IS_IMAGE){
                GdkPixbuf *pixbuf = rfm_create_preview (population_p->en->path, population_p->icon_size);//refs
                if (pixbuf) {
                    population_p->thumbnail = pixbuf;
                    population_p->flags |= POPULATION_PIXBUF_CLEAN;
                    if (element % 5 == 0) rodent_expose_all(view_p);
                    //rfm_expose_item(view_p, population_p);
                }
            }

            if (population_p->pixbuf == NULL) {
                if (population_p->icon_id) 
                    population_p->pixbuf = rfm_get_pixbuf(population_p->icon_id, ICON_SIZE(view_p));
                else if (population_p->en && population_p->en->mimetype)
                    population_p->pixbuf = rfm_get_pixbuf(population_p->en->mimetype, ICON_SIZE(view_p));
                else if (population_p->en && population_p->en->mimemagic)
                    population_p->pixbuf = rfm_get_pixbuf(population_p->en->mimemagic, ICON_SIZE(view_p));
                else
                    population_p->pixbuf = rfm_get_pixbuf("xffm/whatever", ICON_SIZE(view_p));

            }
        } else {
            // monitor will take care of new icons
            // path absolute, en != NULL en-module ==NULL
            // memset(population_p->en->st, 0., sizeof(struct stat));
        }
        population_p->layout=NULL;
        population_p->layout2=NULL;
        population_p->layout_full=NULL;

        rfm_layout_pango_layout_setup(view_p); // this gets name column width for details view
        rfm_layout_population_grid_row (view_p, population_p, element); // this sets correct row,col
    }

    rfm_population_read_unlock (view_p, "apply_new_icon_size");
    rodent_expose_all(view_p);
    view_p->flags.thumbnailer_active=FALSE;
    running = FALSE;
    
}

static GMutex *
get_sweep_mutex(void){
    static GMutex *mutex = NULL;
    static gsize initialized = 0;
    if (g_once_init_enter (&initialized)){
	rfm_mutex_init(mutex);
      g_once_init_leave (&initialized, 1);
    }
    return mutex;
}
static void
reset_saved_iconsize (DBHashTable * old) {
    DBHashTable *new = old->sweep_data;
    gint record_size = DBH_RECORD_SIZE(old);

    dbh_set_recordsize (new, record_size);
    memcpy(DBH_KEY(new), DBH_KEY(old), DBH_KEYLENGTH(old));
    memcpy(new->data, old->data, record_size);
    view_preferences_t *view_preferences_p = new->data;
    view_preferences_p->icon_size = rfm_get_default_size();
    NOOP("sweep now, record size=%d\n", record_size);
    dbh_update(new);
    return;
}

static void
default_iconsize_all (widgets_t *widgets_p){
    // The easy way would be just to unlink the preferences file,
    // but that would take down sort orders and show hidden
    // attributes as well. So instead we just set each saved preference
    // item to user default size.
    NOOP("rodent_default_iconsize_all\n");

    view_t *view_p = widgets_p->view_p;
    gint default_size = rfm_get_default_size();

    gchar *f;
    if (view_p->flags.type==ICONVIEW_TYPE) {
	f = g_build_filename (GRID_PREFERENCES_FILE, NULL);
    } else {
	f = g_build_filename (DESK_PREFERENCES_FILE, NULL);
    }
    FILE *f_p = fopen(f,"r");
    if (!f_p){
	g_free(f);
	return;
    }
    fclose(f_p);
    TRACE("opening %s...\n",f); 
    DBHashTable *old = dbh_new (f, NULL, DBH_PARALLEL_SAFE);
    TRACE("opened %s.\n",f); 
    if(old == NULL) {
	g_free(f);
	return;
    }
    dbh_set_parallel_lock_timeout(old, 3);

    gchar *ff=g_strconcat(f,"-new",NULL);
    GMutex *sweep_mutex=get_sweep_mutex() ;
    g_mutex_lock(sweep_mutex);
    unsigned char keylength=DBH_KEYLENGTH (old);
    gchar *directory = g_path_get_dirname(ff);
    if (!g_file_test(directory, G_FILE_TEST_IS_DIR)){
        g_mkdir_with_parents(directory, 0700);
    }
    g_free(directory);
    TRACE("opening %s...\n",ff); 
    DBHashTable *new = dbh_new (ff, &keylength, DBH_PARALLEL_SAFE|DBH_CREATE);
    TRACE("open %s.\n",ff); 

    if(new == NULL) {
	dbh_close(old);
	DBG("cannot create file %s\n", ff);
	g_free(f);
	g_free(ff);
	g_mutex_unlock(sweep_mutex);
	return;
    }
    dbh_set_parallel_lock_timeout(new, 3);
    old->sweep_data = new;
    NOOP("sweep now\n");
    dbh_foreach_sweep (old, reset_saved_iconsize);

    dbh_close(old);
    dbh_close(new);

    /*if (unlink(f) < 0) {
	DBG("unlink(%s): %s\n", f, strerror(errno));
    }*/
    NOOP("rename(%s, %s)\n", f, ff);
    if (rename (ff,f) < 0) {
	DBG("rename(%s, %s): %s\n", f, ff, strerror(errno));
    }

	
    g_free(f);
    g_free(ff);
	
    g_mutex_unlock(sweep_mutex);

    if (default_size != ICON_SIZE_ID(view_p)) {
	rfm_layout_set_icon_size_full(view_p, default_size);
	apply_new_icon_size (view_p);
    }
}
static void
default_iconsize (widgets_t *widgets_p){
    view_t *view_p = widgets_p->view_p;
    gint default_size = rfm_get_default_size();
    if (default_size != ICON_SIZE_ID(view_p)) {
	rfm_layout_set_icon_size_full(view_p, default_size);
	apply_new_icon_size (view_p);
    }
}

static void
increase_iconsize (widgets_t *widgets_p){
    view_t *view_p = widgets_p->view_p;
    gint original_size = ICON_SIZE_ID(view_p);
    gint new_size = original_size + 24;
    //if (new_size > BIG_ICON_SIZE) new_size = LIST_ICON_SIZE;
    if (new_size > BIG_ICON_SIZE) new_size = BIG_ICON_SIZE;
    TRACE( "newsize/original %d/%d\n", new_size, original_size);

    if (original_size != new_size) {
	rfm_layout_set_icon_size_full(view_p, new_size);
	apply_new_icon_size (view_p);
    }
}

static void
decrease_iconsize (widgets_t *widgets_p){
    view_t *view_p = widgets_p->view_p;
    gint original_size = ICON_SIZE_ID(view_p);
    gint new_size = original_size - 24;
    //if (new_size < LIST_ICON_SIZE) new_size = BIG_ICON_SIZE; 
    if (new_size < LIST_ICON_SIZE) new_size = LIST_ICON_SIZE; 
    if (original_size != new_size) {
	rfm_layout_set_icon_size_full(view_p, new_size);
	apply_new_icon_size (view_p);
    }
}


static void 
toggle_casesort(widgets_t *widgets_p){
    view_t *view_p = widgets_p->view_p;
    // Toggle value
    view_p->flags.preferences ^= __CASE_INSENSITIVE;

    rfm_save_view_preferences (view_p, view_p->en);
    record_entry_t *en = rfm_copy_entry (view_p->en);
    NOOP(stderr, "*-*-*-* toggle_casesort rodent_refresh\n");
    if (!rodent_refresh (widgets_p, en)){ rfm_destroy_entry(en); }
}

static void 
backup_unbackup(widgets_t *widgets_p){
    view_t *view_p = widgets_p->view_p;

    // Toggle value
    view_p->flags.preferences ^= __SHOWS_BACKUP;
    rfm_save_view_preferences (view_p, view_p->en);
    record_entry_t *en = rfm_copy_entry (view_p->en);
    /*if (SHOWS_BACKUP(view_p->flags.preferences)) {
	SET_SHOWS_BACKUP(en->type);
    } else {
	UNSET_SHOWS_BACKUP(en->type);
    }*/
    rodent_unsaturate_icon (view_p);

    if (!rodent_refresh (widgets_p, en)){ rfm_destroy_entry(en); }
}

static void 
hide_unhide(widgets_t *widgets_p){
    view_t *view_p = widgets_p->view_p;

    // Toggle value
    view_p->flags.preferences ^= __SHOW_HIDDEN;
    rfm_save_view_preferences (view_p, view_p->en);
    record_entry_t *en = rfm_copy_entry (view_p->en);
    /*if (SHOWS_HIDDEN(view_p->flags.preferences)) {
	SET_SHOWS_HIDDEN(en->type);
    } else {
	UNSET_SHOWS_HIDDEN(en->type);
    }*/
    rodent_unsaturate_icon (view_p);

    if (!rodent_refresh (widgets_p, en)){ rfm_destroy_entry(en); }
}

static void
preview (widgets_t *widgets_p) {
    view_t *view_p = widgets_p->view_p;

    // Toggle value
    view_p->flags.preferences ^= __SHOW_IMAGES;
    // Save preferences
    rfm_save_view_preferences (view_p, view_p->en);
    // Now do the refresh.
    record_entry_t *en = rfm_copy_entry (view_p->en);
    if (!rodent_refresh (widgets_p, en)){ rfm_destroy_entry(en); }
}

static void 
sort(widgets_t *widgets_p, gpointer user_data){
    view_t *view_p = widgets_p->view_p;
    view_p->flags.sortcolumn = GPOINTER_TO_INT (user_data);
    switch (GPOINTER_TO_INT (user_data)){
	case DATE_SORT:
	case SIZE_SORT:
	case MODE_SORT:
	view_p->flags.preferences &= (SORT_ASCENDING ^ 0xffffffff);
		break;
	default:
	view_p->flags.preferences |= SORT_ASCENDING;
    }
    NOOP(stderr, "sort callback: return on apply new sort: %d\n", GPOINTER_TO_INT (user_data));
    rfm_save_view_preferences (view_p, view_p->en);
    record_entry_t *en = rfm_copy_entry (view_p->en);
    if (!rodent_refresh (widgets_p, en)){ rfm_destroy_entry(en); }
}

static void 
size(widgets_t *widgets_p, gpointer user_data){
    NOOP(stderr, "size now...\n");
    view_t *view_p = widgets_p->view_p;
    gint new_size = GPOINTER_TO_INT (user_data);
    if (new_size != ICON_SIZE_ID(view_p)) {
	rfm_layout_set_icon_size_full(view_p, new_size);
        apply_new_icon_size (view_p);
    }
}


/*****************************************************************************/


// New thread pool compatible method.
static void *
threaded_callback(void *data){
    TRACE("threaded_callback()...\n");
    void **arg = data;
    enum menu_enum_t menu_enum = GPOINTER_TO_INT(arg[0]);
    GtkWidget *menuitem = arg[1];
    view_t *view_p = arg[2];

    g_free(data);

    // Put a lock on the view list to hold janitor in
    // case the view scheduled to be destroyed.
    // This will return false if view_p is no longer valid.
    if (!rfm_view_list_lock(view_p, "threaded_callback")) return NULL;

    widgets_t *widgets_p = &(view_p->widgets);
    
    // Do we really need to stop monitor?
    // Probably. I had a crash on a ctrl-f, when it tests for directory 
    // condition. But placed in such a top level place as such, it
    // causes diverse callback deadlocks. Must fine tune the lock placement.
    //rfm_rw_lock_reader_lock (&(view_p->mutexes.monitor_lock));

	NOOP(stderr, "threaded_callback: 0x%d\n", menu_enum);

    gboolean valid = FALSE;
    switch (menu_enum){
  	case RENAME_ACTIVATE: 
 	case DUPLICATE_ACTIVATE: 
 	case SYMLINK_ACTIVATE: 
	case MOUNT_ACTIVATE: 
 	case UNMOUNT_ACTIVATE: 
 	case AUTOTYPE_ACTIVATE: 
 	case AUTOTYPE_R_ACTIVATE: 
	    if (!is_valid_view_entry(widgets_p, menu_enum)) break;
	    if (!is_single_selection(widgets_p, menu_enum)) break;	    
	    valid = TRUE; break;

  	case BCRYPT_ACTIVATE: 
	case OPEN_WITH_ACTIVATE: 
  	case TOUCH_ACTIVATE: 
 	case PROPERTIES_ACTIVATE: 
	case REMOVE_ACTIVATE: 
	case CUT_ACTIVATE: 
 	case COPY_ACTIVATE: 
	    if (!is_valid_view_entry(widgets_p, menu_enum)) break;
	    if (!is_multiple_selection(widgets_p, menu_enum)) break;	    
	    valid = TRUE; break;

 	case PASTE_ACTIVATE: 
	case DIFFERENCES_ACTIVATE: 
	case RELOAD_ACTIVATE: 
	case REFRESH_ACTIVATE: 
	case NEW_WINDOW_ACTIVATE: 
	case NEW_TAB_ACTIVATE: 
	case NEWDIR_ACTIVATE: 
	case NEWFILE_ACTIVATE: 
	case UNSELECT_ALL_ACTIVATE: 
	case SELECT_INVERT_ACTIVATE: 
	case SELECT_ALL_ACTIVATE: 
	case UNSELECT_BY_FILTER_ACTIVATE: 
	case SELECT_BY_FILTER_ACTIVATE: 
	case BOOKMARK_TOGGLED: 
	case ASCENDING_ACTIVATE: 
	case DESCENDING_ACTIVATE: 
	case BOOKMARK_ADD_ACTIVATE: 
	case BOOKMARK_REMOVE_ACTIVATE: 
	case LS_ACTIVATE: 
	    if (!is_valid_view_entry(widgets_p, menu_enum)) break;
	    valid = TRUE; break;
	// toggles:
	case HIDDEN_TOGGLED: 
	case BACKUP_TOGGLED: 
	case PREVIEW_TOGGLED: 
	case CASESORT_TOGGLED: 


	case NAME_SORT_ACTIVATE: 
	case TYPE_SORT_ACTIVATE: 
	case DATE_SORT_ACTIVATE: 
	case SIZE_SORT_ACTIVATE: 
	case OWNER_SORT_ACTIVATE: 
	case GROUP_SORT_ACTIVATE: 
	case MODE_SORT_ACTIVATE: 
	    if (!is_valid_view_entry(widgets_p, menu_enum)) break;
	    valid = TRUE; break;
	
	case COMPACT_ICONSIZE_TOGGLED: 
	case TINY_ICONSIZE_TOGGLED: 
	case NORMAL_ICONSIZE_TOGGLED:
	case BIG_ICONSIZE_TOGGLED: 
	case HUGE_ICONSIZE_TOGGLED: 
	case PLUS_ICONSIZE_ACTIVATE: 
	case MINUS_ICONSIZE_ACTIVATE: 
	case DEFAULT_ICONSIZE_ACTIVATE: 
	case DEFAULT_ICONSIZE_ALL_ACTIVATE: 
	default:
	    valid = TRUE;
    }
    NOOP(  "***** callback: valid=%d\n", valid);
    if (valid) switch (menu_enum){

// Size callbacks:	     
	case COMPACT_ICONSIZE_TOGGLED: 
	    size(widgets_p, GINT_TO_POINTER(LIST_ICON_SIZE)); break;
	case TINY_ICONSIZE_TOGGLED: 
	    size(widgets_p, GINT_TO_POINTER(TINY_ICON_SIZE)); break;
	case NORMAL_ICONSIZE_TOGGLED:
	    size(widgets_p, GINT_TO_POINTER(SMALL_ICON_SIZE)); break;
	case BIG_ICONSIZE_TOGGLED: 
	    size(widgets_p, GINT_TO_POINTER(MEDIUM_ICON_SIZE)); break;
	case HUGE_ICONSIZE_TOGGLED: 
	    size(widgets_p, GINT_TO_POINTER(BIG_ICON_SIZE)); break;
	case PLUS_ICONSIZE_ACTIVATE: 
	    increase_iconsize(widgets_p); break;
	case MINUS_ICONSIZE_ACTIVATE: 
	    decrease_iconsize(widgets_p); break;
	case DEFAULT_ICONSIZE_ACTIVATE: 
	    default_iconsize(widgets_p); break;
	case DEFAULT_ICONSIZE_ALL_ACTIVATE: 
	    default_iconsize_all(widgets_p); break;
// Edit callbacks:
	case CUT_ACTIVATE: copy_cut_callback (widgets_p, TRUE); break;
 	case COPY_ACTIVATE: copy_cut_callback (widgets_p, FALSE); break;
 	case PASTE_ACTIVATE: paste_callback (widgets_p, FALSE); break;
	case REMOVE_ACTIVATE: remove_x( widgets_p); break;
	case DIFFERENCES_ACTIVATE: differences( widgets_p); break;
 	case TOUCH_ACTIVATE: touch(widgets_p); break;
 	case PROPERTIES_ACTIVATE: properties(widgets_p); break;
	case ACTIVATE: double_click(widgets_p); break;
	case RUN_ACTIVATE: run(widgets_p); break;
	case OPEN_WITH_ACTIVATE: open_x(widgets_p); break;
 	case MOUNT_ACTIVATE: mount(widgets_p); break;
 	case UNMOUNT_ACTIVATE: unmount(widgets_p); break;
 	case AUTOTYPE_ACTIVATE: autotype(widgets_p, menuitem); break;
 	case AUTOTYPE_R_ACTIVATE: autotype_r(widgets_p, menuitem); break;

	case FIND_ACTIVATE: 
	case GLOB_ACTIVATE: glob_x(widgets_p); break;

  	case BCRYPT_ACTIVATE: bcrypt( widgets_p); break;
	case LS_ACTIVATE: ls(widgets_p); break;
  	case RENAME_ACTIVATE: rename_x( widgets_p); break;
 	case DUPLICATE_ACTIVATE: duplicate( widgets_p); break;
 	case SYMLINK_ACTIVATE: symlink_x( widgets_p); break;
 	case DONE_WITH_RENAME: 
		rfm_context_function(done_with_rename, widgets_p); 
		break;
// View independent callbacks
 	case KEYBINDINGS_ACTIVATE:  keybindings(widgets_p); break;
	case HELP_ACTIVATE: help(widgets_p); break;
	case ABOUT_ACTIVATE: about(widgets_p); break;

	case HOST_ACTIVATE: host(widgets_p); break;     
	case OPEN_IN_TERMINAL_ACTIVATE: open_in_terminal(widgets_p); break;
	case SETTINGS_ACTIVATE: settings(widgets_p); break;
 	case CLOSE_ACTIVATE: close_x(widgets_p); break;
 	case QUIT_ACTIVATE: quit(widgets_p); break;
 	case MENU_ACTIVATE:  
		DBG("MENU_ACTIVATE: should not reach here.\n");
		break;

/////////////////////

// Go Callbacks:
	case PLUGIN_GOTO_ACTIVATE: plugin_goto(widgets_p, menuitem); break;
	case LEVEL_GOTO_ACTIVATE: level_goto(widgets_p, menuitem); break;
 	case BOOKMARK_ADD_ACTIVATE: bookmark_add(widgets_p, menuitem); break;
 	case BOOKMARK_REMOVE_ACTIVATE: bookmark_remove(widgets_p, menuitem); break;

	case RELOAD_ACTIVATE: 
	case REFRESH_ACTIVATE: refresh(widgets_p); break;     
	case GOUP_ACTIVATE: goup(widgets_p); break;     
	case FORWARD_ACTIVATE: forward(widgets_p); break;     
	case BACK_ACTIVATE: back(widgets_p); break;     
	case HOME_ACTIVATE: home(widgets_p); break;     
	case JUMP_TO_ACTIVATE: jump_to(widgets_p); break;     
	case HIDDEN_TOGGLED: hide_unhide(widgets_p); break;
	case BACKUP_TOGGLED: backup_unbackup(widgets_p); break;
	case PREVIEW_TOGGLED: preview(widgets_p); break;
	case NAME_SORT_ACTIVATE: sort(widgets_p, 
					 GINT_TO_POINTER(NAME_SORT)); break;
	case TYPE_SORT_ACTIVATE: sort(widgets_p, 
					 GINT_TO_POINTER(TYPE_SORT)); break;
	case DATE_SORT_ACTIVATE: sort(widgets_p, 
					 GINT_TO_POINTER(DATE_SORT)); break;
	case SIZE_SORT_ACTIVATE: sort(widgets_p, 
					 GINT_TO_POINTER(SIZE_SORT)); break;
	case OWNER_SORT_ACTIVATE: sort(widgets_p, 
					  GINT_TO_POINTER(OWNER_SORT)); break;
	case GROUP_SORT_ACTIVATE: sort(widgets_p, 
					  GINT_TO_POINTER(GROUP_SORT)); break;
	case MODE_SORT_ACTIVATE: sort(widgets_p, 
					 GINT_TO_POINTER(MODE_SORT)); break;
	case NEW_WINDOW_ACTIVATE: new_window(widgets_p); break;
	case NEW_TAB_ACTIVATE: new_tab_f(widgets_p); break;
	case NEWDIR_ACTIVATE: newdir (widgets_p); break;
 	case NEWFILE_ACTIVATE: newfile (widgets_p); break;
 	case UNSELECT_ALL_ACTIVATE: unselect_all(widgets_p); break;
	case SELECT_INVERT_ACTIVATE: select_all(widgets_p, GINT_TO_POINTER(1)); break;
 	case SELECT_ALL_ACTIVATE: select_all(widgets_p, NULL); break;
	case UNSELECT_BY_FILTER_ACTIVATE: select_by_filter(widgets_p, NULL); break;
 	case SELECT_BY_FILTER_ACTIVATE: select_by_filter(widgets_p, GINT_TO_POINTER(1)); break;
 	case BOOKMARK_TOGGLED: toggle_bookmark(widgets_p); break;
 	case CASESORT_TOGGLED:  toggle_casesort(widgets_p); break;
	case ASCENDING_ACTIVATE:
	case DESCENDING_ACTIVATE:
	    ascending_descending(widgets_p, menu_enum); break;  

  
//////////////
// Dummy item:
	case ENUM_CALLBACKS: break;
				
   } // end switch.
    
    //rfm_rw_lock_reader_unlock (&(view_p->mutexes.monitor_lock));

    rfm_view_list_unlock("threaded_callback()");
    TRACE("** threaded_callback is done\n");
   return NULL;
}


