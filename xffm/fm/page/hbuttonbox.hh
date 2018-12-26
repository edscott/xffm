#ifndef XF_HBUTTONBOX
#define XF_HBUTTONBOX


namespace xf {

template <class Type>
class HButtonBox {
    using gtk_c = Gtk<double>;

    GtkLabel *statusLabel_;
    GtkBox *hButtonBox_;
    GtkBox *fmButtonBox_;
    GtkBox *termButtonBox_;
    GtkTextView *input_;
    GtkButton *toggleToIconview_;
    GtkButton *toggleToTerminal_;
    GtkButton *clearButton_;
    GtkButton *scriptButton_;
    GtkScale *sizeScale_;
public:
    GtkTextView *input(void){return input_;}
protected:   
    GtkButton *toggleToIconview(void){return toggleToIconview_;}
    GtkButton *toggleToTerminal(void){return toggleToTerminal_;}
    GtkButton *clearButton(void){return clearButton_;}
    GtkButton *scriptButton(void){return scriptButton_;}
    GtkScale *sizeScale(void){return sizeScale_;}
public:
    HButtonBox(void){
	// The box
	hButtonBox_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
	fmButtonBox_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
	termButtonBox_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
	
        toggleToTerminal_ =  newButton("utilities-terminal-symbolic",_("Embedded Terminal"));
	toggleToIconview_ = newButton("system-file-manager-symbolic", _("Iconview"));
	scriptButton_ =  newButton("document-revert-symbolic", _("Script Recorder"));
	clearButton_ =  newButton("edit-clear-all", _("Clear log view"));
	input_ = createStatus(); // Status textview
	sizeScale_ = newSizeScale(_("Terminal font"));
	statusLabel_ = GTK_LABEL(gtk_label_new (""));
	g_object_ref(statusLabel_);
	auto statusButton = createStatusButton(statusLabel_);


        gtk_box_pack_start (hButtonBox_, GTK_WIDGET(fmButtonBox_), TRUE, TRUE, 0);
        gtk_box_pack_start (hButtonBox_, GTK_WIDGET(termButtonBox_), TRUE, TRUE, 0);


        gtk_box_pack_start (fmButtonBox_, GTK_WIDGET(toggleToTerminal_), FALSE, FALSE, 0);
        gtk_box_pack_start (fmButtonBox_, GTK_WIDGET(statusButton), TRUE, TRUE, 0);
        
        gtk_box_pack_start (termButtonBox_, GTK_WIDGET(toggleToIconview_), FALSE, FALSE, 0);
	gtk_box_pack_start (termButtonBox_, GTK_WIDGET(input_), TRUE, TRUE, 0);

	gtk_box_pack_end (termButtonBox_, GTK_WIDGET(clearButton_), FALSE, FALSE, 0);
	gtk_box_pack_end (termButtonBox_, GTK_WIDGET(sizeScale_), FALSE, FALSE, 0);
	gtk_box_pack_end (termButtonBox_, GTK_WIDGET(scriptButton_), FALSE, FALSE, 0);
	
	gtk_widget_show_all(GTK_WIDGET(hButtonBox_));


        return;  
    }

    ~HButtonBox(void){
	statusLabel_ = NULL;
	g_object_unref(statusLabel_);
    }

    GtkBox *hButtonBox(void){return hButtonBox_;}
    
    void showFmBox(void){
	gtk_widget_hide(GTK_WIDGET(termButtonBox_));
	gtk_widget_show_all(GTK_WIDGET(fmButtonBox_));
    }

    void showTermBox(void){
	gtk_widget_hide(GTK_WIDGET(fmButtonBox_));
	gtk_widget_show_all(GTK_WIDGET(termButtonBox_));
    }

    void setStatusLabel(const gchar *text){
	if (!statusLabel_) return;
        gchar *gg = g_strdup_printf("xffm+-%s", VERSION);
        gchar *g = g_strdup_printf("<span color=\"blue\"><b>%s</b></span>", 
		text?text:gg);
        g_free(gg);
        gtk_label_set_markup(statusLabel_,g);
        g_free(g); 
    }


    static GtkButton *newButton(const gchar *icon, const gchar *tooltipText){
	auto button =  GTK_BUTTON(gtk_button_new());
        gtk_c::setup_image_button(button, icon, tooltipText);
	gtk_widget_set_can_focus (GTK_WIDGET(button), FALSE);
	gtk_button_set_relief (button, GTK_RELIEF_NONE);
        gtk_widget_set_tooltip_markup (GTK_WIDGET(button),tooltipText);
	return button;
    }
private:

    static GtkButton *newButtonL(const gchar *icon, const gchar *tooltipText){
	auto button =  gtk_c::dialog_button(icon, NULL);
	gtk_widget_set_can_focus (GTK_WIDGET(button), FALSE);
	gtk_button_set_relief (button, GTK_RELIEF_NONE);
        gtk_widget_set_tooltip_markup (GTK_WIDGET(button),tooltipText);
	return button;
    }
    static GtkScale *newSizeScale(const gchar *tooltipText){
	auto size_scale = GTK_SCALE(gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 6.0, 24.0, 6.0));
        // Load saved value fron xffm+/settings.ini file (if any)
        gint size = Settings<Type>::getSettingInteger("xfterm", "fontSize");
        if (size < 0) size = DEFAULT_FIXED_FONT_SIZE;
        gtk_range_set_value(GTK_RANGE(size_scale), size);
        gtk_range_set_increments (GTK_RANGE(size_scale), 2.0, 6.0);
	gtk_widget_set_size_request (GTK_WIDGET(size_scale),75,-1);
	gtk_scale_set_value_pos (size_scale,GTK_POS_RIGHT);
        gtk_adjustment_set_upper (gtk_range_get_adjustment(GTK_RANGE(size_scale)), 24.0);
        gtk_widget_set_tooltip_markup (GTK_WIDGET(size_scale),tooltipText);	
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

    static GtkButton *createStatusButton(GtkLabel *label){
	auto statusBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
	auto statusButton = GTK_BUTTON(gtk_button_new());
        gtk_container_add (GTK_CONTAINER (statusButton), GTK_WIDGET(statusBox));
        gtk_box_pack_start (statusBox, GTK_WIDGET(label), FALSE, FALSE, 0);
	gtk_widget_show_all(GTK_WIDGET(statusButton));
        return statusButton;
    }

 };

}
#endif
