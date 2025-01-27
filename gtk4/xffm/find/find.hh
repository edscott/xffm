#ifndef FIND__HH
# define FIND__HH
//
//#include "fgr.hh"
//#include "dialog.hh"
//
//msgid "Show search results for this query"
//msgid "Clear the search results."
//msgid "Show my search results"
//
// 1 file chooser button broken
typedef struct opt_t{
  const char *text;
  const char *id;
  gboolean defaultValue;
} opt_t;

namespace xf
{
    template <class Type> class findSignals;

    template <class Type>
    class FindResponse {
      using subClass_t = FindResponse<Type>;
      using dialog_t = DialogComplex<subClass_t>;
   
      //GtkBox *mainVbox_ = NULL;
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

    
      char *folder_ = NULL;
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
   GtkWindow *dialog_ = NULL;
   GtkCheckButton *firstGrepRadio_ = NULL;
      GtkBox *mainBox_ = NULL;
      GtkBox *topPaneVbox_ = NULL;
      GtkBox *topPaneHbox_ = NULL;
      GtkPaned *vpane_ = NULL;
      GtkNotebook *notebook_ = NULL;
      GtkBox *advancedVbox_ = NULL;

public:
      const char *label(void){return "xffm::find";}

      FindResponse (void){
      }

      ~FindResponse (void){
        g_free(folder_);
        DBG("*** ~FindResponse\n");
      }

      char *folder(void){ return folder_;}
      void folder(const char *value){ 
        g_free(folder_);
        if (!value) folder_ = g_get_current_dir();
        else folder_ = realpath(value, NULL);
      }

      GtkWindow *dialog(void){return dialog_;}

      void dialog(GtkWindow *value){
        dialog_ = value;
      }

     
      const char *title(void){ return _("Find files");}
      const char *iconName(void){ return EMBLEM_FIND;}

       static void *asyncYes(void *data){
        auto dialogObject = (dialog_t *)data;
        DBG("%s", "hello world asyncYes\n");
        return NULL;
      }

      static void *asyncNo(void *data){
        auto dialogObject = (dialog_t *)data;
        DBG("%s", "goodbye world asyncNo\n");
        return NULL;
      }


      GtkBox *mainBox(const char *folder) {

        /* FIXME:
(process:824184): GLib-GObject-CRITICAL **: 16:24:08.620: g_object_get_data: assertion 'G_IS_OBJECT (object)' failed
(process:824184): Gtk-CRITICAL **: 16:24:08.620: gtk_box_prepend: assertion 'GTK_IS_BOX (box)' failed
        auto closeBox =GTK_BOX(g_object_get_data(G_OBJECT(dialog_), "closeBox"));

        auto foo = gtk_label_new("foo");
        gtk_box_prepend(closeBox, foo);
        */
        
          if (g_file_test(folder, G_FILE_TEST_IS_DIR)) folder_ = realpath(folder, NULL);
          else {
            if (g_file_test(Child::getWorkdir(), G_FILE_TEST_IS_DIR)) folder_ = g_strdup(Child::getWorkdir());
            else folder_ = g_strdup(g_get_home_dir());
          }


          mainBox_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
          gtk_widget_set_size_request(GTK_WIDGET(mainBox_), 550, 400);
          gtk_widget_set_vexpand(GTK_WIDGET(mainBox_), false);
          gtk_widget_set_hexpand(GTK_WIDGET(mainBox_), false);
          //gtk_window_set_child(findDialog, GTK_WIDGET(mainBox_));
          DBG("mkVpane  <- \ni");
          mkVpane();

          DBG("mkPathEntry  <- \n");
          mkPathEntry();
          DBG("mkFilterEntry  <- \n");
          mkFilterEntry();
          ////////////////  grep options.... /////////////////////////
          DBG("mkGrepEntry  <- \n");
          mkGrepEntry();
          DBG("mkButtonBox  <- \n");
         
          mkButtonBox(); 

          //gtk_widget_realize(GTK_WIDGET(findDialog));
          return mainBox_;
      }

private:
          
