#ifndef GRIDVIEW_HH
#define GRIDVIEW_HH
static GdkDragAction dndStatus = GDK_ACTION_COPY;


namespace xf {
template <class Type> class Dnd;
template <class Type> class RodentMonitor;
template <class Type> class FstabMonitor;
template <class Type>
  class GridView  {
      using clipboard_t = ClipBoard<LocalDir>;
      GtkWidget *child_=NULL;
      GtkMultiSelection *selectionModel_ = NULL;
      GtkWidget *view_=NULL;
      void *gridViewClick_f_=NULL;
      char *path_=NULL;

      char *regexp_=NULL;
      GRegex *regex_=NULL;
      filterData_t filterData_;

      // myMenu is for processing keys for individual widget popovers
      Menu<GridviewMenu<Type> > *myMenu_=NULL;
      int maxNameLen_ = 0;
      double x_ = 0.0; // factory widget
      double y_ = 0.0;
      double X_ = 0.0; // main widget
      double Y_ = 0.0;
      int flags_=0;
      RodentMonitor<Type> *rodentMonitor_ = NULL;
      FstabMonitor<Type> *fstabMonitor_ = NULL;
      GFileMonitor *monitor_ = NULL;
  public:
      FstabMonitor<Type> *fstabMonitor(void){return fstabMonitor_;}
      RodentMonitor<Type> *rodentMonitor(void){return rodentMonitor_;}

      void regexp(const char *value){
        g_free(regexp_);
        regexp_ = value? g_strdup(value): NULL;

      }
      GRegex *regex(void){return regex_;}
      void regex(GRegex *value){
        if (regex_) g_regex_unref(regex_);
        regex_ = value;
      }


      const char *regexp(void){return regexp_;}
      
      GFileMonitor *monitor(void){return monitor_;}
      void monitor(GFileMonitor *monitor){monitor_ = monitor;}
      double X(void){return X_;}
      double Y(void){return Y_;}
      double x(void){return x_;}
      double y(void){return y_;}
      void X(double value){X_ = value;}
      void Y(double value){Y_ = value;}
      void x(double value){x_ = value;}
      void y(double value){y_ = value;}
     
      int maxNameLen(void){return maxNameLen_;}
      
      GListModel *listModel(void){ return G_LIST_MODEL(selectionModel_);}
      GListStore *listStore(void){ 
        auto store = g_object_get_data(G_OBJECT(selectionModel_), "store");
        return G_LIST_STORE(store);
      }
      GListStore *store(void){return listStore();}

      void flagOn (int flag){ 
        flags_ |= flag;
      }
      void flagOff (int flag){  
        flags_ &= (0xffff ^ flag);
      }
      int flags(void){return flags_;}


      
  public:
      //bool dndOn = false;
      GridView(const char *path, void *gridViewClick_f){
        Child::addGridView((void *)this);

        gridViewClick_f_ = gridViewClick_f;
        path_ = g_strdup(path);
        //gtk_grid_view_set_single_click_activate (GTK_GRID_VIEW(view_), true);
        flags_ = Settings::getInteger(path_, "flags", 0);
        regexp_ = Settings::getString(path_, "regexp", "");
        GError *_error=NULL;
        if (regexp_ && strlen(regexp_)){
          auto cflags = (GRegexCompileFlags)((guint)G_REGEX_CASELESS | (guint)G_REGEX_OPTIMIZE);
          regex_ = g_regex_new (regexp_, cflags,(GRegexMatchFlags) 0, &_error);
          if (!regex_) {
            gchar *markup = g_strdup_printf("%s: %s (%s)\n", regexp_,
                _("Regular Expression syntax is incorrect"), _error->message);
            DBG("%s", markup);
            Print::printInfo(Child::getOutput(), markup);
            // done by printInfo(): g_free(markup);    
            g_error_free(_error);
          }    
        }
        filterData_.flags = flags_;
        filterData_.count = 0;
        filterData_.total = 0;
        filterData_.regex = regex_;
        filterData_.regexp = regexp_;
        
        TRACE("gridview flags = 0x%x\n", flags_);
        view_ = getGridView();
        
        myMenu_ = new Menu<GridviewMenu<Type> >("foo");
        addGestureClickView1(view_, NULL, this);// unselect all on release
        //addGestureDown(view_, NULL, this);// 
        addGestureClickView3(view_, NULL, this); // menu 

        addMotionController();
        if (g_object_get_data(G_OBJECT(store()), "xffm::root")){
          fstabMonitor_ = NULL;
        } else {
          fstabMonitor_ = new FstabMonitor<Type>(this); 
        } 

        //auto dropController = Dnd<Type>::createDropController(this);
        //gtk_widget_add_controller (GTK_WIDGET (view_), GTK_EVENT_CONTROLLER (dropController));
      }

