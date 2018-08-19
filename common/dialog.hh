#ifndef XF_DIALOG
#define XF_DIALOG
namespace xf {
template <class Type>
class Dialog {
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
	dialog_ = GTK_WINDOW(gtk_window_new (GTK_WINDOW_TOPLEVEL));

	gtk_widget_get_preferred_width (GTK_WIDGET(dialog_), &dialogMinW_, &dialogNatW_);
	gtk_widget_get_preferred_height (GTK_WIDGET(dialog_), &dialogMinH_, &dialogNatH_);
	gtk_window_set_type_hint(dialog_, GDK_WINDOW_TYPE_HINT_DIALOG);
	setWindowMaxSize(dialog_);
	gtk_window_set_position (dialog_, GTK_WIN_POS_MOUSE);
	setDialogIcon(icon);
	setDialogTitle(title);
	return;
    }

protected:
    GtkWindow *dialog(){
	return dialog_;
    }
    /*void setSize(gint w, gint h){
	gint minW, natW;
	gint minH, natH;
	DBG("minimum_width=%d natural_width=%d\n", minW,natW);
	DBG("minimum_height=%d natural_height=%d\n", minH,natH);

	gtk_widget_set_size_request (GTK_WIDGET(dialog_), w, h);
    }*/
	
    void setDialogTitle(const gchar *title){
	gtk_window_set_title (dialog_, title);
    }
    void setDialogIcon(const gchar *icon){
	GdkPixbuf *pixbuf = pixbuf_c::get_pixbuf(icon, SIZE_ICON);
	gtk_window_set_icon (dialog_, pixbuf);
	g_object_unref(pixbuf);
    }

private:
    void setWindowMaxSize(GtkWindow *dialog){
	gint x_return, y_return;
	guint w_return, h_return, d_return, border_return;
	Window root_return;
	Drawable drawable = gdk_x11_get_default_root_xwindow ();
	//Visual Xvisual = gdk_x11_visual_get_xvisual(gdk_visual_get_system());
	Display *display = gdk_x11_display_get_xdisplay(gdk_display_get_default());
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
