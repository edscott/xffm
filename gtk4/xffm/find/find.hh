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
    GtkBox *mainBox;
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
 
static GHashTable *controllerHash = NULL;
static GSList *lastFind = NULL;

namespace xf
{
    template <class Type> class FindSignals;
    template <class Type> class Util;
    template <class Type>
    class FindResponse : public FindSignals<Type>{
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
        return NULL;
      }


      GtkWidget *cbox(void){
        return gtk_label_new("bar");
      }

      GtkBox *mainBox(const char *folder) {

        /*auto cbox =GTK_BOX(g_object_get_data(G_OBJECT(dialog_), "cbox"));
        //auto foo = gtk_label_new("bar");
        gtk_box_prepend(cbox, foo);*/
        
        //auto parentObject = this->parent();
         
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
                  BUTTON_CALLBACK(FindSignals<Type>::onFindButton), (void *)mainBox_);
//                  BUTTON_CALLBACK(onFindButton), (gpointer)this);
          gtk_box_append(actionBox, GTK_WIDGET(findButton_));
          gtk_notebook_set_action_widget(notebook_, GTK_WIDGET(actionBox), GTK_PACK_END);
//          gtk_notebook_set_action_widget(notebook_, GTK_WIDGET(findButton_), GTK_PACK_END);
          Basic::setTooltip(GTK_WIDGET(findButton_), _("Show search results for this query"));

          //gtk_scrolled_window_set_child(sw, GTK_WIDGET(topPaneVbox_));
          //gtk_container_add(GTK_CONTAINER(sw), GTK_WIDGET(topPaneVbox_));
          gtk_scrolled_window_set_policy (sw, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

          auto diagnostics = GTK_TEXT_VIEW(gtk_text_view_new ());
          g_object_set_data(G_OBJECT(mainBox_), "diagnostics", diagnostics);
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
                            mainBox_);
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
                    G_CALLBACK(callback), mainBox_);
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

        ///////////////////   signals  /////////////////////////
template <class Type>
class FindSignals {
    static const gchar *
    get_time_type(GtkBox *mainBox){
        if (gtk_toggle_button_get_active ((GtkToggleButton *) g_object_get_data(G_OBJECT(mainBox), "radio1"))){
            return "-M";
        }
        if (gtk_toggle_button_get_active ((GtkToggleButton *) g_object_get_data(G_OBJECT(mainBox), "radio2"))){
            return "-C";
        }
        return "-A";
    }

    static gboolean
    on_find_clicked_action (void *data) {
      auto mainBox = GTK_BOX(data);
        TRACE("on_find_clicked_action\n");
        GtkTextView *diagnostics = Child::getOutput();
//        GtkTextView *diagnostics = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(mainBox), "diagnostics"));
        // Get the search path.
        GtkEntry *entry = GTK_ENTRY(g_object_get_data(G_OBJECT(mainBox), "path_entry"));
        auto buffer = gtk_entry_get_buffer(entry);
        char *path = g_strdup(gtk_entry_buffer_get_text(buffer));
        /* tilde expansion */
        if(path[0] == '~'){
            char *t = g_strconcat(g_get_home_dir(),"/", path+1, NULL);
            g_free(path);
            path = t;
        }
        /* environment variables */
        else if(path[0] == '$') {
            const gchar *p = getenv (path + 1);
            if(p){
                g_free(path);
                path = g_strdup(p);
            }
        }
        if(!path || strlen (path) == 0 || !g_file_test (path, G_FILE_TEST_EXISTS)) {
            char *message = g_strconcat(strerror (ENOENT), ": ", path, "\n", NULL);
            Print::printError(diagnostics, message);
            g_free(path);
            return FALSE;
        }
        fgrData_t *Data = (fgrData_t *)calloc(1,sizeof(fgrData_t));
        Data->mainBox = mainBox;
        Data->done = FALSE; // (This is redundant with calloc, here just for clarity).


        GtkWidget *cancel = GTK_WIDGET(g_object_get_data(G_OBJECT(mainBox), "cancel_button"));
        //gtk_widget_show(cancel);
        gtk_widget_set_sensitive(cancel, TRUE);

        /* get the parameters set by the user... *****/
        get_arguments(path, Data);

        fprintf(stderr, "DBG> arguments: \'");
        for (auto p=Data->argument; p && *p; p++){
          fprintf(stderr, "%s ", *p);
        }
        fprintf(stderr, "\n");


        /*
    // not here: FIXME    
        if (Data->find_list) {
            GSList *list = find_list;
            for (;list && list->data; list=list->next){
                g_free(list->data);
            }
            g_slist_free(find_list);
            find_list = NULL;
        }
    */
#if 10
        int flags = TUBO_EXIT_TEXT|TUBO_VALID_ANSI|TUBO_CONTROLLER_PID;
        Data->pid = Run<Type>::thread_run(Data, (const gchar **)Data->argument, stdout_f, stderr_f, forkCleanup);

        // When the dialog is destroyed on action, then output is going to a
        // widgets_p away from dialog window, so we let the process run on and on.
        gchar *command = g_strdup("");
        gchar **p=Data->argument;
        for (;p && *p; p++){
            gchar *g = g_strconcat(command, " ", *p, NULL);
            g_free(command);
            command = g;
        }
        g_hash_table_replace(controllerHash, GINT_TO_POINTER(Data->pid), command);
        Print::print(diagnostics, EMBLEM_FIND, "green", g_strconcat( _("Searching..."), "\n", NULL));
        Print::print(diagnostics, EMBLEM_RUN, "bold",
            g_strdup_printf("%s: \"%s\"\n",_("Searching..."), (gchar *) command));
#endif 
        return FALSE;
    }

