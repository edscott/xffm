#include <string.h>
#include <strings.h>
#include <errno.h>
#include <stdlib.h>
#include "view_c.hpp"
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include "xfdir_local_c.hpp"

static gint compare_by_name (const void *, const void *);

xfdir_local_c::xfdir_local_c(data_c *data0, const gchar *data, gboolean data2): 
    local_monitor_c(data0, data)
{
    data_p = data0;
    shows_hidden = data2;
    NOOP( "data2=%d\n", data2);
    treemodel = mk_tree_model();
    user_string_mutex=PTHREAD_MUTEX_INITIALIZER;
    group_string_mutex=PTHREAD_MUTEX_INITIALIZER;
    date_string_mutex=PTHREAD_MUTEX_INITIALIZER;
    start_monitor(data, treemodel);
}

xfdir_local_c::~xfdir_local_c(void){
    stop_monitor();
}

gboolean
xfdir_local_c::receive_dnd(const gchar *target, GtkSelectionData *data, GdkDragAction action)
{
    gchar *options=NULL;
    GList *list = NULL;
    gint retval = 0;
    gint nitems = parse_url_list ((const char *)gtk_selection_data_get_data (data), &list);

    fprintf(stderr, "rodent_mouse: DND receive, nitems=%d\n", nitems);
    if(!nitems) return FALSE;
    remove_file_prefix_from_uri_list (list);
    /* nonsense check first */
    const gchar *file = (const gchar *)list->data;
    gchar *src_dir = g_path_get_dirname(file);
    struct stat st;
    struct stat target_st;
    gchar *fulltarget;
    if (target && g_file_test(target, G_FILE_TEST_IS_DIR)){
        fulltarget = g_build_filename(path, target, NULL);
    } else {
        fulltarget = g_strdup(path);
    }

    
    GError *error=NULL;
    // 1. get options from user resource file

    gchar *config_dir = g_build_filename(g_get_home_dir(),".config","xffm",NULL);
    if (!g_file_test(config_dir, G_FILE_TEST_IS_DIR)){
        g_mkdir_with_parents(config_dir, 0700);
    }
    gchar *kfile = g_build_filename(config_dir, "xffm.rc",NULL);
    g_free(config_dir);    
    GKeyFile *key_file = g_key_file_new ();
    // here we allow user to change options with dialog
    // set options to environment, also to a simple user config file

    const gchar *vv;
    const gchar *which = "";
    switch (action){
        case GDK_ACTION_COPY:
                vv="cp";
           which = "GNU_COPY_OPTIONS";
            break;
        case GDK_ACTION_MOVE:
                vv="mv";
           which = "GNU_MOVE_OPTIONS";
            break;
        case GDK_ACTION_LINK:
                vv="ln";
           which = "GNU_LINK_OPTIONS";
         break;
    }
    if (!g_key_file_load_from_file (key_file, kfile,
		(GKeyFileFlags)((gint)G_KEY_FILE_KEEP_COMMENTS|(gint)G_KEY_FILE_KEEP_TRANSLATIONS), 
                NULL)){
        NOOP("New file: %s\n", kfile);
    } else {
        error=NULL;
        options = g_key_file_get_string (key_file, "gnu_utils_c", which, &error);
        if (error){
            fprintf(stderr, "g_key_file_get_string(): %s\n", error->message);
            g_error_free(error);
        }


    if (!options) {
        // if we still do not have options, set them to defaults
        switch (action){
            case GDK_ACTION_COPY:
                options = g_strdup("-R -v --preserve=mode,timestamps --backup=simple --suffix=.bak");
                break;
            case GDK_ACTION_MOVE:
                options = g_strdup("-v --backup=simple --suffix=.bak");
                break;
            case GDK_ACTION_LINK:
                options = g_strdup("-s -v --backup=simple --suffix=.bak");
             break;
        }
    }
    gchar *no_c = g_strdup_printf("%s_no_confirm", vv);
    error=NULL;
    if (!g_file_test(kfile, G_FILE_TEST_EXISTS) ||
            !g_key_file_get_boolean (key_file, "gnu_utils_c", no_c, &error)){
        // confirm dialog here, which has toggle for no_confirm ;
        GtkDialogFlags flags = (GtkDialogFlags)((gint)GTK_DIALOG_MODAL | (gint)GTK_DIALOG_DESTROY_WITH_PARENT);
        GtkDialog *dialog = GTK_DIALOG(gtk_dialog_new_with_buttons ("My dialog",
                                      NULL, //main_app_window,
                                      flags,
                                      _("_OK"),
                                      GTK_RESPONSE_ACCEPT,
                                      _("_Cancel"),
                                      GTK_RESPONSE_REJECT,
                                      NULL));
        gchar *title = g_strdup_printf(_("GNU %s\n"), vv);  
        gtk_window_set_title(GTK_WINDOW(dialog), title);
        GtkEntry *options_entry = GTK_ENTRY(gtk_entry_new());
        gtk_entry_set_width_chars (options_entry, strlen(options)+3);
        gtk_entry_set_text(options_entry, options);
        gchar *v = g_strdup_printf(_("Do not show this %s confirm dialog again"), vv);
        
        GtkToggleButton *no_confirm = GTK_TOGGLE_BUTTON(gtk_check_button_new_with_label(v));
        g_free(v);
        // make button for each of cp,mv.ln
        
        gchar *fullabel = g_strdup_printf(_("%s %d paths to:\n<b>%s</b>\nwith options:"), vv, g_list_length(list), fulltarget);
        GtkLabel *label = GTK_LABEL(gtk_label_new(""));
        gtk_label_set_markup(label, fullabel);
        g_free(fullabel);
        gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area(dialog)), GTK_WIDGET(label), FALSE, FALSE, 0);
        gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area(dialog)), GTK_WIDGET(options_entry), TRUE, TRUE, 0);
        gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area(dialog)), GTK_WIDGET(no_confirm), FALSE, FALSE, 0);
        label = GTK_LABEL(gtk_label_new(""));
        gtk_label_set_markup(label,_("<i>~/.config/xffm/xffm.rc</i>"));
        gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area(dialog)), GTK_WIDGET(label), FALSE, FALSE, 0);

    
        gtk_widget_show_all(GTK_WIDGET(dialog));
        gint result = gtk_dialog_run (GTK_DIALOG (dialog));
        switch (result)
        {
            case GTK_RESPONSE_ACCEPT:
            case GTK_RESPONSE_OK:
               break;
            default:
               // If cancel, goto done elsewise set options
               fprintf(stderr, "%s operation cancelled\n", vv);
               gtk_widget_hide(GTK_WIDGET(dialog));
               gtk_widget_destroy(GTK_WIDGET(dialog));
               goto done;
               break;
        }

        g_key_file_set_boolean(key_file, "gnu_utils_c", no_c, 
                gtk_toggle_button_get_active(no_confirm));
        const gchar *op = gtk_entry_get_text(options_entry);
        g_free(options);
        if (op) options = (op)?g_strdup(op): g_strdup("");
        g_key_file_set_string (key_file, "gnu_utils_c", which, options);
        gtk_widget_hide(GTK_WIDGET(dialog));
        gtk_widget_destroy(GTK_WIDGET(dialog));
        g_free(title);
    }
    g_free(no_c);

    //
    // // 5. Save resource file with option value for command
    }
    error = NULL;
    g_key_file_save_to_file (key_file, kfile, &error);
    if (error){
        fprintf(stderr, "g_key_file_save_to_file(): %s\n", error->message);
        g_error_free(error);
    }


    fprintf(stderr, "DnD path=\"%s\", fulltarget=\"%s\"\n", path, fulltarget);
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
            //rfm_diagnostics(&(view_p->widgets),"xffm/stock_dialog-warning",NULL);
            //rfm_diagnostics (widgets_p, "xffm_tag/stderr", " ", strerror (EEXIST), ": ", target_en->path, "\n", NULL);
	    goto done;
        }
    } else {
	fprintf(stderr, "unable to stat target or source...\n");
	goto done;
    }

    
    fprintf(stderr, "action=%d options=%s: ->%s\n", action, options, fulltarget);

    

    // 6. Proceed with command
    switch (action){
        case GDK_ACTION_COPY:
            cp(list, fulltarget, options);
            break;
        case GDK_ACTION_MOVE:
            mv(list, fulltarget, options);
            break;
        case GDK_ACTION_LINK:
            ln(list, fulltarget, options);
         break;

    }
    
    g_free(options);
    g_free(fulltarget);
    //rfm_complex(RFM_MODULE_DIR, "callbacks", GINT_TO_POINTER(mode), list, target_en->path, "cp"); 
    retval = TRUE;
