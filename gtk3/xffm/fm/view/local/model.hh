#ifndef XF_LOCALMODEL__HH
# define XF_LOCALMODEL__HH
#include "common/util.hh"

// Linux has d_type, and FreeBSD12 now also has it
//#ifndef HAVE_STRUCT_DIRENT_D_TYPE
//#warning "HAVE_STRUCT_DIRENT_D_TYPE not defined"
//#endif

typedef struct xd_t{
    gchar *d_name;
    gchar *path;
    unsigned char d_type;
    struct stat *st;
    gchar *mimetype;
    gchar *icon;
}xd_t;
static pthread_mutex_t readdir_mutex=PTHREAD_MUTEX_INITIALIZER;
// XXX: This previewMutex could belong to view so that only busy tab
//      will be locked... It would have to go into base view...
//      But the same goes for readdir_mutex.
static pthread_mutex_t previewMutex=PTHREAD_MUTEX_INITIALIZER;

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
static pthread_mutex_t dateStringMutex=PTHREAD_MUTEX_INITIALIZER;
template <class Type> class LocalIcons;
template <class Type> class Preview;
template <class Type>
class LocalModel
{
public:
    static gchar *
    sizeString (size_t size) {
        if (size > 1024 * 1024 * 1024){
            return g_strdup_printf("%ld GB", size/(1024 * 1024 * 1024));
        }
        if (size > 1024 * 1024){
            return g_strdup_printf("%ld MB", size/(1024 * 1024));
        }
        if (size > 1024){
            return g_strdup_printf("%ld KB", size/(1024));
        }
        return g_strdup_printf("%ld ", size);
    }
    static gchar *
    dateString (time_t the_time) {
        pthread_mutex_lock(&dateStringMutex);

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
        pthread_mutex_unlock(&dateStringMutex);

        return date_string;
    }
    

    // This mkTreeModel should be static...
    static gint
    loadModel (View<Type> *view, const gchar *path)
    {
      // XXX: This screws up one tab if a different tab is loading...
      //      Create a mutex per each tab... 
      //      or do not loop a reload for all tabs (preferred)
#if 10
      if (pthread_mutex_trylock(&previewMutex) != 0){
        DBG("Image preview thread is locked.\n");
        auto text = g_strdup_printf("%s (%s)", 
            _("There are unfinished jobs: please wait until they are finished."), _("Previews"));
        DBG_("%s", text);
        g_free(text);
        gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), TRUE);
        return 0;
      }
      pthread_mutex_unlock(&previewMutex);
#endif

      // Verify conectivity:
      if (!Thread<Type>::fileTest(path,G_FILE_TEST_EXISTS)){
                gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), TRUE);
          ERROR_("no response: %s\n", path);
          return 0;
      }
        
      TRACE("*** local/model.hh loadModel()\n");
      auto treeModel = view->treeModel();


      auto reading = g_strdup_printf(_("Reading \"%s\""), path);
      view->page()->updateStatusLabel(reading);
      g_free(reading);
      while(gtk_events_pending())gtk_main_iteration();
      GList *directoryList = NULL;

      directoryList = read_items (path);

      /*auto inserting = g_strdup_printf("%s (%d)",_("Inserting rows..."), g_list_length(directoryList));
      view->page()->updateStatusLabel(reading);
      g_free(inserting);*/
      while(gtk_events_pending())gtk_main_iteration();

      // start adding items... (threaded...)
      auto arg = (void **)calloc(3, sizeof(void *));
      arg[0] = (void *)directoryList;
      arg[1] = (void *)view;
      arg[2] = (void *)g_strdup(path);
      auto text = g_strdup_printf("LocalModel::loadModel(): threadInsert(%s)", path);


      new(Thread<Type>)(text, threadInsert, (void *)arg);
      g_free(text);
      return 0;
    }


   
