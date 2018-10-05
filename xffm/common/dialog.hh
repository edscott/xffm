#ifndef XF_DIALOG
#define XF_DIALOG
GtkWidget *mainWindow;
#include "common/types.h"
#include "signals/dialog.hh"
#include "notebook.hh"
#include "print.hh"
#include <memory>

namespace xf {

template <class Type>
class Dialog :public Notebook<Type> {
    using pixbuf_c = Pixbuf<double>;
    using print_c = Print<double>;
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
    Dialog(const gchar *path){
	init(path);
    }
    Dialog(void){
	init(NULL);
    }

    void init(const gchar *path){
	dialog_ = GTK_WINDOW(gtk_window_new (GTK_WINDOW_TOPLEVEL));
        mainWindow = GTK_WIDGET(dialog_);
        g_signal_connect (G_OBJECT (dialog_), "delete-event", EVENT_CALLBACK (dialogSignals<Type>::delete_event), NULL);

        gtk_widget_set_has_tooltip (GTK_WIDGET(dialog_), TRUE);
        // FIXME:
        //g_signal_connect (G_OBJECT (dialog_), "query-tooltip", G_CALLBACK (window_tooltip_f), (void *)this);
        g_signal_connect (G_OBJECT (dialog_), "key-press-event", KEY_EVENT_CALLBACK (dialogSignals<Type>::window_keyboard_event), (void *)this);

	gtk_widget_get_preferred_width (GTK_WIDGET(dialog_), &dialogMinW_, &dialogNatW_);
	gtk_widget_get_preferred_height (GTK_WIDGET(dialog_), &dialogMinH_, &dialogNatH_);
	gtk_window_set_type_hint(dialog_, GDK_WINDOW_TYPE_HINT_DIALOG);
	setWindowMaxSize(dialog_);
	gtk_window_set_position (dialog_, GTK_WIN_POS_MOUSE);
        this->insertNotebook(dialog_);
#ifdef XFFM_CC
        auto dialog = (fmDialog<Type> *) this;
#else
        auto dialog = (Dialog<Type> *) this;
#endif       

        dialog->addPage(path); 
        TRACE("dialog this=%p\n", (void *)this);
        //this->insertPageChild(this->notebook());
        
        
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

    void resizeWindow(gint fontSize){
        gint Dw = 2*maximumSize_.width/3 - naturalSize_.width ;
        if (Dw < 0) Dw = 0;
        gint Dh = 4*maximumSize_.height/5 - naturalSize_.height;
        if (Dh < 0) Dh = 0;
        double fraction = (double)(fontSize - 6)/(24 - 6);
        gint w = fraction * Dw + naturalSize_.width;
        gint h = fraction * Dh + naturalSize_.height;
        WARN("resize window %d: %lf --> %d,%d\n",
               fontSize, fraction, w, h);
        gtk_window_resize (GTK_WINDOW(dialog_), w, h);
        
    } 
protected:
    GtkWindow *dialog(){
	return dialog_;
    }

    void setDefaultSize(gint w, gint h){
        chosenSize_.width = w;
        chosenSize_.height = h;
        gtk_widget_get_preferred_size (GTK_WIDGET(dialog_),
                               &minimumSize_,
                               &naturalSize_);
        WARN("Size: minimum=%d,%d, natural=%d,%d, choosen=%d,%d max=%d,%d\n",
                minimumSize_.width, minimumSize_.height,
                naturalSize_.width, naturalSize_.height,
                w,h,
                maximumSize_.width, maximumSize_.height);

	//gtk_window_set_default_size (GTK_WINDOW(dialog_), w, h);
        //gtk_window_resize (GTK_WINDOW(dialog_), w, h);
    }
    void setDefaultFixedFontSize(gint size){
	DEFAULT_FIXED_FONT_SIZE = size;
        auto page = this->currentPageObject();
        page->setSizeScale(size);
        print_c::set_font_size(GTK_WIDGET(page->output()), size);
        print_c::set_font_size(GTK_WIDGET(page->input()), size);
        resizeWindow(size);
    }

private:
        GtkRequisition minimumSize_;
        GtkRequisition naturalSize_;
        GtkRequisition chosenSize_;
        GtkRequisition maximumSize_;
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
        maximumSize_.width = geometry.max_width;
        maximumSize_.height = geometry.max_height;
	gtk_window_set_geometry_hints (dialog, GTK_WIDGET(dialog), &geometry, GDK_HINT_MAX_SIZE);
    }

private:
    GtkWindow *dialog_;
    gint dialogMinW_, dialogNatW_, dialogMinH_, dialogNatH_;
};
}

#endif
