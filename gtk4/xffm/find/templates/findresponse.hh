#ifndef FINDRESPONSE_HH
# define FINDRESPONSE_HH

#include "../findtypes.h"
#include "../classes/ffunctions.hh"
#include "../classes/fgr.hh"
#include "findsignals.hh"
#include "find.hh"
#include "dndbox.hh"

namespace xf
{
    template <class Type>
    class FindResponse : protected Ffunctions,
                         protected FindSignals<Type>
    {
      using subClass_t = FindResponse<Type>;
      using dialog_t = DialogComplex<subClass_t>;

   
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
      gboolean default_anywhere=TRUE;
      gboolean default_match_words=FALSE;
      gboolean default_match_lines=FALSE;
      gboolean default_match_no_match=FALSE;
//      GSList *find_list = NULL;
      gchar  *last_workdir = NULL;
      GtkWidget *findButton_ = NULL;
      GtkButton *cancelButton_ = NULL;
      GtkLabel *findLabel_ = NULL;
    
      char *folder_ = NULL;
      bool active_grep_ = false;
      GtkWindow *dialog_ = NULL;
      GtkCheckButton *firstGrepRadio_ = NULL;
      GtkTextView *textview_ = NULL;
      GtkBox *mainBox_ = NULL;
      GtkFrame *frame_ = NULL;
      GtkBox *topPaneVbox_ = NULL;
      GtkBox *topPaneHbox_ = NULL;
      GtkPaned *vpane_ = NULL;
      GtkNotebook *notebook_ = NULL;
      GtkBox *advancedVbox_ = NULL;

      GtkEntry *grepEntry_ = NULL;
      fgrData_t *Data_=NULL;

public:

       static void *asyncYes(void *data){
        auto dialogObject = (dialog_t *)data;
        TRACE("%s", "hello world asyncYes\n");
        return NULL;
      }

      static void *asyncNo(void *data){
        auto dialogObject = (dialog_t *)data;
        TRACE("%s", "findresponse ................ goodbye world asyncNo\n");
        gtk_widget_set_visible(GTK_WIDGET(dialogObject->dialog()), false);
        Basic::flushGTK();
        exit(0);
        return NULL;
      }

      GtkWidget *findButton(void) {return findButton_;}
      GtkButton *cancelButton(void) {return cancelButton_;}
      GtkLabel *findLabel(void) {return findLabel_;}

      void Data(fgrData_t *value) {Data_ = value;}
      fgrData_t *Data(void) {return Data_;}
      GtkPaned *vpane(void){return vpane_;}
      const char *label(void){return "xffm::find";}
      GtkTextView *textview(void){return textview_;}

      FindResponse (void){
      }

      ~FindResponse (void){
        TRACE("************** ~FindResponse\n");
        g_free(folder_);
        //exit(0);
      }

      GtkBox *mainBox(void) { return _mainBox();}
      GtkBox *mainBox(const char *folder) {return _mainBox(folder);}

      GtkWindow *dialog(void){return dialog_;}
      void dialog(GtkWindow *value){
        TRACE("*** findResponse setting dialog to %p\n", value);
        dialog_ = value;
        gtk_window_set_default_widget(GTK_WINDOW(dialog_), GTK_WIDGET(findButton_));
      }
      
      const char *title(void){ return _("Find files");}

      char *folder(void){ return folder_;}
      void folder(const char *value){ 
        g_free(folder_);
        if (!value) folder_ = g_get_current_dir();
        else folder_ = realpath(value, NULL);
      }

      void 
      saveHistories(void){ 
          char *history;
          GtkEntry *entry;
          GtkEntryBuffer *buffer;
          const char *text;

          history = g_build_filename (FILTER_HISTORY);
          entry = GTK_ENTRY(g_object_get_data(G_OBJECT(mainBox_), "filter_entry"));
          buffer = gtk_entry_get_buffer(entry);
          text = gtk_entry_buffer_get_text(buffer);
          saveHistory(entry,history, text);
          g_free(history);

          history = g_build_filename (GREP_HISTORY);
          entry = GTK_ENTRY(g_object_get_data(G_OBJECT(mainBox_), "grep_entry"));
          buffer = gtk_entry_get_buffer(entry);
          text = gtk_entry_buffer_get_text(buffer);
          saveHistory(entry,history, text);
          g_free(history);

          history = g_build_filename (PATH_HISTORY);
          entry = GTK_ENTRY(g_object_get_data(G_OBJECT(mainBox_), "path_entry"));
          buffer = gtk_entry_get_buffer(entry);
          text = gtk_entry_buffer_get_text(buffer);
          saveHistory(entry,history, text);
          g_free(history);
      }

private:

      GList *
      loadHistory (const gchar *history) {
        GList *list = NULL;
        if (!history || !g_file_test(history, G_FILE_TEST_EXISTS)){
          TRACE("loadHistory(): creating new history: \"%s\"\n", history);
          list = g_list_prepend(list, (void *)"");
          return list;
        }
        FILE *file = fopen (history, "r");
        if(!file) {
           ERROR_("*** Error::loadHistory(): unable to open history: \"%s\"\n", history);
           list = g_list_prepend(list, (void *) "");
           return list;
        }
        gchar line[2048];
        memset (line, 0, 2048);
        while(fgets (line, 2047, file) && !feof (file)) {
            if(strchr (line, '\n')) *strchr (line, '\n') = 0;
            gchar *newline = compact_line(line);
            if(strlen (newline) == 0) {
              g_free(newline);
              continue;
            }
            list = g_list_prepend(list, newline);
            TRACE("%s: %s\n", history, newline);
        }
        fclose (file);
        if (list) list = g_list_reverse(list);
        return list;
      }


      gchar *
      compact_line(const gchar *line){
          //1. Remove leading and trailing whitespace
          //2. Compact intermediate whitespace

          gchar *newline= g_strdup(line); 
          g_strstrip(newline);
          gchar *p = newline;
          for(;p && *p; p++){
              if (*p ==' ') g_strchug(p+1);
          }
          return newline;
      }

      void freeHistoryList(GList *list){
        if (!list) return;
        for (auto l=list; l && l->data; l=l->next) g_free(l->data);
        g_list_free(list);
      }

     
      const char *iconName(void){ return EMBLEM_FIND;}


      GtkWidget *cbox(void){
        return gtk_label_new("bar");
      }
      GtkBox *_mainBox(void) { return mainBox_;}

      GtkBox *_mainBox(const char *folder) {
          if (g_file_test(folder, G_FILE_TEST_IS_DIR)) folder_ = realpath(folder, NULL);
          else {
            if (g_file_test(Child::getWorkdir(), G_FILE_TEST_IS_DIR)) folder_ = g_strdup(Child::getWorkdir());
            else folder_ = g_strdup(g_get_home_dir());
          }
          mainBox_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
          g_object_set_data(G_OBJECT(mainBox_), "object", this);
          gtk_widget_set_vexpand(GTK_WIDGET(mainBox_), false);
          gtk_widget_set_hexpand(GTK_WIDGET(mainBox_), false);
          mkVpane();
          mkFilterEntry();
          ////////////////  grep options.... /////////////////////////
          mkGrepEntry();
          gtk_widget_set_size_request(GTK_WIDGET(mainBox_), 600, 460);
          postRealize();
          return mainBox_;
      }

private:

     void postRealize(void){
          // Post realize... WTF
          auto buffer = gtk_entry_get_buffer(grepEntry_);
          const char *text = gtk_entry_buffer_get_text(buffer);
          active_grep_ =  text && strlen(text); 
          TRACE("*** entry=%p, buffer=%p text=\"%s\" active_grep=%d\n", 
              grepEntry_, buffer, text, active_grep_);
          auto optionsBox = GTK_WIDGET(g_object_get_data(G_OBJECT(grepEntry_), "optionsBox"));
          gtk_widget_set_sensitive(GTK_WIDGET(optionsBox), active_grep_);
            /// FIXME: the following does not do the trick
            //         crashes with cannot access memory address 0x12 for optionbox
            //         Probably must come after window mapped...          
          //g_signal_connect(G_OBJECT(buffer), "inserted-text", G_CALLBACK(sensitivize), GTK_WIDGET(optionsBox));
          //g_signal_connect(G_OBJECT(buffer), "deleted-text", G_CALLBACK(sensitivize), GTK_WIDGET(optionsBox));
          


     }
          
      void mkVpane(void){
          frame_ = GTK_FRAME(gtk_frame_new(NULL));
          gtk_box_append(mainBox_, GTK_WIDGET(frame_));

          vpane_ = GTK_PANED(gtk_paned_new(GTK_ORIENTATION_VERTICAL));
          gtk_widget_set_vexpand(GTK_WIDGET(vpane_), true);
          gtk_paned_set_wide_handle (vpane_,TRUE);
          gtk_frame_set_child(frame_, GTK_WIDGET(vpane_));

          topPaneVbox_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 2));
          gtk_widget_set_vexpand(GTK_WIDGET(topPaneVbox_), true);
          auto advanced = advancedOptions();

