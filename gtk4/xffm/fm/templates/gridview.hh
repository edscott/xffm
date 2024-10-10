#ifndef GRIDVIEW_HH
#define GRIDVIEW_HH

namespace xf {
  template <class DirectoryClass>
  class GridView  {
  private:
      GtkMultiSelection *selection_model_ = NULL;
      GtkPopover *menu_=NULL;
      GtkWidget *view_;
      void *gridViewClick_f_;
      char *path_;
      Menu<GridviewMenu<bool> > *myMenu_=NULL;
  public:
      GtkMultiSelection *selectionModel(void){ return selection_model_;}
      GtkPopover *menu(void){ return menu_;}
      GtkWidget *view(void){ return view_;}
      void *gridViewClick_f(void){ return gridViewClick_f_;}
      char *path(void){ return path_;}
      
      GridView(const char *path, void *gridViewClick_f){
        gridViewClick_f_ = gridViewClick_f;
        path_ = g_strdup(path);
        view_ = getGridView();
        myMenu_ = new Menu<GridviewMenu<bool> >("foo");
        addGestureClickSelection(view_, NULL, this);
      }
      ~GridView(void){
        if (menu_)g_object_unref(G_OBJECT(menu_));
        if (myMenu_) delete myMenu_;
        g_free(path_);
      }

      GtkWidget *
      getGridView(){
        auto child = Child::getChild();
        selection_model_ = NULL;
        if (strcmp(path_, "xffm:root")==0) {
          selection_model_ = DirectoryClass::rootSelectionModel();
        } else {
          // Create the initial GtkDirectoryList (G_LIST_MODEL).
          selection_model_ = DirectoryClass::xfSelectionModel(path_);
          //selection_model_ = DirectoryClass::standardSelectionModel(path_);     
        }
       
        // GtkListItemFactory implements GtkSignalListItemFactory, which can be connected to
        // bind, setup, teardown and unbind
        GtkListItemFactory *factory = gtk_signal_list_item_factory_new();
        g_object_set_data(G_OBJECT(factory), "child", child);

        /* Connect handler to the factory.
         */
        g_signal_connect( factory, "setup", G_CALLBACK(factorySetup), NULL );
        g_signal_connect( factory, "bind", G_CALLBACK(factoryBind), this);

        GtkWidget *view;
        /* Create the view.
         */
        view = gtk_grid_view_new(GTK_SELECTION_MODEL(selection_model_), factory);
        g_object_set_data(G_OBJECT(child), "gridview", view);
        gtk_grid_view_set_max_columns(GTK_GRID_VIEW(view), 20);
        //gtk_grid_view_set_min_columns(GTK_GRID_VIEW(view), 10);
        gtk_widget_add_css_class(view, "gridviewColors");
        gtk_grid_view_set_enable_rubberband(GTK_GRID_VIEW(view), TRUE);


        return view;
      }

  private:
    static void setPopoverItems(GtkPopover *popover, const char *path, GridView *gridView_p ){
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

    static GtkPopover *getPopover(GFileInfo *info, GridView *gridView_p){ 
      auto path = Basic::getPath(info);
      auto markup = g_strdup_printf("<span color=\"blue\"><b>%s</b></span>", path);
      auto popover = gridView_p->myMenu_->mkMenu(markup);
      g_object_set_data(G_OBJECT(popover), "info", info);

      setPopoverItems(GTK_POPOVER(popover), path, gridView_p);
      g_free(markup);
      auto menuBox2 = GTK_WIDGET(g_object_get_data(G_OBJECT(info), "menuBox2"));
      g_object_set_data(G_OBJECT(menuBox2), "menu", popover);
      gtk_widget_set_parent(GTK_WIDGET(popover), menuBox2);
      g_free(path);
      return popover;
    }
    
  static void setupMenu(GtkPopover *popover, GFileInfo *info){
      auto path = Basic::getPath(info);
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
          auto paintable = Texture::load(n, 16);

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
      //gtk_widget_remove_css_class (GTK_WIDGET(imageBox), "pathbarboxNegative" );
      g_free(path);
  }

    static gboolean
    menu_f(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              void *data){
      auto gridView_p = (GridView *)data;
      auto eventController = GTK_EVENT_CONTROLLER(self);
      auto event = gtk_event_controller_get_current_event(eventController);
      auto box = gtk_event_controller_get_widget(eventController);
      auto menuBox = GTK_WIDGET(g_object_get_data(G_OBJECT(box), "menuBox"));

      auto modType = gdk_event_get_modifier_state(event);

      TRACE("modType = 0x%x\n", modType);
      if (modType & GDK_CONTROL_MASK) return false;
      if (modType & GDK_SHIFT_MASK) return false;

      auto selectionModel = gridView_p->selectionModel();


      GtkBitset *bitset = gtk_selection_model_get_selection (GTK_SELECTION_MODEL(selectionModel));
      auto size = gtk_bitset_get_size(bitset);
      if (size > 0) return false;


      
      TRACE("gestureClick; object=%p button=%d\n", object,
          gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(self)));
      TRACE("gestureClick; path=%p\n", gridView_p->path());
      TRACE("menu for %s\n", gridView_p->path());

      auto info = G_FILE_INFO(g_object_get_data(G_OBJECT(menuBox), "info"));
      auto popover = g_object_get_data(G_OBJECT(menuBox), "menu");
  
      if (!popover){
        popover = getPopover(info, gridView_p);
      }
      
      if (popover) {
        setupMenu(GTK_POPOVER(popover), info);
        gtk_popover_popup(GTK_POPOVER(popover));
      }
      
   
      return true;
    }

