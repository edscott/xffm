#ifndef FINDRESPONSE_HH
# define FINDRESPONSE_HH
//
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
    GtkBox *mainBox;
    pid_t pid;
    gint resultLimit;
    gint resultLimitCounter;
    GSList *findList;
    gchar **argument;
    gboolean done;
    void *object;
}fgrData_t;

typedef struct opt_t{
  const char *text;
  const char *id;
  gboolean defaultValue;
} opt_t;
 
//static GHashTable *controllerHash = NULL;
static GSList *lastFind = NULL;

namespace xf
{
    template <class Type> class FindSignals;
    template <class Type> class Util;
    template <class Type>
    class FindResponse : public FindSignals<Type>{
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
      GSList *find_list = NULL;
      gchar  *last_workdir = NULL;
      GtkButton *findButton_ = NULL;
    
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
      GtkTextView *textview_ = NULL;
      GtkBox *mainBox_ = NULL;
      GtkBox *topPaneVbox_ = NULL;
      GtkBox *topPaneHbox_ = NULL;
      GtkPaned *vpane_ = NULL;
      GtkNotebook *notebook_ = NULL;
      GtkBox *advancedVbox_ = NULL;

      GtkEntry *grepEntry_ = NULL;
      fgrData_t *Data_=NULL;

public:
       GtkButton *findButton(void) {return findButton_;}

      void Data(fgrData_t *value) {Data_ = value;}
      fgrData_t *Data(void) {return Data_;}
      GtkPaned *vpane(void){return vpane_;}
      const char *label(void){return "xffm::find";}
      GtkTextView *textview(void){return textview_;}

      FindResponse (void){
      }

      ~FindResponse (void){
        DBG("*** ~FindResponse\n");
        g_free(folder_);
        //exit(0);
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

  DBG("notify callback, data=%p, item=%p, selected=%s\n", data,item,selected);
  
}


    static void 
    sensitivizeSpin (GtkCheckButton *check, gpointer data){
        TRACE("*** sensitivizeSpin, widget = %p\n", data);
        GtkWidget *widget = GTK_WIDGET(data);
        gtk_widget_set_sensitive(widget, gtk_check_button_get_active(check));
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
        DBG("*** findResponse setting dialog to %p\n", value);
        dialog_ = value;
        gtk_window_set_default_widget(GTK_WINDOW(dialog_), GTK_WIDGET(findButton_));
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
        gtk_widget_set_visible(GTK_WIDGET(dialogObject->dialog()), false);
        Basic::flushGTK();
        exit(0);
        return NULL;
      }


      GtkWidget *cbox(void){
        return gtk_label_new("bar");
      }
      GtkBox *mainBox(void) { return mainBox_;}