      ~GridView(void){
        Child::removeGridView((void *)this);
      TRACE("GridView<Type> destructor\n");
        //if (menu_){
          // menu_ goes down with gridview. No need to
          //       unparent or
          //gtk_widget_unparent(GTK_WIDGET(menu_));
          //g_object_unref(G_OBJECT(menu_));
        //}
        if (myMenu_) delete myMenu_; // main menu
        if (fstabMonitor_) delete fstabMonitor_; 

        g_free(path_);
        g_free(regexp_);
        if (regex_) g_regex_unref(regex_);
        TRACE("~GridView complete.\n");
      }     

      void child(GtkWidget *child){
        child_ = child;
        auto store = G_OBJECT(g_object_get_data(G_OBJECT(selectionModel_), "store"));
        g_object_set_data(store, "child", child_);
      }

      GtkWidget *child(void){return child_;}
      //void setMenu(GtkPopover *menu){ menu_ = menu;}
      GtkMultiSelection *multiSelectionModel(void){ return selectionModel_;}
      GtkSelectionModel *selectionModel(void){ return GTK_SELECTION_MODEL(selectionModel_);}
      //GtkPopover *menu(void){ return menu_;}
      GtkWidget *view(void){ return view_;}
      void *gridViewClick_f(void){ return gridViewClick_f_;}
      char *path(void){ return path_;}

      
  private:
      static void
      updateButtons (GtkSelectionModel* self, guint position, guint n_items,  gpointer user_data){
          auto bitset = gtk_selection_model_get_selection(self);
          gtk_widget_set_sensitive(GTK_WIDGET(cutButton), (gtk_bitset_get_size(bitset) > 0));
          gtk_widget_set_sensitive(GTK_WIDGET(copyButton), (gtk_bitset_get_size(bitset) > 0));
      }

  public:

      GtkWidget *
      getGridView(){
        auto child = Child::getChild();
        selectionModel_ = NULL;
        bool isBookmarks = (strcmp(path_, "Bookmarks")==0);
        bool isFstab = (strcmp(path_, "Disk Mounter")==0);
        if (isBookmarks) {
          selectionModel_ = rootDir::rootSelectionModel();
        } else if (isFstab) {
          selectionModel_ = FstabDir::fstabSelectionModel();
        } else {
          // Create the initial GtkDirectoryList (G_LIST_MODEL).
          selectionModel_ = LocalDir::xfSelectionModel(path_, &filterData_);
          g_object_set_data(G_OBJECT(child), "selection", selectionModel_);
          auto store = G_LIST_MODEL(g_object_get_data(G_OBJECT(selectionModel_), "store"));
          maxNameLen_ = Basic::getMaxNameLen(store);
          g_signal_connect(G_OBJECT(selectionModel_), "selection-changed", G_CALLBACK(updateButtons), NULL);
//          maxNameLen_ = LocalDir::getMaxNameLen(path_);
        }
       
        // GtkListItemFactory implements GtkSignalListItemFactory, which can be connected to
        // bind, setup, teardown and unbind
        GtkListItemFactory *factory = gtk_signal_list_item_factory_new();
        g_object_set_data(G_OBJECT(factory), "child", child);

        /* Connect handler to the factory.
         */
        g_signal_connect( factory, "setup", G_CALLBACK(Factory<Type>::factorySetup), this );
        g_signal_connect( factory, "bind", G_CALLBACK(Factory<Type>::factoryBind), this);
        g_signal_connect( factory, "unbind", G_CALLBACK(Factory<Type>::factoryUnbind), this);
        g_signal_connect( factory, "teardown", G_CALLBACK(Factory<Type>::factoryTeardown), this);

        TRACE("size = %d\n",Settings::getInteger("xfterm", "iconsize",24));

        TRACE("got maxNameLen_ = %d\n", maxNameLen_);

        GtkWidget *view;
        /* Create the view.
         */
        view = gtk_grid_view_new(GTK_SELECTION_MODEL(selectionModel_), factory);
        g_object_set_data(G_OBJECT(child), "gridview", view);
        gtk_grid_view_set_max_columns(GTK_GRID_VIEW(view), 20);
        //gtk_grid_view_set_min_columns(GTK_GRID_VIEW(view), 10);
        gtk_widget_add_css_class(view, "gridviewColors");
        //gtk_grid_view_set_enable_rubberband(GTK_GRID_VIEW(view), TRUE);
        gtk_grid_view_set_enable_rubberband(GTK_GRID_VIEW(view), false);
#if 0
        //FIXME: not working...
        if (!isBookmarks && !isFstab) {
          rodentMonitor_ = new RodentMonitor<Type>(this); 
          
        }
        
#else
 ////////////////
        if (!isBookmarks && !isFstab) {
          // We wait until here to fireup the monitor.
          auto store = G_LIST_MODEL(g_object_get_data(G_OBJECT(selectionModel_), "store"));
          auto file = G_FILE(g_object_get_data(G_OBJECT(store), "file"));
          GError *error_ = NULL;
          monitor_ = g_file_monitor_directory (file, G_FILE_MONITOR_WATCH_MOVES, NULL,&error_);
          g_object_set_data(G_OBJECT(monitor_), "file", file);
          Child::addMonitor(monitor_);

          TRACE("monitor_=%p file=%p store=%p\n", monitor_, file, store);
          if (error_){
              ERROR_("g_file_monitor_directory(%s) failed: %s\n",
                      "fixme", error_->message);
              g_error_free(error_);
          } else {
            g_signal_connect (monitor_, "changed", 
                  G_CALLBACK (changed_f), (void *)this);
          }
          g_object_set_data(G_OBJECT(store), "monitor", monitor_);
        }
 ////////////////
#endif
        return view;
      }

