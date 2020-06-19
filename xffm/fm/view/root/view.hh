#ifndef XF_ROOTVIEW_HH
# define XF_ROOTVIEW_HH
#include "popup.hh"

namespace xf
{
template <class Type>
class RootView  :
    public RootModel<Type>
{
    public:
    static gboolean
    loadModel (View<Type> * view)
    {
        auto iconView = view->iconView();
        gtk_icon_view_set_selection_mode (iconView,GTK_SELECTION_SINGLE); 
        g_object_set_data(G_OBJECT(iconView), "iconViewType", (void *)"RootView");
        view->disableDnD();        
        RootModel<Type>::loadModel(view->treeModel());
        return TRUE;
    }
        
    static gboolean
    isSelectable(GtkTreeModel *treeModel, GtkTreeIter *iter){
        return TRUE;
       /* gchar *path;
        gtk_tree_model_get (treeModel, iter, DISPLAY_NAME, &path, -1);
        gboolean retval = TRUE;
        if (strcmp(path, "..")==0 )retval = FALSE;
        TRACE("is %s selectable? %d\n", path, retval);
        g_free(path);
        return retval;*/
    }

};
}
#endif
