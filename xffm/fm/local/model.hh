#ifndef XF_LOCALMODEL__HH
# define XF_LOCALMODEL__HH
#include "common/util.hh"
#ifndef HAVE_STRUCT_DIRENT_D_TYPE
#error "HAVE_STRUCT_DIRENT_D_TYPE not defined"
#endif

typedef struct xd_t{
    gchar *d_name;
    gchar *path;
    unsigned char d_type;
    struct stat *st;
    gchar *mimetype;
    gchar *icon;
}xd_t;
static pthread_mutex_t readdir_mutex=PTHREAD_MUTEX_INITIALIZER;
static gboolean inserted_;

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
// Maximum character length to put file extension as a icon label:
#define EXTENSION_LABEL_LENGTH 4

namespace xf
{
template <class Type> class LocalIcons;
template <class Type> class Preview;
template <class Type>
class LocalModel
{
public:

    // This mkTreeModel should be static...
    static gint
    loadModel (GtkTreeModel *treeModel, const gchar *path)
    {
        while (gtk_events_pending()) gtk_main_iteration();
 	GtkTreeIter iter;
	if (gtk_tree_model_get_iter_first (treeModel, &iter)){
	    while (gtk_list_store_remove (GTK_LIST_STORE(treeModel),&iter));
	}
        while (gtk_events_pending()) gtk_main_iteration();
        TRACE("removed old stuff\n");

        int heartbeat = 0;
    
        GList *directory_list = read_items (path, &heartbeat);
        insert_list_into_model(directory_list, GTK_LIST_STORE(treeModel), path);
        TRACE("added new stuff\n");
        // Start the file monitor
        // count items...
        gint items = 0;
        if (gtk_tree_model_get_iter_first (treeModel, &iter)) {
            while (gtk_tree_model_iter_next(treeModel, &iter)) items++;
        }

	return items;
    }


    
    static gboolean
    isSelectable(GtkTreeModel *treeModel, GtkTreeIter *iter){
        gchar *name;
	gtk_tree_model_get (treeModel, iter, ACTUAL_NAME, &name, -1);
        gboolean retval = TRUE;
        if (strcmp(name, "..")==0 )retval = FALSE;
        g_free(name);
        return retval;
    }

    static const gchar *
    get_xfdir_iconname(void){
	return "system-file-manager";
    }

    //FIXME: must be done by non main thread (already mutex protected)
    static GList *
    read_items (const gchar *path,  gint *heartbeat)
    {
        GList *directory_list = NULL;
        if (!g_file_test(path, G_FILE_TEST_IS_DIR)){
            ERROR("read_items(): g_file_test(%s, G_FILE_TEST_IS_DIR) failed\n", path);
            return NULL;
        }
        TRACE( "readfiles: %s\n", path);
        DIR *directory = opendir(path);
        if (!directory) {
            ERROR("xfdir_local_c::read_items(): opendir %s: %s\n", path, strerror(errno));
            return NULL;
        }
    //  mutex protect...
        TRACE("** requesting readdir mutex for %s...\n", path);
        pthread_mutex_lock(&readdir_mutex);
        TRACE( "++ mutex for %s obtained.\n", path);
        struct dirent *d; // static pointer
        errno=0;
        TRACE( "shows hidden=%d\n", showHidden);
	gint count = 0;
        while ((d = readdir(directory))  != NULL){
            TRACE( "%p  %s\n", d, d->d_name);
            if(strcmp (d->d_name, ".") == 0) continue;
            if (strcmp(path,"/")==0 && strcmp (d->d_name, "..") == 0) continue;
	    // stat first 104 items.
            xd_t *xd_p = get_xd_p(path, d, (count++ < 104));
            directory_list = g_list_prepend(directory_list, xd_p);
            if (heartbeat) {
                (*heartbeat)++;
                TRACE(stderr,"incrementing heartbeat records to %d\n", *heartbeat);
            }
        }
        if (errno) {
            ERROR("read_files_local: %s: %s\n", strerror(errno), path);
        }
    // unlock mutex
        pthread_mutex_unlock(&readdir_mutex);
        TRACE("-- mutex for %s released.\n", path);

        closedir (directory);

        // At least the ../ record should have been read. If this
        // is not so, then a read error occurred.
        // (not uncommon in bluetoothed obexfs)
        if (!directory_list) {
            ERROR("read_files_local(): Count failed! Directory not read!\n");
        }
        directory_list = sort_directory_list (directory_list);
        return (directory_list);
    }

