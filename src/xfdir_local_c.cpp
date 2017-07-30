#include "window_c.hpp"
#include "view_c.hpp"
#include "local_dnd_c.hpp"
#include "xfdir_local_c.hpp"

pthread_mutex_t xfdir_local_c::readdir_mutex=PTHREAD_MUTEX_INITIALIZER;

static gint compare_by_name (const void *, const void *);
static gboolean free_stat_func (GtkTreeModel *model,
                            GtkTreePath *path,
                            GtkTreeIter *iter,
                            gpointer data);

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

xfdir_local_c::xfdir_local_c(const gchar *data, void *dataV): 
    local_monitor_c(data), local_dnd_c(dataV)
{
    view_v = dataV;
    create_menu();
    view_c *view_p = (view_c *)view_v;
    shows_hidden = view_p->shows_hidden();
    NOOP( "data2=%d\n", data2);
    treemodel = mk_tree_model();
    user_string_mutex=PTHREAD_MUTEX_INITIALIZER;
    group_string_mutex=PTHREAD_MUTEX_INITIALIZER;
    date_string_mutex=PTHREAD_MUTEX_INITIALIZER;
    start_monitor(data, treemodel);
}

xfdir_local_c::~xfdir_local_c(void){
    destroy_tree_model();
    gtk_widget_destroy(GTK_WIDGET(selection_menu));
    gtk_widget_destroy(GTK_WIDGET(directory_menu));
}

void
xfdir_local_c::destroy_tree_model(void){
    gtk_tree_model_foreach (treemodel, free_stat_func, NULL); 
}

