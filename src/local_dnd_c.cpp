#include "local_dnd_c.hpp"

local_dnd_c::local_dnd_c(void *data){view_p = data;}

gchar *
local_dnd_c::get_fulltarget(const gchar *view_target, const gchar *target, GList *list){
    /* nonsense check first */
    const gchar *file = (const gchar *)list->data;
    gchar *ftarget;
    if (target && g_file_test(target, G_FILE_TEST_IS_DIR)){
        ftarget = g_build_filename(view_target, target, NULL);
    } else {
        ftarget = g_strdup(view_target);
    }
    return ftarget;
}

gboolean
local_dnd_c::is_nonsense(const gchar *src){
    struct stat st;
    struct stat target_st;
    gchar *src_dir = g_path_get_dirname(src);
    if (lstat ((const gchar *)src_dir, &st)==0 && lstat (fulltarget, &target_st)==0){
	// Here we check if the file source and destination is actually 
	// the same thing, this time by stat information instead of
	// path string.
	// This is a more robust test. We must test *both* st_ino and
	// st_dev, because stuff on different devices may (and do) have
	// equal st_ino.
        if(st.st_ino == target_st.st_ino &&
		st.st_dev != target_st.st_dev)
	{
	    g_free(src_dir);
	    fprintf(stderr, "Source and target directories are the same.\n");
	    return TRUE;
        }
    } else {
	g_free(src_dir);
	fprintf(stderr, "unable to stat target or source...\n");
	return TRUE;
    }
    g_free(src_dir);
    return FALSE;
}

void 
local_dnd_c::free_src_list(GList *list){
    GList *tmp = list;
    for (;tmp && tmp->data;tmp=tmp->next) g_free(tmp->data);
    g_list_free(list);
}

void
local_dnd_c::show_message_dialog(GtkDialog *dialog)
{
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
    GtkWidget *message = gtk_message_dialog_new (GTK_WINDOW(dialog),
			 flags,
			 GTK_MESSAGE_INFO,
			 GTK_BUTTONS_CLOSE,
			 _("If you ever want to enable the confirmation dialog for %s again, edit: %s\n"), 
			 vv, 
			 "~/.config/xffm/xffm.rc");
			 

    // Destroy the dialog when the user responds to it
    // (e.g. clicks a button)

    g_signal_connect_swapped (message, "response",
		  G_CALLBACK (gtk_widget_destroy),
		  message);
    gtk_window_set_title(GTK_WINDOW(message), _("For your information"));
    gtk_dialog_run (GTK_DIALOG (message));
    gtk_widget_destroy(message);
 
}

void
local_dnd_c::touch_rc_file(void){
    if (!g_file_test(kfile, G_FILE_TEST_EXISTS)){
	FILE *rcfile = fopen(kfile, "w");
	if (!rcfile){
	    fprintf(stderr, "Cannot create rcfile: %s\n", kfile);
	    return ;
	}
	fclose(rcfile);
    }
    return;
}

gchar *
local_dnd_c::get_rc_option(GList *list){
    gchar *options=NULL;

    key_file = g_key_file_new ();
    if (!g_key_file_load_from_file (key_file, kfile,
		(GKeyFileFlags)
		((gint)G_KEY_FILE_KEEP_COMMENTS|
		 (gint)G_KEY_FILE_KEEP_TRANSLATIONS), NULL))
    {
        NOOP("New file: %s\n", kfile);
    } else {
        GError *error=NULL;
        options = g_key_file_get_string (key_file, 
		"gnu_utils_c", which, &error);
        if (error){
            NOOP( "g_key_file_get_string(): %s\n", error->message);
            g_error_free(error);
	}
    }
    g_key_file_free(key_file);
    return options;
}

