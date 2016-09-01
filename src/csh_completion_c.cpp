#define DEBUG_TRACE 1
#include "csh_completion_c.hpp"
#include "view_c.hpp"

csh_completion_c::csh_completion_c(void *data): bash_completion_c(data){
    csh_command_list = NULL;
    csh_cmd_save = NULL;
    csh_completing = FALSE;
    csh_load_history();
}

void 
csh_completion_c::csh_set_completing(gboolean state){ csh_completing = state;}

gboolean
csh_completion_c::csh_completion(gint direction, gint offset){

    if (!csh_cmd_save) {
	// initialize csh completion.
	csh_cmd_save = get_current_text ();
	if (!csh_cmd_save || !strlen(csh_cmd_save)) {
	    // empty line: no completion attempted.
	    TRACE("lpterm_c::csh_completion: empty line: no completion attempted.\n");
	    g_free(csh_cmd_save);
	    csh_cmd_save = NULL;
	    csh_offset_history(0);
	    return FALSE;
	}
	csh_nth = 0;
    }
    // cursor_position is a GtkTextBuffer internal property (read only)
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(status));
    g_object_get (G_OBJECT (buffer), "cursor-position", &csh_cmd_len, NULL);
    TRACE( "lpterm_c::csh_completion: to cursor position=%d \n", csh_cmd_len);
    if (!csh_cmd_len || !csh_completing) {
	// no text before cursor: no completion attempted.
	TRACE("lpterm_c::csh_completion: return on !csh_cmd_len\n");
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
        TRACE( "cshnth=%d\n",csh_nth); 
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
	    TRACE( "lpterm_c::csh_completion: back to original command: %s.\n", csh_cmd_save);
	    csh_place_command(csh_cmd_save);
	    csh_completion_init();
	    return TRUE;
    }
    // push/pop history
    csh_offset_history(offset);
    return FALSE;
}

void
csh_completion_c::csh_completion_init(void){
    g_free(csh_cmd_save);
    csh_cmd_save = NULL;
    csh_completing = FALSE;
    csh_nth = 0;
}

void 
csh_completion_c::csh_place_command(const gchar *data){
    print_status ("%s", data);
    place_cursor();
}

gpointer
csh_completion_c::csh_load_history (void) {
    gchar *history = g_build_filename (CSH_HISTORY, NULL);
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
	
    return NULL;
}

void
csh_completion_c::csh_save_history (const gchar * data) {
    GList *p;
    gchar *command_p = g_strdup(data);
    g_strstrip (command_p);
    // Get last registered command
    void *last = g_list_nth_data (csh_command_list, 0);
    if (last && strcmp((gchar *)last, command_p) == 0) {
	g_free(command_p);
	// repeat of last command. nothing to do here.
	return;
    }
    // don't save to file if invalid command (cd counts as invalid).
    if(!csh_is_valid_command (command_p)) {
	if(strcmp (command_p, "cd") != 0 && strncmp (command_p, "cd ", strlen ("cd ")) != 0) {
	    DBG ("not saving %s\n", command_p);
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
	csh_command_list = g_list_insert(csh_command_list, data, 0);
	goto save_to_disk;
    }

    // so the item was not found. proceed to insert
    csh_command_list = g_list_insert(csh_command_list, command_p, 0);

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
    disk_history = g_list_prepend (disk_history, g_strdup (command_p));
    disk_history = g_list_reverse(disk_history);

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
    return;
}

gboolean
csh_completion_c::csh_is_valid_command (const gchar *cmd_fmt) {
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
csh_completion_c::csh_offset_history(gint offset){
    gint item = csh_command_counter + offset;
    if (item < 0) item = 0;
    if (item >= g_list_length(csh_command_list)) item = g_list_length(csh_command_list) - 1;
    const gchar *p = (const gchar *)g_list_nth_data (csh_command_list, item<0?0:item);
    TRACE( "csh_completion_c::csh_offset_history: get csh_nth csh_command_counter=%d offset=%d item=%d\n", csh_command_counter, offset, item);
    if (csh_command_counter + offset < 0) p = "";
    if(p) {
	csh_command_counter = item;
	csh_place_command(p);
	csh_completion_init();
	
    }
    return TRUE;
}


void 
csh_completion_c::place_cursor(void){
    GtkTextIter iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(status));
    if (csh_completing) {
        gtk_text_buffer_get_iter_at_offset (buffer, &iter, csh_cmd_len);
    } else {
        gchar *text = get_current_text ();
        gtk_text_buffer_get_iter_at_offset (buffer, &iter, strlen(text));
        g_free(text);
    }
    gtk_text_buffer_place_cursor (buffer, &iter);
}


