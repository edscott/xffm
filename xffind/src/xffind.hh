#ifndef XFFIND__HH
# define XFFIND__HH
#include "xfpixbuf.hh"
#include "xfgtk.hh"
#include "xftooltip.hh"

typedef struct radio_t {
    GtkBox *box;
    GtkToggleButton *toggle[5];
} radio_t;

namespace xf
{
    static const gchar *
    filter_text_help=
	        N_("Basic rules:\n" "\n"
                          "*  Will match any character zero or more times.\n"
                          "?  Will match any character exactly one time\n"
                          "[] Match any character within the [] \n"
                          "^  Match at beginning of string\n" 
			  "$  Match at end of string \n");
    static const gchar *
    grep_text_help=
		N_("Reserved characters for extended regexp are\n"
                          ". ^ $ [ ] ? * + { } | \\ ( ) : \n"
                          "In  basic regular expressions the metacharacters\n"
                          "?, +, {, |, (, and ) lose their special meaning.\n"
                          "\n"
                          "The  period . matches  any  single  character.\n"
                          "The caret ^ matches at the start of line.\n"
                          "The dollar $ matches at the end of line.\n" "\n"
                          "Characters within [ ] matches any single \n"
                          "       character in the list.\n"
                          "Characters within [^ ] matches any single\n"
                          "       character *not* in the list.\n"
                          "Characters inside [ - ] matches a range of\n"
                          "       characters (ie [0-9] or [a-z]).\n" "\n"
                          "A regular expression may be followed by one\n"
                          "       of several repetition operators:\n"
                          "?      The preceding item is optional and matched\n"
                          "       at most once.\n"
                          "*      The preceding item will be matched zero\n"
                          "       or more times.\n"
                          "+      The preceding item will be matched one or\n"
                          "       more times.\n"
                          "{n}    The preceding item is matched exactly n times.\n"
                          "{n,}   The preceding item is matched n or more times.\n"
                          "{n,m}  The preceding item is matched at least n times,\n"
                          "       but not more than m times.\n" "\n"
                          "To match any reserved character, precede it with \\. \n"
                          "\n"
                          "Two regular expressions may be joined by the logical or\n"
                          "       operator |.\n"
                          "Two regular expressions may be concatenated.\n" "\n"
                          "More information is available by typing \"man grep\"\n"
			  );


template <class Type>
class Signals{
public:
    static void 
    sensitivize (GtkToggleButton *togglebutton, gpointer data){
	GtkWidget *widget = GTK_WIDGET(data);
	gtk_widget_set_sensitive(widget, gtk_toggle_button_get_active(togglebutton));
    }

    static void 
    sensitivize_radio (GtkToggleButton *togglebutton, gpointer data){
	if (!data) return;
	radio_t *radio_p = (radio_t *)data;
	gtk_widget_set_sensitive(GTK_WIDGET(radio_p->box), FALSE);
	GtkToggleButton **tb_p = radio_p->toggle;
	for (; tb_p && *tb_p; tb_p++){
	    if (gtk_toggle_button_get_active(*tb_p)){
		gtk_widget_set_sensitive(GTK_WIDGET(radio_p->box), TRUE);
	    }
	}
    }

    static void
    command_help (GtkWidget * button, gpointer data) {
	GtkWidget *dialog_=GTK_WIDGET(g_object_get_data(G_OBJECT(button), "dialog_"));
	gchar *argv[]={(gchar *)data, (gchar *)"--help", NULL};
	std::cerr<<"fixme: signals::command_help\n";
	// FIXME
	/*
	rfm_clear_text (widgets_p); 
	rfm_show_text (widgets_p); 
	rfm_thread_run_argv_full (widgets_p, argv, FALSE, NULL, rfm_markup_stdout_f, rfm_dump_output, rfm_null_function);
	*/
    }

    static void
    on_help_filter (GtkToggleButton * button, gpointer data) {
	std::cerr<<"fixme: signals::on_help_filter\n";
	// FIXME
	/*GtkWidget *dialog=data;
	widgets_t *widgets_p=g_object_get_data(G_OBJECT(dialog), "widgets_p");
	if (gtk_toggle_button_get_active (button)) {
	    rfm_clear_text(widgets_p);
	    rfm_show_text (widgets_p); 
	    rfm_diagnostics(widgets_p,"xffm_tag/blue", _(filter_text_help), NULL);
	    rfm_scroll_to_top(widgets_p);
	} else {
	    rfm_clear_text(widgets_p);
	}*/
    }

    static void
    on_help_grep (GtkToggleButton * button, gpointer data) {
	std::cerr<<"fixme: signals::on_help_grep\n";
	// FIXME
	/*GtkWidget *dialog=data;
	widgets_t *widgets_p=g_object_get_data(G_OBJECT(dialog), "widgets_p");
	if (gtk_toggle_button_get_active (button)) {
	    rfm_clear_text(widgets_p);
	    rfm_show_text (widgets_p); 
	    rfm_diagnostics(widgets_p,"xffm_tag/blue", _(grep_text_help), NULL);
	    rfm_scroll_to_top(widgets_p);
	} else {
	    rfm_clear_text(widgets_p);
	}*/
    }
};

template <class Type>
class FindDialog
{
    typedef Gtk<double> gtk_c;
    typedef Signals<double> signals;
    typedef Pixbuf<double> pixbuf_c;
    typedef Tooltip<double> tooltip_c;
#define TB_CALLBACK(X)  (void (*)(GtkButton *,gpointer)) X
#define W_CALLBACK(X)  (void (*)(GtkWidget *,gpointer)) X
//    typedef typename  void (*)(GtkToggleButton *,gpointer) toggleButtonCallback;
public:
    FindDialog(const gchar *path){
	if (!whichGrep()){
	    std::cerr<<"DBG> grep command not found\n";
	    exit(1);
	}
	createDialog(path);
    }
private:

    gboolean whichGrep(void){
	gchar *grep = g_find_program_in_path ("grep");
	if (!grep) return FALSE;
	FILE *pipe;
	const gchar *cmd = "grep --version";
	gnuGrep_ = FALSE;
	pipe = popen (cmd, "r");
	if(pipe) {
	    gchar line[256];
	    memset (line, 0, 256);
	    if(fgets (line, 255, pipe) == NULL)
		DBG ("fgets: %s\n", strerror (errno));
	    pclose (pipe);
	    if(strstr (line, "GNU")) gnuGrep_ = TRUE;
	}
	g_free (grep);
	return TRUE;
    }
    void setWindowMaxSize(void){
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
	gtk_window_set_geometry_hints (dialog_, GTK_WIDGET(dialog_), &geometry_, GDK_HINT_MAX_SIZE);
    }
    
    void mkDialog(void){
	dialog_ = GTK_WINDOW(gtk_window_new (GTK_WINDOW_TOPLEVEL));
	gtk_window_set_type_hint(dialog_, GDK_WINDOW_TYPE_HINT_DIALOG);
	setWindowMaxSize();
	//g_object_set_data(G_OBJECT(dialog_), "window", dialog_);
	gtk_window_set_title (dialog_, _("Find"));
	gtk_window_set_position (dialog_, GTK_WIN_POS_MOUSE);

	GdkPixbuf *pixbuf = pixbuf_c::get_pixbuf("edit-find", SIZE_ICON);
	gtk_window_set_icon (dialog_, pixbuf);
	g_object_unref(pixbuf);
	
    }


