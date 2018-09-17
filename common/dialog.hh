#ifndef XF_DIALOG
#define XF_DIALOG
#include "common/types.h"
#include "notebook.hh"
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
        DBG("window_keyboard_event\n");
        auto dialog_p = (Dialog<Type> *)data;
        auto page_p = dialog_p->currentPageObject();
        //auto notebook = dialog_p->notebook();
        //auto input = page_p->input();
        //auto output = page_p->output();
        // do the completion thing
        page_p->keyboard_event(event);
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
	    resizePane(vpane);

	}
	// do this for all notebook pages, visible or not
	//auto page_p = dialog_p->currentPageObject();
	//auto vpane = page_p->vpane();
	//resizePane(vpane);
    }
private:

};



template <class Type>
class Dialog :public Notebook<Type> {
    using pixbuf_c = Pixbuf<double>;
public:
    /*Dialog(void){
	dialog_ = GTK_WINDOW(gtk_window_new (GTK_WINDOW_TOPLEVEL));
	gtk_window_set_type_hint(dialog_, GDK_WINDOW_TYPE_HINT_DIALOG);
	setWindowMaxSize(dialog_);
	gtk_window_set_position (dialog_, GTK_WIN_POS_MOUSE);
	return;
    }*/
    Dialog(const gchar *title, const gchar *icon){

        Dialog();
	setDialogIcon(icon);
	setDialogTitle(title);
    }

    Dialog(void){
	dialog_ = GTK_WINDOW(gtk_window_new (GTK_WINDOW_TOPLEVEL));
        g_signal_connect (G_OBJECT (dialog_), "delete-event", EVENT_CALLBACK (dialogSignals<Type>::delete_event), NULL);


                

	gtk_window_set_default_size (dialog_,600 ,400);
        gtk_widget_set_has_tooltip (GTK_WIDGET(dialog_), TRUE);
        // FIXME:
        //g_signal_connect (G_OBJECT (dialog_), "query-tooltip", G_CALLBACK (window_tooltip_f), (void *)this);
        g_signal_connect (G_OBJECT (dialog_), "key-press-event", EVENT_CALLBACK (dialogSignals<Type>::window_keyboard_event), (void *)this);

	gtk_widget_get_preferred_width (GTK_WIDGET(dialog_), &dialogMinW_, &dialogNatW_);
	gtk_widget_get_preferred_height (GTK_WIDGET(dialog_), &dialogMinH_, &dialogNatH_);
	gtk_window_set_type_hint(dialog_, GDK_WINDOW_TYPE_HINT_DIALOG);
	setWindowMaxSize(dialog_);
	gtk_window_set_position (dialog_, GTK_WIN_POS_MOUSE);
        this->insertNotebook(dialog_);
        this->addPage("startup");
        DBG("dialog this=%p\n", (void *)this);
        //this->insertPageChild(this->notebook());
        
        
        //gtk_widget_show_all(GTK_WIDGET(pageChild));
	gtk_widget_realize(GTK_WIDGET(dialog_));
	gint max, current;
        auto vpane = this->vpane();
	g_object_get(G_OBJECT(vpane), "max-position", &max, NULL);
	g_object_get(G_OBJECT(vpane), "position", &current, NULL);
	g_object_set_data(G_OBJECT(vpane), "oldCurrent", GINT_TO_POINTER(current));
	g_object_set_data(G_OBJECT(vpane), "oldMax", GINT_TO_POINTER(max));
        
	g_signal_connect (G_OBJECT (dialog_), "size-allocate", 
		SIZE_CALLBACK(dialogSignals<Type>::onSizeAllocate), (void *)this);
        
        gtk_window_present (dialog_);
        while (gtk_events_pending()) gtk_main_iteration();
	return;
    }

    void setDialogTitle(const gchar *title){
	gtk_window_set_title (dialog_, title);
    }
    void setDialogIcon(const gchar *icon){
	auto pixbuf = pixbuf_c::get_pixbuf(icon, SIZE_ICON);
	gtk_window_set_icon (dialog_, pixbuf);
	g_object_unref(pixbuf);
    }

protected:
    GtkWindow *dialog(){
	return dialog_;
    }
    void setDialogSize(gint w, gint h){
	gint minW, natW;
	gint minH, natH;
	DBG("minimum_width=%d natural_width=%d\n", minW,natW);
	DBG("minimum_height=%d natural_height=%d\n", minH,natH);

	gtk_widget_set_size_request (GTK_WIDGET(dialog_), w, h);
    }
	

private:
    void setWindowMaxSize(GtkWindow *dialog){
	gint x_return, y_return;
	guint w_return, h_return, d_return, border_return;
	Window root_return;
	auto drawable = gdk_x11_get_default_root_xwindow ();
	//Visual Xvisual = gdk_x11_visual_get_xvisual(gdk_visual_get_system());
	auto display = gdk_x11_display_get_xdisplay(gdk_display_get_default());
	XGetGeometry(display, drawable, &root_return,
		&x_return, &y_return, 
		&w_return, &h_return, 
		&border_return, 
		&d_return);
	GdkGeometry geometry;
	geometry.max_width = w_return - 25;
	geometry.max_height = h_return -25;
	gtk_window_set_geometry_hints (dialog, GTK_WIDGET(dialog), &geometry, GDK_HINT_MAX_SIZE);
    }

private:
    GtkWindow *dialog_;
    gint dialogMinW_, dialogNatW_, dialogMinH_, dialogNatH_;
};
}

#endif