    // Convert a dirent entry into a xd_t structure.
    static xd_t *
    get_xd_p(const gchar *directory, struct dirent *d, gboolean withStat){
        xd_t *xd_p = (xd_t *)calloc(1,sizeof(xd_t));
        xd_p->d_name = g_strdup(d->d_name);
        if (strcmp(d->d_name, "..")==0){
	    xd_p->path = g_path_get_dirname(directory);
        } else if (strcmp(directory, G_DIR_SEPARATOR_S)==0){
            xd_p->path = g_strconcat(G_DIR_SEPARATOR_S, d->d_name, NULL);
        } else {
            xd_p->path = g_strconcat(directory, G_DIR_SEPARATOR_S, d->d_name, NULL);
        }

	xd_p->d_type = d->d_type;
	xd_p->st = NULL;
        /*
        if (xd_p->d_type==DT_REG && withStat){
        if (stat(path, &st) < 0) {
	    ERROR("stat(%s): %s\n", path, strerror(errno));
            name = g_strdup("inode/unknown");
	} 
        */
	xd_p->mimetype = getMimeType(xd_p);
	// symlinks and directories are stat'd in getMimeType()  
        xd_p->icon = g_strdup(LocalIcons<Type>::getIconname(xd_p));
        errno=0;
        return xd_p;
    }

    static gchar *
    getMimeType(xd_t *xd_p){
	auto mimetype = Mime<Type>::basicMimeType(xd_p->d_type);
	TRACE("%s -> %s\n", xd_p->path, mimetype);
	if (strcmp(mimetype, "inode/symlink")==0 ||
	    strcmp(mimetype, "inode/directory")==0 )
	{
            xd_p->st = (struct stat *)calloc(1, sizeof(struct stat));
	    if (!xd_p->st){
		ERROR("calloc: %s\n", strerror(errno));
		exit(1);
	    }
            errno=0;
	    stat(xd_p->path, xd_p->st);
            if (errno){
                DBG("stat: %s: %s\n", xd_p->path, strerror(errno));
                errno=0;
            }
	    g_free(mimetype);
	    mimetype = Mime<Type>::statMimeType(xd_p->st);
	}
	if (strcmp(mimetype,"inode/regular")==0){
	    auto type = Mime<Type>::extensionMimeType(xd_p->path);
	    if (type) {
		g_free(mimetype);
		mimetype = type;
	    }
	}
	return mimetype;
    }