    void mkVpane(void){
	mainVbox_ = gtk_c::vboxNew(FALSE, 0);
	gtk_widget_show (GTK_WIDGET(mainVbox_));
	gtk_container_add (GTK_CONTAINER (dialog_), GTK_WIDGET(mainVbox_));

	vpane_ = GTK_PANED(gtk_paned_new(GTK_ORIENTATION_VERTICAL));
	g_object_set_data(G_OBJECT(dialog_), "vpane_", vpane_);
	gtk_widget_show (GTK_WIDGET(vpane_));
	gtk_box_pack_start (mainVbox_, GTK_WIDGET(vpane_), TRUE, TRUE, 0);
	// hack: widgets_p->paper = dialog_;
	topPaneVbox_ = gtk_c::vboxNew(FALSE, 6);
	gtk_container_set_border_width (GTK_CONTAINER (topPaneVbox_), 5);

	GtkScrolledWindow *sw = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new(NULL, NULL));
	gtk_paned_pack1 (GTK_PANED (vpane_), GTK_WIDGET (sw), FALSE, TRUE);
	gtk_container_add(GTK_CONTAINER(sw), GTK_WIDGET(topPaneVbox_));
	gtk_widget_show (GTK_WIDGET(sw));
	gtk_scrolled_window_set_policy (sw, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	GtkTextView *diagnostics = GTK_TEXT_VIEW(gtk_text_view_new ());
	GtkScrolledWindow *scrolledwindow = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new (NULL, NULL));
	gtk_widget_show (GTK_WIDGET(scrolledwindow));
	gtk_widget_show (GTK_WIDGET(diagnostics));
	gtk_paned_pack2 (GTK_PANED (vpane_), GTK_WIDGET(scrolledwindow), TRUE, TRUE);
	gtk_scrolled_window_set_policy (scrolledwindow, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	gtk_container_add (GTK_CONTAINER (scrolledwindow), GTK_WIDGET(diagnostics));
	gtk_container_set_border_width (GTK_CONTAINER (diagnostics), 2);
	gtk_widget_set_can_focus(GTK_WIDGET(diagnostics), FALSE);
	gtk_text_view_set_wrap_mode (diagnostics, GTK_WRAP_WORD);
	gtk_text_view_set_cursor_visible (diagnostics, FALSE);

	topPaneHbox_= gtk_c::hboxNew(FALSE, 0);
	gtk_box_pack_start (topPaneVbox_, GTK_WIDGET(topPaneHbox_), FALSE, FALSE, 0);

	gchar *t=g_strdup_printf("<b>%s </b>  ", _("Find"));
	GtkLabel *title = GTK_LABEL(gtk_label_new (t));
	g_free(t);
	gtk_widget_show (GTK_WIDGET(title));
	gtk_box_pack_start (topPaneHbox_, GTK_WIDGET(title), FALSE, FALSE, 0);
	gtk_label_set_use_markup (title, TRUE);
    }


    void createDialog(const gchar *path){
	// FIXME: path not in use...
	mkDialog();
	mkVpane();

	GtkAccelGroup *accel_group = gtk_accel_group_new ();


			
	GtkButton *button;
	button = gtk_c::dialog_button ("dialog-question", NULL);
	tooltip_c::custom_tooltip(GTK_WIDGET(button), NULL, "fgr --help");
	g_signal_connect (G_OBJECT (button), "clicked", 
		 G_CALLBACK (W_CALLBACK(signals::command_help)), (gpointer)"fgr");
	g_object_set_data(G_OBJECT(button), "dialog_", dialog_);
	gtk_box_pack_start (GTK_BOX (topPaneHbox_), GTK_WIDGET(button), FALSE, FALSE, 0);
	gtk_widget_show(GTK_WIDGET(button));

       

	GtkBox *vbox7a = gtk_c::vboxNew (FALSE, 5);
	gtk_widget_show (GTK_WIDGET(vbox7a));
	gtk_box_pack_start (topPaneVbox_, GTK_WIDGET(vbox7a), TRUE, TRUE, 0);

	GtkBox *path_box = gtk_c::hboxNew (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox7a), GTK_WIDGET(path_box), FALSE, FALSE, 0);

	gchar *t=g_strdup_printf("%s:", _("Path"));
	GtkWidget *path_label = gtk_label_new (t);
	g_free(t);
	gtk_widget_show (path_label);

	GtkComboBox *path_combo = GTK_COMBO_BOX(gtk_combo_box_new_with_entry ());

	g_object_set_data (G_OBJECT (path_combo), "GladeParentKey", path_combo);
	g_object_set_data(G_OBJECT(dialog_), "path_combo", path_combo);


	button = gtk_c::dialog_button ("folder", NULL);
	GtkBox *vbox = gtk_c::vboxNew (FALSE, 6);
	gtk_box_pack_start (vbox, GTK_WIDGET(button), FALSE, FALSE, 0);
	gtk_box_pack_start (path_box, GTK_WIDGET(path_label), FALSE, FALSE, 0);
	gtk_box_pack_start (path_box, GTK_WIDGET(path_combo), FALSE, TRUE, 0);
	gtk_box_pack_start (path_box, GTK_WIDGET(vbox), FALSE, FALSE, 0);
	gtk_widget_show (GTK_WIDGET(vbox));
	gtk_widget_show (GTK_WIDGET(path_combo));
	gtk_widget_show (GTK_WIDGET(button));
	g_object_set_data(G_OBJECT(dialog_), "fileselector", button);
	

	GtkBox *filter_box = gtk_c::hboxNew (FALSE, 0);
	gtk_box_pack_start (vbox7a, GTK_WIDGET(filter_box), TRUE, FALSE, 5);

	gchar *text=g_strdup_printf("%s ", _("Filter:"));
	GtkLabel *filter_label = GTK_LABEL(gtk_label_new (text));
	g_free(text);
	gtk_widget_show (GTK_WIDGET(filter_label));
	gtk_box_pack_start (filter_box, GTK_WIDGET(filter_label), FALSE, FALSE, 0);


	GtkComboBox *filter_combo = GTK_COMBO_BOX(gtk_combo_box_new_with_entry ());
	g_object_set_data (G_OBJECT (filter_combo), "GladeParentKey", (gpointer)filter_combo);
	gtk_widget_show (GTK_WIDGET(filter_combo));
	gtk_box_pack_start (GTK_BOX (filter_box), GTK_WIDGET(filter_combo), FALSE, TRUE, 0);
	g_object_set_data(G_OBJECT(dialog_), "filter_combo", (gpointer)filter_combo);
	
	GtkToggleButton *togglebutton2 = gtk_c::toggle_button("dialog-question", "");
	gtk_box_pack_start (filter_box, GTK_WIDGET(togglebutton2), FALSE, FALSE, 0);
	g_object_set_data(G_OBJECT(dialog_), "togglebutton2", togglebutton2);
	tooltip_c::custom_tooltip(GTK_WIDGET(togglebutton2), NULL,  _(filter_text_help));
	g_signal_connect (togglebutton2,
			  "toggled", G_CALLBACK (TB_CALLBACK(signals::on_help_filter)), 
			  (gpointer) dialog_);
	

	GtkBox *hbox17 = gtk_c::hboxNew (FALSE, 0);
	gtk_widget_show (GTK_WIDGET(hbox17));
	gtk_box_pack_start (vbox7a, GTK_WIDGET(hbox17), TRUE, FALSE, 0);