/*    static gboolean
    isSelectable(GtkTreeModel *treeModel, GtkTreePath *tpath){
        GtkTreeIter iter;
        if (!gtk_tree_model_get_iter(treeModel, &iter, tpath)) {
            DBG("isSelectable() cannot get iter\n");
            return FALSE;
        }
        return isSelectable(treeModel, &iter);
    }*/
    
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
        return FILE_MANAGER;
    }

    //XXX: Could be done by non main thread (already mutex protected)
    //       if this is going to take long (network connection...)
    static GList *
    read_items (const gchar *path)
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
            Dialogs<Type>::quickHelp(GTK_WINDOW(mainWindow), strerror(errno), "dialog-error");
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
        //TRACE( "shows hidden=%d\n", showHidden);
        gint count = 0;
        gboolean bySize = 
            (Settings<Type>::getInteger("LocalView", "BySize") > 0);
        gboolean byDate = 
            (Settings<Type>::getInteger("LocalView", "ByDate") > 0);
        gboolean byType = 
            (Settings<Type>::getInteger("LocalView", "ByType") > 0);

        auto needStat = bySize || byDate;
        //gboolean doPreviews = (Settings<Type>::getInteger("ImageSize", path) > 0);
        gboolean doPreviews = FALSE;
        while ((d = readdir(directory))  != NULL){
            TRACE( "%p  %s\n", d, d->d_name);
            if(strcmp (d->d_name, ".") == 0) continue;
            if (strcmp(path,"/")==0 && strcmp (d->d_name, "..") == 0) {
                TRACE("skipp: %s : %s\n", path, d->d_name);
                continue;
            }
            needStat = needStat || (d->d_type==DT_DIR);
            needStat = needStat || (d->d_type==DT_LNK);
            needStat = needStat || (d->d_type==DT_UNKNOWN);
            // On this pass, stat according to sort order or dt_type.
            xd_t *xd_p = get_xd_p(path, d, needStat, doPreviews);
            if (xd_p) directory_list = g_list_prepend(directory_list, xd_p);
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
    get_xd_p(const gchar *directory, struct dirent *d, gboolean withStat, gboolean doPreviews=FALSE){
        // Allocate memory.
        xd_t *xd_p = NULL;
        TRACE("sizeof(xd_t) = %lu\n", sizeof(xd_t));
        errno=0;
        xd_p = (xd_t *)calloc(1,sizeof(xd_t));
        if (!xd_p){
            DBG("get_xd_p(): %s\n", strerror(errno));
            errno=0;
        }
        // Duplicate basename.
        xd_p->d_name = g_strdup(d->d_name);
        // Duplicate absolute path.
        if (strcmp(d->d_name, "..")==0){
            // Up item.
            xd_p->path = g_path_get_dirname(directory);
        } else if (strcmp(directory, G_DIR_SEPARATOR_S)==0){
            // Filesystem root.
            xd_p->path = g_strconcat(G_DIR_SEPARATOR_S, d->d_name, NULL);
        } else {
            // All others.
            xd_p->path = g_strconcat(directory, G_DIR_SEPARATOR_S, d->d_name, NULL);
        }
        // Assign d_type.
        xd_p->d_type = d->d_type;

        TRACE("model::get_xd_p() path=%s d_type = %d withStat=%d\n",
                xd_p->path,   xd_p->d_type, withStat);
        // Stat necessary items.
        if (withStat){
            if (getStat(xd_p) != 0){
		DBG("getStat failed at get_xd_p()\n");
	    }
        }
        xd_p->mimetype = getMimeType(xd_p);
        // the following call uses xd_p->mimetype
        xd_p->icon = g_strdup(LocalIcons<Type>::getIconname(xd_p, doPreviews));
        TRACE("d_type: %s (%s) -> %d icon: %s mime: %s\n", xd_p->d_name, xd_p->path, xd_p->d_type, xd_p->icon, xd_p->mimetype);
        errno=0;
        return xd_p;
    }


    static int
    getStat(xd_t *xd_p){
        if (xd_p->st) return 0;
            // Allocate memory.
        xd_p->st = (struct stat *)calloc(1, sizeof(struct stat));
        if (!xd_p->st){
            ERROR("fm/view/local/model.hh::calloc: %s\n", strerror(errno));
        exitDialogs = true;
            exit(1);
        }
        auto retval = stat(xd_p->path, xd_p->st);
        if (retval < 0) {
            // File disappeared or broken link.
            WARN("getStat() stat(%s): %s\n", xd_p->path, strerror(errno));
            if (xd_p->d_type == DT_LNK){
                retval = lstat(xd_p->path, xd_p->st);
                if (retval < 0) {
                    WARN("getStat() lstat(%s): %s\n", xd_p->path, strerror(errno));
                }
            }
        } 
        errno=0;        
        return retval;
            
    }

    static gchar *
    getMimeType(xd_t *xd_p){
        auto mimetype = Mime<Type>::basicMimeType(xd_p->d_type);
        TRACE("%s -> %s\n", xd_p->path, mimetype);
        //if (xd_p->st==NULL && (xd_p->d_type == DT_LNK || xd_p->d_type == DT_DIR))
        if (xd_p->d_type == DT_LNK || xd_p->d_type == DT_DIR)
        {
            if (getStat(xd_p)==0){
                g_free(mimetype);
                mimetype = Mime<Type>::statMimeType(xd_p->st);
            }
        }
        // on asyncronous nfs connections, d_type may resolve to inode/unknown 
        if (strcmp(mimetype,"inode/regular")==0 || strcmp(mimetype,"inode/unknown")==0){
            auto type = MimeSuffix<Type>::mimeType(xd_p->path);
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


 /*   static gint 
    directoryTest(const void *a, const void *b, gboolean descending){
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
        if (a_cond && b_cond) {
            // directory comparison always by name.
            if (descending) return -strcasecmp(xd_a->d_name, xd_b->d_name);
            return strcasecmp(xd_a->d_name, xd_b->d_name);
        }
        return 0;

    }*/

    static gint 
    directoryTest(const void *a, const void *b, gboolean descending, int which){
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
        if (a_cond && b_cond) {
            // directory comparison by name is default;
            switch (which) {
                case 0: //date
                {
                    auto result = xd_a->st->st_mtime - xd_b->st->st_mtime;
                    if (descending) return -result;
                    break;
                }
                case 1: //size
                {
                    auto result = xd_a->st->st_size - xd_b->st->st_size;
                    if (descending) return -result;
                    break;
                }
                default:
                {
                    if (descending) return -strcasecmp(xd_a->d_name, xd_b->d_name);
                    return strcasecmp(xd_a->d_name, xd_b->d_name);
                    break;
                }
            }
        }
        return 0;

    }

    static gint 
    compareBySize(const void *a, const void *b, gboolean descending){
        auto test = directoryTest(a, b, descending, 1);
        if (test != 0) return test;

        const xd_t *xd_a = (const xd_t *)a;
        const xd_t *xd_b = (const xd_t *)b;

        auto result = xd_a->st->st_size - xd_b->st->st_size;
        if (descending) return -result;
        return result;
    }

    static gint 
    compareByDate(const void *a, const void *b, gboolean descending){
        auto test = directoryTest(a, b, descending, 0);
        if (test != 0) return test;

        const xd_t *xd_a = (const xd_t *)a;
        const xd_t *xd_b = (const xd_t *)b;

        auto result = xd_a->st->st_mtime - xd_b->st->st_mtime;
        if (descending) return -result;
        return result;
    }

    static gint 
    compareByDateSize(const void *a, const void *b, gboolean descending){
        auto test = directoryTest(a, b, descending, 0);
        if (test != 0) return test;

        const xd_t *xd_a = (const xd_t *)a;
        const xd_t *xd_b = (const xd_t *)b;

        auto result = xd_a->st->st_mtime - xd_b->st->st_mtime;
        if (result == 0) result = xd_a->st->st_size - xd_b->st->st_size;
        if (descending) return -result;
        return result;
    }

    static gint 
    compareByName(const void *a, const void *b, gboolean descending){
        auto test = directoryTest(a, b, descending,2);
        if (test != 0) return test;
        const xd_t *xd_a = (const xd_t *)a;
        const xd_t *xd_b = (const xd_t *)b;
        
        if (descending) return -strcasecmp(xd_a->d_name, xd_b->d_name);
        return strcasecmp(xd_a->d_name, xd_b->d_name);
    }

    static gint 
    compareByType(const void *a, const void *b, gboolean descending){
        auto test = directoryTest(a, b, descending,2);
        if (test != 0) return test;
        auto sign = (descending)? -1: 1;
        const xd_t *xd_a = (const xd_t *)a;
        const xd_t *xd_b = (const xd_t *)b;
        
        gchar *ext_a = strrchr(xd_a->d_name,'.');
        gchar *ext_b = strrchr(xd_b->d_name,'.');
        if (!ext_a && !ext_b){
          return sign * strcasecmp(xd_a->d_name, xd_b->d_name);
        }
        if (!ext_a) return sign;
        if (!ext_b) return -sign;

        return sign * strcasecmp(ext_a, ext_b);
    }


    static gint 
    compareByNameType(const void *a, const void *b, gboolean descending){
        auto test = directoryTest(a, b, descending,2);
        if (test != 0) return test;
        auto result = compareByType(a, b, descending);
        if (result) return result;
        // subsorting
        const xd_t *xd_a = (const xd_t *)a;
        const xd_t *xd_b = (const xd_t *)b;
        auto sign = (descending)? -1: 1;
        return  sign * strcasecmp(xd_a->d_name, xd_b->d_name);
    }

    static gint 
    compareByDateType(const void *a, const void *b, gboolean descending){
        auto test = directoryTest(a, b, descending,2);
        if (test != 0) return test;
        auto result = compareByType(a, b, descending);
        if (result) return result;
        // subsorting
        const xd_t *xd_a = (const xd_t *)a;
        const xd_t *xd_b = (const xd_t *)b;
        auto sign = (descending)? -1: 1;
        return  compareByDate(a, b, descending);
    }

    static gint 
    compareBySizeType(const void *a, const void *b, gboolean descending){
        auto test = directoryTest(a, b, descending,2);
        if (test != 0) return test;
        auto result = compareByType(a, b, descending);
        if (result) return result;
        // subsorting
        const xd_t *xd_a = (const xd_t *)a;
        const xd_t *xd_b = (const xd_t *)b;
        auto sign = (descending)? -1: 1;
        return  compareByDate(a, b, descending);
    }

    static gint 
    compareByDateSizeType(const void *a, const void *b, gboolean descending){
        auto test = directoryTest(a, b, descending,2);
        if (test != 0) return test;
        auto result = compareByType(a, b, descending);
        if (result) return result;
        // subsorting
        const xd_t *xd_a = (const xd_t *)a;
        const xd_t *xd_b = (const xd_t *)b;
        auto sign = (descending)? -1: 1;
        return  compareByDateSize(a, b, descending);
    }

    static gint
    compareByTypeUp (const void *a, const void *b) {
        return compareByNameType(a, b, FALSE);
    }
    
    static gint
    compareByTypeDown (const void *a, const void *b) {
        return compareByNameType(a, b, TRUE);
    }

 
    static gint
    compareByDateTypeUp (const void *a, const void *b) {
        return compareByDateType(a, b, FALSE);
    }
    
    static gint
    compareByDateTypeDown (const void *a, const void *b) {
        return compareByDateType(a, b, TRUE);
    }


    static gint
    compareBySizeTypeUp (const void *a, const void *b) {
        return compareBySizeType(a, b, FALSE);
    }
    
    static gint
    compareBySizeTypeDown (const void *a, const void *b) {
        return compareBySizeType(a, b, TRUE);
    }

    static gint
    compareByDateSizeTypeUp (const void *a, const void *b) {
        return compareByDateSizeType(a, b, FALSE);
    }
    
    static gint
    compareByDateSizeTypeDown (const void *a, const void *b) {
        return compareByDateSizeType(a, b, TRUE);
    }

    static gint
    compareByNameUp (const void *a, const void *b) {
        return compareByName(a, b, FALSE);
    }
    
    static gint
    compareByNameDown (const void *a, const void *b) {
        return compareByName(a, b, TRUE);
    }

    static gint
    compareByDateUp (const void *a, const void *b) {
        return compareByDate(a, b, FALSE);
    }
    
    static gint
    compareByDateDown (const void *a, const void *b) {
        return compareByDate(a, b, TRUE);
    }

    static gint
    compareByDateSizeUp (const void *a, const void *b) {
        return compareByDateSize(a, b, FALSE);
    }
    
    static gint
    compareByDateSizeDown (const void *a, const void *b) {
        return compareByDateSize(a, b, TRUE);
    }

    static gint
    compareBySizeUp (const void *a, const void *b) {
        return compareBySize(a, b, FALSE);
    }
    
    static gint
    compareBySizeDown (const void *a, const void *b) {
        return compareBySize(a, b, TRUE);
    }
private:

    static GList *
    sortList(GList *list){
      // Default sort order:
      gboolean descending = Settings<Type>::getInteger("LocalView", "Descending") > 0;
      gboolean bySize = (Settings<Type>::getInteger("LocalView", "BySize") > 0);
      gboolean byDate = (Settings<Type>::getInteger("LocalView", "ByDate") > 0);
      gboolean byType = (Settings<Type>::getInteger("LocalView", "ByType") > 0);
      if (bySize || byDate){
        for (auto l=list; l && l->data; l=l->next){
          auto xd_p = (xd_t *) l->data;
          if (!xd_p->st) {
            if (getStat(xd_p) != 0) DBG("getStat failed at sortList()\n");
          }
        }
      }

      if (descending) { // byDate takes presedence over bySize...
        if (byDate && bySize && byType) return g_list_sort (list,compareByDateSizeTypeDown); 
        if (byDate && bySize) return g_list_sort (list,compareByDateSizeDown); 
        else if (byDate && byType) return g_list_sort (list,compareByDateTypeDown); 
        else if (byType && bySize) return g_list_sort (list,compareBySizeTypeDown); 

        else if (byDate) return g_list_sort (list,compareByDateDown); 
        else if (bySize) return g_list_sort (list,compareBySizeDown); 
        else if (byType) return g_list_sort (list,compareByTypeDown); 
        else return g_list_sort (list,compareByNameDown);
      } else {
        if (byDate && bySize && byType) return g_list_sort (list,compareByDateSizeTypeUp); 
        if (byDate && bySize) return g_list_sort (list,compareByDateSizeUp); 
        else if (byDate && byType) return g_list_sort (list,compareByDateTypeUp); 
        else if (byType && bySize) return g_list_sort (list,compareBySizeTypeUp); 

        else if (byDate) return g_list_sort (list,compareByDateUp); 
        else if (bySize) return g_list_sort (list,compareBySizeUp);
        else if (byType) return g_list_sort (list,compareByTypeUp);
        else return g_list_sort (list,compareByNameUp);
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

        auto p = new(LocalMonitor<Type>)(view->treeModel(), view);
        p->start_monitor(view, view->path());
        // deprecated view->monitorObject()->setMonitorStore(GTK_LIST_STORE(view->treeModel()));
        TRACE("replaceTreeModel() *** localMonitor object= %p\n", view->monitorObject());
        // XXX This timing is correct for mountThread :-)  

        //((LocalMonitor<Type> *)(view->monitorObject()))->startMountThread();
        
        return NULL;

    }

public:

    static void *statusMessage(void *data){
        auto arg = (void **)data;
        auto view = (View<Type> *)arg[0]; 
        auto text = (const gchar *)arg[1]; 
        view->page()->showFmButtonBox(FALSE);
        view->page()->updateStatusLabel(text);
        while(gtk_events_pending())gtk_main_iteration();
        return NULL;
    }

    static void *finishLoad(void *data){
         auto view = (View<Type> *)data; 
        if (isTreeView) {
            gtk_tree_view_columns_autosize(view->treeView());
        }
       

        
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

    static gboolean
    updateIcon(GtkTreeModel *treeModel, GtkTreePath *tpath,
            GtkTreeIter *iter, gpointer data){
        auto pixels = GPOINTER_TO_INT(data);
        // Get path.
        gchar *path;
        gchar *mimetype;
        gtk_tree_model_get(treeModel, iter, PATH, &path,
                MIMETYPE, &mimetype, -1);
        // get icon name.
        auto iconName = LocalIcons<Type>::getBasicIconname(path, mimetype, TRUE);
        if (!iconName || !g_path_is_absolute(iconName)){
            g_free(path);
            g_free(iconName);
            g_free(mimetype);
            return FALSE;
        }
        // 
        // Create Pixbuf.
        auto normal_pixbuf = Pixbuf<Type>::getImageAtSize(iconName, pixels, mimetype);

        g_free(path);
        g_free(iconName);
        g_free(mimetype);

        if (normal_pixbuf) {
            //auto highlight_pixbuf = gdk_pixbuf_copy(normal_pixbuf);
            // Update treestore. 
            // This should be done in the graphic context thread.
            void *arg[]={(void *)treeModel, (void *)iter, (void *)normal_pixbuf};
            Util<Type>::context_function(updatePixbuf_f, (void *)arg);
        }
        
        return FALSE;
    }
    static void *
    updatePixbuf_f (void *data){
        auto arg = (void **)data;
        auto treeModel = (GtkTreeModel *) arg[0];
        auto iter = (GtkTreeIter *) arg[1];
        auto normal_pixbuf = (GdkPixbuf *) arg[2];
        gtk_list_store_set (GTK_LIST_STORE(treeModel), iter, 
                DISPLAY_PIXBUF, normal_pixbuf, 
                NORMAL_PIXBUF, normal_pixbuf, 
                HIGHLIGHT_PIXBUF, normal_pixbuf, //highlight_pixbuf, 
       -1);
        return NULL;
    }

    static void updateIcons(View<Type> *view, GList *directoryList, const gchar *path){
        // if no previews, return.
        gint pixels = Settings<Type>::getInteger("ImageSize", path);
        if (pixels <= 24) {
            TRACE("No previews for %s\n", path);
            return;
        }
        TRACE("Previews on for %s, size=%d\n", path, pixels);
        // Status line with preview size information. 
        // XXX

        auto treeModel = view->treeModel();
        gtk_tree_model_foreach(treeModel, updateIcon, GINT_TO_POINTER(pixels));
        return;
    }

    static void *threadInsert(void *data){
        pthread_mutex_lock(&previewMutex);
        auto arg = (void **)data;
        auto directoryList = (GList *)arg[0];
        auto view = (View<Type> *)arg[1];
        auto path = (gchar *)arg[2];
        time_t start = time(NULL);
        TRACE("local/threadInsert() ***Starting... \n");
        insert_list_into_model(directoryList, view, path);
        TRACE("local/threadInsert() *** done... at %ld seconds \n", time(NULL)-start);
        // replaceTreeModel will fix treeModel used by monitorObject.
        Util<Type>::context_function(replaceTreeModel, (void *)view);
        // clear out backTreeModel
        gtk_list_store_clear (GTK_LIST_STORE(view->backTreeModel()));

        Util<Type>::context_function(finishLoad, (void *)view);
        
        updateIcons(view, directoryList, path);
        statusLoadCount(view, g_list_length(directoryList),g_list_length(directoryList));
        g_free(arg);
        GList *p = directoryList;
        for (;p && p->data; p=p->next){
            xd_t *xd_p = (xd_t *)p->data;
            free_xd_p(xd_p);
        }
        g_list_free(directoryList);
        g_free(path);
        pthread_mutex_unlock(&previewMutex);
       
        return NULL;

    }

    static void
    insert_list_into_model(GList *data, View<Type> *view, const gchar *path){
        //auto list_store = GTK_LIST_STORE(view->treeModel());
        auto list_store = GTK_LIST_STORE(view->backTreeModel());
        if(strcmp(path, "/")==0){
            TRACE("adding root item to \"%s\"\n", path);
            RootView<Type>::addXffmItem(GTK_TREE_MODEL(list_store));
        } else {
            TRACE("not adding root item to \"%s\"\n", path);
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
                FLAGS, &type, 
                -1);
        gboolean up = (type&0x100);
        type &= 0xff;
        
        xd_t *xd_p = (xd_t *)data;
        xd_t *xd_b = (xd_t *)calloc(1, sizeof(xd_t));
        xd_b->path = path;
        xd_b->d_name = up?g_strdup(".."):g_path_get_basename(path);
        xd_b->d_type = type;
        //TRACE("compare %s with iconview item \"%s\"\n", xd_p->d_name, xd_b->name);
        gint sortResult;
        gboolean descending = Settings<Type>::getInteger("LocalView", "Descending") > 0;
        gboolean bySize = (Settings<Type>::getInteger("LocalView", "BySize") > 0);
        gboolean byDate = (Settings<Type>::getInteger("LocalView", "ByDate") > 0);
        gboolean byType = (Settings<Type>::getInteger("LocalView", "ByType") > 0);
        if ((bySize || byDate) && !xd_b->st) {
    	    if (getStat(xd_b) != 0) DBG("getStat failed at insertItem()\n");
	    //getStat(xd_b);
	}
        if (byDate && bySize && byType) sortResult = compareByDateSizeType((void *)xd_p, (void *)(xd_b), descending);
        else if (byDate && bySize) sortResult = compareByDateSize((void *)xd_p, (void *)(xd_b), descending);
        else if (byDate) sortResult = compareByDate((void *)xd_p, (void *)(xd_b), descending);
        else if (bySize) sortResult = compareBySize((void *)xd_p, (void *)(xd_b), descending);
        else if (byType) sortResult = compareByType((void *)xd_p, (void *)(xd_b), descending);
        else sortResult = compareByName((void *)xd_p, (void *)(xd_b), descending);

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
        gboolean showHidden = (Settings<Type>::getInteger("LocalView", "ShowHidden") > 0);
        if (!showHidden && xd_p->d_name[0] == '.'  && strcmp("..", xd_p->d_name)){
            TRACE("add local item returns on %s\n", xd_p->d_name);
            return;
        }
        gboolean showBackups = (Settings<Type>::getInteger("LocalView", "ShowBackups") > 0);
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
          /* 
            // this will chop off extension if less than EXTENSION_LABEL_LENGTH
            // (obsolete)
            gchar *t = g_strdup(xd_p->d_name);
            if (strchr(t, '.') && strrchr(t, '.') != t){
                if (strlen(strrchr(t, '.')+1) <= EXTENSION_LABEL_LENGTH) {
                    *strrchr(t, '.') = 0;
                    g_free(utf_name);
                    utf_name = Util<Type>::utf_string(t);
                } 
                g_free(t);
            }*/
        }
        gboolean up = (strcmp(xd_p->d_name, "..")==0);

        TRACE("iconname, : %s\n", icon_name);
        GdkPixbuf *treeViewPixbuf = NULL;
        GdkPixbuf *normal_pixbuf = NULL;
        GdkPixbuf *highlight_pixbuf = NULL;

        auto dir = g_path_get_dirname(xd_p->path);
        auto pixels = Settings<Type>::getInteger("ImageSize", dir);
        gboolean doPreview = (pixels >= 48);
        
        g_free(dir);

        //fprintf (stderr, "%s: mimetype=%s\n", filePath, mimetype);
        if (doPreview) {
          if (!xd_p->st) getStat(xd_p);
          if (!xd_p->mimetype)xd_p->mimetype = getMimeType(xd_p);
        }
        
        if (doPreview && strcmp(xd_p->mimetype,"inode/regular")==0){
          gchar *m2 = Util<Type>::fileInfo(xd_p->path);
          if (strstr(m2, "text")) {// Further info necesary on inode/regular
            g_free(xd_p->mimetype);
            xd_p->mimetype = g_strdup("text/plain");
          }         
        }
        if (doPreview) {
          if (pixels != 384){
            if (strncmp(xd_p->mimetype, "text", strlen("text"))==0) doPreview = FALSE;
            if (xd_p->d_type  == DT_DIR)  doPreview = FALSE;
          }
        }

        // obsolete: ?
/*        if (icon_name && g_path_is_absolute(icon_name)){
            
            DBG("add_local_item(%s): pixels = %d \n", icon_name, pixels);
            if (doPreview) {
                normal_pixbuf = 
                    Pixbuf<Type>::getImageAtSize(icon_name, pixels, xd_p->mimetype, xd_p->st);
                if (strchr(xd_p->path, '.')){
                  void *arg[] = {NULL, (void *)normal_pixbuf, (void *)(strrchr(xd_p->path, '.')), NULL, NULL };
                  // Done by main gtk thread:
                  Util<Type>::context_function(Pixbuf<Type>::insert_decoration_f, arg);
                }

                treeViewPixbuf = 
                    Pixbuf<Type>::getImageAtSize(icon_name, 24, xd_p->mimetype);
            } else {
                normal_pixbuf = 
                    Pixbuf<Type>::getPixbuf(IMAGE_X_GENERIC, -48);
                treeViewPixbuf = 
                    Pixbuf<Type>::getPixbuf(IMAGE_X_GENERIC, -24);
            }
        }*/


        //fprintf(stderr, "1) %s: mime=%s pixbuf=%p\n", xd_p->path, xd_p->mimetype, normal_pixbuf);
        
        if (xd_p->d_type  == DT_DIR) {
            if (up) highlight_pixbuf = Pixbuf<Type>::getPixbuf(HIGHLIGHT_UP, -48);
            else {
              if (pixels != 384) highlight_pixbuf = Pixbuf<Type>::getPixbuf(DOCUMENT_OPEN, -48);
            }
        }

      
        if (!treeViewPixbuf){ 
            treeViewPixbuf = Pixbuf<Type>::getPixbuf(icon_name, -24);
        }
        if (!normal_pixbuf) {
            if (doPreview) {
              normal_pixbuf = Pixbuf<Type>::getPreview(pixels, xd_p->path, xd_p->mimetype, xd_p->st);  
//              normal_pixbuf = Pixbuf<Type>::getPreview(xd_p->path, xd_p->mimetype, xd_p->st);  
              const char *tag = "";
              if (xd_p->d_type  == DT_DIR) tag = _("Directory");
#ifdef ADD_EXTENSION_TAG
              else if (strchr(xd_p->path, '.')) tag = strrchr(xd_p->path, '.');
#endif
              if (strlen(tag)){
                TRACE("insert_decoration_f for %s %s\n", xd_p->path,tag);
                void *arg[] = {NULL, (void *)normal_pixbuf, (void *)(tag), NULL, NULL };
                // Done by main gtk thread:
                Util<Type>::context_function(Pixbuf<Type>::insert_decoration_f, arg);
              }

            } else {
              TRACE("normal pixbuf %s\n", icon_name);
              normal_pixbuf = Pixbuf<Type>::getPixbuf(icon_name, -48);
            }
            
        }
        //fprintf(stderr, "2) %s: mime=%s pixbuf=%p\n", xd_p->path, xd_p->mimetype, normal_pixbuf);
        //Highlight emblem macros are defined in types.h
        //
        // Decorate highlight pixbuf
        // (duplicate code in monitor.hh/model.hh)
        if (!highlight_pixbuf) {
            highlight_pixbuf = gdk_pixbuf_copy(normal_pixbuf);
        
            const gchar *emblem= "";
            if (xd_p->d_type  == DT_DIR) {
              emblem = up?HIGHLIGHT_UP:HIGHLIGHT_JUMP;
            }
            else emblem = HIGHLIGHT_APP;   

            // Now decorate the pixbuf with emblem (types.h).
            void *arg[] = {NULL, (void *)highlight_pixbuf, NULL, NULL, (void *)emblem };
            // Done by main gtk thread:
            Util<Type>::context_function(Pixbuf<Type>::insert_decoration_f, arg);
        }
        
        if (xd_p->st){TRACE("xd_p->st is populated: %s\n", utf_name);}
        gchar *statInfo = NULL;
        // statInfo is too long for big directories, and only 
        // required for treeview...
        if (isTreeView) {
            if (getStat(xd_p) != 0){
		TRACE("getStat failed at add_local_item\n");
	    }
            TRACE("getstat for %s\n", xd_p->path);
            statInfo = Util<Type>::statInfo(xd_p->st);
        }
         guint flags=(xd_p->d_type & 0xff);
        auto size = sizeString((xd_p->st)?xd_p->st->st_size:0);
        auto date = dateString((xd_p->st)?xd_p->st->st_mtime:0);
        gchar **p = NULL;
        if (!statInfo) statInfo = g_strdup("");
        if (up) flags |= 0x100;
        TRACE("local/model gtk_list_store_set(%s)\n", icon_name);
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
        g_free(date);
        g_free(size);
    }




};
}
#endif

