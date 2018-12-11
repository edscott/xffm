#ifndef XF_VPANE
#define XF_VPANE

namespace xf {

template <class Type>
class Vpane{
    GtkPaned *vpane_;
    GtkTextView *output_;
    GtkScrolledWindow *topScrolledWindow_;
    GtkScrolledWindow *treeScrolledWindow_;
    GtkScrolledWindow *bottomScrolledWindow_;
public:
    GtkPaned *vpane(void){return vpane_;}
    GtkTextView *output(void){return output_;}
    GtkScrolledWindow *treeScrolledWindow(void){return treeScrolledWindow_;}
    GtkScrolledWindow *topScrolledWindow(void){return topScrolledWindow_;}
    GtkScrolledWindow *bottomScrolledWindow(void){return bottomScrolledWindow_;}
public:
    Vpane(void){
	vpane_ = GTK_PANED(gtk_paned_new(GTK_ORIENTATION_VERTICAL));
	//gtk_paned_set_wide_handle (vpane_, TRUE);
	gtk_paned_set_wide_handle (vpane_, FALSE);
	topScrolledWindow_ = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new (NULL, NULL));
	treeScrolledWindow_ = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new (NULL, NULL));
	
        bottomScrolledWindow_ = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new (NULL, NULL));
	output_ = newTextView();

	 g_object_set_data(G_OBJECT(vpane_), "diagnostics", output_);
	 g_object_set_data(G_OBJECT(output_), "vpane", vpane_);

        auto vbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0)); 
        gtk_box_pack_start (vbox, GTK_WIDGET(topScrolledWindow_), TRUE, TRUE, 0);
        gtk_box_pack_start (vbox, GTK_WIDGET(treeScrolledWindow_), TRUE, TRUE, 0);
        gtk_paned_pack1 (vpane_, GTK_WIDGET(vbox), FALSE, TRUE);
       
	//gtk_paned_pack1 (vpane_, GTK_WIDGET(topScrolledWindow_), FALSE, TRUE);
	
        gtk_paned_pack2 (vpane_, GTK_WIDGET(bottomScrolledWindow_), TRUE, TRUE);
	g_object_set(G_OBJECT(vpane_), "position-set", TRUE, NULL);
        gtk_container_add (GTK_CONTAINER(bottomScrolledWindow_), GTK_WIDGET(output_));
        
        //gtk_widget_show_all(GTK_WIDGET(vpane_));
        return ;
    }


    

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