      void mkVpane(void){
          vpane_ = GTK_PANED(gtk_paned_new(GTK_ORIENTATION_VERTICAL));
          gtk_widget_set_vexpand(GTK_WIDGET(vpane_), true);
          gtk_paned_set_wide_handle (vpane_,TRUE);
          gtk_box_append(mainBox_, GTK_WIDGET(vpane_));

          topPaneVbox_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 2));
          gtk_widget_set_vexpand(GTK_WIDGET(topPaneVbox_), true);
          advancedVbox_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 2));

          mkTopPaneHbox();
          gtk_box_append(topPaneVbox_, GTK_WIDGET(topPaneHbox_));
          
          // FIXME: g_object_set_data(G_OBJECT(findDialog), "vpane", (gpointer)vpane_);
          //compat<bool>::boxPack0 (mainBox_, GTK_WIDGET(vpane_), TRUE, TRUE, 0);
          // hack: widgets_p->paper = findDialog;
          //gtk_container_set_border_width (GTK_CONTAINER (topPaneVbox_), 5);

          auto sw = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new());
          gtk_paned_set_start_child (vpane_, GTK_WIDGET(sw));
          //gtk_paned_pack1 (GTK_PANED (vpane_), GTK_WIDGET (sw), FALSE, TRUE);

          notebook_ = GTK_NOTEBOOK(gtk_notebook_new());
          gtk_notebook_append_page(notebook_, GTK_WIDGET(topPaneVbox_), gtk_label_new(_("Grep Frontend")));
          gtk_scrolled_window_set_child(sw, GTK_WIDGET(notebook_));

          gtk_notebook_append_page(notebook_, GTK_WIDGET(advancedVbox_), gtk_label_new(_("Advanced options")));


          //gtk_scrolled_window_set_child(sw, GTK_WIDGET(topPaneVbox_));
          //gtk_container_add(GTK_CONTAINER(sw), GTK_WIDGET(topPaneVbox_));
          gtk_scrolled_window_set_policy (sw, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

          auto diagnostics = GTK_TEXT_VIEW(gtk_text_view_new ());
          // FIXME: g_object_set_data(G_OBJECT(findDialog), "diagnostics", (gpointer)diagnostics);
          g_object_set_data(G_OBJECT(diagnostics), "vpane", (gpointer)vpane_);
          
          auto sw2 = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new ());
          gtk_paned_set_end_child (vpane_, GTK_WIDGET(sw2));
          //gtk_paned_pack2 (GTK_PANED (vpane_), GTK_WIDGET(sw2), FALSE, TRUE);
          //gtk_paned_pack2 (GTK_PANED (vpane_), GTK_WIDGET(scrolledwindow), TRUE, TRUE);
          gtk_scrolled_window_set_policy (sw2, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
          gtk_scrolled_window_set_child(sw2, GTK_WIDGET(diagnostics));
          //gtk_container_add (GTK_CONTAINER (scrolledwindow), GTK_WIDGET(diagnostics));

          //gtk_container_set_border_width (GTK_CONTAINER (diagnostics), 2);
          gtk_widget_set_can_focus(GTK_WIDGET(diagnostics), FALSE);
          gtk_text_view_set_wrap_mode (diagnostics, GTK_WRAP_WORD);
          gtk_text_view_set_cursor_visible (diagnostics, FALSE);
           
          gtk_paned_set_position(vpane_, 3000);
          return ;
      }

      void mkTopPaneHbox(){
          topPaneHbox_= GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
          gtk_widget_set_hexpand(GTK_WIDGET(topPaneHbox_), TRUE);

          auto vbox1 = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
          auto vbox2 = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
          auto vbox3 = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));

          ////////////   closeButton... obsolete /////////////////////////
          /*auto closeButton =  UtilBasic::mkButton gtk_c::dialog_button(WINDOW_CLOSE, "");

          g_object_set_data(G_OBJECT(findDialog), "close_button", closeButton);
          g_object_set_data(G_OBJECT(closeButton), "findDialog", findDialog);
          g_signal_connect (G_OBJECT (closeButton), "clicked",
                  BUTTON_CALLBACK(Type::onCloseButton), (gpointer)findDialog);
          compat<bool>::boxPack0 (vbox2, GTK_WIDGET(closeButton), TRUE, TRUE, 0);
          auto h = g_strconcat(_("Close find"), "\n\n", NULL); 
          Basic::setTooltip(GTK_WIDGET(closeButton), h);
          g_free(h);*/

          ////////////   findButton... /////////////////////////
          auto findButton = UtilBasic::mkButton(EMBLEM_FIND, NULL);
          //FIXME gtk_widget_set_can_default(GTK_WIDGET(findButton), TRUE);
          g_signal_connect (G_OBJECT (findButton), "clicked",
                  BUTTON_CALLBACK(findSignals<Type>::onFindButton), (gpointer)this);