	GtkBox *left_options_vbox = gtk_c::vboxNew (FALSE, 0);
	gtk_box_pack_start (hbox17, GTK_WIDGET(left_options_vbox), FALSE, FALSE, 0);
	GtkBox *center_options_vbox = gtk_c::vboxNew (FALSE, 0);
	gtk_box_pack_start (hbox17, GTK_WIDGET(center_options_vbox), FALSE, FALSE, 0);
	GtkBox *right_options_vbox = gtk_c::vboxNew (FALSE, 0);
	gtk_box_pack_start (hbox17, GTK_WIDGET(right_options_vbox), FALSE, FALSE, 0);

	/// option -r "recursive"
	gtk_toggle_button_set_active(add_option_spin(left_options_vbox, "recursive", NULL, _("Recursive"), 0), default_recursive);

	/// option -D "recursiveH"
	gtk_toggle_button_set_active(add_option_spin(left_options_vbox, "recursiveH", NULL, _("Find hidden files and directories"), 0), default_recursiveH);

	/// option -a "xdev"
	gtk_toggle_button_set_active(add_option_spin(left_options_vbox, "xdev", NULL, _("Stay on single filesystem"), 0), default_xdev);

	/// option "upper_limit_spin" (only in gtk dialog_)
	text = g_strdup_printf("%s (%s)", _("Results"), _("Upper limit"));
	add_option_spin(left_options_vbox, NULL, "upper_limit_spin", text, result_limit);
	g_free(text);

	// option -s +KByte "size_greater", "size_greater_spin"
	text = g_strdup_printf("%s (%s)", _("At Least"), _("kBytes"));
	add_option_spin(center_options_vbox, "size_greater", "size_greater_spin", text, size_greater);
	g_free(text);
	
	// option -s -KByte "size_smaller", "size_smaller_spin"
	text = g_strdup_printf("%s (%s)", _("At Most"), _("kBytes"));
	add_option_spin(center_options_vbox, "size_smaller", "size_smaller_spin", text, size_smaller);
	g_free(text);

	GSList *slist;
	// option -u uid "uid" "uid_combo"
	slist = get_user_slist();
	add_option_combo(center_options_vbox, "uid", "uid_combo", _("User"), slist);
	slist = free_string_slist(slist);

	// option -g gid "gid" "gid_combo"
	slist = get_group_slist();
	add_option_combo(center_options_vbox, "gid", "gid_combo", _("Group"), slist);
	slist = free_string_slist(slist);
	
	// option -o octal "octal_p" "permissions_entry"
	add_option_entry(center_options_vbox, "octal_p", "permissions_entry", _("Octal Permissions"), "0666");
	GtkEntry *entry = GTK_ENTRY(g_object_get_data(G_OBJECT(dialog_), "permissions_entry"));
	gtk_widget_set_size_request (GTK_WIDGET(entry), 75, -1);
	
	// option -p suid | exe 
	add_option_radio2(center_options_vbox, "suidexe", "suid_radio", "exe_radio", _("SUID"), _("Executable"));

	
	// option -M -A -C
	radio_t *radio_p = create_radios(right_options_vbox);
	// radio_p freed on destroy event for dialog_.

	// option -k minutes "last_minutes", "last_minutes_spin"
	radio_p->toggle[0] = add_option_spin(right_options_vbox, "last_minutes", "last_minutes_spin", _("Minutes"), last_minutes);
	g_signal_connect (G_OBJECT (radio_p->toggle[0]), "toggled", 
		G_CALLBACK (TB_CALLBACK(signals::sensitivize_radio)), radio_p);
	
       // option -h hours "last_hours", "last_hours_spin"
	radio_p->toggle[1] = add_option_spin(right_options_vbox, "last_hours", "last_hours_spin", _("Hours"), last_hours);
	g_signal_connect (G_OBJECT (radio_p->toggle[1]), "toggled", 
	    G_CALLBACK (TB_CALLBACK(signals::sensitivize_radio)), radio_p);
	
	// option -d days "last_days", "last_days_spin"
	radio_p->toggle[2] = add_option_spin(right_options_vbox, "last_days", "last_days_spin", _("Days"), last_days);
	    //gtk_box_pack_start (GTK_BOX (topPaneHbox_), GTK_WIDGET(check), FALSE, FALSE, 0);
	    g_signal_connect (G_OBJECT (radio_p->toggle[2]), "toggled", 
		G_CALLBACK (TB_CALLBACK(signals::sensitivize_radio)), radio_p);
	
	// option -m months "last_months", "last_months_spin"
	radio_p->toggle[3] = add_option_spin(right_options_vbox, "last_months", "last_months_spin", _("Months"), last_months);
	g_signal_connect (G_OBJECT (radio_p->toggle[3]), "toggled", 
	    G_CALLBACK (TB_CALLBACK(signals::sensitivize_radio)), radio_p);
	
	 ///////////

     
	GtkBox *hbox21 = gtk_c::hboxNew (FALSE, 0);
	gtk_widget_show (GTK_WIDGET(hbox21));
	gtk_box_pack_start (left_options_vbox, GTK_WIDGET(hbox21), TRUE, FALSE, 0);

	GtkLabel *label37 = GTK_LABEL(gtk_label_new (_("File type : ")));
	gtk_widget_show (GTK_WIDGET(label37));
	gtk_box_pack_start (hbox21, GTK_WIDGET(label37), FALSE, FALSE, 0);

	GtkComboBoxText *file_type_om =  GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
	gtk_widget_show (GTK_WIDGET(file_type_om));
	gtk_box_pack_start (hbox21, GTK_WIDGET(file_type_om), TRUE, TRUE, 0);
	g_object_set_data(G_OBJECT(dialog_), "file_type_om", file_type_om);

	////////////////  grep options.... /////////////////////////
	
	t=g_strdup_printf("<b>%s</b>", _("Contains"));
	GtkLabel *contains_label = GTK_LABEL(gtk_label_new (t));
	g_free(t);
	gtk_widget_show (GTK_WIDGET(contains_label));
	gtk_box_pack_start (topPaneVbox_, GTK_WIDGET(contains_label), FALSE, FALSE, 1);
	gtk_label_set_use_markup (GTK_LABEL (contains_label), TRUE);

	GtkBox *hbox26 = gtk_c::hboxNew (FALSE, 0);
	gtk_widget_show (GTK_WIDGET(hbox26));
	gtk_box_pack_start (topPaneVbox_, GTK_WIDGET(hbox26), FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox26), 2);

