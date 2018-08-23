#ifndef XF_HBUTTONBOX
#define XF_HBUTTONBOX

namespace xf {

template <class Type>
class HButtonBox {
public:
    HButtonBox(void){
	// The box
	hButtonBox_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
	// Term input (status) icon.
	auto status_icon = GTK_IMAGE(gtk_image_new_from_icon_name ("utilities-terminal", GTK_ICON_SIZE_SMALL_TOOLBAR)); 
	 g_object_set_data(G_OBJECT(hButtonBox_), "status_icon", status_icon);
	// Iconview icon
	auto iconview_icon = GTK_IMAGE(gtk_image_new_from_icon_name ("system-file-manager", GTK_ICON_SIZE_SMALL_TOOLBAR)); 
	 g_object_set_data(G_OBJECT(hButtonBox_), "iconview_icon", iconview_icon);
	// Status textview
	 createStatus();

	auto statusBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));

	auto status_button = GTK_BUTTON(gtk_button_new());
	auto status_label = GTK_LABEL(gtk_label_new (""));
        gchar *g = g_strdup_printf("<span color=\"blue\"><b>%s-%s</b></span>",
                "xffm+", VERSION);
        gtk_label_set_markup(status_label,g);
        g_free(g);
	 g_object_set_data(G_OBJECT(statusBox), "status_label", status_label);
	
        gtk_box_pack_start (hButtonBox_, GTK_WIDGET(status_icon), FALSE, FALSE, 5);
        gtk_box_pack_start (hButtonBox_, GTK_WIDGET(iconview_icon), FALSE, FALSE, 5);
        gtk_box_pack_start (hButtonBox_, GTK_WIDGET(status_), TRUE, TRUE, 0);
        gtk_box_pack_start (hButtonBox_, GTK_WIDGET(status_button), TRUE, TRUE, 0);
        gtk_container_add (GTK_CONTAINER (status_button), GTK_WIDGET(statusBox));
        gtk_box_pack_start (statusBox, GTK_WIDGET(status_label), FALSE, FALSE, 0);
	gtk_widget_show_all(GTK_WIDGET(statusBox));
	gtk_widget_show_all(GTK_WIDGET(hButtonBox_));
        return;  
    }
    


    GtkBox *hButtonBox(void){return hButtonBox_;}
    GtkTextView *status(void){ return status_;}
private:
    GtkBox *hButtonBox_;
    GtkTextView *status_;
    void createStatus(void){
	status_ = GTK_TEXT_VIEW(gtk_text_view_new ());
	gtk_text_view_set_pixels_above_lines (status_, 10);
	gtk_text_view_set_monospace (status_, TRUE);
	gtk_text_view_set_editable (status_, TRUE);
	gtk_text_view_set_cursor_visible (status_, TRUE);
	gtk_text_view_place_cursor_onscreen(status_);
	gtk_text_view_set_wrap_mode (status_, GTK_WRAP_CHAR);
	gtk_widget_set_can_focus(GTK_WIDGET(status_), TRUE);
	gtk_widget_show_all(GTK_WIDGET(status_));
	return;
    }

 };
}
#endif