  private:

      static void
      changed_f ( GFileMonitor* self,  
          // This runs in main context, I presume.
          GFile* first, GFile* second, //same as GioFile * ?
          GFileMonitorEvent event, 
          void *data){

        // Switch to pause monitor execution.
        pthread_mutex_lock(&monitorMutex);   
        auto inactive = g_object_get_data(G_OBJECT(self), "inactive");
        pthread_mutex_unlock(&monitorMutex);   
        if (inactive) {
          TRACE("monitor %p inactive\n", self);
          return;
        }

        //
        auto gridView_p = (GridView<LocalDir> * )data;
        if (!Child::validGridView(gridView_p)) return;
        auto model = gridView_p->listModel();  // Model to find
        auto store = gridView_p->store();      // Store to remove/add
        //GListStore *store = G_LIST_STORE(data);


        auto child = gridView_p->child();
//        auto child = (GtkWidget *)g_object_get_data(G_OBJECT(store), "child");
        if (!child){
          Basic::Exit("localdir.hh::changed_f(): this should not happen\n");
        }
        TRACE("*** monitor changed_f call \n");
        gchar *f= first? g_file_get_path (first):g_strdup("--");
        gchar *s= second? g_file_get_path (second):g_strdup("--");

      /*  GError *error_=NULL;
        GFileInfo *infoF = first? g_file_query_info (first, "standard::,G_FILE_ATTRIBUTE_TIME_MODIFIED,G_FILE_ATTRIBUTE_TIME_CREATED", 
            G_FILE_QUERY_INFO_NONE, NULL, &error_):NULL;
        if (error_){
          ERROR_("Error: %s\n", error_->message);
          g_error_free(error_);
          return;
        }*/

        /* 
        if (p->view()->serial() != p->serial()){
            ERROR_("LocalMonitor::changeItem() serial out of sync (%d != %d)\n",p->view()->serial(), p->serial());
            return;
        }*/

        bool verbose = false;
        guint positionF;
        auto dirFile = G_FILE(g_object_get_data(G_OBJECT(self), "file"));
        auto dirPath = g_file_get_path(dirFile);
        int flags = Settings::getInteger(dirPath, "flags", 0); 
        g_free(dirPath);
        
        if (verbose) DBG("monitor thread %p...\n", g_thread_self());
         switch (event){
            case G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED:
              {
                if (verbose) 
                {DBG("Received  ATTRIBUTE_CHANGED (%d): \"%s\", \"%s\"\n", event, f, s);}
                auto found = LocalDir::findPositionModel2(model, f, &positionF);
//                auto found = LocalDir::findPositionStore(store, f, &positionF, flags);
                if (found) {
                   Child::incrementSerial(child);
                   // Position in store not necesarily == to model (filter)
                   LocalDir::findPositionStore(store, f, &positionF);
                   g_list_store_remove(store, positionF);
                   if (verbose)DBG("removing %s\n",f);
                   Child::incrementSerial(child);
                   LocalDir::insert(store, f, verbose);                        
                   if (verbose)DBG("inserting %s\n",f);
                } else {
                  if (verbose)DBG("%s not found!\n", f);
                }

                //p->restat_item(f);
              } 
                break;
            case G_FILE_MONITOR_EVENT_PRE_UNMOUNT:
                if (verbose) {DBG("Received  PRE_UNMOUNT (%d): \"%s\", \"%s\"\n", event, f, s);}
                break;
            case G_FILE_MONITOR_EVENT_UNMOUNTED:
                if (verbose) {DBG("Received  UNMOUNTED (%d): \"%s\", \"%s\"\n", event, f, s);}
                break;

            case G_FILE_MONITOR_EVENT_DELETED:
            case G_FILE_MONITOR_EVENT_MOVED_OUT:
                {
                  if (verbose) 
                  {DBG("Received DELETED  (%d): \"%s\", \"%s\"\n", event, f, s);}  
                  auto found = LocalDir::findPositionModel2(model, f, &positionF);
                  if (found) {
                    guint positionX;
                    Child::incrementSerial(child);
                    LocalDir::findPositionStore(store, f, &positionX);
                    g_list_store_remove(store, positionX);
                    //g_list_model_items_changed (model, positionF, 1, 0);

                  } else {
                  if (verbose)DBG("%s not found!\n", f);
                }

                }
                break;

            case G_FILE_MONITOR_EVENT_CREATED:
            case G_FILE_MONITOR_EVENT_MOVED_IN:
                {
                  if (verbose) 
                  {DBG("Received  CREATED (%d): \"%s\", \"%s\"\n", event, f, s);}
                  auto found = LocalDir::findPositionModel2(model, f, &positionF);
                  if (found) {
                     Child::incrementSerial(child);
                     LocalDir::findPositionStore(store, f, &positionF);
                     g_list_store_remove(store, positionF);
                     Child::incrementSerial(child);
                     LocalDir::insert(store, f, verbose);                        
                  } else {
                     Child::incrementSerial(child);
                     LocalDir::insert(store, f, verbose);    
                  }
                }
                break;
            case G_FILE_MONITOR_EVENT_CHANGED:
            {
              
         // When doing cut/copy, problem is that callback is
         // happening before change signal.
          if (verbose) 
                {DBG("monitor_f(): Received  CHANGED (%d): \"%s\", \"%s\"\n", event, f, s);}
          // This works, but scrollbar is not updated to last position
          // And then it is broken because icon is not updated when a different
          // icon is set to cut or copy
                // Only for updating icons after cut/copy
                //MainWindow<Type>::update(g_path_get_dirname(f));
                //MainWindow<Type>::update(g_path_get_dirname(f));
  
            } break;
            case G_FILE_MONITOR_EVENT_MOVED:
            case G_FILE_MONITOR_EVENT_RENAMED:
            {
                if (verbose) 
                {DBG("Received  MOVED (%d): \"%s\", \"%s\"\n", event, f, s);}
                // Delete old item
                auto found1 = LocalDir::findPositionModel2(model, f, &positionF);
                if (found1){
                    guint positionX;
                    Child::incrementSerial(child);
                    LocalDir::findPositionStore(store, f, &positionX);
                    g_list_store_remove(store, positionX);
                }
                // if new item exists, remove it from the list first 
                auto found2 = LocalDir::findPositionModel2(model, s, &positionF);
                if (found2){
                    guint positionX;
                    Child::incrementSerial(child);
                    LocalDir::findPositionStore(store, s, &positionX);
                    g_list_store_remove(store, positionX);
                }
                // add updated info to the list.
                Child::incrementSerial(child);
                LocalDir::insert(store, s, verbose);
            
            }
                break;
            case G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT:
                if (verbose) {DBG("Received  CHANGES_DONE_HINT (%d): \"%s\", \"%s\"\n", event, f, s);}

                //p->reSelect(f); // Will only select if in selection list (from move).
                break;       
        }
        g_free(f);
        g_free(s);
       

      }


static void setPopoverItems(GtkPopover *popover, GridView<Type> *gridView_p){
  // multiple select
      auto keys = gridView_p->myMenu_->keys();
      for (auto p=keys; p && *p; p++){
        auto widget = g_object_get_data(G_OBJECT(popover), *p);
        if (widget){
          //TRACE("hide widget \"%s\"\n", *p);
          gtk_widget_set_visible(GTK_WIDGET(widget), false);
        } else {
          ERROR_("* Warning: cannot find widget \"%s\" to hide.\n", *p);
        }
      }
      //show
      const char *items[]={_("Open with"), _("Delete"),_("Copy"),_("Cut"),_("Create a compressed archive with the selected objects"),NULL};
      for (auto p=items; p && *p; p++){
        auto widget = g_object_get_data(G_OBJECT(popover), *p);
        if (widget){
          gtk_widget_set_visible(GTK_WIDGET(widget), true);
        } else {
          ERROR_("* Warning: cannot find widget \"%s\" to show.\n", *p);
        }
      }

    }
    

