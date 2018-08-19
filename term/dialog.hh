#ifndef XF_TERM_DIALOG
#define XF_TERM_DIALOG


#include "common/types.h"
#include "common/icons.hh"
#include "common/pixbuf.hh"
#include "common/gtk.hh"
#include "common/tooltip.hh"
#include "common/util.hh"
#include "common/dialog.hh"

namespace xf 
{
    
template <class Type>
class termDialog : public Dialog<Type> {
    using util_c = Util<double>;
    using gtk_c = Gtk<double>;
    using pixbuf_c = Pixbuf<double>;
    using pixbuf_icons_c = Icons<double>;
    using tooltip_c = Tooltip<double>;
 //   using dialog_c = xfDialog<double>;
 //
private:

 

    ///
public:
    termDialog(const gchar *title, const gchar *icon):Dialog<Type>(title, icon){
	fprintf(stderr, "%s %s\n", title, icon);
	gtk_widget_set_size_request (GTK_WIDGET(this->dialog()),400,200);
	gtk_window_set_default_size (this->dialog(),600 ,400);

	//this->setSize(600,400);
    }
    void createDialog(const gchar *path){
        gchar *default_path=NULL;
        if (path) default_path = g_strdup(path);
DBG("1\n");
	GtkWindow *dialog = this->dialog();
	//GtkWindow *dialog = this->mkDialog("Term","utilities-terminal" );
DBG("12\n");

	// create view page box
	auto page_child = createPageBox();
DBG("13\n");
	//   create pathbar
	//FIXME
	//   page vpane
	auto vpane = createVPaned(GTK_WIDGET(page_child));
DBG("14\n");
        //     vpane whatever
        /*gtk_container_add (
		GTK_CONTAINER (
		    g_object_get_data(G_OBJECT(vpane),
			"top_scrolled_window")), 
		GTK_WIDGET(diagnostics));*/
	//     vpane diagnostics
	auto diagnostics = createDiagnostics();
DBG("15\n");
        gtk_container_add (
		GTK_CONTAINER (
		    g_object_get_data(G_OBJECT(vpane),
			"bottom_scrolled_window")), 
		GTK_WIDGET(diagnostics));
	// Pack vpane
DBG("151\n");
	GtkBox *hview_box = GTK_BOX(
	    g_object_get_data(G_OBJECT(page_child),"hview_box"));
	gtk_box_pack_start (hview_box, GTK_WIDGET(vpane), TRUE, TRUE, 0);
DBG("152\n");
	gtk_paned_set_position (vpane, 1000);
DBG("153\n");
	// big button box
	auto big_button_space = createBigButtonSpace();
DBG("16\n");
	gtk_box_pack_start (hview_box, GTK_WIDGET(big_button_space), FALSE, FALSE, 0);
DBG("161\n");
	 g_object_set_data(G_OBJECT(hview_box), "big_button_space", big_button_space);
	//   page status
DBG("162\n");
	auto buttonSpace = createButtonSpace(); 
DBG("17\n");
	gtk_box_pack_start (page_child, GTK_WIDGET(buttonSpace), FALSE, FALSE, 0);
DBG("171\n");
	gtk_container_add(GTK_CONTAINER(dialog) , GTK_WIDGET(page_child));
	gtk_widget_show_all(GTK_WIDGET(dialog));
DBG("172\n");
	
    }
    GtkBox *createPageBox(void){
	auto page_child = GTK_BOX(
		gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));