gchar *
local_dnd_c::get_default_options(GdkDragAction action){
        // if we do not have options, set them to defaults
    switch (action){
        case GDK_ACTION_COPY:
            return 
                g_strdup("-R -v --preserve=mode,timestamps --backup=simple --suffix=.bak");
        case GDK_ACTION_MOVE:
            return
                g_strdup("-v --backup=simple --suffix=.bak");
        case GDK_ACTION_LINK:
            return 
                g_strdup("-s -v --backup=simple --suffix=.bak");
    }
    fprintf(stderr, "get_default_options(): incorrect option.\n");
    return NULL;
}

gboolean
local_dnd_c::get_dialog_options(gchar **options_p, GList *list){
    // confirm dialog here, which has toggle for no_confirm ;
    GtkDialogFlags flags = (GtkDialogFlags)
            ((gint)GTK_DIALOG_MODAL |
            (gint)GTK_DIALOG_DESTROY_WITH_PARENT);
    GtkDialog *dialog = GTK_DIALOG(gtk_dialog_new_with_buttons (
                "GNU utils options",
                 NULL, // main_app_window,
                 flags,
                 _("_OK"),
                 GTK_RESPONSE_OK,
                 _("_Cancel"),
                 GTK_RESPONSE_REJECT,
                 NULL));
 // strings   
    gchar *title = g_strdup_printf(_("GNU %s"), vv); 
    gchar *v = g_strdup_printf(_("Do not show this %s confirm dialog again"), vv);        
    gchar *full_label;
    if (g_list_length(list) > 1) full_label = g_strdup_printf(_("%s %d paths to:\n<b>%s</b>\nwith options:"), 
            operation, g_list_length(list), fulltarget);
    else full_label = g_strdup_printf(_("%s %d path to:\n<b>%s</b>\nwith options:"), 
            operation, g_list_length(list), fulltarget);

// widgets
    GtkEntry *options_entry = GTK_ENTRY(gtk_entry_new());
    GtkToggleButton *no_confirm = GTK_TOGGLE_BUTTON(gtk_check_button_new_with_label(v));
    GtkLabel *label = GTK_LABEL(gtk_label_new(""));
// configure widgets
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_entry_set_width_chars (options_entry, strlen(*options_p)+3);
    gtk_entry_set_text(options_entry, *options_p);
    gtk_label_set_markup(label, full_label);
// pack widgets
    gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area(dialog)), GTK_WIDGET(label), FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area(dialog)), GTK_WIDGET(options_entry), TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area(dialog)), GTK_WIDGET(no_confirm), FALSE, FALSE, 0);
    gtk_widget_show_all(GTK_WIDGET(dialog));
        
        

    gint result = gtk_dialog_run (GTK_DIALOG (dialog));

    if (result == GTK_RESPONSE_OK){
        if (gtk_toggle_button_get_active(no_confirm)){
            g_key_file_set_boolean(key_file, "gnu_utils_c", no_c, TRUE);
            show_message_dialog(dialog);
            const gchar *op = gtk_entry_get_text(options_entry);
            g_free(*options_p);
            if (op) *options_p = (op)?g_strdup(op): g_strdup("");
        }
    } else  fprintf(stderr, "%s operation cancelled\n", vv);

// widget cleanup
    gtk_widget_hide(GTK_WIDGET(dialog));
    gtk_widget_destroy(GTK_WIDGET(dialog));
// string cleanup
    g_free(v);
    g_free(title);
    g_free(full_label);

    if (result != GTK_RESPONSE_OK) return FALSE;
    return TRUE;
}

