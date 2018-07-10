#ifdef COPYRIGHT_INFORMATION
#include "gplv3.h"
#endif
/*
 * Copyright (C) 2003-2012 Edscott Wilson Garcia
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

static char *ftypes[] = {
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

static char *ft[] = {
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


static gint result_limit_counter;
// These externals are in find-module_gui.c
extern gint result_limit;
extern gint size_greater;
extern gint size_smaller;
extern gint last_minutes;
extern gint last_hours;
extern gint last_days;
extern gint last_months;
extern gboolean default_recursive;
extern gboolean default_recursiveH;
extern gboolean default_xdev;
extern gboolean default_line_count;
extern gboolean default_case_sensitive;
extern gboolean default_ext_regexp;
extern gboolean default_look_in_binaries;
extern gint default_type_index;
extern gboolean default_anywhere;
extern gboolean default_match_words;
extern gboolean default_match_lines;
extern gboolean default_match_no_match;
extern gboolean have_grep;
extern gboolean have_gnu_grep;
extern GSList *find_list;
extern gchar  *last_workdir;

static gchar *
#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
get_combo_entry (void* combo_info, GtkComboBoxEntry *combobox);
#else
get_combo_entry (void* combo_info, GtkComboBox *combobox);
#endif

static void
grep_options (GtkComboBox *widget, gpointer data) {
    find_struct_t *find_struct_p = data;
    gchar *s = get_combo_entry (find_struct_p->findgrep_combo_info, find_struct_p->combogrep);
    gboolean state=FALSE;
    if (s && strlen(s)) {
	// sensitive
	state=TRUE;
    }   
    g_free(s);
    gtk_widget_set_sensitive (
	    GTK_WIDGET(g_object_get_data(
		    G_OBJECT(find_struct_p->dialog), "case_sensitive")), state);
    gtk_widget_set_sensitive (
	    GTK_WIDGET(g_object_get_data(
		    G_OBJECT(find_struct_p->dialog), "line_count")), state);
   if (have_gnu_grep) 
    {
	gtk_widget_set_sensitive ( 
	    GTK_WIDGET(g_object_get_data(
		    G_OBJECT(find_struct_p->dialog), "ext_regexp")), state);
	gtk_widget_set_sensitive ( 
	    GTK_WIDGET(g_object_get_data(
		    G_OBJECT(find_struct_p->dialog), "look_in_binaries")), state);
    }
    gtk_widget_set_sensitive ( 
	    GTK_WIDGET(g_object_get_data(
		    G_OBJECT(find_struct_p->dialog), "label40")), state);
    gtk_widget_set_sensitive (
	    GTK_WIDGET(g_object_get_data(
		    G_OBJECT(find_struct_p->dialog), "anywhere")), state);
    gtk_widget_set_sensitive (
	    GTK_WIDGET(g_object_get_data(
		    G_OBJECT(find_struct_p->dialog), "match_words")), state);
    gtk_widget_set_sensitive ( 
	    GTK_WIDGET(g_object_get_data(
		    G_OBJECT(find_struct_p->dialog), "match_lines")), state);
    gtk_widget_set_sensitive ( 
	    GTK_WIDGET(g_object_get_data(
		    G_OBJECT(find_struct_p->dialog), "match_no_match")), state);

}
    
static gboolean will_destroy = FALSE;

static void *
hide_cancel_button(gpointer data){
    GtkWidget *cancel = data;
    gtk_widget_set_sensitive(cancel, FALSE);
    gtk_widget_hide(cancel);
    return NULL;
}

static void *
exit_loop(gpointer data){
    gtk_main_quit();
    return NULL;
}

// This is a thread function, must have GDK mutex set for gtk commands...
static
void
stderr_f (void *user_data, void *stream, int childFD) {
    // Grep behaviour has changed. stderr had become more verbose.
    // So let us just dump it.
    return;
}

// This is a thread function...
static
void
stdout_f (void *user_data, void *stream, int childFD) {
    widgets_t *widgets_p = user_data;
    char *line;
    line = (char *)stream;
    NOOP ("FORK stdout: %s\n", line);

    if(line[0] == '\n') return;

    if (result_limit > 0 && result_limit==result_limit_counter) {
	gchar *g=g_strdup_printf("%s. %s %d", _("Results"), _("Upper limit:"), result_limit);
	if (will_destroy) {  // Standalone
	    fprintf(stdout, "%c%s%s\n", 27, rfm_lp_color(GREEN), g); 
	    fprintf(stdout, "%c%s%s\n", 27, rfm_lp_color(BLUE),_("Counting files...")); 
	    fprintf(stdout, "%c[0m", 27 );fflush(stdout);
	} else {
	    rfm_threaded_diagnostics (widgets_p, "xffm/stock_dialog-warning", NULL);
	    rfm_threaded_diagnostics (widgets_p, "xffm_tag/green", g_strconcat(g, "\n", NULL));
	    rfm_threaded_diagnostics (widgets_p, "xffm/stock_dialog-info", NULL);
	    rfm_threaded_diagnostics (widgets_p, "xffm_tag/blue",  g_strconcat(_("Counting files..."), "\n", NULL));
	}
	g_free(g);
    }

    if(strncmp (line, "fgr search complete!", strlen ("fgr search complete!")) == 0) {
#if 0
	if (will_destroy) {  // Standalone
	    fprintf(stdout, "%c%s%s\n", 27, rfm_lp_color(BLUE),  _("Search Complete")); 
	    fprintf(stdout, "%c[0m", 27 );fflush(stdout);
	} else {
	    rfm_threaded_diagnostics (widgets_p, "xffm/stock_find", NULL);
	    rfm_threaded_diagnostics (widgets_p, "xffm_tag/blue", g_strconcat(_("Search Complete"), NULL));
	    rfm_threaded_diagnostics (widgets_p, NULL, g_strconcat("\n", NULL));
            
	}
#endif
    } else if(strncmp (line, "Tubo-id exit:", strlen ("Tubo-id exit:")) == 0) {

            if(strchr (line, '\n')) *strchr (line, '\n') = 0;
	    GtkWidget *cancel = widgets_p->data; 
	    if (cancel && GTK_IS_WIDGET(cancel)) {
		rfm_context_function(hide_cancel_button, cancel);
	    }
	    gchar *plural_text = 
		g_strdup_printf (
			ngettext ("Found %d match", "Found %d matches", 
			    result_limit_counter), result_limit_counter);
	    gchar *fgr = g_find_program_in_path("fgr");
	    gchar *m = g_strdup_printf(_("%s Finished : %s"), fgr, plural_text);
	    g_free(fgr);
	    g_free(plural_text);
	    if (will_destroy) {  // Standalone
		fprintf(stdout, "%c%s%s.\n", 27, rfm_lp_color(RED), m); 
		fprintf(stdout, "%c[0m\n", 27 );fflush(stdout);
		rfm_context_function(exit_loop, widgets_p->paper);
	    } else {
                gchar *g = g_strdup_printf("%c[31m%s\n",27, m);
		rfm_threaded_diagnostics (widgets_p, "xffm/emblem_redball", g);
	    }
	    g_free(m);
    } else {
	result_limit_counter++; 
	if (result_limit ==0 ||
	    (result_limit > 0 && result_limit > result_limit_counter) ) {
	    if (will_destroy) {  // Standalone
		fprintf(stdout, "%s", line); 
		fprintf(stdout, "%c[0m", 27 );fflush(stdout); 
	    } else {
		rfm_threaded_diagnostics (widgets_p, NULL, g_strdup(line));
	    }
	    gchar *file = g_strdup(line);
	    if (strchr(file, '\n')) *strchr(file, '\n') = 0;
	    find_list = g_slist_prepend(find_list, file);
	} 
    }
    return;
}



static void
path_filechooser (GtkButton * button, gpointer user_data) {
    filechooser_t *filechooser_p = (filechooser_t *) user_data;
    const gchar *text;
    if(filechooser_p->filechooser_action == GTK_FILE_CHOOSER_ACTION_OPEN) {
        text = _("Select Files...");
    } else if(filechooser_p->filechooser_action == GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER) {
        text = _("Select folder to search in");
    } else {
        text = "FIXME";
    }
    NOOP ("FIND: parent=0x%lu action=%d, text=%s\n", (long unsigned)filechooser_p->parent, filechooser_p->filechooser_action, text);
    GtkWidget *dialog = gtk_file_chooser_dialog_new (text,
                                                     GTK_WINDOW (filechooser_p->parent),
                                                     filechooser_p->filechooser_action,
                                                     _("Cancel"),
                                                     GTK_RESPONSE_CANCEL,
                                                     _("Open"),
                                                     GTK_RESPONSE_ACCEPT,
                                                     NULL);
    gtk_file_chooser_set_action ((GtkFileChooser *) dialog, filechooser_p->filechooser_action);
    if(filechooser_p->folder) {
        gtk_file_chooser_set_current_folder ((GtkFileChooser *) dialog, filechooser_p->folder);
    }

    gint response = rfm_dialog_run_response(dialog);

    if(response == GTK_RESPONSE_ACCEPT) {
        char *filename;
        void *combo_info = filechooser_p->combo_info;
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
	if (rfm_void(RFM_MODULE_DIR, "combobox", "module_active")){
	    COMBOBOX_set_entry (combo_info, filename);
	} else {
	    gtk_entry_set_text (filechooser_p->entry, filename);
	}
        NOOP ("Got %s\n", filename);
        g_free (filename);
    }
    gtk_widget_destroy (dialog);

}

static void
save_ff_text (char *p) {
    gchar *fname;
    if(!p || !strlen (p))
        return;
    fname = g_build_filename (FIND_DBH_FILE, NULL);
    COMBOBOX_save_to_history (fname, p);
    g_free (fname);
}

static void
save_fpath_text (const char *p) {
    gchar *fname;
    NOOP ("save_fpath_text-> %s\n", p);
    if(!p || !strlen (p))
        return;
    {
        fname = g_build_filename (FIND_PATHS_DBH_FILE, NULL);
    }
    NOOP ("saving to history-> %s\n", p);
    COMBOBOX_save_to_history (fname, p);
    g_free (fname);
}

static void
save_fgrep_text (char *p) {
    gchar *fname;
    if(!p || !strlen (p))
        return;
    fname = g_build_filename (FIND_GREP_DBH_FILE, NULL);
    COMBOBOX_save_to_history (fname, p);
    g_free (fname);
}

static void
cancel_all (find_struct_t *find_struct_p) {
    // Send a KILL to each fgr that is still running. This is done
    // by sending a SIGUSR2 to the controller (SIGUSR1 would pass on a TERM).
    GSList *tmp = find_struct_p->controllers;
    for (; tmp && tmp->data; tmp=tmp->next){
	kill(GPOINTER_TO_INT(tmp->data), SIGUSR2);
    }
    g_slist_free(find_struct_p->controllers);
    find_struct_p->controllers=NULL;
}

static void
cancel_callback (GtkWidget * button, gpointer data) {
    GtkWidget *dialog=g_object_get_data(G_OBJECT(button), "dialog");
    find_struct_t *find_struct_p = g_object_get_data(G_OBJECT(dialog), "find_struct_p");
    cancel_all(find_struct_p);
    gtk_widget_set_sensitive(button, FALSE);
}


static void
destroy_find (GtkWidget * dialog, gpointer data) {
    gtk_widget_hide (dialog);
    find_struct_t *find_struct_p = g_object_get_data(G_OBJECT(dialog), "find_struct_p");
    if (!find_struct_p) {
	return;
    }
    g_object_set_data(G_OBJECT(dialog),"find_struct_p", NULL);
    //widgets_t *widgets_p=g_object_get_data(G_OBJECT(dialog), "widgets_p");
    COMBOBOX_destroy_combo(find_struct_p->findpath_combo_info);
    COMBOBOX_destroy_combo(find_struct_p->find_combo_info);
    COMBOBOX_destroy_combo(find_struct_p->findgrep_combo_info);

    g_free(find_struct_p);
    g_object_set_data(G_OBJECT(dialog), "widgets_p", NULL);
    g_free(g_object_get_data(G_OBJECT(dialog), "radio_p" ));
    
    gtk_widget_destroy (dialog);
    if (find_list) {
	GSList *list = find_list;
	for (;list && list->data; list=list->next){
	    g_free(list->data);
	}
	g_slist_free(find_list);
	find_list = NULL;
    }
    gtk_main_quit();
}

static void
on_find_close (GtkButton * button, gpointer data) {
    GtkWidget *dialog=g_object_get_data(G_OBJECT(button), "dialog");
    destroy_find (dialog, data);
}


static gchar *
#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
get_combo_entry (void* combo_info, GtkComboBoxEntry *combobox) {
#else
get_combo_entry (void* combo_info, GtkComboBox *combobox) {
#endif
    const gchar *choice;
    gchar *s = NULL;

    if (rfm_void(RFM_MODULE_DIR, "combobox", "module_active")){
	choice = (gchar *) COMBOBOX_get_entry (combo_info);
    } else {
	choice = gtk_entry_get_text (GTK_ENTRY(
		    gtk_bin_get_child(GTK_BIN(combobox))));
    }
    /*NOOP("choice== %s\n", choice);
       NOOP("choice== %s\n", choice); */

    if(choice) {
        s = g_locale_from_utf8 (choice, -1, NULL, NULL, NULL);
    }
    if(s)
        s = g_strchug (s);
    if(s)
        s = g_strchomp (s);
    if(!s)
        s = g_strdup ("");

    return s;
}

