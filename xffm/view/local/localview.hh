#ifndef XF_LOCALVIEW__HH
# define XF_LOCALVIEW__HH

// FIXME: determine HAVE_STRUCT_DIRENT_D_TYPE on configure (for freebsd)
#define HAVE_STRUCT_DIRENT_D_TYPE 1
#undef USE_MIME
//#define USE_MIME 1
#undef USE_LITE
//#define USE_LITE 1
//#include "mime/lite.hh"
#include "mime/mime.hh"
#include "common/util.hh"

#include "localpopup.hh"

typedef struct xd_t{
    gchar *d_name;
    gchar *path;
    unsigned char d_type;
    struct stat *st;
    const gchar *mimetype;
    const gchar *mimefile;
}xd_t;
static pthread_mutex_t readdir_mutex=PTHREAD_MUTEX_INITIALIZER;
static gboolean inserted_;

// Maximum character length to put file extension as a icon label:
#define EXTENSION_LABEL_LENGTH 4

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
template <class Type> class BaseView;
template <class Type>
class LocalView: public LocalPopUp<Type> {
    
    using gtk_c = Gtk<Type>;
    using pixbuf_c = Pixbuf<Type>;
    using util_c = Util<Type>;


public:

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

    static gboolean
    item_activated (GtkIconView *iconview, const gchar *path)
    {
	WARN("LocalView::item activated: %s\n", path);
	return FALSE;
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
        TRACE("** requesting readdir mutex for %s...\n", path);
        pthread_mutex_lock(&readdir_mutex);
        TRACE( "++ mutex for %s obtained.\n", path);
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
        TRACE("-- mutex for %s released.\n", path);

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

public:
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
        // stat symbolic links now...
        if (xd_p->d_type == DT_LNK){
            xd_p->st = (struct stat *)calloc(1, sizeof(struct stat));
            stat(xd_p->path, xd_p->st);
        }
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
        //this is a problematic memory leak (FIXME)
        //g_free(xd_p->st);
        g_free(xd_p);
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
        a_cond = ((xd_a->d_type == DT_DIR )||(xd_a->st && S_ISDIR(xd_a->st->st_mode)));
        b_cond = ((xd_b->d_type == DT_DIR )||(xd_b->st && S_ISDIR(xd_b->st->st_mode)));
#else
        if (xd_a->st && xd_b->st && 
                (S_ISDIR(xd_a->st->st_mode) || S_ISDIR(xd_b->st->st_mode))) {
            a_cond = (S_ISDIR(xd_a->st->st_mode)|| S_ISDIR(xd_a->st->st_mode));
            b_cond = (S_ISDIR(xd_b->st->st_mode)|| S_ISDIR(xd_b->st->st_mode));
        } 
#endif

        if (a_cond && !b_cond) return -1; 
        if (!a_cond && b_cond) return 1;
        return strcasecmp(xd_a->d_name, xd_b->d_name);
    }
private:
    static GList *
    sort_directory_list(GList *list){
        // FIXME:  only do stat when sort order is date or size
        /*
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
                xd_p->mimetype = Mime<Type>::mimeType(xd_p->d_name, xd_p->st); // using stat obtained above
                xd_p->mimefile = g_strdup(Mime<Type>::mimeFile(xd_p->d_name)); // 
            }
        }
        */
        // Default sort order:
        return g_list_sort (list,compare_by_name);
    }

