#include "completion_c.hpp"
#include "view_c.hpp"

completion_c::completion_c(void *data): print_c(data){
    csh_command_list = NULL;
    csh_cmd_save = NULL;
    csh_command_mutex = PTHREAD_MUTEX_INITIALIZER;
    csh_load_history();
}



gboolean
completion_c::csh_completion(gint direction, gint offset){
    if (!csh_cmd_save) {
	// initialize csh completion.
	csh_cmd_save = get_current_text ();
	if (!csh_cmd_save || !strlen(csh_cmd_save)) {
	    // empty line: no completion attempted.
	    TRACE("lpterm_c::csh_completion: empty line: no completion attempted.\n");
	    g_free(csh_cmd_save);
	    csh_cmd_save = NULL;
	    csh_offset_history(offset);
	    return FALSE;
	}
	csh_nth = 0;
    }
    // cursor_position is a GtkTextBuffer internal property (read only)
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(status));
    g_object_get (G_OBJECT (buffer), "cursor-position", &csh_cmd_len, NULL);
    TRACE ("lpterm_c::csh_completion: to cursor position=%d \n", csh_cmd_len);
    if (!csh_cmd_len) {
	// no text before cursor: no completion attempted.
	TRACE ("lpterm_c::csh_completion: return on !csh_cmd_len\n");
	csh_offset_history(offset);
        return FALSE;
    }
    gchar *command = get_text_to_cursor ();

    const gchar *csh_cmd_found = NULL;
    if (direction > 0){ 
	// up arrow.
	if (csh_nth && csh_nth < g_list_length(csh_command_list)) csh_nth++;
	GList *list = g_list_nth(csh_command_list, csh_nth);
	for (;list && list->data;list=list->next, csh_nth++){
	    if (list->data && strncmp(command, (gchar *)list->data, csh_cmd_len)==0) {
		csh_cmd_found = (const gchar *)list->data;
		break;
	    }
	    csh_cmd_found = NULL;
	} 
    } else {
	// down arrow.
	if (csh_nth > 0) csh_nth--;
	GList *list = g_list_nth(csh_command_list, csh_nth);
	for (;list && list->data && csh_nth>0;list=list->prev, csh_nth--){
	    csh_cmd_found = (gchar *)list->data;
	    if (csh_cmd_found && strncmp(command, csh_cmd_found, csh_cmd_len)==0) {
		csh_cmd_found = (const gchar *)list->data;
		break;
	    }
	    csh_cmd_found = NULL;
	} 
	if (csh_nth < 0) csh_nth = 0;
    }
    g_free(command);

    if (csh_cmd_found){
	TRACE("lpterm_c::csh_completion: csh_nth=%d/%d\n", 
	    csh_nth, g_list_length(csh_command_list));
	csh_place_command(csh_cmd_found);
	return TRUE;    
    } 
    if (!csh_nth){
	    TRACE("lpterm_c::csh_completion: back to original command: %s.\n", csh_cmd_save); 
	    csh_place_command(csh_cmd_save);
	    completion_init();
	    return TRUE;
    }
    // push/pop history
    csh_offset_history(offset);
    return FALSE;
}

void
completion_c::completion_init(void){
    g_free(csh_cmd_save);
    csh_cmd_save = NULL;

}

void 
completion_c::csh_place_command(const gchar *data){
    print_status ("%s", data);
    place_cursor();
}

gpointer
completion_c::csh_load_history (void) {
    gchar *history = g_build_filename (CSH_HISTORY, NULL);
    pthread_mutex_lock(&csh_command_mutex);
    GList *p;
    // clean out history list before loading a new one.
    for(p = csh_command_list; p; p = p->next) {
        g_free (p->data);
    }
    g_list_free (csh_command_list);
    csh_command_list = NULL;
    csh_command_counter = 0; // XXX ??

    FILE *sh_history = fopen (history, "r");
    if(sh_history) {
        gchar line[2048];
        memset (line, 0, 2048);
        while(fgets (line, 2047, sh_history) && !feof (sh_history)) {
            if(strchr (line, '\n')) *strchr (line, '\n') = 0;
            if(strlen (line) == 0) continue;
            // skip invalid commands (except cd):
            if(!csh_is_valid_command (line)) {
                if(strcmp (line, "cd") != 0 && strncmp (line, "cd ", strlen ("cd ")) != 0) {
                    TRACE ("lpterm_c::csh_load_history: HISTORY: invalid history command in %s: %s\n", history, line);
                    continue;
                }
            }
	    gchar *newline = compact_line(line);
	    GList *element = find_in_string_list(csh_command_list, newline);

	    if (element) { 
		// remove old element
		gchar *data=(gchar *)element->data;
		csh_command_list = g_list_remove(csh_command_list, data);
		g_free(data);
	    }
	    // put element at top of the pile
	    csh_command_list = g_list_prepend(csh_command_list, newline);
        }
        fclose (sh_history);
    }
    g_free (history);
    // reverse list
    csh_command_list = g_list_reverse(csh_command_list);
	
    pthread_mutex_unlock(&csh_command_mutex);
    return NULL;
}

