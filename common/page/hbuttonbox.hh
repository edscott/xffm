#ifndef XF_HBUTTONBOX
#define XF_HBUTTONBOX

namespace xf {

template <class Type>
class HButtonBox {
public:
    static GtkBox *newBox(void){
	// The box
	auto hButtonBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
	
	auto status_icon = // Term input (status) icon. 
            GTK_IMAGE(gtk_image_new_from_icon_name ("utilities-terminal", GTK_ICON_SIZE_SMALL_TOOLBAR)); 
	 g_object_set_data(G_OBJECT(hButtonBox), "status_icon", status_icon);
	auto iconview_icon = 
            GTK_IMAGE(gtk_image_new_from_icon_name ("system-file-manager", GTK_ICON_SIZE_SMALL_TOOLBAR)); 
	 g_object_set_data(G_OBJECT(hButtonBox), "iconview_icon", iconview_icon);
	auto status = 
            createStatus(); // Status textview
         g_object_set_data(G_OBJECT(hButtonBox), "status", status);
	auto statusBox = 
            GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
	 g_object_set_data(G_OBJECT(hButtonBox), "statusBox", statusBox);
	auto status_button = 
            GTK_BUTTON(gtk_button_new());
	 g_object_set_data(G_OBJECT(hButtonBox), "status_button", status_button);
	auto status_label = 
            createStatusLabel();
	 g_object_set_data(G_OBJECT(hButtonBox), "status_label", status_label);

	
        gtk_box_pack_start (hButtonBox, GTK_WIDGET(status_icon), FALSE, FALSE, 5);
        gtk_box_pack_start (hButtonBox, GTK_WIDGET(iconview_icon), FALSE, FALSE, 5);
        gtk_box_pack_start (hButtonBox, GTK_WIDGET(status), TRUE, TRUE, 0);
        gtk_box_pack_start (hButtonBox, GTK_WIDGET(status_button), TRUE, TRUE, 0);
	gtk_widget_show_all(GTK_WIDGET(hButtonBox));

        gtk_container_add (GTK_CONTAINER (status_button), GTK_WIDGET(statusBox));
        gtk_box_pack_start (statusBox, GTK_WIDGET(status_label), FALSE, FALSE, 0);
	gtk_widget_show_all(GTK_WIDGET(statusBox));
        return hButtonBox;  
    }
    


private:
    static GtkTextView *createStatus(void){
	GtkTextView *status = GTK_TEXT_VIEW(gtk_text_view_new ());
	gtk_text_view_set_pixels_above_lines (status, 10);
	gtk_text_view_set_monospace (status, TRUE);
	gtk_text_view_set_editable (status, TRUE);
	gtk_text_view_set_cursor_visible (status, TRUE);
	gtk_text_view_place_cursor_onscreen(status);
	gtk_text_view_set_wrap_mode (status, GTK_WRAP_CHAR);
	gtk_widget_set_can_focus(GTK_WIDGET(status), TRUE);
	gtk_widget_show_all(GTK_WIDGET(status));
	return status;
    }
    static GtkLabel *createStatusLabel(void){
        auto status_label =
            GTK_LABEL(gtk_label_new (""));
        gchar *g = g_strdup_printf("<span color=\"blue\"><b>%s-%s</b></span>",
                "xffm+", VERSION);
        gtk_label_set_markup(status_label,g);
        g_free(g);
        return status_label;
    }

 };
}
#endif