done:
    g_free(src_dir);
    // free uri list now
    GList *tmp = list;
    for (;tmp && tmp->data;tmp=tmp->next) g_free(tmp->data);
    g_list_free(list);
    g_free(kfile);
    g_key_file_free(key_file);

    return retval;
}

gboolean
xfdir_local_c::remove_url_file_prefix (gchar * data) {
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
xfdir_local_c::remove_file_prefix_from_uri_list (GList * list) {
    while(list) {
        gchar *url = (gchar *) list->data;
	if (!remove_url_file_prefix (url)){
	    fprintf(stderr, "xfdir_local_c: unable to parse url list\n");
	    return FALSE;
	}
	list = list->next;
    }
    return TRUE;
}


gint
xfdir_local_c::parse_url_list(const gchar * DnDtext, GList ** list){
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
xfdir_local_c::set_dnd_data(GtkSelectionData * selection_data, GList *selection_list){
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
	    ACTUAL_NAME, &g, -1); 
	gchar *dndpath = g_build_filename(get_path(), g, NULL);
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
	    ACTUAL_NAME, &g, -1); 
	gchar *dndpath = g_build_filename(get_path(), g, NULL);
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

void
xfdir_local_c::item_activated (GtkIconView *iconview, GtkTreePath *tpath, void *data)
{
    view_c *view_p = (view_c *)data;
    GtkTreeModel *tree_model = gtk_icon_view_get_model (iconview);
    GtkTreeIter iter;
    if (!gtk_tree_model_get_iter (tree_model, &iter, tpath)) return;
    
    gchar *ddname;
    gchar *mimetype = NULL;
    gtk_tree_model_get (tree_model, &iter,
                          ACTUAL_NAME, &ddname,
                          COL_MIMETYPE, &mimetype,
                          -1);
    if (strcmp(ddname, "..")==0){
        g_free(ddname);
        ddname = g_path_get_dirname(path);
    } else {
        gchar *g = g_build_filename(path, ddname, NULL);
        g_free(ddname);
        ddname = g;
    }

    if (!mimetype){
        mimetype = mime_type(ddname, NULL);
    }

    if (!mimetype){
        view_p->get_lpterm_p()->print_error(g_strdup_printf("%s = NULL)\n",
                    _("Mime Type"), mimetype));
        return;
    }

    if (strcmp(mimetype, "inode/directory")==0){
        view_p->reload(ddname);
    } else {
        gchar *command = mime_command(mimetype);
        view_p->get_lpterm_p()->print_error(g_strdup_printf("mimetype = %s (%s)\n", mimetype, command));
        if (!command){
            // try pure mime magic
            g_free(mimetype);
            mimetype = mime_function(ddname, "mime_magic");
            command = mime_command(mimetype);
        }
        if (!command && strncmp(mimetype, "text/", strlen("text/"))==0){
                command = get_text_editor();
            view_p->get_lpterm_p()->print_error(g_strdup_printf("text mimetype = %s (%s)\n", mimetype, command));
        }
       /* gchar **a = mime_apps(mimetype);
        gchar **p = a;
        for (;p && *p; p++) fprintf(stderr, "mimeapp = %s\n", *p);
        g_strfreev(a);*/
        

        if (command){
          // ddname should be quoted and 
          // command not saved in csh history
            //gchar *m = g_strdup_printf("%s(%s) = %s)\n",_("Command"), mimetype, command);
            //view_p->get_lpterm_p()->print_error(m);

            gchar *c = NULL;
            gchar *q = g_strdup_printf("\"%s\"", ddname);
            if (strstr(command, "%s")){
                //format
                c = g_strdup_printf(command, q);
            } else if (command) {
                c = g_strdup_printf("%s \"%s\"", command, q);
            }
            view_p->get_lpterm_p()->shell_command(c, FALSE);
            fprintf(stderr, "%s\n", c);
            g_free(c);
            g_free(command);
        } else {
            gchar *m = g_strdup_printf("FIXME openwith dialog here: %s(%s) = NULL)\n",_("Command"), mimetype);
            view_p->get_lpterm_p()->print_error(m);
        }
    }
    g_free(mimetype);
    g_free(ddname);
} 


gchar *
xfdir_local_c::make_tooltip_text (GtkTreePath *tpath) {
    if (!tpath) return g_strdup("tpath is NULL\n");

    gchar *text = get_tooltip_text(tpath);
    if (text) return text;

    text = get_path_info(treemodel, tpath, path);  
    set_tooltip_text(tpath, text);
    return text;
}
 
void 
xfdir_local_c::highlight_drop(GtkTreePath *tpath){
    GtkTreeIter iter;
    gchar *filename;
    gtk_tree_model_get_iter (treemodel, &iter, tpath);
    gtk_tree_model_get (treemodel, &iter, 
            COL_ACTUAL_NAME, &filename, -1);
    gchar *fullpath = g_build_filename(path, filename, NULL);
    if (g_file_test(fullpath, G_FILE_TEST_IS_DIR)){
        highlight(tpath);
    }
    g_free(fullpath);
    return;
}
   
GtkTreeModel *
xfdir_local_c::mk_tree_model (void)
{
    NOOP( "xfdir_local_c::mk_tree_model for %s\n", path);
    if (!path || !g_file_test(path, G_FILE_TEST_EXISTS)) {
        fprintf(stderr, "%s does not exist\n", path);
        return NULL;
    }
    if (chdir(path)<0){
        fprintf(stderr, "chdir(%s): %s\n", path, strerror(errno));
        return NULL;
    }

    
    GtkListStore *list_store = gtk_list_store_new (NUM_COLS, 
	    GDK_TYPE_PIXBUF, // icon in display
	    GDK_TYPE_PIXBUF, // normal icon reference
	    GDK_TYPE_PIXBUF, // highlight icon reference
	    GDK_TYPE_PIXBUF, // preview, tooltip image (cache)
	    G_TYPE_STRING,   // name in display (UTF-8)
	    G_TYPE_STRING,   // name from filesystem (verbatim)
	    G_TYPE_STRING,   // tooltip text (cache)
	    G_TYPE_STRING,   // icon identifier (name or composite key)
	    G_TYPE_INT,      // mode (to identify directories)
	    G_TYPE_STRING,   // mimetype (further identification of files)
	    G_TYPE_STRING,   // mimefile (further identification of files)
	    G_TYPE_POINTER,  // stat record or NULL
	    G_TYPE_STRING,   // Preview path
	    G_TYPE_INT,      // Preview time
	    GDK_TYPE_PIXBUF); // Preview pixbuf

    heartbeat = 0;
    GList *directory_list = read_items (&heartbeat);
    insert_list_into_model(directory_list, list_store);

    return GTK_TREE_MODEL (list_store);
}

GList *
xfdir_local_c::read_items (gint *heartbeat) {
    GList *directory_list = NULL;
    if (chdir(path)){
	g_warning("xfdir_local_c::read_items(): chdir %s: %s\n", path, strerror(errno));
        return NULL;
    }
    NOOP( "readfiles: %s\n", path);
    DIR *directory = opendir(path);
    if (!directory) {
	g_warning("xfdir_local_c::read_items(): opendir %s: %s\n", path, strerror(errno));
	return NULL;
    }

// readdir way
//  mutex protect...
        //FIXME: must be mutex protected (OK) and threaded
    fprintf(stderr, "** requesting readdir mutex for %s...\n", path);
    pthread_mutex_t *mutex = data_p->get_readdir_mutex();
    pthread_mutex_lock(mutex);
    fprintf(stderr, "++ mutex for %s obtained.\n", path);
    struct dirent *d; // static pointer
    errno=0;
    NOOP( "shows hidden=%d\n", shows_hidden);
    while ((d = readdir(directory))  != NULL){
        NOOP( "%p  %s\n", d, d->d_name);
        if(strcmp (d->d_name, ".") == 0) continue;
        if(!shows_hidden && d->d_name[0] == '.' && strcmp (d->d_name, "..")) continue;
        xd_t *xd_p = get_xd_p(d);
	directory_list = g_list_prepend(directory_list, xd_p);
	if (heartbeat) {
	    (*heartbeat)++;
	    NOOP(stderr,"incrementing heartbeat records to %d\n", *heartbeat);
	}
    }
    if (errno) {
        fprintf(stderr, "read_files_local: %s\n", strerror(errno));
    }
// unlock mutex
    pthread_mutex_unlock(mutex);
    fprintf(stderr, "-- mutex for %s released.\n", path);

    closedir (directory);

    // At least the ../ record should have been read. If this
    // is not so, then a read error occurred.
    // (not uncommon in bluetoothed obexfs)
    if (!directory_list) {
	NOOP("read_files_local(): Count failed! Directory not read!\n");
    }
    directory_list = sort_directory_list (directory_list);
    return (directory_list);
}

GList *
xfdir_local_c::sort_directory_list(GList *list){
    // FIXME: get sort order and type
    gboolean do_stat = (g_list_length(list) <= MAX_AUTO_STAT);

    if (do_stat){
        GList *l;
        for (l=list; l && l->data; l=l->next){
            xd_t *xd_p = (xd_t *)l->data;
            xd_p->st = (struct stat *)calloc(1, sizeof(struct stat));
	    if (!xd_p->st) continue;
            if (stat(xd_p->d_name, xd_p->st)){
                DBG("xfdir_local_c::sort_directory_list: cannot stat %s (%s)\n", 
                        xd_p->d_name, strerror(errno));
                continue;
            } 
            xd_p->mimetype = mime_type(xd_p->d_name, xd_p->st); // using stat obtained above
            xd_p->mimefile = g_strdup(mime_file(xd_p->d_name)); // 
	}
    }
    // Default sort order:
    return g_list_sort (list,compare_by_name);
}

void 
xfdir_local_c::reload(const gchar *data){
    if (!data || !g_file_test(data, G_FILE_TEST_EXISTS)) {
        fprintf(stderr, "%s does not exist\n", data);
        return;
    }
    if (chdir(data)<0){
        fprintf(stderr, "chdir(%s): %s\n", data, strerror(errno));
        return;
    }
    if (strcmp(path, data)){
        stop_monitor();
        g_free(path);
        path = g_strdup(data);
        DBG("current dir is %s\n", path);
    }
    gtk_list_store_clear (GTK_LIST_STORE(treemodel));   
    heartbeat = 0;
    GList *directory_list = read_items (&heartbeat);
    while (gtk_events_pending()) gtk_main_iteration();
    insert_list_into_model(directory_list, GTK_LIST_STORE(treemodel));
    start_monitor(path, treemodel);

    
}
   

void
xfdir_local_c::insert_list_into_model(GList *data, GtkListStore *list_store){

    GList *directory_list = (GList *)data;
    dir_count = g_list_length(directory_list);
    if (dir_count > MAX_AUTO_STAT) large = TRUE;
    else large = FALSE;
    GList *l = directory_list;
    for (; l && l->data; l= l->next){
        while (gtk_events_pending()) gtk_main_iteration();
	xd_t *xd_p = (xd_t *)l->data;
        add_local_item(list_store, xd_p);
    }
    GList *p = directory_list;
    for (;p && p->data; p=p->next){
	xd_t *xd_p = (xd_t *)p->data;
        free_xd_p(xd_p);
    }
    g_list_free(directory_list);
}


gboolean
xfdir_local_c::popup(GtkTreePath *tpath){
    GtkTreeIter iter;
    gtk_tree_model_get_iter (treemodel, &iter, tpath);
    
    gchar *name;
    gchar *actual_name;
    gtk_tree_model_get (treemodel, &iter, 
            DISPLAY_NAME, &name, 
            ACTUAL_NAME, &actual_name, 
	    -1);
    // here we do the local xfdir popup menu method (overloaded)
    gchar *fullpath = g_build_filename(get_path(), actual_name, NULL);
    fprintf(stderr, "xfdir_local_c::popup: popup for %s (%s)\n", name, fullpath);
    g_free(name);
    g_free(actual_name);
    g_free(fullpath);
    return TRUE;
}


/////////////////////////////////////////////////////////////////////

static gint
compare_by_name (const void *a, const void *b) {
    // compare by name, directories or symlinks to directories on top
    const xd_t *xd_a = (const xd_t *)a;
    const xd_t *xd_b = (const xd_t *)b;

    if (strcmp(xd_a->d_name, "..")==0) return -1;
    if (strcmp(xd_b->d_name, "..")==0) return 1;

    gboolean a_cond = FALSE;
    gboolean b_cond = FALSE;

#ifdef HAVE_STRUCT_DIRENT_D_TYPE
    a_cond = (xd_a->d_type == DT_DIR);
    b_cond = (xd_b->d_type == DT_DIR);
#else
    if (xd_a->st && xd_b->st && 
	    (S_ISDIR(xd_a->st->st_mode) || S_ISDIR(xd_b->st->st_mode))) {
        a_cond = (S_ISDIR(xd_a->st->st_mode));
        b_cond = (S_ISDIR(xd_b->st->st_mode));
    } 
#endif

    if (a_cond && !b_cond) return -1; 
    if (!a_cond && b_cond) return 1;
    return strcasecmp(xd_a->d_name, xd_b->d_name);
}

//////////////////////////////////////////////////

gchar *
xfdir_local_c::get_path_info (GtkTreeModel *treemodel, GtkTreePath *tpath, const gchar *dir) {
    // retrieve cache st info, if available.    
    GtkTreeIter iter;
    struct stat st;
    struct stat *st_p;
    gtk_tree_model_get_iter (treemodel, &iter, tpath);
    gchar *file_path;
    gchar *mimetype;
    gchar *mimefile;
    gchar *g;
    gtk_tree_model_get (treemodel, &iter, 
	    COL_STAT, &st, 
	    COL_MIMETYPE, &mimetype, 
	    COL_MIMEFILE, &mimefile, 
	    COL_ACTUAL_NAME, &file_path, -1);
    gchar *full_path = g_build_filename(dir,file_path,NULL);
    g_free(file_path);
    
    if (st_p) memcpy(&st, st_p, sizeof(struct stat)); 
    else {
	if (lstat(full_path, &st) != 0) {
	    gchar *u = utf_string(full_path);
	    g_free(full_path);
	    g_free(mimetype);
	    g = g_strdup_printf(_("Cannot lstat \"%s\":\n%s\n"), u, strerror(errno));
	    g_free(u);
	    return g;
	} else {
	    st_p = (struct stat *)calloc(1, sizeof(struct stat));
	    if (st_p) {
		memcpy(st_p, &st, sizeof(struct stat));
		gtk_list_store_set (GTK_LIST_STORE(treemodel), &iter,
		    COL_STAT, st_p, 
		    -1);
	    }
	}
    }
    if (!mimefile){
	mimefile = mime_function(full_path, "mime_file");
	gtk_list_store_set (GTK_LIST_STORE(treemodel), &iter,
		COL_MIMEFILE, mimefile, 
	    -1);
    }
    
    g = g_strdup("");
    if(S_ISDIR (st.st_mode)) {
        gint files = count_files (full_path);
        gint hidden = count_hidden_files (full_path);
        if(files) {
            gchar *files_string = g_strdup_printf (ngettext (" (containing %'d item)", " (containing %'d items)", files),files);
    
            gchar *plural_string = 
                g_strdup_printf(ngettext ("%'u item","%'u items",hidden), hidden);
            gchar *hidden_string = 
                g_strdup_printf ("%s: %s.",_("Hidden"), plural_string);
            g_free(plural_string);
            g_free(g);
            g = g_strdup_printf ("%s\n%s", files_string, hidden_string);
            g_free(hidden_string);
            g_free (files_string);
        } else {
            g = g_strdup_printf ("%s", _("The location is empty."));
        }
    } 
    gchar *info = path_info (full_path, &st, g, mimetype, mimefile);
    g_free(full_path);
    g_free(mimetype);
    g_free(mimefile);
    g_free(g);
    g = info;
    
    return g;
}


/**
 * @path: directory path
 * Returns: file count within directory, including hidden files
 *
 * This function is non-recursive.
 **/
gint
xfdir_local_c::count_files (const gchar * file_path) {
    if(!g_file_test (file_path, G_FILE_TEST_IS_DIR)) return 0;
    DIR *directory;
    directory = opendir (file_path);
    if(!directory) return 0;
    gint count = 0;
    struct dirent *d;
    while((d = readdir (directory)) != NULL) {
        if(strcmp (d->d_name, ".") == 0)
            continue;
        if(strcmp (d->d_name, "..") == 0)
            continue;
        count++;
    }
    closedir (directory);
    return count;
}

/**
 * @path: directory path
 * Returns: hidden file count within directory
 *
 * This function is non-recursive.
 **/
gint
xfdir_local_c::count_hidden_files (const gchar * file_path) {
    if(!g_file_test (file_path, G_FILE_TEST_IS_DIR)) return 0;
    DIR *directory;
    directory = opendir (file_path);
    if(!directory) return 0;
    gint count = 0;
    struct dirent *d;
    while((d = readdir (directory)) != NULL) {
        if(strcmp (d->d_name, ".") == 0)
            continue;
        if(strcmp (d->d_name, "..") == 0)
            continue;
        if(d->d_name[0] != '.')
            continue;
        count++;
    }
    closedir (directory);
    return count;
}


gchar *
xfdir_local_c::path_info (const gchar *file_path, struct stat *st, const gchar *pretext, const gchar *mimedata, const gchar *mimefiledata) {
    gchar *pretext_stuff = NULL, *stat_stuff = NULL;
    gchar *info = NULL;
    if(!file_path) return NULL;
    if(S_ISLNK (st->st_mode)) {
	NOOP(stderr, "local lnk  type...\n");
        gchar lpath[_POSIX_PATH_MAX + 1];
        memset (lpath, 0, _POSIX_PATH_MAX + 1);
        if(readlink (file_path, lpath, _POSIX_PATH_MAX) > 0) {
            gchar *v = utf_string(lpath);
            gchar *escaped_markup = g_markup_escape_text(v, -1);
            g_free(v);
            gchar *q = utf_string (escaped_markup);
            g_free(escaped_markup);
            gchar *linkto=g_strdup_printf (_("Link to %s"), q);
            pretext_stuff = g_strdup_printf ("%s\n<i>%s</i>\n\n", linkto, pretext);
            g_free(linkto);
            g_free (q);
        }
    } 
    gchar *p = g_strdup_printf ("<i>%s</i>\n", pretext);
    pretext_stuff = p;
    gchar *mime_stuff = NULL;
	
    // mime overkill    
    gchar *mimetype;
    if (!mimedata) {
	mimetype = mime_type(file_path, st);
	if (!mimetype) mimetype = mime_function(file_path, "mime_magic");
	if (!mimetype)mimetype = g_strdup(_("unknown"));
    } else {
	mimetype = g_strdup(mimedata);
    }
        
    gchar *u = g_strdup(mimefiledata);
    gchar *mimefile = wrap_utf_string(u, 40);
    g_free(u);
    gchar *mimeencoding = mime_function(file_path, "mime_encoding");

    if (!mimefile)mimefile = g_strdup(_("unknown"));    
    if (!mimeencoding)mimeencoding = g_strdup(_("unknown"));    

    if (strstr(mimetype, "x-trash") || 
	file_path[strlen(file_path)-1] =='~' ||
	file_path[strlen(file_path)-1] =='%' ) {
	g_free(mimefile);
	mimefile = g_strdup(_("Backup file"));
    }
    mime_stuff = g_strdup_printf("<b>%s</b>:\n%s\n<b>%s</b>: %s\n<b>%s</b>: %s\n\n",
	    _("File Type"), mimefile,
	    _("MIME Type"), mimetype,
	    _("Encoding"), mimeencoding);
    g_free (mimetype);
    g_free (mimefile);
    g_free (mimeencoding);
        
    gchar *grupo=group_string(st);
    gchar *owner=user_string(st);
    gchar *tag = sizetag ((off_t) st->st_size, -1);

    //    gchar *ss= rfm_time_to_string(st->st_mtime);   

    //gchar *t = g_path_get_dirname (file_path);
    //gchar *v = utf_string(t);
    //gchar *escaped_markup = g_markup_escape_text(v, -1);
    //g_free(v);
    //gchar *dirname = utf_string (escaped_markup);
    //g_free(t);
    //g_free(escaped_markup);
    
    gchar *mode_string_s=mode_string (st->st_mode);
    stat_stuff = g_strdup_printf (
            "<b>%s/%s</b>: %s/%s\n<b>%s</b>: %s\n<b>%s</b>: %s",
             _("Owner"),_("Group"), owner, grupo,
            _("Permissions"), mode_string_s,
            _("Size"),  tag);

    g_free (owner);
    g_free (grupo);
    g_free (tag);
    //g_free (dirname);
    g_free (mode_string_s);

    gchar buf[1024];

    gchar *date_string_s=date_string(st->st_ctime);

    sprintf (buf, "<b>%s :</b> %s", _("Status Change"), date_string_s);
    g_free(date_string_s);

    gchar *s3 = g_strconcat (stat_stuff, "\n", buf, NULL);
    g_free (stat_stuff);
    stat_stuff = s3;

    date_string_s=date_string(st->st_mtime);
    sprintf (buf, "<b>%s</b> %s", _("Modification Time :"), date_string_s);
    g_free(date_string_s);


    s3 = g_strconcat (stat_stuff, "\n", buf, NULL);
    g_free (stat_stuff);
    stat_stuff = s3;

    date_string_s=date_string(st->st_atime);
    sprintf (buf, "<b>%s</b> %s", _("Access Time :"), date_string_s);
    g_free(date_string_s);

    s3 = g_strconcat (stat_stuff, "\n", buf, NULL);
    g_free (stat_stuff);
    stat_stuff = s3;

    gchar *hard_links = g_strconcat(_("Links")," (", _("hard"), ")", NULL);
    s3 = g_strdup_printf ("%s\n\n<b>%s</b>: %ld\n<b>%s</b>: %ld",
            stat_stuff, hard_links,
            (long)st->st_nlink, _("Inode"), (long)st->st_ino);
    g_free(hard_links);
            
    g_free (stat_stuff);
    stat_stuff = s3;

    if(!pretext_stuff) pretext_stuff = g_strdup ("");
    if(!mime_stuff) mime_stuff = g_strdup ("");
    if(!stat_stuff) stat_stuff = g_strdup ("");
    info = g_strconcat (pretext_stuff, mime_stuff, stat_stuff, NULL);
    g_free (pretext_stuff);
    g_free (stat_stuff);
    g_free (mime_stuff);
   return info;
}




gchar *
xfdir_local_c::mode_string (mode_t mode) {
    gchar *str=(gchar *)malloc(13);
    if (!str) g_error("malloc: %s", strerror(errno));
    str[0] = ftypelet (mode);
    str[1] = mode & S_IRUSR ? 'r' : '-';
    str[2] = mode & S_IWUSR ? 'w' : '-';
    str[3] = mode & S_IXUSR ? 'x' : '-';
    str[4] = mode & S_IRGRP ? 'r' : '-';
    str[5] = mode & S_IWGRP ? 'w' : '-';
    str[6] = mode & S_IXGRP ? 'x' : '-';
    str[7] = mode & S_IROTH ? 'r' : '-';
    str[8] = mode & S_IWOTH ? 'w' : '-';
    str[9] = mode & S_IXOTH ? 'x' : '-';
    if(mode & S_ISUID)
        str[3] = mode & S_IXUSR ? 's' : 'S';
    if(mode & S_ISGID)
        str[6] = mode & S_IXGRP ? 's' : 'S';
    if(mode & S_ISVTX)
        str[9] = mode & S_IXOTH ? 't' : 'T';
    str[10] = 0;
    return (str);
}
    

gchar *
xfdir_local_c::user_string (struct stat *st) {
    pthread_mutex_lock(&user_string_mutex);
    struct passwd *p;
    gchar *user_string;
    if((p = getpwuid (st->st_uid)) != NULL)
            user_string = g_strdup(p->pw_name);
        else if((gint)st->st_uid < 0)
            user_string = g_strdup("");
        else
            user_string = g_strdup_printf("%d", (gint)st->st_uid);
    pthread_mutex_unlock(&user_string_mutex);
    return user_string;
}



gchar *
xfdir_local_c::group_string (struct stat *st) {
    pthread_mutex_lock(&group_string_mutex);
    struct group *g;
    gchar *group_string;
    if((g =  getgrgid(st->st_gid)) != NULL)
            group_string = g_strdup(g->gr_name);
    else
        group_string = g_strdup_printf("%d", (gint)st->st_gid);
    pthread_mutex_unlock(&group_string_mutex);
    return group_string;
}

gchar *
xfdir_local_c::date_string (time_t the_time) {
    pthread_mutex_lock(&date_string_mutex);

#ifdef HAVE_LOCALTIME_R
        struct tm t_r;
#endif
        struct tm *t;

#ifdef HAVE_LOCALTIME_R
        t = localtime_r (&the_time, &t_r);
#else
        t = localtime (&the_time);
#endif
        gchar *date_string=
	    g_strdup_printf ("%04d/%02d/%02d  %02d:%02d", t->tm_year + 1900,
                 t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min);
    pthread_mutex_unlock(&date_string_mutex);

    return date_string;
}


gchar *
xfdir_local_c::sizetag (off_t tama, gint count) {
    gchar *tag = _("bytes");
    gchar *buf = NULL;
    double utama = tama;

    buf = NULL;
    if(utama > 0) {
        if(utama >= (off_t)1000 * 1000 * 1000) {
            utama /= ((off_t)1000 * 1000 * 1000);
            tag = _("Gigabytes");
        } else if(utama >= 1000 * 1000) {
            utama /= (1000 * 1000);
            tag = _("Megabytes");
        } else if(utama >= 1000) {
            utama /= 1000;
            tag = _("Kilobytes");
        }
        if(count <= 0) {
            /* format for size column of regular files */
            buf = g_strdup_printf ("%.2lf %s", utama, tag);
        } else {
            gchar *plural_text=
                g_strdup_printf (ngettext ("%'u item", "%'u items", 
                            count),count);
	    if (tama < 1000) {
		buf = g_strdup_printf ("%s: %.0lf %s.", plural_text,
                    utama, tag);
	    } else {
		buf = g_strdup_printf ("%s: %.2lf %s.", plural_text,
                    utama, tag);
	    }
            g_free(plural_text);
    
        }
    } else {
        if(count <=0) {
            buf = g_strdup_printf (_("The location is empty."));
        } else {
            buf=
                g_strdup_printf (ngettext ("%'u item", "%'u items", count),
                        count);
        }
    }
    return buf;
}


/* Return a character indicating the type of file described by
   file mode BITS:
   'd' for directories
   'D' for doors
   'b' for block special files
   'c' for character special files
   'n' for network special files
   'm' for multiplexor files
   'M' for an off-line (regular) file
   'l' for symbolic links
   's' for sockets
   'p' for fifos
   'C' for contigous data files
   '-' for regular files
   '?' for any other file type.  */


gchar
xfdir_local_c::ftypelet (mode_t bits) {
#ifdef S_ISBLK
    if(S_ISBLK (bits)) return 'b';
#endif
    if(S_ISCHR (bits)) return 'c';
    if(S_ISDIR (bits)) return 'd';
    if(S_ISREG (bits)) return '-';
#ifdef S_ISFIFO
    if(S_ISFIFO (bits)) return 'p';
#endif
#ifdef S_ISLNK
    if(S_ISLNK (bits)) return 'l';
#endif
#ifdef S_ISSOCK
    if(S_ISSOCK (bits)) return 's';
#endif
#ifdef S_ISMPC
    if(S_ISMPC (bits)) return 'm';
#endif
#ifdef S_ISNWK
    if(S_ISNWK (bits)) return 'n';
#endif
#ifdef S_ISDOOR
    if(S_ISDOOR (bits)) return 'D';
#endif
#ifdef S_ISCTG
    if(S_ISCTG (bits)) return 'C';
#endif

    /* The following two tests are for Cray DMF (Data Migration
       Facility), which is a HSM file system.  A migrated file has a
       `st_dm_mode' that is different from the normal `st_mode', so any
       tests for migrated files should use the former.  */

#ifdef S_ISOFD
        /* off line, with data  */
    if(S_ISOFD (bits)) return 'M';
#endif
#ifdef S_ISOFL
    /* off line, with no data  */
    if(S_ISOFL (bits)) return 'M';
#endif
    return '?';
}


const gchar *
xfdir_local_c::get_xfdir_iconname(void){
    if (strcmp(path, g_get_home_dir())==0) {
	return "user-home";
    }
    gchar *d = g_path_get_dirname(path);
    if (strcmp(d, g_get_home_dir())==0) {
	g_free(d);
	gchar *b = g_path_get_basename(path);
	const gchar *iconname = get_home_iconname(b);
	g_free(b);
	return iconname;
    }
    g_free(d);
    return "folder";
}


