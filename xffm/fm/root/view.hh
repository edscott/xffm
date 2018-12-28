#ifndef XF_ROOTVIEW__HH
# define XF_ROOTVIEW__HH
#include "model.hh"

namespace xf
{

template <class Type> class View;
template <class Type> class LocalPopUp;
template <class Type> class BasePopUp;
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
    static GtkMenu *popUpItem(void){
        if (!rootItemPopUp) rootItemPopUp = createItemPopUp();   
        return rootItemPopUp;
    }
    static GtkMenu *popUp(void){
        if (!rootPopUp) rootPopUp = createPopUp();   
        return rootPopUp;
    }
    
    static void
    resetMenuItems(void) {
        auto path = (const gchar *)g_object_get_data(G_OBJECT(rootItemPopUp), "path");
	gboolean isBookMark = RootView<Type>::isBookmarked(path);
	auto menuitem = GTK_WIDGET(g_object_get_data(G_OBJECT(rootItemPopUp), "Remove bookmark"));
	gtk_widget_set_sensitive(menuitem, isBookMark);

    }

    static void
    resetPopup(void) {
        DBG("reset root popup, is TreeView=%d\n", isTreeView);
        auto w = GTK_WIDGET(g_object_get_data(G_OBJECT(rootPopUp), "View as list"));
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w), isTreeView);

    }


    
private:

    static GtkMenu *createPopUp(void){
         menuItem_t item[]={
            {N_("View as list"), (void *)LocalPopUp<Type>::toggleView,  
		(void *)"TreeView", "window"},
            //{N_("Add bookmark"), (void *)BasePopUp<Type>::noop, NULL, NULL},
            {N_("Empty trash bin"), (void *)emptyTrash, NULL, NULL},
            {NULL,NULL,NULL, NULL}};
	rootPopUp = BasePopUp<Type>::createPopup(item); 
        auto text = g_strdup_printf("Xffm+-%s", VERSION);
        BasePopUp<Type>::changeTitle(rootPopUp, text, NULL);
        g_free(text);
        //decorateEditItems(localPopUp);
        return rootPopUp;        
    }  
    static GtkMenu *createItemPopUp(void){
	menuItem_t item[]=
        {
	    {N_("Open in New Tab"), (void *)LocalPopUp<Type>::newTab, NULL, NULL},
            {N_("Remove bookmark"), (void *)removeBookmarkItem, NULL, NULL},
	     {NULL,NULL,NULL,NULL}
        };
	rootItemPopUp = BasePopUp<Type>::createPopup(item); 
        auto text = g_strdup_printf("Xffm+-%s", VERSION);
        BasePopUp<Type>::changeTitle(rootItemPopUp, text, NULL);
        g_free(text);
        return rootItemPopUp;
    }

    static void
    removeBookmarkItem(GtkMenuItem *menuItem, gpointer data)
    {
        DBG("Remove bookmark\n");
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "path");
        if (!RootView<Type>::removeBookmark(path)) return;
	auto view =  (View<Type> *)g_object_get_data(G_OBJECT(data), "view");
	view->reloadModel();
    }

    static void
    emptyTrash(GtkMenuItem *menuItem, gpointer data) {
	auto trash = g_build_filename(g_get_home_dir(), ".local/share/Trash", NULL);
	Gio<Type>::doIt((GtkDialog *)NULL, trash, MODE_RM);
	auto view = (View<Type> *)g_object_get_data(G_OBJECT(data), "view");
	g_free(trash);
	view->reloadModel();

    }
	

};
}
#endif