    static gboolean
    menuSelection_f(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              void *data){
      DBG("menuSelection_f...\n");
      auto gridView_p = (GridView *)data;
      auto eventController = GTK_EVENT_CONTROLLER(self);
      auto event = gtk_event_controller_get_current_event(eventController);
      auto selectionModel = gridView_p->selectionModel();


      GtkBitset *bitset = gtk_selection_model_get_selection (GTK_SELECTION_MODEL(selectionModel));
      auto size = gtk_bitset_get_size(bitset);
      DBG("gtk_bitset_get_size = %ld\n", size);
return true;      
      guint position;
      GtkBitsetIter iter;
      if (gtk_bitset_iter_init_first (&iter, bitset, &position)){
        /*do{
          auto list = G_LIST_MODEL(selectionModel);
          auto info = G_FILE_INFO(g_list_model_get_item (list, position));
          auto path = Basic::getPath(info);
          DBG("item=%lu, path = %s\n", position, path);
          g_free(path);
        } while (gtk_bitset_iter_next (&iter,&position));*/
        auto list = G_LIST_MODEL(selectionModel);
        auto info = G_FILE_INFO(g_list_model_get_item (list, position));
        auto menuBox2 = GTK_WIDGET(g_object_get_data(G_OBJECT(info), "menuBox2"));
        auto popover = g_object_get_data(G_OBJECT(menuBox2), "menu");
        if (!popover){          
          auto path = Basic::getPath(info);
          auto markup = g_strdup_printf("<span color=\"blue\"><b>%s</b></span>", path);
          popover = gridView_p->myMenu_->mkMenu(markup);
          g_object_set_data(G_OBJECT(popover), "info", info);

          setPopoverItems(GTK_POPOVER(popover), path, gridView_p);
          g_free(markup);
          g_object_set_data(G_OBJECT(menuBox2), "menu", popover);
          gtk_widget_set_parent(GTK_WIDGET(popover), menuBox2);
          g_free(path);
        }
        if (popover) gtk_popover_popup(GTK_POPOVER(popover));

      }
      return true;
    }
    