    static void setPopoverItemsFstab(GtkPopover *popover, const char *path, GridView<Type> *gridView_p ){
      auto keys = gridView_p->myMenu_->keys();
      for (auto p=keys; p && *p; p++){
      auto widget = g_object_get_data(G_OBJECT(popover), *p);
        if (widget){
          //TRACE("hide widget \"%s\"\n", *p);
          gtk_widget_set_visible(GTK_WIDGET(widget), false);
        } else {
          ERROR_("* Warning: cannot find widget \"%s\" to hide.\n", *p);
        }
      }
      void *widget;
      widget = g_object_get_data(G_OBJECT(popover), _("Properties"));
      gtk_widget_set_visible(GTK_WIDGET(widget), true);
      if (FstabDir::isMounted(path)){
        widget = g_object_get_data(G_OBJECT(popover), _("Unmount Volume"));
        gtk_widget_set_visible(GTK_WIDGET(widget), true);
      } else {
        widget = g_object_get_data(G_OBJECT(popover), _("Mount Volume"));
        gtk_widget_set_visible(GTK_WIDGET(widget), true);
      }
    }

    static void setPopoverItemsEfs(GtkPopover *popover, const char *path, GridView<Type> *gridView_p ){
      auto keys = gridView_p->myMenu_->keys();
      for (auto p=keys; p && *p; p++){
      auto widget = g_object_get_data(G_OBJECT(popover), *p);
        if (widget){
          //TRACE("hide widget \"%s\"\n", *p);
          gtk_widget_set_visible(GTK_WIDGET(widget), false);
        } else {
          ERROR_("* Warning: cannot find widget \"%s\" to hide.\n", *p);
        }
      }
      void *widget;
      if (FstabDir::isMounted(path)){
        widget = g_object_get_data(G_OBJECT(popover), _("Unmount Volume"));
        gtk_widget_set_visible(GTK_WIDGET(widget), true);
      } else {
        widget = g_object_get_data(G_OBJECT(popover), _("Mount Volume"));
        gtk_widget_set_visible(GTK_WIDGET(widget), true);
      }
    }

