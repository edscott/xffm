#ifndef GRIDVIEW_HH
#define GRIDVIEW_HH
static GdkDragAction dndStatus = GDK_ACTION_COPY;
namespace xf {
template <class Type> class Dnd;
template <class DirectoryClass>
  class GridView  {
  private:
      GtkWidget *child_;
      GtkMultiSelection *selectionModel_ = NULL;
      GtkWidget *view_;
      void *gridViewClick_f_;
      char *path_;
      // myMenu is for processing keys for individual widget popovers
      Menu<GridviewMenu<DirectoryClass> > *myMenu_=NULL;
      int maxNameLen_ = 0;
      double x_;
      double y_;
      int flags_=0;
  public:
      double x(void){return x_;}
      double y(void){return y_;}
      void x(double value){x_ = value;}
      void y(double value){y_ = value;}
     
      int maxNameLen(void){return maxNameLen_;}
      
      GListModel *listModel(void){ return G_LIST_MODEL(selectionModel_);}
      GListStore *listStore(void){ 
        return G_LIST_STORE(g_object_get_data(G_OBJECT(selectionModel_), "store"));
      }
      GListStore *store(void){return listStore();}

      void flagOn (int flag){ 
        flags_ |= flag;
        //Settings::setInteger("flags", path_, flags_);
      }
      void flagOff (int flag){  
        flags_ &= (0xff ^ flag);
        //Settings::setInteger("flags", path_, flags_);
      }
      int flags(void){return flags_;}


  private:
      
  public:
      //bool dndOn = false;
      GridView(const char *path, void *gridViewClick_f){
        gridViewClick_f_ = gridViewClick_f;
        path_ = g_strdup(path);
        view_ = getGridView();
        flags_ = Settings::getInteger("flags", path_);
        if (flags_ < 0) flags_ = 0;
        TRACE("gridview flags = 0x%x\n", flags_);
        
        myMenu_ = new Menu<GridviewMenu<DirectoryClass> >("foo");
        addGestureClickView1(view_, NULL, this);// unselect all on release
        addGestureClickView3(view_, NULL, this); // menu 

        //addTopMotionController();
        
        //auto dropController = Dnd<DirectoryClass>::createDropController(this);
        //gtk_widget_add_controller (GTK_WIDGET (view_), GTK_EVENT_CONTROLLER (dropController));
      }