	GtkBox *vbox8 = gtk_c::vboxNew (FALSE, 5);
	gtk_widget_show (GTK_WIDGET(vbox8));
	gtk_box_pack_start (hbox26, GTK_WIDGET(vbox8), TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox8), 5);

	GtkBox *grep_box = gtk_c::hboxNew (FALSE, 0);
	gtk_box_pack_start (vbox8, GTK_WIDGET(grep_box), FALSE, FALSE, 0);

	t=g_strdup_printf("%s: ",_("Contains the text"));
	GtkLabel *grep_label = GTK_LABEL(gtk_label_new (t));
	g_free(t);
	gtk_widget_show (GTK_WIDGET(grep_label));
	gtk_box_pack_start (grep_box, GTK_WIDGET(grep_label), FALSE, FALSE, 0);

	GtkComboBox *grep_combo = GTK_COMBO_BOX(gtk_combo_box_new_with_entry ());
	g_object_set_data (G_OBJECT (grep_combo), "GladeParentKey", grep_combo);
	gtk_widget_show (GTK_WIDGET(grep_combo));
	gtk_box_pack_start (grep_box, GTK_WIDGET(grep_combo), FALSE, FALSE, 5);
	gtk_widget_set_sensitive (GTK_WIDGET(grep_combo), TRUE);
	g_object_set_data(G_OBJECT(dialog_), "grep_combo", grep_combo);

	
	GtkToggleButton *togglebutton3 = gtk_c::toggle_button ("dialog-question", "");
	gtk_box_pack_start (grep_box, GTK_WIDGET(togglebutton3), FALSE, FALSE, 0);
	g_object_set_data(G_OBJECT(dialog_), "togglebutton3", togglebutton3);
	tooltip_c::custom_tooltip(GTK_WIDGET(togglebutton3), NULL, _(grep_text_help));
	g_signal_connect (GTK_WIDGET(togglebutton3),
			  "toggled", G_CALLBACK (TB_CALLBACK(signals::on_help_grep)), 
			  (gpointer) dialog_);
	

	GtkBox *hbox20 = gtk_c::hboxNew (FALSE, 0);
	gtk_widget_show (GTK_WIDGET(hbox20));
	gtk_box_pack_start (vbox8, GTK_WIDGET(hbox20), FALSE, FALSE, 0);

	GtkBox *vbox13 = gtk_c::vboxNew (FALSE, 0);
	gtk_box_pack_start (hbox20, GTK_WIDGET(vbox13), TRUE, TRUE, 0);

	GtkCheckButton *case_sensitive =  
	    GTK_CHECK_BUTTON(gtk_check_button_new_with_mnemonic (_("Case Sensitive")));
	gtk_widget_show (GTK_WIDGET(case_sensitive));
	gtk_box_pack_start (GTK_BOX (vbox13), GTK_WIDGET(case_sensitive), FALSE, FALSE, 0);
	gtk_widget_set_sensitive (GTK_WIDGET(case_sensitive), FALSE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (case_sensitive), default_case_sensitive);
	g_object_set_data(G_OBJECT(dialog_), "case_sensitive", case_sensitive);

	GtkCheckButton *ext_regexp =  
	    GTK_CHECK_BUTTON(gtk_check_button_new_with_mnemonic (_("Extended regexp")));
	gtk_widget_show (GTK_WIDGET(ext_regexp));
	gtk_box_pack_start (GTK_BOX (vbox13), GTK_WIDGET(ext_regexp), FALSE, FALSE, 0);
	gtk_widget_set_sensitive (GTK_WIDGET(ext_regexp), FALSE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ext_regexp), default_ext_regexp);
	g_object_set_data(G_OBJECT(dialog_), "ext_regexp", ext_regexp);

	GtkCheckButton *look_in_binaries = 
	    GTK_CHECK_BUTTON(gtk_check_button_new_with_mnemonic (_("Include binary files")));
	gtk_widget_show (GTK_WIDGET(look_in_binaries));
	gtk_box_pack_start (GTK_BOX (vbox13), GTK_WIDGET(look_in_binaries), FALSE, FALSE, 0);
	gtk_widget_set_sensitive (GTK_WIDGET(look_in_binaries), FALSE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (look_in_binaries), default_look_in_binaries);
	g_object_set_data(G_OBJECT(dialog_), "look_in_binaries", look_in_binaries);

	GtkCheckButton *line_count = 
	    GTK_CHECK_BUTTON(gtk_check_button_new_with_mnemonic (_("Line Count")));
	// XXX: (FIXME) this option (-c) does not work in fgr...
	//    gtk_widget_show (line_count);
	gtk_box_pack_start (GTK_BOX (vbox13), GTK_WIDGET(line_count), FALSE, FALSE, 0);
	gtk_widget_set_sensitive (GTK_WIDGET(line_count), FALSE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (line_count), default_line_count);
	g_object_set_data(G_OBJECT(dialog_), "line_count", line_count);
	
       
	GtkBox *hbox28 = gtk_c::hboxNew (FALSE, 0);
	gtk_box_pack_start (topPaneVbox_, GTK_WIDGET(hbox28), TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox28), 2);

	GtkBox *vbox11 = gtk_c::vboxNew (FALSE, 0);
	gtk_widget_show (GTK_WIDGET(vbox11));
	gtk_box_pack_start (hbox28, GTK_WIDGET(vbox11), TRUE, TRUE, 0);



	GtkBox *hbox24 = gtk_c::hboxNew (FALSE, 0);
	gtk_box_pack_start (vbox11, GTK_WIDGET(hbox24), FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox24), 5);
	
	GtkLabel *label40 = GTK_LABEL(gtk_label_new (""));
	t=g_strdup_printf("<b>%s</b>: ", _("Match"));
	gtk_label_set_markup (label40, t);
	g_free(t);
	gtk_widget_show (GTK_WIDGET(label40));
	gtk_box_pack_start (GTK_BOX (hbox24), GTK_WIDGET(label40), FALSE, FALSE, 0);
	gtk_widget_set_sensitive (GTK_WIDGET(label40), FALSE);
	g_object_set_data(G_OBJECT(dialog_), "label40", label40);

	GSList *anywhere_group = NULL;
	GtkRadioButton *anywhere = 
	    GTK_RADIO_BUTTON (gtk_radio_button_new_with_mnemonic (NULL, _("Anywhere")));
	gtk_widget_show (GTK_WIDGET(anywhere));
	gtk_box_pack_start (GTK_BOX (hbox24), GTK_WIDGET(anywhere), FALSE, FALSE, 0);
	gtk_radio_button_set_group (anywhere, anywhere_group);
	g_object_set_data(G_OBJECT(dialog_), "anywhere", anywhere);
	anywhere_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (anywhere));
	gtk_widget_set_sensitive (GTK_WIDGET(anywhere), FALSE);
	if (default_anywhere) {
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (anywhere), default_anywhere);
	}
	 

	GtkRadioButton *match_words = 
	    GTK_RADIO_BUTTON (gtk_radio_button_new_with_mnemonic (NULL, _("Whole words only")));
	gtk_widget_show (GTK_WIDGET(match_words));
	gtk_box_pack_start (GTK_BOX (hbox24), GTK_WIDGET(match_words), FALSE, FALSE, 0);
	gtk_radio_button_set_group (match_words, anywhere_group);
	g_object_set_data(G_OBJECT(dialog_), "match_words", match_words);
	anywhere_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (match_words));
	gtk_widget_set_sensitive (GTK_WIDGET(match_words), FALSE);
	if (default_match_words) {
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (match_words), default_match_words);
	}


	GtkRadioButton *match_lines = 
	    GTK_RADIO_BUTTON (gtk_radio_button_new_with_mnemonic (NULL, _("lines")));
	gtk_widget_show (GTK_WIDGET(match_lines));
	gtk_box_pack_start (GTK_BOX (hbox24), GTK_WIDGET(match_lines), FALSE, FALSE, 0);
	gtk_radio_button_set_group (match_lines, anywhere_group);
	g_object_set_data(G_OBJECT(dialog_), "match_lines", match_lines);
	anywhere_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (match_lines));
	gtk_widget_set_sensitive (GTK_WIDGET(match_lines), FALSE);
	if (default_match_lines) {
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (match_lines), default_match_lines);
	}

	GtkRadioButton *match_no_match =
	    GTK_RADIO_BUTTON (gtk_radio_button_new_with_mnemonic (NULL, _("No match")));
	gtk_widget_show (GTK_WIDGET(match_no_match));
	gtk_box_pack_start (GTK_BOX (hbox24), GTK_WIDGET(match_no_match), FALSE, FALSE, 0);
	gtk_radio_button_set_group (match_no_match, anywhere_group);
	g_object_set_data(G_OBJECT(dialog_), "match_no_match", match_no_match);
	anywhere_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (match_no_match));
	gtk_widget_set_sensitive (GTK_WIDGET(match_no_match), FALSE);
	if (default_match_no_match) {
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (match_no_match), default_match_no_match);
	}

	gtk_widget_show(GTK_WIDGET(topPaneHbox_));
	gtk_widget_show (GTK_WIDGET(path_box));
	gtk_widget_show (GTK_WIDGET(filter_box));
	gtk_widget_show (GTK_WIDGET(right_options_vbox));
	gtk_widget_show (GTK_WIDGET(center_options_vbox));
	gtk_widget_show (GTK_WIDGET(left_options_vbox));
	gtk_widget_show (GTK_WIDGET(grep_box));
	gtk_widget_show (GTK_WIDGET(vbox13));
	gtk_widget_show (GTK_WIDGET(hbox28));
	gtk_widget_show (GTK_WIDGET(hbox24));
	gtk_widget_show(GTK_WIDGET(topPaneVbox_));


	GtkButtonBox *hbuttonbox2 = GTK_BUTTON_BOX (gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL));
	gtk_widget_show (GTK_WIDGET(hbuttonbox2));
	gtk_box_pack_start (GTK_BOX (mainVbox_), GTK_WIDGET(hbuttonbox2), FALSE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (hbuttonbox2), 3);
	gtk_button_box_set_layout (hbuttonbox2, GTK_BUTTONBOX_EDGE);

