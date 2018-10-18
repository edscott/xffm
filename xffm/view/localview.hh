#ifndef XF_LOCALVIEW__HH
# define XF_LOCALVIEW__HH
// FIXME: determine HAVE_STRUCT_DIRENT_D_TYPE on configure (for freebsd)
#define HAVE_STRUCT_DIRENT_D_TYPE 1

// FIXME: #include "lite.hh"
#include "common/util.hh"
// FIXME: #include "common/mime.hh"

typedef struct xd_t{
    gchar *d_name;
    gchar *path;
    unsigned char d_type;
    struct stat *st;
    const gchar *mimetype;
    const gchar *mimefile;
}xd_t;
static pthread_mutex_t readdir_mutex=PTHREAD_MUTEX_INITIALIZER;

#define MAX_AUTO_STAT 500

#define O_ALL(x) ((S_IROTH & x) && (S_IWOTH & x) &&  (S_IXOTH & x))
#define G_ALL(x) ((S_IRGRP & x) && (S_IWGRP & x) &&  (S_IXGRP & x))
#define U_ALL(x) ((S_IRUSR & x) && (S_IWUSR & x) &&  (S_IXUSR & x))
#define O_RX(x) ((S_IROTH & x) &&  (S_IXOTH & x))
#define G_RX(x) ((S_IRGRP & x) &&  (S_IXGRP & x))
#define U_RX(x) ((S_IRUSR & x) &&  (S_IXUSR & x))
#define O_RW(x) ((S_IROTH & x) && (S_IWOTH & x))
#define G_RW(x) ((S_IRGRP & x) && (S_IWGRP & x))
#define U_RW(x) ((S_IRUSR & x) && (S_IWUSR & x))
#define O_RX(x) ((S_IROTH & x) && (S_IXOTH & x))
#define G_RX(x) ((S_IRGRP & x) && (S_IXGRP & x))
#define U_RX(x) ((S_IRUSR & x) && (S_IXUSR & x))
#define O_R(x) (S_IROTH & x)
#define G_R(x) (S_IRGRP & x)
#define U_R(x) (S_IRUSR & x)
#define MY_FILE(x) (x == geteuid())
#define MY_GROUP(x) (x == getegid())

namespace xf
{
static GtkMenu *localPopUp=NULL;


template <class Type>
class LocalView {
    
    using gtk_c = Gtk<Type>;
    using pixbuf_c = Pixbuf<Type>;
    using util_c = Util<Type>;

public:
    static void
    toggleItem(GtkCheckMenuItem *menuItem, gpointer data)
    {
        auto item = (const gchar *)data;
        gint value; 
        if (Dialog<Type>::getSettingInteger("LocalView", item) > 0){
            value = 0;
            gtk_check_menu_item_set_active(menuItem, FALSE);
        } else {
            value = 1;
            gtk_check_menu_item_set_active(menuItem, TRUE);
        }
        Dialog<Type>::saveSettings("LocalView", item, value);

    }
    static void
    noop(GtkMenuItem *menuItem, gpointer data)
    {
        DBG("noop\n")
    }

    static GtkMenu *popUp(void){
        if (localPopUp) return localPopUp;
        auto menu = GTK_MENU(gtk_menu_new());
         menuCheckItem_t item[]={
            {N_("Show hidden files"), (void *)toggleItem, 
                (void *) "ShowHidden", "ShowHidden"},
            {N_("Show Backup Files"), (void *)toggleItem, 
                (void *) "ShowBackups", "ShowBackups"},
            
            {N_("Add bookmark"), (void *)noop, (void *) menu, FALSE},
            {N_("Remove bookmark"), (void *)noop, (void *) menu, FALSE},
            {N_("Create a new empty folder inside this folder"), (void *)noop, (void *) menu, FALSE},
            {N_("Open in New Window"), (void *)noop, (void *) menu, FALSE},
            {N_("Reload"), (void *)noop, (void *) menu, FALSE},
            {N_("Close"), (void *)noop, (void *) menu, FALSE},
            // main menu items
            //{N_("Open in New Tab"), (void *)noop, (void *) menu},
            //{N_("Home"), (void *)noop, (void *) menu},
            //{N_("Open terminal"), (void *)noop, (void *) menu},
            //{N_("About"), (void *)noop, (void *) menu},
            //
            //common buttons /(also an iconsize +/- button)
            //{N_("Paste"), (void *)noop, (void *) menu},
            //{N_("Sort data in ascending order"), (void *)noop, (void *) menu},
            //{N_("Sort data in descending order"), (void *)noop, (void *) menu},
            //{N_("Sort case insensitive"), (void *)noop, (void *) menu},
            
            //{N_("Select All"), (void *)noop, (void *) menu},
            //{N_("Invert Selection"), (void *)noop, (void *) menu},
            //{N_("Unselect"), (void *)noop, (void *) menu},
            //{N_("Select Items Matching..."), (void *)noop, (void *) menu},
            //{N_("Unselect Items Matching..."), (void *)noop, (void *) menu},
            //{N_("Sort by name"), (void *)noop, (void *) menu},
            //{N_("Default sort order"), (void *)noop, (void *) menu},
            //{N_("Sort by date"), (void *)noop, (void *) menu},
            //{N_("Sort by size"), (void *)noop, (void *) menu},
            //{N_("View as list""), (void *)noop, (void *) menu},
            {NULL,NULL,NULL, FALSE}};
        
        auto p = item;
        gint i;
        for (i=0;p && p->label; p++,i++){
            GtkWidget *v;
            if (p->toggleID){
                v = gtk_check_menu_item_new_with_label(_(p->label));
                if (Dialog<Type>::getSettingInteger("LocalView", p->toggleID) > 0){
                   gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(v), TRUE);
                } 
            }
            else v = gtk_menu_item_new_with_label (_(p->label));
            gtk_container_add (GTK_CONTAINER (menu), v);
            g_signal_connect ((gpointer) v, "activate", MENUITEM_CALLBACK (p->callback), p->callbackData);
            gtk_widget_show (v);
        }
        gtk_widget_show (GTK_WIDGET(menu));
        return menu;
        
    }
    