	auto page_label_box = GTK_BOX(
		gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
	 g_object_set_data(G_OBJECT(page_child), 
		 "page_label_box", page_label_box);
	
	auto page_label_spinner_box = GTK_BOX(
		gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
	 g_object_set_data(G_OBJECT(page_child),
		 "page_label_spinner_box", page_label_spinner_box);
	gtk_box_pack_start (page_label_box, 
		GTK_WIDGET(page_label_spinner_box), TRUE, TRUE, 0);

	auto page_spinner = GTK_SPINNER(gtk_spinner_new());
	 g_object_set_data(G_OBJECT(page_child), 
		 "page_spinner", page_spinner);
	gtk_box_pack_start (page_label_spinner_box, 
		GTK_WIDGET(page_spinner), TRUE, TRUE, 0);

	auto page_label_icon_box = GTK_BOX(
		gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
	 g_object_set_data(G_OBJECT(page_child), 
		 "page_label_icon_box", page_label_icon_box);
	gtk_box_pack_start (page_label_box,
		GTK_WIDGET(page_label_icon_box), TRUE, TRUE, 0);

	auto page_label = GTK_LABEL(gtk_label_new (_("Loading folder...")));
	 g_object_set_data(G_OBJECT(page_child), 
		 "page_label", page_label);
	gtk_box_pack_start (page_label_box,
		GTK_WIDGET(page_label), TRUE, TRUE, 2);

	auto page_label_button = GTK_BUTTON(gtk_button_new ());
	 g_object_set_data(G_OBJECT(page_child), "page_label_button", page_label_button);
	gtk_box_pack_end (page_label_box, 
		GTK_WIDGET(page_label_button), TRUE, TRUE, 0);
    
	set_spinner(page_spinner, TRUE, page_label_icon_box);

	// pathbar is already created with pathbar_c object.
	// FIXME: add pathbar
	// gtk_box_pack_start (page_child, pathbar_p->get_pathbar(), FALSE, FALSE, 0);
	
	auto hview_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
	 g_object_set_data(G_OBJECT(page_child), "hview_box", hview_box);
	gtk_box_pack_start (page_child, GTK_WIDGET(hview_box), TRUE, TRUE, 0);
	gtk_widget_show_all(GTK_WIDGET(page_child));

	return page_child;
    }
    GtkPaned *createVPaned(GtkWidget *parent){
	auto vpane = GTK_PANED(gtk_paned_new(GTK_ORIENTATION_VERTICAL));
	 g_object_set_data(G_OBJECT(parent), "vpane", vpane); 
	gtk_paned_set_wide_handle (vpane, TRUE);
	auto top_scrolled_window = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new (NULL, NULL));
	 g_object_set_data(G_OBJECT(vpane), "top_scrolled_window", top_scrolled_window);
	 
	auto bottom_scrolled_window = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new (NULL, NULL));
	 g_object_set_data(G_OBJECT(vpane), "bottom_scrolled_window", bottom_scrolled_window);
	gtk_paned_pack1 (vpane, GTK_WIDGET(top_scrolled_window), FALSE, TRUE);
	gtk_paned_pack2 (vpane, GTK_WIDGET(bottom_scrolled_window), TRUE, TRUE);
	gtk_widget_show_all(GTK_WIDGET(vpane));
	return vpane;

    }
    GtkTextView *createDiagnostics(void){
	auto diagnostics = GTK_TEXT_VIEW(gtk_text_view_new ());
	gtk_text_view_set_monospace (diagnostics, TRUE);
	gtk_widget_set_can_focus(GTK_WIDGET(diagnostics), FALSE);
	gtk_text_view_set_wrap_mode (diagnostics, GTK_WRAP_WORD);
	gtk_text_view_set_cursor_visible (diagnostics, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (diagnostics), 2);
	return diagnostics;
    }
    GtkBox *createButtonSpace(void){
	// The box
	auto button_space = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
	// Term input (status) icon.
	auto status_icon = GTK_IMAGE(gtk_image_new_from_icon_name ("utilities-terminal", GTK_ICON_SIZE_SMALL_TOOLBAR)); 
	 g_object_set_data(G_OBJECT(button_space), "status_icon", status_icon);
	gtk_box_pack_start (button_space, GTK_WIDGET(status_icon), FALSE, FALSE, 5);
	// Iconview icon
	auto iconview_icon = GTK_IMAGE(gtk_image_new_from_icon_name ("system-file-manager", GTK_ICON_SIZE_SMALL_TOOLBAR)); 
	 g_object_set_data(G_OBJECT(button_space), "iconview_icon", iconview_icon);
	gtk_box_pack_start (button_space, GTK_WIDGET(iconview_icon), FALSE, FALSE, 5);
	// Status textview
	auto status = createStatus();
	 g_object_set_data(G_OBJECT(button_space), "status", status);
	gtk_box_pack_start (button_space, GTK_WIDGET(status), TRUE, TRUE, 0);
	gtk_widget_show_all(GTK_WIDGET(button_space));
	return button_space;    
    }
    GtkTextView *createStatus(void){
	auto status = GTK_TEXT_VIEW(gtk_text_view_new ());
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

    GtkBox *createBigButtonSpace(void){
	auto big_button_space = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
	auto clear_button =  GTK_BUTTON(gtk_button_new ());
	 g_object_set_data(G_OBJECT(big_button_space), "clear_button", clear_button);
	gtk_box_pack_end (big_button_space, GTK_WIDGET(clear_button), FALSE, FALSE, 0);
	auto hidden_button =  GTK_TOGGLE_BUTTON(gtk_toggle_button_new ());
	 g_object_set_data(G_OBJECT(big_button_space), "hidden_button", hidden_button);
	gtk_box_pack_end (big_button_space, GTK_WIDGET(hidden_button), FALSE, FALSE, 0);
	auto size_scale = GTK_SCALE(gtk_scale_new_with_range(GTK_ORIENTATION_VERTICAL, 0.0, 96.0, 12.0));
	 g_object_set_data(G_OBJECT(big_button_space), "size_scale", size_scale);
	
	gtk_box_pack_end (big_button_space, GTK_WIDGET(size_scale), FALSE, FALSE, 0);
	gtk_widget_show_all(GTK_WIDGET(big_button_space));
	return big_button_space;
   }
    
    GtkBox *createStatusBox(const gchar *text){
	auto status_box = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));

	auto status_button = GTK_BUTTON(gtk_button_new());
	gtk_container_add (GTK_CONTAINER (status_button), GTK_WIDGET(status_box));
	auto status_label = GTK_LABEL(gtk_label_new (""));
	 g_object_set_data(G_OBJECT(status_box), "status_label", status_label);
	gtk_box_pack_start (status_box, GTK_WIDGET(status_label), FALSE, FALSE, 0);
        if (text)
	    gtk_label_set_markup(status_label,text); 
	gtk_widget_show_all(GTK_WIDGET(status_box));
	return status_box;
    }

    void set_spinner(GtkSpinner *page_spinner, gboolean state,
	    GtkBox *page_label_icon_box)
    {
	if (!state){
	    gtk_spinner_stop (page_spinner);
	    gtk_widget_show(GTK_WIDGET(page_label_icon_box));
	    return;
	}
	gtk_widget_hide(GTK_WIDGET(page_label_icon_box));  
	gtk_spinner_start (page_spinner);
    }

private:


};
}



#endif
