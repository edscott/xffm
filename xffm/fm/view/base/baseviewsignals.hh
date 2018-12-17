#ifndef XF_BASEVIEWSIGNALS__HH
# define XF_BASEVIEWSIGNALS__HH

#include "fm/view/root/rootview.hh"
#include "common/pixbuf.hh"
#include "fm/view/local/localview.hh"

#define SET_DIR(x) x|=0x01
#define IS_DIR (x&0x01)
#define CONTROL_MODE (event->state & GDK_CONTROL_MASK)
#define SHIFT_MODE (event->state & GDK_SHIFT_MASK)
static gboolean dragOn_=FALSE;
static gboolean rubberBand_=FALSE;
static gint buttonPressX=-1;
static gint buttonPressY=-1;

static GtkTargetList *targets=NULL;
static GdkDragContext *context=NULL;

static gboolean controlMode = FALSE;

namespace xf
{


template <class Type> class LocalClipboard;
template <class Type> class BaseView;

template <class Type> 
class BaseViewSignals {
public:

    static gboolean
    unhighlight (gpointer key, gpointer value, gpointer data){
        auto baseView = (BaseView<Type> *)data;
        if (!baseView)  return FALSE;
        TRACE("unhighlight %s\n", (gchar *)key);
        GtkTreeModel *model =baseView->treeModel();
                
        GtkTreePath *tpath = gtk_tree_path_new_from_string ((gchar *)key);
        if (!tpath) return FALSE;

        GtkTreeIter iter;
        gboolean result = gtk_tree_model_get_iter (model, &iter, tpath);
        gtk_tree_path_free (tpath);

        if (!result) return TRUE;
        GdkPixbuf *normal_pixbuf;

        gtk_tree_model_get (model, &iter, 
                NORMAL_PIXBUF, &normal_pixbuf, -1);
        gtk_list_store_set (GTK_LIST_STORE(model), &iter,
                DISPLAY_PIXBUF, normal_pixbuf, 
            -1);
       return TRUE;

    }
    static void
    activate (GtkTreePath *tpath, gpointer data)
    {
        // Get activated path.
        auto baseView = (BaseView<Type> *)data;

	gchar *path;
        GtkTreeIter iter;
        auto treeModel = baseView->treeModel();
        gtk_tree_model_get_iter (treeModel, &iter, (GtkTreePath *)tpath);
        GdkPixbuf *normal_pixbuf;

        gtk_tree_model_get (treeModel, &iter, PATH, &path, -1);
	
        DBG("BaseView::activate: %s\n", path);
        auto lastPath = g_strdup(baseView->path());
	if (!baseView->loadModel(treeModel, tpath, path)){
            WARN("reloading %s\n", lastPath);
            baseView->loadModel(lastPath);
        }
	g_free(path);
	g_free(lastPath);
    }


};
}
#endif