//                  BUTTON_CALLBACK(findSignals<Type>::onFindButton), (gpointer)findDialog);
          gtk_box_append(vbox3, GTK_WIDGET(findButton));

         Basic::setTooltip(GTK_WIDGET(findButton), _("Show search results for this query"));

          ////////////   advanced options... FIXME /////////////////////////
#if 0
          auto advancedDialog = advancedOptions();
          gtk_window_set_transient_for(GTK_WINDOW(advancedDialog), GTK_WINDOW(findDialog));
         


          auto advancedButton = gtk_c::toggle_button(DOCUMENT_PROPERTIES, NULL);
  //        auto advancedButton = gtk_c::toggle_button(NULL, _("Details"));
          auto h3 = g_strconcat(_("Details"), "\n\n", NULL); 
          Basic::setTooltip(GTK_WIDGET(advancedButton), h3);
          g_free(h3);
          g_object_set_data(G_OBJECT(findDialog), "advancedButton", advancedButton);
          g_object_set_data(G_OBJECT(findDialog), "advancedDialog", advancedDialog);
          compat<bool>::boxPack0 (vbox1, GTK_WIDGET(advancedButton), TRUE, TRUE, 5);
          g_signal_connect (advancedButton,
                            "clicked", WIDGET_CALLBACK(findSignals<Type>::onDetails), 
                            (gpointer)findDialog);
