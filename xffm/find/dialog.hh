#ifndef XFDIALOG__HH
# define XFDIALOG__HH
#include "signals.hh"
typedef struct opt_t{
  const char *text;
  const char *id;
  gboolean defaultValue;
} opt_t;

namespace xf
{


template <class Type>
class FindDialog
{
    using util_c = Util<double>;
    using gtk_c = Gtk<double>;
    using tooltip_c = Tooltip<double>;
//    typedef typename  void (*)(GtkToggleButton *,gpointer) toggleButtonCallback;
protected:
    void createDialog(const gchar *path){
        createDialog_(path);
    }
    
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
            if(fgets (line, 255, pipe) == NULL){
                // Definitely not GNU grep. BSD version.
                TRACE("pipe for \"grep --version\"\n"); 
                TRACE ("fgets: %s\n", strerror (errno));
            } else {
                if(strstr (line, "GNU")) gnuGrep_ = TRUE;
            }
            pclose (pipe);
        }
        g_free (grep);
        return TRUE;
    }
    
private:
    GtkBox *topPaneHbox_;
    GtkBox *topPaneVbox_;
    GtkBox *mainVbox_;
    
    gboolean gnuGrep_;
    GdkGeometry geometry_;
    
    GtkWidget *
    fill_string_option_menu (GtkComboBox *om, const gchar *ftypes[]) {
        // Create GSList.
        GSList *strings = NULL;    
        for(int i = 0; ftypes[i] != NULL; i++) {
            strings = g_slist_append (strings, g_strdup(_(ftypes[i])));
        }
        //Fill her up.
        GSList *p;
        for (p=strings; p && p->data; p=p->next) {
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(om), (const gchar *)p->data);
          /* Note: The model will keep a copy of the string internally, 
           * so the list may be freed */
        }
        gtk_combo_box_set_active(om, default_type_index);

        for(p=strings; p && p->data; p=p->next) {
            g_free(p->data);
        }
        g_slist_free(strings);
        return (GtkWidget *)om;
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
        gtk_window_set_geometry_hints (findDialog, GTK_WIDGET(findDialog), &geometry_, GDK_HINT_MAX_SIZE);
    }
    
    void mkDialog(void){
        findDialog = GTK_WINDOW(gtk_window_new (GTK_WINDOW_TOPLEVEL));
        
        gtk_window_set_type_hint(findDialog, GDK_WINDOW_TYPE_HINT_DIALOG);
        //gtk_window_set_position (findDialog, GTK_WIN_POS_MOUSE);
        
        setWindowMaxSize();
        //g_object_set_data(G_OBJECT(findDialog), "window", findDialog);
        gtk_window_set_title (findDialog, _("Find"));
        //gtk_window_set_position (findDialog, GTK_WIN_POS_MOUSE);

        GdkPixbuf *pixbuf = Pixbuf<Type>::getPixbuf("edit-find", SIZE_ICON);
        gtk_window_set_icon (findDialog, pixbuf);
        g_object_unref(pixbuf);
        
        gtk_window_present (findDialog);
        while (gtk_events_pending()) gtk_main_iteration();
      
    }

    void mkTopPaneHbox(){
        topPaneHbox_= gtk_c::hboxNew(FALSE, 0);
        gtk_widget_set_hexpand(GTK_WIDGET(topPaneHbox_), TRUE);

        auto vbox1 = gtk_c::vboxNew(FALSE, 0);
        auto vbox2 = gtk_c::vboxNew(FALSE, 0);
        auto vbox3 = gtk_c::vboxNew(FALSE, 0);

        ////////////   closeButton... /////////////////////////
        auto closeButton =  gtk_c::dialog_button(WINDOW_CLOSE, "");
        g_object_set_data(G_OBJECT(findDialog), "close_button", closeButton);
        g_object_set_data(G_OBJECT(closeButton), "findDialog", findDialog);
        g_signal_connect (G_OBJECT (closeButton), "clicked",
                BUTTON_CALLBACK(Type::onCloseButton), (gpointer)findDialog);
        compat<bool>::boxPack0 (vbox2, GTK_WIDGET(closeButton), TRUE, TRUE, 0);
        auto h = g_strconcat(_("Close find"), "\n\n", NULL); 
        tooltip_c::custom_tooltip(GTK_WIDGET(closeButton), NULL, h);
        g_free(h);

        ////////////   findButton... /////////////////////////
        auto findButton =  gtk_c::dialog_button(SEARCH, "");
        gtk_widget_set_can_default(GTK_WIDGET(findButton), TRUE);
        g_signal_connect (G_OBJECT (findButton), "clicked",
                BUTTON_CALLBACK(Type::onFindButton), (gpointer)findDialog);
        compat<bool>::boxPack0 (vbox3, GTK_WIDGET(findButton), TRUE, TRUE, 0);
        auto h2 = g_strconcat(_("Search Action"), "\n\n", NULL); 
        tooltip_c::custom_tooltip(GTK_WIDGET(findButton), NULL, h2);
        g_free(h2);

        ////////////   advanced options... /////////////////////////
        auto advancedDialog = advancedOptions();
        gtk_window_set_transient_for(GTK_WINDOW(advancedDialog), GTK_WINDOW(findDialog));
       


        auto advancedButton = gtk_c::toggle_button(DOCUMENT_PROPERTIES, NULL);
//        auto advancedButton = gtk_c::toggle_button(NULL, _("Details"));
        auto h3 = g_strconcat(_("Details"), "\n\n", NULL); 
        tooltip_c::custom_tooltip(GTK_WIDGET(advancedButton), NULL, h3);
        g_free(h3);
        g_object_set_data(G_OBJECT(findDialog), "advancedButton", advancedButton);
        g_object_set_data(G_OBJECT(findDialog), "advancedDialog", advancedDialog);
        compat<bool>::boxPack0 (vbox1, GTK_WIDGET(advancedButton), TRUE, TRUE, 5);
        g_signal_connect (advancedButton,
                          "clicked", WIDGET_CALLBACK(Type::onDetails), 
                          (gpointer)findDialog);

        auto t=g_strdup_printf("<span color=\"blue\" size=\"large\"><b>%s</b></span>  ", _("Find Files"));
        auto title = GTK_LABEL(gtk_label_new (t));
        gtk_label_set_use_markup (title, TRUE);
        g_free(t);
        compat<bool>::boxPack0 (topPaneHbox_, GTK_WIDGET(vbox1), FALSE, FALSE, 0);
        compat<bool>::boxPack0 (topPaneHbox_, GTK_WIDGET(title), TRUE, TRUE, 0);
        compat<bool>::boxPack0 (topPaneHbox_, GTK_WIDGET(vbox3), FALSE, FALSE, 0);
        compat<bool>::boxPack0 (topPaneHbox_, GTK_WIDGET(vbox2), FALSE, FALSE, 0);


    }
    
    GtkPaned *mkVpane(void){

        mainVbox_ = gtk_c::vboxNew(FALSE, 0);
        topPaneVbox_ = gtk_c::vboxNew(FALSE, 2);
        gtk_widget_set_vexpand(GTK_WIDGET(topPaneVbox_), FALSE);
        
        mkTopPaneHbox();
        compat<bool>::boxPack0 (topPaneVbox_, GTK_WIDGET(topPaneHbox_), FALSE, TRUE, 0);

        auto vpane = GTK_PANED(gtk_paned_new(GTK_ORIENTATION_VERTICAL));
        

        gtk_container_add (GTK_CONTAINER (findDialog), GTK_WIDGET(mainVbox_));


        // Cursor does not change over handle in OpenBSD,
        // so now we default to wide handle.
        gtk_paned_set_wide_handle (vpane,TRUE);
        
        g_object_set_data(G_OBJECT(findDialog), "vpane", (gpointer)vpane);
        compat<bool>::boxPack0 (mainVbox_, GTK_WIDGET(vpane), TRUE, TRUE, 0);
        // hack: widgets_p->paper = findDialog;
        gtk_container_set_border_width (GTK_CONTAINER (topPaneVbox_), 5);

        auto sw = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new(NULL, NULL));
        gtk_paned_pack1 (GTK_PANED (vpane), GTK_WIDGET (sw), FALSE, TRUE);
        gtk_container_add(GTK_CONTAINER(sw), GTK_WIDGET(topPaneVbox_));
        gtk_scrolled_window_set_policy (sw, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

        auto diagnostics = GTK_TEXT_VIEW(gtk_text_view_new ());
        g_object_set_data(G_OBJECT(findDialog), "diagnostics", (gpointer)diagnostics);
        g_object_set_data(G_OBJECT(diagnostics), "vpane", (gpointer)vpane);
        
        auto scrolledwindow = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new (NULL, NULL));
        gtk_paned_pack2 (GTK_PANED (vpane), GTK_WIDGET(scrolledwindow), FALSE, TRUE);
        //gtk_paned_pack2 (GTK_PANED (vpane), GTK_WIDGET(scrolledwindow), TRUE, TRUE);
        gtk_scrolled_window_set_policy (scrolledwindow, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

        gtk_container_add (GTK_CONTAINER (scrolledwindow), GTK_WIDGET(diagnostics));
        gtk_container_set_border_width (GTK_CONTAINER (diagnostics), 2);
        gtk_widget_set_can_focus(GTK_WIDGET(diagnostics), FALSE);
        gtk_text_view_set_wrap_mode (diagnostics, GTK_WRAP_WORD);
        gtk_text_view_set_cursor_visible (diagnostics, FALSE);


         return vpane;
    }

    GtkWidget *advancedOptions(void){
        auto advancedBox = gtk_c::vboxNew (FALSE, 0);
        gtk_widget_set_vexpand(GTK_WIDGET(advancedBox), FALSE);
        auto topHbox = gtk_c::hboxNew (FALSE, 0);
        auto t = g_strconcat("<span color=\"red\" size=\"large\">",_("Select advanced options"),"</span>",NULL);
        auto label = GTK_LABEL(gtk_label_new(""));
        gtk_label_set_markup(label, t);

        compat<bool>::boxPack0 (advancedBox, GTK_WIDGET(topHbox), TRUE, TRUE, 0);
        compat<bool>::boxPack0 (topHbox, GTK_WIDGET(label), TRUE, TRUE, 0);

        auto closeButton = gtk_c::dialog_button(WINDOW_CLOSE, "");
        auto vBox = gtk_c::vboxNew (FALSE, 0);
        compat<bool>::boxPack0 (vBox, GTK_WIDGET(closeButton), TRUE, TRUE, 0);

        compat<bool>::boxPack0 (topHbox, GTK_WIDGET(vBox), FALSE, FALSE, 0);

        g_signal_connect(G_OBJECT (closeButton), "clicked", 
                BUTTON_CALLBACK(Type::onCloseDetails), (void *)findDialog);

        auto hbox17 = gtk_c::hboxNew (FALSE, 0);
        compat<bool>::boxPack0 (advancedBox, GTK_WIDGET(hbox17), TRUE, FALSE, 0);

        auto left_options_vbox = gtk_c::vboxNew (FALSE, 0);
        compat<bool>::boxPack0 (hbox17, GTK_WIDGET(left_options_vbox), FALSE, FALSE, 0);
        auto center_options_vbox = gtk_c::vboxNew (FALSE, 0);
        compat<bool>::boxPack0 (hbox17, GTK_WIDGET(center_options_vbox), FALSE, FALSE, 0);
        auto right_options_vbox = gtk_c::vboxNew (FALSE, 0);
        compat<bool>::boxPack0 (hbox17, GTK_WIDGET(right_options_vbox), FALSE, FALSE, 0);

        /// option -r "recursive"
        gtk_toggle_button_set_active(add_option_spin(left_options_vbox, "recursive", NULL, _("Recursive"), 0), default_recursive);

        /// option -D "recursiveH"
        gtk_toggle_button_set_active(add_option_spin(left_options_vbox, "recursiveH", NULL, _("Find hidden files and directories"), 0), default_recursiveH);

        /// option -a "xdev"
        gtk_toggle_button_set_active(add_option_spin(left_options_vbox, "xdev", NULL, _("Stay on single filesystem"), 0), default_xdev);

        /// option "upper_limit_spin" (only in gtk findDialog)
        gchar *text = g_strdup_printf("%s (%s)", _("Results"), _("Upper limit"));
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

        auto slist = get_user_slist();
        // option -u uid "uid" "uid_combo"
        add_option_combo(center_options_vbox, "uid", "uid_combo", _("User"), slist);
        slist = free_string_slist(slist);

        // option -g gid "gid" "gid_combo"
        slist = get_group_slist();
        add_option_combo(center_options_vbox, "gid", "gid_combo", _("Group"), slist);
        slist = free_string_slist(slist);
        
        // option -o octal "octal_p" "permissions_entry"
        add_option_entry(center_options_vbox, "octal_p", "permissions_entry", _("Octal Permissions"), "0666");
        auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(findDialog), "permissions_entry"));
        gtk_widget_set_size_request (GTK_WIDGET(entry), 75, -1);
        
        // option -p suid | exe 
        add_option_radio2(center_options_vbox, "suidexe", "suid_radio", "exe_radio", _("SUID"), _("Executable"));

        
        // option -M -A -C
        radio_t *radio_p = create_radios(right_options_vbox);
        // radio_p freed on destroy event for findDialog.

        // option -k minutes "last_minutes", "last_minutes_spin"
        radio_p->toggle[0] = add_option_spin(right_options_vbox, "last_minutes", "last_minutes_spin", _("Minutes"), last_minutes);
        g_signal_connect (G_OBJECT (radio_p->toggle[0]), "toggled", 
                BUTTON_CALLBACK(Type::sensitivize_radio), radio_p);
        
       // option -h hours "last_hours", "last_hours_spin"
        radio_p->toggle[1] = add_option_spin(right_options_vbox, "last_hours", "last_hours_spin", _("Hours"), last_hours);
        g_signal_connect (G_OBJECT (radio_p->toggle[1]), "toggled", 
            BUTTON_CALLBACK(Type::sensitivize_radio), radio_p);
        
        // option -d days "last_days", "last_days_spin"
        radio_p->toggle[2] = add_option_spin(right_options_vbox, "last_days", "last_days_spin", _("Days"), last_days);
            //compat<bool>::boxPack0 (GTK_BOX (topPaneHbox), GTK_WIDGET(check), FALSE, FALSE, 0);
            g_signal_connect (G_OBJECT (radio_p->toggle[2]), "toggled", 
                BUTTON_CALLBACK(Type::sensitivize_radio), radio_p);
        
        // option -m months "last_months", "last_months_spin"
        radio_p->toggle[3] = add_option_spin(right_options_vbox, "last_months", "last_months_spin", _("Months"), last_months);
        g_signal_connect (G_OBJECT (radio_p->toggle[3]), "toggled", 
            BUTTON_CALLBACK(Type::sensitivize_radio), radio_p);
        
         ///////////

     
        auto hbox21 = gtk_c::hboxNew (FALSE, 0);
        compat<bool>::boxPack0 (left_options_vbox, GTK_WIDGET(hbox21), TRUE, FALSE, 0);

        auto label37 = GTK_LABEL(gtk_label_new (_("File type : ")));
        compat<bool>::boxPack0 (hbox21, GTK_WIDGET(label37), FALSE, FALSE, 0);

        auto file_type_om =  GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
        compat<bool>::boxPack0 (hbox21, GTK_WIDGET(file_type_om), TRUE, TRUE, 0);
        g_object_set_data(G_OBJECT(findDialog), "file_type_om", file_type_om);


        fill_string_option_menu (GTK_COMBO_BOX(file_type_om), ftypes);

        

        gtk_widget_show_all(GTK_WIDGET(advancedBox));
        auto window = GTK_DIALOG(gtk_dialog_new());
        gtk_window_set_title(GTK_WINDOW(findDialog), "xffm --find");
            
        auto contentArea = GTK_BOX(gtk_dialog_get_content_area (window));
        compat<bool>::boxPack0 (contentArea, GTK_WIDGET(advancedBox), FALSE, FALSE, 0);
        gtk_widget_show_all(GTK_WIDGET(contentArea));


        return GTK_WIDGET(window);
    }

    void mkPathEntry(const gchar *path){
        GtkWidget *path_label;
        gchar *default_path=NULL;
        if (path) default_path = g_strdup(path);
        auto path_box = gtk_c::hboxNew (FALSE, 0);
        gtk_widget_set_hexpand(GTK_WIDGET(path_box), TRUE);
        compat<bool>::boxPack0 (GTK_BOX (topPaneVbox_), GTK_WIDGET(path_box), FALSE, TRUE, 0);

        auto t=g_strdup_printf("%s:", _("Path"));
        path_label = gtk_label_new (t);
        g_free(t);

        auto historyPath = g_build_filename(PATH_HISTORY);
        auto path_entry = mkCompletionEntry(historyPath);
        //gtk_widget_set_size_request (GTK_WIDGET(path_entry), 50, -1);
        g_object_set_data(G_OBJECT(findDialog), "path_entry", path_entry);
        g_free(historyPath);
        if (default_path){
            gtk_entry_set_text(path_entry, default_path);
            g_free(default_path);
        }
        auto button = gtk_c::dialog_button ("folder-symbolic", NULL);
        auto h = g_strconcat(_("Display file selection dialog"), "\n\n", NULL); 
        tooltip_c::custom_tooltip(GTK_WIDGET(button), NULL, h);
        g_free(h);
        auto vbox = gtk_c::vboxNew (FALSE, 6);
        compat<bool>::boxPack0 (vbox, GTK_WIDGET(button), FALSE, FALSE, 0);
        compat<bool>::boxPack0 (path_box, GTK_WIDGET(path_label), FALSE, FALSE, 0);
        compat<bool>::boxPack0 (path_box, GTK_WIDGET(path_entry), TRUE, TRUE, 0);
        compat<bool>::boxPack0 (path_box, GTK_WIDGET(vbox), FALSE, FALSE, 0);
        g_object_set_data(G_OBJECT(findDialog), "fileselector", button);
        
        g_signal_connect (G_OBJECT(button), 
                        "clicked", BUTTON_CALLBACK (ChooserResponse<Type>::folderChooser), 
                        (gpointer) path_entry);

        g_signal_connect (path_entry,
                          "activate", BUTTON_CALLBACK(Type::onFindButton), 
                          (gpointer)findDialog);


    }

    void mkFilterEntry(void){
        auto filter_box = gtk_c::hboxNew (FALSE, 0);
        compat<bool>::boxPack0 (topPaneVbox_, GTK_WIDGET(filter_box), FALSE, TRUE, 5);

        auto text=g_strdup_printf("%s ", _("Filter:"));
        auto filter_label = GTK_LABEL(gtk_label_new (text));
        g_free(text);


        auto history = g_build_filename(FILTER_HISTORY);
        auto filter_entry = mkCompletionEntry(history);
        g_object_set_data(G_OBJECT(findDialog), "filter_entry", (gpointer)filter_entry);
        g_free(history);



        auto dialogbutton2 = gtk_c::dialog_button("dialog-question-symbolic", "");
        g_object_set_data(G_OBJECT(dialogbutton2), "findDialog", findDialog);
        tooltip_c::custom_tooltip(GTK_WIDGET(dialogbutton2), NULL,  _(filter_text_help));
        g_signal_connect (dialogbutton2,
                          "clicked", WIDGET_CALLBACK(Type::on_buttonHelp), 
                          (gpointer)filter_text_help);
        
        auto vbox = gtk_c::vboxNew (FALSE, 6);
        compat<bool>::boxPack0 (vbox, GTK_WIDGET(dialogbutton2), FALSE, FALSE, 0);
        compat<bool>::boxPack0 (filter_box, GTK_WIDGET(filter_label), FALSE, FALSE, 0);
        compat<bool>::boxPack0 (filter_box, GTK_WIDGET(filter_entry), TRUE, TRUE, 0);
        compat<bool>::boxPack0 (filter_box, GTK_WIDGET(vbox), FALSE, FALSE, 0);
        g_signal_connect (filter_entry,
                          "activate", BUTTON_CALLBACK(Type::onFindButton), 
                          (gpointer)findDialog);

    }

    void mkGrepEntry(void){
        auto grep_box = gtk_c::hboxNew (FALSE, 0);
        compat<bool>::boxPack0 (topPaneVbox_, GTK_WIDGET(grep_box), FALSE, TRUE, 5);

        auto t=g_strdup_printf("%s: ",_("Contains the text"));
        auto grep_label = GTK_LABEL(gtk_label_new (t));
        g_free(t);



    
        auto history = g_build_filename(GREP_HISTORY);
        auto grep_entry = mkCompletionEntry(history);
        g_object_set_data(G_OBJECT(findDialog), "grep_entry", grep_entry);
        g_free(history);        
        gtk_widget_set_sensitive (GTK_WIDGET(grep_entry), TRUE);   
        
        auto button = gtk_c::dialog_button ("dialog-question-symbolic", "");
        g_object_set_data(G_OBJECT(button), "findDialog", findDialog);
        tooltip_c::custom_tooltip(GTK_WIDGET(button), NULL, _(grep_text_help));
        g_signal_connect (GTK_WIDGET(button),
                          "clicked", WIDGET_CALLBACK(Type::on_buttonHelp), 
                          (gpointer) _(grep_text_help));

        auto vbox = gtk_c::vboxNew (FALSE, 6);
        compat<bool>::boxPack0 (vbox, GTK_WIDGET(button), FALSE, FALSE, 0);
        compat<bool>::boxPack0 (grep_box, GTK_WIDGET(grep_label), FALSE, FALSE, 0);
        compat<bool>::boxPack0 (grep_box, GTK_WIDGET(grep_entry), TRUE, TRUE, 0);
        compat<bool>::boxPack0 (grep_box, GTK_WIDGET(vbox), FALSE, FALSE, 0);

        gboolean active_grep =  (gtk_entry_get_text(grep_entry) && strlen(gtk_entry_get_text(grep_entry))); 
  
        auto checkBox = gtk_c::vboxNew (FALSE, 0);
        opt_t options[] = {
          {_("Case Sensitive"),"case_sensitive",default_case_sensitive},
          {_("Extended regexp"), "ext_regexp",default_ext_regexp},
          {_("Include binary files"),"look_in_binaries",default_look_in_binaries},
          {_("Line Count"),"line_count",default_line_count},
          {NULL,NULL,0}};
        for (opt_t *p=options; p->text != NULL; p++) mkGrepOption(p, active_grep, grep_entry, checkBox);
        
           
        auto radioBox = gtk_c::vboxNew (FALSE, 0);
        GSList *radioGroup = NULL;
        opt_t rOptions[] = {
          {_("Anywhere"),"anywhere",default_anywhere},
          {_("Whole words only"), "match_words",default_match_words},
          {_("lines"),"match_lines",default_match_lines},
          {_("No match"),"match_no_match",default_match_no_match},
          {NULL,NULL,0}};
        for (opt_t *p=rOptions; p->text != NULL; p++) mkGrepRadio(p, active_grep, grep_entry, &radioGroup, radioBox);
        
        
        auto optionBox = gtk_c::hboxNew (FALSE, 10);
 
        compat<bool>::boxPack0 (optionBox, GTK_WIDGET(checkBox), FALSE, FALSE, 0);
        compat<bool>::boxPack0 (optionBox, GTK_WIDGET(radioBox), FALSE, FALSE, 0);
        compat<bool>::boxPack0 (topPaneVbox_, GTK_WIDGET(optionBox), FALSE, FALSE, 0);

    }
    void mkGrepOption(opt_t *opt, gboolean active_grep, GtkEntry *grep_entry, GtkBox *checkBox){
        auto check = GTK_CHECK_BUTTON(gtk_check_button_new_with_mnemonic (opt->text));
        gtk_widget_set_sensitive (GTK_WIDGET(check), active_grep);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), opt->defaultValue);
        g_object_set_data(G_OBJECT(findDialog), opt->id, check);
        g_signal_connect (G_OBJECT (grep_entry), "event", 
                KEY_EVENT_CALLBACK (Type::on_key_release), (gpointer) check);
        compat<bool>::boxPack0 (checkBox, GTK_WIDGET(check), FALSE, FALSE, 0);
    }

    void mkGrepRadio(opt_t *opt, gboolean active_grep, GtkEntry *grep_entry, GSList **group, GtkBox *radioBox){
        auto radio = 
            GTK_RADIO_BUTTON (gtk_radio_button_new_with_mnemonic (*group, opt->text));
        *group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio));
        g_object_set_data(G_OBJECT(findDialog), opt->id, radio);
        gtk_widget_set_sensitive (GTK_WIDGET(radio), active_grep);

        if (opt->defaultValue) {
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), TRUE);
        }
        g_signal_connect (G_OBJECT (grep_entry), "event", 
                KEY_EVENT_CALLBACK (Type::on_key_release), (gpointer) radio);
        compat<bool>::boxPack0 (radioBox, GTK_WIDGET(radio), FALSE, FALSE, 0);
    }

    void mkButtonBox(void){
        auto hbuttonbox2 = gtk_c::hboxNew (TRUE, 3);

        auto cancelButton =  gtk_c::dialog_button(PROCESS_STOP, "");
        g_object_set_data(G_OBJECT(findDialog), "cancel_button", cancelButton);
        g_object_set_data(G_OBJECT(cancelButton), "findDialog", findDialog);
        gtk_widget_set_sensitive(GTK_WIDGET(cancelButton), FALSE);
        g_signal_connect (G_OBJECT (cancelButton), "clicked",
                BUTTON_CALLBACK(Type::onCancelButton), (void *)findDialog);

        auto clearButton =  gtk_c::dialog_button(EDIT_CLEAR, "");
        g_object_set_data(G_OBJECT(findDialog), "clear_button", clearButton);
        g_object_set_data(G_OBJECT(clearButton), "findDialog", findDialog);
        g_signal_connect (G_OBJECT (clearButton), "clicked",
                BUTTON_CALLBACK(Type::onClearButton), g_object_get_data(G_OBJECT(findDialog), "diagnostics"));



        GtkButton *edit_button = NULL;
        auto editor = Util<Type>::getEditor();
        if (editor && strlen(editor)){
            auto basename = g_strdup(editor);
            if (strchr(basename, ' ')) *strchr(basename, ' ') = 0;
            auto editor_path = g_find_program_in_path(basename);
            if (editor_path){
                auto iconpath=g_strdup(basename);
                auto pix = Pixbuf<Type>::getPixbuf(iconpath, SMALL_ICON_SIZE);
                if (!pix) {
                    iconpath = g_strdup("document-open");
                    pix = Pixbuf<Type>::getPixbuf (iconpath, SMALL_ICON_SIZE); //refs
                }
                g_object_unref(pix);

                edit_button = 
                    gtk_c::dialog_button(iconpath, "");
//                    gtk_c::dialog_button(iconpath, _("Edit"));
                g_free(iconpath);
                g_object_set_data(G_OBJECT(findDialog), "edit_button", edit_button);
                g_object_set_data(G_OBJECT(edit_button), "findDialog", findDialog);
                g_signal_connect (G_OBJECT (edit_button), "clicked", 
                        BUTTON_CALLBACK (Type::onEditButton), (gpointer)findDialog);
                g_free(editor_path);
                gtk_widget_set_sensitive(GTK_WIDGET(edit_button), FALSE);
            } 
            g_free(basename);
        } else {
            TRACE("getEditor() = \"%s\"\n", editor);
        }

        compat<bool>::boxPack0 (hbuttonbox2, GTK_WIDGET(clearButton), FALSE, FALSE, 0);
        if (edit_button) compat<bool>::boxPack0 (hbuttonbox2, GTK_WIDGET(edit_button), FALSE, FALSE, 0);
        compat<bool>::boxPack0 (hbuttonbox2, GTK_WIDGET(cancelButton), FALSE, FALSE, 0);
        compat<bool>::boxPack0 (mainVbox_, GTK_WIDGET(hbuttonbox2), FALSE, TRUE, 0);
        
    }

    void createDialog_(const gchar *path){
        gchar *default_path=NULL;
        if (path) default_path = g_strdup(path);
        mkDialog();
        auto vpane = mkVpane();
        mkPathEntry(path);
        mkFilterEntry();
        ////////////////  grep options.... /////////////////////////
        mkGrepEntry();
       
        mkButtonBox(); 

        gtk_widget_realize(GTK_WIDGET(findDialog));
     
#if 0
        //GtkAllocation label_allocation;
        GtkAllocation allocation;
        gtk_widget_get_allocation(GTK_WIDGET(topPaneVbox_), &allocation);
        //gtk_widget_get_allocation(GTK_WIDGET(path_label), &label_allocation);
        // width will be the smaller of the two.
        gint width = (geometry_.max_width < allocation.width)?
            geometry_.max_width: allocation.width;
        //TRACE(stderr, "max w= %d, top_pane w= %d, label w=%d\n", 
          //      geometry_.max_width,
            //     allocation.width, 
              //  label_allocation.width);
        //gint combo_width;
        //combo_width = width - label_allocation.width - 120;

        //if (combo_width > 100){
           // gtk_widget_set_size_request (GTK_WIDGET(path_entry), combo_width, -1);
        //}
        /*gtk_widget_get_allocation(GTK_WIDGET(filter_label), &label_allocation);
        combo_width = width - label_allocation.width - 120;
        if (combo_width > 100){
            gtk_widget_set_size_request (GTK_WIDGET(filter_entry), combo_width, -1);
        }
        gtk_widget_get_allocation(GTK_WIDGET(grep_label), &label_allocation);
        combo_width = width - label_allocation.width - 120;
        if (combo_width > 100){
            gtk_widget_set_size_request (GTK_WIDGET(grep_entry), combo_width, -1);
        }*/

        // This will be constrained to the maximum width set by geometry_ hints:
        gtk_window_set_default_size(GTK_WINDOW (findDialog),  allocation.width+50,  allocation.height+75);
#endif         
        //gtk_paned_set_position (vpane, 1000);

       auto grep_entry = GTK_ENTRY(g_object_get_data(G_OBJECT(findDialog), "grep_entry"));
       auto filter_entry = GTK_ENTRY(g_object_get_data(G_OBJECT(findDialog), "filter_entry"));
       auto path_entry = g_object_get_data(G_OBJECT(findDialog), "path_entry");
       g_signal_connect (grep_entry,
                          "activate", BUTTON_CALLBACK(Type::onFindButton), 
                          (gpointer)findDialog);
        g_signal_connect (G_OBJECT (findDialog), 
                "destroy_event",  EVENT_CALLBACK(Type::onCloseEvent), NULL);
        g_signal_connect (G_OBJECT (findDialog), 
                "delete_event",  EVENT_CALLBACK(Type::onCloseEvent), NULL);
        g_signal_connect (G_OBJECT (findDialog), 
                "size-allocate", EVENT_CALLBACK(Type::onSizeAllocate), (void *)this);

        if(gtk_entry_get_text(grep_entry) && strlen(gtk_entry_get_text(grep_entry))){
            gtk_editable_set_position (GTK_EDITABLE(grep_entry), 0);
            gtk_widget_grab_focus(GTK_WIDGET(grep_entry));

        } else if(gtk_entry_get_text(filter_entry) && strlen(gtk_entry_get_text(filter_entry))){
            gtk_editable_set_position (GTK_EDITABLE(filter_entry), 0);
            gtk_widget_grab_focus (GTK_WIDGET(filter_entry));
        } else {
            gtk_editable_set_position (GTK_EDITABLE(path_entry), 0);
            gtk_widget_grab_focus (GTK_WIDGET(path_entry));
        }
        gint height = setFindSize(findDialog);

        TRACE("height at %d\n", height);
        gtk_paned_set_position (vpane,height);
        gtk_widget_show_all (GTK_WIDGET(mainVbox_));

        return;
    }

    
    

    
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
        auto buf = g_strdup_printf ("%s", pw ? pw->pw_name : _("unknown"));
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
        auto buf = g_strdup_printf ("%s", gr ? gr->gr_name : _("unknown"));
        g_group = g_slist_prepend (g_group, buf);
        return g_group;
    }

    radio_t *
    create_radios(GtkBox *options_vbox){
        radio_t *radio_p = (radio_t *)malloc(sizeof(radio_t));
        if (!radio_p) g_error("malloc: %s", strerror(errno));
        g_object_set_data(G_OBJECT(findDialog), "radio_p", radio_p );
        memset(radio_p, 0, sizeof(radio_t));

        auto radio1 =  
            GTK_RADIO_BUTTON(gtk_radio_button_new_with_label (NULL, "mtime"));
        gtk_widget_set_tooltip_text(GTK_WIDGET(radio1), _("Modified"));
        //tooltip_c::custom_tooltip(GTK_WIDGET(radio1), NULL, _("Modified"));
        auto radio2 = 
            GTK_RADIO_BUTTON(gtk_radio_button_new_with_label_from_widget ( radio1, "ctime"));
        gtk_widget_set_tooltip_text(GTK_WIDGET(radio2), _("Created"));
        //tooltip_c::custom_tooltip(GTK_WIDGET(radio2), NULL, _("Created"));
        auto radio3 = 
            GTK_RADIO_BUTTON(gtk_radio_button_new_with_label_from_widget ( radio1, "atime"));
        gtk_widget_set_tooltip_text(GTK_WIDGET(radio3), _("Accessed"));
        //tooltip_c::custom_tooltip(GTK_WIDGET(radio3), NULL, _("Accessed"));

        g_object_set_data(G_OBJECT(findDialog), "radio1", radio1 );
        g_object_set_data(G_OBJECT(findDialog), "radio2", radio2 );
        g_object_set_data(G_OBJECT(findDialog), "radio3", radio3 );
        gtk_widget_show (GTK_WIDGET(radio1));
        gtk_widget_show (GTK_WIDGET(radio2));
        gtk_widget_show (GTK_WIDGET(radio3));

        auto radio_box=gtk_c::vboxNew (FALSE, 0);
        g_object_set_data(G_OBJECT(findDialog), "radio_box", radio_box );
        gtk_widget_show (GTK_WIDGET(radio_box));
        compat<bool>::boxPack0 (options_vbox, GTK_WIDGET(radio_box), TRUE, FALSE, 0);
        gtk_widget_set_sensitive(GTK_WIDGET(radio_box), FALSE);
        radio_p->box = radio_box;


        /*GtkWidget *label = gtk_label_new(_("modified"));
        compat<bool>::boxPack0 (GTK_BOX (radio_box), label, TRUE, FALSE, 0);
        gtk_widget_show (label);*/
        auto box=gtk_c::vboxNew (FALSE, 0);
        gtk_widget_show (GTK_WIDGET(box));
        compat<bool>::boxPack0 (radio_box, GTK_WIDGET(box), TRUE, FALSE, 0);

        compat<bool>::boxPack0 (box, GTK_WIDGET(radio1), TRUE, FALSE, 0);
        compat<bool>::boxPack0 (box, GTK_WIDGET(radio2), TRUE, FALSE, 0);
        compat<bool>::boxPack0 (box, GTK_WIDGET(radio3), TRUE, FALSE, 0);

        /*label = gtk_label_new(_("within the last"));
        compat<bool>::boxPack0 (GTK_BOX (radio_box), label, TRUE, FALSE, 0);
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
        if ((!entry_name && !check_name)|| !options_vbox || !findDialog) {
            ERROR("add_option_entry(): incorrect function call\n");
            return NULL;
        }
        auto hbox = gtk_c::hboxNew (FALSE, 0);
        gtk_widget_show (GTK_WIDGET(hbox));
        compat<bool>::boxPack0 (options_vbox, GTK_WIDGET(hbox), TRUE, FALSE, 0);

        auto size_hbox = gtk_c::hboxNew (FALSE, 0);
        GtkCheckButton *check = NULL;
        if (check_name) {
            check = GTK_CHECK_BUTTON(gtk_check_button_new());
            gtk_widget_show (GTK_WIDGET(check));
            g_object_set_data(G_OBJECT(findDialog), check_name, check);
            compat<bool>::boxPack0 (hbox, GTK_WIDGET(check), FALSE, FALSE, 0);
            g_signal_connect (G_OBJECT (check), "toggled", 
                    BUTTON_CALLBACK(Type::sensitivize), 
                    size_hbox);
            gtk_widget_set_sensitive(GTK_WIDGET(size_hbox), FALSE);
        }
        gtk_widget_show (GTK_WIDGET(size_hbox));
        compat<bool>::boxPack0 (hbox, GTK_WIDGET(size_hbox), FALSE, FALSE, 0);

        if (text) {
            auto label = GTK_LABEL(gtk_label_new (text));
            gtk_widget_show (GTK_WIDGET(label));
            compat<bool>::boxPack0 (size_hbox, GTK_WIDGET(label), TRUE, FALSE, 0);
        }
        
        if (entry_name) {
            auto label = GTK_LABEL(gtk_label_new (": "));
            gtk_widget_show (GTK_WIDGET(label));
            compat<bool>::boxPack0 (size_hbox, GTK_WIDGET(label), FALSE, FALSE, 0);
            
            auto entry = GTK_ENTRY(gtk_entry_new());
            gtk_widget_show (GTK_WIDGET(entry));
            compat<bool>::boxPack0 (size_hbox, GTK_WIDGET(entry), TRUE, TRUE, 0);
            g_object_set_data(G_OBJECT(findDialog), entry_name, entry);
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
        if ((!radio1_name  && !check_name)|| !options_vbox || !findDialog) {
            ERROR("add_option_radio2(): incorrect function call\n");
            return NULL;
        }
        auto hbox = gtk_c::hboxNew (FALSE, 0);
        gtk_widget_show (GTK_WIDGET(hbox));
        compat<bool>::boxPack0 (GTK_BOX (options_vbox), GTK_WIDGET(hbox), TRUE, FALSE, 0);

        auto size_hbox = gtk_c::hboxNew (FALSE, 0);
        GtkCheckButton *check = NULL;
        if (check_name) {
            check = GTK_CHECK_BUTTON(gtk_check_button_new());
            gtk_widget_show (GTK_WIDGET(check));
            g_object_set_data(G_OBJECT(findDialog), check_name, check);
            compat<bool>::boxPack0 (GTK_BOX (hbox), GTK_WIDGET(check), FALSE, FALSE, 0);
            g_signal_connect (G_OBJECT (check), "toggled", 
                     BUTTON_CALLBACK(Type::sensitivize), size_hbox);
            gtk_widget_set_sensitive(GTK_WIDGET(size_hbox), FALSE);
        }
        gtk_widget_show (GTK_WIDGET(size_hbox));
        compat<bool>::boxPack0 (GTK_BOX (hbox), GTK_WIDGET(size_hbox), FALSE, FALSE, 0);

        if (text1 && radio1_name) {
            auto radio1 = GTK_RADIO_BUTTON(gtk_radio_button_new_with_label (NULL, text1));
            gtk_widget_show (GTK_WIDGET(radio1));
            compat<bool>::boxPack0 (GTK_BOX (size_hbox), GTK_WIDGET(radio1), TRUE, TRUE, 0);
            g_object_set_data(G_OBJECT(findDialog), radio1_name, radio1);
            if (text2 && radio2_name) {
                auto radio2 = GTK_RADIO_BUTTON(gtk_radio_button_new_with_label_from_widget(radio1, text2));
                gtk_widget_show (GTK_WIDGET(radio2));
                compat<bool>::boxPack0 (GTK_BOX (size_hbox), GTK_WIDGET(radio2), TRUE, TRUE, 0);
                g_object_set_data(G_OBJECT(findDialog), radio2_name, radio2);
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

        if ((!spin_name && !check_name)|| !options_vbox || !findDialog) {
            ERROR("add_option_spin(): incorrect function call\n");
            return NULL;
        }
        auto hbox = gtk_c::hboxNew (FALSE, 0);
        gtk_widget_set_hexpand(GTK_WIDGET(hbox), FALSE);
        gtk_widget_show (GTK_WIDGET(hbox));
        compat<bool>::boxPack0 (options_vbox, GTK_WIDGET(hbox), TRUE, FALSE, 0);

        auto size_hbox = gtk_c::hboxNew (FALSE, 0);
        GtkCheckButton *check = NULL;
        if (check_name) {
            check = GTK_CHECK_BUTTON(gtk_check_button_new());
            gtk_widget_show (GTK_WIDGET(check));
            g_object_set_data(G_OBJECT(findDialog), check_name, (gpointer)check);
            compat<bool>::boxPack0 (hbox, GTK_WIDGET(check), FALSE, FALSE, 0);
            g_signal_connect (G_OBJECT (check), "toggled", 
                    BUTTON_CALLBACK(Type::sensitivize), (gpointer)size_hbox);
            gtk_widget_set_sensitive(GTK_WIDGET(size_hbox), FALSE);
        }
        gtk_widget_show (GTK_WIDGET(size_hbox));
        compat<bool>::boxPack0 (hbox, GTK_WIDGET(size_hbox), FALSE, FALSE, 0);

        if (text) {
            auto label = GTK_LABEL(gtk_label_new (text));
            gtk_widget_show (GTK_WIDGET(label));
            compat<bool>::boxPack0 (size_hbox, GTK_WIDGET(label), TRUE, FALSE, 0);
        }
        
        if (spin_name) {
            auto label = GTK_LABEL(gtk_label_new (": "));
            gtk_widget_show (GTK_WIDGET(label));
            compat<bool>::boxPack0 (size_hbox, GTK_WIDGET(label), FALSE, FALSE, 0);
            auto spinbutton_adj = GTK_ADJUSTMENT(gtk_adjustment_new (default_value, 0, 4096*4096, 1, 64, 0));
            auto spinbutton = GTK_SPIN_BUTTON(gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton_adj), 0, 0));
            gtk_widget_show (GTK_WIDGET(spinbutton));
            compat<bool>::boxPack0 (size_hbox, GTK_WIDGET(spinbutton), TRUE, TRUE, 0);
            gtk_spin_button_set_update_policy (spinbutton, GTK_UPDATE_IF_VALID);
            g_object_set_data(G_OBJECT(findDialog), spin_name, (gpointer)spinbutton);
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
        if ((!combo_name && !check_name)|| !options_vbox || !findDialog) {
            ERROR("add_option_spin(): incorrect function call\n");
            return NULL;
        }
        auto hbox = gtk_c::hboxNew (FALSE, 0);
        gtk_widget_show (GTK_WIDGET(hbox));
        compat<bool>::boxPack0 (options_vbox, GTK_WIDGET(hbox), TRUE, FALSE, 0);

        auto size_hbox = gtk_c::hboxNew (FALSE, 0);
        GtkCheckButton *check = NULL;
        if (check_name) {
            check = GTK_CHECK_BUTTON(gtk_check_button_new());
            gtk_widget_show (GTK_WIDGET(check));
            g_object_set_data(G_OBJECT(findDialog), check_name, check);
            compat<bool>::boxPack0 (hbox, GTK_WIDGET(check), FALSE, FALSE, 0);
            g_signal_connect (G_OBJECT (check), "toggled", 
                    BUTTON_CALLBACK(Type::sensitivize), size_hbox);
            gtk_widget_set_sensitive(GTK_WIDGET(size_hbox), FALSE);
        }
        gtk_widget_show (GTK_WIDGET(size_hbox));
        compat<bool>::boxPack0 (hbox, GTK_WIDGET(size_hbox), FALSE, FALSE, 0);

        if (text) {
            auto label = GTK_LABEL(gtk_label_new (text));
            gtk_widget_show (GTK_WIDGET(label));
            compat<bool>::boxPack0 (size_hbox, GTK_WIDGET(label), TRUE, FALSE, 0);
            label = GTK_LABEL(gtk_label_new (": "));
            gtk_widget_show (GTK_WIDGET(label));
            compat<bool>::boxPack0 (size_hbox, GTK_WIDGET(label), FALSE, FALSE, 0);
       }

        if (combo_name) {
            auto usermodel=gtk_list_store_new (1, G_TYPE_STRING);
            setStoreDataFromList (usermodel, &list);

            auto combo = GTK_COMBO_BOX(gtk_combo_box_new_with_entry());
            gtk_combo_box_set_model (combo,GTK_TREE_MODEL(usermodel));
            gtk_combo_box_set_entry_text_column (combo,0);
            
            auto entry  = GTK_ENTRY (gtk_bin_get_child(GTK_BIN(combo)));
            gtk_entry_set_text (entry, (const gchar *)list->data);
            gtk_widget_show(GTK_WIDGET(combo));
            compat<bool>::boxPack0 (size_hbox, GTK_WIDGET(combo), TRUE, TRUE, 0);
            g_object_set_data(G_OBJECT(findDialog), combo_name, combo);
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
    static constexpr const gchar *
    pathSelect_text_help=
                N_("Close find" "\n\n");

    static constexpr const gchar *
    filter_text_help=
                N_("Basic rules:\n" "\n"
                          "*  Will match any character zero or more times.\n"
                          "?  Will match any character exactly one time\n"
                          "[] Match any character within the [] \n"
                          "^  Match at beginning of string\n" 
                          "$  Match at end of string \n");
public:
    static constexpr const gchar *
    grep_text_help=
                N_("Reserved characters for extended regexp are "
                          ". ^ $ [ ] ? * + { } | \\ ( ) : \n"
                          "In  basic regular expressions the metacharacters ?, +, {, |, (, and ) \n"
                          "  lose their special meaning.\n"
                          "\n"
                          "The  period . matches  any  single  character.\n"
                          "The caret ^ matches at the start of line.\n"
                          "The dollar $ matches at the end of line.\n" "\n"
                          "Characters within [ ] matches any single character in the list.\n"
                          "Characters within [^ ] matches any single character *not* in the list.\n"
                          "Characters inside [ - ] matches a range of characters (ie [0-9] or [a-z]).\n" "\n"
                          "A regular expression may be followed by one of several repetition operators:\n"
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
                          "Two regular expressions may be joined by the logical or operator |.\n"
                          "Two regular expressions may be concatenated.\n" "\n"
                          "More information is available by typing \"man grep\"\n");
private:
    static gint
    setFindSize(GtkWindow *dialog){
        gint width = Settings<Type>::getInteger("xffind", "width");
        gint height = Settings<Type>::getInteger("xffind", "height");
        gtk_widget_realize(GTK_WIDGET(dialog));
        
        gtk_widget_set_size_request (GTK_WIDGET(dialog), 450, 440);
        gtk_widget_realize(GTK_WIDGET(dialog));
        GtkRequisition minimumSize;
        GtkRequisition naturalSize;
        gtk_widget_get_preferred_size (GTK_WIDGET(dialog),
                               &minimumSize,
                               &naturalSize);
       // TRACE("xffind Size: minimum=%d,%d, natural=%d,%d, settings=%d,%d\n",
         //       minimumSize.width, minimumSize.height,
           //     naturalSize.width, naturalSize.height,gravity
             //   width, height);
        if (width >= naturalSize.width && height >= naturalSize.height){
            gtk_window_resize (GTK_WINDOW(dialog), width, height);
        }

        gtk_widget_show(GTK_WIDGET(dialog));
        Dialogs<Type>::placeDialog(GTK_WINDOW(dialog));
        GtkAllocation allocation;
        gtk_widget_get_allocation(GTK_WIDGET(dialog), &allocation);
        TRACE("xffind Size: allocation=%d,%d\n",
                allocation.width, allocation.height);
        return (height >= naturalSize.height)? height: naturalSize.height;
    }

    GtkEntry *mkCompletionEntry(const gchar *history, int width){
        auto entry = mkCompletionEntry(history);
        if (width > 0) gtk_widget_set_size_request(GTK_WIDGET(entry), width, -1);
        return entry;
    }

    GtkEntry *mkCompletionEntry(const gchar *history){
        auto entry = GTK_ENTRY(gtk_entry_new());
        auto model = util_c::loadHistory(history);
        g_object_set_data(G_OBJECT(entry), "model", model);
        GtkTreeIter iter;
        if (gtk_tree_model_get_iter_first (model, &iter)){
            gchar *value;
            gtk_tree_model_get (model, &iter, 0, &value, -1);
            gtk_entry_set_text(entry, value);        
            gtk_editable_select_region (GTK_EDITABLE(entry), 0, strlen(value));
            g_free(value);
        }
        
        auto completion = gtk_entry_completion_new();
        gtk_entry_set_completion (entry, completion);
        gtk_entry_completion_set_model (completion, model);
        gtk_entry_completion_set_popup_completion(completion, TRUE);
        gtk_entry_completion_set_text_column (completion, 0);
                                      
        g_signal_connect (entry,
                          "key_release_event", KEY_EVENT_CALLBACK(Type::on_completion), 
                          (gpointer)NULL);
        return entry;
    }
    

};
} // namespace xf
#endif

