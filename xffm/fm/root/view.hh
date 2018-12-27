#ifndef XF_ROOTVIEW__HH
# define XF_ROOTVIEW__HH
#include "model.hh"

namespace xf
{
static GtkMenu *rootPopUp=NULL;
static GtkMenu *rootItemPopUp=NULL;

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
            {N_("Add bookmark"), (void *)BasePopUp<Type>::noop, NULL, NULL},
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
            {N_("Remove bookmark"), (void *)BasePopUp<Type>::noop, NULL, NULL},
	     {NULL,NULL,NULL,NULL}
        };
	rootItemPopUp = BasePopUp<Type>::createPopup(item); 
        return rootItemPopUp;
    }

};
}
#endif
