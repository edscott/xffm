#ifndef FILERESPONSE_HH
#define FILERESPONSE_HH

namespace xf {
  template <class Type> class EfsResponse;
  template <class Type> class FileDialog;
  //
  // FileResponse is the template used for construction of the
  // class which in turn will be used by the dialog construction
  // template to construct the dialog object.
  //
  // The resulting dialog object will point to the actual
  // FileResponse object with object->subClass().
  //
  // The FileResponse object, in turn, has a pointer to a
  // FileResponsePathbar object, subClass()->responsePathbar_p.
  // 
  // The FileResponsePathbar class could also be inherited,
  // but methinks this way is more clear to keep things
  // a bit separated.
  //
  // The FileResponse object, in turn, will also construct
  // a mkdirResponse dialog to obtain the name of a directory
  // which will be created in the root of the columnView tree. 
  //

  template <class Type>
  class FileResponse {

private:

    GtkBox *mainBox_ = NULL;
    GtkWindow *dialog_ = NULL;
    char *title_ = _("Select Directory");
    const char *iconName_;
    GtkEntry *remoteEntry_ = NULL;
    GtkEntry *mountPointEntry_ = NULL;
    GtkTextView *output_;
    GtkWidget *sw_;
    FileResponsePathbar *responsePathbar_p;
    char *startFolder_ = NULL;
    GtkSingleSelection *selectionModel_;
    GtkWidget *selectLabel_;
    GtkEntry *parentEntry_=NULL;

public:
    GtkEntry *parentEntry(void){ return parentEntry_;}
    void parentEntry(void *value){parentEntry_ = GTK_ENTRY(value);}

    GtkLabel *selectLabel(void){return GTK_LABEL(selectLabel_);}
    GtkSingleSelection *selectionModel(void){ return selectionModel_;}
    
    char *startFolder(){return  startFolder_;}
    void startFolder(const char *value){
      g_free(startFolder_);
      if (!value) startFolder_ = g_strdup("/");
      else startFolder_ = g_strdup(value);
    }

    GtkWidget *sw(void){ return sw_;}
    const char *title(void){ return title_;}
    const char *iconName(void){ return "emblem-run";}
    const char *label(void){return "xffm::efs";}
    GtkEntry *remoteEntry(void){return remoteEntry_;}
    GtkEntry *mountPointEntry(void){return mountPointEntry_;}

    FileResponse (void){
      DBG("***initial FileResponse=%p\n", this);
      responsePathbar_p = new FileResponsePathbar((void *)reload_f, (void *)this);
    }

    ~FileResponse (void){
      delete responsePathbar_p;
    }

    // static void *asyncYes(void *data)
    //
    // When dialog propery "response" is > 0, this function
    // is called in main context before FileResponse object 
    // is deleted along with the dialog object. Here we set
// any action to be performed on the value of "response".
    //
    // It really does not matter which thread queues this
    // function to the main context thread.
    // 
     static void *asyncYes(void *data){
      auto dialogObject = (DialogComplex<FileResponse> *)data;
      DBG("%s", "hello world\n");
      const char *path = dialogObject->subClass()->responsePathbar_p->path();
      auto label = dialogObject->subClass()->selectLabel();
      const char *base = gtk_label_get_text(label);
      char *target = NULL;
      if (strcmp(base, _("No folder selected."))){ // folder selected...
        target = g_strconcat(path, G_DIR_SEPARATOR_S, base, NULL);
        auto *entry = dialogObject->subClass()->parentEntry();
        auto *buffer = gtk_entry_get_buffer(entry);
        gtk_entry_buffer_set_text(buffer, target, -1);
      }
/*      
      auto p = (EfsResponse<Type> *)dialogObject->subClass()->parentObject();
      const char *path = dialogObject->subClass()->responsePathbar_p->path();
      auto label = dialogObject->subClass()->selectLabel();
      const char *base = gtk_label_get_text(label);
      char *dir = NULL;
      if (strcmp(base, _("No folder selected."))){ // folder actually selected...
        dir = g_strconcat(path, G_DIR_SEPARATOR_S, base, NULL);
        auto retval = p->asyncCallback((void *)dir);
        DBG("asyncCallback(\"%s\") --> %s...\n", dir, (const char *)retval);
        g_free(dir);
      }
 */     
      //auto retval = p->asyncCallback((void *)"foo");
      //DBG("asyncCallback(\"foo\") --> %s...\n", (const char *)retval);
      return NULL;
    }
    