void
completion_c::csh_save_history (const gchar * data) {
    GList *p;
    gchar *command_p = g_strdup(data);
    g_strstrip (command_p);
    pthread_mutex_lock(&csh_command_mutex);
    // Get last registered command
    void *last = g_list_nth_data (csh_command_list, 1);
    if (last && strcmp((gchar *)last, command_p) == 0) {
	g_free(command_p);
	// repeat of last command. nothing to do here.
	return;
    }
    // don't save to file if invalid command (cd counts as invalid).
    if(!csh_is_valid_command (command_p)) {
	if(strcmp (command_p, "cd") != 0 && strncmp (command_p, "cd ", strlen ("cd ")) != 0) {
	    DBG ("not saving %s\n", command_p);
	    pthread_mutex_unlock(&csh_command_mutex);
	    g_free(command_p);
	    return;
	}
    }

    // if command is already in history, bring it to the front
    GList *item = find_in_string_list(csh_command_list, command_p);
    if (item){
	void *data = item->data;
	// remove old position
	csh_command_list=g_list_remove(csh_command_list, data);
	// insert at top of list (item 0 is empty string)
	csh_command_list = g_list_insert(csh_command_list, data, 1);
	goto save_to_disk;
    }

    // so the item was not found. proceed to insert
    csh_command_list = g_list_insert(csh_command_list, command_p, 1);

save_to_disk:
    // rewrite history file
    gchar *history = g_build_filename (CSH_HISTORY, NULL);
    // read it first to synchronize with other xffm+ instances
    GList *disk_history = NULL;       
    FILE *sh_history = fopen (history, "r");
    if(sh_history) {

	char line[2048];
	memset (line, 0, 2048);
	while(fgets (line, 2047, sh_history) && !feof (sh_history)) {
	    if(strchr (line, '\n')) *strchr (line, '\n') = 0;
	    if(strcmp (line, command_p) != 0) {
		disk_history = g_list_prepend (disk_history, g_strdup (line));
	    }
	}
	fclose (sh_history);
    }
    disk_history = g_list_reverse(disk_history);
    disk_history = g_list_prepend (disk_history, g_strdup (command_p));

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
    pthread_mutex_unlock(&csh_command_mutex);
    return;
}

gboolean
completion_c::csh_is_valid_command (const gchar *cmd_fmt) {
    //return GINT_TO_POINTER(TRUE);
    NOOP ("csh_is_valid_command(%s)\n", cmd_fmt);
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
        retval=csh_is_valid_command(argv[i]);
    }

    g_strfreev (argv);
    g_free (path);
    return retval;
}

gboolean
completion_c::csh_offset_history(gint offset){
    void *p = g_list_nth_data (csh_command_list, csh_command_counter + offset);
    NOOP ("get csh_nth csh_command_counter=%d\n", csh_command_counter + offset);
    if(p) {
	csh_command_counter += offset;
	csh_place_command((const gchar *)p);
    }
    return TRUE;
}


void 
completion_c::place_cursor(void){
    GtkTextIter iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(status));
    gtk_text_buffer_get_iter_at_offset (buffer, &iter, csh_cmd_len);
    gtk_text_buffer_place_cursor (buffer, &iter);
}

gchar *
completion_c::get_current_text (void) {
    // get current text
    GtkTextIter start, end;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer ((GtkTextView *) status);

    gtk_text_buffer_get_bounds (buffer, &start, &end);
    gchar *t = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);
    g_strchug(t);
    return t;
}

gchar *
completion_c::get_text_to_cursor (void) {
    // get current text
    GtkTextIter start, end;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(status));
    gint cursor_position;
    // cursor_position is a GtkTextBuffer internal property (read only)
    g_object_get (G_OBJECT (buffer), "cursor-position", &cursor_position, NULL);
    
    gtk_text_buffer_get_iter_at_offset (buffer, &start, 0);
    gtk_text_buffer_get_iter_at_offset (buffer, &end, cursor_position);
    gchar *t = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);
    g_strchug(t);
    TRACE ("lpterm_c::get_text_to_cursor: to cursor position=%d %s\n", cursor_position, t);
    return t;
}



