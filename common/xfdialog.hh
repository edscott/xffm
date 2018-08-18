#ifndef XF_TERM_DIALOG
#define XF_TERM_DIALOG

namespace xf {
template <class Type>
class Dialog :{
protected:
    void createDialog(const gchar *path){
        createDialog_(path);
    }

private:
    GtkWindow *createXFDialog_(const gchar *path){
        gchar *default_path=NULL;
        if (path) default_path = g_strdup(path);
	return mkDialog();
    }
    GtkWindow * mkDialog(void){
	GtkWindow *dialog = GTK_WINDOW(gtk_window_new (GTK_WINDOW_TOPLEVEL));
	gtk_window_set_type_hint(dialog, GDK_WINDOW_TYPE_HINT_DIALOG);
	setWindowMaxSize();
	//g_object_set_data(G_OBJECT(dialog_), "window", dialog_);
	gtk_window_set_title (dialog, _("Term"));
	gtk_window_set_position (dialog, GTK_WIN_POS_MOUSE);

	GdkPixbuf *pixbuf = pixbuf_c::get_pixbuf("edit-find", SIZE_ICON);
	gtk_window_set_icon (dialog, pixbuf);
	g_object_unref(pixbuf);
	return dialog;
    }

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
	geometry_.max_width = w_return - 25;
	geometry_.max_height = h_return -25;
	gtk_window_set_geometry_hints (dialog, GTK_WIDGET(dialog_), &geometry_, GDK_HINT_MAX_SIZE);
    }
};
}

#endif
