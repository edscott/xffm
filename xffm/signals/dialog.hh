#ifndef XF_DIALOG_SIGNALS
#define XF_DIALOG_SIGNALS
#include "common/types.h"
#include "common/notebook.hh"
#include <memory>

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
        auto dialog_p = (Dialog<Type> *)data;
        auto page_p = dialog_p->currentPageObject();
        //auto notebook = dialog_p->notebook();
        //auto input = page_p->input();
        //auto output = page_p->output();
        // do the completion thing
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
	TRACE("dialog.hh::onSizeAllocate():SIZE allocate\n");
        auto dialog_p = (Dialog<Type> *)data;

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