#endif
          auto t=g_strdup_printf("<span color=\"blue\" size=\"large\"><b>%s</b></span>  ", _("Find in Files"));
          auto label = GTK_LABEL(gtk_label_new (t));
          gtk_label_set_use_markup (label, TRUE);
          g_free(t);

          gtk_box_append(topPaneHbox_, GTK_WIDGET(vbox1));
          gtk_box_append(topPaneHbox_, GTK_WIDGET(label));
          gtk_box_append(topPaneHbox_, GTK_WIDGET(vbox3));
          //compat<bool>::boxPack0 (topPaneHbox_, GTK_WIDGET(vbox1), FALSE, FALSE, 0);
         // compat<bool>::boxPack0 (topPaneHbox_, GTK_WIDGET(label), TRUE, TRUE, 0);
          //compat<bool>::boxPack0 (topPaneHbox_, GTK_WIDGET(vbox3), FALSE, FALSE, 0);


      }
      void mkPathEntry(void){
          GtkWidget *path_label;

        /*  gchar *default_path=NULL;
          if (path) default_path = g_strdup(path);*/

          //auto path_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
          //gtk_widget_set_hexpand(GTK_WIDGET(path_box), TRUE);
          // DBG("gtk_box_append path_box \n");
          //gtk_box_append(topPaneVbox_, GTK_WIDGET(path_box));
          //compat<bool>::boxPack0 (GTK_BOX (topPaneVbox_), GTK_WIDGET(path_box), FALSE, TRUE, 0);


          auto text=g_strdup_printf("%s:", _("Path"));
          auto path_entry = FileResponse<Type, subClass_t>::addEntry(topPaneVbox_, "entry1", text, this);

          g_free(text);

          //gtk_widget_set_size_request (GTK_WIDGET(path_entry), 50, -1);
          // FIXME: g_object_set_data(G_OBJECT(findDialog), "path_entry", path_entry);

          auto buffer = gtk_entry_get_buffer(GTK_ENTRY(path_entry));  
          DBG("folder_ = %s\n", folder_);
          gtk_entry_buffer_set_text(buffer, folder_, -1);

          //DBG("gtk_box_append path_box path_entry\n");
          //gtk_box_append(path_box, GTK_WIDGET(path_entry));
          //compat<bool>::boxPack0 (path_box, GTK_WIDGET(path_entry), TRUE, TRUE, 0);

          g_signal_connect (path_entry,
                            "activate", BUTTON_CALLBACK(findSignals<Type>::onFindButton), 
                            (gpointer)this);
//                            (gpointer)findDialog);


      }

      // simple entry, no filechooser.
       GtkEntry *addEntry(GtkBox *child, const char *id, const char *text){
          auto hbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
          gtk_widget_set_vexpand(GTK_WIDGET(hbox), false);
          gtk_widget_set_hexpand(GTK_WIDGET(hbox), true);
          auto label = gtk_label_new(text);
          gtk_widget_set_hexpand(GTK_WIDGET(label), false);
          auto entry = gtk_entry_new();

          //auto buffer = gtk_entry_buffer_new(NULL, -1);
          //auto entry = gtk_entry_new_with_buffer(buffer);
          gtk_widget_set_hexpand(GTK_WIDGET(entry), true);
          g_object_set_data(G_OBJECT(child), id, entry);
          //gtk_widget_set_sensitive(GTK_WIDGET(entry), true); // FIXME: put to false 
                                                             // when filedialog button
                                                             // is working.
    //      auto button = UtilBasic::mkButton(EMBLEM_FOLDER_OPEN, NULL);
    //      g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(getDirectory), this);

          gtk_box_append(hbox, label);
          gtk_box_append(hbox, entry);
     //     gtk_box_append(hbox, GTK_WIDGET(button));
          gtk_box_append(child, GTK_WIDGET(hbox));
          return GTK_ENTRY(entry);
        }

        void mkFilterEntry(void){
            auto filter_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
            gtk_box_append(topPaneVbox_, GTK_WIDGET(filter_box));
            //compat<bool>::boxPack0 (topPaneVbox_, GTK_WIDGET(filter_box), FALSE, TRUE, 5);

            auto text=g_strdup_printf("%s ", _("Filter:"));
            auto filter_label = GTK_LABEL(gtk_label_new (text));
            g_free(text);

            auto history = g_build_filename(FILTER_HISTORY);
            auto filter_entry = mkCompletionEntry(history);
            g_free(history);


            auto dialogbutton2 = UtilBasic::mkButton(EMBLEM_QUESTION, NULL);
            //FIXME: g_object_set_data(G_OBJECT(dialogbutton2), "findDialog", findDialog);
            Basic::setTooltip(GTK_WIDGET(dialogbutton2), _(filter_text_help));
            g_signal_connect (dialogbutton2,
                              "clicked", WIDGET_CALLBACK(findSignals<Type>::on_buttonHelp), 
                              (gpointer)filter_text_help);
            
            auto vbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 6));
            gtk_box_append(vbox, GTK_WIDGET(dialogbutton2));
            gtk_box_append(filter_box, GTK_WIDGET(filter_label));
            gtk_box_append(filter_box, GTK_WIDGET(filter_entry));
            gtk_box_append(filter_box, GTK_WIDGET(vbox));
            //compat<bool>::boxPack0 (vbox, GTK_WIDGET(dialogbutton2), FALSE, FALSE, 0);
            //compat<bool>::boxPack0 (filter_box, GTK_WIDGET(filter_label), FALSE, FALSE, 0);
            //compat<bool>::boxPack0 (filter_box, GTK_WIDGET(filter_entry), TRUE, TRUE, 0);
            //compat<bool>::boxPack0 (filter_box, GTK_WIDGET(vbox), FALSE, FALSE, 0);
            g_signal_connect (filter_entry,
                              "activate", BUTTON_CALLBACK(findSignals<Type>::onFindButton), 
                              (gpointer)this);
                             // (gpointer)findDialog);

        }


        GtkEntry *mkCompletionEntry(const gchar *history){
            auto entry = GTK_ENTRY(gtk_entry_new());
            gtk_widget_set_hexpand(GTK_WIDGET(entry), true);
            /* FIXME
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
                              "key_release_event", KEY_EVENT_CALLBACK(findSignals<Type>::on_completion), 
                              (gpointer)NULL);
                              */
            return entry;
        }
        
        void mkGrepEntry(void){
            auto grep_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
            gtk_box_append(topPaneVbox_, GTK_WIDGET(grep_box));
            //compat<bool>::boxPack0 (topPaneVbox_, GTK_WIDGET(grep_box), FALSE, TRUE, 5);

            auto t=g_strdup_printf("%s: ",_("Contains the text"));
            auto grep_label = GTK_LABEL(gtk_label_new (t));
            g_free(t);
        
            auto history = g_build_filename(GREP_HISTORY);
            auto grep_entry = mkCompletionEntry(history);
            //FIXME: g_object_set_data(G_OBJECT(findDialog), "grep_entry", grep_entry);
            g_free(history);        
            gtk_widget_set_sensitive (GTK_WIDGET(grep_entry), TRUE);   
            
            auto button = UtilBasic::mkButton(EMBLEM_QUESTION, NULL);
            //FIXME: g_object_set_data(G_OBJECT(button), "findDialog", findDialog);
            Basic::setTooltip(GTK_WIDGET(button), _(grep_text_help));
            g_signal_connect (GTK_WIDGET(button),
                              "clicked", WIDGET_CALLBACK(findSignals<Type>::on_buttonHelp), 
                              (gpointer) _(grep_text_help));

            auto vbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 6));
            gtk_box_append(vbox, GTK_WIDGET(button));
            gtk_box_append(grep_box, GTK_WIDGET(grep_label));
            gtk_box_append(grep_box, GTK_WIDGET(grep_entry));
            gtk_box_append(grep_box, GTK_WIDGET(vbox));
            //compat<bool>::boxPack0 (vbox, GTK_WIDGET(button), FALSE, FALSE, 0);
            //compat<bool>::boxPack0 (grep_box, GTK_WIDGET(grep_label), FALSE, FALSE, 0);
            //compat<bool>::boxPack0 (grep_box, GTK_WIDGET(grep_entry), TRUE, TRUE, 0);
            //compat<bool>::boxPack0 (grep_box, GTK_WIDGET(vbox), FALSE, FALSE, 0);

            auto buffer = gtk_entry_get_buffer(grep_entry);
            const char *text = gtk_entry_buffer_get_text(buffer);
            gboolean active_grep =  text && strlen(text); 
      
            auto checkBox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
            opt_t options[] = {
              {_("Case Sensitive"),"case_sensitive",default_case_sensitive},
              {_("Extended regexp"), "ext_regexp",default_ext_regexp},
              {_("Include binary files"),"look_in_binaries",default_look_in_binaries},
              {_("Line Count"),"line_count",default_line_count},
              {NULL,NULL,0}};
            for (opt_t *p=options; p->text != NULL; p++) mkGrepOption(p, active_grep, grep_entry, checkBox);
            
               
            auto radioBox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
            GSList *radioGroup = NULL;
            opt_t rOptions[] = {
              {_("Anywhere"),"anywhere",default_anywhere},
              {_("Whole words only"), "match_words",default_match_words},
              {_("lines"),"match_lines",default_match_lines},
              {_("No match"),"match_no_match",default_match_no_match},
              {NULL,NULL,0}};
            for (opt_t *p=rOptions; p->text != NULL; p++) {
              mkGrepRadio(p, active_grep, grep_entry, &radioGroup, radioBox);
            }
            
            
            auto optionBox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10));
            gtk_box_append(optionBox, GTK_WIDGET(checkBox));
            gtk_box_append(optionBox, GTK_WIDGET(radioBox));
            gtk_box_append(topPaneVbox_, GTK_WIDGET(optionBox));
     
            //compat<bool>::boxPack0 (optionBox, GTK_WIDGET(checkBox), FALSE, FALSE, 0);
            //compat<bool>::boxPack0 (optionBox, GTK_WIDGET(radioBox), FALSE, FALSE, 0);
            //compat<bool>::boxPack0 (topPaneVbox_, GTK_WIDGET(optionBox), FALSE, FALSE, 0);

        }

        void mkGrepOption(opt_t *opt, gboolean active_grep, GtkEntry *grep_entry, GtkBox *checkBox){
            auto check = GTK_CHECK_BUTTON(gtk_check_button_new_with_mnemonic (opt->text));
            gtk_widget_set_sensitive (GTK_WIDGET(check), active_grep);
            gtk_check_button_set_active (check, opt->defaultValue);
            //FIXME: g_object_set_data(G_OBJECT(findDialog), opt->id, check);
            // FIXME g_signal_connect (G_OBJECT (grep_entry), "event", 
                   // KEY_EVENT_CALLBACK (findSignals<Type>::on_key_release), (gpointer) check);
            gtk_box_append(checkBox, GTK_WIDGET(check));
            //compat<bool>::boxPack0 (checkBox, GTK_WIDGET(check), FALSE, FALSE, 0);
        }


        void mkGrepRadio(opt_t *opt, gboolean active_grep, GtkEntry *grep_entry, GSList **group, GtkBox *radioBox){
            auto radio = GTK_CHECK_BUTTON(gtk_check_button_new_with_label(opt->text));
            
            gtk_check_button_set_group(radio, firstGrepRadio_);
            if (!firstGrepRadio_) firstGrepRadio_ = radio;
            
            //*group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio));
            //FIXME: g_object_set_data(G_OBJECT(findDialog), opt->id, radio);
            gtk_widget_set_sensitive (GTK_WIDGET(radio), active_grep);

            if (opt->defaultValue) {
                gtk_check_button_set_active (radio, TRUE);
            }
            // FIXME g_signal_connect (G_OBJECT (grep_entry), "event", 
                   // KEY_EVENT_CALLBACK (findSignals<Type>::on_key_release), (gpointer) radio);
            gtk_box_append(radioBox, GTK_WIDGET(radio));
            //compat<bool>::boxPack0 (radioBox, GTK_WIDGET(radio), FALSE, FALSE, 0);
        }
        

        GtkBox *boxButton(const char *iconName, void *callback){
            auto cbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
            gtk_widget_set_hexpand(GTK_WIDGET(cbox), true);
            gtk_widget_set_halign(GTK_WIDGET(cbox), GTK_ALIGN_CENTER);
            auto button =  UtilBasic::mkButton(iconName, NULL);
            g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK(callback), (void *)this);
            gtk_box_append(cbox, GTK_WIDGET(button));
            return cbox;
        }

        void mkButtonBox(void){
            auto hbuttonbox2 = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3));
            auto cancelButton =  boxButton(EMBLEM_RED_BALL, (void *)findSignals<Type>::onCancelButton);
            //FIXME: g_object_set_data(G_OBJECT(findDialog), "cancel_button", cancelButton);
            //FIXME: g_object_set_data(G_OBJECT(cancelButton), "findDialog", findDialog);
            gtk_widget_set_sensitive(GTK_WIDGET(cancelButton), FALSE);

            auto clearButton =  boxButton(EDIT_CLEAR,(void *) findSignals<Type>::onClearButton);


            //FIXME: g_object_set_data(G_OBJECT(findDialog), "clear_button", clearButton);
            //FIXME: g_object_set_data(G_OBJECT(clearButton), "findDialog", findDialog);

            GtkBox *edit_button = NULL;

            auto editor =Basic::getEditor();
            if (editor && strlen(editor)){
                auto basename = g_strdup(editor);
                if (strchr(basename, ' ')) *strchr(basename, ' ') = 0;
                auto editor_path = g_find_program_in_path(basename);
                g_free(basename);
                if (editor_path){
                    auto icon_id = Basic::getAppIconName(editor_path, EMBLEM_EDIT);
                    edit_button = boxButton(icon_id, (void *)findSignals<Type>::onEditButton);
                    g_free(icon_id);
                    //FIXME: g_object_set_data(G_OBJECT(findDialog), "edit_button", edit_button);
                    //FIXME: g_object_set_data(G_OBJECT(edit_button), "findDialog", findDialog);
                    g_free(editor_path);
                    gtk_widget_set_sensitive(GTK_WIDGET(edit_button), FALSE);
                } 
            } else {
                TRACE("getEditor() = \"%s\"\n", editor);
            }
          
            gtk_box_append(hbuttonbox2, GTK_WIDGET(clearButton));
            if (edit_button) gtk_box_append(hbuttonbox2, GTK_WIDGET(edit_button));
            gtk_box_append(hbuttonbox2, GTK_WIDGET(cancelButton));
            gtk_box_append(mainBox_, GTK_WIDGET(hbuttonbox2));            
        }