    static void addGestureClickSelection(GtkWidget *self, GObject *object, GridView *gridView_p){
      auto gesture = gtk_gesture_click_new();
      gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),3); 
      // 3 for popover pressed
      g_signal_connect (G_OBJECT(gesture) , "pressed", EVENT_CALLBACK (menuSelection_f), (void *)gridView_p);
      gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(gesture));
      gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture), 
          GTK_PHASE_CAPTURE);

    }    
    
    static void addGestureClickMenu(GtkWidget *imageBox, GObject *object, GridView *gridView_p){
      TRACE("addGestureClick; object=%p\n", gridView_p);
      g_object_set_data(G_OBJECT(imageBox), "object", object);// info, already set.
      auto gesture = gtk_gesture_click_new();
      gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),3); 
      // 3 for popover pressed
      g_signal_connect (G_OBJECT(gesture) , "pressed", EVENT_CALLBACK (menu_f), (void *)gridView_p);
      gtk_widget_add_controller(GTK_WIDGET(imageBox), GTK_EVENT_CONTROLLER(gesture));
      gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture), 
          GTK_PHASE_CAPTURE);

    }    
    
    static void addGestureClick(GtkWidget *imageBox, GObject *object, GridView *gridView_p){
      TRACE("addGestureClick; object=%p\n", object);
      auto gesture = gtk_gesture_click_new();
      gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),1); 
      // 1 for action released.
      g_signal_connect (G_OBJECT(gesture) , "pressed", EVENT_CALLBACK (gridView_p->gridViewClick_f()), (void *)object);
      gtk_widget_add_controller(GTK_WIDGET(imageBox), GTK_EVENT_CONTROLLER(gesture));
      gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture), 
          GTK_PHASE_CAPTURE);
    }    


      static void
      factorySetup(GtkSignalListItemFactory *self, GObject *object, void *data){
        GtkWidget *box;
        if (Settings::getInteger("xfterm", "iconsize") <= 30)
        {
          box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        } else {
          box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
        }
        gtk_widget_add_css_class(box, "gridviewBox");
        gtk_widget_set_vexpand(GTK_WIDGET(box), FALSE);
        gtk_widget_set_hexpand(GTK_WIDGET(box), FALSE);
        GtkListItem *list_item = GTK_LIST_ITEM(object);
        gtk_list_item_set_child(list_item, box);

   }

      /* The bind function for the factory */
      static void
      factoryBind(GtkSignalListItemFactory *factory, GObject *object, GridView *gridView_p)
      {
        auto child = GTK_WIDGET(g_object_get_data(G_OBJECT(factory), "child"));
        auto list_item =GTK_LIST_ITEM(object);
        auto box = GTK_BOX(gtk_list_item_get_child( list_item ));
        auto list = Basic::getChildren(box);
        for (auto l=list; l && l->data; l=l->next){
          gtk_widget_unparent(GTK_WIDGET(l->data));
        }

        auto menuBox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        auto menuBox2 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        auto imageBox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        g_object_set_data(G_OBJECT(imageBox), "menuBox", menuBox);
        g_object_set_data(G_OBJECT(box), "imageBox", imageBox);
        g_object_set_data(G_OBJECT(box), "menuBox", menuBox);
        gtk_widget_add_css_class(menuBox, "gridviewBox");
        gtk_widget_add_css_class(imageBox, "gridviewBox");
        gtk_box_append(GTK_BOX(menuBox), imageBox);
        gtk_box_append(GTK_BOX(menuBox2), menuBox);

        auto labelBox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_add_css_class(labelBox, "gridviewBox");
        g_object_set_data(G_OBJECT(box), "labelBox", labelBox);
        
        auto info = G_FILE_INFO(gtk_list_item_get_item(list_item));
        g_object_set_data(G_OBJECT(menuBox), "info", info);
        g_object_set_data(G_OBJECT(info), "menuBox2", menuBox2);
        auto file = G_FILE(g_file_info_get_attribute_object (info, "standard::file"));
        auto path = g_file_get_path(file);

        
        auto type = g_file_info_get_file_type(info);
        char *name = g_strdup(g_file_info_get_name(info));
        auto size = Settings::getInteger("xfterm", "iconsize");
        GdkPaintable *texture = NULL;
        /*
        auto texture = Texture::load(path);
        g_free(path);
        if (texture) scaleFactor = 2;
         */
        if (!texture) {
          // Gets the texture from the GIcon. Fast.
          texture = Texture::load(info);
        }
        // XXX: put all icons in hash (debug)
          
     //   if ((type == G_FILE_TYPE_DIRECTORY )||(symlinkToDir(info, type))) {
        if (name[0] == '.' && name[1] != '.') {

          auto *iconPath = Texture::findIconPath(info);
          // Only for the hidden items. Applies background mask.
          if (iconPath) texture = Texture::getSvgPaintable(iconPath, size, size);   

        }
        //
        GtkWidget *image = gtk_image_new_from_paintable(GDK_PAINTABLE(texture));
        if (type == G_FILE_TYPE_DIRECTORY ) {
          DirectoryClass::addDirectoryTooltip(image, info);
        }
        auto isImage = (type == G_FILE_TYPE_REGULAR && Mime::is_image(path));
        if (size < 0) size = 48;
        double scaleFactor = 1.;
        if (isImage) scaleFactor = 2;

        if (size == 24) scaleFactor = 0.75;
        gtk_widget_set_size_request(image, size*scaleFactor, size*scaleFactor);

   

        //auto label = GTK_LABEL(g_object_get_data(G_OBJECT(vbox), "label"));

        if (name && strlen(name) > 16){
          name[15] = 0;
          name[14] ='~';
        }
        char buffer[20];
        if (size == 24) snprintf(buffer, 20, "%-16s", name);
        else snprintf(buffer, 20, "%s", name);
        
        char *markup = g_strconcat("<span size=\"small\">", buffer, "</span>", NULL);
//        char *markup = g_strconcat("<span size=\"small\">", name, "</span>", NULL);
        auto label = gtk_label_new("");
        gtk_label_set_markup(GTK_LABEL(label), markup);
        g_free(markup);
        DirectoryClass::addLabelTooltip(label, path); 

        Basic::boxPack0(GTK_BOX(imageBox), GTK_WIDGET(image), FALSE, FALSE, 0);    
        Basic::boxPack0(GTK_BOX(labelBox), label, FALSE, FALSE, 0);    
        Basic::boxPack0(GTK_BOX(box), menuBox2, FALSE, FALSE, 0);    
        Basic::boxPack0(GTK_BOX(box), labelBox, FALSE, FALSE, 0);  
  

        if (Settings::getInteger("xfterm", "iconsize") == 24 && strcmp(name, "..")){
          // file information string
          auto hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
          gtk_widget_add_css_class(hbox, "gridviewBox");
          auto props = gtk_label_new("");

          struct stat st;
          lstat(path, &st);
       
          auto m1 = Basic::statInfo(&st);

          char *markup = g_strdup("<span size=\"small\" color=\"blue\">");
          Basic::concat(&markup, m1);
          Basic::concat(&markup, "</span>");
          
          g_free(m1);
          gtk_label_set_markup(GTK_LABEL(props), markup);
          g_free(markup);

          Basic::boxPack0(GTK_BOX(box), props, FALSE, FALSE, 0);  
          Basic::boxPack0(GTK_BOX(box), GTK_WIDGET(hbox), FALSE, TRUE, 0);    


        } else {
        }
        g_free(name);

        addMotionController(labelBox);
        // FIXME:: gray mask falls onto popover:
         addMotionController(imageBox);
        addGestureClick(imageBox, object, gridView_p);
        addGestureClickMenu(imageBox, object, gridView_p);
        g_free(path);

        // Now for replacement of image icons for previews
        // This could only be done when load has completed,
        // and that is because disk access is serialize by bus.
        // But it really fast from sd disk...
        // 
        if (isImage && Texture::previewOK()){ // thread number limited.
          auto path = g_file_get_path(file);
          // path, imageBox, image, serial
          auto arg = (void **)calloc(6, sizeof(void *));
          arg[0] = (void *)path;
          arg[1] = imageBox;
          arg[2] = image;
          arg[3] = GINT_TO_POINTER(Child::getSerial()); // in main context
          arg[4] = GINT_TO_POINTER(size*scaleFactor); // in main context
          arg[5] = child; // in main context
          Thread::threadPoolAdd(Texture::preview, (void *)arg);
        }


      }


    private:
      static
      void addMotionController(GtkWidget  *widget){
        auto controller = gtk_event_controller_motion_new();
        gtk_event_controller_set_propagation_phase(controller, GTK_PHASE_CAPTURE);
        gtk_widget_add_controller(GTK_WIDGET(widget), controller);
        g_signal_connect (G_OBJECT (controller), "enter", 
            G_CALLBACK (negative), widget);
        g_signal_connect (G_OBJECT (controller), "leave", 
            G_CALLBACK (positive), widget);
    }
     // FIXME: if gridview Settings color for background is
      //        too close to #acaaa5, use a different css class color
      //
      //        Also: add highlight color for text box and change 
      //              text from label to entry, to allow inline
      //              renaming on longpress.

    static gboolean
    negative ( GtkEventControllerMotion* self,
                    gdouble x,
                    gdouble y,
                    gpointer data) 
    {
        auto eventBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
        gtk_widget_add_css_class (GTK_WIDGET(eventBox), "pathbarboxNegative" );
        //Basic::flushGTK(); // this will cause race condition crash...
        return FALSE;
    }
    static gboolean
    positive ( GtkEventControllerMotion* self,
                    gdouble x,
                    gdouble y,
                    gpointer data) 
    {
        auto eventBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
        gtk_widget_remove_css_class (GTK_WIDGET(eventBox), "pathbarboxNegative" );
        //Basic::flushGTK();
        return FALSE;
    }

    
    private:

  };
}
#endif
