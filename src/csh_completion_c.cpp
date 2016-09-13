//#define DEBUG_TRACE 1
#include "csh_completion_c.hpp"
#include "view_c.hpp"

csh_completion_c::csh_completion_c(void *data): bash_completion_c(data){
    csh_cmd_save = NULL;
    csh_history_list = NULL;
    csh_load_history();
    csh_clean_start();
}

void 
csh_completion_c::csh_dirty_start(void){ 
    TRACE( "csh_completion_c::csh_dirty_start\n");
    //start from current position.
    g_free(csh_cmd_save);
    while (gtk_events_pending()) gtk_main_iteration();
    csh_cmd_save = get_text_to_cursor();
    TRACE( "csh_completion_c::csh_dirty_start: %s\n", csh_cmd_save);
    if (csh_cmd_save && strlen(csh_cmd_save)){
        csh_nth = 0;
        csh_completing = TRUE;
    } else {
        csh_clean_start();
    }

}


void 
csh_completion_c::csh_clean_start(void){
    TRACE( "csh_completion_c::csh_clean_start\n");
    //start from bottom.
    g_free(csh_cmd_save);
    csh_cmd_save = NULL;
    csh_nth = 0;
    csh_completing = FALSE;
    csh_history_counter = -1; // non completion counter.
    
}

const gchar *
csh_completion_c::csh_find(const gchar *token, gint direction){
    if (csh_nth) csh_nth += direction;
    if (csh_nth >= g_list_length(csh_history_list)){
        csh_nth = g_list_length(csh_history_list)-1;
        return NULL;
    }
    GList *list = g_list_nth(csh_history_list, csh_nth);
    for (;list && list->data; (direction>0)?list=list->next:list=list->prev, csh_nth+=direction){
        if (list->data && strncmp(token, (gchar *)list->data, strlen(token))==0) {
            return (const gchar *)list->data;
        }
    }
    // When you reach the bottom, put command back.
    if (direction < 0) {
        csh_nth = 0;
        csh_place_command(token);
    }
    return NULL;
}

gboolean
csh_completion_c::is_completing(void){
    return csh_completing;
}

gboolean
csh_completion_c::query_cursor_position(void){
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(status));
    gint position = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(buffer), "cursor-position"));
    TRACE("cursor position=%d\n", position);
    if (!position) csh_completing = FALSE;
    return csh_completing;
}

gboolean
csh_completion_c::csh_completion(gint direction){

    // Completing?
    const gchar *suggest = csh_find(csh_cmd_save, direction);
    if (suggest){
	TRACE("csh_completion_c::csh_completion: suggest=%s at csh_nth=%d\n", suggest, csh_nth); 
	csh_place_command(suggest);
        return TRUE;
    }
    TRACE("csh_completion_c::csh_completion: no match to %s\n", csh_cmd_save);
    csh_place_command(csh_cmd_save);
    
    return FALSE;
}

void 
csh_completion_c::csh_place_command(const gchar *data){
    print_status (g_strdup(data));
    place_cursor();
}

gpointer
csh_completion_c::csh_load_history (void) {
    gchar *history = g_build_filename (CSH_HISTORY, NULL);
    GList *p;
    // clean out history list before loading a new one.
    for(p = csh_history_list; p; p = p->next) {
        g_free (p->data);
    }
    g_list_free (csh_history_list);
    csh_history_list = NULL;

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
	    GList *element = find_in_string_list(csh_history_list, newline);

	    if (element) { 
		// remove old element
		gchar *data=(gchar *)element->data;
		csh_history_list = g_list_remove(csh_history_list, data);
		g_free(data);
	    }
	    // put element at top of the pile
	    csh_history_list = g_list_prepend(csh_history_list, newline);
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
    void *last = g_list_nth_data (csh_history_list, 0);
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
    GList *item = find_in_string_list(csh_history_list, command_p);
    if (item){
	void *data = item->data;
	// remove old position
	csh_history_list=g_list_remove(csh_history_list, data);
	// insert at top of list (item 0 is empty string)
	csh_history_list = g_list_insert(csh_history_list, data, 0);
	goto save_to_disk;
    }

    // so the item was not found. proceed to insert
    csh_history_list = g_list_insert(csh_history_list, command_p, 0);

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
    // this is to add the -A option, which is necesary for
    // password dialog to pop up, when password is required.
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
csh_completion_c::csh_history(gint offset){
    csh_history_counter += offset;
    if (csh_history_counter < 0)  {
        csh_history_counter = -1;
	csh_place_command("");
        return TRUE;
    }
    if (csh_history_counter > g_list_length(csh_history_list)-2) 
        csh_history_counter = g_list_length(csh_history_list) - 2;

    const gchar *p = (const gchar *)g_list_nth_data (csh_history_list, csh_history_counter);
    
    TRACE( "csh_completion_c::csh_history: get csh_history_counter=%d offset=%d \n", csh_history_counter, offset);
    
    if(p) {
	csh_place_command(p);
    }
    return TRUE;
}


void 
csh_completion_c::place_cursor(void){
    GtkTextIter iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(status));
    if (csh_completing && csh_cmd_save) {
        gtk_text_buffer_get_iter_at_offset (buffer, &iter, strlen(csh_cmd_save));
    } else {
        gchar *text = get_current_text ();
        gtk_text_buffer_get_iter_at_offset (buffer, &iter, strlen(text));
        g_free(text);
    }
    gtk_text_buffer_place_cursor (buffer, &iter);
}