/*
        static void getDirectory(GtkButton *button, void *data){
          auto subClass = (FindResponse *)data;
          TRACE("*** getDirectory Folder = %s\n", subClass->folder());
          auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "entry"));
          auto parent = subClass->dialog();
          auto folder = subClass->folder();
          auto newObject = new dialog_t(parent, folder);
          //newObject->subClass()->parentEntry(entry);
          //newObject->subClass()->folder(folder);
        }
*/
      };

        ///////////////////   signals  /////////////////////////
template <class Type>
class findSignals: public Run<Type>{
//class findSignals: public Run<Type>{
  public:
    static void
    onFindButton (GtkWidget * button, gpointer dialog) {
      DBG("onFindButton...\n");
 /*       if (!controllerHash){
            controllerHash = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, g_free);
        }
        GtkTextView *diagnostics = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(dialog), "diagnostics"));
        updateCompletions(dialog);
        
        print_c::showTextSmall(diagnostics);
        on_find_clicked_action (GTK_WINDOW(dialog));*/
    }

    static void
    onEditButton (GtkWidget * button, gpointer data) {
      DBG("onEditButton...\n");
/*        GtkTextView *diagnostics = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(data), "diagnostics"));
        TRACE("fixme: signals::onEditButton\n");
        //print_c::print_status(diagnostics, g_strdup("fixme: signals::onEditButton testing run\n"));
        print_c::showTextSmall(diagnostics);        
        edit_command(data);*/
    }

