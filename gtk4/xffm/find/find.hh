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

#define FILTER_HISTORY g_get_user_data_dir(),"xffm+","xffind.filter",NULL
#define GREP_HISTORY g_get_user_data_dir(),"xffm+","xffind.grep",NULL
#define PATH_HISTORY g_get_user_data_dir(),"xffm+","xffind.path",NULL

typedef struct radio_t {
    GtkBox *box;
    GtkCheckButton *toggle[5];
} radio_t;

static const gchar *ftypes[] = {
    N_("Regular"),
    N_("Directory"),
    N_("Symbolic Link"),
    N_("Socket"),
    N_("Block device"),
    N_("Character device"),
    N_("FIFO"),
    N_("Any"),
    NULL
};
const char *ft[] = {
    "reg",
    "dir",
    "sym",
    "sock",
    "blk",
    "chr",
    "fifo",
    "any",
    NULL
};

typedef struct fgrData_t{
    GtkWindow *dialog;
    pid_t pid;
    gint resultLimit;
    gint resultLimitCounter;
    GSList *findList;
    gchar **argument;
    gboolean done;
}fgrData_t;

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
      gboolean default_anywhere=TRUE;
      gboolean default_match_words=FALSE;
      gboolean default_match_lines=FALSE;
      gboolean default_match_no_match=FALSE;
      GSList *find_list = NULL;
      gchar  *last_workdir = NULL;

    
      char *folder_ = NULL;
      bool active_grep_ = false;
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

      GtkEntry *grepEntry_ = NULL;

public:
      const char *label(void){return "xffm::find";}

      FindResponse (void){
      }

      ~FindResponse (void){
        DBG("*** ~FindResponse\n");
        g_free(folder_);
      }
       
      static char **historyVector(GList *list){
        int size = 0;
        for (auto l=list; l && l->data; l=l->next){
          size++;
          if (size == 10) break;
        }
        size++;
        auto vector = (char **)calloc(size+1, sizeof(char *));
        int k=1;
        //vector[0] = (char *)"";
        for (auto l=list; l && l->data; l=l->next, k++){
          vector[k-1] = (char *)l->data;
          if (k == 10) break;
        }
        vector[k-1] = (char *)"";
        return vector;
      }

      GListModel *getDropDownModel(GList *list){
        return NULL;
      }

static void
notify ( GObject* self, GParamSpec *pspec, void *data){
  auto call = g_param_spec_get_name(pspec);
  if (!call) return;
  if (strcmp(call, "selected-item")) return;
  if (g_object_get_data(self, "halt")){
    g_object_set_data(self, "halt", NULL);
    TRACE(" return on halt.\n");
    return;
  }

  
  DBG("*** notify name=%s\n", g_param_spec_get_name(pspec));

  if (!GTK_IS_ENTRY(data)) {
    DBG("*** FIXME: identify notify call\n");
    return; // FIXME hack!
            // notify is called when window is closed
            // and by then the pointer to entry is invalid.
            // Must use pspec or something to distinguish the
            // notify call.
  }
  auto dd = GTK_DROP_DOWN(self); 

  auto item = GTK_STRING_OBJECT(gtk_drop_down_get_selected_item(dd));
  auto selected = gtk_string_object_get_string(item);
  auto buffer = gtk_entry_get_buffer(GTK_ENTRY(data));
  gtk_entry_buffer_set_text(buffer, selected, -1);
  auto sWidget = g_object_get_data(G_OBJECT(data), "sWidget");
  if (sWidget){
    gtk_widget_set_sensitive(GTK_WIDGET(sWidget), (selected && strlen(selected) > 0));
  }

  DBG("notify callback, data=%p, item=%d, selected=%s\n", data,item,selected);
  
}

static void
sensitivize ( GtkEntryBuffer* self, guint position, gchar* chars, guint n_chars, void *data){
  auto box = GTK_WIDGET(data);
  auto text = gtk_entry_buffer_get_text(self);
  gtk_widget_set_sensitive(box, strlen(text)>0);
}

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
           DBG("*** Error::loadHistory(): unable to open history: \"%s\"\n", history);
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


      GtkWidget *cbox(void){
        return gtk_label_new("bar");
      }

      GtkBox *mainBox(const char *folder) {

        /*auto cbox =GTK_BOX(g_object_get_data(G_OBJECT(dialog_), "cbox"));
        //auto foo = gtk_label_new("bar");
        gtk_box_prepend(cbox, foo);*/
        
        
          if (g_file_test(folder, G_FILE_TEST_IS_DIR)) folder_ = realpath(folder, NULL);
          else {
            if (g_file_test(Child::getWorkdir(), G_FILE_TEST_IS_DIR)) folder_ = g_strdup(Child::getWorkdir());
            else folder_ = g_strdup(g_get_home_dir());
          }


          mainBox_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
          gtk_widget_set_vexpand(GTK_WIDGET(mainBox_), false);
          gtk_widget_set_hexpand(GTK_WIDGET(mainBox_), false);
          //gtk_window_set_child(findDialog, GTK_WIDGET(mainBox_));
          DBG("mkVpane  <- \n");
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
          gtk_widget_set_size_request(GTK_WIDGET(mainBox_), 600, 400);

          //gtk_widget_realize(GTK_WIDGET(dialog())); //WTF
          postRealize();
          return mainBox_;
      }