    static void setPopoverItemsBookmark(GtkPopover *popover, const char *path, GridView<Type> *gridView_p ){
      auto keys = gridView_p->myMenu_->keys();
      for (auto p=keys; p && *p; p++){
      auto widget = g_object_get_data(G_OBJECT(popover), *p);
        if (widget){
          //TRACE("hide widget \"%s\"\n", *p);
          gtk_widget_set_visible(GTK_WIDGET(widget), false);
        } else {
          ERROR_("* Warning: cannot find widget \"%s\" to hide.\n", *p);
        }
      }
      void *widget;
      widget = g_object_get_data(G_OBJECT(popover), _("Properties"));
      gtk_widget_set_visible(GTK_WIDGET(widget), true);
      widget = g_object_get_data(G_OBJECT(popover), _("Remove bookmark"));
      gtk_widget_set_visible(GTK_WIDGET(widget), true);

    }

    static void setPopoverItems(GtkPopover *popover, const char *path, GridView<Type> *gridView_p ){
      // single selection
      auto keys = gridView_p->myMenu_->keys();
      for (auto p=keys; p && *p; p++){
      auto widget = g_object_get_data(G_OBJECT(popover), *p);
        if (widget){
          //TRACE("hide widget \"%s\"\n", *p);
          gtk_widget_set_visible(GTK_WIDGET(widget), false);
        } else {
          ERROR_("* Warning: cannot find widget \"%s\" to hide.\n", *p);
        }
      }
      // Directory test
      if (g_file_test(path, G_FILE_TEST_IS_DIR)){
        const char *show[]={
          _("Open in new tab"),
          _("Create a compressed archive with the selected objects"),
          _("Copy"),
          _("Cut"),
          _("Rename"),
          _("Duplicate"),
          _("Link"),
          _("Properties"),
          _("Delete"),
          NULL};
        for (auto p=show; p && *p; p++){
          //TRACE("show widget \"%s\"\n", *p);
          auto widget = g_object_get_data(G_OBJECT(popover), *p);
          if (widget){
            gtk_widget_set_visible(GTK_WIDGET(widget), true);
          }
        }
    
        // bookmark test
        if (!Bookmarks::isBookmarked(path)){ 
          auto widget = g_object_get_data(G_OBJECT(popover), _("Add bookmark"));
          gtk_widget_set_visible(GTK_WIDGET(widget), true);
        } else {
          auto widget = g_object_get_data(G_OBJECT(popover), _("Remove bookmark"));
          gtk_widget_set_visible(GTK_WIDGET(widget), true);
        }
        // pasteboard test
        {
          auto c = (clipboard_t *)g_object_get_data(G_OBJECT(Child::mainWidget()), "ClipBoard");
          auto widget = g_object_get_data(G_OBJECT(popover), _("Paste"));
          gtk_widget_set_visible(GTK_WIDGET(widget), c->validClipBoard());
        }

        // mount test 
        if (FstabUtil::isMounted(path)){
          auto widget = g_object_get_data(G_OBJECT(popover), _("Unmount Volume"));
          gtk_widget_set_visible(GTK_WIDGET(widget), true);
        } else if (FstabUtil::isInFstab(path)) {
          auto widget = g_object_get_data(G_OBJECT(popover), _("Mount Volume"));
          gtk_widget_set_visible(GTK_WIDGET(widget), true);
        }
      } else { // Regular
        const char *show[]={
          _("auto"), //
          _("Open with"), //
          _("Copy"),
          _("Cut"),
          _("Rename"),
          _("Duplicate"),
          _("Link"),
          _("Properties"),
          _("Delete"),
          NULL};
        for (auto p=show; p && *p; p++){
          auto widget = g_object_get_data(G_OBJECT(popover), *p);
          if (widget){
            gtk_widget_set_visible(GTK_WIDGET(widget), true);
          }
        }
        if (strrchr(path, '.') && strcmp(strrchr(path, '.'), ".gpg") == 0){
          auto widget = g_object_get_data(G_OBJECT(popover), _("Decrypt File..."));
          gtk_widget_set_visible(GTK_WIDGET(widget), true);
          widget = g_object_get_data(G_OBJECT(popover), _("Encrypt File..."));
          gtk_widget_set_visible(GTK_WIDGET(widget), false);
        } else {
          auto widget = g_object_get_data(G_OBJECT(popover), _("Encrypt File..."));
          gtk_widget_set_visible(GTK_WIDGET(widget), true);
          widget = g_object_get_data(G_OBJECT(popover), _("Decrypt File..."));
          gtk_widget_set_visible(GTK_WIDGET(widget), false);
        }
      }
    }

