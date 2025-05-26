#ifndef DNDBOX__HH
# define DNDBOX__HH


namespace xf {
GList *findResultsWidgets = NULL;
pthread_mutex_t findResultsMutex = PTHREAD_MUTEX_INITIALIZER;
GtkWindow *findDialog;


template <class Type> 
class DnDBox{
public:

    static GtkWindow *
    openDnDBox(const gchar *dir, GSList *list, GtkTextView *textview){
      TRACE("openDnDBox... dir = %s\n", dir);
      if (g_slist_length(list) == 0) return NULL;

      auto window = createWindow(dir, list);
      g_object_set_data(G_OBJECT(window), "list", list);
      g_object_set_data(G_OBJECT(window), "textview", textview);
      auto mainBox = mkMainBox(dir, window);
      auto listBox = mkListBox(dir,list,(void *)window);
      g_object_set_data(G_OBJECT(listBox), "textview", textview);
      auto sw = mkScrolledWindow();
     
      gtk_window_set_child(window, GTK_WIDGET(mainBox));
      gtk_box_append(mainBox, GTK_WIDGET(sw));
      gtk_scrolled_window_set_child(sw, GTK_WIDGET(listBox));
      
      mkGesture(GTK_WIDGET(listBox), (void *)window);

      // unselect all does not work if dnd source not set.
      //gtk_list_box_unselect_all(listBox);
      gtk_list_box_select_all(listBox);
      
      gtk_widget_realize(GTK_WIDGET(window));
      Basic::setAsDialog(window);
      gtk_window_present(window);

      return window;
    }

private:

    static void close(GtkButton *button, void *window){
      auto list = (GSList *)g_object_get_data(G_OBJECT(window), "list");

      for (auto l=list; l && l->data; l=l->next){
        g_free(l->data);
      }
      g_slist_free(list);
      auto dir = g_object_get_data(G_OBJECT(window), "dir");
      g_free(dir);
      gtk_widget_set_visible(GTK_WIDGET(window), false);
      gtk_window_destroy(GTK_WINDOW(window));
    }