      ~GridView(void){
      TRACE("GridView<DirectoryClass> destructor\n");
        //if (menu_){
          // menu_ goes down with gridview. No need to
          //       unparent or
          //gtk_widget_unparent(GTK_WIDGET(menu_));
          //g_object_unref(G_OBJECT(menu_));
        //}
        if (myMenu_) delete myMenu_; // main menu

        g_free(path_);
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

      


      GtkWidget *
      getGridView(){
        auto child = Child::getChild();
        selectionModel_ = NULL;
        if (strcmp(path_, "Gtk:bookmarks")==0) {
          selectionModel_ = DirectoryClass::rootSelectionModel();
        } else {
          // Create the initial GtkDirectoryList (G_LIST_MODEL).
          selectionModel_ = DirectoryClass::xfSelectionModel(path_);
          maxNameLen_ = DirectoryClass::getMaxNameLen(path_);
          //selectionModel_ = DirectoryClass::standardSelectionModel(path_);     
        }
       
        // GtkListItemFactory implements GtkSignalListItemFactory, which can be connected to
        // bind, setup, teardown and unbind
        GtkListItemFactory *factory = gtk_signal_list_item_factory_new();
        g_object_set_data(G_OBJECT(factory), "child", child);

        /* Connect handler to the factory.
         */
        g_signal_connect( factory, "setup", G_CALLBACK(Factory<DirectoryClass>::factorySetup), this );
        g_signal_connect( factory, "bind", G_CALLBACK(Factory<DirectoryClass>::factoryBind), this);
        g_signal_connect( factory, "unbind", G_CALLBACK(Factory<DirectoryClass>::factoryUnbind), this);
        g_signal_connect( factory, "teardown", G_CALLBACK(Factory<DirectoryClass>::factoryTeardown), this);

        TRACE("size = %d\n",Settings::getInteger("xfterm", "iconsize"));

        TRACE("got maxNameLen_ = %d\n", maxNameLen_);

        GtkWidget *view;
        /* Create the view.
         */
        view = gtk_grid_view_new(GTK_SELECTION_MODEL(selectionModel_), factory);
        g_object_set_data(G_OBJECT(child), "gridview", view);
        gtk_grid_view_set_max_columns(GTK_GRID_VIEW(view), 20);
        //gtk_grid_view_set_min_columns(GTK_GRID_VIEW(view), 10);
        gtk_widget_add_css_class(view, "gridviewColors");
        gtk_grid_view_set_enable_rubberband(GTK_GRID_VIEW(view), TRUE);


        return view;
      }

  private:
    static void setPopoverItems(GtkPopover *popover, GridView<DirectoryClass> *gridView_p){
      auto keys = gridView_p->myMenu_->keys();
      for (auto p=keys; p && *p; p++){
        auto widget = g_object_get_data(G_OBJECT(popover), *p);
        if (widget){
          //TRACE("hide widget \"%s\"\n", *p);
          gtk_widget_set_visible(GTK_WIDGET(widget), false);
        } else {
          DBG("* Warning: cannot find widget \"%s\" to hide.\n", *p);
        }
      }
      //show
      const char *items[]={_("Open with"), _("Delete"),_("Copy"),_("Cut"),_("Create a compressed archive with the selected objects"),NULL};
      for (auto p=items; p && *p; p++){
        auto widget = g_object_get_data(G_OBJECT(popover), *p);
        if (widget){
          gtk_widget_set_visible(GTK_WIDGET(widget), true);
        } else {
          DBG("* Warning: cannot find widget \"%s\" to show.\n", *p);
        }
      }

    }
    
/*
    static GtkPopover *getPopover(GridView<DirectoryClass> *gridView_p){ 
      auto path = gridView_p->path();
      auto markup = g_strdup_printf("<span color=\"blue\"><b>%s</b></span>", path);
      auto popover = gridView_p->myMenu_->mkMenu(markup);
      //gridView_p->setMenu(popover);
      g_object_set_data(G_OBJECT(popover), "gridView_p", gridView_p);

      setPopoverItems(GTK_POPOVER(popover), path, gridView_p);
      g_free(markup);
      g_object_set_data(G_OBJECT(gridView_p->view()), "menu", popover);
      gtk_widget_set_parent(GTK_WIDGET(popover), gridView_p->view());
      return popover;
    }
    */

    static void setPopoverItems(GtkPopover *popover, const char *path, GridView<DirectoryClass> *gridView_p ){
      auto keys = gridView_p->myMenu_->keys();
      for (auto p=keys; p && *p; p++){
      auto widget = g_object_get_data(G_OBJECT(popover), *p);
        if (widget){
          //TRACE("hide widget \"%s\"\n", *p);
          gtk_widget_set_visible(GTK_WIDGET(widget), false);
        } else {
          DBG("* Warning: cannot find widget \"%s\" to hide.\n", *p);
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
    
        //const char *hide[]={_("auto"), NULL};

        // bookmark test
        // mount test
        // pasteboard test
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
      }
    }

    static GtkPopover *getPopover(GtkWidget *menubox, GridView<DirectoryClass> *gridView_p){ 
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

    static GtkPopover *getPopover(GFileInfo *info, GridView<DirectoryClass> *gridView_p){ 
      auto menubox = GTK_WIDGET(g_object_get_data(G_OBJECT(info), "menuBox"));
      auto path = Basic::getPath(info);
      auto markup = g_strdup_printf("<span color=\"blue\"><b>%s</b></span>", path);
      auto popover = gridView_p->myMenu_->mkMenu(markup);
      g_object_set_data(G_OBJECT(popover), "gridView_p", gridView_p);
      g_object_set_data(G_OBJECT(popover), "info", info);

      setPopoverItems(GTK_POPOVER(popover), path, gridView_p);
      g_free(markup);
      g_object_set_data(G_OBJECT(menubox), "menu", popover);
      gtk_widget_set_parent(GTK_WIDGET(popover), menubox);
      g_free(path);
      return popover;
    }

    static GtkPopover *getPopover(GObject *object, GridView<DirectoryClass> *gridView_p){ 
      auto list_item =GTK_LIST_ITEM(object);
      auto info = G_FILE_INFO(gtk_list_item_get_item(list_item));
      auto menubox = GTK_WIDGET(g_object_get_data(G_OBJECT(object), "menuBox"));
      auto path = Basic::getPath(info);
      auto markup = g_strdup_printf("<span color=\"blue\"><b>%s</b></span>", path);
      auto popover = gridView_p->myMenu_->mkMenu(markup);
      g_object_set_data(G_OBJECT(popover), "gridView_p", gridView_p);
      g_object_set_data(G_OBJECT(popover), "info", info);

      setPopoverItems(GTK_POPOVER(popover), path, gridView_p);
      g_free(markup);
      g_object_set_data(G_OBJECT(menubox), "menu", popover);
      gtk_widget_set_parent(GTK_WIDGET(popover), menubox);
      g_free(path);
      return popover;
    }
/*    
*/    
  static void setupMenu(GtkPopover *popover, GFileInfo *info){
      auto path = Basic::getPath(info);
      auto isDir = g_file_test(path, G_FILE_TEST_IS_DIR);
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
          auto paintable = Texture<bool>::load(n, 16);


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

        
      } else {DBG("** Error:: no auto button\n");}
      if (isDir){
        auto paste = g_object_get_data(G_OBJECT(popover), _("Paste"));
        auto nopaste = g_object_get_data(G_OBJECT(popover), _("Clipboard is empty."));
        auto c = (ClipBoard *)g_object_get_data(G_OBJECT(MainWidget), "ClipBoard");
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
  static void placeMenu(GtkWidget *menubox, GridView<DirectoryClass> *gridView_p){
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
  static void placeMenu(GFileInfo *info, GridView<DirectoryClass> *gridView_p){
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
  static void placeMenu(GObject *object, GridView<DirectoryClass> *gridView_p){
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
      auto gridView_p = (GridView<DirectoryClass> *)data;
      if (gridView_p->getSelectionSize() == 0)
      {
        MainWindow<DirectoryClass>::clickMenu( mainMenuButton, NULL);
      } else {
        return Factory<DirectoryClass>::menu_f(self, n_press, x , y, data);
      }
      return true;
    }
   static gboolean
    unselect_f(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              void *data){
      auto gridView_p = (GridView<DirectoryClass> *)data;
      auto selectionModel = gridView_p->selectionModel();
      // if control or shift down, return false.
      auto eventController = GTK_EVENT_CONTROLLER(self);
      auto event = gtk_event_controller_get_current_event(eventController);
      auto modifierType = gdk_event_get_modifier_state (event);
      if (modifierType & ((GDK_CONTROL_MASK & GDK_MODIFIER_MASK))) return false;
      if (modifierType & ((GDK_SHIFT_MASK & GDK_MODIFIER_MASK))) return false;
      
      gtk_selection_model_unselect_all(GTK_SELECTION_MODEL(selectionModel));
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

    static void addGestureClickView1(GtkWidget *self, GObject *object, GridView<DirectoryClass> *gridView_p){
      auto gesture = gtk_gesture_click_new();
      gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),1); 
      // 1 for unselect
      g_signal_connect (G_OBJECT(gesture) , "released", EVENT_CALLBACK (unselect_f), (void *)gridView_p);
      gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(gesture));
      gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture), 
          GTK_PHASE_BUBBLE);

    }    

    static void addGestureClickView3(GtkWidget *self, GObject *object, GridView<DirectoryClass> *gridView_p){
      auto gesture = gtk_gesture_click_new();
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
        //TRACE("viewMotion %lf, %lf\n", x, y);
        auto gridview_p = (GridView<DirectoryClass> *)data;
        return FALSE;
    }
      void addTopMotionController(void){
        auto controller = gtk_event_controller_motion_new();
        gtk_event_controller_set_propagation_phase(controller, GTK_PHASE_CAPTURE);
        gtk_widget_add_controller(GTK_WIDGET(view_), controller);
        g_signal_connect (G_OBJECT (controller), "motion", 
            G_CALLBACK (viewMotion), this);
    }


    
    private:
      static void
      onDragBegin (GtkDragSource *source,
                     GdkDrag *drag,
                     void *data)
      {
        // Set the widget as the drag icon
        GdkPaintable *paintable = Texture<bool>::load("emblem-redball", 48);
        gtk_drag_source_set_icon (source, paintable, 0, 0);
        g_object_unref (paintable);
      }

static GdkContentProvider *
on_drag_prepare (GtkDragSource *source,
                 double         x,
                 double         y,
                 GridView<DirectoryClass> *gridView_p)
{

  auto list = gridView_p->getSelectionList();
  const char *text = "hello";
  return gdk_content_provider_new_typed ("text/uri-list", text);

  // This widget supports two types of content: GFile objects
  // and GdkPixbuf objects; GTK will handle the serialization
  // of these types automatically
/*  GFile *file = my_widget_get_file (self);
  GdkPixbuf *pixbuf = my_widget_get_pixbuf (self);

  return gdk_content_provider_new_union ((GdkContentProvider *[2]) {
      gdk_content_provider_new_typed (G_TYPE_FILE, file),
      gdk_content_provider_new_typed (GDK_TYPE_PIXBUF, pixbuf),
    }, 2);*/
}

  };
}
#endif
