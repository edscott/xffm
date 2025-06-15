#ifndef HISTORYENTRY_HH
# define HISTORYENTRY_HH

namespace xf
{
  class HistoryEntry {
    public:
       GtkBox *entryBox(const char *labelText, 
                        const char *tooltipText, 
                        const char *history,
                        void *callback,
                        void *data)
       {
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

          auto bBox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
          g_object_set_data(G_OBJECT(entry), "buttonBox", bBox);

          auto dropdown = gtk_drop_down_new_from_strings(vector);
          g_object_set_data(G_OBJECT(entry), "dropdown", dropdown);

          gtk_entry_buffer_set_text(buffer, vector[0], -1);
          g_free(vector);
          g_signal_connect(G_OBJECT(dropdown), "notify", G_CALLBACK(notify), entry);
          gtk_box_append(bBox, GTK_WIDGET(dropdown));
          gtk_box_append(box, GTK_WIDGET(bBox));

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
            auto image = Dialog::buttonBox(EMBLEM_QUESTION,
              _("Show help"),
              (void *)infoClick,
              (void *)tooltipText);
            
            //auto image = Texture<bool>::getImage(EMBLEM_QUESTION, 18);
            //Basic::setTooltip(GTK_WIDGET(image), tooltipText);
            gtk_box_append(box, GTK_WIDGET(image));
            auto lab = gtk_label_new("   ");
            gtk_box_append(box, GTK_WIDGET(lab));
          }
          g_signal_connect (entry, "activate", G_CALLBACK(callback), data);
                  //BUTTON_CALLBACK(FindSignals<Type>::onFindButton), this);

          return box;
      }
    private:

      static gboolean
      infoClick(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              void *data){
        Dialogs::info((const char *)data);
        return true;
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
      if (!inactive) HistoryEntry::updateEntry(GTK_ENTRY(data), dd);
      //g_object_set_data(G_OBJECT(data), "inactive", NULL);
      //TRACE("notify callback, data=%p, item=%p, selected=%s\n", data,item,selected);
      
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
           ERROR_("*** Error::loadHistory(): unable to open history: \"%s\"\n", history);
           list = g_list_prepend(list, (void *) "");
           return list;
        }
        char line[2048];
        memset (line, 0, 2048);
        while(fgets (line, 2047, file) && !feof (file)) {
            if(strchr (line, '\n')) *strchr (line, '\n') = 0;
            char *newline = compact_line(line);
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

      char * compact_line(const char *line){
          //1. Remove leading and trailing whitespace
          //2. Compact intermediate whitespace
          char *newline= g_strdup(line); 
          g_strstrip(newline);
          char *p = newline;
          for(;p && *p; p++){
              if (*p ==' ') g_strchug(p+1);
          }
          return newline;
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

/*      GListModel *getDropDownModel(GList *list){
        return NULL;
      }*/

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

  };

}

#endif

