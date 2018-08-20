#ifndef XF_VBUTTONBOX
#define XF_VBUTTONBOX

namespace xf {

template <class Type>
class VButtonBox {
    using gtk_c = Gtk<double>;
    
public:
    VButtonBox(void){
	vButtonBox_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
	auto clear_button =  gtk_c::dialog_button("edit-clear", NULL);
	 g_object_set_data(G_OBJECT(vButtonBox_), "clear_button", clear_button);
	gtk_box_pack_end (vButtonBox_, GTK_WIDGET(clear_button), FALSE, FALSE, 0);
        // FIXME: hidden_button icon is custom and button should be at tab level.
	/*auto hidden_button =  gtk_c::toggle_button("semi-starred", NULL);
	 g_object_set_data(G_OBJECT(vButtonBox_), "hidden_button", hidden_button);
	gtk_box_pack_end (vButtonBox_, GTK_WIDGET(hidden_button), FALSE, FALSE, 0);*/
	auto size_scale = GTK_SCALE(gtk_scale_new_with_range(GTK_ORIENTATION_VERTICAL, 6.0, 24.0, 6.0));
        gtk_range_set_value(GTK_RANGE(size_scale), 12);
        gtk_range_set_increments (GTK_RANGE(size_scale), 2.0, 6.0);
	 g_object_set_data(G_OBJECT(vButtonBox_), "size_scale", size_scale);
	gtk_widget_set_size_request (GTK_WIDGET(size_scale),-1,75);
	
	gtk_box_pack_end (vButtonBox_, GTK_WIDGET(size_scale), FALSE, FALSE, 0);
	gtk_widget_show_all(GTK_WIDGET(vButtonBox_));
	return;
    }
    
    GtkBox *vButtonBox(void){ return vButtonBox_;}
    void insertVButtonBox(GtkBox *hViewBox){
	gtk_box_pack_start (hViewBox, GTK_WIDGET(vButtonBox_), FALSE, FALSE, 0);
    }
   
private:
    GtkBox *vButtonBox_;

};



}



#endif