    static gint
    insert_list_into_model(GList *data, GtkListStore *list_store){
        GList *directory_list = (GList *)data;
        gint dir_count = g_list_length(directory_list);
        GList *l = directory_list;
        for (; l && l->data; l= l->next){
            //while (gtk_events_pending()) gtk_main_iteration();
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

private:
    static gboolean
    insertItem(GtkTreeModel *treeModel, GtkTreePath *path, GtkTreeIter *iter, gpointer data){
        // get current xd_p
        struct stat *st;
        gchar *name;
        gint type;
        gtk_tree_model_get(treeModel, iter, ST_DATA, &st, ACTUAL_NAME, &name, TYPE, &type, -1);
        xd_t *xd_p = (xd_t *)data;
        xd_t *xd_b = (xd_t *)calloc(1, sizeof(xd_t));
        xd_b->d_name = name;
        xd_b->d_type = type;
        xd_b->st = st;
        TRACE("compare %s with iconview item \"%s\"\n", xd_p->d_name, name);
        if (compare_by_name((void *)xd_p, (void *)(xd_b)) < 0){
            GtkTreeIter newIter;
            gtk_list_store_insert_before (GTK_LIST_STORE(treeModel), &newIter, iter);
            add_local_item(GTK_LIST_STORE(treeModel), &newIter, xd_p);
            free_xd_p(xd_b);
            inserted_ = TRUE;
            return inserted_;
        }
        free_xd_p(xd_b);
        
        return inserted_;
    }


public:
    static void
    insertLocalItem(GtkListStore *listStore, xd_t *xd_p){
        inserted_=FALSE;
        gtk_tree_model_foreach (GTK_TREE_MODEL(listStore), insertItem, (void *)xd_p);
        if (!inserted_) add_local_item(listStore, xd_p);
    }

    static void
    add_local_item(GtkListStore *list_store, xd_t *xd_p){
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
        //gtk_list_store_prepend (list_store, &iter);
        gtk_list_store_append (list_store, &iter);
        add_local_item(list_store, &iter, xd_p);
    }

private:
    static void
    add_local_item(GtkListStore *list_store, GtkTreeIter *iter, xd_t *xd_p){


        
        gchar *utf_name = util_c::utf_string(xd_p->d_name);
        // plain extension mimetype fallback
	//
	// FIXME: maybe do a group mimetype for all items...
#ifdef USE_MIME
        if (!xd_p->mimetype) xd_p->mimetype = Mime<Type>::mimeType(xd_p->path); 
#endif
        gchar *icon_name = get_iconname(xd_p);
	TRACE("icon name for %s is %s\n", xd_p->d_name, icon_name);
        
        // chop file extension (will now appear on the icon). (XXX only for big icons)
        gboolean is_dir;
        gboolean is_reg_not_link;

// FIXME: get HAVE_STRUCT_DIRENT_D_TYPE from cmake...
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
                if (strlen(strrchr(t, '.')+1) <= EXTENSION_LABEL_LENGTH) {
                    *strrchr(t, '.') = 0;
                    g_free(utf_name);
                    utf_name = util_c::utf_string(t);
                } 
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
        gtk_list_store_set (list_store, iter, 
		FLAGS, flags,
                DISPLAY_NAME, utf_name,
                ACTUAL_NAME, xd_p->d_name,
                PATH, xd_p->path,
                ICON_NAME, icon_name,
                DISPLAY_PIXBUF, normal_pixbuf, 
                NORMAL_PIXBUF, normal_pixbuf, 
                HIGHLIGHT_PIXBUF, highlight_pixbuf, 
                TYPE,xd_p->d_type, 
                ST_DATA,xd_p->st, 
                MIMETYPE, xd_p->mimetype,
                MIMEFILE, xd_p->mimefile, // may be null here.
                -1);
        g_free(icon_name);
        g_free(highlight_name);
        g_free(utf_name);
	// FIXME: enable itemsHash_ for monitor function.
        //g_hash_table_replace(itemsHash_, g_strdup(xd_p->d_name), GINT_TO_POINTER(1));
    }
private:
    static gchar *
    get_iconname(xd_t *xd_p){
        return get_iconname(xd_p, TRUE);
    }

    static gchar *
    get_iconname(xd_t *xd_p, gboolean use_lite){
        gchar *name = get_basic_iconname(xd_p);
	TRACE("basic iconname: %s --> %s\n", xd_p->d_name, name);
        gchar *emblem = get_emblem_string(xd_p, use_lite);
	if (!name) name = g_strdup("image-missing");
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
#warning "HAVE_STRUCT_DIRENT_D_TYPE defined"
        // Symlinks:
    /*    if (xd_p->d_type == DT_LNK) {
            return  g_strdup("text-x-generic-template/SW/emblem-symbolic-link/2.0/220");
        }
    */
        if ((xd_p->d_type == DT_DIR )||(xd_p->st && S_ISDIR(xd_p->st->st_mode))) {
            if (strcmp(xd_p->path, g_get_home_dir())==0) {
                return get_home_iconname(xd_p->d_name);
            }
            return  g_strdup("folder");
        }
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
        const gchar *basic = NULL;
#ifdef USE_MIME
        if (xd_p->mimetype) {
            // here we should get generic-icon from mime-module.xml!
            basic = Mime<Type>::get_mimetype_iconname(xd_p->mimetype);
            TRACE("xfdir_local_c::get_mime_iconname(%s) -> %s\n", xd_p->mimetype, basic);
            if (basic) {
                // check if the pixbuf is actually available
                GdkPixbuf *pixbuf = pixbuf_c::get_pixbuf(basic,  GTK_ICON_SIZE_DIALOG);
                if (pixbuf) return basic;
		else return "text-x-generic";
            } else {
                if (strstr(xd_p->mimetype, "text/html")){
                    return "text-html";
                }
		return "text-x-generic";
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
#endif
        return "text-x-generic";
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
    regularEmblem(xd_t *xd_p, gchar *emblem){
	gchar *g;
	if (xd_p->d_name[0] == '.') {
	    g = g_strconcat(emblem, "#888888", NULL); 
	    g_free(emblem); 
	    emblem = g;
	    return emblem;
	}
	guchar red;
	guchar green;
	guchar blue;
	gchar *colors = g_strdup("");
#if 0
	if (getColors(xd_p, &red, &green, &blue)){
	    g_free(colors);
	    colors = g_strdup_printf("#%02x%02x%02x", red, green, blue);
	}
	
	gchar *extension = g_strdup("");
	if (strrchr(xd_p->d_name, '.') && strrchr(xd_p->d_name, '.') != xd_p->d_name
		&& strlen(strrchr(xd_p->d_name, '.')+1) <= EXTENSION_LABEL_LENGTH) {
	    extension = g_strconcat("*", strrchr(xd_p->d_name, '.')+1, NULL) ;
	}
	if (!xd_p->st) {
	    g = g_strdup_printf("%s%s", 
		    extension, colors);
	}
	// all access:
	else if (O_ALL(xd_p->st->st_mode) || O_RW(xd_p->st->st_mode)){
		g = g_strdup_printf("%s%s%s/C/face-surprise-symbolic/2.5/180/NW/application-x-executable-symbolic/3.0/180",
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
		g = g_strdup_printf("%s%s%s/NW/face-surprise-symbolic/3.0/130", 
			extension, colors, emblem);
	} else if (S_ISREG(xd_p->st->st_mode)) {
	    // no access: (must be have stat info to get this emblem)
	    g = g_strdup_printf("%s%s%s/NW/face-sick-symbolic/2.0/180", 
		    extension, colors, emblem);
	} else {
	    g = g_strdup_printf("%s%s", 
		    extension, colors);
	}
	g_free(extension);
	g_free(emblem); 
	emblem = g;
#ifdef USE_LITE
	if (use_lite) {
	    const gchar *lite_emblem = Lite<Type>::get_lite_emblem(xd_p->mimetype);
	    WARN("lite_emblem=%s\n", lite_emblem);
	    if (lite_emblem){
		g = g_strconcat(emblem, "/NE/", lite_emblem, "/1.8/200", NULL); 
		g_free(emblem); 
		emblem = g;
	    }
	} 
#endif
#endif
	return emblem;
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
	    emblem = regularEmblem(xd_p, emblem);
        }
        return emblem;
    }

};
}
#endif