      GtkBox *mainBox(const char *folder) {

        /*auto cbox =GTK_BOX(g_object_get_data(G_OBJECT(dialog_), "cbox"));
        //auto foo = gtk_label_new("bar");
        gtk_box_prepend(cbox, foo);*/
        
        //auto parentObject = this->parent();
DBG("*** if doFind...1: folder=%s\n", folder);
         
          if (g_file_test(folder, G_FILE_TEST_IS_DIR)) folder_ = realpath(folder, NULL);
          else {
            if (g_file_test(Child::getWorkdir(), G_FILE_TEST_IS_DIR)) folder_ = g_strdup(Child::getWorkdir());
            else folder_ = g_strdup(g_get_home_dir());
          }

DBG("*** if doFind...13\n");

          mainBox_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
          g_object_set_data(G_OBJECT(mainBox_), "object", this);
          gtk_widget_set_vexpand(GTK_WIDGET(mainBox_), false);
          gtk_widget_set_hexpand(GTK_WIDGET(mainBox_), false);
          //gtk_window_set_child(findDialog, GTK_WIDGET(mainBox_));
          DBG("mkVpane  <- \n");
          mkVpane();

          DBG("mkFilterEntry  <- \n");
          mkFilterEntry();
          ////////////////  grep options.... /////////////////////////
          DBG("mkGrepEntry  <- \n");
          mkGrepEntry();
          DBG("mkButtonBox  <- \n");

 
          mkButtonBox(); 
          //gtk_widget_set_size_request(GTK_WIDGET(mainBox_), 812, 462);
          gtk_widget_set_size_request(GTK_WIDGET(mainBox_), 600, 460);

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
          auto advanced = advancedOptions();

          DBG("mkTopPaneHbox  <- \n");
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
          auto t=g_strdup_printf("<span color=\"blue\" size=\"large\"><b>%s</b></span>  ", _("Find in Files"));
          auto label = GTK_LABEL(gtk_label_new (t));
          gtk_label_set_use_markup (label, TRUE);
          g_free(t);
          gtk_box_append(actionBox, GTK_WIDGET(label));

          findButton_ = UtilBasic::mkButton(EMBLEM_FIND, NULL);
          //gtk_window_set_default_widget(GTK_WINDOW(dialog_), GTK_WIDGET(findButton_));
          // gtk_widget_set_can_default(GTK_WIDGET(findButton_), TRUE);
          g_signal_connect (G_OBJECT (findButton_), "clicked",
                  BUTTON_CALLBACK(FindSignals<Type>::onFindButton), (gpointer)this);
          gtk_box_append(actionBox, GTK_WIDGET(findButton_));
          gtk_notebook_set_action_widget(notebook_, GTK_WIDGET(actionBox), GTK_PACK_END);
//          gtk_notebook_set_action_widget(notebook_, GTK_WIDGET(findButton_), GTK_PACK_END);
          Basic::setTooltip(GTK_WIDGET(findButton_), _("Show search results for this query"));

          //gtk_scrolled_window_set_child(sw, GTK_WIDGET(topPaneVbox_));
          //gtk_container_add(GTK_CONTAINER(sw), GTK_WIDGET(topPaneVbox_));
          gtk_scrolled_window_set_policy (sw, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

          textview_ = GTK_TEXT_VIEW(gtk_text_view_new ());
          g_object_set_data(G_OBJECT(mainBox_), "textview", textview_);
          g_object_set_data(G_OBJECT(textview_), "vpane", (gpointer)vpane_);
          
          auto sw2 = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new ());
          gtk_paned_set_end_child (vpane_, GTK_WIDGET(sw2));
          //gtk_paned_pack2 (GTK_PANED (vpane_), GTK_WIDGET(sw2), FALSE, TRUE);
          //gtk_paned_pack2 (GTK_PANED (vpane_), GTK_WIDGET(scrolledwindow), TRUE, TRUE);
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
      void mkPathEntry(GtkBox *parentBox){
          GtkWidget *path_label;
          auto text=g_strdup_printf("%s:", _("Path"));
          auto path_entry = FileResponse<Type, subClass_t>::addEntry(parentBox, "entry1", text, this);
          g_object_set_data(G_OBJECT(mainBox_), "path_entry", path_entry);
          g_free(text);

          auto buffer = gtk_entry_get_buffer(GTK_ENTRY(path_entry));  
          DBG("folder_ = %s\n", folder_);
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
            auto box = entryBox(_("Filter:"), _(filter_text_help), history);
            g_free(history);
            auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(box), "entry"));
            g_object_set_data(G_OBJECT(mainBox_), "filter_entry", entry);

            gtk_box_append(vbox, GTK_WIDGET(box));
            auto recursive = simpleCheck(vbox, _("Recursive"));
            auto hidden = simpleCheck(vbox, _("Find hidden files and directories"));
            gtk_check_button_set_active(hidden, false);
            auto xdev = simpleCheck(vbox, _("Stay on single filesystem"));

          /// option "upper_limit_spin" (only in gtk findDialog)
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
            auto box = entryBox(_("Contains the text"), _(grep_text_help), history);
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
            // FIXME g_signal_connect (G_OBJECT (grep_entry), "event", 
                   // KEY_EVENT_CALLBACK (on_key_release), (gpointer) check);
            gtk_box_append(checkBox, GTK_WIDGET(check));
        }