private:

     void postRealize(void){
          // Post realize... WTF
          auto buffer = gtk_entry_get_buffer(grepEntry_);
          const char *text = gtk_entry_buffer_get_text(buffer);
          active_grep_ =  text && strlen(text); 
          DBG("*** entry=%p, buffer=%p text=\"%s\" active_grep=%d\n", 
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
          vpane_ = GTK_PANED(gtk_paned_new(GTK_ORIENTATION_VERTICAL));
          gtk_widget_set_vexpand(GTK_WIDGET(vpane_), true);
          gtk_paned_set_wide_handle (vpane_,TRUE);
          gtk_box_append(mainBox_, GTK_WIDGET(vpane_));

          topPaneVbox_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 2));
          gtk_widget_set_vexpand(GTK_WIDGET(topPaneVbox_), true);
          DBG("advancedOptions  <- \n");
          advancedVbox_ = advancedOptions();

          DBG("mkTopPaneHbox  <- \n");
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

          ////////////   findButton... /////////////////////////
          auto findButton = UtilBasic::mkButton(EMBLEM_FIND, NULL);
          //FIXME gtk_widget_set_can_default(GTK_WIDGET(findButton), TRUE);
          g_signal_connect (G_OBJECT (findButton), "clicked",
                  BUTTON_CALLBACK(findSignals<Type>::onFindButton), (gpointer)this);