    // static void *asyncNo(void *data)
    //
    // On the other hand, when the dialog property "response" 
    // is < 0, this function is called to cancel the dialog.
    // This is also called in the main context thread.
    //
    // Note that while "response" == NULL, or zero, dialog
    // window will remain on top of parent window in wait
    // for the "response" property to be set != NULL.
    //
    static void *asyncNo(void *data){
      auto dialogObject = (DialogComplex<FileResponse> *)data;
      DBG("%s", "goodbye world\n");
      return NULL;
    }

    // GtkBox *mainBox(const char *folder)
    //
    // subClass()->mainBox(const char *folder) will construct
    // the contents of the GtkWindow, dialogObject->dialog().
    // The parameter folder defines the directory which will
    // be displayed in the columnView tree. 
    //
    // The folder parameter may be NULL, in which case the
    // directory to be shown will be the root "/". 
    // Not compatible with mingw-x86_64.
    // 
    GtkBox *mainBox(const char *folder) {
      startFolder(folder);
      return constructMainBox();
    }

    // void dialog(GtkWindow *value)
    //
    // Set a pointer to the GtkWindow in the FileResponse
    // object so that it can be referred to in the
    // async main context thread callbacks.
    // 
    void dialog(GtkWindow *value){
      dialog_ = value;
    }

    // void *asyncCallback(void *data)
    //
    // This will be executed by the subClassObject->subClass(),
    // In other words, by the DialogEntry<mkdirResponse> dialog
    // subClass object.
    //
    void *asyncCallback(void *data){
       auto path = (const char *)data;
       //auto label = this->selectLabel();
       //gtk_label_set_markup(label, text); 
       reload(path);
       return NULL;
       //DBG("asyncCallback(%s)...\n", (const char *)data);
       //return (void *) "foo";
    }

    //void *asyncCallbackData(void)
    //
    // Just for completeness for now.
    // 
    void *asyncCallbackData(void){
      DBG("asyncCallbackData...\n");
      return (void *) "bar";
    }

////////////////////////////////////////////////////////////////////////////////

private:
  
 /*       auto gesture1 = gtk_gesture_click_new();
        gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture1),1);
        g_signal_connect (G_OBJECT(gesture1) , "released", EVENT_CALLBACK (Workdir<DirectoryClass>::pathbar_go), (void *)pathbar_);
        gtk_widget_add_controller(GTK_WIDGET(pb_button), GTK_EVENT_CONTROLLER(gesture1));
 
*/
      /*
    static gboolean // on release... Coordinates are in icon's frame of reference.
    select_f(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer object){
      auto eventController = GTK_EVENT_CONTROLLER(self);
      auto columnView = gtk_event_controller_get_widget(eventController);
      auto listModel = G_LIST_MODEL(g_object_get_data(G_OBJECT(columnView), "store"));
      auto selection = GTK_SELECTION_MODEL(g_object_get_data(G_OBJECT(columnView), "selection"));
      auto items = g_list_model_get_n_items(listModel);
      for (int i=0; i<items; i++){
        if (gtk_selection_model_is_selected(selection, i)){
          auto info = G_FILE_INFO(g_list_model_get_item(listModel, i));
          DBG("selected: %s\n", g_file_info_get_name(info));
        }
      }
      return true;
    }

    static void addGestureClick(GtkWidget *columnView){
      auto gesture = gtk_gesture_click_new();
      gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),1); 
      // 1 for select
      g_signal_connect (G_OBJECT(gesture) , "released", EVENT_CALLBACK (select_f), (void *)columnView);
      gtk_widget_add_controller(GTK_WIDGET(columnView), GTK_EVENT_CONTROLLER(gesture));
      gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture), 
          GTK_PHASE_BUBBLE);

    }  
*/