gboolean
local_dnd_c::user_selected_options(gchar **options_p, GList *list){
    key_file = g_key_file_new ();
    g_key_file_load_from_file (key_file, kfile,
		(GKeyFileFlags)
		((gint)G_KEY_FILE_KEEP_COMMENTS|
		 (gint)G_KEY_FILE_KEEP_TRANSLATIONS), NULL);
    // First we check whether user has disabled the confirmation dialog
    gboolean proceed = TRUE;
    GError *error = NULL;
    if (!g_key_file_get_boolean (key_file, "gnu_utils_c", no_c, &error)){
        proceed = get_dialog_options(options_p, list);
        if (proceed){
            // set selected options in rc file
            g_key_file_set_string (key_file, "gnu_utils_c", which, *options_p);
            error = NULL;
            g_key_file_save_to_file (key_file, kfile, &error);
            if (error){
                fprintf(stderr, "g_key_file_save_to_file(): %s\n",
                        error->message);
                g_error_free(error);
            }
        } 
    }
    g_key_file_free(key_file);
    return proceed;
}

gchar *
local_dnd_c::get_options(GdkDragAction action, GList *list)
{   
    config_dir = g_build_filename(g_get_home_dir(),".config","xffm",NULL);
    if (!g_file_test(config_dir, G_FILE_TEST_IS_DIR)){
        g_mkdir_with_parents(config_dir, 0700);
    }
    kfile = g_build_filename(config_dir, "xffm.rc",NULL);
    touch_rc_file();
    // pre-initialize other strings
    no_c=NULL;

    vv="mv";
    which = "GNU_MOVE_OPTIONS";
    operation=_("Move");
    switch (action){
        case GDK_ACTION_COPY:
            vv="cp";
            operation=_("Copy");
            which = "GNU_COPY_OPTIONS";
            break;
        case GDK_ACTION_LINK:
            vv="ln";
            operation=_("Link");
            which = "GNU_LINK_OPTIONS"; 
         break;
    }
    no_c = g_strdup_printf("%s_no_confirm", vv);
    gchar *options=get_rc_option(list);

    // if we do not have options, set them to defaults
    if (!options) options = get_default_options(action);

    // Here we allow user to change and save options as new defaults
    if (!user_selected_options(&options, list)){
            // Cancelled by confirmation dialog
            g_free(options);
            options=NULL;
    }

    // final string cleanup
    g_free(kfile);
    g_free(no_c);
    g_free(config_dir);
    return options;
}

gboolean
local_dnd_c::remove_url_file_prefix (gchar * data) {
    static gchar *url_he = NULL;
    if(strncmp (data, "file:/", strlen ("file:/")) == 0 ){
	const gchar *f = "file:";
	if(strncmp (data, "file:///", strlen ("file:///")) == 0) {
	    f = "file://";
	} else 	if(strncmp (data, "file://", strlen ("file://")) == 0) {
	    f = "file:/";
	} 
	if (g_file_test(data + strlen (f), G_FILE_TEST_EXISTS)) {
	    memmove (data, data + strlen (f), strlen (data + strlen (f)) + 1);
	    fprintf(stderr, "DnD source: %s\n", data);
	    return TRUE;
	}

    }
    fprintf(stderr, "%s is not a local file\n", data);
    return FALSE;
}


gboolean
local_dnd_c::remove_file_prefix_from_uri_list (GList * list) {
    while(list) {
        gchar *url = (gchar *) list->data;
	if (!remove_url_file_prefix (url)){
	    fprintf(stderr, "local_dnd_c: unable to parse url list\n");
	    return FALSE;
	}
	list = list->next;
    }
    return TRUE;
}


gint
local_dnd_c::parse_url_list(const gchar * DnDtext, GList ** list){
    gchar *q, *p;
    const gchar *linesep;

    NOOP ("DnD text: %s\n", DnDtext);

    q = p = g_strdup (DnDtext);

    if(strstr (p, "\r\n")) linesep = "\r\n";
    else if(strstr (p, "\n")) linesep = "\n";
    else if(strstr (p, "\r")) linesep = "\r";
    else if(strlen (p)) {
	// only one item...
        *list = g_list_append (*list, g_strdup (p));
        NOOP ("add list: %s\n", p);
        return 1;
    } else return 0;

    for(p = strtok (p, linesep); p; p = strtok (NULL, linesep)) {
        gchar *t = g_strdup (p);
        NOOP ("add list: %s\n", t);
        *list = g_list_append (*list, t);
    }
    g_free (q);
    return g_list_length(*list);
}