    static void selectables(GtkIconView *iconview){
        GtkTreePath *tpath = gtk_tree_path_new_first ();
	GtkTreeModel *treeModel = gtk_icon_view_get_model (iconview);
	GtkTreeIter iter;
	if (!gtk_tree_model_get_iter (treeModel, &iter, tpath)) {
            gtk_tree_path_free(tpath);
            return ;
        }
        gchar *name;
	gtk_tree_model_get (treeModel, &iter, ACTUAL_NAME, &name, -1);
        gboolean retval = TRUE;
        if (strcmp(name, "..")==0 )retval = FALSE;
        g_free(name);
        if (!retval) {
            gtk_icon_view_unselect_path (iconview,tpath);
        }
        gtk_tree_path_free(tpath);
            
        return ;
    }


    static const gchar *
    get_xfdir_iconname(void){
	return "system-file-manager";
    }

    static gchar *
    item_activated (GtkIconView *iconview, GtkTreePath *tpath, void *data)
    {
	    DBG("LocalView::item activated\n");
	GtkTreeModel *treeModel = gtk_icon_view_get_model (iconview);
	GtkTreeIter iter;
	if (!gtk_tree_model_get_iter (treeModel, &iter, tpath)) return NULL;
	gchar *path=NULL;
	gtk_tree_model_get (treeModel, &iter, 
		PATH, &path,
		-1);

	if (g_file_test(path, G_FILE_TEST_IS_DIR)) {
	   /* gchar *basename = g_path_get_basename(path);
	    if (strcmp(basename, "..")==0){
		gchar *current=g_path_get_dirname(path);
		gchar *up=g_path_get_dirname(current);
		g_free(current);
		g_free(path);
		path=up;
	    }*/
	    WARN("FIXME: stop monitor (if running) \"%s\"\n", path);
	    // FIXME: stop monitor (if running)
	    // FIXME: reload treemodel
	    return(path);
	    // FIXME: restart Monitor
	} else {
	    WARN("FIXME: open or execute file \"%s\"\n", path);
	    // FIXME: open or execute file
	}

	//view_p->reload(name);
	g_free(path);
	return NULL;
    }

    // This mkTreeModel should be static...
    static gboolean
    loadModel (GtkIconView *iconView, const gchar *path)
    {
        if (!g_file_test(path, G_FILE_TEST_EXISTS)){
            ERROR("loadModel. %s does not exist\n", path);
            return FALSE;
        }
        if (!g_file_test(path, G_FILE_TEST_IS_DIR)){
            WARN("localView.hh::loadModel(): here we should open the file with app or dialog\n");
            return FALSE;
        }
        g_object_set_data(G_OBJECT(iconView), "iconViewType", (void *)"LocalView");
                
        gtk_icon_view_set_selection_mode (iconView,GTK_SELECTION_MULTIPLE);      
        
        auto treeModel = gtk_icon_view_get_model (iconView);
        while (gtk_events_pending()) gtk_main_iteration();
 	GtkTreeIter iter;
	if (gtk_tree_model_get_iter_first (treeModel, &iter)){
	    while (gtk_list_store_remove (GTK_LIST_STORE(treeModel),&iter));
	}


        int heartbeat = 0;
    
        GList *directory_list = read_items (path, &heartbeat);
        insert_list_into_model(directory_list, GTK_LIST_STORE(treeModel));
		

	return TRUE;
    }

private:




    //FIXME: must be done by non main thread (already mutex protected)
    static GList *
    read_items (const gchar *path,  gint *heartbeat)
    {
        GList *directory_list = NULL;
        if (!g_file_test(path, G_FILE_TEST_IS_DIR)){
            WARN("read_items(): g_file_test(%s, G_FILE_TEST_IS_DIR) failed\n", path);
            return NULL;
        }
        TRACE( "readfiles: %s\n", path);
        DIR *directory = opendir(path);
        if (!directory) {
            WARN("xfdir_local_c::read_items(): opendir %s: %s\n", path, strerror(errno));
            return NULL;
        }
    //  mutex protect...
        DBG("** requesting readdir mutex for %s...\n", path);
        pthread_mutex_lock(&readdir_mutex);
        DBG( "++ mutex for %s obtained.\n", path);
        struct dirent *d; // static pointer
        errno=0;
        TRACE( "shows hidden=%d\n", showHidden);
        while ((d = readdir(directory))  != NULL){
            TRACE( "%p  %s\n", d, d->d_name);
            if(strcmp (d->d_name, ".") == 0) continue;
            if (strcmp(path,"/")==0 && strcmp (d->d_name, "..") == 0) continue;
            xd_t *xd_p = get_xd_p(path, d);
            directory_list = g_list_prepend(directory_list, xd_p);
            if (heartbeat) {
                (*heartbeat)++;
                TRACE(stderr,"incrementing heartbeat records to %d\n", *heartbeat);
            }
        }
        if (errno) {
            WARN("read_files_local: %s\n", strerror(errno));
        }
    // unlock mutex
        pthread_mutex_unlock(&readdir_mutex);
        DBG("-- mutex for %s released.\n", path);

        closedir (directory);

        // At least the ../ record should have been read. If this
        // is not so, then a read error occurred.
        // (not uncommon in bluetoothed obexfs)
        if (!directory_list) {
            WARN("read_files_local(): Count failed! Directory not read!\n");
        }
        directory_list = sort_directory_list (directory_list);
        return (directory_list);
    }

    // Convert a dirent entry into a xd_t structure.
    static xd_t *
    get_xd_p(const gchar *directory, struct dirent *d){
        xd_t *xd_p = (xd_t *)calloc(1,sizeof(xd_t));
        xd_p->d_name = g_strdup(d->d_name);
        if (strcmp(d->d_name, "..")==0){
	    xd_p->path = g_path_get_dirname(directory);
        } else if (strcmp(directory, G_DIR_SEPARATOR_S)==0){
            xd_p->path = g_strconcat(G_DIR_SEPARATOR_S, d->d_name, NULL);
        } else {
            xd_p->path = g_strconcat(directory, G_DIR_SEPARATOR_S, d->d_name, NULL);
        }

        // These will be filled in later by thread:
        xd_p->mimetype = NULL;
        xd_p->mimefile = NULL;
        xd_p->st = NULL;
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
        xd_p->d_type = d->d_type;
#else
        xd_p->d_type = 0;
#endif
        return xd_p;
    }

    static void
    free_xd_p(xd_t *xd_p){
	// The following 2 must be "const gchar *"
        //g_free(xd_p->mimefile);
        //g_free(xd_p->mimetype);
        g_free(xd_p->d_name);
        g_free(xd_p->path);
        g_free(xd_p);
    }

