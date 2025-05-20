#ifndef DNDBOX__HH
# define DNDBOX__HH


namespace xf {
GList *findResultsWidgets = NULL;
pthread_mutex_t findResultsMutex = PTHREAD_MUTEX_INITIALIZER;
GtkWindow *findDialog;


template <class Type> 
class DnDBox{
public:

  //GList *findResultsWidgets_ = NULL;
/*
    static void
    onResponse (GtkWidget * widget, gpointer data) {
      pthread_mutex_lock(&findResultsMutex);
        findResultsWidgets = g_list_remove(findResultsWidgets, widget);
      pthread_mutex_unlock(&findResultsMutex);
    }
    */

//private:

    static void close(GtkButton *button, GtkWindow *window){
      //gtk_widget_set_visible(window, false);
      auto list = (GSList *)g_object_get_data(G_OBJECT(window), "list");

      for (auto l=list; l && l->data; l=l->next){
        g_free(l->data);
      }
      g_slist_free(list);
      gtk_window_destroy(window);
    }

    static void
    openDnDBox(const gchar *dir, GSList *list){
      if (g_slist_length(list) == 0) return;

      GtkWindow *window = GTK_WINDOW(gtk_window_new());
      g_object_set_data(G_OBJECT(window), "list", list);
      gtk_window_set_title(window, _("Search results"));
      gtk_widget_set_size_request(GTK_WIDGET(window), 500, 400);

      DBG("openDnDBox... title %s\n", dir);
      auto dirLen = strlen(dir)+1;
      auto mainBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
      auto buttonBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
      auto button = Basic::newButtonX(EMBLEM_CLOSE, _("Close"));
      g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(close), window);
      gtk_box_append(buttonBox, GTK_WIDGET(button));
      auto label =GTK_LABEL(gtk_label_new(""));
      auto string = g_strdup_printf(_("Search results for %s"), dir);
      auto markup = g_strconcat("<span color=\"green\">",string,"/</span>", NULL);
      gtk_label_set_markup(label, markup);
      g_free(string);
      g_free(markup);
      gtk_box_append(buttonBox, GTK_WIDGET(label));

      auto listBox = GTK_LIST_BOX(gtk_list_box_new());
      gtk_list_box_set_selection_mode(listBox, GTK_SELECTION_SINGLE);
      for (auto l=list; l && l->data; l=l->next){
        auto path = (char *)l->data;
//        auto listBoxRow = GTK_LIST_BOX_ROW(gtk_list_box_row_new());
        DBG("Process path: %s\n", path);
        auto box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        auto imageBox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
        gtk_box_append(GTK_BOX(box), imageBox);
          
        //auto info = Basic::getInfo(file);
        auto file = g_file_new_for_path(path);
        GError *error_ = NULL;
        auto fileInfo = g_file_query_info(file, "standard::", G_FILE_QUERY_INFO_NONE, NULL, &error_);
        if (error_){
          g_object_unref(G_OBJECT(file));
          g_error_free(error_);
          continue;
        }
        auto image = Texture<bool>::getImage(fileInfo, 24);
        g_object_unref(G_OBJECT(file));
        g_object_unref(G_OBJECT(fileInfo));
        gtk_box_append(GTK_BOX(imageBox), GTK_WIDGET(image));
        
        auto label = gtk_label_new(path+dirLen);
        gtk_box_append(GTK_BOX(box), label);
        gtk_list_box_append(listBox, box);
      }
      //auto columnView = getColumnView(list); 

      gtk_widget_set_vexpand(GTK_WIDGET(mainBox), true);
      gtk_widget_set_hexpand(GTK_WIDGET(mainBox), true); 



      //GtkBox *mainBox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
      //GtkWidget *sw = gtk_scrolled_window_new();
      //gtk_widget_set_vexpand(GTK_WIDGET(listBox), true);
      //gtk_widget_set_hexpand(GTK_WIDGET(listBox), true);      
      auto  sw = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new());
      gtk_scrolled_window_set_policy(sw, GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
      gtk_widget_set_vexpand(GTK_WIDGET(sw), true);
      gtk_widget_set_hexpand(GTK_WIDGET(sw), true); 

      gtk_window_set_child(window, GTK_WIDGET(mainBox));
      gtk_box_append(mainBox, GTK_WIDGET(buttonBox));
      gtk_box_append(mainBox, GTK_WIDGET(sw));
      gtk_scrolled_window_set_child(sw, GTK_WIDGET(listBox));
      // FIXME: unselect all is not working...
      gtk_list_box_unselect_all(listBox);
      
      gtk_widget_realize(GTK_WIDGET(window));
      Basic::setAsDialog(window);
      gtk_window_present(window);



      return;
    }

private:
#if 0
    static void
    activate (GtkTreeView     *treeView,
               GtkTreePath       *tpath,
               GtkTreeViewColumn *column,
               gpointer           data)
    {
        auto object = (DnD<double> *)data;
        object->cancelDragState();
        // Get activated path.
        auto treeModel = gtk_tree_view_get_model(treeView);

        gchar *path;
        GtkTreeIter iter;
        if (!gtk_tree_model_get_iter (treeModel, &iter, (GtkTreePath *)tpath)){
            DBG("tpath does not exist. Aborting activate signal.\n");
            return;
        }
        GdkPixbuf *normal_pixbuf;

        gtk_tree_model_get (treeModel, &iter, 1, &path, -1);
        
        TRACE("base-signals::activate: %s\n", path);
        /*if (!view->loadModel(treeModel, tpath, path)){
            TRACE("base-signals:activate():cannot load %s\n", path);
        }*/
        TRACE("path is %s\n", path);
        gchar *wd = g_path_get_dirname(path);
        GList *pathList = g_list_prepend(NULL, path);
        gchar *command = Run<Type>::getOpenWithCommand(findDialog, pathList, wd);

        if (command) {  

            GError *error = NULL;
            gint argc;
            gchar **argv= NULL; 
            if(!g_shell_parse_argv (command, &argc, &argv, &error)) {
                auto msg = g_strcompress (error->message);
                DBG("%s: %s\n", msg, command);
                g_error_free (error);
                g_free (msg);
            } else {
                Run<Type>::thread_runReap(NULL, (const gchar**)argv, NULL, NULL, NULL);
            }

            g_strfreev(argv);
        }

        g_free(path);
        g_free(wd);
        g_list_free(pathList);
    }
    static void
    setUpSignals(GObject *dialog){
        auto treeView = GTK_TREE_VIEW(g_object_get_data(dialog, "treeView"));
        auto model = GTK_TREE_MODEL(g_object_get_data(dialog, "model"));
    }
#endif
};

} // namespace xf
#endif