// FIXME
	/*
	GtkWidget *button_image;
	gchar *g=g_strdup_printf("%s/%s", _("Find"), _("Close"));
	GtkWidget *find_button =  
	    rfm_dialog_button("xffm/stock_find", g);
	g_free(g);
	g_object_set_data(G_OBJECT(dialog_), "find_button", find_button);
	g_object_set_data(G_OBJECT(find_button), "dialog_", dialog_);
	
	GtkWidget *apply_button =  
	    rfm_dialog_button("xffm/stock_find", _("Find"));
	g_object_set_data(G_OBJECT(dialog_), "apply_button", apply_button);
	g_object_set_data(G_OBJECT(apply_button), "dialog_", dialog_);

	GtkWidget *cancel_button =  
	    rfm_dialog_button("xffm/stock_cancel", _("Cancel"));
	g_object_set_data(G_OBJECT(dialog_), "cancel_button", cancel_button);
	g_object_set_data(G_OBJECT(cancel_button), "dialog_", dialog_);
	gtk_widget_set_sensitive(cancel_button, FALSE);
	

	GtkWidget *clear_button =  
	    rfm_dialog_button("xffm/stock_clear", _("Clear"));
	g_object_set_data(G_OBJECT(dialog_), "clear_button", clear_button);
	g_object_set_data(G_OBJECT(clear_button), "dialog_", dialog_);
	g_signal_connect (G_OBJECT (clear_button), "clicked",
		G_CALLBACK (clear_text), NULL);

	GtkWidget *close_button =  
	    rfm_dialog_button("xffm/stock_close", _("Close"));
	g_object_set_data(G_OBJECT(dialog_), "close_button", close_button);
	g_object_set_data(G_OBJECT(close_button), "dialog_", dialog_);


	gtk_widget_show (find_button);
	gtk_container_add (GTK_CONTAINER (hbuttonbox2), find_button);
	gtk_widget_set_can_default(find_button, TRUE);

	gtk_widget_show (apply_button);
	gtk_container_add (GTK_CONTAINER (hbuttonbox2), apply_button);
	gtk_widget_set_can_default(apply_button, TRUE);

	gtk_widget_show (clear_button);
	gtk_container_add (GTK_CONTAINER (hbuttonbox2), clear_button);

	gtk_widget_show (cancel_button);
	gtk_container_add (GTK_CONTAINER (hbuttonbox2), cancel_button);

	const gchar *editor = getenv("EDITOR");
	if (editor && strlen(editor)){
	    gchar *basename = g_strdup(editor);
	    if (strchr(basename, ' ')) *strchr(basename, ' ') = 0;
	    gchar *editor_path = g_find_program_in_path(basename);
	    if (editor_path){
		gchar *iconpath=g_strdup(basename);
		GdkPixbuf *pix = rfm_get_pixbuf (iconpath, SMALL_ICON_SIZE); //refs
		if (!pix) iconpath = g_strdup("xffm/stock_edit");
		else g_object_unref(pix);

		GtkWidget *edit_button = 
		    rfm_dialog_button(iconpath, _("Edit"));
		g_free(iconpath);
		g_object_set_data(G_OBJECT(dialog_), "edit_button", edit_button);
		g_object_set_data(G_OBJECT(edit_button), "dialog_", dialog_);
		g_signal_connect (G_OBJECT (edit_button), "clicked", G_CALLBACK (edit_command), NULL);
		gtk_widget_show(edit_button);
		gtk_container_add (GTK_CONTAINER (hbuttonbox2), edit_button);
		g_free(editor_path);
	    } 
	    g_free(basename);
	} else {
	    NOOP("getenv(\"EDITOR\") = %s\n", editor);
	}


	gtk_widget_show (close_button);
	gtk_container_add (GTK_CONTAINER (hbuttonbox2), close_button);
	gtk_widget_add_accelerator (close_button, "clicked", accel_group, GDK_KEY_Escape, 0, GTK_ACCEL_VISIBLE);

	gtk_widget_grab_default (find_button);
	gtk_window_add_accel_group (GTK_WINDOW (dialog_), accel_group);

	*/

	gtk_widget_realize(GTK_WIDGET(dialog_));
     
	GtkAllocation label_allocation;
	GtkAllocation allocation;
	gtk_widget_get_allocation(GTK_WIDGET(topPaneVbox_), &allocation);
	gtk_widget_get_allocation(GTK_WIDGET(path_label), &label_allocation);
	// width will be the smaller of the two.
	gint width = (geometry_.max_width < allocation.width)?
	    geometry_.max_width: allocation.width;
	NOOP(stderr, "max w= %d, top_pane w= %d, label w=%d\n", 
		geometry_.max_width,
		 allocation.width, 
		label_allocation.width);
	gint combo_width;
	combo_width = width - label_allocation.width - 120;
	if (combo_width > 100){
	    gtk_widget_set_size_request (GTK_WIDGET(path_combo), combo_width, -1);
	}
	gtk_widget_get_allocation(GTK_WIDGET(filter_label), &label_allocation);
	combo_width = width - label_allocation.width - 120;
	if (combo_width > 100){
	    gtk_widget_set_size_request (GTK_WIDGET(filter_combo), combo_width, -1);
	}
	gtk_widget_get_allocation(GTK_WIDGET(grep_label), &label_allocation);
	combo_width = width - label_allocation.width - 120;
	if (combo_width > 100){
	    gtk_widget_set_size_request (GTK_WIDGET(grep_combo), combo_width, -1);
	}

	// This will be constrained to the maximum width set by geometry_ hints:
	gtk_window_set_default_size(GTK_WINDOW (dialog_),  allocation.width+50,  allocation.height+75);
 	
	
	gtk_paned_set_position (vpane_, 1000);
	gtk_widget_show_all(GTK_WIDGET(dialog_));
	
	return;
    }

    GtkWindow *dialog_;
    gboolean gnuGrep_;
    GtkBox *topPaneVbox_;
    GtkBox *topPaneHbox_;
    GtkBox *mainVbox_;
    GdkGeometry geometry_;
    GtkPaned *vpane_;
    

    
    GSList *
    get_user_slist(void){
	GSList *g_user = NULL;
	struct passwd *pw;
	while((pw = getpwent ()) != NULL) {
	    g_user = g_slist_append (g_user, g_strdup (pw->pw_name));
	}
	g_user = g_slist_sort (g_user, (GCompareFunc) strcmp);
	endpwent ();
	pw = getpwuid (geteuid ());
	gchar *buf = g_strdup_printf ("%s", pw ? pw->pw_name : _("unknown"));
	g_user = g_slist_prepend (g_user, buf);
	return g_user;
    }

    GSList *
    get_group_slist(void){
	GSList *g_group=NULL;
	struct group *gr;
	while((gr = getgrent ()) != NULL) {
	   g_group = g_slist_append (g_group, g_strdup (gr->gr_name));
	}
	endgrent ();
	g_group = g_slist_sort (g_group, (GCompareFunc) strcmp);
	gr = getgrgid (geteuid ());
	gchar *buf = g_strdup_printf ("%s", gr ? gr->gr_name : _("unknown"));
	g_group = g_slist_prepend (g_group, buf);
	return g_group;
    }

    radio_t *
    create_radios(GtkBox *options_vbox){
	radio_t *radio_p = (radio_t *)malloc(sizeof(radio_t));
	if (!radio_p) g_error("malloc: %s", strerror(errno));
	g_object_set_data(G_OBJECT(dialog_), "radio_p", radio_p );
	memset(radio_p, 0, sizeof(radio_t));

	GtkRadioButton *radio1 =  
	    GTK_RADIO_BUTTON(gtk_radio_button_new_with_label (NULL, "mtime"));
	// FIXME should work when radios are sensitive... check this
	tooltip_c::custom_tooltip(GTK_WIDGET(radio1), NULL, _("Modified"));
	GtkRadioButton *radio2 = 
	    GTK_RADIO_BUTTON(gtk_radio_button_new_with_label_from_widget ( radio1, "ctime"));
	// FIXME should work when radios are sensitive... check this
	tooltip_c::custom_tooltip(GTK_WIDGET(radio2), NULL, _("Created"));
	GtkRadioButton *radio3 = 
	    GTK_RADIO_BUTTON(gtk_radio_button_new_with_label_from_widget ( radio1, "atime"));
	// FIXME should work when radios are sensitive... check this
	tooltip_c::custom_tooltip(GTK_WIDGET(radio3), NULL, _("Accessed"));

	g_object_set_data(G_OBJECT(dialog_), "radio1", radio1 );
	g_object_set_data(G_OBJECT(dialog_), "radio2", radio2 );
	g_object_set_data(G_OBJECT(dialog_), "radio3", radio3 );
	gtk_widget_show (GTK_WIDGET(radio1));
	gtk_widget_show (GTK_WIDGET(radio2));
	gtk_widget_show (GTK_WIDGET(radio3));

	GtkBox *radio_box=gtk_c::vboxNew (FALSE, 0);
	g_object_set_data(G_OBJECT(dialog_), "radio_box", radio_box );
	gtk_widget_show (GTK_WIDGET(radio_box));
	gtk_box_pack_start (options_vbox, GTK_WIDGET(radio_box), TRUE, FALSE, 0);
	gtk_widget_set_sensitive(GTK_WIDGET(radio_box), FALSE);
	radio_p->box = radio_box;


	/*GtkWidget *label = gtk_label_new(_("modified"));
	gtk_box_pack_start (GTK_BOX (radio_box), label, TRUE, FALSE, 0);
	gtk_widget_show (label);*/
	GtkBox *box=gtk_c::vboxNew (FALSE, 0);
	gtk_widget_show (GTK_WIDGET(box));
	gtk_box_pack_start (radio_box, GTK_WIDGET(box), TRUE, FALSE, 0);

	gtk_box_pack_start (box, GTK_WIDGET(radio1), TRUE, FALSE, 0);
	gtk_box_pack_start (box, GTK_WIDGET(radio2), TRUE, FALSE, 0);
	gtk_box_pack_start (box, GTK_WIDGET(radio3), TRUE, FALSE, 0);

	/*label = gtk_label_new(_("within the last"));
	gtk_box_pack_start (GTK_BOX (radio_box), label, TRUE, FALSE, 0);
	gtk_widget_show (label);*/
	return radio_p;
    }


    GtkToggleButton *
    add_option_entry(GtkBox *options_vbox, 
	    const gchar *check_name,
	    const gchar *entry_name,
	    const gchar *text,
	    const gchar *default_value)
    {
	if ((!entry_name && !check_name)|| !options_vbox || !dialog_) {
	    DBG("add_option_entry(): incorrect function call\n");
	    return NULL;
	}
	GtkBox *hbox = gtk_c::hboxNew (FALSE, 0);
	gtk_widget_show (GTK_WIDGET(hbox));
	gtk_box_pack_start (options_vbox, GTK_WIDGET(hbox), TRUE, FALSE, 0);

	GtkBox *size_hbox = gtk_c::hboxNew (FALSE, 0);
	GtkCheckButton *check = NULL;
	if (check_name) {
	    check = GTK_CHECK_BUTTON(gtk_check_button_new());
	    gtk_widget_show (GTK_WIDGET(check));
	    g_object_set_data(G_OBJECT(dialog_), check_name, check);
	    gtk_box_pack_start (hbox, GTK_WIDGET(check), FALSE, FALSE, 0);
	    g_signal_connect (G_OBJECT (check), "toggled", 
		    G_CALLBACK (TB_CALLBACK(signals::sensitivize)), 
		    size_hbox);
	    gtk_widget_set_sensitive(GTK_WIDGET(size_hbox), FALSE);
	}
	gtk_widget_show (GTK_WIDGET(size_hbox));
	gtk_box_pack_start (hbox, GTK_WIDGET(size_hbox), FALSE, FALSE, 0);

	if (text) {
	    GtkLabel *label = GTK_LABEL(gtk_label_new (text));
	    gtk_widget_show (GTK_WIDGET(label));
	    gtk_box_pack_start (size_hbox, GTK_WIDGET(label), TRUE, FALSE, 0);
	}
	
	if (entry_name) {
	    GtkLabel *label = GTK_LABEL(gtk_label_new (": "));
	    gtk_widget_show (GTK_WIDGET(label));
	    gtk_box_pack_start (size_hbox, GTK_WIDGET(label), FALSE, FALSE, 0);
	    
	    GtkEntry *entry = GTK_ENTRY(gtk_entry_new());
	    gtk_widget_show (GTK_WIDGET(entry));
	    gtk_box_pack_start (size_hbox, GTK_WIDGET(entry), TRUE, TRUE, 0);
	    g_object_set_data(G_OBJECT(dialog_), entry_name, entry);
	    gtk_entry_set_text (entry, default_value);                          
	}

	if (check) return GTK_TOGGLE_BUTTON(check);
	return NULL;
    }


    GtkToggleButton *
    add_option_radio2(
	    GtkBox *options_vbox, 
	    const gchar *check_name,
	    const gchar *radio1_name,
	    const gchar *radio2_name,
	    const gchar *text1,
	    const gchar *text2)
    {
	if ((!radio1_name  && !check_name)|| !options_vbox || !dialog_) {
	    DBG("add_option_radio2(): incorrect function call\n");
	    return NULL;
	}
	GtkBox *hbox = gtk_c::hboxNew (FALSE, 0);
	gtk_widget_show (GTK_WIDGET(hbox));
	gtk_box_pack_start (GTK_BOX (options_vbox), GTK_WIDGET(hbox), TRUE, FALSE, 0);

	GtkBox *size_hbox = gtk_c::hboxNew (FALSE, 0);
	GtkCheckButton *check = NULL;
	if (check_name) {
	    check = GTK_CHECK_BUTTON(gtk_check_button_new());
	    gtk_widget_show (GTK_WIDGET(check));
	    g_object_set_data(G_OBJECT(dialog_), check_name, check);
	    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET(check), FALSE, FALSE, 0);
	    g_signal_connect (G_OBJECT (check), "toggled", 
		     G_CALLBACK (TB_CALLBACK(signals::sensitivize)), size_hbox);
	    gtk_widget_set_sensitive(GTK_WIDGET(size_hbox), FALSE);
	}
	gtk_widget_show (GTK_WIDGET(size_hbox));
	gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET(size_hbox), FALSE, FALSE, 0);

	if (text1 && radio1_name) {
	    GtkRadioButton *radio1 = GTK_RADIO_BUTTON(gtk_radio_button_new_with_label (NULL, text1));
	    gtk_widget_show (GTK_WIDGET(radio1));
	    gtk_box_pack_start (GTK_BOX (size_hbox), GTK_WIDGET(radio1), TRUE, TRUE, 0);
	    g_object_set_data(G_OBJECT(dialog_), radio1_name, radio1);
	    if (text2 && radio2_name) {
		GtkRadioButton *radio2 = GTK_RADIO_BUTTON(gtk_radio_button_new_with_label_from_widget(radio1, text2));
		gtk_widget_show (GTK_WIDGET(radio2));
		gtk_box_pack_start (GTK_BOX (size_hbox), GTK_WIDGET(radio2), TRUE, TRUE, 0);
		g_object_set_data(G_OBJECT(dialog_), radio2_name, radio2);
	    }
	}

	if (check) return GTK_TOGGLE_BUTTON(check);
	return NULL;
    }

    GtkToggleButton *
    add_option_spin(
	    GtkBox *options_vbox, 
	    const gchar *check_name,
	    const gchar *spin_name,
	    const gchar *text,
	    gint default_value)
    {
	/*GtkIconInfo *icon_info =
	    gtk_icon_theme_lookup_icon (gtk_icon_theme_get_default(),
				"list-add-symbolic",
				GTK_ICON_SIZE_MENU,
				0);*/
	// gtk bug workaround. if no icotheme in use, plus/minus icons cannot be loaded.
	//if (icon_info) {
    /*    if (!getenv("RFM_USE_GTK_ICON_THEME") || !strlen(getenv("RFM_USE_GTK_ICON_THEME"))) {
	    gchar *def_val = g_strdup_printf("%d", default_value);
	    GtkToggleButton *t = add_option_entry(options_vbox, check_name, spin_name, text, def_val);
	    g_free(def_val);
	    return t;
	}*/
	//if (icon_info) g_object_unref(icon_info);

	if ((!spin_name && !check_name)|| !options_vbox || !dialog_) {
	    DBG("add_option_spin(): incorrect function call\n");
	    return NULL;
	}
	GtkBox *hbox = gtk_c::hboxNew (FALSE, 0);
	gtk_widget_show (GTK_WIDGET(hbox));
	gtk_box_pack_start (options_vbox, GTK_WIDGET(hbox), TRUE, FALSE, 0);

	GtkBox *size_hbox = gtk_c::hboxNew (FALSE, 0);
	GtkCheckButton *check = NULL;
	if (check_name) {
	    check = GTK_CHECK_BUTTON(gtk_check_button_new());
	    gtk_widget_show (GTK_WIDGET(check));
	    g_object_set_data(G_OBJECT(dialog_), check_name, (gpointer)check);
	    gtk_box_pack_start (hbox, GTK_WIDGET(check), FALSE, FALSE, 0);
	    g_signal_connect (G_OBJECT (check), "toggled", 
		    G_CALLBACK (TB_CALLBACK(signals::sensitivize)), (gpointer)size_hbox);
	    gtk_widget_set_sensitive(GTK_WIDGET(size_hbox), FALSE);
	}
	gtk_widget_show (GTK_WIDGET(size_hbox));
	gtk_box_pack_start (hbox, GTK_WIDGET(size_hbox), FALSE, FALSE, 0);

	if (text) {
	    GtkLabel *label = GTK_LABEL(gtk_label_new (text));
	    gtk_widget_show (GTK_WIDGET(label));
	    gtk_box_pack_start (size_hbox, GTK_WIDGET(label), TRUE, FALSE, 0);
	}
	
	if (spin_name) {
	    GtkLabel *label = GTK_LABEL(gtk_label_new (": "));
	    gtk_widget_show (GTK_WIDGET(label));
	    gtk_box_pack_start (size_hbox, GTK_WIDGET(label), FALSE, FALSE, 0);
	    GtkAdjustment *spinbutton_adj = GTK_ADJUSTMENT(gtk_adjustment_new (default_value, 0, 4096*4096, 1, 64, 0));
	    GtkSpinButton *spinbutton = GTK_SPIN_BUTTON(gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton_adj), 0, 0));
	    gtk_widget_show (GTK_WIDGET(spinbutton));
	    gtk_box_pack_start (size_hbox, GTK_WIDGET(spinbutton), TRUE, TRUE, 0);
	    gtk_spin_button_set_update_policy (spinbutton, GTK_UPDATE_IF_VALID);
	    g_object_set_data(G_OBJECT(dialog_), spin_name, (gpointer)spinbutton);
	    gtk_widget_set_size_request (GTK_WIDGET(spinbutton), 75, -1);
	}

	if (check) return GTK_TOGGLE_BUTTON(check);
	return NULL;
    }



    GtkToggleButton *
    add_option_combo(
	    GtkBox *options_vbox, 
	    const gchar *check_name,
	    const gchar *combo_name,
	    const gchar *text,
	    GSList *list)
    {
	if ((!combo_name && !check_name)|| !options_vbox || !dialog_) {
	    DBG("add_option_spin(): incorrect function call\n");
	    return NULL;
	}
	GtkBox *hbox = gtk_c::hboxNew (FALSE, 0);
	gtk_widget_show (GTK_WIDGET(hbox));
	gtk_box_pack_start (options_vbox, GTK_WIDGET(hbox), TRUE, FALSE, 0);

	GtkBox *size_hbox = gtk_c::hboxNew (FALSE, 0);
	GtkCheckButton *check = NULL;
	if (check_name) {
	    check = GTK_CHECK_BUTTON(gtk_check_button_new());
	    gtk_widget_show (GTK_WIDGET(check));
	    g_object_set_data(G_OBJECT(dialog_), check_name, check);
	    gtk_box_pack_start (hbox, GTK_WIDGET(check), FALSE, FALSE, 0);
	    g_signal_connect (G_OBJECT (check), "toggled", 
		    G_CALLBACK (TB_CALLBACK(signals::sensitivize)), size_hbox);
	    gtk_widget_set_sensitive(GTK_WIDGET(size_hbox), FALSE);
	}
	gtk_widget_show (GTK_WIDGET(size_hbox));
	gtk_box_pack_start (hbox, GTK_WIDGET(size_hbox), FALSE, FALSE, 0);

	if (text) {
	    GtkLabel *label = GTK_LABEL(gtk_label_new (text));
	    gtk_widget_show (GTK_WIDGET(label));
	    gtk_box_pack_start (size_hbox, GTK_WIDGET(label), TRUE, FALSE, 0);
	    label = GTK_LABEL(gtk_label_new (": "));
	    gtk_widget_show (GTK_WIDGET(label));
	    gtk_box_pack_start (size_hbox, GTK_WIDGET(label), FALSE, FALSE, 0);
       }

	if (combo_name) {
	    GtkListStore *usermodel=gtk_list_store_new (1, G_TYPE_STRING);
	    setStoreDataFromList (usermodel, &list);

	    GtkComboBox *combo = GTK_COMBO_BOX(gtk_combo_box_new_with_entry());
	    gtk_combo_box_set_model (combo,GTK_TREE_MODEL(usermodel));
	    gtk_combo_box_set_entry_text_column (combo,0);
	    
	    GtkEntry *entry  = GTK_ENTRY (gtk_bin_get_child(GTK_BIN(combo)));
	    gtk_entry_set_text (entry, (const gchar *)list->data);
	    gtk_widget_show(GTK_WIDGET(combo));
	    gtk_box_pack_start (size_hbox, GTK_WIDGET(combo), TRUE, TRUE, 0);
	    g_object_set_data(G_OBJECT(dialog_), combo_name, combo);
	    gtk_widget_set_size_request (GTK_WIDGET(combo), 120, -1);
	}
	if (check) return GTK_TOGGLE_BUTTON(check);
	return NULL;
    }

    GSList *
    free_string_slist(GSList *slist){
	GSList *tmp;
	for (tmp=slist; tmp && tmp->data; tmp=tmp->next){
	    g_free(tmp->data);
	}
	g_slist_free(slist);
	return NULL;
    }
	    
    void
    setStoreDataFromList(GtkListStore *list_store, GSList **list){
	GtkTreeIter iter;
	gtk_list_store_clear (list_store);
	GSList *p = *list;
	for (; p && p->data; p=p->next) {
	    gtk_list_store_append (list_store, &iter);
	    gtk_list_store_set (list_store, &iter,
			      0, (gchar *)p->data,
			      -1);
	  /* Note: The store will keep a copy of the string internally, 
	   * so the list may be freed */
	}
    }