static const gchar *
get_time_type(GtkWidget *dialog){
    if (gtk_toggle_button_get_active ((GtkToggleButton *) g_object_get_data(G_OBJECT(dialog), "radio1"))){
	return "-M";
    }
    if (gtk_toggle_button_get_active ((GtkToggleButton *) g_object_get_data(G_OBJECT(dialog), "radio2"))){
	return "-C";
    }
    return "-A";
}

static void
on_find_clicked_action (GtkButton * button, gpointer data, gboolean destroy) {
    gchar *sg=NULL;
    gchar *sl=NULL;
    gchar *lm=NULL;
    gchar *ld=NULL;
    gchar *lh=NULL;
    gchar *lmm=NULL;
    gchar *pw_uid=NULL;
    gchar *pw_gid=NULL;
 
    short int i, j;
    gchar *s;
    gchar *argument[MAX_COMMAND_ARGS];
    gchar *find_filter = NULL,
        *filter = NULL,
        *path = NULL,
        *token = NULL;
    find_struct_t *find_struct_p = data;


    GtkWidget *dialog=g_object_get_data(G_OBJECT(button), "dialog");
    will_destroy = destroy;
    

    widgets_t *widgets_p = g_object_get_data(G_OBJECT(dialog), "widgets_p");
    if(!widgets_p) {
        g_warning ("on_find_clicked: !widgets_p\n");
        return;
    }
    GtkWidget *cancel = g_object_get_data(G_OBJECT(dialog), "cancel_button");
    gtk_widget_show(cancel);
    gtk_widget_set_sensitive(cancel, TRUE);
    widgets_p->data = cancel;



    rfm_show_text (widgets_p);

    /* get options */

/* get the parameters set by the user... *****/

/* limit */
    result_limit = gtk_spin_button_get_value_as_int (
	    GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(dialog), "upper_limit_spin")));
    result_limit_counter=0;

