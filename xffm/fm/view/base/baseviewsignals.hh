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


/////////////////////////////////  DnD   ///////////////////////////
    static void
    DragDataSend (GtkWidget * widget,
                       GdkDragContext * context, 
                       GtkSelectionData * selection_data, 
                       guint info, 
                       guint time,
                       gpointer data) {
        WARN("signal_drag_data_send\n");
        //g_free(files);
        
        //int drag_type;

	auto baseView = (BaseView<Type> *)data;
        /* prepare data for the receiver */
        if (info != TARGET_URI_LIST) {
            ERROR("signal_drag_data_send: invalid target");
        }
        DBG( ">>> DND send, TARGET_URI_LIST\n"); 
        gchar *dndData = NULL;
        switch (baseView->viewType()) {
            case (LOCALVIEW_TYPE):
            {
                dndData = LocalDnd<Type>::sendDndData(baseView);
                TRACE("drag finish result=%d\n", result);
                break;
            }

            default :
                DBG("BaseViewSignals:: sendDndData not defined for view type %d\n", baseView->viewType());
                break;
        }

        if (dndData){
            gtk_selection_data_set (selection_data, 
                gtk_selection_data_get_selection(selection_data),
                8, (const guchar *)dndData, strlen(dndData)+1);
        }
                    
    }
};
}
#endif