    static GList *
    sort_directory_list(GList *list){
        // FIXME: get sort order and type
        gboolean do_stat = (g_list_length(list) <= MAX_AUTO_STAT);

        if (do_stat){
            GList *l;
            for (l=list; l && l->data; l=l->next){
                xd_t *xd_p = (xd_t *)l->data;
                xd_p->st = (struct stat *)calloc(1, sizeof(struct stat));
                if (!xd_p->st) continue;
                if (stat(xd_p->path, xd_p->st)){
                    DBG("xfdir_local_c::sort_directory_list: cannot stat %s (%s)\n", 
                            xd_p->path, strerror(errno));
                    continue;
                }
               // FIXME: enamble mime_c 
                //xd_p->mimetype = mime_c::mime_type(xd_p->d_name, xd_p->st); // using stat obtained above
                //xd_p->mimefile = g_strdup(mime_c::mime_file(xd_p->d_name)); // 
            }
        }
        // Default sort order:
        return g_list_sort (list,compare_by_name);
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

    static gint
    insert_list_into_model(GList *data, GtkListStore *list_store){
        GList *directory_list = (GList *)data;
        gint dir_count = g_list_length(directory_list);
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
        return dir_count;
    }
    
    static gboolean backupType(const gchar *file){
        if (!file) return FALSE;
        // GNU backup type:
         if(file[strlen (file) - 1] == '~' || 
                 file[strlen (file) - 1] == '%'|| 
                 file[strlen (file) - 1] == '#') return TRUE;
        // MIME backup type:
        const gchar *e = strrchr(file, '.');
        if (e){
            if (strcmp(e,".old")==0) return TRUE;
            else if (strcmp(e,".bak")==0) return TRUE;
            else if (strcmp(e,".sik")==0) return TRUE;
        }
        return FALSE;
    }

    static void
    add_local_item(GtkListStore *list_store, xd_t *xd_p){
        // FIXME: itemsHash_ is on a object basis, not static item...
        // if it already exists, do nothing
	// FIXME: enable itemsHash_ for monitor function.
        //if (g_hash_table_lookup(itemsHash_, (void *)xd_p->d_name)){    
        //    DBG("local_monitor_c::not re-adding %s\n", xd_p->d_name);
        //  return;
        //}

        //FIXME need for shows_hidden only in monitor_ function...
        //      monitor must reload when showHidden changes...
        gboolean showHidden = (Dialog<Type>::getSettingInteger("LocalView", "ShowHidden") > 0);
        if (!showHidden && xd_p->d_name[0] == '.'  && strcmp("..", xd_p->d_name)){
            return;
        }
        gboolean showBackups = (Dialog<Type>::getSettingInteger("LocalView", "ShowBackups") > 0);
        if (!showBackups && backupType(xd_p->d_name)){
            return;
        }
        
        GtkTreeIter iter;
        gtk_list_store_append (list_store, &iter);
        gchar *utf_name = util_c::utf_string(xd_p->d_name);
        // plain extension mimetype fallback
        // FIXME: enable mime_c template
        //if (!xd_p->mimetype) xd_p->mimetype = mime_c::mime_type(xd_p->d_name); 
        gchar *icon_name = get_iconname(xd_p);
	TRACE("icon name for %s is %s\n", xd_p->d_name, icon_name);
        
        // chop file extension (will now appear on the icon). (XXX only for big icons)
        gboolean is_dir;
        gboolean is_reg_not_link;
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
        is_dir = (xd_p->d_type == DT_DIR);
        is_reg_not_link = (xd_p->d_type == DT_REG && !(xd_p->d_type == DT_LNK));
#else 
        is_dir = (xd_p->st && S_ISDIR(xd_p->st->st_mode));
        is_reg_not_link = (xd_p->st && S_ISREG(xd_p->st->st_mode) && !S_ISLNK(xd_p->st->st_mode));
#endif
        if (is_reg_not_link) {
            gchar *t = g_strdup(xd_p->d_name);
            if (strchr(t, '.') && strrchr(t, '.') != t){
                *strrchr(t, '.') = 0;
                g_free(utf_name);
                utf_name = util_c::utf_string(t);
                g_free(t);
            }
        }
        gchar *highlight_name;
        if (is_dir){
            if (strcmp(xd_p->d_name, "..")==0) {
                highlight_name = g_strdup("go-up");
            } else highlight_name = g_strdup("document-open");
        } else {
            gchar *h_name = get_iconname(xd_p, FALSE);
            if (xd_p->st && U_RX(xd_p->st->st_mode)) {
                highlight_name = 
                    g_strdup_printf("%s/NE/application-x-executable-symbolic/2.5/220", h_name);
            } else {
                highlight_name = 
                    g_strdup_printf("%s/NE/document-open-symbolic/3.0/220", h_name);
            }
            g_free(h_name);
        }
       
        GdkPixbuf *normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  GTK_ICON_SIZE_DIALOG);
        // XXX: what's with function gtk_c::get_icon_size()
	// probably to get a smaller up arrow. Too nerdy...
	//GdkPixbuf *normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  gtk_c::get_icon_size(xd_p->d_name));
        //GdkPixbuf *highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  GTK_ICON_SIZE_DIALOG);
        GdkPixbuf *highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  GTK_ICON_SIZE_DIALOG);
	guint flags=0;
	//setSelectable(xd_p->d_name, flags);
        gtk_list_store_set (list_store, &iter, 
		FLAGS, flags,
                DISPLAY_NAME, utf_name,
                ACTUAL_NAME, xd_p->d_name,
                PATH, xd_p->path,
                ICON_NAME, icon_name,
                DISPLAY_PIXBUF, normal_pixbuf, 
                NORMAL_PIXBUF, normal_pixbuf, 
                HIGHLIGHT_PIXBUF, highlight_pixbuf, 
                TYPE,xd_p->d_type, 
                STAT,xd_p->st, 
                MIMETYPE, xd_p->mimetype,
                MIMEFILE, xd_p->mimefile, // may be null here.
                -1);
        g_free(icon_name);
        g_free(highlight_name);
        g_free(utf_name);
	// FIXME: enable itemsHash_ for monitor function.
        //g_hash_table_replace(itemsHash_, g_strdup(xd_p->d_name), GINT_TO_POINTER(1));
    }

    static gchar *
    get_iconname(xd_t *xd_p){
        return get_iconname(xd_p, TRUE);
    }

    static gchar *
    get_iconname(xd_t *xd_p, gboolean use_lite){
        gchar *name = get_basic_iconname(xd_p);
        gchar *emblem = get_emblem_string(xd_p, use_lite);
        gchar *iconname = g_strconcat(name, emblem, NULL);
        g_free(name);
        g_free(emblem);
        return iconname;
    }

    static gchar *
    get_basic_iconname(xd_t *xd_p){

        // Directories:
        if (strcmp(xd_p->d_name, "..")==0) return  g_strdup("go-up");
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
        if (xd_p->d_type == DT_DIR) {
            if (strcmp(xd_p->path, g_get_home_dir())==0) {
                return get_home_iconname(xd_p->d_name);
                
            }
            return  g_strdup("folder");
        }

        // Symlinks:
    /*    if (xd_p->d_type == xd_p->d_type == DT_LNK) {
            return  g_strdup("text-x-generic-template/SW/emblem-symbolic-link/2.0/220");
        }
    */
        // Character device:
        if (xd_p->d_type == DT_CHR ) {
            return  g_strdup("text-x-generic-template/SW/input-keyboard-symbolic/2.0/220");
        }
        // Named pipe (FIFO):
        if (xd_p->d_type == DT_FIFO ) {
            return  g_strdup("text-x-generic-template/SW/emblem-synchronizing-symbolic/2.0/220");
        }
        // UNIX domain socket:
        if (xd_p->d_type == DT_SOCK ) {
            return  g_strdup("text-x-generic-template/SW/emblem-shared-symbolic/2.0/220");
        }
        // Block device
        if (xd_p->d_type == DT_BLK ) {
            return  g_strdup("text-x-generic-template/SW/drive-harddisk-symbolic/2.0/220");
        }
        // Regular file:

        if (xd_p->d_type == DT_REG ) {
            const gchar *basic = get_mime_iconname(xd_p);
            return g_strdup(basic);
        }

        // Unknown:
        if (xd_p->d_type == DT_UNKNOWN) {
            return  g_strdup("dialog-question");
        }
#else
        if ((xd_p->st && S_ISDIR(xd_p->st->st_mode))) {
            if (strcmp(xd_p->path, g_get_home_dir())==0) {
                return get_home_iconname(xd_p->d_name);
            }
            return  g_strdup("folder");
        }

        // Symlinks:
    /*    if (xd_p->st && xd_p->d_type == xd_p->d_type == DT_LNK) {
            return  g_strdup("text-x-generic-template/SW/emblem-symbolic-link/2.0/220");
        }
    */
        // Character device:
        if ((xd_p->st && S_ISCHR(xd_p->st->st_mode))) {
            return  g_strdup("text-x-generic-template/SW/input-keyboard-symbolic/2.0/220");
        }
        // Named pipe (FIFO):
        if ((xd_p->st && S_ISFIFO(xd_p->st->st_mode))) {
            return  g_strdup("text-x-generic-template/SW/emblem-synchronizing-symbolic/2.0/220");
        }
        // UNIX domain socket:
        if ((xd_p->st && S_ISSOCK(xd_p->st->st_mode))) {
            return  g_strdup("text-x-generic-template/SW/emblem-shared-symbolic/2.0/220");
        }
        // Block device
        if ((xd_p->st && S_ISBLK(xd_p->st->st_mode))) {
            return  g_strdup("text-x-generic-template/SW/drive-harddisk-symbolic/2.0/220");
        }
        // Regular file:

        if ((xd_p->st && S_ISREG(xd_p->st->st_mode))) {
            const gchar *basic = get_mime_iconname(xd_p);
            return g_strdup(basic);
        }
#endif
        return  g_strdup("text-x-generic");
    }


    static const gchar *
    get_mime_iconname(xd_t *xd_p){
        const gchar *basic = "text-x-generic";
//FIXME: enable Mime template
#if 1
        return basic;
#else
        if (xd_p->mimetype) {
            // here we should get generic-icon from mime-module.xml!
            const gchar *basic = mime_c::get_mimetype_iconname(xd_p->mimetype);
            //DBG("xfdir_local_c::get_mime_iconname(%s) -> %s\n", xd_p->mimetype, basic);
            if (basic) {
                // check if the pixbuf is actually available
                GdkPixbuf *pixbuf = pixbuf_c::get_pixbuf(basic,  GTK_ICON_SIZE_DIALOG);
                if (pixbuf) return basic;
            } else {
                if (strstr(xd_p->mimetype, "text/html")){
                    return "text-html";
                }
            }

    /*
             "image-x-generic";
             "audio-x-generic";
             "font-x-generic";
             "package-x-generic";
             "video-x-generic";
             "x-office-address-book";
             "x-office-calendar";
             "x-office-document";
             "x-office-document-template";
             "x-office-drawing";
             "x-office-drawing-template";
             "x-office-presentation";
             "x-office-presentation-template";
             "x-office-spreadsheet";
             "x-office-spreadsheet-template";
             "text-html";
             "text-x-preview";
             "text-x-script";
             "application-x-executable";
             "application-certificate";
             "application-x-addon";
             "application-x-firmware";
             "x-package-repository";
    */
        }
        return  "text-x-generic";
#endif
    }

    static gchar *
    get_home_iconname(const gchar *data){
        if (!data) return g_strdup("user-home");
        const gchar *dir[]={N_("Documents"), N_("Downloads"),N_("Music"),N_("Pictures"),
                    N_("Templates"),N_("Videos"),N_("Desktop"),N_("Bookmarks"),
                    N_(".Trash"),NULL};
        const gchar *icon[]={"folder-documents", "folder-download","folder-music","folder-pictures",
                      "folder-templates","folder-videos","user-desktop","user-bookmarks",
                      "user-trash",NULL};
        const gchar **p, **i;
        for (p=dir, i=icon; p && *p ; p++, i++){
            if (strcasecmp(*p, data) == 0) {
                return g_strdup(*i);
            }
        }
        return g_strdup("folder");
    }

    static gchar *
    get_emblem_string(xd_t *xd_p){
        return get_emblem_string(xd_p, TRUE);
    }

    static gchar *
    get_emblem_string(xd_t *xd_p, gboolean use_lite){
        gchar *emblem = g_strdup("");
        // No emblem for go up
        if (strcmp(xd_p->d_name, "..")==0) return emblem;
        gchar *g;
        gboolean is_dir;
        gboolean is_lnk;
        gboolean is_reg;
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
        is_dir = (xd_p->d_type == DT_DIR);
        is_lnk = (xd_p->d_type == DT_LNK);
        is_reg = (xd_p->d_type == DT_REG);
#else 
        is_dir = (xd_p->st && S_ISDIR(xd_p->st->st_mode));
        is_reg = (xd_p->st && S_ISREG(xd_p->st->st_mode));
        is_lnk = (xd_p->st && S_ISLNK(xd_p->st->st_mode));
#endif
        
        // Symlinks:
        if (is_lnk) {
            if (xd_p->d_name[0] == '.') {
                g = g_strconcat(emblem, "#888888", NULL); 
                g_free(emblem); 
                emblem = g;
            }
            g = g_strconcat(emblem, "/SW/emblem-symbolic-link/2.0/220", NULL);
            g_free(emblem);
            emblem = g;
        }
        if (is_dir && xd_p->d_name[0] == '.') {
            g = g_strconcat(emblem, "#888888", NULL); 
            g_free(emblem); 
            emblem = g;
        }
        if (is_dir){
            if (!xd_p->st){
                g = g_strdup(emblem);
            }
            // all access:
            else if (xd_p->st && O_ALL(xd_p->st->st_mode)){
                g = g_strconcat(emblem, "/C/face-surprise/2.0/180", NULL);
            }
            else if ((MY_GROUP(xd_p->st->st_gid) && G_ALL(xd_p->st->st_mode)) 
                    || (MY_FILE(xd_p->st->st_uid) && U_ALL(xd_p->st->st_mode))){
                g = g_strdup(emblem);
            }
            // read only:
            else if (O_RX(xd_p->st->st_mode) 
                    || (MY_GROUP(xd_p->st->st_gid) && G_RX(xd_p->st->st_mode)) 
                    || (MY_FILE(xd_p->st->st_uid) && U_RX(xd_p->st->st_mode))){
                g = g_strconcat(emblem, "/C/emblem-readonly/3.0/180", NULL);
            }
            else {
                // no access:
                g = g_strconcat(emblem, "/C/face-angry/3.0/180", NULL);
            }
            g_free(emblem); 
            emblem = g;
        }
        
        else if (is_reg){
            guchar red;
            guchar green;
            guchar blue;
            gchar *colors = g_strdup("");
            if (xd_p->d_name[0] == '.') {
                g = g_strconcat(emblem, "#888888", NULL); 
                g_free(emblem); 
                emblem = g;
            }
// FIXME: enable lite template
#if 0 
            else if (lite_c::get_lite_colors(xd_p->mimetype, &red, &green, &blue)){
                g_free(colors);
                colors = g_strdup_printf("#%02x%02x%02x", red, green, blue);
            }
#endif
            gchar *extension = g_strdup("");
            if (strrchr(xd_p->d_name, '.') && strrchr(xd_p->d_name, '.') != xd_p->d_name) {
                extension = g_strconcat("*", strrchr(xd_p->d_name, '.')+1, NULL) ;
            }
            if (!xd_p->st) {
                g = g_strdup_printf("%s%s", 
                        extension, colors);
            }
            // all access:
            else if (O_ALL(xd_p->st->st_mode) || O_RW(xd_p->st->st_mode)){
                    g = g_strdup_printf("%s%s%s/C/face-surprise/2.5/180/NW/application-x-executable-symbolic/3.0/180",
                            extension, colors, emblem);
            // read/write/exec
            } else if((MY_GROUP(xd_p->st->st_gid) && G_ALL(xd_p->st->st_mode)) 
                    || (MY_FILE(xd_p->st->st_uid) && U_ALL(xd_p->st->st_mode))){
                    g = g_strdup_printf("%s%s%s/NW/application-x-executable-symbolic/3.0/180", 
                            extension, colors, emblem);
            // read/exec
            } else if (O_RX(xd_p->st->st_mode)
                    ||(MY_GROUP(xd_p->st->st_gid) && G_RX(xd_p->st->st_mode)) 
                    || (MY_FILE(xd_p->st->st_uid) && U_RX(xd_p->st->st_mode))){
                    g = g_strdup_printf("%s%s%s/NW/application-x-executable-symbolic/3.0/180", 
                            extension, colors, emblem);

            // read/write
            } else if ((MY_GROUP(xd_p->st->st_gid) && G_RW(xd_p->st->st_mode))
                    || (MY_FILE(xd_p->st->st_uid) && U_RW(xd_p->st->st_mode))) {
                    g = g_strdup_printf("%s%s%s", 
                            extension, colors, emblem);

            // read only:
            } else if (O_R(xd_p->st->st_mode) 
                    || (MY_GROUP(xd_p->st->st_gid) && G_R(xd_p->st->st_mode)) 
                    || (MY_FILE(xd_p->st->st_uid) && U_R(xd_p->st->st_mode))){
                    g = g_strdup_printf("%s%s%s/NW/emblem-readonly/3.0/130", 
                            extension, colors, emblem);
            } else if (S_ISREG(xd_p->st->st_mode)) {
                // no access: (must be have stat info to get this emblem)
                g = g_strdup_printf("%s%s%s/NW/emblem-unreadable/3.0/180/C/face-angry/2.0/180", 
                        extension, colors, emblem);
            } else {
                g = g_strdup_printf("%s%s", 
                        extension, colors);
            }
            g_free(extension);
            g_free(emblem); 
            emblem = g;
// FIXME: enable lite template
#if 0 
            if (use_lite) {
                const gchar *lite_emblem = lite_c::get_lite_emblem(xd_p->mimetype);
                if (lite_emblem){
                    g = g_strconcat(emblem, "/NE/", lite_emblem, "/1.8/200", NULL); 
                    g_free(emblem); 
                    emblem = g;
                }
            } 
#endif
        }
        return emblem;
    }


    ///////////////////////////////////////////////////////////
