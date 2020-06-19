#ifndef XF_PKGVIEW__HH
#ifdef ENABLE_PKG_MODULE
# define XF_PKGVIEW__HH

#ifdef HAVE_EMERGE 
# include "emerge.hh"
#else
# ifdef HAVE_PACMAN
#  include "pacman.hh"
# else
#  include "pkg.hh"
# endif
#endif

#include "model.hh"
#include "popup.hh"

namespace xf
{

template <class Type> class View;
template <class Type> class LocalPopUp;
template <class Type> class BasePopUp;
template <class Type>
class PkgView  :
    public PkgModel<Type>
{
    public:
    static gboolean
    loadModel (View<Type> * view)
    {
        auto iconView = view->iconView();
        gtk_icon_view_set_selection_mode (iconView,GTK_SELECTION_SINGLE); 
        g_object_set_data(G_OBJECT(iconView), "iconViewType", (void *)"PkgView");
        view->disableDnD();        
        PkgModel<Type>::loadModel(view->treeModel());
        return TRUE;
    }

 
};
}
#endif
#endif