    static GtkPopover *getPopover(GtkWidget *menubox, GridView<Type> *gridView_p){ 
      TRACE("getPopover 1\n");
      auto markup = g_strdup_printf("<span color=\"blue\"><b>%s</b></span>", _("Multiple selections"));
      auto popover = gridView_p->myMenu_->mkMenu(markup);
      g_free(markup);
      g_object_set_data(G_OBJECT(popover), "gridView_p", gridView_p);
      TRACE("object set selection list\n");
      

      setPopoverItems(GTK_POPOVER(popover), gridView_p);
      gtk_widget_set_parent(GTK_WIDGET(popover), menubox);
      
      TRACE("object set selection popover: %p -> %p\n", selectionList, popover);
      //g_object_set_data(G_OBJECT(selectionList), "menu", popover);
      return popover;
    }

    static GtkPopover *getPopover(GFileInfo *info, GridView<Type> *gridView_p){ 
      TRACE("getPopover 12\n");
      auto menubox = GTK_WIDGET(g_object_get_data(G_OBJECT(info), "menuBox"));
      auto path = Basic::getPath(info);
      auto markup = g_strdup_printf("<span color=\"blue\"><b>%s</b></span>", path);
      auto popover = gridView_p->myMenu_->mkMenu(markup, (void *)GridviewMenu<Type>::gestureProperties, _("Properties"));
      g_object_set_data(G_OBJECT(popover), "gridView_p", gridView_p);
      g_object_set_data(G_OBJECT(popover), "info", info);

      if (g_file_info_get_attribute_object(info, "xffm::fstabMount")){
        setPopoverItemsFstab(GTK_POPOVER(popover), path, gridView_p);
      } else {
        setPopoverItems(GTK_POPOVER(popover), path, gridView_p);
      }
      g_free(markup);
      g_object_set_data(G_OBJECT(menubox), "menu", popover);
      gtk_widget_set_parent(GTK_WIDGET(popover), menubox);
      g_free(path);
      return popover;
    }