/* the rest */

    s = get_combo_entry (find_struct_p->findpath_combo_info, find_struct_p->combopath);
    if(!s || strlen (s) == 0 || !rfm_g_file_test (s, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR)) {
        rfm_diagnostics (widgets_p, "xffm/stock_dialog-error", strerror (ENOENT), ": ", s, NULL);
        rfm_diagnostics (widgets_p, NULL, "\n", NULL);
	g_free(s);
        return;
    }

    /* tilde expansion */
    else if(s[strlen (s) - 1] == '~'){
	g_free(s);
        s = g_strdup("~/");
    }
    /* environment variables */
    else if(s[0] == '$') {
        const gchar *p = getenv (s + 1);
        if(p){
	    g_free(s);
            s = g_strdup(p);
	}else {
	    g_free(s);
            s = g_strdup("/");
	}
    }

    save_fpath_text (s);

	    
    g_free (widgets_p->workdir);
    widgets_p->workdir=g_strdup (s);
    g_free(last_workdir);
    last_workdir = g_strdup (s);
    path = g_strdup (".");
    g_free(s);

    s = get_combo_entry (find_struct_p->find_combo_info, find_struct_p->combo);
    if(s && strlen (s)) {
        filter = g_strdup (s);
        save_ff_text (filter);
    }
    g_free(s);

    s = get_combo_entry (find_struct_p->findgrep_combo_info, find_struct_p->combogrep);

    if(s && strlen (s)) {
        token = g_strdup (s);
        save_fgrep_text (token);
    } else {
        /* if no grep pattern given, its a plain find and
         * we should show the diagnostics */
    }
    g_free(s);

    /* select list */