    static void
    onClearButton (GtkWidget * button, gpointer data) {
      DBG("onClearButton...\n");
     /*   GtkTextView *diagnostics = GTK_TEXT_VIEW(data);
        TRACE("fixme: signals::onClearButton\n");
        print_c::clear_text(diagnostics);
        print_c::hide_text(diagnostics);*/

    }

    static void
    onCancelButton (GtkWidget * button, gpointer data) {
      DBG("onCancelButton...\n");
        //cancel_all(data);
    }

    static void
    on_buttonHelp (GtkWidget * button, gpointer data) {
      DBG("on_buttonHelp...\n");
      /*  GtkWindow *dialog=GTK_WINDOW(g_object_get_data(G_OBJECT(button), "findDialog"));
        const gchar *message = (const gchar *)data;
        TRACE("fixme: signals::on_buttonHelp\n");
        Dialogs<Type>::quickHelp(dialog, message);*/
    }

      };


}
namespace xf
{

template <class Type>
class Find {

    char *fullPath_ = NULL;
    gboolean gnuGrep_ = true;

public:

    Find(const gchar *path){
        if (!whichGrep()){
            ERROR("grep command not found\n");
            exit(1);
        }
        fullPath(path);
        gchar *fullPath = NULL;
        createDialog();
    }

    ~Find(void){
        g_free(fullPath_);
    }

    void fullPath(const char *path){
        if (!path || !g_file_test(path, G_FILE_TEST_EXISTS)) {
          auto current = g_get_current_dir();
          fullPath_ = realpath(current, NULL);
          g_free(current);
          return;
        }
        fullPath = realpath(path, NULL);
        return;
    }



private:

    gboolean whichGrep(void){
        gchar *grep = g_find_program_in_path ("grep");
        if (!grep) return false;
        FILE *pipe;
        const gchar *cmd = "grep --version";
        gnuGrep_ = false;
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
        return true;
    }

    void createDialog(){

    }

};
} // namespace xf
#endif