//                  BUTTON_CALLBACK(findSignals<Type>::onFindButton), (gpointer)findDialog);
          //gtk_box_append(vbox3, GTK_WIDGET(findButton));
          gtk_notebook_set_action_widget(notebook_, GTK_WIDGET(findButton), GTK_PACK_END);
          Basic::setTooltip(GTK_WIDGET(findButton), _("Show search results for this query"));

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



          auto text=g_strdup_printf("%s:", _("Path"));
          auto path_entry = FileResponse<Type, subClass_t>::addEntry(topPaneVbox_, "entry1", text, this);

          g_free(text);

          //gtk_widget_set_size_request (GTK_WIDGET(path_entry), 50, -1);
          // FIXME: g_object_set_data(G_OBJECT(findDialog), "path_entry", path_entry);

          auto buffer = gtk_entry_get_buffer(GTK_ENTRY(path_entry));  
          DBG("folder_ = %s\n", folder_);
          gtk_entry_buffer_set_text(buffer, folder_, -1);



          g_signal_connect (path_entry,
                            "activate", BUTTON_CALLBACK(findSignals<Type>::onFindButton), 
                            (gpointer)this);

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

        static void
        notifyEntry ( GObject* self, GParamSpec *pspec, void *data){
          auto call = g_param_spec_get_name(pspec);
          if (!call) return;

          
          if (strcmp(call, "text")) return;
          auto entry = GTK_ENTRY(self);
          auto dropdown = GTK_DROP_DOWN(data);
          auto buffer = gtk_entry_get_buffer(entry);
          auto text = gtk_entry_buffer_get_text(buffer);
          TRACE("notifyEntry: text=%s\n", text);
          auto list = (GList *)g_object_get_data(G_OBJECT(entry), "list");
          GList *newList = NULL;
          for (auto l=list; l && l->data; l=l->next){
            if (strncasecmp(text, (const char *)l->data, strlen(text)) == 0){
              newList = g_list_append(newList, l->data);
            }
          }

          for (auto l=newList; l && l->data; l=l->next){
            TRACE("newList: %s\n", (const char *)l->data);
          }
          int n = g_list_length(newList);
          char **vector = NULL;
          if (n > 0){
            vector = (char **)calloc(n+1, sizeof(char*));
            int k=0;
            for (auto l=newList; l && l->data; l=l->next,k++){
              vector[k] =  (char *)l->data;
            }
          }
          g_list_free(newList);
          auto sWidget = g_object_get_data(self, "sWidget");
          if (!vector) vector = historyVector(list);
          
          if (sWidget){
            if (strlen(text)) gtk_widget_set_sensitive(GTK_WIDGET(sWidget),true);
            else gtk_widget_set_sensitive(GTK_WIDGET(sWidget), false);
          }

          if (vector) {
            auto model = G_LIST_MODEL (gtk_string_list_new (vector));
            auto button = GTK_DROP_DOWN(g_object_get_data(G_OBJECT(entry), "button"));
            g_object_set_data(G_OBJECT(button), "halt", GINT_TO_POINTER(1));
            gtk_drop_down_set_model(button, model);
            gtk_drop_down_set_selected(button, 0);
          } 


          g_free(vector);



          
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
            if (tooltipText) Basic::setTooltip(GTK_WIDGET(entry), tooltipText);
            if (!history) return box;

            GList *list = loadHistory(history);
            auto vector = historyVector(list);
            //g_object_set_data(G_OBJECT(box), "list", list);
            g_object_set_data(G_OBJECT(entry), "list", list);

            auto buttonBox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
            g_object_set_data(G_OBJECT(entry), "buttonBox", buttonBox);

            auto button = gtk_drop_down_new_from_strings(vector);
            g_object_set_data(G_OBJECT(entry), "button", button);

            gtk_entry_buffer_set_text(buffer, vector[0], -1);
            g_free(vector);
            g_signal_connect(G_OBJECT(button), "notify", G_CALLBACK(notify), entry);
            gtk_box_append(buttonBox, GTK_WIDGET(button));
            gtk_box_append(box, GTK_WIDGET(buttonBox));

            //addKeyController(GTK_WIDGET(entry), (void *)button);
            g_signal_connect(G_OBJECT(entry), "notify", G_CALLBACK(notifyEntry), button);
            



            return box;
        }

  // FIXME: use textview completion.
        GtkEntry *mkCompletionEntry(GList **list_p){ //
            auto entry = GTK_ENTRY(gtk_entry_new());
            gtk_widget_set_hexpand(GTK_WIDGET(entry), true);
            g_object_set_data(G_OBJECT(entry), "list_p", list_p);
         //   addKeyComplete(entry);

            /* FIXME
            auto model = util_c::loadHistory(history);
            g_object_set_data(G_OBJECT(entry), "model", model);
            GtkTreeIter iter;
            if (gtk_tree_model_get_iter_first (model, &iter)){
                gchar *value;
                gtk_tree_model_get (model, &iter, 0, &value, -1);
                auto buffer = gtk_entry_get_buffer(entry)
                gtk_entry_buffer_set_text(entry, value, -1);        
                gtk_editable_select_region (GTK_EDITABLE(entry), 0, strlen(value));
                g_free(value);
            }
            
            auto completion = gtk_entry_completion_new();
            gtk_entry_set_completion (entry, completion);
            gtk_entry_completion_set_model (completion, model);
            gtk_entry_completion_set_popup_completion(completion, TRUE);
            gtk_entry_completion_set_text_column (completion, 0);
                                          
            g_signal_connect (entry, "key_release_event",  G_CALLBACK(on_completion), NULL);*/
                             
            return entry;
        }
        
        GtkBox *grepOptions(GtkEntry *grep_entry){
            auto optionsBox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10));
            auto vbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 6));
            //gtk_box_append(vbox, GTK_WIDGET(button));
            //gtk_box_append(grep_box, GTK_WIDGET(grep_label));
            //gtk_box_append(grep_box, GTK_WIDGET(grep_entry));
            //gtk_box_append(grep_box, GTK_WIDGET(vbox));
            //compat<bool>::boxPack0 (vbox, GTK_WIDGET(button), FALSE, FALSE, 0);
            //compat<bool>::boxPack0 (grep_box, GTK_WIDGET(grep_label), FALSE, FALSE, 0);
            //compat<bool>::boxPack0 (grep_box, GTK_WIDGET(grep_entry), TRUE, TRUE, 0);
            //compat<bool>::boxPack0 (grep_box, GTK_WIDGET(vbox), FALSE, FALSE, 0);

      
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
            auto history = g_build_filename(FILTER_HISTORY);              
            auto box = entryBox(_("Filter:"), _(filter_text_help), history);
            g_free(history);
            auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(box), "entry"));

            gtk_box_append(topPaneVbox_, GTK_WIDGET(box));
        }

        void mkGrepEntry(void){
            auto history = g_build_filename(GREP_HISTORY);              
            auto box = entryBox(_("Contains the text"), _(grep_text_help), history);
            g_free(history);
            grepEntry_ = GTK_ENTRY(g_object_get_data(G_OBJECT(box), "entry"));
            auto optionsBox = grepOptions(grepEntry_);
            g_object_set_data(G_OBJECT(grepEntry_), "optionsBox", optionsBox);
            g_object_set_data(G_OBJECT(grepEntry_), "sWidget", optionsBox);
            gtk_widget_set_sensitive(GTK_WIDGET(optionsBox), false);

            gtk_box_append(topPaneVbox_, GTK_WIDGET(box));
            gtk_box_append(topPaneVbox_, GTK_WIDGET(optionsBox));

#if 0
            // needs to be done after realize...
            // buffer requires realize (X resources, maybe, WTF)

            auto buffer = gtk_entry_get_buffer(entry);
            const char *text = gtk_entry_buffer_get_text(buffer);
            active_grep_ =  text && strlen(text); 
            DBG("*** active_grep=%d\n", active_grep_);
            gtk_widget_set_sensitive(GTK_WIDGET(optionsBox), active_grep_);


            /// FIXME: the following does not do the trick
            //         crashes with cannot access memory address 0x12 for optionbox
            //         Probably must come after window mapped...
            //g_signal_connect(G_OBJECT(buffer), "inserted-text", G_CALLBACK(sensitivize), GTK_WIDGET(optionsBox));
            //g_signal_connect(G_OBJECT(buffer), "deleted-text", G_CALLBACK(sensitivize), GTK_WIDGET(optionsBox));

#endif

#if 0
            g_free(history);
            gtk_box_append(topPaneVbox_, GTK_WIDGET(grep_box));
            g_object_set_data(G_OBJECT(mainBox_), "grep_entry", grep_entry);

            auto optionBox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10));

            auto grep_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
            gtk_box_append(topPaneVbox_, GTK_WIDGET(grep_box));
            //compat<bool>::boxPack0 (topPaneVbox_, GTK_WIDGET(grep_box), FALSE, TRUE, 5);

            auto t=g_strdup_printf("%s: ",_("Contains the text"));
            auto grep_label = GTK_LABEL(gtk_label_new (t));
            g_free(t);
        
            //auto history = g_build_filename(GREP_HISTORY);
            auto grep_entry = mkCompletionEntry(&grepHistory_);
            Basic::setTooltip(GTK_WIDGET(grep_entry), _(grep_text_help));
            g_object_set_data(G_OBJECT(mainBox_), "grep_entry", grep_entry);
            //g_free(history);        
            gtk_widget_set_sensitive (GTK_WIDGET(grep_entry), TRUE);   
            

            auto buffer = gtk_entry_get_buffer(grep_entry);
            const char *text = gtk_entry_buffer_get_text(buffer);
            gboolean active_grep =  text && strlen(text); 

            auto optionBox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10));
            /// FIXME: the following does not do the trick
            //         crashes with cannot access memory address 0x12 for optionbox
            //         Probably must come after window mapped...
            //g_signal_connect(G_OBJECT(buffer), "inserted-text", G_CALLBACK(sensitivize), optionBox);
            //g_signal_connect(G_OBJECT(buffer), "deleted-text", G_CALLBACK(sensitivize), optionBox);

            auto vector = historyVector(grepHistory_);
            GtkWidget *button;
            button = gtk_drop_down_new_from_strings(vector);
            //gtk_entry_buffer_set_text(buffer, vector[0], -1);
            g_free(vector);

            GListModel *model = gtk_drop_down_get_model(GTK_DROP_DOWN(button));
            
            g_signal_connect(G_OBJECT(button), "notify", G_CALLBACK(notify), grep_entry);
            
                
            //auto button = UtilBasic::mkButton(EMBLEM_QUESTION, NULL);
            //FIXME: g_object_set_data(G_OBJECT(button), "findDialog", findDialog);
            //Basic::setTooltip(GTK_WIDGET(button), _(grep_text_help));
            /*g_signal_connect (GTK_WIDGET(button),
                              "clicked", WIDGET_CALLBACK(findSignals<Type>::on_buttonHelp), 
                              (gpointer) _(grep_text_help));*/