    static void
    edit_command (GSList *list, void *window, GtkTextView *textview) {
        auto editor = Basic::getEditor();
        if (!editor || strlen(editor)==0){
          Print::printError(textview, g_strdup_printf("%s\n",
                        _("No editor for current action.")));
            return;
        }
        gchar *command;
        /*if (Run<bool>::runInTerminal(editor)){
            command = Run<Type>::mkTerminalLine(editor, "");
        } else {
            command = g_strdup(editor);
        }*/
        command = g_strdup(editor);
      
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
        Run<Type>::thread_run(textview, command, FALSE);
        close(NULL, window);
    }

    
    static void
    onEditButton (GtkWidget * button, void *window) {
      auto list = (GSList *)g_object_get_data(G_OBJECT(window), "list");
      auto textview = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(window), "textview"));
      TRACE("onEditButton...\n");
      Print::showText(textview);        
      edit_command(list, window, textview);
    }

    static GtkBox *mkMainBox(const gchar *dir, GtkWindow *window){
      auto mainBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));

      auto buttonBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
 
 
      GtkButton *edit_button = NULL;
      auto editor =Basic::getEditor();
      if (editor && strlen(editor)){
          auto basename = g_strdup(editor);
          if (strchr(basename, ' ')) *strchr(basename, ' ') = 0;
          auto editor_path = g_find_program_in_path(basename);
          g_free(basename);
          if (editor_path){
              auto icon_id = Basic::getAppIconName(editor_path, EMBLEM_EDIT);
              auto Image = GTK_WIDGET(Texture<bool>::getImage(icon_id, 20));
              edit_button = GTK_BUTTON(gtk_button_new());
              gtk_button_set_child(edit_button,Image);
              Basic::setTooltip(GTK_WIDGET(edit_button), _("Edit all"));
              g_free(icon_id);
              g_free(editor_path);
              gtk_widget_set_sensitive(GTK_WIDGET(edit_button), true);
              gtk_box_append(buttonBox, GTK_WIDGET(edit_button));
              g_signal_connect(G_OBJECT(edit_button), "clicked", G_CALLBACK(onEditButton), window);
         } //else gtk_widget_set_sensitive(GTK_WIDGET(edit_button), false);
          
      } else {
          TRACE("getEditor() = \"%s\"\n", editor);
      }
     
      auto label =GTK_LABEL(gtk_label_new(""));
      auto string = g_strdup_printf(_("Search results for %s"), dir);
      auto markup = g_strconcat("<span color=\"green\">",string,"/</span>", NULL);
      gtk_label_set_markup(label, markup);
      g_free(string);
      g_free(markup);
      gtk_box_append(buttonBox, GTK_WIDGET(label));

      // pack end hack
      auto margin = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
      gtk_widget_set_hexpand(GTK_WIDGET(margin), true); 
      auto space = gtk_label_new(" ");
      gtk_box_append(GTK_BOX(margin),space);
      gtk_box_append(GTK_BOX(buttonBox),margin);

     auto button = Basic::newButtonX(EMBLEM_CLOSE, _("Close"));
      g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(close), window);
      gtk_box_append(buttonBox, GTK_WIDGET(button));
          
      gtk_box_append(mainBox, GTK_WIDGET(buttonBox));
      gtk_widget_set_vexpand(GTK_WIDGET(mainBox), true);
      gtk_widget_set_hexpand(GTK_WIDGET(mainBox), true); 
      return mainBox;
    }

    static GtkListBox *mkListBox(const gchar *dir, GSList *list, void *window){
      auto dirLen = strlen(dir)+1;
      auto listBox = GTK_LIST_BOX(gtk_list_box_new());
      gtk_list_box_set_selection_mode(listBox,  GTK_SELECTION_SINGLE );

      for (auto l=list; l && l->data; l=l->next){
        auto path = (const char *)l->data;
        TRACE("Process path: %s\n", path);
        auto box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        auto imageBox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
        gtk_box_append(GTK_BOX(box), imageBox);
          
        //auto info = Basic::getInfo(file);
        auto file = Basic::getGfile(path);
        if (!file) continue;
        auto fileInfo = Basic::getFileInfo(file);
        if (!fileInfo){ g_object_unref(G_OBJECT(file)); continue; }

        auto image = Texture<bool>::getImage(fileInfo, 24);
        g_object_unref(G_OBJECT(file));
        g_object_unref(G_OBJECT(fileInfo));
        gtk_box_append(GTK_BOX(imageBox), GTK_WIDGET(image));
        
        auto label = gtk_label_new(path+dirLen);
        gtk_box_append(GTK_BOX(box), label);


        auto row = GTK_LIST_BOX_ROW(gtk_list_box_row_new());
        g_object_set_data(G_OBJECT(row), "window", window);
        gtk_list_box_row_set_child(row, GTK_WIDGET(box));
        g_signal_connect(G_OBJECT(row), "activate", G_CALLBACK(activate), window);
        g_object_set_data(G_OBJECT(row), "listBox", listBox);

        g_object_set_data(G_OBJECT(row), "label", label);

        GtkDragSource *dragSource = gtk_drag_source_new();
        g_signal_connect (dragSource, "prepare", G_CALLBACK (dragPrepare), row);
        g_signal_connect (dragSource, "drag-begin", G_CALLBACK (dragBegin), row);
        gtk_widget_add_controller (GTK_WIDGET (row), GTK_EVENT_CONTROLLER (dragSource));

        gtk_list_box_append(listBox, GTK_WIDGET(row));
        if (l == list) gtk_list_box_select_row (listBox, row);
      }
      return listBox;
    }

    static GdkContentProvider *
    dragPrepare(GtkDragSource* self, gdouble x, gdouble y, void *row){
        TRACE("drag prepare\n");
            GdkContentProvider *content;
            char *string = g_strdup("");
            auto window = g_object_get_data(G_OBJECT(row), "window");
            auto label = GTK_LABEL(g_object_get_data(G_OBJECT(row), "label"));
            auto dir = (const char *)g_object_get_data(G_OBJECT(window), "dir");
            auto text = gtk_label_get_text(label);

            auto path = g_strconcat(dir,"/",text,NULL);
            auto selectedPath = g_strconcat("file://", path, "\n", NULL);
            Basic::concat(&string, selectedPath);
            g_free(path);
            g_free(selectedPath);
            
            GBytes *bytes = g_bytes_new(string, strlen(string)+1);
            content = gdk_content_provider_new_for_bytes ("text/uri-list", bytes);
            g_free(string);
            return content;
    }
    static void
    dragBegin (GtkDragSource *source,
                   GdkDrag       *drag,
                   GtkListBoxRow *self)
    {
        TRACE("drag begin\n");
    /*
      // Set the widget as the drag icon
      GdkPaintable *paintable = gtk_widget_paintable_new (GTK_WIDGET (self));
      gtk_drag_source_set_icon (source, paintable, 0, 0);
      g_object_unref (paintable);
      */
    }




    static GtkScrolledWindow *mkScrolledWindow(){
      auto  sw = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new());
      gtk_scrolled_window_set_policy(sw, GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
      gtk_widget_set_vexpand(GTK_WIDGET(sw), true);
      gtk_widget_set_hexpand(GTK_WIDGET(sw), true); 
      return sw;
   }


    static GtkWindow *createWindow(const gchar *dir, GSList *list){
      GtkWindow *window = GTK_WINDOW(gtk_window_new());
      g_object_set_data(G_OBJECT(window), "list", list);
      gtk_window_set_title(window, _("Search results"));
      gtk_widget_set_size_request(GTK_WIDGET(window), 500, 400);
      g_object_set_data(G_OBJECT(window), "dir", g_strdup(dir));
      return window;
    }

    static void mkGesture(GtkWidget *widget, void *data){
      auto gesture1 = gtk_gesture_click_new();
      gtk_event_controller_set_propagation_phase (GTK_EVENT_CONTROLLER(gesture1),GTK_PHASE_CAPTURE);
      gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture1),1);
      gtk_widget_add_controller (widget, GTK_EVENT_CONTROLLER(gesture1));    
      g_signal_connect (G_OBJECT(gesture1) , "released", 
          EVENT_CALLBACK (cvClick), data);

    }

    static void activate(GtkListBoxRow* row, void *window){
      auto listBox = GTK_LIST_BOX(g_object_get_data(G_OBJECT(row), "listBox"));
      openWith(listBox, row, window);
      return;      
    }

    static void openWith(GtkListBox *listBox, GtkListBoxRow *row, void *window){
      auto textview = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(listBox),"textview"));
      auto label = GTK_LABEL(g_object_get_data(G_OBJECT(row), "label"));
      auto dir = (const char *)g_object_get_data(G_OBJECT(window), "dir");
      auto text = gtk_label_get_text(label);

      TRACE("path = \"%s/%s\"\n", dir,text);
      auto path = g_strconcat(dir, G_DIR_SEPARATOR_S, text, NULL);
      new OpenWith<bool>(textview, path);
      g_free(path);
      close(NULL, window);

      return;
    }

    static gboolean
    cvClick (
              GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              void *window ){
      if (n_press != 2) return FALSE;
      TRACE("cvClick n_press = %d\n", n_press);
      auto listBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
      auto row = gtk_list_box_get_selected_row (GTK_LIST_BOX(listBox));
      openWith(GTK_LIST_BOX(listBox), row, window);
      return TRUE;
    }

};

} // namespace xf
#endif
