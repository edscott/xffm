#ifndef XF_VPANE
#define XF_VPANE

namespace xf {

template <class Type>
class Vpane{
public:
    static GtkPaned *newVpane(void){
	auto vpane = GTK_PANED(gtk_paned_new(GTK_ORIENTATION_VERTICAL));
	gtk_paned_set_wide_handle (vpane, TRUE);
	auto top_scrolled_window = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new (NULL, NULL));
	 g_object_set_data(G_OBJECT(vpane), "top_scrolled_window", top_scrolled_window);	 
	auto bottom_scrolled_window = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new (NULL, NULL));
	 g_object_set_data(G_OBJECT(vpane), "bottom_scrolled_window", bottom_scrolled_window);
	auto diagnostics = newTextView();
	 g_object_set_data(G_OBJECT(vpane), "diagnostics", diagnostics);
	 g_object_set_data(G_OBJECT(diagnostics), "vpane", vpane);

	gtk_paned_pack1 (vpane, GTK_WIDGET(top_scrolled_window), FALSE, TRUE);
	gtk_paned_pack2 (vpane, GTK_WIDGET(bottom_scrolled_window), TRUE, TRUE);
	g_object_set(G_OBJECT(vpane), "position-set", TRUE, NULL);
        gtk_container_add (GTK_CONTAINER(bottom_scrolled_window), GTK_WIDGET(diagnostics));
        
        gtk_widget_show_all(GTK_WIDGET(vpane));
        return vpane;
    }
    

private:
    static GtkTextView *newTextView(void){
	auto diagnostics = GTK_TEXT_VIEW(gtk_text_view_new ());
	gtk_text_view_set_monospace (diagnostics, TRUE);
	gtk_widget_set_can_focus(GTK_WIDGET(diagnostics), FALSE);
	gtk_text_view_set_wrap_mode (diagnostics, GTK_WRAP_WORD);
	gtk_text_view_set_cursor_visible (diagnostics, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (diagnostics), 2);
	return diagnostics;
    }


};
}




#endif
