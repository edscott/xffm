#ifndef FILERESPONSE_HH
#define FILERESPONSE_HH


namespace xf {
  class FileResponse {
   GtkBox *mainBox_ = NULL;
   GtkWindow *dialog_ = NULL;
   char *title_ = _("Select Directory");
   const char *iconName_;
   GtkEntry *remoteEntry_ = NULL;
   GtkEntry *mountPointEntry_ = NULL;
   GtkTextView *output_;
public:
    const char *title(void){ return title_;}
    const char *iconName(void){ return "emblem-run";}
    const char *label(void){return "xffm::efs";}
    GtkEntry *remoteEntry(void){return remoteEntry_;}
    GtkEntry *mountPointEntry(void){return mountPointEntry_;}

    ~FileResponse (void){
    }

    FileResponse (void){
    }

     static void *asyncYes(void *data){
      auto dialogObject = (DialogComplex<FileResponse> *)data;
      DBG("%s", "hello world\n");
      return NULL;
    }

    static void *asyncNo(void *data){
      auto dialogObject = (DialogComplex<FileResponse> *)data;
      DBG("%s", "goodbye world\n");
      return NULL;
    }
    static gint 
    compareFunction(const void *a, const void *b, void *data){

        GFileInfo *infoA = G_FILE_INFO(a);
        GFileInfo *infoB = G_FILE_INFO(b);

        auto nameA = g_file_info_get_name(infoA);
        auto nameB = g_file_info_get_name(infoB);
        

        if (strcmp(g_file_info_get_name(infoA), "..")==0) return -1;
        if (strcmp(g_file_info_get_name(infoB), "..")==0) return 1;
        
        // by name 
        return strcasecmp(nameA, nameB);
    }
    
    static GListModel *getChildModel (gpointer listItem, gpointer user_data)
    {
      //auto item = GTK_LIST_ITEM(listItem);
      //auto info = G_FILE_INFO(gtk_list_item_get_item(item));
      auto info = G_FILE_INFO(listItem);
      auto path = Basic::getPath(info);
      DBG("getChildModel %s\n", path);
      auto listModel = getListModel(path);
      g_free(path);
      return listModel;
    }

    static GListModel *getListModel(const char *path){
        GError *error_ = NULL;
        auto store = g_list_store_new(G_TYPE_FILE_INFO);
        GFile *file = g_file_new_for_path(path);
        GFileEnumerator *dirEnum = 
          g_file_enumerate_children (file,"standard::",G_FILE_QUERY_INFO_NONE,NULL, &error_);
        if (error_) {
          TRACE("*** Error::g_file_enumerate_children: %s\n", error_->message);
          Print::printError(Child::getOutput(), g_strdup(error_->message));
          g_error_free(error_);
          return NULL;
        }
        GFile *outChild = NULL;
        GFileInfo *outInfo = NULL;
        do {
          g_file_enumerator_iterate (dirEnum, &outInfo, &outChild,
              NULL, // GCancellable* cancellable,
              &error_);
          if (error_) {
            DBG("*** Error::g_file_enumerator_iterate: %s\n", error_->message);
            return NULL;
          }
          if (!outInfo || !outChild) break;
          if (g_file_info_get_is_symlink(outInfo)) continue;
          if (g_file_info_get_is_backup(outInfo)) continue;
          if (g_file_info_get_is_hidden(outInfo)) continue;
          auto path = g_file_get_path(outChild);

          if (g_file_test(path, G_FILE_TEST_IS_DIR)){
            //Important: if this is not set, then the GFile cannot be obtained from the GFileInfo:
            g_file_info_set_attribute_object(outInfo, "standard::file", G_OBJECT(outChild));          
            g_list_store_insert_sorted(store, G_OBJECT(outInfo), compareFunction, NULL);
            TRACE("insert path=%s info=%p\n", g_file_get_path(outChild), outInfo);
          }
          g_free(path);
          /*auto _path = g_file_get_path(outChild);
          setPaintableIcon(outInfo, _path);
          g_free(_path);*/
        } while (true);
        g_object_unref(file);
        return G_LIST_MODEL(store);
    }

