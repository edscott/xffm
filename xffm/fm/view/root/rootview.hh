#ifndef XF_ROOTVIEW__HH
# define XF_ROOTVIEW__HH
#include "fm/model/rootmodel.hh"

namespace xf
{
static GtkMenu *rootPopUp=NULL;
static GtkMenu *rootItemPopUp=NULL;

template <class Type> class LocalPopUp;
template <class Type> class BasePopUp;
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
    resetLocalPopup(void) {
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