    static GtkPopover *getPopover(GObject *object, GridView<Type> *gridView_p){ 
      TRACE("getPopover 123\n");
      auto list_item =GTK_LIST_ITEM(object);
      auto info = G_FILE_INFO(gtk_list_item_get_item(list_item));
      auto menubox = GTK_WIDGET(g_object_get_data(G_OBJECT(object), "menuBox"));
      auto path = Basic::getPath(info);
      auto markup = g_strdup_printf("<span color=\"blue\"><b>%s</b></span>", path);
      auto popover = gridView_p->myMenu_->mkMenu(markup, (void *)GridviewMenu<Type>::gestureProperties, _("Properties"));
      g_object_set_data(G_OBJECT(popover), "gridView_p", gridView_p);
      g_object_set_data(G_OBJECT(popover), "info", info);

     if (g_file_info_get_attribute_object(info, "xffm::fstabMount")){
        setPopoverItemsFstab(GTK_POPOVER(popover), path, gridView_p);
      } else if (g_file_info_get_attribute_object(info, "xffm::bookmark")){
        setPopoverItemsBookmark(GTK_POPOVER(popover), path, gridView_p);
      } else if (g_file_info_get_attribute_object(info, "xffm::ecryptfs")){
        setPopoverItemsEfs(GTK_POPOVER(popover), path, gridView_p);
      } else {
        setPopoverItems(GTK_POPOVER(popover), path, gridView_p);
      }
      g_free(markup);
      g_object_set_data(G_OBJECT(menubox), "menu", popover);
      gtk_widget_set_parent(GTK_WIDGET(popover), menubox);
      g_free(path);
      return popover;
    }
/*    
*/    
  static void setupMenu(GtkPopover *popover, GFileInfo *info){
    TRACE("setupMenu\n");
      auto path = Basic::getPath(info);
      if (EfsResponse<Type>::isEfsMount(path)){
        TRACE("*** %s isEfs\n", path);
        // hide all

      }

      auto isDir = g_file_test(path, G_FILE_TEST_IS_DIR);
      if (g_file_info_get_attribute_object(info, "xffm::fstabMount")) isDir = false;
      if (g_file_info_get_attribute_object(info, "xffm::bookmark")) isDir = false;
      if (g_file_info_get_attribute_object(info, "xffm::ecryptfs")) isDir = false;


      auto abutton = g_object_get_data(G_OBJECT(popover), _("auto"));
          TRACE("data get %p %s --> %p\n", popover, _("auto"), abutton);
      if (abutton){
        const char *defaultApp = GridviewMenu<bool>::getDefaultApp(path);

        gtk_widget_set_visible(GTK_WIDGET(abutton), defaultApp != NULL);
        if (defaultApp && GTK_IS_BUTTON(abutton)) {
          TRACE("// set icon and text: defaultApp=%s.\n", defaultApp);        
          auto box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
          gtk_widget_add_css_class (GTK_WIDGET(box), "inquireBox" );
          gtk_widget_set_hexpand(GTK_WIDGET(box), false);
          gtk_widget_set_vexpand(GTK_WIDGET(box), false);
          auto n = g_strdup(defaultApp);
          if (strchr(n, ' ')) *strchr(n, ' ') = 0;
          auto paintable = Texture<bool>::load16(n);


          if (paintable){
            auto image = gtk_image_new_from_paintable(paintable);
            gtk_box_append (box, GTK_WIDGET(image));
          } else {
            auto image = gtk_image_new_from_icon_name("emblem-run");
            gtk_box_append (box, GTK_WIDGET(image));
          }
          auto label = gtk_label_new("");
          auto base = g_path_get_basename(path);
          auto markup = g_strconcat("", n, " <span color=\"blue\">", base,"</span>", NULL); 
          gtk_box_append (box, GTK_WIDGET(label));
          gtk_label_set_markup(GTK_LABEL(label), markup);
          g_free(n);
          g_free(markup);
          gtk_button_set_child(GTK_BUTTON(abutton), GTK_WIDGET(box));
        } else {
          gtk_widget_set_visible(GTK_WIDGET(abutton), false);
        }

        
      } else {ERROR_("** Error:: no auto button\n");}
      if (isDir){
        auto paste = g_object_get_data(G_OBJECT(popover), _("Paste"));
        auto nopaste = g_object_get_data(G_OBJECT(popover), _("Clipboard is empty."));
        auto c = (clipboard_t *)g_object_get_data(G_OBJECT(Child::mainWidget()), "ClipBoard");
        if (c->validClipBoard()){
          gtk_widget_set_visible(GTK_WIDGET(paste), true);
          gtk_widget_set_visible(GTK_WIDGET(nopaste), false);
        } else {
          gtk_widget_set_visible(GTK_WIDGET(paste), false);
          gtk_widget_set_visible(GTK_WIDGET(nopaste), true);
        }
        
      }
      //gtk_widget_remove_css_class (GTK_WIDGET(imageBox), "pathbarboxNegative" );
      g_free(path);
  }
  public:
  // for selected items
  static void placeMenu(GtkWidget *menubox, GridView<Type> *gridView_p){
      auto popover = g_object_get_data(G_OBJECT(menubox), "menu");
      if (!popover){
        TRACE("getPopover...\n");
        popover = getPopover(menubox, gridView_p);
        TRACE("getPopover OK.\n");
      }
      
      if (popover) {
        //g_object_set_data(G_OBJECT(popover), "selectionList", selectionList);
        //setupMenu(GTK_POPOVER(popover), selectionList);
        gtk_popover_popup(GTK_POPOVER(popover));
      }
  }