    static void
    free_xd_p(xd_t *xd_p){
        g_free(xd_p->icon);
        g_free(xd_p->mimetype);
        g_free(xd_p->d_name);
        g_free(xd_p->path);
        g_free(xd_p->st);
        g_free(xd_p);
    }
    static gint
    compare_by_name2 (const void *a, const void *b) {
       // compare by name, directories or symlinks to directories on top
        const xd_t *xd_a = (const xd_t *)a;
        const xd_t *xd_b = (const xd_t *)b;

        if (strcmp(xd_a->d_name, "..")==0) return -1;
        if (strcmp(xd_b->d_name, "..")==0) return 1;

        gboolean a_cond = FALSE;
        gboolean b_cond = FALSE;

        a_cond = ((xd_a->d_type == DT_DIR )||(xd_a->st && S_ISDIR(xd_a->st->st_mode)));
        b_cond = ((xd_b->d_type == DT_DIR )||(xd_b->st && S_ISDIR(xd_b->st->st_mode)));

        if (a_cond && !b_cond) return -1; 
        if (!a_cond && b_cond) return 1;
        
        return -strcasecmp(xd_a->d_name, xd_b->d_name);
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

        a_cond = ((xd_a->d_type == DT_DIR )||(xd_a->st && S_ISDIR(xd_a->st->st_mode)));
        b_cond = ((xd_b->d_type == DT_DIR )||(xd_b->st && S_ISDIR(xd_b->st->st_mode)));

        if (a_cond && !b_cond) return -1; 
        if (!a_cond && b_cond) return 1;
        return strcasecmp(xd_a->d_name, xd_b->d_name);
    }
private:
    static GList *
    sort_directory_list(GList *list){
#if 0
        stat is done in xd_t record creation...
        // FIXME:  only do a full stat when sort order is date or size
        
        gboolean do_stat = (g_list_length(list) <= MAX_AUTO_STAT);

        if (do_stat){
            GList *l;
            for (l=list; l && l->data; l=l->next){
                xd_t *xd_p = (xd_t *)l->data;
                xd_p->st = (struct stat *)calloc(1, sizeof(struct stat));
                if (!xd_p->st) continue;
                if (stat(xd_p->path, xd_p->st)){
                    TRACE("xfdir_local_c::sort_directory_list: cannot stat %s (%s)\n", 
                            xd_p->path, strerror(errno));
                    continue;
                }
            }
        }
#endif        
        // Default sort order:
        if (Settings<Type>::getSettingInteger("LocalView", "Descending") <= 0) {
            return g_list_sort (list,compare_by_name);
        } else {
            return g_list_sort (list,compare_by_name2);
        }
    }

public:
    static gint
    insert_list_into_model(GList *data, GtkListStore *list_store, const gchar *path){
	if(strcmp(path, "/")==0){
	    RootView<Type>::addXffmItem(GTK_TREE_MODEL(list_store));
	}
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

    static gboolean
    insertItem(GtkTreeModel *treeModel, GtkTreePath *path, GtkTreeIter *iter, gpointer data){
        // get current xd_p
        struct stat *st;
        guint size;
        guint date;
        gchar *name;
        guint type;
        gtk_tree_model_get(treeModel, iter, 
                ACTUAL_NAME, &name, 
                SIZE, &size,
                DATE, &date,
                TYPE, &type, -1);
        xd_t *xd_p = (xd_t *)data;
        xd_t *xd_b = (xd_t *)calloc(1, sizeof(xd_t));
        xd_b->d_name = name;
        xd_b->d_type = type;
        TRACE("compare %s with iconview item \"%s\"\n", xd_p->d_name, name);
        gint sortResult;
        if (Settings<Type>::getSettingInteger("LocalView", "Descending") <= 0) {
            sortResult = compare_by_name((void *)xd_p, (void *)(xd_b));
        } else {
            sortResult = compare_by_name2((void *)xd_p, (void *)(xd_b));
        }

        if (sortResult < 0){
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
        gboolean showHidden = (Settings<Type>::getSettingInteger("LocalView", "ShowHidden") > 0);
        if (!showHidden && xd_p->d_name[0] == '.'  && strcmp("..", xd_p->d_name)){
            return;
        }
        gboolean showBackups = (Settings<Type>::getSettingInteger("LocalView", "ShowBackups") > 0);
        if (!showBackups && LocalIcons<Type>::backupType(xd_p->d_name)){
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


        
        gchar *utf_name = Util<Type>::utf_string(xd_p->d_name);
        const gchar *icon_name = xd_p->icon;
	TRACE("icon name for %s is %s\n", xd_p->d_name, icon_name);
        
        // chop file extension (will now appear on the icon). (XXX only for big icons)
        gboolean is_dir;
        gboolean is_reg_not_link;

        is_dir = (xd_p->d_type == DT_DIR);
        is_reg_not_link = (xd_p->d_type == DT_REG && !(xd_p->d_type == DT_LNK));
        if (is_reg_not_link) {
            gchar *t = g_strdup(xd_p->d_name);
            if (strchr(t, '.') && strrchr(t, '.') != t){
                if (strlen(strrchr(t, '.')+1) <= EXTENSION_LABEL_LENGTH) {
                    *strrchr(t, '.') = 0;
                    g_free(utf_name);
                    utf_name = Util<Type>::utf_string(t);
                } 
                g_free(t);
            }
        }
        gchar *highlight_name=NULL;
	if (g_path_is_absolute(icon_name)) highlight_name = g_strdup(icon_name);
        if (!highlight_name && is_dir){
            if (strcmp(xd_p->d_name, "..")==0) {
                highlight_name = g_strdup("go-up/NW/go-up-symbolic/2.0/225");
            } else highlight_name = g_strdup("document-open");
        } else if (!highlight_name){
            gchar *h_name = LocalIcons<Type>::getIconname(xd_p);
            if (xd_p->st && U_RX(xd_p->st->st_mode)) {
                highlight_name = 
                    g_strdup_printf("%s/NE/application-x-executable-symbolic/2.5/220", h_name);
            } else {
                highlight_name = 
                    g_strdup_printf("%s/NE/document-open-symbolic/3.0/220", h_name);
            }
            g_free(h_name);
        }
        TRACE("iconname, highlight: %s, %s\n", icon_name, highlight_name);
        GdkPixbuf *treeViewPixbuf = NULL;
        GdkPixbuf *normal_pixbuf = NULL;
        GdkPixbuf *highlight_pixbuf = NULL;

	if (g_path_is_absolute(icon_name))
	    normal_pixbuf = Preview<Type>::loadFromThumbnails(icon_name, xd_p->st, 48, 48);
	if (g_path_is_absolute(highlight_name)){
	    if (strcmp(highlight_name, icon_name)==0) highlight_pixbuf = normal_pixbuf;
	    else highlight_pixbuf = Preview<Type>::loadFromThumbnails(highlight_name, xd_p->st, 48, 48);
	}
      
        if (!treeViewPixbuf) 
	    treeViewPixbuf = Pixbuf<Type>::get_pixbuf(icon_name, -24);
        if (!normal_pixbuf) {
	    auto thumbnail = Hash<Type>::get_thumbnail_path (icon_name, 48);
	    normal_pixbuf = Pixbuf<Type>::get_pixbuf(icon_name, -48);
	    Pixbuf<Type>::pixbuf_save(normal_pixbuf, thumbnail);
	}
        if (!highlight_pixbuf) {
	    if (strcmp(highlight_name, icon_name)==0) highlight_pixbuf = normal_pixbuf;
	    else {
		auto thumbnail = Hash<Type>::get_thumbnail_path (highlight_name, 48);
		highlight_pixbuf = Pixbuf<Type>::get_pixbuf(highlight_name, -48);
		Pixbuf<Type>::pixbuf_save(highlight_pixbuf, thumbnail);
	    }
	}
	guint flags=0;
        guint size = (xd_p->st)?xd_p->st->st_size:0;
        guint date = (xd_p->st)?xd_p->st->st_mtim.tv_sec:0;
        gchar *statInfo = (xd_p->st)?Util<Type>::statInfo(xd_p->path):NULL;
        gchar **p = NULL;
       /* if (statInfo){
            p = g_strsplit(statInfo, " ", 6);
            if (p) {
                g_free(statInfo);
                statInfo = g_strdup_printf("%s %s %s", p[5], p[4], p[0]);
            } else {
                g_free(statInfo);
                statInfo = NULL;
            }
        }*/
        if (!statInfo) statInfo = g_strdup("");
	//setSelectable(xd_p->d_name, flags);
        gtk_list_store_set (list_store, iter, 
		FLAGS, flags,
                DISPLAY_NAME, utf_name,
                ACTUAL_NAME, xd_p->d_name,
                PATH, xd_p->path,
                ICON_NAME, icon_name,
                TREEVIEW_PIXBUF, treeViewPixbuf, 
                DISPLAY_PIXBUF, normal_pixbuf, 
                NORMAL_PIXBUF, normal_pixbuf, 
                HIGHLIGHT_PIXBUF, highlight_pixbuf, 
                TYPE,xd_p->d_type, 
                SIZE,size, 
                DATE,date, 
                MIMETYPE, xd_p->mimetype,
                TOOLTIP_TEXT, statInfo,
                -1);
        g_free(statInfo);
        g_free(highlight_name);
        g_free(utf_name);
    }




};
}
#endif

