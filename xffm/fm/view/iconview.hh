#ifndef XF_ICONVIEW__HH
# define XF_ICONVIEW__HH
namespace xf
{

template <class Type>
class IconView {

public: 
    static GtkIconView *createIconview(View<Type> *view){
        auto iconView = GTK_ICON_VIEW(gtk_icon_view_new());




        g_object_set(G_OBJECT(iconView), "has-tooltip", TRUE, NULL);
        gtk_icon_view_set_item_width (iconView, 60);
        gtk_icon_view_set_activate_on_single_click(iconView, TRUE);

 	
	g_object_set_data(G_OBJECT(view->treeModel()), "iconview", iconView); //do we really need this?
	gtk_icon_view_set_model(iconView, view->treeModel());

	gtk_icon_view_set_text_column (iconView, DISPLAY_NAME);
	gtk_icon_view_set_pixbuf_column (iconView,  DISPLAY_PIXBUF);
	gtk_icon_view_set_selection_mode (iconView, GTK_SELECTION_SINGLE);
        setUpSignals(view, G_OBJECT(iconView));
       
        return iconView;
    }

private:

    static void
    setUpSignals(View<Type> *view, GObject * iconView){
        g_signal_connect (iconView, "item-activated", 
            ICONVIEW_CALLBACK (BaseSignals<Type>::activate), 
            (void *)view);
        g_signal_connect (iconView, "motion-notify-event", 
            ICONVIEW_CALLBACK (BaseSignals<Type>::motionNotifyEvent), 
            (void *)view);
         //g_signal_connect (iconView, "query-tooltip", 
           //     G_CALLBACK (query_tooltip_f), 
           //     (void *)view);
 

         g_signal_connect (iconView, "button-release-event",
             G_CALLBACK(BaseSignals<Type>::buttonRelease), 
             (void *)view);
         g_signal_connect (iconView, "button-press-event",
             G_CALLBACK(BaseSignals<Type>::buttonPress), 
             (void *)view);
         // Why not "clicked" signal? 
         // Because this is to filter cancelled dnd event from
         // actual activated events.
         g_signal_connect (iconView, "button-release-event",
             G_CALLBACK(BaseSignals<Type>::buttonClick), 
             (void *)view);
      

    }

    ////////////////////   signal handlers iconview specific  ///////////////////
/////////////////////////////////  DnD   ///////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////

    

    
};
}
#endif