      static void
      factoryTeardown(GtkSignalListItemFactory *self, GObject *object, void *data){
        //DBG("factoryTeardown...\n");

      }
      static void
      factorySetup(GtkSignalListItemFactory *self, GObject *object, void *data){
        auto box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        DBG("factorySetup...\n");        
#if 1
        auto expander = gtk_tree_expander_new();
        gtk_tree_expander_set_child(GTK_TREE_EXPANDER(expander), box);
        gtk_list_item_set_child(GTK_LIST_ITEM(object), expander); 
#else
        gtk_list_item_set_child(GTK_LIST_ITEM(object), box); 
#endif
        g_object_set_data(G_OBJECT(object), "box", box);
        auto paintable = Texture<bool>::load16("folder");
        auto image = gtk_image_new_from_paintable(paintable);
        gtk_box_append(GTK_BOX(box), image);
        auto label = gtk_label_new("foo");
        gtk_box_append(GTK_BOX(box), label);
        g_object_set_data(G_OBJECT(object), "label", label);
        DBG("factorySetup2...\n");        

      }
      static void
      factoryUnbind(GtkSignalListItemFactory *self, GObject *object, void *data){
        DBG("factoryUnbind...\n");
      }
      static void
      factoryBind(GtkSignalListItemFactory *factory, GObject *object, void *data){
        DBG("factoryBind...\n");
#if 10
        auto list_item =GTK_LIST_ITEM(object);
        auto treeListRow = GTK_TREE_LIST_ROW(gtk_list_item_get_item(list_item));
        auto info = G_FILE_INFO(gtk_tree_list_row_get_item(treeListRow));
        //auto info = G_FILE_INFO(object);
        DBG("info name = %s\n", g_file_info_get_name(info));
        auto expander = gtk_list_item_get_child( GTK_LIST_ITEM(object) );
        auto box = GTK_BOX(gtk_tree_expander_get_child( GTK_TREE_EXPANDER(expander) ));
//        auto box = GTK_BOX(gtk_expander_get_child( GTK_EXPANDER(expander) ));
#else
        auto list_item =GTK_LIST_ITEM(object);
        auto info = G_FILE_INFO(gtk_list_item_get_item(list_item));
        auto box = GTK_BOX(gtk_list_item_get_child( list_item ));
#endif
        DBG("factoryBind2...\n");
        const char *name = g_file_info_get_name(info);
        auto label = GTK_LABEL(g_object_get_data(object, "label"));
        
        auto maxLen = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(factory), "maxLen"));
        auto format = g_strdup_printf("<tt>%%-%ds", maxLen);
        char buffer[128];
        snprintf(buffer, 128, (const char *)format, name);
        auto markup = g_strdup_printf("%s", buffer);

        auto path = Basic::getPath(info);
        struct stat st;
        lstat(path, &st);
        g_free(path);
        auto m1 = Basic::statInfo(&st);
        char *markup2 = g_strdup(" <span color=\"blue\">");
        Basic::concat(&markup2, m1);
        Basic::concat(&markup2, "</span></tt>");           
        g_free(m1);

        Basic::concat(&markup, markup2);
        g_free(markup2);
                    
        gtk_label_set_markup(label, markup);
        g_free(markup);
        
      }
