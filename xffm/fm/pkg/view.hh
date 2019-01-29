#ifndef XF_PKGVIEW__HH
# define XF_PKGVIEW__HH
#include "model.hh"

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


    static GtkMenu *popUpItem(void){
        DBG("PkgView popUpItem\n");
        /*if (!rootItemPopUp) rootItemPopUp = createItemPopUp();   
        return rootItemPopUp;*/
        return NULL;
    }
    static GtkMenu *popUp(void){
        DBG("PkgView popUp\n");
        /*if (!rootPopUp) rootPopUp = createPopUp();   
        return rootPopUp;*/
        return NULL;

    }
    
    static void
    resetMenuItems(void) {
        DBG("PkgView resetMenuItems\n");
   /*     auto path = (const gchar *)g_object_get_data(G_OBJECT(rootItemPopUp), "path");
	gboolean isBookMark = RootView<Type>::isBookmarked(path);
	auto menuitem = GTK_WIDGET(g_object_get_data(G_OBJECT(rootItemPopUp), "Remove bookmark"));
	gtk_widget_set_sensitive(menuitem, isBookMark);
        if (isBookMark) gtk_widget_show(menuitem);
        else gtk_widget_hide(menuitem);
	menuitem = GTK_WIDGET(g_object_get_data(G_OBJECT(rootItemPopUp), "Empty trash"));
        auto trashFiles = g_build_filename(g_get_home_dir(), ".local/share/Trash", NULL);
        if (strstr(path, trashFiles)) gtk_widget_show(menuitem);
        else gtk_widget_hide(menuitem);
        gtk_widget_set_sensitive(menuitem, g_file_test(trashFiles, G_FILE_TEST_IS_DIR));
       g_free(trashFiles); */

    }

    static void
    resetPopup(void) {
        DBG("PkgView resetPopup\n");
        TRACE("reset root popup, is TreeView=%d\n", isTreeView);
        //auto w = GTK_WIDGET(g_object_get_data(G_OBJECT(rootPopUp), "View as list"));
        //gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w), isTreeView);

    }


 
};
}
#endif