#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
    s = gtk_combo_box_get_active_text ((GtkComboBox *)
	    g_object_get_data(G_OBJECT(dialog), "file_type_om"));
#else
    s = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT(
	    g_object_get_data(G_OBJECT(dialog), "file_type_om")));
#endif
    default_type_index=gtk_combo_box_get_active((GtkComboBox *)
	    g_object_get_data(G_OBJECT(dialog), "file_type_om"));

    i = 0;
    {
        argument[i++] = "fgr";

        /*argument[i++] = "-P"; */

        if(gtk_toggle_button_get_active ((GtkToggleButton *) g_object_get_data(G_OBJECT(dialog), "recursive"))) {
	    default_recursive=TRUE;
            if(gtk_toggle_button_get_active ((GtkToggleButton *) g_object_get_data(G_OBJECT(dialog), "recursiveH"))) {
                argument[i++] = "-r";
		default_recursiveH=TRUE;
            } else {
                argument[i++] = "-R";
		default_recursiveH=FALSE;
            }
        } else {
	    default_recursive=FALSE;
	}

        if(gtk_toggle_button_get_active ((GtkToggleButton *) g_object_get_data(G_OBJECT(dialog), "size_greater"))) {
	     size_greater = gtk_spin_button_get_value_as_int (
		GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(dialog), "size_greater_spin")));	    
             argument[i++] = "-s";
	     argument[i++] = sg = g_strdup_printf("+%d", size_greater);
	}
        if(gtk_toggle_button_get_active ((GtkToggleButton *) g_object_get_data(G_OBJECT(dialog), "size_smaller"))) {
	     size_smaller = gtk_spin_button_get_value_as_int (
		GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(dialog), "size_smaller_spin")));	    
             argument[i++] = "-s";
	     argument[i++] = sl = g_strdup_printf("-%d", size_smaller);
	}
        if(gtk_toggle_button_get_active ((GtkToggleButton *) g_object_get_data(G_OBJECT(dialog), "last_months"))) {
	    argument[i++] = (gchar *)get_time_type(dialog);
	    last_months = gtk_spin_button_get_value_as_int (
		GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(dialog), "last_months_spin")));	    
             argument[i++] = "-m";
	     argument[i++] = lm = g_strdup_printf("%d", last_months);
	}
	else if(gtk_toggle_button_get_active ((GtkToggleButton *) g_object_get_data(G_OBJECT(dialog), "last_days"))) {
	    argument[i++] = (gchar *)get_time_type(dialog);
	     last_days = gtk_spin_button_get_value_as_int (
		GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(dialog), "last_days_spin")));	    
             argument[i++] = "-d";
	     argument[i++] = ld = g_strdup_printf("%d", last_days);
	}
	else if(gtk_toggle_button_get_active ((GtkToggleButton *) g_object_get_data(G_OBJECT(dialog), "last_hours"))) {
	    argument[i++] = (gchar *)get_time_type(dialog);
	     last_hours = gtk_spin_button_get_value_as_int (
		GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(dialog), "last_hours_spin")));	    
             argument[i++] = "-h";
	     argument[i++] = lh = g_strdup_printf("%d", last_hours);
	}
	else if(gtk_toggle_button_get_active ((GtkToggleButton *) g_object_get_data(G_OBJECT(dialog), "last_minutes"))) {
	    argument[i++] = (gchar *)get_time_type(dialog);
	     last_minutes = gtk_spin_button_get_value_as_int (
		GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(dialog), "last_minutes_spin")));	    
             argument[i++] = "-k";
	     argument[i++] = lmm = g_strdup_printf("%d", last_minutes);
	}



        if(gtk_toggle_button_get_active ((GtkToggleButton *) g_object_get_data(G_OBJECT(dialog), "suidexe"))){
            argument[i++] = "-p";
	    if(gtk_toggle_button_get_active ((GtkToggleButton *) g_object_get_data(G_OBJECT(dialog), "suid_radio"))){
		argument[i++] = "suid";
	    } else {
		argument[i++] = "exe";
	    }
	} 
        if(gtk_toggle_button_get_active ((GtkToggleButton *) g_object_get_data(G_OBJECT(dialog), "octal_p"))){
            argument[i++] = "-o";
	    const gchar *c = gtk_entry_get_text ((GtkEntry *)g_object_get_data(G_OBJECT(dialog), "permissions_entry"));
	    if (c && strlen(c)) argument[i++] = (gchar *)c;
	    else argument[i++] = "0666";
	} 

        if(gtk_toggle_button_get_active ((GtkToggleButton *) g_object_get_data(G_OBJECT(dialog), "uid"))){
	    GtkWidget *entry = gtk_bin_get_child(GTK_BIN(g_object_get_data(G_OBJECT(dialog), "uid_combo")));
	    const gchar *val = gtk_entry_get_text (GTK_ENTRY (entry));
	    if(val && strlen(val)) {
		argument[i++] = "-u";
		struct passwd *pw = getpwnam (val);
		if(pw) {
		    pw_uid = g_strdup_printf("%d", pw->pw_uid);
		} else {
		    pw_uid = g_strdup_printf("%d",atoi(val));
		}
		argument[i++] = pw_uid;

	    }
	}
        if(gtk_toggle_button_get_active ((GtkToggleButton *) g_object_get_data(G_OBJECT(dialog), "gid"))){
	    GtkWidget *entry = gtk_bin_get_child(GTK_BIN(g_object_get_data(G_OBJECT(dialog), "gid_combo")));
	    const gchar *val = gtk_entry_get_text (GTK_ENTRY (entry));
	    if(val && strlen(val)) {
		argument[i++] = "-g";
		struct group *gr = getgrnam (val);
		if(gr) {
		    pw_gid = g_strdup_printf("%d", gr->gr_gid);
		} else {
		    pw_gid = g_strdup_printf("%d",atoi(val));
		}
		argument[i++] = pw_gid;

	    }
	}



        if(!gtk_toggle_button_get_active ((GtkToggleButton *) g_object_get_data(G_OBJECT(dialog), "case_sensitive"))){
            argument[i++] = "-i";
	    default_case_sensitive=FALSE;
	} else {
	    default_case_sensitive=TRUE;
	}
        if(gtk_toggle_button_get_active ((GtkToggleButton *) g_object_get_data(G_OBJECT(dialog), "line_count"))){
            argument[i++] = "-c";
	    default_line_count=TRUE;
	} else {
	    default_case_sensitive=FALSE;
	}
        if(gtk_toggle_button_get_active ((GtkToggleButton *) g_object_get_data(G_OBJECT(dialog), "xdev"))){
            argument[i++] = "-a";
	    default_xdev=TRUE;
	} else {
	    default_xdev=FALSE;
	}
        if(token) {
            if(gtk_toggle_button_get_active ((GtkToggleButton *) g_object_get_data(G_OBJECT(dialog), "ext_regexp"))){
                argument[i++] = "-E";
		default_ext_regexp=TRUE;
	    } else {
                argument[i++] = "-e";
		default_ext_regexp=FALSE;
	    }
            argument[i++] = token;

            /* options for grep: ***** */
            if(gtk_toggle_button_get_active ((GtkToggleButton *) g_object_get_data(G_OBJECT(dialog), "look_in_binaries"))){
		default_look_in_binaries=TRUE;
	    } else {
                argument[i++] = "-I";
		default_look_in_binaries=FALSE;
	    }
	    default_anywhere=default_match_words=default_match_lines=default_match_no_match=FALSE;
            if(gtk_toggle_button_get_active ((GtkToggleButton *) g_object_get_data(G_OBJECT(dialog), "anywhere"))) {
		default_anywhere=TRUE;
	    }
            else if(gtk_toggle_button_get_active ((GtkToggleButton *) g_object_get_data(G_OBJECT(dialog), "match_words"))){
                argument[i++] = "-w";
		default_match_words=TRUE;
	    }
            else if(gtk_toggle_button_get_active ((GtkToggleButton *) g_object_get_data(G_OBJECT(dialog), "match_lines"))){
                argument[i++] = "-x";
		default_match_lines=TRUE;
	    }
            else if(gtk_toggle_button_get_active ((GtkToggleButton *) g_object_get_data(G_OBJECT(dialog), "match_no_match"))){
                argument[i++] = "-L";
		default_match_no_match=TRUE;
	    }
        }

        for(j = 0; ftypes[j] != NULL; j++) {
            if(s && strcmp (s, _(ftypes[j])) == 0) {
                argument[i++] = "-t";
                argument[i++] = ft[j];
                break;
            }
        }

        /* apply wildcard filter if not specified (with dotfiles option) */
        argument[i++] = "-f";
        if(filter && strlen (filter)) {
            argument[i++] = filter;
        } else {
            argument[i++] = "*";
            argument[i++] = "-D";
        }

        /* last argument is the path (which could be NULL) */
        argument[i++] = path;
    }
        
    argument[i++] = "-v"; // (verbose output) 

    argument[i] = (char *)0;

    if (will_destroy) {  // Standalone
	fprintf(stdout, "%c%s", 27, rfm_lp_color(BLUE)); 
	gchar **ap = argument;
	for (;ap && *ap; ap++) {
	    if (strchr(*ap, '*') || strchr(*ap, '?') || strchr(*ap, ' ')) {
		fprintf(stdout, " \"%s\"", *ap);
	    }
	    else fprintf(stdout, " %s", *ap);
	}
	fprintf(stdout, "\n"); 
	fprintf(stdout, "%c%s%s\n", 27, rfm_lp_color(GREEN),  _("Searching...")); 
	fprintf(stdout, "%c[0m", 27 );fflush(stdout);
    } else {    
	rfm_diagnostics (widgets_p, "xffm/emblem_find", NULL);
	rfm_diagnostics (widgets_p, "xffm_tag/green", _("Searching..."), "\n", NULL);
    }

    if (find_list) {
	GSList *list = find_list;
	for (;list && list->data; list=list->next){
	    g_free(list->data);
	}
	g_slist_free(find_list);
	find_list = NULL;
    }



    void **arg = (void **)malloc(7*sizeof(void *));
    if (!arg) g_error("malloc: %s\n", strerror(errno));
    arg[0] =  widgets_p;
    arg[1] =  (void *)argument;
    arg[2] =   NULL;
    arg[3] =   NULL;
    arg[4] =   (void *)stdout_f;
    arg[5] =   (void *)stderr_f;
    arg[6] =   NULL;

    pid_t controller =
        GPOINTER_TO_INT(rfm_natural(RFM_MODULE_DIR, "run", arg, "m_thread_run_argv"));