// default values:
gint result_limit=256;
gint size_greater=16;
gint size_smaller=1024;
gint last_minutes=60;
gint last_hours=2;
gint last_days=7;
gint last_months=2;
gboolean default_recursive=TRUE;
gboolean default_recursiveH=FALSE;
gboolean default_xdev=TRUE;
gboolean default_case_sensitive=FALSE;
gboolean default_ext_regexp=FALSE;
gboolean default_look_in_binaries=FALSE;
gboolean default_line_count=FALSE;
gint default_type_index=0;
gboolean default_anywhere=TRUE;
gboolean default_match_words=FALSE;
gboolean default_match_lines=FALSE;
gboolean default_match_no_match=FALSE;
GSList *find_list = NULL;
gchar  *last_workdir = NULL;
    
//gboolean have_grep = FALSE;

const gchar *
filter_text_help=
	        N_("Basic rules:\n" "\n"
                          "*  Will match any character zero or more times.\n"
                          "?  Will match any character exactly one time\n"
                          "[] Match any character within the [] \n"
                          "^  Match at beginning of string\n" 
			  "$  Match at end of string \n");
const gchar *
grep_text_help=
		N_("Reserved characters for extended regexp are\n"
                          ". ^ $ [ ] ? * + { } | \\ ( ) : \n"
                          "In  basic regular expressions the metacharacters\n"
                          "?, +, {, |, (, and ) lose their special meaning.\n"
                          "\n"
                          "The  period  .   madirenttches  any  single  character.\n"
                          "The caret ^ matches at the start of line.\n"
                          "The dollar $ matches at the end of line.\n" "\n"
                          "Characters within [ ] matches any single \n"
                          "       character in the list.\n"
                          "Characters within [^ ] matches any single\n"
                          "       character *not* in the list.\n"
                          "Characters inside [ - ] matches a range of\n"
                          "       characters (ie [0-9] or [a-z]).\n" "\n"
                          "A regular expression may be followed by one\n"
                          "       of several repetition operators:\n"
                          "?      The preceding item is optional and matched\n"
                          "       at most once.\n"
                          "*      The preceding item will be matched zero\n"
                          "       or more times.\n"
                          "+      The preceding item will be matched one or\n"
                          "       more times.\n"
                          "{n}    The preceding item is matched exactly n times.\n"
                          "{n,}   The preceding item is matched n or more times.\n"
                          "{n,m}  The preceding item is matched at least n times,\n"
                          "       but not more than m times.\n" "\n"
                          "To match any reserved character, precede it with \\. \n"
                          "\n"
                          "Two regular expressions may be joined by the logical or\n"
                          "       operator |.\n"
                          "Two regular expressions may be concatenated.\n" "\n"
                          "More information is available by typing \"man grep\"\n"
			  );

};