#endif
     
            //compat<bool>::boxPack0 (optionBox, GTK_WIDGET(checkBox), FALSE, FALSE, 0);
            //compat<bool>::boxPack0 (optionBox, GTK_WIDGET(radioBox), FALSE, FALSE, 0);
            //compat<bool>::boxPack0 (topPaneVbox_, GTK_WIDGET(optionBox), FALSE, FALSE, 0);

        }

        void mkGrepOption(opt_t *opt, GtkEntry *grep_entry, GtkBox *checkBox){
            auto check = GTK_CHECK_BUTTON(gtk_check_button_new_with_mnemonic (opt->text));
            //gtk_widget_set_sensitive (GTK_WIDGET(check), active_grep);
            gtk_check_button_set_active (check, opt->defaultValue);
            //FIXME: g_object_set_data(G_OBJECT(findDialog), opt->id, check);
            // FIXME g_signal_connect (G_OBJECT (grep_entry), "event", 
                   // KEY_EVENT_CALLBACK (findSignals<Type>::on_key_release), (gpointer) check);
            gtk_box_append(checkBox, GTK_WIDGET(check));
            //compat<bool>::boxPack0 (checkBox, GTK_WIDGET(check), FALSE, FALSE, 0);
        }


        void mkGrepRadio(opt_t *opt, GtkEntry *grep_entry, GSList **group, GtkBox *radioBox){
            auto radio = GTK_CHECK_BUTTON(gtk_check_button_new_with_label(opt->text));
            
            gtk_check_button_set_group(radio, firstGrepRadio_);
            if (!firstGrepRadio_) firstGrepRadio_ = radio;
            
            //*group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio));
            //FIXME: g_object_set_data(G_OBJECT(findDialog), opt->id, radio);
            //gtk_widget_set_sensitive (GTK_WIDGET(radio), active_grep);

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

            auto clearButton =  boxButton(EMBLEM_CLEAR,(void *) findSignals<Type>::onClearButton);


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
        
#if 10
        GtkBox *advancedOptions(void){
            auto advancedBox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
            gtk_widget_set_vexpand(GTK_WIDGET(advancedBox), FALSE);
            auto topHbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
            auto t = g_strconcat("<span color=\"red\" size=\"large\">",_("Select advanced options"),"</span>",NULL);
            auto label = GTK_LABEL(gtk_label_new(""));
            gtk_label_set_markup(label, t);

            gtk_box_append(advancedBox, GTK_WIDGET(topHbox));
            //compat<bool>::boxPack0 (advancedBox, GTK_WIDGET(topHbox), TRUE, TRUE, 0);
            gtk_box_append(topHbox, GTK_WIDGET(label));
            //compat<bool>::boxPack0 (topHbox, GTK_WIDGET(label), TRUE, TRUE, 0);

            /*auto closeButton = gtk_c::dialog_button(WINDOW_CLOSE, "");
            auto vBox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
            compat<bool>::boxPack0 (vBox, GTK_WIDGET(closeButton), TRUE, TRUE, 0);
            compat<bool>::boxPack0 (topHbox, GTK_WIDGET(vBox), FALSE, FALSE, 0);
            g_signal_connect(G_OBJECT (closeButton), "clicked", 
                    BUTTON_CALLBACK(Type::onCloseDetails), (void *)findDialog);*/

            auto hbox17 = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
            gtk_box_append(advancedBox, GTK_WIDGET(hbox17));
            //compat<bool>::boxPack0 (advancedBox, GTK_WIDGET(hbox17), TRUE, FALSE, 0);

            auto left_options_vbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
            gtk_box_append(hbox17, GTK_WIDGET(left_options_vbox));
            //compat<bool>::boxPack0 (hbox17, GTK_WIDGET(left_options_vbox), FALSE, FALSE, 0);
            auto center_options_vbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
            gtk_box_append(hbox17, GTK_WIDGET(center_options_vbox));
            //compat<bool>::boxPack0 (hbox17, GTK_WIDGET(center_options_vbox), FALSE, FALSE, 0);
            auto right_options_vbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
            gtk_box_append(hbox17, GTK_WIDGET(right_options_vbox));
            //compat<bool>::boxPack0 (hbox17, GTK_WIDGET(right_options_vbox), FALSE, FALSE, 0);
          DBG("advancedOptions  <- 1\n");

            /// option -r "recursive"
            gtk_check_button_set_active(add_option_spin(left_options_vbox, "recursive", NULL, _("Recursive"), 0), default_recursive);
          DBG("advancedOptions  <- 2\n");

            /// option -D "recursiveH"
            gtk_check_button_set_active(add_option_spin(left_options_vbox, "recursiveH", NULL, _("Find hidden files and directories"), 0), default_recursiveH);

          DBG("advancedOptions  <- 3\n");
            /// option -a "xdev"
            gtk_check_button_set_active(add_option_spin(left_options_vbox, "xdev", NULL, _("Stay on single filesystem"), 0), default_xdev);

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
          DBG("advancedOptions  <- 4\n");

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
            auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(mainBox_), "permissions_entry"));
            gtk_widget_set_size_request (GTK_WIDGET(entry), 75, -1);
            
            // option -p suid | exe 
            add_option_radio2(center_options_vbox, "suidexe", "suid_radio", "exe_radio", _("SUID"), _("Executable"));

         
            auto hbox21 = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
            gtk_box_append(left_options_vbox, GTK_WIDGET(hbox21));
            //compat<bool>::boxPack0 (left_options_vbox, GTK_WIDGET(hbox21), TRUE, FALSE, 0);

            auto label37 = GTK_LABEL(gtk_label_new (_("File type : ")));
            gtk_box_append(hbox21, GTK_WIDGET(label37));
            //compat<bool>::boxPack0 (hbox21, GTK_WIDGET(label37), FALSE, FALSE, 0);

            auto file_type_om =  GTK_DROP_DOWN(gtk_drop_down_new_from_strings(ftypes));
            gtk_box_append(hbox21, GTK_WIDGET(file_type_om));
            g_object_set_data(G_OBJECT(mainBox_), "file_type_om", file_type_om);
            gtk_drop_down_set_selected(file_type_om, 0);

            // option -M -A -C
            radio_t *radio_p = create_radios(left_options_vbox);