/*
    static void addGestureClickDown(GtkWidget *self, GObject *item, GridView<DirectoryClass> *gridView_p){
      g_object_set_data(G_OBJECT(self), "item", item);
      auto gesture = gtk_gesture_click_new();
      gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),1); 
      // 1 for select
      TRACE("addGestureClickDown: self = %p, item=%p\n", self, item);
      g_signal_connect (G_OBJECT(gesture) , "released", EVENT_CALLBACK (reload_f), (void *)gridView_p);
      gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(gesture));
      gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture), 
          GTK_PHASE_CAPTURE);

    }    
*/
    GtkBox *mainBox(void) {
        //auto dialog = gtk_dialog_new ();
        //gtk_window_set_type_hint(GTK_WINDOW(dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
        mainBox_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        //gtk_widget_set_size_request(GTK_WIDGET(mainBox_), 550, 400);
        gtk_widget_set_vexpand(GTK_WIDGET(mainBox_), false);
        gtk_widget_set_hexpand(GTK_WIDGET(mainBox_), false);

        auto label = gtk_label_new("file response dialog now...\n");
        gtk_box_append(mainBox_, label);

        auto sw = gtk_scrolled_window_new();
        gtk_widget_set_vexpand(GTK_WIDGET(sw), true);
        gtk_widget_set_hexpand(GTK_WIDGET(sw), true);
        gtk_box_append(mainBox_, sw);
        // listview...
        // gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(sw), GTK_WIDGET(output_));
        gtk_widget_set_size_request(GTK_WIDGET(sw), 600, 200);
        auto listModel = getListModel("/home/tmp");
#if 10
  GtkTreeListModel * treemodel = gtk_tree_list_model_new (G_LIST_MODEL (listModel),
                                       FALSE,
                                       TRUE,
                                       getChildModel,
                                       NULL,
                                       NULL);
       

  g_object_set(G_OBJECT(treemodel), "passthrough", FALSE, NULL);
  auto filterModel = gtk_filter_list_model_new (G_LIST_MODEL (treemodel), NULL);
  GtkSingleSelection *selection = gtk_single_selection_new (G_LIST_MODEL (filterModel));
#else  
  GtkSingleSelection *selection = gtk_single_selection_new (listModel);
#endif
  
  auto maxLen = Basic::getMaxNameLen(listModel);
  GtkListItemFactory *factory = gtk_signal_list_item_factory_new();

  g_object_set_data(G_OBJECT(factory), "maxLen", GINT_TO_POINTER(maxLen));
  g_signal_connect( factory, "setup", G_CALLBACK(factorySetup), this );
  g_signal_connect( factory, "bind", G_CALLBACK(factoryBind), this);
  g_signal_connect( factory, "unbind", G_CALLBACK(factoryUnbind), this);
  g_signal_connect( factory, "teardown", G_CALLBACK(factoryTeardown), this);
  
  auto listview = gtk_list_view_new (GTK_SELECTION_MODEL(selection), factory);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(sw), GTK_WIDGET(listview));

 
      

        auto action_area = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_vexpand(GTK_WIDGET(action_area), false);
        gtk_widget_set_hexpand(GTK_WIDGET(action_area), false);
        gtk_box_append(mainBox_, GTK_WIDGET(action_area));

        auto cancelButton = Basic::mkButton("emblem-redball", _("Cancel"));
        gtk_box_append(action_area,  GTK_WIDGET(cancelButton));
        gtk_widget_set_vexpand(GTK_WIDGET(cancelButton), false);

        auto saveButton = Basic::mkButton ("emblem-floppy", _("Accept"));
        gtk_box_append(action_area,  GTK_WIDGET(saveButton));
        gtk_widget_set_vexpand(GTK_WIDGET(saveButton), false);


        g_signal_connect (G_OBJECT (saveButton), "clicked", G_CALLBACK (button_save), this);
        g_signal_connect (G_OBJECT (cancelButton), "clicked", G_CALLBACK (button_cancel), this);

        // FIXME: 
        return mainBox_;
    }

    void setSubClassDialog(GtkWindow *dialog){
      dialog_ = dialog;
    }

    GtkWindow *dialog(void){return dialog_;}

    private:

    
 

    gboolean save(void){
      return true;        
    }

    static void
    button_save (GtkButton * button, gpointer data) {
      auto subClass = (FileResponse *)data;
      if (subClass->save()){
        g_object_set_data(G_OBJECT(subClass->dialog()), "response", GINT_TO_POINTER(1));
      }
    }

    static void
    button_cancel (GtkButton * button, gpointer data) {
      auto subClass = (FileResponse *)data;
      g_object_set_data(G_OBJECT(subClass->dialog()), "response", GINT_TO_POINTER(-1));
    }

  };

  class FileDialog {
    public:
    static void newFileDialog(GtkWindow *parent, GList **children){
      auto dialogObject = new DialogComplex<FileResponse>;
      dialogObject->setParent(parent);
      auto dialog = dialogObject->dialog();
      *children = g_list_append(*children, dialog);
      
      gtk_window_set_decorated(dialog, true);
      dialogObject->setSubClassDialog();

      gtk_widget_realize(GTK_WIDGET(dialog));
      Basic::setAsDialog(GTK_WIDGET(dialog), "dialog", "Dialog");
      gtk_window_present(dialog);

      dialogObject->run();
      

    }



  };


}
#endif