#if 0
template <class Type>
class xffind: public xffindGUI<Type>
{
public:
    xffind(const char *path){
    }
    xffind(void){
    }
    void helpFilter (GtkToggleButton * button) {
	GtkTextView *textView=GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(window), "helpTextView"));
	clearText(textView);
	if (gtk_toggle_button_get_active (button)) {
	    showText(textView); 
	    printTag("blue", _(filter_text_help), NULL);
	    scrollTop(textView);
	} 
    }

    void clearText (GtkTextView *textView) {
	if (!textView){
	    std::cerr<<"DBG> clearText(): textView is null\n";
	    return;
	}
	GtkTextIter start, end;
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (textView);
	gtk_text_buffer_get_bounds (buffer, &start, &end);
	gtk_text_buffer_delete (buffer, &start, &end);
	GtkPaned *vpane_ = GTK_PANED(g_object_get_data(G_OBJECT(textView), "vpane_"));
        hidePane (vpane_);
    }
    void hidePane (GtkPaned *vpane_) {
	if(!vpane_){
	    std::cerr<<"DBG> hidePane(): vpane_ is null\n";
	    return ;
	}
	GtkAllocation allocation;
	gtk_widget_get_allocation (vpane_, &allocation);
	gtk_paned_set_position (GTK_PANED (vpane_), 10000);
    }

    void showText (GtkTextView *textView) {
	if(!textView){
	    std::cerr<<"DBG> showText(): textView is null\n";
	    return ;
	}
	GtkPaned *vpane_ = GTK_PANED(g_object_get_data(G_OBJECT(textView), "vpane_"));
	if(!vpane_){
	    std::cerr<<"DBG> showText(): vpane_ is null\n";
	    return ;
	}
	GtkAllocation allocation;
	gtk_widget_get_allocation (vpane_, &allocation);
	if (allocation.height <= 50) {
	    std::cerr<<"DBG> showText(): allocation.height <= 50\n";
	    return ;
	}
	gdouble position = gtk_paned_get_position (vpane_);
	if(position > allocation.height * 0.90) {
	    gtk_paned_set_position (vpane_, allocation.height * 0.75);
	}
	return ;
    }


private:
    GtkWidget *fill_string_option_menu (GtkComboBox *om, GSList * strings);
    GtkWidget *create_find_dialog (void);
};
#endif
} // namespace xf
#endif
