#ifndef  XF_BASEMODEL__HH
# define XF_BASEMODEL__HH

enum
{
    ROOTVIEW_TYPE,
    LOCALVIEW_TYPE,
    FSTAB_TYPE,
    NFS_TYPE,
    SSHFS_TYPE,
    ECRYPTFS_TYPE,
    CIFS_TYPE,
    PKG_TYPE
};

enum
{
  FLAGS,
  DISPLAY_PIXBUF,
  NORMAL_PIXBUF,
  HIGHLIGHT_PIXBUF,
  TOOLTIP_PIXBUF,
  DISPLAY_NAME,
  ACTUAL_NAME,
  PATH,
  SIZE,
  DATE,
  TOOLTIP_TEXT,
  ICON_NAME,
  TYPE,
  MIMETYPE, 
  PREVIEW_PATH,
  PREVIEW_TIME,
  PREVIEW_PIXBUF,
  NUM_COLS
};

enum {
    TARGET_URI_LIST,
    TARGETS
};

#define URIFILE "file://"

namespace xf
{
template <class Type> class LocalMonitor;
template <class Type> class LocalView;
template <class Type> class LocalDnd;
template <class Type> class FstabMonitor;
template <class Type> class BaseViewSignals;
template <class Type> class RootView;
template <class Type> class Fstab;

template <class Type>
class BaseModel
{
    using util_c = Util<Type>;
    using page_c = Page<Type>;
    
    Page<Type> *page_;
    gchar *path_;
protected:
    GList *selectionList_;
    LocalMonitor<Type> *localMonitor_;
    FstabMonitor<Type> *fstabMonitor_;
    GtkTreeModel *treeModel_;
    
public:    
    BaseModel(Page<Type> *page){
	page_ = page; 
        path_ = NULL;
        selectionList_ = NULL;
        localMonitor_ = NULL;
        fstabMonitor_ = NULL;
	treeModel_ = mkTreeModel();

	
    }
    ~BaseModel(void){
        TRACE("BaseModel destructor.\n");
	
        g_free(path_); 
        g_object_unref(treeModel_);
    }


    Page<Type> *page(void){return page_;}
    const gchar *path(){return path_;}
    GtkTreeModel *treeModel(void){return treeModel_;}

    void setPath(const gchar *path){
        g_free(path_);
        if (path) path_ = g_strdup(path);
        else {
            ERROR("baseView::setPath(NULL)\n");
            exit(1);
        }

        TRACE("Baseview:: setPath()\n");
        if (g_file_test(path_, G_FILE_TEST_IS_DIR)){
            page_->setPageWorkdir(path_);
        } else {
            page_->setPageWorkdir(g_get_home_dir());
        }
    }
    gchar *
    get_window_name (void) {
        gchar *iconname;
        if(!path_) {
            iconname = util_c::utf_string (g_get_host_name());
        } else if(g_path_is_absolute(path_) &&
                g_file_test (path_, G_FILE_TEST_EXISTS)) {
            gchar *basename = g_path_get_basename (path_);
            gchar *pathname = g_strdup (path_);
            gchar *b = util_c::utf_string (basename);   // non chopped
            util_c::chop_excess (pathname);
            gchar *q = util_c::utf_string (pathname);   // non chopped

            g_free (basename);
            g_free (pathname);
            //iconname = g_strconcat (display_host, ":  ", b, " (", q, ")", NULL);
            iconname = g_strconcat (b, " (", q, ")", NULL);
            g_free (q);
            g_free (b);
        } else {
            iconname = util_c::utf_string (path_);
            util_c::chop_excess (iconname);
        }

#ifdef DEBUG
        gchar *gg = g_strdup_printf("%s-%d-D", iconname, getpid());
        g_free(iconname);
        iconname = gg;
#else
#ifdef CORE
        gchar *gg = g_strdup_printf("%s-%d-C", iconname, getpid());
        g_free(iconname);
        iconname = gg;
#endif
#endif
        return (iconname);
    }

    
    // This mkTreeModel should be static...
    static GtkTreeModel *
    mkTreeModel (void)
    {

	GtkTreeIter iter;
	GtkListStore *list_store = gtk_list_store_new (NUM_COLS, 
	    G_TYPE_UINT,      // flags
	    GDK_TYPE_PIXBUF, // icon in display
	    GDK_TYPE_PIXBUF, // normal icon reference
	    GDK_TYPE_PIXBUF, // highlight icon reference
	    GDK_TYPE_PIXBUF, // preview, tooltip image (cache)
	    G_TYPE_STRING,   // name in display (UTF-8)
	    G_TYPE_STRING,   // name from filesystem (verbatim)
	    G_TYPE_STRING,   // path (verbatim)
            G_TYPE_UINT,     // date
            G_TYPE_UINT,     // size
	    G_TYPE_STRING,   // tooltip text (cache)
	    G_TYPE_STRING,   // icon identifier (name or composite key)
	    G_TYPE_INT,      // mode (to identify directories)
	    G_TYPE_STRING,   // mimetype (further identification of files)
	    G_TYPE_STRING,   // Preview path
	    G_TYPE_UINT,      // Preview time
	    GDK_TYPE_PIXBUF  // Preview pixbuf
            ); // 
	return GTK_TREE_MODEL (list_store);
    }
    
    static gint
    getViewType(const gchar *path){
        if (!path) return ROOTVIEW_TYPE;
        if (g_file_test(path, G_FILE_TEST_EXISTS)) return (LOCALVIEW_TYPE);
        if (strcmp(path, "xffm:local")==0) return (LOCALVIEW_TYPE);
        if (strcmp(path, "xffm:root")==0) return (ROOTVIEW_TYPE);
        if (strcmp(path, "xffm:fstab")==0) return (FSTAB_TYPE);
        if (strcmp(path, "xffm:nfs")==0) return (NFS_TYPE);
        if (strcmp(path, "xffm:sshfs")==0) return (SSHFS_TYPE);
        if (strcmp(path, "xffm:ecryptfs")==0) return (ECRYPTFS_TYPE);
        if (strcmp(path, "xffm:cifs")==0) return (CIFS_TYPE);
        if (strcmp(path, "xffm:pkg")==0) return (PKG_TYPE);
        ERROR("BaseView::loadModel() %s not defined.\n", path);
        return (ROOTVIEW_TYPE);
    }

    
};
}
#endif