        static void getSizeOptions(fgrData_t *Data, int *i){
          GtkBox *mainBox = Data->mainBox;
          if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                      g_object_get_data(G_OBJECT(mainBox), "size_greater")))) 
          {
               int size_greater = gtk_spin_button_get_value_as_int (
                  GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(mainBox), "size_greater_spin")));            
               Data->argument[(*i)++] = g_strdup("-s");
               Data->argument[(*i)++] = g_strdup_printf("+%d", size_greater);
          }
          if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                      g_object_get_data(G_OBJECT(mainBox), "size_smaller")))) 
          {
               gint size_smaller = gtk_spin_button_get_value_as_int (
                  GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(mainBox), "size_smaller_spin")));            
               Data->argument[(*i)++] = g_strdup("-s");
               Data->argument[(*i)++] = g_strdup_printf("-%d", size_smaller);
          }
        }

        static void getTimeOptions(fgrData_t *Data, int *i){
          GtkBox *mainBox = Data->mainBox;
          if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                      g_object_get_data(G_OBJECT(mainBox), "last_months")))) 
          {
              Data->argument[(*i)++] = g_strdup((gchar *)get_time_type(mainBox));
              gint last_months = gtk_spin_button_get_value_as_int (
                  GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(mainBox), "last_months_spin")));            
              Data->argument[(*i)++] = g_strdup("-m");
              Data->argument[(*i)++] = g_strdup_printf("%d", last_months);
          }
          else if(gtk_check_button_get_active (GTK_CHECK_BUTTON(
                      g_object_get_data(G_OBJECT(mainBox), "last_days"))))
          {
              Data->argument[(*i)++] = (gchar *)get_time_type(mainBox);
              gint last_days = gtk_spin_button_get_value_as_int (
                  GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(mainBox), "last_days_spin")));            
              Data->argument[(*i)++] = g_strdup("-d");
              Data->argument[(*i)++] = g_strdup_printf("%d", last_days);
          }
          else if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                      g_object_get_data(G_OBJECT(mainBox), "last_hours"))))
          {
              Data->argument[(*i)++] = (gchar *)get_time_type(mainBox);
              gint last_hours = gtk_spin_button_get_value_as_int (
                  GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(mainBox), "last_hours_spin")));            
              Data->argument[(*i)++] = g_strdup("-h");
              Data->argument[(*i)++] = g_strdup_printf("%d", last_hours);
          }
          else if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                      g_object_get_data(G_OBJECT(mainBox), "last_minutes"))))
          {
              Data->argument[(*i)++] = g_strdup((gchar *)get_time_type(mainBox));
              gint last_minutes = gtk_spin_button_get_value_as_int (
                  GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(mainBox), "last_minutes_spin")));            
              Data->argument[(*i)++] = g_strdup("-k");
              Data->argument[(*i)++] = g_strdup_printf("%d", last_minutes);
          }
        }

      static void getFileOptions(fgrData_t *Data, int *i){
          GtkBox *mainBox = Data->mainBox;
        // SUID/EXE
        if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                    g_object_get_data(G_OBJECT(mainBox), _("Executable"))))
            ||
            gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                        g_object_get_data(G_OBJECT(mainBox), _("SUID"))))){
              Data->argument[(*i)++] = g_strdup("-p");
              if (gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                    g_object_get_data(G_OBJECT(mainBox), _("Executable"))))) {
                Data->argument[(*i)++] = g_strdup("exe");
              } else {
                Data->argument[(*i)++] = g_strdup("suid");
              }
              return;
        }
        if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                    g_object_get_data(G_OBJECT(mainBox), _("Octal Permissions")))))
        {
            Data->argument[(*i)++] = g_strdup("-o");
            auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(mainBox), "permissions_entry"));
            auto buffer = gtk_entry_get_buffer(entry);
            const char *c = gtk_entry_buffer_get_text (buffer);
            // FIXME: check for valid octal number <= 0777
            if (c && strlen(c)) Data->argument[(*i)++] = g_strdup(c);
            else Data->argument[(*i)++] = g_strdup("0666");
            return;
        } 
        if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                    g_object_get_data(G_OBJECT(mainBox), "uid"))))
        {
            auto dd = GTK_DROP_DOWN(g_object_get_data(G_OBJECT(mainBox), "uid_combo")); 
            auto item = GTK_STRING_OBJECT(gtk_drop_down_get_selected_item(dd));
            auto selected = gtk_string_object_get_string(item);
          
            if(selected && strlen(selected)) {
                Data->argument[(*i)++] = g_strdup("-u");
                struct passwd *pw = getpwnam (selected);
                if(pw) {
                    Data->argument[(*i)++] = g_strdup_printf("%d", pw->pw_uid);
                } else {
                    Data->argument[(*i)++] = g_strdup_printf("%d",atoi(selected));
                }

            }
            return;
        }
        if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                    g_object_get_data(G_OBJECT(mainBox), "gid"))))
        {
            auto dd = GTK_DROP_DOWN(g_object_get_data(G_OBJECT(mainBox), "uid_combo")); 
            auto item = GTK_STRING_OBJECT(gtk_drop_down_get_selected_item(dd));
            auto selected = gtk_string_object_get_string(item);
          
            if(selected && strlen(selected)) {
                Data->argument[(*i)++] = g_strdup("-g");
                struct group *gr = getgrnam (selected);
                if(gr) {
                    Data->argument[(*i)++] = g_strdup_printf("%d", gr->gr_gid);
                } else {
                    Data->argument[(*i)++] = g_strdup_printf("%d",atoi(selected));
                }

            }
            return;
        }

        

      }

      static void getGrepOptions(fgrData_t *Data, int *i){
          GtkBox *mainBox = Data->mainBox;
        if(!gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                    g_object_get_data(G_OBJECT(mainBox), "case_sensitive"))))
        {
            Data->argument[(*i)++] = g_strdup("-i");
        } 
        if(gtk_check_button_get_active (GTK_CHECK_BUTTON(
                    g_object_get_data(G_OBJECT(mainBox), "line_count"))))
        {
            Data->argument[(*i)++] = g_strdup("-c");
        } 


        auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(mainBox), "grep_entry"));
        auto buffer = gtk_entry_get_buffer(entry);
        const char *token = gtk_entry_buffer_get_text(buffer);  
  
        //const gchar *token = gtk_entry_get_text(GTK_ENTRY(g_object_get_data(G_OBJECT(mainBox), "grep_entry")));
        if(token && strlen(token)) {
              if(gtk_check_button_get_active (GTK_CHECK_BUTTON(
                        g_object_get_data(G_OBJECT(mainBox), "ext_regexp"))))
            {
                Data->argument[(*i)++] = g_strdup("-E");
            } else {
                Data->argument[(*i)++] = g_strdup("-e");
            }
            Data->argument[(*i)++] = g_strdup(token);

            /* options for grep: ***** */
            if(!gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                        g_object_get_data(G_OBJECT(mainBox), "look_in_binaries"))))
            {
                Data->argument[(*i)++] = g_strdup("-I");
            }
            if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                        g_object_get_data(G_OBJECT(mainBox), "match_words"))))
            {
                Data->argument[(*i)++] = g_strdup("-w");
            }
            else if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                        g_object_get_data(G_OBJECT(mainBox), "match_lines"))))
            {
                Data->argument[(*i)++] = g_strdup("-x");
            }
            else if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                        g_object_get_data(G_OBJECT(mainBox), "match_no_match"))))
            {
                Data->argument[(*i)++] = g_strdup("-L");
            }
        }
      }

    static void
    get_arguments(gchar *path, fgrData_t *Data){
        int i=0;
        GtkBox *mainBox = Data->mainBox;

        /* limit */
        Data->resultLimit = gtk_spin_button_get_value_as_int (
                GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(mainBox), "upper_limit_spin")));
        Data->resultLimitCounter = 0; // fgr ends with "fgr search complete"
        Data->argument = (char **)calloc(MAX_COMMAND_ARGS, sizeof(gchar *));
        if (!Data->argument){
            std::cerr<<"calloc error at get_arguments()\n";
            exit(1);
        }
        
        /* the rest */
        Data->argument[i++] = g_strdup(xffindProgram);
        Data->argument[i++] = g_strdup("--fgr");
        Data->argument[i++] = g_strdup("-v"); // (verbose output) 
        if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                    g_object_get_data(G_OBJECT(mainBox), _("Recursive")))))
        {
            if(gtk_check_button_get_active (GTK_CHECK_BUTTON(
                        g_object_get_data(G_OBJECT(mainBox), _("Find hidden files and directories"))))) 
            {
                Data->argument[i++] = g_strdup("-r");
            } else {
                Data->argument[i++] = g_strdup("-R");
            }
        } 

        if(gtk_check_button_get_active (GTK_CHECK_BUTTON( //?? check this XXX
                    g_object_get_data(G_OBJECT(mainBox), _("Stay on single filesystem")))))
        {
            Data->argument[i++] = g_strdup("-a");
        } 
        
        if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                    g_object_get_data(G_OBJECT(mainBox), _("File size"))))) {
          getSizeOptions(Data, &i);
        }


        if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                    g_object_get_data(G_OBJECT(mainBox), _("Specific Time"))))){
          getTimeOptions(Data, &i);
        }

        if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                    g_object_get_data(G_OBJECT(mainBox), _("File properties"))))){
          getFileOptions(Data, &i);
        }


        //  grep options
        getGrepOptions(Data, &i);

        /* select list */

        auto dd = GTK_DROP_DOWN(g_object_get_data(G_OBJECT(mainBox), "file_type_om")); 
        auto item = GTK_STRING_OBJECT(gtk_drop_down_get_selected_item(dd));
        auto ftype = gtk_string_object_get_string(item);
        //const char *ftype = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT(
                //g_object_get_data(G_OBJECT(mainBox), "file_type_om")));
        for(int j = 0; ftypes[j] != NULL; j++) {
            if(ftype && strcmp (ftype, _(ftypes[j])) == 0) {
                Data->argument[i++] = g_strdup("-t");
                Data->argument[i++] = g_strdup(ft[j]);
                break;
            }
        }

        auto filterEntry = GTK_ENTRY(g_object_get_data(G_OBJECT(mainBox), "filter_entry"));
        auto filterBuffer = gtk_entry_get_buffer(filterEntry);
        const char *filter = gtk_entry_buffer_get_text(filterBuffer);
        //const char *filter = gtk_entry_get_text(GTK_ENTRY(g_object_get_data(G_OBJECT(mainBox), "filter_entry")));
        /* apply wildcard filter if not specified (with dotfiles option) */
        Data->argument[i++] = g_strdup("-f");
        if(filter && strlen (filter)) {
            Data->argument[i++] = g_strdup(filter);
        } else {
            Data->argument[i++] = g_strdup("*");
            Data->argument[i++] = g_strdup("-D");
        }

        /* last Data->argument is the path */
        //Data->argument[i++] = g_strdup_printf("\"%s\"", path); g_free(path);
        Data->argument[i++] = path;
        Data->argument[i] = (char *)0;
    }

  public:
    static void
    onFindButton (GtkWidget * button, void *data) {
      DBG("onFindButton...\n");
        if (!controllerHash){
            controllerHash = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, g_free);
        }
        GtkTextView *diagnostics = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(data), "diagnostics"));
        // WTF? updateCompletions(dialog);
        
        Print::showText(diagnostics);
        //Print::showTextSmall(diagnostics);
        on_find_clicked_action (data);
    }

    static void
    edit_command (gpointer data) {
        GtkWindow *dialog = GTK_WINDOW(data);
        GtkTextView *diagnostics = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(dialog), "diagnostics"));
        GSList *list = lastFind;

        /*if (!list || g_slist_length(list) < 1) {
            rfm_diagnostics(widgets_p, "xffm/stock_dialog-warning",NULL);
            rfm_diagnostics(widgets_p, "xffm_stderr", _("Search returned no results"), "\n", NULL);
            return;
        }*/

        auto editor = Basic::getEditor();
        if (!editor || strlen(editor)==0){
          Print::printError(diagnostics, g_strdup_printf("%s\n",
                        _("No editor for current action.")));
            return;
        }
        TRACE("editor = %s\n", editor);
        gchar *command;
        if (Run<Type>::runInTerminal(editor)){
            command = Run<Type>::mkTerminalLine(editor, "");
        } else {
            command = g_strdup(editor);
        }
      
        TRACE("command = %s\n", command);

        for (; list && list->data; list=list->next){
            gchar *g = g_strconcat(command, " \"", (gchar *)list->data, "\"", NULL);
            g_free(command);
            command = g;
        }
        TRACE("command args = %s\n", command);

        // Hack: for nano or vi, run in terminal
        gboolean in_terminal = FALSE;
        if (strstr(command, "nano") || 
                (strstr(command, "vi") && !strstr(command, "gvim")))
        {
            in_terminal = TRUE;
        }

        TRACE("thread_run %s\n", command);
        Run<Type>::thread_run(diagnostics, command, FALSE);
        //RFM_THREAD_RUN2ARGV(widgets_p, command, in_terminal);
        
        //widgets_p->workdir = g_strdup(g_get_home_dir());
    }


    static void
    onEditButton (GtkWidget * button, gpointer data) {
      DBG("onEditButton...\n");
        GtkTextView *diagnostics = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(data), "diagnostics"));
        TRACE("fixme: signals::onEditButton\n");
        //print_c::print_status(diagnostics, g_strdup("fixme: signals::onEditButton testing run\n"));
        Print::showText(diagnostics);        
        //Print::showTextSmall(diagnostics);        
        edit_command(data);
    }

    static void
    onClearButton (GtkWidget * button, gpointer data) {
      DBG("onClearButton...\n");

        auto diagnostics = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(data), "diagnostics"));
        TRACE("fixme: signals::onClearButton\n");
        Print::clear_text(diagnostics);
        auto vpane =GTK_PANED(g_object_get_data(G_OBJECT(data), "vpane"));
        gtk_paned_set_position(vpane, 2500);
    }

    static gboolean
    freeFgrData(void *data){
        fgrData_t *Data = (fgrData_t *)data;
        gchar **a=Data->argument;
        for (;*a; a++) g_free(*a);
        g_free(Data->argument);
        g_free(Data);
        return FALSE;
    }

    static gboolean
    Cleanup (void *data) {
       TRACE("Cleanup\n");
       fgrData_t *Data = (fgrData_t *)data;
       auto mainBox = Data->mainBox;
       if (g_hash_table_size(controllerHash) == 0){
            GtkWidget *cancel = GTK_WIDGET(g_object_get_data(G_OBJECT(mainBox), "cancel_button"));
            gtk_widget_set_sensitive(cancel, FALSE);
       }

       GtkWidget *edit_button = GTK_WIDGET(g_object_get_data(G_OBJECT(mainBox), "edit_button"));
       if (g_slist_length(lastFind)){
            const gchar *editor = Basic::getEditor();
            if (!editor || strlen(editor)==0){
                GtkTextView *diagnostics = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(mainBox), "diagnostics"));
                Print::print(diagnostics, EMBLEM_WARNING, 
                        g_strdup_printf("%s\n", _("No editor for current action.")));
                return FALSE;
            }
            gtk_widget_set_sensitive(GTK_WIDGET(edit_button), TRUE);
       } else {
            gtk_widget_set_sensitive(GTK_WIDGET(edit_button), FALSE);
       }
       if (Data->done) {
           freeFgrData(Data);
           return FALSE;
       }
       return TRUE;
    }


    static void
    forkCleanup (void *data) {
       TRACE("forkCleanup\n");
        fgrData_t *Data = (fgrData_t *)data;
        g_hash_table_remove(controllerHash, GINT_TO_POINTER(Data->pid));
        g_timeout_add_seconds(1, Cleanup, (void *)Data);
        
    }

    static void
    stderr_f (void *data, void *stream, int childFD) {
        fgrData_t *Data = (fgrData_t *)data;
        GtkTextView *diagnostics = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(Data->mainBox), "diagnostics"));
        if (!gtk_widget_is_visible(GTK_WIDGET(diagnostics))) return;

        char *line;
        line = (char *)stream;


        if(line[0] != '\n') {
            Print::print(diagnostics, EMBLEM_WARNING, "red", g_strdup(line));
        }

        // With this, this thread will not do a DOS attack
        // on the gtk event loop.
        static gint count = 1;
        if (count % 20 == 0){
            usleep(10000);
        } 
       return;
    }


    static void
    stdout_f (void *data, void *stream, int childFD) {
        fgrData_t *Data = (fgrData_t *)data;
        char *line;
        line = (char *)stream;
        TRACE("FORK stdout: %s\n", line);

        if(line[0] == '\n') return;
        GtkTextView *diagnostics = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(Data->mainBox), "diagnostics"));

        if (Data->resultLimit > 0 && Data->resultLimit==Data->resultLimitCounter) {
            gchar *g=g_strdup_printf("%s. %s %d", _("Results"), _("Upper limit:"), Data->resultLimit);
            Print::print(diagnostics, EMBLEM_WARNING, "green", g_strconcat(g, "\n", NULL));
            Print::print(diagnostics, "blue",  g_strconcat(_("Counting files..."), "\n", NULL));
            g_free(g);
        }

        if(strncmp (line, "Tubo-id exit:", strlen ("Tubo-id exit:")) == 0) {
                if(strchr (line, '\n')) *strchr (line, '\n') = 0;
                
                /*FIXME this wont work if another fgr is running
                 * must check for any running fgr subprocess
                 * GtkWidget *cancel = widgets_p->data; 
                if (cancel && GTK_IS_WIDGET(cancel)) {
                    rfm_context_function(hide_cancel_button, cancel);
                }*/
                gchar *plural_text = 
                    g_strdup_printf (
                            ngettext ("Found %d match", "Found %d matches", 
                                Data->resultLimitCounter), Data->resultLimitCounter);
                gchar *message = g_strdup_printf(_("%s Finished : %s"), xffindProgram, plural_text);
                gchar *g = g_strdup_printf("%c[31m%s\n",27, message);
                Print::print(diagnostics, EMBLEM_CANCEL, "Red", g);
                // Free last find results
                GSList *list = lastFind;
                for(;list; list=list->next) g_free(list->data);
                g_slist_free(lastFind);
                // assign new find results
                lastFind = Data->findList;
                Data->done = TRUE;
                list = lastFind;
                for (;list && list->data; list=list->next){
                    TRACE("last find: %s\n", (gchar *)list->data);
                }
                // FIXME FIXME construct DnDBox class template
                // DnDBox<Type>::openDnDBox(findDialog, plural_text, lastFind);
                

                // cleanupmake
                //
                g_free(plural_text);
                g_free(message);
        } else {
            if (!gtk_widget_is_visible(GTK_WIDGET(diagnostics))) return;
            gchar *file = g_strdup(line);
            if (strchr(file, '\n')) *strchr(file, '\n') = 0;
            if (g_file_test(file, G_FILE_TEST_EXISTS)) {
                Data->resultLimitCounter++; 
                if (Data->resultLimit ==0 ||
                    (Data->resultLimit > 0 && Data->resultLimit > Data->resultLimitCounter) ) {
                    Print::print(diagnostics, g_strdup_printf("%s\n", file));
                    TRACE("--> %s\n",file);
                    //TRACE("resultLimitCounter:%d %s\n", Data->resultLimitCounter, line);
                    Data->findList = g_slist_prepend(Data->findList, file);
                }
            } else {
               TRACE("ignoring: \"%s\"\n", file);
               g_free(file);
            }
        }         
        // With this, this thread will not do a DOS attack
        // on the gtk event loop.
        static gint count = 1;
        if (count % 20 == 0){
            usleep(10000);
        } 
        
        return;
    }

    static gboolean removeFunc(gpointer key, gpointer value, gpointer data){
        GtkTextView *diagnostics = GTK_TEXT_VIEW(data);
        Print::print(diagnostics, EMBLEM_DELETE, "bold", 
                g_strdup_printf("%s: \"%s\"\n",_("Cancel"), (gchar *) value));
        //Signal process controller to kill child process.
      fprintf(stderr, "removeFunc...\n");
        kill(GPOINTER_TO_INT(key), SIGUSR2);
      fprintf(stderr, "removeFunc\n");
        return FALSE;
    }
    
    static void
    cancel_all(void * dialog){
        TRACE("cancel_all\n");
        void *diagnostics = g_object_get_data(G_OBJECT(dialog), "diagnostics");
        g_hash_table_foreach_remove (controllerHash, removeFunc, diagnostics);
        GtkWidget *cancel = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "cancel_button"));
        gtk_widget_set_sensitive(cancel, FALSE);
    }

    static void
    onCancelButton (GtkWidget * button, gpointer data) {
      DBG("onCancelButton...\n");
        cancel_all(data);
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