//          m_thread_run_argv(arg);
//	rfm_thread_run_argv_full (widgets_p, argument, FALSE, NULL, stdout_f, stderr_f, NULL);
    g_free(sg);
    g_free(sl);
    g_free(lm);
    g_free(ld);
    g_free(lh);
    g_free(lmm);
    g_free(pw_uid);
    g_free(pw_gid);
    // When the dialog is destroyed on action, then output is going to a
    // widgets_p away from dialog window, so we let the process run on and on.
    if (!destroy) {
	find_struct_p->controllers = 
	    g_slist_prepend(find_struct_p->controllers, GINT_TO_POINTER(controller));
    }


    NOOP ("FGR: --> ");
    rfm_show_text (widgets_p);
    for(j = 0; j < i; j++) {
        NOOP (" %s", argument[j]);
    }
    NOOP ("\n");
    if (destroy) {
	// In this case we let the fgr process run on and on
	// (see above)
	gtk_widget_destroy (dialog);
    }
    g_free(s);
    g_free (token);
    g_free (filter);
    g_free (find_filter);
    g_free (path);
}

static void
on_find_clicked (GtkButton * button, gpointer data) {
    on_find_clicked_action (button, data, TRUE);
}

static void
on_apply_clicked (GtkButton * button, gpointer data) {
    on_find_clicked_action (button, data, FALSE);
}

static void
on_find_clicked_wrapper (GtkEntry * entry, gpointer data) {
    on_find_clicked_action ((GtkButton *) entry, data, FALSE);
}