//            radio_t *radio_p = create_radios(right_options_vbox);
            // radio_p freed on destroy event for findDialog.

            // option -k minutes "last_minutes", "last_minutes_spin"
            radio_p->toggle[0] = add_option_spin(left_options_vbox, "last_minutes", "last_minutes_spin", _("Minutes"), last_minutes);
            // FIXME: g_signal_connect (G_OBJECT (radio_p->toggle[0]), "toggled", 
            //         BUTTON_CALLBACK(Type::sensitivize_radio), radio_p);
            
           // option -h hours "last_hours", "last_hours_spin"
            radio_p->toggle[1] = add_option_spin(left_options_vbox, "last_hours", "last_hours_spin", _("Hours"), last_hours);
            // FIXME: g_signal_connect (G_OBJECT (radio_p->toggle[1]), "toggled", 
              //  BUTTON_CALLBACK(Type::sensitivize_radio), radio_p);
            
            // option -d days "last_days", "last_days_spin"
            radio_p->toggle[2] = add_option_spin(left_options_vbox, "last_days", "last_days_spin", _("Days"), last_days);
                //compat<bool>::boxPack0 (GTK_BOX (topPaneHbox), GTK_WIDGET(check), FALSE, FALSE, 0);
                // FIXME: g_signal_connect (G_OBJECT (radio_p->toggle[2]), "toggled", 
                    //BUTTON_CALLBACK(Type::sensitivize_radio), radio_p);
            
            // option -m months "last_months", "last_months_spin"
            radio_p->toggle[3] = add_option_spin(left_options_vbox, "last_months", "last_months_spin", _("Months"), last_months);
            // FIXME: g_signal_connect (G_OBJECT (radio_p->toggle[3]), "toggled", 
                //BUTTON_CALLBACK(Type::sensitivize_radio), radio_p);
            
             ///////////

            /*auto file_type_om =  GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
            gtk_box_append(hbox21, GTK_WIDGET(file_type_om));
            //compat<bool>::boxPack0 (hbox21, GTK_WIDGET(file_type_om), TRUE, TRUE, 0);
            g_object_set_data(G_OBJECT(mainBox_), "file_type_om", file_type_om);
            fill_string_option_menu (GTK_COMBO_BOX(file_type_om), ftypes);   */         

           // auto window = GTK_DIALOG(gtk_dialog_new());
           // gtk_window_set_title(GTK_WINDOW(mainBox_), "xffm4 --find");

            return GTK_BOX(advancedBox);
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
                ERROR("add_option_spin(): incorrect function call\n");
                return NULL;
            }
            auto hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
            gtk_widget_set_hexpand(GTK_WIDGET(hbox), FALSE);
            gtk_box_append(options_vbox, GTK_WIDGET(hbox));
            //compat<bool>::boxPack0 (options_vbox, GTK_WIDGET(hbox), TRUE, FALSE, 0);

            auto size_hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
            GtkCheckButton *check = NULL;
            if (check_name) {
                check = GTK_CHECK_BUTTON(gtk_check_button_new());
                g_object_set_data(G_OBJECT(mainBox_), check_name, (gpointer)check);
                gtk_box_append(hbox, GTK_WIDGET(check));
                //compat<bool>::boxPack0 (hbox, GTK_WIDGET(check), FALSE, FALSE, 0);
                // FIXME: g_signal_connect (G_OBJECT (check), "toggled", 
                     //   BUTTON_CALLBACK(Type::sensitivize), (gpointer)size_hbox);
                gtk_widget_set_sensitive(GTK_WIDGET(size_hbox), FALSE);
            }
            gtk_box_append(hbox, GTK_WIDGET(size_hbox));
            //compat<bool>::boxPack0 (hbox, GTK_WIDGET(size_hbox), FALSE, FALSE, 0);

            if (text) {
                auto label = GTK_LABEL(gtk_label_new (text));
                gtk_box_append(size_hbox, GTK_WIDGET(label));
                //compat<bool>::boxPack0 (size_hbox, GTK_WIDGET(label), TRUE, FALSE, 0);
            }
            
            if (spin_name) {
                auto label = GTK_LABEL(gtk_label_new (": "));
                gtk_box_append(size_hbox, GTK_WIDGET(label));
                //compat<bool>::boxPack0 (size_hbox, GTK_WIDGET(label), FALSE, FALSE, 0);
                auto spinbutton_adj = GTK_ADJUSTMENT(gtk_adjustment_new (default_value, 0, 4096*4096, 1, 64, 0));
                auto spinbutton = GTK_SPIN_BUTTON(gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton_adj), 0, 0));
                gtk_box_append(size_hbox, GTK_WIDGET(spinbutton));
                //compat<bool>::boxPack0 (size_hbox, GTK_WIDGET(spinbutton), TRUE, TRUE, 0);
                gtk_spin_button_set_update_policy (spinbutton, GTK_UPDATE_IF_VALID);
                g_object_set_data(G_OBJECT(mainBox_), spin_name, (gpointer)spinbutton);
                gtk_widget_set_size_request (GTK_WIDGET(spinbutton), 75, -1);
            }

            if (check) return GTK_CHECK_BUTTON(check);
            return NULL;
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
                ERROR("add_option_entry(): incorrect function call\n");
                return NULL;
            }
            auto hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
            gtk_box_append(options_vbox, GTK_WIDGET(hbox));
            //compat<bool>::boxPack0 (options_vbox, GTK_WIDGET(hbox), TRUE, FALSE, 0);

            auto size_hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
            GtkCheckButton *check = NULL;
            if (check_name) {
                check = GTK_CHECK_BUTTON(gtk_check_button_new());
                g_object_set_data(G_OBJECT(mainBox_), check_name, check);
                gtk_box_append(hbox, GTK_WIDGET(check));
                //compat<bool>::boxPack0 (hbox, GTK_WIDGET(check), FALSE, FALSE, 0);
                // FIXME: g_signal_connect (G_OBJECT (check), "toggled", 
                        //BUTTON_CALLBACK(Type::sensitivize), size_hbox);
                gtk_widget_set_sensitive(GTK_WIDGET(size_hbox), FALSE);
            }
            gtk_box_append(hbox, GTK_WIDGET(size_hbox));
            //compat<bool>::boxPack0 (hbox, GTK_WIDGET(size_hbox), FALSE, FALSE, 0);

            if (text) {
                auto label = GTK_LABEL(gtk_label_new (text));
                gtk_box_append(size_hbox, GTK_WIDGET(label));
                //compat<bool>::boxPack0 (size_hbox, GTK_WIDGET(label), TRUE, FALSE, 0);
            }
            
            if (entry_name) {
                auto label = GTK_LABEL(gtk_label_new (": "));
                gtk_box_append(size_hbox, GTK_WIDGET(label));
                //compat<bool>::boxPack0 (size_hbox, GTK_WIDGET(label), FALSE, FALSE, 0);
                
                auto entry = GTK_ENTRY(gtk_entry_new());
                gtk_box_append(size_hbox, GTK_WIDGET(entry));
                //compat<bool>::boxPack0 (size_hbox, GTK_WIDGET(entry), TRUE, TRUE, 0);
                g_object_set_data(G_OBJECT(mainBox_), entry_name, entry);
                auto buffer = gtk_entry_get_buffer(entry);
                gtk_entry_buffer_set_text (buffer, default_value, -1);                          
            }

            if (check) return GTK_CHECK_BUTTON(check);
            return NULL;
        }

        GtkCheckButton *
        add_option_radio2(
                GtkBox *options_vbox, 
                const gchar *check_name,
                const gchar *radio1_name,
                const gchar *radio2_name,
                const gchar *text1,
                const gchar *text2)
        {
            if ((!radio1_name  && !check_name)|| !options_vbox) {
                ERROR("add_option_radio2(): incorrect function call\n");
                return NULL;
            }
            auto hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
            gtk_box_append(options_vbox, GTK_WIDGET(hbox));
            //compat<bool>::boxPack0 (GTK_BOX (options_vbox), GTK_WIDGET(hbox), TRUE, FALSE, 0);

            auto size_hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
            GtkCheckButton *check = NULL;
            if (check_name) {
                check = GTK_CHECK_BUTTON(gtk_check_button_new());
                g_object_set_data(G_OBJECT(mainBox_), check_name, check);
                gtk_box_append(hbox, GTK_WIDGET(check));
                //compat<bool>::boxPack0 (GTK_BOX (hbox), GTK_WIDGET(check), FALSE, FALSE, 0);
                // FIXME: g_signal_connect (G_OBJECT (check), "toggled", 
                    //     BUTTON_CALLBACK(Type::sensitivize), size_hbox);
                gtk_widget_set_sensitive(GTK_WIDGET(size_hbox), FALSE);
            }
            gtk_box_append(hbox, GTK_WIDGET(size_hbox));
            //compat<bool>::boxPack0 (GTK_BOX (hbox), GTK_WIDGET(size_hbox), FALSE, FALSE, 0);

            if (text1 && radio1_name) {
                auto radio1 = GTK_CHECK_BUTTON(gtk_check_button_new_with_label (text1));
                gtk_box_append(size_hbox, GTK_WIDGET(radio1));
                //compat<bool>::boxPack0 (GTK_BOX (size_hbox), GTK_WIDGET(radio1), TRUE, TRUE, 0);
                g_object_set_data(G_OBJECT(mainBox_), radio1_name, radio1);
                if (text2 && radio2_name) {
                    auto radio2 = GTK_CHECK_BUTTON(gtk_check_button_new_with_label(text2));
                    gtk_check_button_set_group(radio2, radio1);
                    gtk_box_append(size_hbox, GTK_WIDGET(radio2));
                    //compat<bool>::boxPack0 (GTK_BOX (size_hbox), GTK_WIDGET(radio2), TRUE, TRUE, 0);
                    g_object_set_data(G_OBJECT(mainBox_), radio2_name, radio2);
                }
            }

            if (check) return GTK_CHECK_BUTTON(check);
            return NULL;
        }


        radio_t *
        create_radios(GtkBox *options_vbox){
            radio_t *radio_p = (radio_t *)malloc(sizeof(radio_t));
            if (!radio_p) g_error("malloc: %s", strerror(errno));
            g_object_set_data(G_OBJECT(mainBox_), "radio_p", radio_p );
            memset(radio_p, 0, sizeof(radio_t));

            auto radio1 =  
                GTK_CHECK_BUTTON(gtk_check_button_new_with_label ("mtime"));
            gtk_widget_set_tooltip_text(GTK_WIDGET(radio1), _("Modified"));
            //tooltip_c::custom_tooltip(GTK_WIDGET(radio1), NULL, _("Modified"));
            auto radio2 = 
                GTK_CHECK_BUTTON(gtk_check_button_new_with_label ("ctime"));
            gtk_check_button_set_group(radio2, radio1);
            gtk_widget_set_tooltip_text(GTK_WIDGET(radio2), _("Created"));
            //tooltip_c::custom_tooltip(GTK_WIDGET(radio2), NULL, _("Created"));
            auto radio3 = 
                GTK_CHECK_BUTTON(gtk_check_button_new_with_label ("atime"));
            gtk_check_button_set_group(radio3, radio1);
            gtk_widget_set_tooltip_text(GTK_WIDGET(radio3), _("Accessed"));
            //tooltip_c::custom_tooltip(GTK_WIDGET(radio3), NULL, _("Accessed"));

            g_object_set_data(G_OBJECT(mainBox_), "radio1", radio1 );
            g_object_set_data(G_OBJECT(mainBox_), "radio2", radio2 );
            g_object_set_data(G_OBJECT(mainBox_), "radio3", radio3 );

            auto radio_box=GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
            g_object_set_data(G_OBJECT(mainBox_), "radio_box", radio_box );
            gtk_box_append(options_vbox, GTK_WIDGET(radio_box));
            //compat<bool>::boxPack0 (options_vbox, GTK_WIDGET(radio_box), TRUE, FALSE, 0);
            //gtk_widget_set_sensitive(GTK_WIDGET(radio_box), FALSE);
            radio_p->box = radio_box;


            /*GtkWidget *label = gtk_label_new(_("modified"));
            gtk_box_append(, GTK_WIDGET());
            */
            auto box=GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
            gtk_box_append(radio_box, GTK_WIDGET(box));
            //compat<bool>::boxPack0 (radio_box, GTK_WIDGET(box), TRUE, FALSE, 0);

            gtk_box_append(box, GTK_WIDGET(radio1));
            //compat<bool>::boxPack0 (box, GTK_WIDGET(radio1), TRUE, FALSE, 0);
            gtk_box_append(box, GTK_WIDGET(radio2));
            //compat<bool>::boxPack0 (box, GTK_WIDGET(radio2), TRUE, FALSE, 0);
            gtk_box_append(box, GTK_WIDGET(radio3));
            //compat<bool>::boxPack0 (box, GTK_WIDGET(radio3), TRUE, FALSE, 0);

            /*label = gtk_label_new(_("within the last"));
            gtk_box_append(radio_box, GTK_WIDGET(label));
            //compat<bool>::boxPack0 (GTK_BOX (radio_box), label, TRUE, FALSE, 0);
            */
            return radio_p;
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
                ERROR("add_option_spin(): incorrect function call\n");
                return NULL;
            }
            auto hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
            gtk_box_append(options_vbox, GTK_WIDGET(hbox));
            //compat<bool>::boxPack0 (options_vbox, GTK_WIDGET(hbox), TRUE, FALSE, 0);

            auto size_hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
            GtkCheckButton *check = NULL;
            if (check_name) {
                check = GTK_CHECK_BUTTON(gtk_check_button_new());
                g_object_set_data(G_OBJECT(mainBox_), check_name, check);
                gtk_box_append(hbox, GTK_WIDGET(check));
                //compat<bool>::boxPack0 (hbox, GTK_WIDGET(check), FALSE, FALSE, 0);
                // FIXME: g_signal_connect (G_OBJECT (check), "toggled", 
                    //    BUTTON_CALLBACK(Type::sensitivize), size_hbox);
                gtk_widget_set_sensitive(GTK_WIDGET(size_hbox), FALSE);
            }
            gtk_box_append(hbox, GTK_WIDGET(size_hbox));
            //compat<bool>::boxPack0 (hbox, GTK_WIDGET(size_hbox), FALSE, FALSE, 0);

            if (text) {
                auto label = GTK_LABEL(gtk_label_new (text));
                gtk_box_append(size_hbox, GTK_WIDGET(label));
                //compat<bool>::boxPack0 (size_hbox, GTK_WIDGET(label), TRUE, FALSE, 0);
                label = GTK_LABEL(gtk_label_new (": "));
                gtk_box_append(size_hbox, GTK_WIDGET(label));
                //compat<bool>::boxPack0 (size_hbox, GTK_WIDGET(label), FALSE, FALSE, 0);
           }

            if (combo_name) {
                //auto usermodel=gtk_list_store_new (1, G_TYPE_STRING);
                //setStoreDataFromList (usermodel, &list);

                //auto combo = GTK_COMBO_BOX(gtk_combo_box_new_with_entry());
                //gtk_combo_box_set_model (combo,GTK_TREE_MODEL(usermodel));
                //gtk_combo_box_set_entry_text_column (combo,0);
                
                auto strings = (const char **)calloc(g_slist_length(list)+1, sizeof (char *));
                int k=0;
                for (auto l=list; l && l->data; l=l->next, k++){
                  strings[k] = (char *)l->data;
                }
                auto combo = GTK_DROP_DOWN(gtk_drop_down_new_from_strings(strings));
                g_free(strings);
                gtk_drop_down_set_selected(combo, 0);


                //auto entry  = GTK_ENTRY (gtk_bin_get_child(GTK_BIN(combo)));
                //gtk_entry_set_text (entry, (const gchar *)list->data);
                
                gtk_box_append(size_hbox, GTK_WIDGET(combo));
                //compat<bool>::boxPack0 (size_hbox, GTK_WIDGET(combo), TRUE, TRUE, 0);
                g_object_set_data(G_OBJECT(mainBox_), combo_name, combo);
                gtk_widget_set_size_request (GTK_WIDGET(combo), 120, -1);
            }
            if (check) return GTK_CHECK_BUTTON(check);
            return NULL;
        }


#endif

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
