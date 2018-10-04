#ifndef XF_LOCALVIEW__HH
# define XF_LOCALVIEW__HH

#include "baseview.hh"
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

enum
{
  COL_DISPLAY_PIXBUF,
  COL_NORMAL_PIXBUF,
  COL_HIGHLIGHT_PIXBUF,
  COL_TOOLTIP_PIXBUF,
  COL_DISPLAY_NAME,
  COL_ACTUAL_NAME,
  COL_PATH,
  COL_TOOLTIP_TEXT,
  COL_ICON_NAME,
  COL_TYPE,
  COL_MIMETYPE, 
  COL_MIMEFILE, 
  COL_STAT,
  COL_PREVIEW_PATH,
  COL_PREVIEW_TIME,
  COL_PREVIEW_PIXBUF,
  NUM_COLS
};

pthread_mutex_t readdir_mutex=PTHREAD_MUTEX_INITIALIZER;

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

template <class Type>
class LocalView: public BaseView<Type> {

    using gtk_c = Gtk<Type>;
    using pixbuf_c = Pixbuf<Type>;
    using util_c = Util<Type>;
    //using lite_c = Lite<Type>;
    //using mime_c = Mime<Type>;
public:
/*
RootView(const gchar *path): 
    BaseView<Type>("xffm:root")
{
    this->treemodel_ = mk_tree_model();
    g_object_set_data(G_OBJECT(this->treemodel_), "iconview", this->iconView_);
    gtk_icon_view_set_model(this->iconView_, this->treemodel_);
    gtk_icon_view_set_text_column (this->iconView_, this->get_text_column());
    gtk_icon_view_set_pixbuf_column (this->iconView_,  this->get_icon_column());
    gtk_icon_view_set_selection_mode (this->iconView_, GTK_SELECTION_SINGLE);   
}
*/

    static gboolean enableDragSource(void){ return TRUE;}
    static gboolean enableDragDest(void){ return TRUE;}


    static gint
    actualNameColumn(void){ return COL_ACTUAL_NAME;}
    static gint 
    iconColumn(void){ return COL_DISPLAY_PIXBUF;}
    static gint 
    textColumn(void){ return COL_DISPLAY_NAME;}
    static gint
    highlightPixbufC(void){return COL_HIGHLIGHT_PIXBUF;}
    static gint
    normalPixbufC(void){return COL_NORMAL_PIXBUF;}
    static gint
    tooltipPixbufC(void){return COL_TOOLTIP_PIXBUF;}
    static gint
    tooltipTextC(void){return COL_TOOLTIP_TEXT;}


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

    static GtkTreeModel *
    mkTreeModel (const gchar *path)
    {
        if (!path || !g_file_test(path, G_FILE_TEST_EXISTS)) {
            ERROR( "%s does not exist\n", path);
            return NULL;
        }
        if (chdir(path)<0){
            ERROR( "chdir(%s): %s\n", path, strerror(errno));
            return NULL;
        }

	GtkTreeIter iter;
       // path = g_strdup("xffm:root");  
	GtkListStore *list_store = gtk_list_store_new (NUM_COLS, 
	    GDK_TYPE_PIXBUF, // icon in display
	    GDK_TYPE_PIXBUF, // normal icon reference
	    GDK_TYPE_PIXBUF, // highlight icon reference
	    GDK_TYPE_PIXBUF, // preview, tooltip image (cache)
	    G_TYPE_STRING,   // name in display (UTF-8)
	    G_TYPE_STRING,   // name from filesystem (verbatim)
	    G_TYPE_STRING,   // path (verbatim)
	    G_TYPE_STRING,   // tooltip text (cache)
	    G_TYPE_STRING,   // icon identifier (name or composite key)
	    G_TYPE_INT,      // mode (to identify directories)
	    G_TYPE_STRING,   // mimetype (further identification of files)
	    G_TYPE_STRING,   // mimefile (further identification of files)
	    G_TYPE_POINTER,  // stat record or NULL
	    G_TYPE_STRING,   // Preview path
	    G_TYPE_INT,      // Preview time
	    GDK_TYPE_PIXBUF); // Preview pixbuf

        int heartbeat = 0;

        GList *directory_list = read_items (path, TRUE, &heartbeat);
        insert_list_into_model(directory_list, list_store);
		

	return GTK_TREE_MODEL (list_store);
    }