        void mkGrepRadio(opt_t *opt, GtkEntry *grep_entry, GSList **group, GtkBox *radioBox){
            auto radio = GTK_CHECK_BUTTON(gtk_check_button_new_with_label(opt->text));
            
            gtk_check_button_set_group(radio, firstGrepRadio_);
            if (!firstGrepRadio_) firstGrepRadio_ = radio;
            
            //*group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio));
            g_object_set_data(G_OBJECT(mainBox_), opt->id, radio);
            //gtk_widget_set_sensitive (GTK_WIDGET(radio), active_grep);

            if (opt->defaultValue) {
                gtk_check_button_set_active (radio, TRUE);
            }
            // FIXME g_signal_connect (G_OBJECT (grep_entry), "event", 
                   // KEY_EVENT_CALLBACK (on_key_release), (gpointer) radio);
            gtk_box_append(radioBox, GTK_WIDGET(radio));
            //compat<bool>::boxPack0 (radioBox, GTK_WIDGET(radio), FALSE, FALSE, 0);
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

        void mkButtonBox(void){
            auto hbuttonbox2 = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3));
            auto cancelButton =  boxButton(EMBLEM_RED_BALL, (void *)FindSignals<Type>::onCancelButton);
            g_object_set_data(G_OBJECT(mainBox_), "cancel_button", cancelButton);
            g_object_set_data(G_OBJECT(cancelButton), "mainBox", mainBox_);
            gtk_widget_set_sensitive(GTK_WIDGET(cancelButton), FALSE);

            auto clearButton =  boxButton(EMBLEM_CLEAR,(void *) FindSignals<Type>::onClearButton);


            g_object_set_data(G_OBJECT(mainBox_), "clear_button", clearButton);
            g_object_set_data(G_OBJECT(clearButton), "mainBox", mainBox_);

            GtkBox *edit_button = NULL;

            auto editor =Basic::getEditor();
            if (editor && strlen(editor)){
                auto basename = g_strdup(editor);
                if (strchr(basename, ' ')) *strchr(basename, ' ') = 0;
                auto editor_path = g_find_program_in_path(basename);
                g_free(basename);
                if (editor_path){
                    auto icon_id = Basic::getAppIconName(editor_path, EMBLEM_EDIT);
                    edit_button = boxButton(icon_id, (void *)FindSignals<Type>::onEditButton);
                    g_free(icon_id);
                    g_object_set_data(G_OBJECT(mainBox_), "edit_button", edit_button);
                    g_object_set_data(G_OBJECT(edit_button), "mainBox", mainBox_);
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
            //gtk_check_button_set_group(radio1, radio1);
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
          DBG("advancedOptions  <- 1\n");
         
            auto hbox21 = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3));
            gtk_box_append(left_options_vbox, GTK_WIDGET(hbox21));
            //compat<bool>::boxPack0 (left_options_vbox, GTK_WIDGET(hbox21), TRUE, FALSE, 0);

            auto label37 = GTK_LABEL(gtk_label_new (_("File type : ")));
            gtk_box_append(hbox21, GTK_WIDGET(label37));
            //compat<bool>::boxPack0 (hbox21, GTK_WIDGET(label37), FALSE, FALSE, 0);

            auto file_type_om =  GTK_DROP_DOWN(gtk_drop_down_new_from_strings(ftypes));
            gtk_box_append(hbox21, GTK_WIDGET(file_type_om));
            g_object_set_data(G_OBJECT(mainBox_), "file_type_om", file_type_om);
            gtk_drop_down_set_selected(file_type_om, 0);

            fileSizeFrame(center_options_vbox);

          DBG("advancedOptions  <- 4\n");
            mkUserFrame(center_options_vbox);


            // option -M -A -C
            //radio_t *radio_p = create_radios(left_options_vbox);

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
                ERROR("add_option_spin(): incorrect function call\n");
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
                g_signal_connect (G_OBJECT (check), "toggled", 
                        G_CALLBACK(sensitivizeSpin), size_hbox);
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
                g_signal_connect (G_OBJECT (check), "toggled", 
                        G_CALLBACK(sensitivizeSpin), size_hbox);
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



      };


}
#endif
