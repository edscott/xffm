#ifndef XF_DIALOG_SIGNALS
#define XF_DIALOG_SIGNALS
#include "fm/notebook.hh"

namespace xf {
template <class Type> class Dialog;
template <class Type> class dialogSignals{
public:
    static gboolean delete_event (GtkWidget *widget,
               GdkEvent  *event,
               gpointer   user_data){
	gtk_widget_hide(widget);
        while (gtk_events_pending()) gtk_main_iteration();
        gtk_main_quit();
        _exit(123);
	return TRUE;
    }

    static gboolean
    window_keyboard_event (GtkWidget *window, GdkEventKey * event, gpointer data)
    {

	TRACE("window_keyboard_event\n");
	/* asian Input methods */
	if(event->keyval == GDK_KEY_space && (event->state & (GDK_MOD1_MASK | GDK_SHIFT_MASK))) {
	    return FALSE;
	}
    /*
       Ctrl-Left               Word left
       Ctrl-Right              Word right
       Ctrl-Y                  Delete line
       Ctrl-K                  Delete to end of line
       Ctrl-BS                 Delete word left
       Ctrl-Del        	   Delete word right
       Ctrl-A                  Select all text
       Ctrl-U                  Deselect block
       Ctrl-V       	   Paste block from clipboard
       Ctrl-X                  Cut block
       Ctrl-C                  Copy block to clipboard
       */

	gint ignore[]={
	    GDK_KEY_Control_L,
	    GDK_KEY_Control_R,
	    GDK_KEY_Shift_L,
	    GDK_KEY_Shift_R,
	    GDK_KEY_Shift_Lock,
	    GDK_KEY_Caps_Lock,
	    GDK_KEY_Meta_L,
	    GDK_KEY_Meta_R,
	    GDK_KEY_Alt_L,
	    GDK_KEY_Alt_R,
	    GDK_KEY_Super_L,
	    GDK_KEY_Super_R,
	    GDK_KEY_Hyper_L,
	    GDK_KEY_Hyper_R,
	    GDK_KEY_ISO_Lock,
	    GDK_KEY_ISO_Level2_Latch,
	    GDK_KEY_ISO_Level3_Shift,
	    GDK_KEY_ISO_Level3_Latch,
	    GDK_KEY_ISO_Level3_Lock,
	    GDK_KEY_ISO_Level5_Shift,
	    GDK_KEY_ISO_Level5_Latch,
	    GDK_KEY_ISO_Level5_Lock,
	    0
	};

	gint i;
	for (i=0; ignore[i]; i++) {
	    if(event->keyval ==  ignore[i]) {
		TRACE("window_keyboard_event: key ignored\n");
		return TRUE;
	    }
	}


        TRACE("window_keyboard_event\n");
        auto dialog_p = (Dialog<Type> *)data;
        auto page_p = dialog_p->currentPageObject();

        page_p->keyboardEvent(event);
        return TRUE;
    }


    static void resizePane(GtkPaned *vpane){
	gint max, current;
	g_object_get(G_OBJECT(vpane), "max-position", &max, NULL);
	g_object_get(G_OBJECT(vpane), "position", &current, NULL);
	TRACE(">> max=%d, current=%d dialogw=%d dialogH+%d\n",
		max, current, allocation->width, allocation->height);
	if (!G_IS_OBJECT(vpane)) return;
	auto oldMax = 
	    GPOINTER_TO_INT(g_object_get_data(G_OBJECT(vpane), "oldMax"));
	if (oldMax == 0){
	    // vpane is not set up yet...
	    return;
	}
	if (!G_IS_OBJECT(vpane)) return;
	auto oldCurrent = 
	    GPOINTER_TO_INT(g_object_get_data(G_OBJECT(vpane), "oldCurrent"));
	    TRACE("//  oldCurrent= %d, oldMax=%d,  max=%d\n",
		    oldCurrent, oldMax, max);
	if (max != oldMax) {
	    // window size is changing
	    auto ratio = (gdouble)oldCurrent / oldMax;
	    gint newCurrent = floor(ratio * max);
	    TRACE("// window size is changing oldCurrent= %d, oldMax=%d, newcurrent=%d, max=%d\n",
		    oldCurrent, oldMax, newCurrent, max);
	    if (!G_IS_OBJECT(vpane)) return;
	    g_object_set_data(G_OBJECT(vpane), "oldCurrent", GINT_TO_POINTER(newCurrent));
	    if (!G_IS_OBJECT(vpane)) return;
	    g_object_set_data(G_OBJECT(vpane), "oldMax", GINT_TO_POINTER(max));
	    if (!G_IS_OBJECT(vpane)) return;
	    TRACE("resizePane(): new pane position=%d\n", newCurrent);
	    gtk_paned_set_position(vpane, newCurrent);

	} else if (current != oldCurrent) {
	    // pane is resizing
	    TRACE("// pane is resizing\n");
	    if (!G_IS_OBJECT(vpane)) return;
	    g_object_set_data(G_OBJECT(vpane), "oldCurrent", GINT_TO_POINTER(current));
	}
    }
    
    static void onSizeAllocate (GtkWidget    *widget,
		   GdkRectangle *allocation,
		   gpointer      data){
        static gint lastX=-1;
        static gint lastY=-1;
        if (allocation->width == lastX && allocation->height == lastY) return;
	TRACE("dialog.hh::onSizeAllocate():SIZE allocate\n");
        lastX = allocation->width;
        lastY = allocation->height;

        auto dialog_p = (Dialog<Type> *)data;
        // Save selection width and height to .ini
	Settings<Type>::setSettingInteger( "window", "width", lastX);
	Settings<Type>::setSettingInteger( "window", "height", lastY);

	gint pages = gtk_notebook_get_n_pages (dialog_p->notebook());
	TRACE("pages = %d\n", pages);
	for (int i=0; i<pages; i++){
	    TRACE("resize page %d\n", i);
	    GtkWidget *child = gtk_notebook_get_nth_page (dialog_p->notebook(), i);
	    auto vpane = dialog_p->vpane(child);
	    if (vpane) resizePane(vpane);

	}
	// do this for all notebook pages, visible or not
	//auto page_p = dialog_p->currentPageObject();
	//auto vpane = page_p->vpane();
	//resizePane(vpane);
    }
private:

};


}

#endif
