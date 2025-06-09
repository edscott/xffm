#ifndef FFUNCTIONS_HH
# define FFUNCTIONS_HH
namespace xf {
  class Ffunctions {
    public:
      
     static const char **ftypes(void){
       static const char *ftypes_[] = {
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
       return ftypes_;
     }
     static const char **ft(void) {
       static const char *ft_[] = {
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
       return ft_;
     }


    protected:

      
    const char *
    filter_text_help(void) {
      return _("Basic rules:\n" "\n"
                "*  Will match any character zero or more times.\n"
                "?  Will match any character exactly one time\n"
                "[] Match any character within the [] \n"
                "^  Match at beginning of string\n" 
                "$  Match at end of string \n");
    }
    const char *
    grep_text_help(void) {
      return _("Reserved characters for extended regexp are . ^ $ [ ] ? * + { } | \\ "
        "( ) : \n"
        "In  basic regular expressions the metacharacters ?, +, {, |, (, "
        "and ) \n"
        "  lose their special meaning.\n"
        "\n"
        "The  period . matches  any  single  character.\n"
        "The caret ^ matches at the start of line.\n"
        "The dollar $ matches at the end of line.\n"
        "\n"
        "Characters within [ ] matches any single character in the list.\n"
        "Characters within [^ ] matches any single character *not* in the "
        "list.\n"
        "Characters inside [ - ] matches a range of characters (ie [0-9] or "
        "[a-z]).\n"
        "\n"
        "A regular expression may be followed by one of several repetition "
        "operators:\n"
        "?      The preceding item is optional and matched\n"
        "       at most once.\n"
        "*      The preceding item will be matched zero\n"
        "       or more times.\n"
        "+      The preceding item will be matched one or\n"
        "       more times.\n"
        "{n}    The preceding item is matched exactly n times.\n"
        "{n,}   The preceding item is matched n or more times.\n"
        "{n,m}  The preceding item is matched at least n times,\n"
        "       but not more than m times.\n"
        "\n"
        "To match any reserved character, precede it with \\. \n"
        "\n"
        "Two regular expressions may be joined by the logical or operator |.\n"
        "Two regular expressions may be concatenated.\n"
        "\n"
        "More information is available by typing \"man grep\"\n"
);
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

      
      TRACE("*** notify name=%s\n", g_param_spec_get_name(pspec));

      if (!GTK_IS_ENTRY(data)) {
        TRACE("*** FIXME: identify notify call\n");
        return; // FIXME hack!
                // notify is called when window is closed
                // and by then the pointer to entry is invalid.
                // Must use pspec or something to distinguish the
                // notify call.
      }
      auto dd = GTK_DROP_DOWN(self); 
      auto inactive = g_object_get_data(G_OBJECT(data), "inactive");
      if (!inactive) updateEntry(GTK_ENTRY(data), dd);
      //g_object_set_data(G_OBJECT(data), "inactive", NULL);
      //TRACE("notify callback, data=%p, item=%p, selected=%s\n", data,item,selected);
      
    }

    static void updateEntry(GtkEntry *entry, GtkDropDown *dd){
      //if (g_object_get_data(G_OBJECT(entry), "inactive")) return;
      auto item = GTK_STRING_OBJECT(gtk_drop_down_get_selected_item(dd));
      auto selected = gtk_string_object_get_string(item);
      auto buffer = gtk_entry_get_buffer(GTK_ENTRY(entry));
      gtk_entry_buffer_set_text(buffer, selected, -1);
      auto sWidget = g_object_get_data(G_OBJECT(entry), "sWidget");
      if (sWidget){
        gtk_widget_set_sensitive(GTK_WIDGET(sWidget), (selected && strlen(selected) > 0));
      }
      gtk_editable_set_position(GTK_EDITABLE(entry), strlen(selected));

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


    static void 
    saveHistory (GtkEntry *entry, const gchar *history, const gchar *text) {
        char *historyDir = g_path_get_dirname(history);
        TRACE("history dir = %s\n", historyDir);
        if (!g_file_test(historyDir,G_FILE_TEST_IS_DIR)){
            g_mkdir_with_parents (historyDir, 0770);
        }
        g_free(historyDir);

        auto list = (GList *)g_object_get_data(G_OBJECT(entry), "list");

        // if item is already in history, bring it to the front
        // else, prepend item.
       
        bool found = false;
        for (auto l=list; l && l->data; l=l->next){
          auto data = (char *)l->data;
          if (strcmp(data, text)==0){
            list = g_list_remove(list, data);
            list = g_list_prepend(list,data);
            found = true;
            break;
          }
        }
        if (!found){
            list = g_list_prepend(list,g_strdup(text));
        }
        g_object_set_data(G_OBJECT(entry), "list", list);
        // rewrite history file
        FILE *historyFile = fopen (history, "w");
        if(!historyFile) {
            ERROR_("saveHistory(): unable to write to file: \"%s\"\n", history);
            return;
        }

        for (auto l=list; l && l->data; l=l->next){
            fprintf (historyFile, "%s\n", (char *)l->data);
        }
        fclose (historyFile);
        return;
    }

        static void
        updateDD (GtkEntry *entry, GtkDropDown *dropdown){
          auto buffer = gtk_entry_get_buffer(entry);
          auto text = gtk_entry_buffer_get_text(buffer);
          TRACE("updateDD: text=%s\n", text);
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
          auto sWidget = g_object_get_data(G_OBJECT(entry), "sWidget");
          if (!vector) vector = historyVector(list);
          
          if (sWidget){
            if (strlen(text)) gtk_widget_set_sensitive(GTK_WIDGET(sWidget),true);
            else gtk_widget_set_sensitive(GTK_WIDGET(sWidget), false);
          }

          if (vector) {
            auto model = G_LIST_MODEL (gtk_string_list_new (vector));
            //auto button = GTK_DROP_DOWN(g_object_get_data(G_OBJECT(entry), "button"));
            g_object_set_data(G_OBJECT(dropdown), "halt", GINT_TO_POINTER(1));
            gtk_drop_down_set_model(dropdown, model);
            gtk_drop_down_set_selected(dropdown, 0);
          } 
          g_free(vector);
        }

   
    static gboolean
    processKey1 (GtkEventControllerKey* self,
          guint keyval,
          guint keycode,
          GdkModifierType state,
          gpointer data){
      auto controller = GTK_EVENT_CONTROLLER(self);
      auto entry = GTK_ENTRY(gtk_event_controller_get_widget(controller));
      TRACE("processKey1 keyval=%d\n", keyval);
      if (keyval != GDK_KEY_Tab) {
        return false;
      }
      TRACE("got tab key, update entry buffer if possible\n");
      auto dropdown = GTK_DROP_DOWN(data);
      updateEntry(entry, dropdown);
      
      
      return true;
    }
   
    static gboolean
    processKey2 (GtkEventControllerKey* self,
          guint keyval,
          guint keycode,
          GdkModifierType state,
          gpointer data){
      auto controller = GTK_EVENT_CONTROLLER(self);
      auto entry = GTK_ENTRY(gtk_event_controller_get_widget(controller));
      TRACE("processKey2 keyval=%d\n", keyval);
      switch (keyval){
        case GDK_KEY_Return: 
        case GDK_KEY_BackSpace:
        case GDK_KEY_Delete:
        case GDK_KEY_Home:
        case GDK_KEY_Left:
        case GDK_KEY_Up:
        case GDK_KEY_Right:
        case GDK_KEY_Page_Up:
        case GDK_KEY_Page_Down:
        case GDK_KEY_End:
        case GDK_KEY_Begin:
          return false;
      }
      auto dropdown = GTK_DROP_DOWN(g_object_get_data(G_OBJECT(entry), "dropdown"));
      updateDD(entry, dropdown);
      auto buffer = gtk_entry_get_buffer(entry);

      return false;
    }

    static void addKeyController1(GtkEntry  *entry, void *data){
        auto keyController = gtk_event_controller_key_new();
        gtk_event_controller_set_propagation_phase(keyController, GTK_PHASE_CAPTURE);
        gtk_widget_add_controller(GTK_WIDGET(entry), keyController);
        g_signal_connect (G_OBJECT (keyController), "key-pressed", 
            G_CALLBACK (processKey1), (void *)data);
    }

    static void addKeyController2(GtkEntry  *entry, void *data){
        auto keyController = gtk_event_controller_key_new();
        gtk_event_controller_set_propagation_phase(keyController, GTK_PHASE_BUBBLE);
        gtk_widget_add_controller(GTK_WIDGET(entry), keyController);
        g_signal_connect (G_OBJECT (keyController), "key-released", 
            G_CALLBACK (processKey2), (void *)data);
    }

    static gboolean
    ddClick1 (
              GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              GtkEntry *entry ){
      //if (n_press != 2) return FALSE;
      TRACE("ddClick ddclick1 n_press = %d\n", n_press);
      auto buffer = gtk_entry_get_buffer(entry);
      g_object_set_data(G_OBJECT(entry), "inactive", GINT_TO_POINTER(1));
      auto dd = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
      updateEntry(entry, GTK_DROP_DOWN(dd));
      //gtk_entry_buffer_set_text(buffer, "foo", -1);
      return false;
    }
    static gboolean
    ddClick2 (
              GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              GtkEntry *entry ){
      TRACE("ddClick ddclick2 n_press = %d\n", n_press);
      g_object_set_data(G_OBJECT(entry), "inactive", NULL);
      return false;
    }
        

  };
  
}
#endif
