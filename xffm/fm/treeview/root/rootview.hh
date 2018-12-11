#ifndef XF_ROOTVIEW__HH
# define XF_ROOTVIEW__HH
#include "fm/model/rootmodel.hh"

namespace xf
{

template <class Type>
class RootView  :
    public RootModel<Type>
{
    public:
    static gboolean
    loadModel (BaseView<Type> * baseView)
    {
        auto iconView = baseView->iconView();
        gtk_icon_view_set_selection_mode (iconView,GTK_SELECTION_SINGLE); 
        g_object_set_data(G_OBJECT(iconView), "iconViewType", (void *)"RootView");
	baseView->disableDnD();	
	RootModel<Type>::loadModel(baseView->treeModel());
	return TRUE;
    }

};
}
#endif