          TRACE("mkTopPaneHbox  <- \n");
          mkTopPaneHbox();
          gtk_box_append(topPaneVbox_, GTK_WIDGET(topPaneHbox_));
          
          g_object_set_data(G_OBJECT(mainBox_), "vpane", vpane_);

          auto sw = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new());
          gtk_paned_set_start_child (vpane_, GTK_WIDGET(sw));

          notebook_ = GTK_NOTEBOOK(gtk_notebook_new());
          gtk_notebook_append_page(notebook_, GTK_WIDGET(topPaneVbox_), gtk_label_new(_("Options")));
          gtk_scrolled_window_set_child(sw, GTK_WIDGET(notebook_));

          gtk_notebook_append_page(notebook_, GTK_WIDGET(advanced), gtk_label_new(_("Advanced options")));

          ////////////   findButton... /////////////////////////
          auto actionBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3));
          //gtk_frame_set_label_widget(frame_, GTK_WIDGET(actionBox));
          gtk_notebook_set_action_widget(notebook_, GTK_WIDGET(actionBox), GTK_PACK_END);


   /*       auto t=g_strdup_printf("<span color=\"blue\" size=\"large\"><b><u>%s</u></b></span>  ", _("Find in Files"));
          auto label = GTK_LABEL(gtk_label_new (t));
          gtk_label_set_use_markup (label, TRUE);
          g_free(t);
          gtk_box_append(actionBox, GTK_WIDGET(label));*/

          findLabel_ = Basic::hyperLabelLarge(_("Find in Files"),
              (void *)findLabelClick,
              (void *)this);
          gtk_box_append(actionBox, GTK_WIDGET(findLabel_));
          
          

          findButton_ = GTK_WIDGET(UtilBasic::mkButton(EMBLEM_FIND, NULL));
          g_object_set_data(G_OBJECT(findButton_), "dialog", dialog_);
          Basic::setTooltip(GTK_WIDGET(findButton_), _("Show search results for this query"));
          g_signal_connect (G_OBJECT (findButton_), "clicked",
                  BUTTON_CALLBACK(FindSignals<Type>::onFindButton), (gpointer)this);
      /* crash
          findButton_ = Dialog::buttonBox(EMBLEM_FIND,
              _("Show search results for this query"),
              (void *)FindSignals<Type>::onFindButton,
              (void *)this);
              */

          cancelButton_ = UtilBasic::mkButton(EMBLEM_DELETE, NULL);
          
          // gtk_widget_set_can_default(GTK_WIDGET(findButton_), TRUE);
          g_signal_connect (G_OBJECT (cancelButton_), "clicked",
                  BUTTON_CALLBACK(FindSignals<Type>::onCancelButton), (gpointer)this);  
          gtk_box_append(actionBox, GTK_WIDGET(findButton_));
          gtk_box_append(actionBox, GTK_WIDGET(cancelButton_));
          gtk_widget_set_visible(GTK_WIDGET(cancelButton_), false);
        
         /////////////  clear button  /////
         auto clearButton =  UtilBasic::mkButton(EMBLEM_CLEAR, NULL);
         // no work: gtk_widget_set_size_request(GTK_WIDGET(clearButton), 33, -1);
         g_signal_connect (G_OBJECT (clearButton), "clicked",
                  BUTTON_CALLBACK(FindSignals<Type>::onClearButton), (gpointer)this);
         g_object_set_data(G_OBJECT(mainBox_), "clear_button", clearButton);
         g_object_set_data(G_OBJECT(clearButton), "mainBox", mainBox_);
         //gtk_notebook_set_action_widget(notebook_, GTK_WIDGET(clearButton), GTK_PACK_END);
         

         Basic::setTooltip(GTK_WIDGET(cancelButton_), _("Cancel Operation"));

         gtk_scrolled_window_set_policy (sw, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

         textview_ = GTK_TEXT_VIEW(gtk_text_view_new ());
         g_object_set_data(G_OBJECT(mainBox_), "textview", textview_);
         g_object_set_data(G_OBJECT(textview_), "vpane", (gpointer)vpane_);
          
         auto sw2 = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new ());
         auto frame2 = GTK_FRAME(gtk_frame_new(NULL));
         gtk_frame_set_child(frame2, GTK_WIDGET(sw2));
         gtk_frame_set_label_widget(frame2, GTK_WIDGET(clearButton));
         gtk_frame_set_label_align(frame2, 1.0);
         
         gtk_paned_set_end_child (vpane_, GTK_WIDGET(frame2));
         //gtk_paned_set_end_child (vpane_, GTK_WIDGET(sw2));
         gtk_scrolled_window_set_policy (sw2, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
         gtk_scrolled_window_set_child(sw2, GTK_WIDGET(textview_));

         gtk_widget_set_can_focus(GTK_WIDGET(textview_), FALSE);
         gtk_text_view_set_wrap_mode (textview_, GTK_WRAP_WORD);
         gtk_text_view_set_cursor_visible (textview_, FALSE);
          
         gtk_paned_set_position(vpane_, 3000);
         return ;
      }

      void mkTopPaneHbox(){
          topPaneHbox_= GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
          gtk_widget_set_hexpand(GTK_WIDGET(topPaneHbox_), TRUE);

          auto vbox1 = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
          auto vbox2 = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
          auto vbox3 = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));

          gtk_box_append(topPaneHbox_, GTK_WIDGET(vbox1));
          gtk_box_append(topPaneHbox_, GTK_WIDGET(vbox3));


      }

      static gboolean
      findLabelClick(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              void *data){
        FindSignals<Type>::onFindButton(NULL, data);
        return true;
      }

      void mkPathEntry(GtkBox *parentBox){
          GtkWidget *path_label;
          auto text=g_strdup_printf("%s:", _("Path"));
          auto path_entry = FileResponse<Type, subClass_t>::addEntry(parentBox, "entry1", text, this);
          g_object_set_data(G_OBJECT(mainBox_), "path_entry", path_entry);
          g_free(text);

          auto buffer = gtk_entry_get_buffer(GTK_ENTRY(path_entry));  
          TRACE("folder_ = %s\n", folder_);
          gtk_entry_buffer_set_text(buffer, folder_, -1);
          g_signal_connect (path_entry,
                            "activate", BUTTON_CALLBACK(FindSignals<Type>::onFindButton), 
                            this);
      }

      // simple entry, no filechooser.
       GtkEntry *addEntry(GtkBox *child, const char *id, const char *text){
          auto hbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
          gtk_widget_set_vexpand(GTK_WIDGET(hbox), false);
          gtk_widget_set_hexpand(GTK_WIDGET(hbox), true);
          auto label = gtk_label_new(text);
          gtk_widget_set_hexpand(GTK_WIDGET(label), false);
          auto entry = gtk_entry_new();

          gtk_widget_set_hexpand(GTK_WIDGET(entry), true);
          g_object_set_data(G_OBJECT(child), id, entry);

          gtk_box_append(hbox, label);
          gtk_box_append(hbox, entry);
          gtk_box_append(child, GTK_WIDGET(hbox));
          return GTK_ENTRY(entry);
        }

       GtkBox *entryBox(const char *labelText, const char *tooltipText, const char *history) {
          auto box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
          auto label = GTK_LABEL(gtk_label_new (labelText));
          
          auto entry = GTK_ENTRY(gtk_entry_new());     
          gtk_box_append(box, GTK_WIDGET(label));
          gtk_box_append(box, GTK_WIDGET(entry));

          g_object_set_data(G_OBJECT(box), "entry", entry);
          auto buffer = gtk_entry_get_buffer(entry);
          
          gtk_widget_set_hexpand(GTK_WIDGET(entry), true);
          if (!history) return box;

          GList *list = loadHistory(history);
          auto vector = historyVector(list);
          g_object_set_data(G_OBJECT(entry), "list", list);

          auto buttonBox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
          g_object_set_data(G_OBJECT(entry), "buttonBox", buttonBox);

          auto dropdown = gtk_drop_down_new_from_strings(vector);
          g_object_set_data(G_OBJECT(entry), "dropdown", dropdown);

          gtk_entry_buffer_set_text(buffer, vector[0], -1);
          g_free(vector);
          g_signal_connect(G_OBJECT(dropdown), "notify", G_CALLBACK(notify), entry);
          gtk_box_append(buttonBox, GTK_WIDGET(dropdown));
          gtk_box_append(box, GTK_WIDGET(buttonBox));

          auto gesture1 = gtk_gesture_click_new();
          gtk_event_controller_set_propagation_phase (GTK_EVENT_CONTROLLER(gesture1),GTK_PHASE_CAPTURE);
          gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture1),1);
          g_signal_connect (G_OBJECT(gesture1) , "pressed", 
              EVENT_CALLBACK (ddClick1), entry);
          gtk_widget_add_controller (dropdown, GTK_EVENT_CONTROLLER(gesture1));   

          auto gesture2 = gtk_gesture_click_new();
          gtk_event_controller_set_propagation_phase (GTK_EVENT_CONTROLLER(gesture2),GTK_PHASE_CAPTURE);
          gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture2),1);
          g_signal_connect (G_OBJECT(gesture2) , "released", 
              EVENT_CALLBACK (ddClick2), entry);
          gtk_widget_add_controller (dropdown, GTK_EVENT_CONTROLLER(gesture2));   

          addKeyController1(entry, (void *)dropdown);
          addKeyController2(entry, (void *)dropdown);
     
          if (tooltipText) {
            auto image = Texture<bool>::getImage(EMBLEM_QUESTION, 18);
            gtk_box_append(box, GTK_WIDGET(image));
            Basic::setTooltip(GTK_WIDGET(image), tooltipText);
          }
          g_signal_connect (entry,
                  "activate", BUTTON_CALLBACK(FindSignals<Type>::onFindButton), 
                  this);

          return box;
      }

        GtkEntry *mkCompletionEntry(GList **list_p){ 
            auto entry = GTK_ENTRY(gtk_entry_new());
            gtk_widget_set_hexpand(GTK_WIDGET(entry), true);
            g_object_set_data(G_OBJECT(entry), "list_p", list_p);
            return entry;
        }
        
        GtkBox *grepOptions(GtkEntry *grep_entry){
            auto optionsBox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10));
            auto vbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 6));
            auto checkBox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
            opt_t options[] = {
              {_("Case Sensitive"),"case_sensitive",default_case_sensitive},
              {_("Extended regexp"), "ext_regexp",default_ext_regexp},
              {_("Include binary files"),"look_in_binaries",default_look_in_binaries},
              {_("Line Count"),"line_count",default_line_count},
              {NULL,NULL,0}};
            for (opt_t *p=options; p->text != NULL; p++) {
              mkGrepOption(p, grep_entry, checkBox);
            }
               
            auto radioBox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
            GSList *radioGroup = NULL;
            opt_t rOptions[] = {
              {_("Anywhere"),"anywhere",default_anywhere},
              {_("Whole words only"), "match_words",default_match_words},
              {_("lines"),"match_lines",default_match_lines},
              {_("No match"),"match_no_match",default_match_no_match},
              {NULL,NULL,0}};
            for (opt_t *p=rOptions; p->text != NULL; p++) {
              mkGrepRadio(p, grep_entry, &radioGroup, radioBox);
            }
            gtk_box_append(optionsBox, GTK_WIDGET(checkBox));
            gtk_box_append(optionsBox, GTK_WIDGET(radioBox));
            return optionsBox;

        }
        void mkFilterEntry(void){

            auto frame = GTK_FRAME(gtk_frame_new(""));
            auto label = GTK_LABEL(gtk_label_new(""));
            auto markup = g_strconcat("<span color=\"green\"><b>",_("Options")," (glob)","</b></span>", NULL);
            gtk_label_set_markup(label, markup);
            auto vbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
            gtk_frame_set_label_widget(frame, GTK_WIDGET(label));
            gtk_frame_set_child(frame, GTK_WIDGET(vbox));
            gtk_box_append(topPaneVbox_, GTK_WIDGET(frame));

            mkPathEntry(vbox);

            auto history = g_build_filename(FILTER_HISTORY);              
            auto box = entryBox(_("Filter:"), filter_text_help(), history);
            g_free(history);
            auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(box), "entry"));
            g_object_set_data(G_OBJECT(mainBox_), "filter_entry", entry);

            gtk_box_append(vbox, GTK_WIDGET(box));
            auto recursive = simpleCheck(vbox, _("Recursive"));
            auto hidden = simpleCheck(vbox, _("Find hidden files and directories"));
            gtk_check_button_set_active(hidden, false);
            auto xdev = simpleCheck(vbox, _("Stay on single filesystem"));

          char *text = g_strdup_printf(" (%s):",  _("Upper limit"));
          auto results = add_option_spin(vbox, _("Results"), "upper_limit_spin", text, result_limit);
          g_free(text);
          gtk_check_button_set_active(results, true);

          

        }

        void mkGrepEntry(void){
            auto frame = GTK_FRAME(gtk_frame_new(""));
            auto label = GTK_LABEL(gtk_label_new(""));
            auto markup = g_strconcat("<span color=\"green\"><b>",_("Options")," (grep)","</b></span>", NULL);
            gtk_label_set_markup(label, markup);
            auto vbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
            gtk_frame_set_label_widget(frame, GTK_WIDGET(label));
            gtk_frame_set_child(frame, GTK_WIDGET(vbox));
            gtk_box_append(topPaneVbox_, GTK_WIDGET(frame));

            auto history = g_build_filename(GREP_HISTORY);              
            auto box = entryBox(_("Contains the text"), grep_text_help(), history);
            g_free(history);
            grepEntry_ = GTK_ENTRY(g_object_get_data(G_OBJECT(box), "entry"));
            g_object_set_data(G_OBJECT(mainBox_), "grep_entry", grepEntry_);

            auto optionsBox = grepOptions(grepEntry_);
            g_object_set_data(G_OBJECT(grepEntry_), "optionsBox", optionsBox);
            g_object_set_data(G_OBJECT(grepEntry_), "sWidget", optionsBox);
            gtk_widget_set_sensitive(GTK_WIDGET(optionsBox), false);

            gtk_box_append(vbox, GTK_WIDGET(box));
            gtk_box_append(vbox, GTK_WIDGET(optionsBox));
        }

        void mkGrepOption(opt_t *opt, GtkEntry *grep_entry, GtkBox *checkBox){
            auto check = GTK_CHECK_BUTTON(gtk_check_button_new_with_label (opt->text));
            gtk_check_button_set_active (check, opt->defaultValue);
            g_object_set_data(G_OBJECT(mainBox_), opt->id, check);
            gtk_box_append(checkBox, GTK_WIDGET(check));
        }


        void mkGrepRadio(opt_t *opt, GtkEntry *grep_entry, GSList **group, GtkBox *radioBox){
            auto radio = GTK_CHECK_BUTTON(gtk_check_button_new_with_label(opt->text));
            
            gtk_check_button_set_group(radio, firstGrepRadio_);
            if (!firstGrepRadio_) firstGrepRadio_ = radio;
            
            g_object_set_data(G_OBJECT(mainBox_), opt->id, radio);

            if (opt->defaultValue) {
                gtk_check_button_set_active (radio, TRUE);
            }
            // FIXME g_signal_connect (G_OBJECT (grep_entry), "event", 
                   // KEY_EVENT_CALLBACK (on_key_release), (gpointer) radio);
            gtk_box_append(radioBox, GTK_WIDGET(radio));  
        }
        

        GtkBox *boxButton(const char *iconName, void *callback){
            auto cbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
            gtk_widget_set_hexpand(GTK_WIDGET(cbox), true);
            gtk_widget_set_halign(GTK_WIDGET(cbox), GTK_ALIGN_CENTER);
            auto button =  UtilBasic::mkButton(iconName, NULL);
            g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK(callback), this);
            gtk_box_append(cbox, GTK_WIDGET(button));
            return cbox;
        }

    GtkCheckButton *simpleCheck(GtkBox *parentBox, const char *checkName){
      auto box = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3));
      auto check = GTK_CHECK_BUTTON(gtk_check_button_new());
      auto label = gtk_label_new(checkName);
      g_object_set_data(G_OBJECT(mainBox_), checkName, GTK_WIDGET(check));
      //g_signal_connect(G_OBJECT(check), "toggled", G_CALLBACK(sensitivizeSpin), label);
      gtk_check_button_set_active(check,true);
      gtk_box_append(box, GTK_WIDGET(check));
      gtk_box_append(box, GTK_WIDGET(label));
      gtk_box_append(parentBox, GTK_WIDGET(box));
      return check;
    }
        
    GtkFrame *fileSizeFrame(GtkBox *parentBox){
            auto action = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3));
            auto check = simpleCheck(action, _("File size"));
            gtk_check_button_set_active(check, false);
            auto frame = GTK_FRAME(gtk_frame_new(NULL));
            gtk_frame_set_label_widget(frame, GTK_WIDGET(action));
            auto vbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 3));
            gtk_frame_set_child(frame, GTK_WIDGET(vbox));
            gtk_box_append(parentBox, GTK_WIDGET(frame));
            gtk_frame_set_label_align(frame, 0);
            g_signal_connect(G_OBJECT(check), "toggled", G_CALLBACK(sensitivizeSpin), vbox);
            gtk_widget_set_sensitive(GTK_WIDGET(vbox), false); // default


            // option -s +KByte "size_greater", "size_greater_spin"
            auto text = g_strdup_printf("%s (%s): ", _("At Least"), _("kBytes"));
            auto radioS1 = add_option_spin(vbox, "", "size_greater_spin", text, size_greater);
            //gtk_check_button_set_group(radioS1, radioS1);
            gtk_check_button_set_active(radioS1, true);
            g_free(text);
            
            // option -s -KByte "size_smaller", "size_smaller_spin"
            text = g_strdup_printf("%s (%s): ", _("At Most"), _("kBytes"));
            auto radioS2 = add_option_spin(vbox, "", "size_smaller_spin", text, size_smaller);
            gtk_check_button_set_group(radioS2, radioS1);
            g_free(text);
            return frame;
    }
      GtkFrame *mkTimeFrame(GtkBox *parentBox){
            auto timeAction = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3));
            auto timeCheck = simpleCheck(timeAction, _("Specific Time")); 
            gtk_check_button_set_active(timeCheck, false);

            auto timeFrame = GTK_FRAME(gtk_frame_new(NULL));
            gtk_frame_set_label_widget(timeFrame, GTK_WIDGET(timeAction));
            auto timeBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 3));
            gtk_frame_set_child(timeFrame, GTK_WIDGET(timeBox));
            gtk_frame_set_label_align(timeFrame, 0);

            gtk_box_append(parentBox, GTK_WIDGET(timeFrame));
            add_option_radio2(timeBox, _("Modified"),_("Created"),_("Accessed"), NULL);
            auto radio1 = add_option_spin(timeBox, _("Minutes"), "last_minutes_spin",  ": ", last_minutes);
            gtk_check_button_set_active(radio1, true);
            
           // option -h hours "last_hours", "last_hours_spin"
            auto radio2 = add_option_spin(timeBox, _("Hours"), "last_hours_spin",   ": ", last_hours);
            gtk_check_button_set_group(radio2, radio1);
            
            // option -d days "last_days", "last_days_spin"
            auto radio3 = add_option_spin(timeBox, _("Days"),"last_days_spin",   ": ", last_days);
            gtk_check_button_set_group(radio3, radio1);
            
            // option -m months "last_months", "last_months_spin"
            auto radio4 = add_option_spin(timeBox,  _("Months"), "last_months_spin",  ": ", last_months);
            gtk_check_button_set_group(radio4, radio1);
           
            gtk_widget_set_sensitive(GTK_WIDGET(timeBox), false);
            TRACE("*** timeBox=%p\n", timeBox);
            g_signal_connect(G_OBJECT(timeCheck), "toggled", G_CALLBACK(sensitivizeSpin), timeBox);
            return timeFrame;
      }

    GtkFrame *mkUserFrame(GtkBox *parentBox){
            auto action = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3));
            auto check = simpleCheck(action, _("File properties"));
            gtk_check_button_set_active(check, false);
            auto frame = GTK_FRAME(gtk_frame_new(NULL));
            gtk_frame_set_label_widget(frame, GTK_WIDGET(action));
            auto vbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 3));
            gtk_frame_set_child(frame, GTK_WIDGET(vbox));
            gtk_box_append(parentBox, GTK_WIDGET(frame));
            gtk_frame_set_label_align(frame, 0);
            g_signal_connect(G_OBJECT(check), "toggled", G_CALLBACK(sensitivizeSpin), vbox);
            gtk_widget_set_sensitive(GTK_WIDGET(vbox), false); // default

            auto slist = get_user_slist();
            // option -u uid "uid" "uid_combo"
            auto radio1 = add_option_combo(vbox, "uid", "uid_combo", _("User"), slist);
            slist = free_string_slist(slist);
            gtk_check_button_set_active(radio1, true);

            // option -g gid "gid" "gid_combo"
            slist = get_group_slist();
            auto radio2 = add_option_combo(vbox, "gid", "gid_combo", _("Group"), slist);
            slist = free_string_slist(slist);
            gtk_check_button_set_group(radio2, radio1);
            
            // option -o octal "octal_p" "permissions_entry"
            auto radio3 = add_option_entry(vbox, "octal_p", "permissions_entry", _("Octal Permissions"), "0666");
            auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(mainBox_), "permissions_entry"));
            gtk_widget_set_size_request (GTK_WIDGET(entry), 75, -1);
            gtk_check_button_set_group(radio3, radio1);
            
            // option -p suid | exe 
           // auto privilege = simpleCheck(parentBox, _("Privileges"));
            auto privBox = add_option_radio2(vbox, _("Executable"),_("SUID"), NULL);
            auto radio4 = GTK_CHECK_BUTTON(g_object_get_data(G_OBJECT(mainBox_), _("Executable")));
            //gtk_check_button_set_group(radio4, radio1);
            gtk_check_button_set_active(radio4, false);

            auto radio5 = GTK_CHECK_BUTTON(g_object_get_data(G_OBJECT(mainBox_), _("SUID")));
            gtk_check_button_set_group(radio5, radio1);
            gtk_check_button_set_active(radio5, false);

           // gtk_widget_set_sensitive(GTK_WIDGET(privBox), false);
           // g_signal_connect(G_OBJECT(privilege), "toggled", G_CALLBACK(sensitivizeSpin), privBox);
           // gtk_check_button_set_active(privilege, false);


            return frame;
    }

        GtkFrame *advancedOptions(void){
            auto frame = GTK_FRAME(gtk_frame_new(""));
            auto label = GTK_LABEL(gtk_label_new(""));
            auto markup = g_strconcat("<span color=\"red\"><b>",_("Advanced options")," (glob)","</b></span>", NULL);
            gtk_label_set_markup(label, markup);
            auto vbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 3));
            gtk_frame_set_label_widget(frame, GTK_WIDGET(label));
            gtk_frame_set_child(frame, GTK_WIDGET(vbox));

          
            gtk_widget_set_vexpand(GTK_WIDGET(vbox), FALSE);

            auto hbox17 = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3));
            gtk_box_append(vbox, GTK_WIDGET(hbox17));

            auto left_options_vbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 3));
            gtk_box_append(hbox17, GTK_WIDGET(left_options_vbox));
            auto center_options_vbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 3));
            gtk_box_append(hbox17, GTK_WIDGET(center_options_vbox));
          TRACE("advancedOptions  <- 1\n");
         
            auto hbox21 = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3));
            gtk_box_append(left_options_vbox, GTK_WIDGET(hbox21));

            auto label37 = GTK_LABEL(gtk_label_new (_("File type : ")));
            gtk_box_append(hbox21, GTK_WIDGET(label37));

            auto file_type_om =  GTK_DROP_DOWN(gtk_drop_down_new_from_strings(ftypes()));
            gtk_box_append(hbox21, GTK_WIDGET(file_type_om));
            g_object_set_data(G_OBJECT(mainBox_), "file_type_om", file_type_om);
            gtk_drop_down_set_selected(file_type_om, 0);

            fileSizeFrame(center_options_vbox);

          TRACE("advancedOptions  <- 4\n");
            mkUserFrame(center_options_vbox);
            // option -M -A -C
            mkTimeFrame(left_options_vbox);
            
            return GTK_FRAME(frame);
        }

        GtkCheckButton *
        add_option_spin(
                GtkBox *options_vbox, 
                const gchar *check_name,
                const gchar *spin_name,
                const gchar *text,
                gint default_value)
        {

            if ((!spin_name && !check_name)|| !options_vbox) {
                ERROR_("add_option_spin(): incorrect function call\n");
                return NULL;
            }
            auto hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
            gtk_widget_set_hexpand(GTK_WIDGET(hbox), FALSE);
            gtk_box_append(options_vbox, GTK_WIDGET(hbox));

            auto size_hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
            GtkCheckButton *check = GTK_CHECK_BUTTON(gtk_check_button_new_with_label(check_name));
            g_object_set_data(G_OBJECT(mainBox_), check_name, (gpointer)check);
            gtk_box_append(hbox, GTK_WIDGET(check));
            g_signal_connect (G_OBJECT (check), "toggled", G_CALLBACK(sensitivizeSpin), (gpointer)size_hbox);
            gtk_widget_set_sensitive(GTK_WIDGET(size_hbox), FALSE);
            gtk_box_append(hbox, GTK_WIDGET(size_hbox));
           
            GtkLabel *label = NULL;
            if (text)  label = GTK_LABEL(gtk_label_new (text));
            else label = GTK_LABEL(gtk_label_new (": "));
            gtk_box_append(size_hbox, GTK_WIDGET(label));
            auto spinbutton_adj = GTK_ADJUSTMENT(gtk_adjustment_new (default_value, 0, 4096*4096, 1, 64, 0));
            auto spinbutton = GTK_SPIN_BUTTON(gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton_adj), 0, 0));
            gtk_box_append(size_hbox, GTK_WIDGET(spinbutton));
            gtk_spin_button_set_update_policy (spinbutton, GTK_UPDATE_IF_VALID);
            g_object_set_data(G_OBJECT(mainBox_), spin_name, (gpointer)spinbutton);
            gtk_widget_set_size_request (GTK_WIDGET(spinbutton), 75, -1);

            return (check);
        }

        
        GSList *
        get_user_slist(void){
            GSList *g_user = NULL;
            struct passwd *pw;
            while((pw = getpwent ()) != NULL) {
                g_user = g_slist_append (g_user, g_strdup (pw->pw_name));
            }
            g_user = g_slist_sort (g_user, (GCompareFunc) strcasecmp);
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
            g_group = g_slist_sort (g_group, (GCompareFunc) strcasecmp);
            gr = getgrgid (geteuid ());
            auto buf = g_strdup_printf ("%s", gr ? gr->gr_name : _("unknown"));
            g_group = g_slist_prepend (g_group, buf);
            return g_group;
        }


        GtkCheckButton *
        add_option_entry(GtkBox *options_vbox, 
                const gchar *check_name,
                const gchar *entry_name,
                const gchar *text,
                const gchar *default_value)
        {
            if ((!entry_name && !check_name)|| !options_vbox) {
                ERROR_("add_option_entry(): incorrect function call\n");
                return NULL;
            }
            auto hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
            gtk_box_append(options_vbox, GTK_WIDGET(hbox));

            auto size_hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
            GtkCheckButton *check = NULL;
            if (check_name) {
                check = GTK_CHECK_BUTTON(gtk_check_button_new());
                g_object_set_data(G_OBJECT(mainBox_), check_name, check);
                gtk_box_append(hbox, GTK_WIDGET(check));
                g_signal_connect (G_OBJECT (check), "toggled", 
                        G_CALLBACK(sensitivizeSpin), size_hbox);
                gtk_widget_set_sensitive(GTK_WIDGET(size_hbox), FALSE);
            }
            gtk_box_append(hbox, GTK_WIDGET(size_hbox));

            if (text) {
                auto label = GTK_LABEL(gtk_label_new (text));
                gtk_box_append(size_hbox, GTK_WIDGET(label));
            }
            
            if (entry_name) {
                auto label = GTK_LABEL(gtk_label_new (": "));
                gtk_box_append(size_hbox, GTK_WIDGET(label));
                
                auto entry = GTK_ENTRY(gtk_entry_new());
                gtk_box_append(size_hbox, GTK_WIDGET(entry));
                g_object_set_data(G_OBJECT(mainBox_), entry_name, entry);
                auto buffer = gtk_entry_get_buffer(entry);
                gtk_entry_buffer_set_text (buffer, default_value, -1);                          
            }

            if (check) return GTK_CHECK_BUTTON(check);
            return NULL;
        }

   

        GtkBox *
        add_option_radio2(
                GtkBox *parentBox, 
                ...){

            auto hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
            gtk_box_append(parentBox, GTK_WIDGET(hbox));

            va_list ap;

            GtkCheckButton *firstRadio = NULL;
            va_start (ap, parentBox);
            do {
              const char *radioText = va_arg (ap, const char *);
              if (radioText == NULL) break;

              auto radio = GTK_CHECK_BUTTON(gtk_check_button_new_with_label (radioText));
              if (!firstRadio) {
                firstRadio = radio;
                gtk_check_button_set_active(radio, true);
              } else gtk_check_button_set_group(radio, firstRadio);
              gtk_box_append(hbox, GTK_WIDGET(radio));
              g_object_set_data(G_OBJECT(mainBox_), radioText, radio);

            } while (true);

            return hbox;
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

        GtkCheckButton *
        add_option_combo(
                GtkBox *options_vbox, 
                const gchar *check_name,
                const gchar *combo_name,
                const gchar *text,
                GSList *list)
        {
            if ((!combo_name && !check_name)|| !options_vbox) {
                ERROR_("add_option_spin(): incorrect function call\n");
                return NULL;
            }
            auto hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
            gtk_box_append(options_vbox, GTK_WIDGET(hbox));

            auto size_hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
            GtkCheckButton *check = NULL;
            if (check_name) {
                check = GTK_CHECK_BUTTON(gtk_check_button_new());
                g_object_set_data(G_OBJECT(mainBox_), check_name, check);
                gtk_box_append(hbox, GTK_WIDGET(check));
                g_signal_connect (G_OBJECT (check), "toggled", 
                        G_CALLBACK(sensitivizeSpin), size_hbox);
                gtk_widget_set_sensitive(GTK_WIDGET(size_hbox), FALSE);
            }
            gtk_box_append(hbox, GTK_WIDGET(size_hbox));

            if (text) {
                auto label = GTK_LABEL(gtk_label_new (text));
                gtk_box_append(size_hbox, GTK_WIDGET(label));
                label = GTK_LABEL(gtk_label_new (": "));
                gtk_box_append(size_hbox, GTK_WIDGET(label));
           }

            if (combo_name) {
                auto strings = (const char **)calloc(g_slist_length(list)+1, sizeof (char *));
                int k=0;
                for (auto l=list; l && l->data; l=l->next, k++){
                  strings[k] = (char *)l->data;
                }
                auto combo = GTK_DROP_DOWN(gtk_drop_down_new_from_strings(strings));
                g_free(strings);
                gtk_drop_down_set_selected(combo, 0);
                
                gtk_box_append(size_hbox, GTK_WIDGET(combo));
                g_object_set_data(G_OBJECT(mainBox_), combo_name, combo);
                gtk_widget_set_size_request (GTK_WIDGET(combo), 120, -1);
            }
            if (check) return GTK_CHECK_BUTTON(check);
            return NULL;
        }



      };


}
#endif