    //FIXME: must be done by non main thread (already mutex protected)
    static GList *
    read_items (const gchar *path, gboolean showsHidden, gint *heartbeat) {
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
        TRACE( "shows hidden=%d\n", showsHidden);
        while ((d = readdir(directory))  != NULL){
            TRACE( "%p  %s\n", d, d->d_name);
            if(strcmp (d->d_name, ".") == 0) continue;
            if(!showsHidden && d->d_name[0] == '.' && strcmp (d->d_name, "..")) continue;
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

    static xd_t *
    get_xd_p(const gchar *directory, struct dirent *d){
        xd_t *xd_p = (xd_t *)calloc(1,sizeof(xd_t));
        xd_p->d_name = g_strdup(d->d_name);
        xd_p->path = g_strconcat(directory, G_DIR_SEPARATOR_S, d->d_name, NULL);

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

    void
    free_xd_p(xd_t *xd_p){
        g_free(xd_p->mimefile);
        g_free(xd_p->mimetype);
        g_free(xd_p->d_name);
        g_free(xd_p->path);
        g_free(xd_p);
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

    static void
    add_local_item(GtkListStore *list_store, xd_t *xd_p){
        // FIXME: items_hash is on a object basis, not static item...
        // if it already exists, do nothing
        if (g_hash_table_lookup(items_hash, (void *)xd_p->d_name)){    
            DBG("local_monitor_c::not re-adding %s\n", xd_p->d_name);
            return;
        }

        //FIXME need for shows_hidden only in monitor function...
        //if (!shows_hidden && xd_p->d_name[0] == '.'  && strcmp("..", xd_p->d_name)){
        if (xd_p->d_name[0] == '.'  && strcmp("..", xd_p->d_name)){
            return;
        }
        
        GtkTreeIter iter;
        gtk_list_store_append (list_store, &iter);
        gchar *utf_name = util_c::utf_string(xd_p->d_name);
        // plain extension mimetype fallback
        // FIXME: enable mime_c template
        //if (!xd_p->mimetype) xd_p->mimetype = mime_c::mime_type(xd_p->d_name); 
        gchar *icon_name = get_iconname(xd_p);
        
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
                    g_strdup_printf("%s/NE/emblem-run/2.0/220", h_name);
            } else {
                highlight_name = 
                    g_strdup_printf("%s/NE/document-open/2.0/220", h_name);
            }
            g_free(h_name);
        }
       
        GdkPixbuf *normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  gtk_c::get_icon_size(xd_p->d_name));
        //GdkPixbuf *highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  GTK_ICON_SIZE_DIALOG);
        GdkPixbuf *highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  GTK_ICON_SIZE_DIALOG);
        gtk_list_store_set (list_store, &iter, 
                COL_DISPLAY_NAME, utf_name,
                COL_ACTUAL_NAME, xd_p->d_name,
                COL_PATH, xd_p->path,
                COL_ICON_NAME, icon_name,
                COL_DISPLAY_PIXBUF, normal_pixbuf, 
                COL_NORMAL_PIXBUF, normal_pixbuf, 
                COL_HIGHLIGHT_PIXBUF, highlight_pixbuf, 
                COL_TYPE,xd_p->d_type, 
                COL_STAT,xd_p->st, 
                COL_MIMETYPE, xd_p->mimetype,
                COL_MIMEFILE, xd_p->mimefile, // may be null here.
                -1);
        g_free(icon_name);
        g_free(highlight_name);
        g_free(utf_name);
        g_hash_table_replace(items_hash, g_strdup(xd_p->d_name), GINT_TO_POINTER(1));
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
                if (stat(xd_p->d_name, xd_p->st)){
                    DBG("xfdir_local_c::sort_directory_list: cannot stat %s (%s)\n", 
                            xd_p->d_name, strerror(errno));
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
            gchar *path = g_file_get_path(gfile);
            if (strcmp(path, g_get_home_dir())==0) {
                g_free(path);
                return get_home_iconname(xd_p->d_name);
                
            }
            g_free(path);
            return  g_strdup("folder");
        }

        // Symlinks:
    /*    if (xd_p->d_type == xd_p->d_type == DT_LNK) {
            return  g_strdup("text-x-generic-template/SW/emblem-symbolic-link/2.0/220");
        }
    */
        // Character device:
        if (xd_p->d_type == DT_CHR ) {
            return  g_strdup("text-x-generic-template/SW/emblem-chardevice/2.0/220");
        }
        // Named pipe (FIFO):
        if (xd_p->d_type == DT_FIFO ) {
            return  g_strdup("text-x-generic-template/SW/emblem-fifo/2.0/220");
        }
        // UNIX domain socket:
        if (xd_p->d_type == DT_SOCK ) {
            return  g_strdup("text-x-generic-template/SW/emblem-socket/2.0/220");
        }
        // Block device
        if (xd_p->d_type == DT_BLK ) {
            return  g_strdup("text-x-generic-template/SW/emblem-blockdevice/2.0/220");
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
            gchar *path = g_file_get_path(gfile);
            if (strcmp(path, g_get_home_dir())==0) {
                g_free(path);
                return get_home_iconname(xd_p->d_name);
            }
            g_free(path);
            return  g_strdup("folder");
        }

        // Symlinks:
    /*    if (xd_p->st && xd_p->d_type == xd_p->d_type == DT_LNK) {
            return  g_strdup("text-x-generic-template/SW/emblem-symbolic-link/2.0/220");
        }
    */
        // Character device:
        if ((xd_p->st && S_ISCHR(xd_p->st->st_mode))) {
            return  g_strdup("text-x-generic-template/SW/emblem-chardevice/2.0/220");
        }
        // Named pipe (FIFO):
        if ((xd_p->st && S_ISFIFO(xd_p->st->st_mode))) {
            return  g_strdup("text-x-generic-template/SW/emblem-fifo/2.0/220");
        }
        // UNIX domain socket:
        if ((xd_p->st && S_ISSOCK(xd_p->st->st_mode))) {
            return  g_strdup("text-x-generic-template/SW/emblem-socket/2.0/220");
        }
        // Block device
        if ((xd_p->st && S_ISBLK(xd_p->st->st_mode))) {
            return  g_strdup("text-x-generic-template/SW/emblem-blockdevice/2.0/220");
        }
        // Regular file:

        if ((xd_p->st && S_ISREG(xd_p->st->st_mode))) {
            const gchar *basic = get_mime_iconname(xd_p);
            return g_strdup(basic);
        }
#endif
        return  g_strdup("text-x-generic");
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
                    g = g_strdup_printf("%s%s%s/C/face-surprise/2.0/180/NW/emblem-exec/3.0/180",
                            extension, colors, emblem);
            // read/write/exec
            } else if((MY_GROUP(xd_p->st->st_gid) && G_ALL(xd_p->st->st_mode)) 
                    || (MY_FILE(xd_p->st->st_uid) && U_ALL(xd_p->st->st_mode))){
                    g = g_strdup_printf("%s%s%s/NW/emblem-exec/3.0/180", 
                            extension, colors, emblem);
            // read/exec
            } else if (O_RX(xd_p->st->st_mode)
                    ||(MY_GROUP(xd_p->st->st_gid) && G_RX(xd_p->st->st_mode)) 
                    || (MY_FILE(xd_p->st->st_uid) && U_RX(xd_p->st->st_mode))){
                    g = g_strdup_printf("%s%s%s/NW/emblem-exec/3.0/180", 
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


};
}
#endif