#if 0
    // FIXME: revise this for monitor function...
    static xd_t *
    get_xd_p(GFile *first){
	gchar *path = g_file_get_path(gfile_);
	gchar *basename = g_file_get_basename(first);
	struct dirent *d; // static pointer
	TRACE("looking for %s info\n", basename);
	DIR *directory = opendir(path);
	xd_t *xd_p = NULL;
	if (directory) {
	  while ((d = readdir(directory))  != NULL) {
	    if(strcmp (d->d_name, basename)) continue;
	    xd_p = get_xd_p(d);
	    break;
	  }
	  closedir (directory);
	} else {
	  g_warning("monitor_f(): opendir %s: %s\n", path, strerror(errno));
	}
	g_free(basename); 
	g_free(path); 
	return xd_p;
    }

    GHashTable *itemsHash_;
    GCancellable *cancellable_;
    GFile *gfile_;
    GFileMonitor *monitor_;
    gboolean showsHidden_;
    GtkListStore *store_;
    
    //using lite_c = Lite<Type>;
    //using mime_c = Mime<Type>;
public:
    LocalView(const gchar *path){
	itemsHash_ = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
	cancellable_ = g_cancellable_new ();
	gfile_ = g_file_new_for_path (path);
	monitor_ = NULL;
	showsHidden_ = FALSE;

    }
    ~LocalView(void){
	stop_monitor();
	g_hash_table_destroy(itemsHash_);
	//g_cancellable_cancel (cancellable_);
	//g_object_unref(cancellable_);
	if (gfile_) g_object_unref(gfile_);
	if (monitor_) g_object_unref(monitor_);
    }

    GFile *
    gfile(void){ return gfile_;}
    

    static const gchar *
    get_xfdir_iconname(void){
	return "system-file-manager";
    }

    static void
    item_activated (GtkIconView *iconview, GtkTreePath *tpath, void *data)
    {
	    DBG("LocalView::item activated\n");
	GtkTreeModel *treeModel = gtk_icon_view_get_model (iconview);
	GtkTreeIter iter;
	if (!gtk_tree_model_get_iter (treeModel, &iter, tpath)) return;
	gchar *name;
	gtk_tree_model_get (treeModel, &iter, COL_ACTUAL_NAME, &name,-1);
	WARN("FIXME: load item iconview \"%s\"\n", name);
	//view_p->reload(name);
	g_free(name);
    }
    gboolean
    add_new_item(GFile *file){
       xd_t *xd_p = get_xd_p(file);
	if (xd_p) {
	    add_local_item(store_, xd_p);
	    free_xd_p(xd_p);
	    return TRUE;
	} 
	return FALSE;
    }

    gboolean 
    remove_item(GFile *file){
	// find the iter and remove item
	gchar *basename = g_file_get_basename(file);
	g_hash_table_remove(itemsHash_, basename); 
	gtk_tree_model_foreach (GTK_TREE_MODEL(store_), rm_func, (gpointer) basename); 
	g_free(basename);
	return TRUE;
    }

    gboolean 
    restat_item(GFile *src){
	gchar *basename = g_file_get_basename(src);
	if (!g_hash_table_lookup(itemsHash_, basename)) {
	    g_free(basename);
	    return FALSE; 
	}
	g_free(basename);
	gchar *fullpath = g_file_get_path(src);
	gtk_tree_model_foreach (GTK_TREE_MODEL(store_), stat_func, (gpointer) fullpath); 
	g_free(fullpath);
	return TRUE;
    }

    void
    start_monitor(const gchar *data, GtkTreeModel *data2){
	store_ = GTK_LIST_STORE(data2);
	DBG("*** start_monitor: %s\n", data);
	if (gfile_) g_object_unref(gfile_);
	gfile_ = g_file_new_for_path (data);
	GError *error=NULL;
	if (monitor_) g_object_unref(monitor_);
	monitor_ = g_file_monitor_directory (gfile_, G_FILE_MONITOR_WATCH_MOVES, cancellable_,&error);
	if (error){
	    DBG("g_file_monitor_directory(%s) failed: %s\n",
		    data, error->message);
	    g_object_unref(gfile_);
	    gfile_=NULL;
	    return;
	}
	g_signal_connect (monitor_, "changed", 
		G_CALLBACK (monitor_f), (void *)this);
    }


    void 
    stop_monitor(void){
	gchar *p = g_file_get_path(gfile_);
	DBG("*** stop_monitor at: %s\n", p);
	g_free(p);
	g_file_monitor_cancel(monitor_);
	while (gtk_events_pending())gtk_main_iteration();  
	g_hash_table_remove_all(itemsHash_);
	// hash table remains alive (but empty) until destructor.
    }

    void
    set_showHidden(gboolean state){showsHidden_ = state;}
    static gboolean stat_func (GtkTreeModel *model,
				GtkTreePath *path,
				GtkTreeIter *iter,
				gpointer data){
	struct stat *st=NULL;
	gchar *text;
	gchar *basename = g_path_get_basename((gchar *)data);
	gtk_tree_model_get (model, iter, 
		COL_ACTUAL_NAME, &text, 
		COL_STAT, &st, 
		-1);  
	
	if (strcmp(basename, text)){
	    g_free(text);
	    g_free(basename);
	    return FALSE;
	}
	g_free(text);
	g_free(basename);
	g_free(st);

	GtkListStore *store = GTK_LIST_STORE(model);
	st = (struct stat *)calloc(1, sizeof(struct stat));
	if (stat((gchar *)data, st) != 0){
	    fprintf(stderr, "stat: %s\n", strerror(errno));
	    return FALSE;
	}

	gtk_list_store_set (store, iter, 
		COL_STAT,st, 
		-1);

	return TRUE;
    }

    static gboolean rm_func (GtkTreeModel *model,
				GtkTreePath *path,
				GtkTreeIter *iter,
				gpointer data){
	gchar *text;
	struct stat *st_p=NULL;
	gtk_tree_model_get (model, iter, 
		COL_STAT, &st_p, 
		COL_ACTUAL_NAME, &text, 
		-1);  
	
	if (strcmp(text, (gchar *)data)){
	    g_free(text);
	    return FALSE;
	}
	DBG("removing %s from treemodel.\n", text);
	GtkListStore *store = GTK_LIST_STORE(model);

    //  free stat record, if any
	g_free(st_p);

	gtk_list_store_remove(store, iter);
	g_free(text);
	return TRUE;
    }


    void
    monitor_f (GFileMonitor      *mon,
	      GFile             *first,
	      GFile             *second,
	      GFileMonitorEvent  event,
	      gpointer           data)
    {
	gchar *f= first? g_file_get_basename (first):g_strdup("--");
	gchar *s= second? g_file_get_basename (second):g_strdup("--");
       

	fprintf(stderr, "*** monitor_f call...\n");
	auto p = (LocalView<Type> *)data;

	switch (event){
	    case G_FILE_MONITOR_EVENT_DELETED:
	    case G_FILE_MONITOR_EVENT_MOVED_OUT:
		fprintf(stderr,"Received DELETED  (%d): \"%s\", \"%s\"\n", event, f, s);
		p->remove_item(first);
		break;
	    case G_FILE_MONITOR_EVENT_CREATED:
	    case G_FILE_MONITOR_EVENT_MOVED_IN:
		fprintf(stderr,"Received  CREATED (%d): \"%s\", \"%s\"\n", event, f, s);
		p->add_new_item(first);
		break;
	    case G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT:
	       fprintf(stderr,"Received  CHANGES_DONE_HINT (%d): \"%s\", \"%s\"\n", event, f, s);
		p->restat_item(first);
		// if image, then reload the pixbuf
		break;
	    case G_FILE_MONITOR_EVENT_CHANGED:
		fprintf(stderr,"Received  CHANGED (%d): \"%s\", \"%s\"\n", event, f, s);
		break;
	    case G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED:
		fprintf(stderr,"Received  ATTRIBUTE_CHANGED (%d): \"%s\", \"%s\"\n", event, f, s);
		p->restat_item(first);
		break;
	    case G_FILE_MONITOR_EVENT_PRE_UNMOUNT:
		fprintf(stderr,"Received  PRE_UNMOUNT (%d): \"%s\", \"%s\"\n", event, f, s);
		break;
	    case G_FILE_MONITOR_EVENT_UNMOUNTED:
		fprintf(stderr,"Received  UNMOUNTED (%d): \"%s\", \"%s\"\n", event, f, s);
		break;
	    case G_FILE_MONITOR_EVENT_MOVED:
	    case G_FILE_MONITOR_EVENT_RENAMED:
		fprintf(stderr,"Received  MOVED (%d): \"%s\", \"%s\"\n", event, f, s);
		p->remove_item(first);
		p->add_new_item(second);
		break;
	}
	g_free(f);
	g_free(s);
    }

#endif


};
}
#endif