      static gboolean equal_f (gconstpointer a, gconstpointer b){
        auto A = G_FILE_INFO(a);
        auto B = G_FILE_INFO(b);
        auto nameA = g_file_info_get_name(A);
        auto nameB = g_file_info_get_name(B);
        TRACE("compare \"%s\" with \"%s\"\n", nameA, nameB);
        if (strcmp(nameA, nameB) == 0) return true;
        return false;
      }

    void reload(const char *path){
        auto columnView = getColumnView(startFolder()); 
        gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(sw_), GTK_WIDGET(columnView));
        
        GFileInfo *infoF = g_file_info_new();
        auto name = g_path_get_basename(path);
        DBG("** now select %s\n", name);
        g_file_info_set_name(infoF, name);
        g_free(name);


        auto store = G_LIST_STORE(g_object_get_data(G_OBJECT(columnView), "store"));
        guint positionS;
        auto found = g_list_store_find_with_equal_func(store, infoF, equal_f, &positionS);
        if (found){
            gtk_single_selection_set_selected (selectionModel_, positionS);
        }
    }

    static void
    button_new (GtkButton * button, gpointer data) {

      auto subClassObject = (FileResponse *)data;
      char *path = g_strconcat(subClassObject->responsePathbar_p->path(), G_DIR_SEPARATOR_S,  _("Private"), NULL);

      DBG("***Entry dialog...path=%s dialog=%p\n", path, subClassObject->dialog());
     
      auto dialogObject = new DialogEntry<mkdirResponse<FileDialog<Type> > >;
      auto dialog = dialogObject->dialog();
      DialogStack::popMainDialog(dialog);

      auto parentDialog = subClassObject->dialog();
      auto dialogStack_p = (DialogStack *)g_object_get_data(G_OBJECT(parentDialog), "dialogStack_p");
      dialogStack_p->pushDialog(dialog);

      // This is to inform FileResponse  parent object 
      // in order to execute main context function from 
      // from FileResponse thread (fill in the entry buffer text).
       dialogObject->setParent(GTK_WINDOW(subClassObject->dialog()));

      auto entry = GTK_ENTRY( g_object_get_data(G_OBJECT(dialog),"entry"));
      g_object_set_data(G_OBJECT(entry), "path", g_strdup(path));

      // This is to inform mkdirResponse who parent object is
      // in order to execute main context function from FileResponse
      // from mkdirResponse thread. (update columnView and selection).
      dialogObject->subClass()->parentObject(subClassObject);
      DBG("*** button_new subClass=%p\n", subClassObject);
      dialogObject->subClass()->setDefaults(dialog, dialogObject->label());
      
      dialogObject->run();
      // get selected path
/*
      // path = g_strconcat(path, G_DIR_SEPARATOR_STRING, _("Private"), NULL);
      dialogPath<mkdirResponse>::action(path);
      g_object_set_data(G_OBJECT(subClass->dialog()), "response", GINT_TO_POINTER(-1));
      // FIXME: if folder exists, update the entry
*/
      //g_object_set_data(G_OBJECT(subClass->dialog()), "response", GINT_TO_POINTER(-1));
    }

    static void
    button_save (GtkButton * button, gpointer data) {
      auto subClassObject = (FileResponse *)data;
      g_object_set_data(G_OBJECT(subClassObject->dialog()), "response", GINT_TO_POINTER(1));
      
    }