  public:
  static void placeMenu(GFileInfo *info, GridView<Type> *gridView_p){
      auto menubox = g_object_get_data(G_OBJECT(info), "menuBox");
      auto popover = g_object_get_data(G_OBJECT(menubox), "menu");
      if (!popover){
        popover = getPopover(info, gridView_p);
      }
      
      if (popover) {
        g_object_set_data(G_OBJECT(popover), "info", info);
        setupMenu(GTK_POPOVER(popover), info);
        gtk_popover_popup(GTK_POPOVER(popover));
      }
  }
  // for single non selected item
  static void placeMenu(GObject *object, GridView<Type> *gridView_p){
      auto list_item =GTK_LIST_ITEM(object);
      auto info = G_FILE_INFO(gtk_list_item_get_item(list_item));
      auto menubox = g_object_get_data(G_OBJECT(object), "menuBox");
      auto popover = g_object_get_data(G_OBJECT(menubox), "menu");
      if (!popover){
        popover = getPopover(object, gridView_p);
      }
      
      if (popover) {
        g_object_set_data(G_OBJECT(popover), "info", info);
        setupMenu(GTK_POPOVER(popover), info);
        gtk_popover_popup(GTK_POPOVER(popover));
      }
  }
  private:
   static gboolean
    openMainMenu(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              void *data){
      auto gridView_p = (GridView<Type> *)data;
      if (gridView_p->getSelectionSize() == 0)
      {
        MainWindow<Type>::clickMenu( mainMenuButton, NULL);
      } else {
        return Factory<Type>::menu_f(self, n_press, x , y, data);
      }
      return true;
    }
   static gboolean
    unselect_f(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              void *data){
      TRACE("unselect_f...\n");
      auto gridView_p = (GridView<Type> *)data;
      auto selectionModel = gridView_p->selectionModel();
      // if control or shift down, return false.
      auto eventController = GTK_EVENT_CONTROLLER(self);
      auto event = gtk_event_controller_get_current_event(eventController);
      auto modifierType = gdk_event_get_modifier_state (event);
      if (modifierType & ((GDK_CONTROL_MASK & GDK_MODIFIER_MASK))) return false;
      if (modifierType & ((GDK_SHIFT_MASK & GDK_MODIFIER_MASK))) return false;
      
      gtk_selection_model_unselect_all(GTK_SELECTION_MODEL(selectionModel));
      gtk_widget_grab_focus(GTK_WIDGET(Child::getInput()));
      return true;
    }

    static gboolean equalItem (gconstpointer a, gconstpointer b){
      TRACE("equalItem %p -- %p\n", a, b);

      if (a == b) return true;
      return false;
    }
   public:
    guint getSelectionSize(void){
      GtkBitset *bitset = gtk_selection_model_get_selection (GTK_SELECTION_MODEL(selectionModel_));
      return gtk_bitset_get_size(bitset);
    }

    GList *getSelectionList(void){
      guint position;
      GtkBitsetIter iter;
      GList *selectionList = NULL;
      GtkBitset *bitset = gtk_selection_model_get_selection (GTK_SELECTION_MODEL(selectionModel_));
      if (gtk_bitset_iter_init_first (&iter, bitset, &position)){
        auto list = G_LIST_MODEL(selectionModel_);        
        do {
          auto info = G_FILE_INFO(g_list_model_get_item (list, position));
          g_object_ref(G_OBJECT(info));
          selectionList = g_list_append(selectionList, info);
        } while (gtk_bitset_iter_next (&iter,&position));
      }
      return selectionList;
    }
    
  private:

    static void addGestureClickView1(GtkWidget *self, GObject *object, GridView<Type> *gridView_p){
      auto gesture = gtk_gesture_click_new();
      g_object_set_data(G_OBJECT(self), "ClickView1", gesture);
      gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),1); 
      // 1 for unselect
      g_signal_connect (G_OBJECT(gesture) , "released", EVENT_CALLBACK (unselect_f), (void *)gridView_p);
      gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(gesture));
      gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture), 
          GTK_PHASE_CAPTURE);

    }    

    static void addGestureClickView3(GtkWidget *self, GObject *object, GridView<Type> *gridView_p){
      auto gesture = gtk_gesture_click_new();
      g_object_set_data(G_OBJECT(self), "ClickView3", gesture);
      gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),3); 
      // 3 for menu
      g_signal_connect (G_OBJECT(gesture) , "released", EVENT_CALLBACK (openMainMenu), (void *)gridView_p);
      gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(gesture));
      gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture), 
          GTK_PHASE_BUBBLE);

    }    
public:

    private:
    static
    gboolean
    viewMotion ( GtkEventControllerMotion* self,
                    gdouble x,
                    gdouble y,
                    gpointer data) 
    {
      auto d = (Dnd<LocalDir> *)g_object_get_data(G_OBJECT(Child::mainWidget()), "Dnd");
      auto dragOn = d->startDrag(self, x, y, data);
      if (dragOn) {
        TRACE("Gridview::viewMotion() dragOn() == true, return true\n");
        return true;
      } else {
        TRACE("Gridview::viewMotion() dragOn() == false, return false\n");
      }
      return false;
    }

      void addMotionController(void){
        auto controller = gtk_event_controller_motion_new();
        g_object_set_data(G_OBJECT(view_), "MotionController", controller);
        gtk_event_controller_set_propagation_phase(controller, GTK_PHASE_CAPTURE);
        gtk_widget_add_controller(GTK_WIDGET(view_), controller);
        g_signal_connect (G_OBJECT (controller), "motion", 
            G_CALLBACK (viewMotion), this);
    }


    
  };
}
#endif