void
xfdir_local_c::open_new_tab(void){
    view_c *view_p = (view_c *)view_v;
    window_c *window_p = (window_c *)view_p->get_window_v();
    window_p->create_new_page(path);
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
        view_p->get_lpterm_p()->print_error(g_strdup_printf("%s = NULL)\n", _("Mime Type")));
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


#if 0
void
xfdir_c::item_activated (GtkIconView *iconview, GtkTreePath *tpath, void *data)
{
    view_c *view_p = (view_c *)data;
    GtkTreeModel *tree_model = gtk_icon_view_get_model (iconview);
    GtkTreeIter iter;
    if (!gtk_tree_model_get_iter (tree_model, &iter, tpath)) return;
    
    gchar *full_path;
    gchar *ddname;
    gtk_tree_model_get (tree_model, &iter,
                          ACTUAL_NAME, &ddname,-1);
    if (strcmp(ddname, "..")==0 && strcmp(path, "/")==0){
        full_path = g_strdup("xffm:root");
    } else {
        if (g_file_test(path, G_FILE_TEST_IS_DIR)) 
            full_path = g_strconcat(path, G_DIR_SEPARATOR_S, ddname, NULL);
        else 
            full_path = g_strdup(ddname);
    }
    TRACE("dname = %s, path = %s\n", ddname, full_path);


    if (g_file_test(full_path, G_FILE_TEST_IS_DIR)){
        view_p->reload(full_path);
    } else {
        // uses mimetype, should be delegated to xfdir_local_c...
        gchar *mimetype = mime_type(full_path);
        const gchar *command = mime_command(mimetype);
        const gchar *command1 = mime_command_text(mimetype);
        const gchar *command2 = mime_command_text2(mimetype);
        view_p->get_lpterm_p()->print_error(g_strdup_printf("%s: %s\n", mimetype, command));
        view_p->get_lpterm_p()->print_error(g_strdup_printf("%s: %s\n", mimetype, command1));
        view_p->get_lpterm_p()->print_error(g_strdup_printf("%s: %s\n", mimetype, command2));
        g_free(mimetype);
        if (strncmp(full_path,"xffm:", strlen("xffm:"))==0){
            gchar *module = full_path + strlen("xffm:");
            DBG("module = \"%s\"\n", module);
            if (strcmp(module, "root")==0){
                view_p->root();
            } else if (strcmp(module, "fstab")==0){
                // load fstab root
            } 
            // other modules:
            // cifs
            // nfs
            // ecryptfs
            // sshfs
            // pacman
        }
    }
    g_free(ddname);
    g_free(full_path);
}
#endif
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
    pthread_mutex_lock(&readdir_mutex);
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
    pthread_mutex_unlock(&readdir_mutex);
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
            xd_p->mimefile = g_strdup(mime_magic_c::mime_file(xd_p->d_name)); // 
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
        
    stop_monitor();
    if (strcmp(path, data)){
	g_free(path);
	path = g_strdup(data);
    }
    DBG("current dir is %s\n", path);
    
    destroy_tree_model();
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

/*
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
*/

/////////////////////////////////////////////////////////////////////

static gboolean free_stat_func (GtkTreeModel *model,
                            GtkTreePath *path,
                            GtkTreeIter *iter,
                            gpointer data){

    struct stat *st_p;
    gchar *name;
    gtk_tree_model_get(model, iter, 
            COL_STAT, &st_p, 
	    -1);
    g_free(st_p);
    return TRUE;
}

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

gboolean
xfdir_local_c::set_dnd_data(GtkSelectionData * selection_data, GList *selection_list){
    _set_dnd_data(selection_data, selection_list, treemodel, get_path(),(gint)ACTUAL_NAME);
    return TRUE;
}

gboolean
xfdir_local_c::receive_dnd(const gchar *target, GtkSelectionData *data, GdkDragAction action){
    _receive_dnd(get_path(), target, data, action);
    return TRUE;
}

gchar *
xfdir_local_c::get_path_info (GtkTreeModel *treemodel, GtkTreePath *tpath, const gchar *dir) {
    // retrieve cache st info, if available.    
    GtkTreeIter iter;
    struct stat st;
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
    
    if (lstat(full_path, &st) != 0) {
        gchar *u = utf_string(full_path);
        g_free(full_path);
        g_free(mimetype);
        g = g_strdup_printf(_("Cannot lstat \"%s\":\n%s\n"), u, strerror(errno));
        g_free(u);
        return g;
    } else {
        struct stat *st_p = (struct stat *)calloc(1, sizeof(struct stat));
        if (!st_p) g_error("xfdir_local_c:: cannot allocate struct stat pointer\n");
        memcpy(st_p, &st, sizeof(struct stat));
        gtk_list_store_set (GTK_LIST_STORE(treemodel), &iter, COL_STAT, st_p, -1);
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
    if (!pretext_stuff) pretext_stuff = g_strdup_printf ("<i>%s</i>\n", pretext);;
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
        
#if 1
    gchar *u = g_strdup(mimefiledata);
    const gchar *cu = chop_excess(u);
    gchar *mimefile = g_strdup(cu);
    g_free(u);
#else
    gchar *u = g_strdup(mimefiledata);
    gchar *mimefile = wrap_utf_string(u, 40);
    g_free(u);
#endif

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


static void menu_option(GtkWidget *menu_item, gpointer data){
    const gchar *what = (const gchar *)data;
    GtkWidget *menu = (GtkWidget *)g_object_get_data(G_OBJECT(menu_item), "menu");
    xfdir_local_c *xfdir_local_p = (xfdir_local_c *)g_object_get_data(G_OBJECT(menu), "xfdir_local_p");
    fprintf(stderr, "menu option: %s\n", what);
    if (strcmp(what, _("Open in New Tab"))==0){xfdir_local_p->open_new_tab();}
    else if (strcmp(what, _("Open with"))==0){}
    else if (strcmp(what, _("Search"))==0){}
    else if (strcmp(what, _("Cut"))==0){}
    else if (strcmp(what, _("Copy"))==0){}
    else if (strcmp(what, _("Paste"))==0){}
    else if (strcmp(what, _("Delete"))==0){}
    else if (strcmp(what, _("Shred"))==0){}
    else if (strcmp(what, _("bcrypt"))==0){} 
}

void
xfdir_local_c::create_menu(void){
   const gchar *directory_items[]={
        N_("Open in New Tab"),
        N_("Cut"),
        N_("Copy"),
        N_("Paste"),
        N_("Delete"),
        N_("Shred"),
        N_("bcrypt"),
         NULL};
    const gchar *selection_items[]={
        N_("Open with"),
        N_("Cut"),
        N_("Copy"),
        N_("Delete"),
        N_("Shred"),
        N_("bcrypt"),
         NULL};

    selection_menu = mk_menu(selection_items, menu_option);
    directory_menu = mk_menu(directory_items, menu_option);
    g_object_set_data(G_OBJECT(selection_menu),"xfdir_local_p", (void *)this);
    g_object_set_data(G_OBJECT(directory_menu),"xfdir_local_p", (void *)this);
}

gboolean
xfdir_local_c::popup(GtkTreePath *tpath){
    GtkIconView *iconview = (GtkIconView *)g_object_get_data(G_OBJECT(treemodel), "iconview");
    GList *selection_list = gtk_icon_view_get_selected_items (iconview);
    gint selected_items = 0;
    if (selection_list){
        selected_items = g_list_length(selection_list);
        g_list_free_full (selection_list, (GDestroyNotify) gtk_tree_path_free);
    }

    if (!tpath && !selected_items){
	view_popup();
    }
    GtkTreeIter iter;
    gtk_tree_model_get_iter (treemodel, &iter, tpath);
    
    gchar *name;
    gchar *actual_name;
    gtk_tree_model_get (treemodel, &iter, 
            DISPLAY_NAME, &name, 
            ACTUAL_NAME, &actual_name, 
	    -1);
    gchar *p = g_build_filename(path, actual_name, NULL);
    // here we do the particular xfdir popup menu method (overloaded)
    fprintf(stderr, "xfdir_c::popup: popup for %s (%s)\n", name, actual_name);
    if (selected_items <= 1 && g_file_test(p, G_FILE_TEST_IS_DIR)){
        gtk_menu_popup(directory_menu, NULL, NULL, NULL, NULL, 3, gtk_get_current_event_time());
    } else {
        gtk_menu_popup(selection_menu, NULL, NULL, NULL, NULL, 3, gtk_get_current_event_time());
    }
    
    
    g_free(name);
    g_free(actual_name);
    g_free(p);
    return TRUE;
}