    static void
    button_cancel (GtkButton * button, gpointer data) {
      auto subClassObject = (FileResponse *)data;
      g_object_set_data(G_OBJECT(subClassObject->dialog()), "response", GINT_TO_POINTER(-1));
    }

    GtkWindow *dialog(void){return dialog_;}
    
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
    
    static GListModel *getChildModel (gpointer listItem, void *data)
    {
      auto info = G_FILE_INFO(listItem);
      auto path = Basic::getPath(info);
      TRACE("getChildModel %s \n", path);
      auto listModel = getListModel(path);
      g_free(path);
      return listModel;
    }

    static GListModel *getBookmarkModel(const char *path){
        Bookmarks::initBookmarks();
        GError *error_ = NULL;
        auto store = g_list_store_new(G_TYPE_FILE_INFO);
        auto list = Bookmarks::bookmarksList();
        for (auto l=list; l && l->data; l=l->next){
          auto p = (bookmarkItem_t *)l->data;
          if (!p->path) continue;
          TRACE("adding bookmark %p -> %s\n", p, p->path);
          if (!g_path_is_absolute(p->path)) continue;
          if (!g_file_test(p->path, G_FILE_TEST_EXISTS)) {
              TRACE("Bookmark %s does not exist\n", p->path);
              continue;
          }
          GFile *file = g_file_new_for_path(p->path);
          auto info = g_file_query_info(file, "standard::", G_FILE_QUERY_INFO_NONE, NULL, &error_);
          auto basename = g_path_get_basename(p->path);
          auto utf_name = Basic::utf_string(basename);
          g_file_info_set_name(info, utf_name);
          g_free(basename);
          g_free(utf_name);
          g_file_info_set_icon(info, g_themed_icon_new(EMBLEM_BOOKMARK));
          g_list_store_insert_sorted(store, G_OBJECT(info), LocalDir::compareFunction, NULL);
          //g_list_store_insert(store, 0, G_OBJECT(info));
          //Important: if this is not set, then the GFile cannot be obtained from the GFileInfo:
          g_file_info_set_attribute_object(info, "standard::file", G_OBJECT(file));          
          g_file_info_set_attribute_object (info, "xffm::bookmark", G_OBJECT(file));
        }

        if (!g_slist_length(list)){
          g_object_unref(store);
          return NULL;
        }
        return G_LIST_MODEL(store);
    }

