#ifndef XF_LOCALMODEL__HH
# define XF_LOCALMODEL__HH
#include "common/util.hh"
#ifndef HAVE_STRUCT_DIRENT_D_TYPE
#warning "HAVE_STRUCT_DIRENT_D_TYPE not defined"
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
    loadModel (View<Type> *view, const gchar *path)
    {
        TRACE("*** local/model.hh loadModel()\n");
	auto treeModel = view->treeModel();
 	/*GtkTreeIter iter;
	if (gtk_tree_model_get_iter_first (treeModel, &iter)){
	    while (gtk_list_store_remove (GTK_LIST_STORE(treeModel),&iter));
	}
        while (gtk_events_pending()) gtk_main_iteration();
        TRACE("removed old stuff\n");*/

        int heartbeat = 0;

        auto reading = g_strdup_printf(_("Reading \"%s\""), path);
	view->page()->updateStatusLabel(reading);
	g_free(reading);
	while(gtk_events_pending())gtk_main_iteration();
    
        GList *directory_list = read_items (path, &heartbeat);
	// start adding items... (threaded...)

	auto arg = (void **)calloc(3, sizeof(void *));
	arg[0] = (void *)directory_list;
	arg[1] = (void *)view;
	arg[2] = (void *)path;
	pthread_t thread;
	pthread_create(&thread, NULL, threadInsert, (void *)arg);
	// detach
	pthread_detach(thread);
	return 0;
    }


   
    static gboolean
    isSelectable(GtkTreeModel *treeModel, GtkTreePath *tpath){
        GtkTreeIter iter;
        if (!gtk_tree_model_get_iter(treeModel, &iter, tpath)) {
            DBG("isSelectable() cannot get iter\n");
            return FALSE;
        }
        return isSelectable(treeModel, &iter);
    }
    
    static gboolean
    isSelectable(GtkTreeModel *treeModel, GtkTreeIter *iter){
        gchar *path;
	gtk_tree_model_get (treeModel, iter, DISPLAY_NAME, &path, -1);
        gboolean retval = TRUE;
        if (strcmp(path, "..")==0 )retval = FALSE;
        TRACE("is %s selectable? %d\n", path, retval);
        g_free(path);
        return retval;
    }

    static const gchar *
    get_xfdir_iconname(void){
	return "system-file-manager";
    }

    //FIXME: must be done by non main thread (already mutex protected)
    //       if this is going to take long (network connection...)
    static GList *
    read_items (const gchar *path,  gint *heartbeat)
    {
        GList *directory_list = NULL;
        if (!g_file_test(path, G_FILE_TEST_IS_DIR)){
            ERROR("fm/view/local/model.hh::read_items(): g_file_test(%s, G_FILE_TEST_IS_DIR) failed\n", path);
            return NULL;
        }
        TRACE( "readfiles: %s\n", path);
	errno=0;
        DIR *directory = opendir(path);
        if (!directory) {
	    Gtk<Type>::quickHelp(GTK_WINDOW(mainWindow), strerror(errno), "dialog-error");
            DBG("xfdir_local_c::read_items(): opendir %s: %s\n", path, strerror(errno));
	    errno=0;
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
            xd_t *xd_p = get_xd_p(path, d, FALSE);
            //xd_t *xd_p = get_xd_p(path, d, (count++ < 104));
            directory_list = g_list_prepend(directory_list, xd_p);
            if (heartbeat) {
                (*heartbeat)++;
                TRACE(stderr,"incrementing heartbeat records to %d\n", *heartbeat);
            }
        }
        if (errno) {
            ERROR("fm/view/local/model.hh::read_files_local: %s: %s\n", strerror(errno), path);
        }
    // unlock mutex
        pthread_mutex_unlock(&readdir_mutex);
        TRACE("-- mutex for %s released.\n", path);

        closedir (directory);

        // At least the ../ record should have been read. If this
        // is not so, then a read error occurred.
        // (not uncommon in bluetoothed obexfs)
        if (!directory_list) {
            ERROR("fm/view/local/model.hh::read_files_local(): Count failed! Directory not read!\n");
        }
        directory_list = sortList (directory_list);
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
        if (d->d_type == 0 || d->d_type == DT_UNKNOWN) withStat = TRUE;
        else xd_p->d_type = d->d_type;

	TRACE("model::get_xd_p() path=%s d_type = %d withStat=%d\n",
		xd_p->path,   xd_p->d_type, withStat);
	if (withStat){
	    xd_p->st = (struct stat *)calloc( 1, sizeof(struct stat));
	    if (!xd_p->st){
		ERROR("fm/view/local/model.hh::calloc(%s): %s\n", xd_p->path, strerror(errno));
		exit(1);
	    }
	    TRACE("get_xd_p:: stat %s\n", xd_p->path);
	    if (lstat(xd_p->path, xd_p->st) < 0) {
		TRACE("get_xd_p() stat(%s): %s (path has disappeared)\n", xd_p->path, strerror(errno));
	    } else {
		xd_p->d_type = LocalIcons<Type>::getDType(xd_p->path, xd_p->st);
	    }
	    errno=0;
	}

	// symlinks and directories are stat'd in getMimeType()  
	xd_p->mimetype = getMimeType(xd_p);
        // the following call uses xd_p->mimetype
        xd_p->icon = g_strdup(LocalIcons<Type>::getIconname(xd_p));

	TRACE("d_type: %s -> %d\n", xd_p->path, xd_p->d_type);
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
		ERROR("fm/view/local/model.hh::calloc: %s\n", strerror(errno));
		exit(1);
	    }
            errno=0;
	    TRACE("getMimeType:: stat %s (%s)\n", xd_p->path, mimetype);
	    if (stat(xd_p->path, xd_p->st)<0){
		if (strcmp(mimetype, "inode/symlink")==0 ){
		   //broken link
		} else DBG("getMimeType() for d_type:inode/directory stat: %s: %s\n", xd_p->path, strerror(errno));
                errno=0;
            } else {
		g_free(mimetype);
		mimetype = Mime<Type>::statMimeType(xd_p->st);
	    }
	}
        // on asyncronous nfs connections, d_type may resolve to inode/unknown 
	if (strcmp(mimetype,"inode/regular")==0 || strcmp(mimetype,"inode/unknown")==0){
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
	TRACE("compare %s --- %s\n", xd_a->d_name, xd_b->d_name);
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
    sortList(GList *list){
        // Default sort order:
        if (Settings<Type>::getSettingInteger("LocalView", "Descending") <= 0) {
            return g_list_sort (list,compare_by_name);
        } else {
            return g_list_sort (list,compare_by_name2);
        }
    }
	
    static void *replaceTreeModel(void *data){
	auto view = (View<Type> *)data; 
	auto tmp = view->treeModel();
	// set iconview/treeview treemodel
	view->setTreeModel(view->backTreeModel());
	view->setBackTreeModel(tmp);
	gtk_tree_view_set_model(view->treeView(), view->treeModel());
	gtk_icon_view_set_model(view->iconView(), view->treeModel());

	view->monitorObject()->setMonitorStore(GTK_LIST_STORE(view->treeModel()));
        TRACE("replaceTreeModel() *** localMonitor object= %p\n", view->monitorObject());
        // XXX This timing is correct for mountThread :-)    
        ((LocalMonitor<Type> *)(view->monitorObject()))->startMountThread();
	
        return NULL;

    }

public:

    static void *statusMessage(void *data){
	auto arg = (void **)data;
	auto view = (View<Type> *)arg[0]; 
	auto text = (const gchar *)arg[1]; 
        view->page()->showFmButtonBox();
	view->page()->updateStatusLabel(text);
	while(gtk_events_pending())gtk_main_iteration();
	return NULL;
    }

    static void *finishLoad(void *data){
        

        
	if (mainWindow && GTK_IS_WIDGET(mainWindow)) gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), TRUE);
	while(gtk_events_pending())gtk_main_iteration();
	return NULL;
    }
    

    static void statusLoadCount(View<Type> *view, int count, int total){
	gchar *text;
	if (count == total){
	    auto fileCount = g_strdup_printf("%0d", total);
	    text = g_strdup_printf(_("Files: %s"), fileCount); 
	    g_free(fileCount);
	} else {
	    text = g_strdup_printf(_("Loaded %d of %d articles"), count, total);
	}

	void *arg[]={
	    (void *)view,
	    (void *)text
	};
	Util<Type>::context_function(statusMessage, (void *)arg);
	g_free(text);
    }

    static void *threadInsert(void *data){
	auto arg = (void **)data;
	auto directory_list = (GList *)arg[0];
	auto view = (View<Type> *)arg[1];
	auto path = (const gchar *)arg[2];
	insert_list_into_model(directory_list, view, path);
	g_free(arg);
        GList *p = directory_list;
        for (;p && p->data; p=p->next){
            xd_t *xd_p = (xd_t *)p->data;
            free_xd_p(xd_p);
        }
        g_list_free(directory_list);
        // replaceTreeModel will fix treeModel used by monitorObject.
        TRACE("threadInsert-->replaceTreeModel() \n");
	Util<Type>::context_function(replaceTreeModel, (void *)view);
	// clear out backTreeModel
	gtk_list_store_clear (GTK_LIST_STORE(view->backTreeModel()));

        // Now you can fire up mountThread, not any sooner.

	Util<Type>::context_function(finishLoad, (void *)view);

	return NULL;

    }

    static void
    insert_list_into_model(GList *data, View<Type> *view, const gchar *path){
	//auto list_store = GTK_LIST_STORE(view->treeModel());
	auto list_store = GTK_LIST_STORE(view->backTreeModel());
	if(strcmp(path, "/")==0){
	    RootView<Type>::addXffmItem(GTK_TREE_MODEL(list_store));
	}
        GList *directory_list = (GList *)data;
        GList *l = directory_list;
        gint dir_count = g_list_length(directory_list);
	int count = 0;
        for (; l && l->data; l= l->next){
            xd_t *xd_p = (xd_t *)l->data;
            add_local_item(list_store, xd_p);
	    if (++count % 50 == 0){
		statusLoadCount(view, count, dir_count);
	    }
        }
	statusLoadCount(view, dir_count, dir_count);
    }

    static gboolean
    insertItem(GtkTreeModel *treeModel, GtkTreePath *tpath, GtkTreeIter *iter, gpointer data){
        gchar *path;
        // get current xd_p
        struct stat *st;
        guint size;
        guint date;
        guint type;
        gtk_tree_model_get(treeModel, iter, 
                PATH, &path, 
                SIZE, &size,
                DATE, &date,
                FLAGS, &type, 
                -1);
        gboolean up = (type&0x100);
        type &= 0xff;
        
        xd_t *xd_p = (xd_t *)data;
        xd_t *xd_b = (xd_t *)calloc(1, sizeof(xd_t));
        xd_b->path = path;
        xd_b->d_name = up?g_strdup(".."):g_path_get_basename(path);
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
    // This is for monitor insertion:
    static void
    insertLocalItem(GtkListStore *listStore, xd_t *xd_p){
        if (!xd_p->path) return;
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
        gboolean up = (strcmp(xd_p->d_name, "..")==0);

        TRACE("iconname, highlight: %s, %s\n", icon_name, highlight_name);
        GdkPixbuf *treeViewPixbuf = NULL;
        GdkPixbuf *normal_pixbuf = NULL;
        GdkPixbuf *highlight_pixbuf = NULL;
        if (xd_p->st) {
            auto type = xd_p->st->st_mode & S_IFMT;
            if (type == S_IFDIR) {
                highlight_pixbuf = Preview<Type>::loadFromThumbnails("document-open", xd_p->st, 48, 48);
                if (!highlight_pixbuf) {
                    auto thumbnail = Hash<Type>::get_thumbnail_path ("document-open", GTK_ICON_SIZE_DIALOG);
                    highlight_pixbuf = Pixbuf<Type>::get_pixbuf("document-open", GTK_ICON_SIZE_DIALOG);
                    Pixbuf<Type>::pixbuf_save(highlight_pixbuf, thumbnail);
                }
            }
	}

	if (g_path_is_absolute(icon_name))
	    normal_pixbuf = Preview<Type>::loadFromThumbnails(icon_name, xd_p->st, 48, 48);
      
        if (!treeViewPixbuf) 
	    treeViewPixbuf = Pixbuf<Type>::get_pixbuf(icon_name, -24);
        if (!normal_pixbuf) {
	    auto thumbnail = Hash<Type>::get_thumbnail_path (icon_name, GTK_ICON_SIZE_DIALOG);
	    normal_pixbuf = Pixbuf<Type>::get_pixbuf(icon_name, GTK_ICON_SIZE_DIALOG);
	    Pixbuf<Type>::pixbuf_save(normal_pixbuf, thumbnail);
	}
        //Highlight emblem macros are defined in types.h
        if (!highlight_pixbuf) {
            highlight_pixbuf = gdk_pixbuf_copy(normal_pixbuf);
            const gchar *emblem;
            if (strcmp(xd_p->d_name, "..")==0) emblem = HIGHLIGHT_UP_EMBLEM;
            else {
                emblem = HIGHLIGHT_EXEC_EMBLEM;
/*
                if (xd_p->st && (
                    (xd_p->st->st_mode & S_IXUSR) ||
                    (xd_p->st->st_mode & S_IXGRP) ||
                    (xd_p->st->st_mode & S_IXOTH) ))
                {
                    emblem = HIGHLIGHT_EXEC_EMBLEM;
                } else {
                    emblem = HIGHLIGHT_OPEN_EMBLEM;
                }
                */
            }
            // Now decorate the pixbuf with emblem (types.h).
            void *arg[] = {NULL, (void *)highlight_pixbuf, NULL, NULL, (void *)emblem };
            // Done by main gtk thread:
            Util<Type>::context_function(Icons<Type>::insert_decoration_f, arg);
	}
	if (xd_p->st){TRACE("xd_p->st is populated: %s\n", utf_name);}
	guint flags=(xd_p->d_type & 0xff);
        guint size = (xd_p->st)?xd_p->st->st_size:0;
        guint date = (xd_p->st)?xd_p->st->st_mtim.tv_sec:0;
        gchar *statInfo = NULL;
	// statInfo is too long for big directories, and only 
	// required for treeview...
	if (isTreeView) statInfo = Util<Type>::statInfo(xd_p->path);
        gchar **p = NULL;
        if (!statInfo) statInfo = g_strdup("");
        if (up) flags |= 0x100;
        gtk_list_store_set (list_store, iter, 
		FLAGS, flags,
                DISPLAY_NAME, utf_name,
                PATH, xd_p->path,
                ICON_NAME, icon_name,
                TREEVIEW_PIXBUF, treeViewPixbuf, 
                DISPLAY_PIXBUF, normal_pixbuf, 
                NORMAL_PIXBUF, normal_pixbuf, 
                HIGHLIGHT_PIXBUF, highlight_pixbuf, 
                //TYPE,xd_p->d_type, 
                SIZE,size, 
                DATE,date, 
                MIMETYPE, xd_p->mimetype,
                TOOLTIP_TEXT, statInfo,
                -1);
        g_free(statInfo);
        g_free(utf_name);
    }




};
}
#endif