gboolean
local_dnd_c::_receive_dnd(const gchar *view_target, const gchar *target, GtkSelectionData *data, GdkDragAction action)
{
    GList *list = NULL;
    if (!parse_url_list ((const char *)gtk_selection_data_get_data (data), &list)) {
	fprintf(stderr, "!parse_url_list\n");
	return FALSE;
    }
    remove_file_prefix_from_uri_list (list);
    fulltarget = get_fulltarget(view_target, target, list);
    if (is_nonsense((gchar *)list->data)){
        g_free(fulltarget);
	free_src_list(list);
	fprintf(stderr, "is_nonsense\n");
	return FALSE;
    }
	       
    gchar *options=get_options(action, list);
    if (!options){
        g_free(fulltarget);
	free_src_list(list);
	fprintf(stderr, "!options\n");
	return FALSE;
    }

    // Proceed with command
    switch (action){
        case GDK_ACTION_COPY:
            cp(view_p, list, fulltarget, options);
            break;
        case GDK_ACTION_MOVE:
            mv(view_p, list, fulltarget, options);
            break;
        case GDK_ACTION_LINK:
            ln(view_p, list, fulltarget, options);
         break;
    }
    
    fprintf(stderr, "action=%d options=%s: ->%s\n", action, options, fulltarget);
    
    g_free(options);
    g_free(fulltarget);
    // free uri list now
    free_src_list(list);

    return TRUE;
}


gboolean
local_dnd_c::_set_dnd_data(GtkSelectionData * selection_data, GList *selection_list,
        GtkTreeModel *treemodel, const gchar *view_path, gint actual_name_column){
    GList *tmp;
    const gchar *format = "file://";
    gint selection_len = 0;
    /* count length of bytes to be allocated */
    for(tmp = selection_list; tmp; tmp = tmp->next) {
	GtkTreePath *tpath = (GtkTreePath *)tmp->data;
	gchar *g;
	GtkTreeIter iter;
	gtk_tree_model_get_iter (treemodel, &iter, tpath);
	gtk_tree_model_get (treemodel, &iter, 
	    actual_name_column, &g, -1); 
	gchar *dndpath = g_build_filename(view_path, g, NULL);
	g_free(g);
	/* 2 is added for the \r\n */
	selection_len += (strlen (dndpath) + strlen (format) + 2);
	g_free(dndpath);
    }
    /* 1 is added for terminating null character */
    fprintf(stderr, "allocating %d bytes for dnd data\n",selection_len + 1);
    //files = (gchar *)calloc (selection_len + 1,1);
    /*if (!files) {
	g_error("signal_drag_data_get(): malloc %s", strerror(errno));
	return;
    }*/
    gchar *files = g_strdup("");
    for(tmp = selection_list; tmp; tmp = tmp->next) {
	GtkTreePath *tpath = (GtkTreePath *)tmp->data;
	gchar *g;
	GtkTreeIter iter;
	gtk_tree_model_get_iter (treemodel, &iter, tpath);
	gtk_tree_model_get (treemodel, &iter, 
	    actual_name_column, &g, -1); 
	gchar *dndpath = g_build_filename(view_path, g, NULL);
	g_free(g);
	g=g_strconcat(files,format,dndpath,"\n", NULL);
	g_free(files);
	files=g;

	/*sprintf (files, "%s%s\r\n", format, dndpath);
	files += (strlen (format) + strlen (dndpath) + 2);
	g_free(dndpath);*/
    }
    gtk_selection_data_set (selection_data, 
	gtk_selection_data_get_selection(selection_data),
	8, (const guchar *)files, selection_len);
    fprintf(stderr, ">>> DND send, drag data is:\n%s\n", files);
    g_free(files);
    return TRUE;
}