    static GListModel *getListModel(const char *path){
        GError *error_ = NULL;
        auto store = g_list_store_new(G_TYPE_FILE_INFO);
        GFile *file = g_file_new_for_path(path);
        GFileEnumerator *dirEnum =
          g_file_enumerate_children (file,"standard::",G_FILE_QUERY_INFO_NONE,NULL, &error_);
        if (error_) {
          TRACE("*** Error::g_file_enumerate_children: %s\n", error_->message);
          Print::printError(Child::getOutput(), g_strconcat(error_->message, "\n", NULL));
          g_error_free(error_);
          return NULL;
        }
        GFile *outChild = NULL;
        GFileInfo *outInfo = NULL;
        int items = 0;
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
            items++;
          }
          g_free(path);
          /*auto _path = g_file_get_path(outChild);
          setPaintableIcon(outInfo, _path);
          g_free(_path);*/
        } while (true);
        g_object_unref(file);
        if (!items){
          g_object_unref(store);
          return NULL;
        }
        return G_LIST_MODEL(store);
    }

    static void
    factorySetup1(GtkSignalListItemFactory *self, GObject *object, void *data){
        auto box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        TRACE("factorySetup1...\n");        
        auto expander = gtk_tree_expander_new();
        gtk_tree_expander_set_child(GTK_TREE_EXPANDER(expander), box);
        gtk_list_item_set_child(GTK_LIST_ITEM(object), expander); 
        g_object_set_data(G_OBJECT(object), "box", box);

        if (data) { // name column
          auto imageBox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
          gtk_box_append(GTK_BOX(box), imageBox);
          g_object_set_data(G_OBJECT(object), "imageBox", imageBox);
        }
        auto label = gtk_label_new("");
        gtk_box_append(GTK_BOX(box), label);
        g_object_set_data(G_OBJECT(object), "label", label);
        TRACE("factorySetup2...\n");        
    }

    static void
    factoryBind1(GtkSignalListItemFactory *factory, GObject *object, void *data){
        TRACE("factoryBind1...\n");

        auto list_item =GTK_LIST_ITEM(object);
        auto treeListRow = GTK_TREE_LIST_ROW(gtk_list_item_get_item(list_item));
        auto info = G_FILE_INFO(gtk_tree_list_row_get_item(treeListRow));
        if (data){
          auto expander = gtk_list_item_get_child( GTK_LIST_ITEM(object) );
          gtk_tree_expander_set_list_row(GTK_TREE_EXPANDER(expander), treeListRow);
        }
        //auto info = G_FILE_INFO(object);
        TRACE("info name = %s\n", g_file_info_get_name(info));

        auto label = GTK_LABEL(g_object_get_data(object, "label"));
        char *markup = NULL;
        if (data) { // name column
          auto imageBox = GTK_BOX(g_object_get_data(object, "imageBox"));
          const char *name = g_file_info_get_name(info);          
          auto maxLen = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(factory), "maxLen"));
          auto format = g_strdup_printf("<tt>%%-%ds", maxLen);
          char buffer[128];
          snprintf(buffer, 128, (const char *)format, name);
          markup = g_strdup_printf("%s</tt>", buffer);                    
          gtk_label_set_markup(label, markup);
          
          auto oldImage = gtk_widget_get_first_child(GTK_WIDGET(imageBox));
          if (oldImage) gtk_widget_unparent(oldImage);
          auto paintable = Texture<bool>::load(info);
          auto image = gtk_image_new_from_paintable(paintable);
          gtk_widget_set_size_request(image, 16, 16);
          gtk_box_append(GTK_BOX(imageBox), image);
        } else { // info column
          auto path = Basic::getPath(info);
          struct stat st;
          lstat(path, &st);
          g_free(path);
          auto m1 = Basic::statInfo(&st);
          markup = g_strdup("<tt> <span color=\"blue\" size=\"small\">");
          Basic::concat(&markup, m1);
          Basic::concat(&markup, "</span></tt>");           
          g_free(m1);                   
          gtk_label_set_markup(label, markup);
        }
        g_free(markup);
        
    }
       
    static gboolean // on release... Coordinates are in icon's frame of reference.
    reload_f(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer object){
      auto p = (FileResponse *)object;
      auto eventController = GTK_EVENT_CONTROLLER(self);
      auto button = gtk_event_controller_get_widget(eventController);
      auto path = (const char *)g_object_get_data(G_OBJECT(button), "path");
      DBG("Reload treemodel with %s\n", path);

      p->responsePathbar_p->path(path); // new path
      auto pathbar = p->responsePathbar_p->pathbar(); 

      auto reload_f = p->responsePathbar_p->reloadFunction();
      auto reload_data = p->responsePathbar_p->reloadData();
      BasicPathbar::updatePathbar(path, pathbar, true, reload_f, reload_data);
      //BasicPathbar::togglePathbar(path, pathbar); 
      // set red
      BasicPathbar::setRed(pathbar, path);
      auto columnView = p->getColumnView(path);
      auto sw = GTK_SCROLLED_WINDOW(p->sw());
      if (columnView) gtk_scrolled_window_set_child(sw, GTK_WIDGET(columnView));
      else {
        auto label = gtk_label_new("empty");
        gtk_scrolled_window_set_child(sw, label);
      }
      return true;
    }

    char *getSelectedPath(GtkSingleSelection *sel){
      GtkTreeListRow *treeListRow = GTK_TREE_LIST_ROW(gtk_single_selection_get_selected_item (sel));
      if (!treeListRow) {
        gtk_label_set_markup(GTK_LABEL(selectLabel_), _("No folder selected."));
        return NULL;
      }
      auto info = G_FILE_INFO(gtk_tree_list_row_get_item(treeListRow));
      DBG("selected: %s\n", g_file_info_get_name(info));
      auto path = Basic::getPath(info);
      auto name = g_file_info_get_name(info);
      gtk_label_set_markup(GTK_LABEL(selectLabel_), name);
      return path;
    }

    static void
    selection_cb (GtkSingleSelection *sel,
                  GParamSpec         *pspec,
                  void *data)
    {
      auto fileResponse_p =(FileResponse *)data;
      auto path = fileResponse_p->getSelectedPath(sel);
      if (!path) return;
      auto redPath = fileResponse_p->responsePathbar_p->path();
      auto pathbar = fileResponse_p->responsePathbar_p->pathbar();

      //BasicPathbar::togglePathbar(path, pathbar);
      /*BasicPathbar::setRed(pathbar,path);*/
      
      auto reload_f = fileResponse_p->responsePathbar_p->reloadFunction();
      auto reload_data = fileResponse_p->responsePathbar_p->reloadData();
      BasicPathbar::updatePathbar(path, pathbar, false, reload_f, reload_data);
      BasicPathbar::setRed(pathbar,redPath);
      g_free(path); 
      
    }



    GtkWidget *getColumnView(const char *path){
        GListModel * listModel;
        if (strcmp(path, _("Bookmarks")) == 0){
          listModel = getBookmarkModel(path);
        } else {
          listModel = getListModel(path);
        }
        if (!listModel) return NULL;

        GtkTreeListModel * treemodel = gtk_tree_list_model_new (G_LIST_MODEL (listModel),
                                             FALSE, // passthrough
                                             FALSE, // autoexpand TRUE,
                                             getChildModel,
                                             NULL,
                                             NULL);
             
        auto filterModel = gtk_filter_list_model_new (G_LIST_MODEL (treemodel), NULL);
        selectionModel_ = gtk_single_selection_new (G_LIST_MODEL (filterModel));
        g_signal_connect (selectionModel_, "notify::selected-item", G_CALLBACK (selection_cb), (void *)this);
        gtk_single_selection_set_autoselect(selectionModel_, false);
        gtk_single_selection_set_can_unselect(selectionModel_, true);
        gtk_single_selection_set_selected(selectionModel_, GTK_INVALID_LIST_POSITION);
        auto maxLen = Basic::getMaxNameLen(listModel);
        auto columnView = gtk_column_view_new(GTK_SELECTION_MODEL(selectionModel_));
        // no good addGestureClick(columnView);
        
        g_object_set_data(G_OBJECT(columnView), "selection", selectionModel_);
        g_object_set_data(G_OBJECT(columnView), "store", listModel);
        gtk_column_view_set_show_column_separators (GTK_COLUMN_VIEW (columnView), false);
        GtkColumnViewColumn *column;

        GtkListItemFactory *factory1 = gtk_signal_list_item_factory_new();
        column = gtk_column_view_column_new (_("Name"), factory1);
        gtk_column_view_append_column (GTK_COLUMN_VIEW (columnView), column);
        g_signal_connect (factory1, "setup", G_CALLBACK (factorySetup1), GINT_TO_POINTER(1));
        g_signal_connect (factory1, "bind", G_CALLBACK (factoryBind1), GINT_TO_POINTER(1));
        g_object_unref (column);


        GtkListItemFactory *factory2 = gtk_signal_list_item_factory_new();
        column = gtk_column_view_column_new (_("Information"), factory2);
        gtk_column_view_append_column (GTK_COLUMN_VIEW (columnView), column);
        g_signal_connect (factory2, "setup", G_CALLBACK (factorySetup1), NULL);
        g_signal_connect (factory2, "bind", G_CALLBACK (factoryBind1), NULL);
        g_object_unref (column);
        
        return columnView;

    }

    GtkBox *constructMainBox(void) {
        // set red path (root of treemodel)       
        responsePathbar_p->path(startFolder()); 
        //auto dialog = gtk_dialog_new ();
        //gtk_window_set_type_hint(GTK_WINDOW(dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
        mainBox_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        //gtk_widget_set_size_request(GTK_WIDGET(mainBox_), 550, 400);
        gtk_widget_set_vexpand(GTK_WIDGET(mainBox_), false);
        gtk_widget_set_hexpand(GTK_WIDGET(mainBox_), false);

        auto boxL = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_box_append(mainBox_, GTK_WIDGET(boxL));

        auto prefix = gtk_label_new("");
        auto markup = g_strconcat("<span color=\"blue\">", _("Selection:"), "</span> ", NULL);
        gtk_label_set_markup(GTK_LABEL(prefix), markup);
        g_free(markup);

        selectLabel_ = gtk_label_new(_("No folder selected."));
        gtk_box_append(boxL, prefix);
        gtk_box_append(boxL, selectLabel_);
        
        auto pathbarBox = responsePathbar_p->pathbar();
       // this->updatePathbarBox(path, pathbarBox, NULL);
       //
        auto path = responsePathbar_p->path();
        auto pathbar = responsePathbar_p->pathbar();
        auto reload_f = responsePathbar_p->reloadFunction();
        auto reload_data = responsePathbar_p->reloadData();
        BasicPathbar::updatePathbar(path, pathbar, true, reload_f, reload_data);
        //responsePathbar_p->updatePathbarBox(responsePathbar_p->path(), false, NULL); 
        
        gtk_box_append(mainBox_, GTK_WIDGET(responsePathbar_p->pathbar()));

        sw_ = gtk_scrolled_window_new();
        gtk_widget_set_vexpand(GTK_WIDGET(sw_), true);
        gtk_widget_set_hexpand(GTK_WIDGET(sw_), true);
        gtk_box_append(mainBox_, sw_);
        // listview...
        // gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(sw), GTK_WIDGET(output_));
        gtk_widget_set_size_request(GTK_WIDGET(sw_), 680, 200);

        auto columnView = getColumnView(startFolder()); 
        
        if (columnView){
          gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(sw_), GTK_WIDGET(columnView));
        } else {
          auto label = gtk_label_new("empty");
          gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(sw_), label);
        }

        auto action_area = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_vexpand(GTK_WIDGET(action_area), false);
        gtk_widget_set_hexpand(GTK_WIDGET(action_area), false);
        gtk_box_append(mainBox_, GTK_WIDGET(action_area));

        auto cancelButton = Basic::mkButton("emblem-redball", _("Cancel"));
        gtk_box_append(action_area,  GTK_WIDGET(cancelButton));
        gtk_widget_set_vexpand(GTK_WIDGET(cancelButton), false);

        auto newButton = Basic::mkButton ("emblem-edit", _("New Folder"));
        gtk_box_append(action_area,  GTK_WIDGET(newButton));
        gtk_widget_set_vexpand(GTK_WIDGET(newButton), false);

        auto saveButton = Basic::mkButton ("emblem-floppy", _("Accept"));
        gtk_box_append(action_area,  GTK_WIDGET(saveButton));
        gtk_widget_set_vexpand(GTK_WIDGET(saveButton), false);

        g_signal_connect (G_OBJECT (saveButton), "clicked", G_CALLBACK (button_save), this);
        g_signal_connect (G_OBJECT (cancelButton), "clicked", G_CALLBACK (button_cancel), this);
        g_signal_connect (G_OBJECT (newButton), "clicked", G_CALLBACK (button_new), this);

        return mainBox_;
    }

    gboolean save(void){
      return true;        
    }

  };



}
#endif

