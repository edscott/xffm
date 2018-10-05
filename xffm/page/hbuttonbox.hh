#ifndef XF_HBUTTONBOX
#define XF_HBUTTONBOX

namespace xf {
template <class Type>
class HButtonBox {
    using gtk_c = Gtk<double>;
public:
    HButtonBox(void){
	// The box
	hButtonBox_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
	termButtonBox_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
	
        toggleToTerminal_ =  GTK_BUTTON(gtk_button_new());
        gtk_c::setup_image_button(toggleToTerminal_, "utilities-terminal-symbolic", _("Terminal"));
        gtk_widget_set_tooltip_markup (GTK_WIDGET(toggleToTerminal_),_("Embedded Terminal"));
	toggleToIconview_ = GTK_BUTTON(gtk_button_new());
        gtk_c::setup_image_button(toggleToIconview_, "system-file-manager-symbolic", _("Iconview"));
        gtk_widget_set_tooltip_markup (GTK_WIDGET(toggleToIconview_),_("All items in the iconview."));


	input_ = createStatus(); // Status textview
	statusBox_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));

	statusButton_ = GTK_BUTTON(gtk_button_new());
	statusLabel_ = createStatusLabel();

	scriptButton_ =  gtk_c::dialog_button("document-revert-symbolic", NULL);
	gtk_widget_set_can_focus (GTK_WIDGET(scriptButton_), FALSE);
	gtk_button_set_relief (scriptButton_, GTK_RELIEF_NONE);
        gtk_widget_set_tooltip_markup (GTK_WIDGET(scriptButton_),_("Script Recorder"));

	clearButton_ =  gtk_c::dialog_button("edit-delete-symbolic", NULL);
	gtk_widget_set_can_focus (GTK_WIDGET(clearButton_), FALSE);
	gtk_button_set_relief (clearButton_, GTK_RELIEF_NONE);
        gtk_widget_set_tooltip_markup (GTK_WIDGET(clearButton_),_("Clear log view"));


	sizeScale_ = newSizeScale();
        gtk_widget_set_tooltip_markup (GTK_WIDGET(sizeScale_),_("Terminal font"));

	gtk_box_pack_end (hButtonBox_, GTK_WIDGET(termButtonBox_), FALSE, FALSE, 0);

	gtk_box_pack_end (termButtonBox_, GTK_WIDGET(clearButton_), FALSE, FALSE, 0);
	gtk_box_pack_end (termButtonBox_, GTK_WIDGET(sizeScale_), FALSE, FALSE, 0);
	gtk_box_pack_end (termButtonBox_, GTK_WIDGET(scriptButton_), FALSE, FALSE, 0);
	
        gtk_box_pack_start (hButtonBox_, GTK_WIDGET(toggleToTerminal_), FALSE, FALSE, 5);
        gtk_box_pack_start (hButtonBox_, GTK_WIDGET(toggleToIconview_), FALSE, FALSE, 5);
        gtk_box_pack_start (hButtonBox_, GTK_WIDGET(input_), TRUE, TRUE, 0);
        gtk_box_pack_start (hButtonBox_, GTK_WIDGET(statusButton_), TRUE, TRUE, 0);
	gtk_widget_show_all(GTK_WIDGET(hButtonBox_));

        gtk_container_add (GTK_CONTAINER (statusButton_), GTK_WIDGET(statusBox_));
        gtk_box_pack_start (statusBox_, GTK_WIDGET(statusLabel_), FALSE, FALSE, 0);
	gtk_widget_show_all(GTK_WIDGET(statusBox_));
        return;  
    }


protected:
    GtkBox *hButtonBox_;
    GtkBox *termButtonBox_;
    GtkBox *statusBox_;
    GtkTextView *input_;
    GtkButton *toggleToIconview_;
    GtkButton *toggleToTerminal_;
    GtkButton *statusButton_;
    GtkLabel *statusLabel_;
    GtkButton *clearButton_;
    GtkButton *scriptButton_;
    GtkScale *sizeScale_;

private:
    static GtkScale *newSizeScale(void){
	auto size_scale = GTK_SCALE(gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 6.0, 24.0, 6.0));
        gtk_range_set_value(GTK_RANGE(size_scale), DEFAULT_FIXED_FONT_SIZE);
        gtk_range_set_increments (GTK_RANGE(size_scale), 2.0, 6.0);
	gtk_widget_set_size_request (GTK_WIDGET(size_scale),75,-1);
	gtk_scale_set_value_pos (size_scale,GTK_POS_RIGHT);
        gtk_adjustment_set_upper (gtk_range_get_adjustment(GTK_RANGE(size_scale)), 24.0);
        return size_scale;
    }
    static GtkTextView *createStatus(void){
	GtkTextView *input = GTK_TEXT_VIEW(gtk_text_view_new ());
	gtk_text_view_set_pixels_above_lines (input, 10);
	gtk_text_view_set_monospace (input, TRUE);
	gtk_text_view_set_editable (input, TRUE);
	gtk_text_view_set_cursor_visible (input, TRUE);
	gtk_text_view_place_cursor_onscreen(input);
	gtk_text_view_set_wrap_mode (input, GTK_WRAP_CHAR);
	gtk_widget_set_can_focus(GTK_WIDGET(input), TRUE);
	gtk_widget_show_all(GTK_WIDGET(input));
	return input;
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
