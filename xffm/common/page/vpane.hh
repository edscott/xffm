#ifndef XF_VPANE
#define XF_VPANE

namespace xf {

template <class Type>
class Vpane{
public:
    Vpane(void){
	vpane_ = GTK_PANED(gtk_paned_new(GTK_ORIENTATION_VERTICAL));
	//gtk_paned_set_wide_handle (vpane_, TRUE);
	gtk_paned_set_wide_handle (vpane_, FALSE);
	top_scrolled_window_ = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new (NULL, NULL));
	bottom_scrolled_window_ = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new (NULL, NULL));
	output_ = newTextView();

	 g_object_set_data(G_OBJECT(vpane_), "diagnostics", output_);
	 g_object_set_data(G_OBJECT(output_), "vpane", vpane_);

	gtk_paned_pack1 (vpane_, GTK_WIDGET(top_scrolled_window_), FALSE, TRUE);
	gtk_paned_pack2 (vpane_, GTK_WIDGET(bottom_scrolled_window_), TRUE, TRUE);
	g_object_set(G_OBJECT(vpane_), "position-set", TRUE, NULL);
        gtk_container_add (GTK_CONTAINER(bottom_scrolled_window_), GTK_WIDGET(output_));
        
        //gtk_widget_show_all(GTK_WIDGET(vpane_));
        return ;
    }

protected:
    GtkPaned *vpane_;
    GtkTextView *output_;
    GtkScrolledWindow *top_scrolled_window_;
    GtkScrolledWindow *bottom_scrolled_window_;
    

private:
    GtkTextView *newTextView(void){
	auto output = GTK_TEXT_VIEW(gtk_text_view_new ());
	gtk_text_view_set_monospace (output, TRUE);
	gtk_widget_set_can_focus(GTK_WIDGET(output), FALSE);
	gtk_text_view_set_wrap_mode (output, GTK_WRAP_WORD);
	gtk_text_view_set_cursor_visible (output, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (output), 2);
	return output;
    }


};
}




#endif
