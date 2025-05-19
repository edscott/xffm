  
#ifndef FACTORYLOCAL_HH
#define FACTORYLOCAL_HH
// object is list_item
// item_get_item is GFileInfo
// item_get_child is GtkWidget
namespace xf {
template <class Type>
  class Factory {
    using clipboard_t = ClipBoard<LocalDir>;
    public:
      /* {{{ Factory callbacks */
      static void
      factoryTeardown(GtkSignalListItemFactory *self, GObject *object, GridView<Type> *gridView_p){
        TRACE("factoryTeardown...\n");
        GtkListItem *list_item = GTK_LIST_ITEM(object);
        GtkBox *vbox = GTK_BOX(gtk_list_item_get_child( list_item ));
        // vbox is null on new updateGridview() (new Gridview).
        // unparent() also fails on close xffm...
        // if (vbox && GTK_IS_WIDGET(vbox)) gtk_widget_unparent(GTK_WIDGET(vbox));
        gtk_list_item_set_child(list_item, NULL);
     }

      static void
      factorySetup(GtkSignalListItemFactory *self, GObject *object, GridView<Type> *gridView_p){
        // object is a GtkListItem!
        //TRACE("factorySetup...object=%p GtkListItem=%p\n", object, GTK_LIST_ITEM(object));
        auto box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
        auto menuBox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
        auto menuBox2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
        auto hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        auto imageBox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        auto labelBox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        auto hlabelBox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        auto hlabel = gtk_label_new("");
        auto label = gtk_label_new("");
        
        GtkWidget *boxes[] = {box, menuBox, menuBox2, hbox, imageBox, labelBox, hlabelBox, NULL};
        for (auto p = boxes; p && *p; p++){
          gtk_widget_add_css_class(*p, "gridviewBox");
          gtk_widget_set_vexpand(GTK_WIDGET(*p), false);
          gtk_widget_set_hexpand(GTK_WIDGET(*p), false);
        }

        gtk_list_item_set_child(GTK_LIST_ITEM(object), box); 
        gtk_box_append(GTK_BOX(box), menuBox);
        gtk_box_append(GTK_BOX(menuBox), menuBox2);
        gtk_box_append(GTK_BOX(menuBox2), hbox);
        gtk_box_append(GTK_BOX(hbox), imageBox);
        gtk_box_append(GTK_BOX(hbox), hlabelBox);
        gtk_box_append(GTK_BOX(menuBox2), labelBox);
        gtk_box_append(GTK_BOX(hlabelBox), hlabel);
        gtk_box_append(GTK_BOX(labelBox), label);
        

        g_object_set_data(G_OBJECT(object), "imageBox", imageBox);
        g_object_set_data(G_OBJECT(object), "label", label);
        g_object_set_data(G_OBJECT(object), "hlabel", hlabel);
        g_object_set_data(G_OBJECT(object), "menuBox", menuBox);
        g_object_set_data(G_OBJECT(object), "menuBox2", menuBox2);

 
        g_object_set_data(G_OBJECT(labelBox), "object", object);
        g_object_set_data(G_OBJECT(hlabelBox), "object", object);
        g_object_set_data(G_OBJECT(imageBox), "object", object);
        g_object_set_data(G_OBJECT(menuBox), "object", object);
        g_object_set_data(G_OBJECT(menuBox2), "object", object);

        TRACE("factorySetup add signal handlers\n");
        
        addMotionController(labelBox);
        addMotionController(hlabelBox);

        addMotionController(imageBox);
        addGestureClickDown(imageBox, object, gridView_p);
        addGestureClickLong(labelBox, object, gridView_p);
        addGestureClickLongMenu(imageBox, object, gridView_p);
        //addGestureClickDownLabel(labelBox, object, gridView_p);
        //addGestureClickDownBox(box, object, gridView_p);
        addGestureClickDown3(imageBox, object, gridView_p);

        addGestureClickUp(imageBox, object, gridView_p);
        addGestureClickUp(labelBox, object, gridView_p);
        addGestureClickUp(hlabelBox, object, gridView_p);

        addGestureClickMenu(imageBox, object, gridView_p);
        addGestureClickMenu(labelBox, object, gridView_p);
        addGestureClickMenu(hlabelBox, object, gridView_p);

        TRACE("factorySetup: object(GTK_LIST_ITEM) = %p box = %p\n", object, box);
        
      }

      static void
      factoryUnbind(GtkSignalListItemFactory *self, GObject *object, GridView<Type> *gridView_p){
        TRACE("factoryUnbind...\n");
        //THREADPOOL->clear();
        //Child::incrementSerial();
     
         // cleanup on regen :
        auto imageBox = GTK_BOX(g_object_get_data(object, "imageBox"));
        auto oldImage = gtk_widget_get_first_child(GTK_WIDGET(imageBox));
        if (oldImage) gtk_widget_unparent(oldImage);
     }


      /* The bind function for the factory */
      static void
      factoryBind(GtkSignalListItemFactory *factory, GObject *object, GridView<Type> *gridView_p)
      {
        //TRACE("factoryBind...object=%p GtkListItem=%p\n", object, GTK_LIST_ITEM(object));
       
        // const:
        auto child = GTK_WIDGET(g_object_get_data(G_OBJECT(factory), "child"));
        auto list_item =GTK_LIST_ITEM(object);
        auto box = GTK_BOX(gtk_list_item_get_child( list_item ));
        auto imageBox = GTK_BOX(g_object_get_data(object, "imageBox"));
        auto label = GTK_LABEL(g_object_get_data(object, "label"));
        auto hlabel = GTK_LABEL(g_object_get_data(object, "hlabel"));

        auto info = G_FILE_INFO(gtk_list_item_get_item(list_item));
        g_object_set_data(G_OBJECT(info), "imageBox", imageBox);
        //g_object_set_data(G_OBJECT(info), "box", box);
        g_object_set_data(G_OBJECT(imageBox), "info", info);
        
        auto menuBox = GTK_BOX(g_object_get_data(object, "menuBox"));
        g_object_set_data(G_OBJECT(info), "menuBox", menuBox);

        auto menuBox2 = GTK_BOX(g_object_get_data(object, "menuBox2"));
        g_object_set_data(G_OBJECT(info), "menuBox2", menuBox2);

        auto type = g_file_info_get_file_type(info);
        //auto size = Settings::getInteger("xfterm", "iconsize");
        auto size = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(child),"iconsize"));
        // allocated:
        auto path = Basic::getPath(info);      
        const char *rawName =  g_file_info_get_name(info);
        char *name = Basic::utf_string(rawName);

        TRACE("factory bind name= %s\n", name);

        if (size < 0) size = 48;
        double scaleFactor = 1.0;
        GdkPaintable *texture = NULL;
        GtkWidget *image = NULL;
        // xffm:paintable takes preference over all. 
        // (custom icon, preferably for mount point emblem, bookmark emblem, )
        auto xffmPaintable = g_file_info_get_attribute_object(info, "xffm:paintable");      
        if (xffmPaintable) {
          image = gtk_image_new_from_paintable(GDK_PAINTABLE(xffmPaintable));
        } else {
          image = emblemedImage(name, path, info, size);
        }

        bool previewLoaded = false;
        bool doPreview = false;
        if (!image && !g_file_info_get_is_symlink(info)){
          image = previewImage(info, path, &doPreview);
          if (image) {
            previewLoaded = true;
          } 
        }
        if (doPreview)  scaleFactor = 2.0;
        if (size == 24) scaleFactor = 0.75;
        
        if (!image){
          if (g_file_info_get_is_symlink(info)){
            struct stat st;
            if (stat(path, &st) < 0) {
              auto paintable = Texture<bool>::load(EMBLEM_BROKEN, size);
              texture = Texture<bool>::addEmblem(paintable,  EMBLEM_SYMLINK, scaleFactor*size, scaleFactor*size);
            } else {
              texture = Texture<bool>::addEmblem(info,  EMBLEM_SYMLINK, scaleFactor*size, scaleFactor*size);
            }
          } else { // simple
            auto gfiletype= g_file_info_get_file_type (info);
            switch (gfiletype){
              default:
                texture = Texture<bool>::load(info, size); // Loads icon from icontheme.
                break;
              /*case (G_FILE_TYPE_MOUNTABLE): // does not work FIXME: use fstab routines
                {
                  bool isMounted = false; // FIXME with fstab routine
                  texture = Texture<bool>::addEmblem(info,  
                      isMounted? EMBLEM_GREEN_BALL : EMBLEM_RED_BALL, 
                      scaleFactor*size, scaleFactor*size);
                }
                break;*/
            }
          }
          image = gtk_image_new_from_paintable(GDK_PAINTABLE(texture));
          // Texture is not referenced in hash table. 
          g_object_unref(texture);        
        }
        if (image) {
          gtk_widget_set_size_request(image, size*scaleFactor, size*scaleFactor);
          // cleanup on regen:
          //auto oldImage = gtk_widget_get_first_child(GTK_WIDGET(imageBox));
          //if (oldImage) gtk_widget_unparent(oldImage);
          Basic::boxPack0(GTK_BOX(imageBox), GTK_WIDGET(image), FALSE, FALSE, 0);    
        } else {DBG("Error:: Should not happen: image is NULL\n");}
        

        if (type == G_FILE_TYPE_DIRECTORY ) {
          // not much use, really.
          //Type::addDirectoryTooltip(image, info);
        }

        char buffer[128];
        if (size == 24){
          gtk_widget_set_visible(GTK_WIDGET(label), false);
          gtk_widget_set_visible(GTK_WIDGET(hlabel), true);
          auto format = g_strdup_printf("%%-%ds", gridView_p->maxNameLen());
          TRACE("size 24, format=%s\n", format);
          snprintf(buffer, 128, (const char *)format, name);

          auto markup = g_strdup_printf("<span size=\"%s\">%s</span>",  "x-small", buffer);

          if (strcmp(name, "..")){
            // file information string
            auto hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
            gtk_widget_add_css_class(hbox, "gridviewBox");

            struct stat st;
            lstat(path, &st);
         
            auto m1 = Basic::statInfo(&st);

            char *markup2 = g_strdup(" <span size=\"x-small\" color=\"blue\">");
            Basic::concat(&markup2, m1);
            Basic::concat(&markup2, "</span>");
            
            g_free(m1);
            Basic::concat(&markup, markup2);
            g_free(markup2);

            //Basic::boxPack0(GTK_BOX(box), GTK_WIDGET(hbox), FALSE, TRUE, 0);    
          } 
          gtk_label_set_markup(hlabel, markup);
          g_free(markup);
        } 
        else 
        {
          gtk_widget_set_visible(GTK_WIDGET(label), true);
          gtk_widget_set_visible(GTK_WIDGET(hlabel), false);
        
          const char *sizeS = "x-small";
          if (size <= 96) sizeS = "small";
          else if (size <= 156) sizeS = "medium";
          else if (size <= 192) sizeS = "large";
          else sizeS = "x-large";
          auto markup = g_strdup_printf("<span size=\"%s\">%s</span>", sizeS, name);
          gtk_label_set_markup(label, markup);
          g_free(markup);
          
          //Type::addLabelTooltip(GTK_WIDGET(label), path);
        }
        // gtk drag (deprecated for low level gdk_drag)
        /*GtkDragSource *source = gtk_drag_source_new ();
        g_signal_connect (source, "prepare", G_CALLBACK (Dnd<Type>::image_drag_prepare),gridView_p);
        
        g_signal_connect (source, "drag-begin", G_CALLBACK (Dnd<Type>::image_drag_begin), image);
        gtk_widget_add_controller (image, GTK_EVENT_CONTROLLER (source));*/
        


        if (doPreview && !previewLoaded){

          // if hash value exists and is ok, skip regen
          // otherwise, plug into threadpool.
          TRACE("factory bind add preview threads\n");
          // path, imageBox, image, serial
          auto arg = (void **)calloc(6, sizeof(void *));
          arg[0] = (void *)g_strdup(path);
          arg[1] = imageBox;
          arg[2] = image;
          arg[3] = GINT_TO_POINTER(Child::getSerial()); // in main context
          arg[4] = GINT_TO_POINTER(size*scaleFactor); // in main context
          arg[5] = child; // in main context
          //Thread::threadPoolAdd(Texture<bool>::preview, (void *)arg);
          // thread number limited.
          THREADPOOL->add(Preview<bool>::preview, (void *)arg);
        }
        g_free(path);
        TRACE("factory bind complete: %s\n", name);
        g_free(name);
      }

      /* }}} */

    private:
      static
      void addMotionController(GtkWidget  *widget){
        auto controller = gtk_event_controller_motion_new();
        gtk_event_controller_set_propagation_phase(controller, GTK_PHASE_CAPTURE);
        gtk_widget_add_controller(GTK_WIDGET(widget), controller);
        g_signal_connect (G_OBJECT (controller), "enter", 
            G_CALLBACK (gridNegative), widget);
        g_signal_connect (G_OBJECT (controller), "leave", 
            G_CALLBACK (gridPositive), widget);
    }

      static gboolean
    gridNegative ( GtkEventControllerMotion* self,
                    gdouble x,
                    gdouble y,
                    gpointer data) 
    {
        auto eventBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
        gtk_widget_add_css_class (GTK_WIDGET(eventBox), "gridNegative" );
        //Basic::flushGTK(); // this will cause race condition crash...
        return FALSE;
    }
    static gboolean
    gridPositive ( GtkEventControllerMotion* self,
                    gdouble x,
                    gdouble y,
                    gpointer data) 
    {
        auto eventBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
        gtk_widget_remove_css_class (GTK_WIDGET(eventBox), "gridNegative" );
        //Basic::flushGTK();
        return FALSE;
    }
    
    static void addGestureClickMenu(GtkWidget *box, GObject *object, GridView<Type> *gridView_p){
      TRACE("addGestureClickMenu; object=%p\n", gridView_p);
      auto gesture = gtk_gesture_click_new();
      gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),3); 
      // 3 for popover pressed
      g_signal_connect (G_OBJECT(gesture) , "released", EVENT_CALLBACK (menu_f), (void *)gridView_p);
      gtk_widget_add_controller(GTK_WIDGET(box), GTK_EVENT_CONTROLLER(gesture));
      gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture), 
          GTK_PHASE_CAPTURE);

    }    
/*    
    static void addGestureClickDownBox(GtkWidget *self, GObject *item, GridView<Type> *gridView_p){
      g_object_set_data(G_OBJECT(self), "item", item);
      auto gesture = gtk_gesture_click_new();
      gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),1); 
      // 1 for rename
      TRACE("addGestureClickDownBox: self = %p, item=%p\n", self, item);
      g_signal_connect (G_OBJECT(gesture) , "pressed", EVENT_CALLBACK (box_f), (void *)gridView_p);
      gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(gesture));
      gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture), 
          GTK_PHASE_CAPTURE);

    }    
*/
    static void addGestureClickLong(GtkWidget *self, GObject *item, GridView<Type> *gridView_p){
      g_object_set_data(G_OBJECT(self), "item", item);
      auto gesture = gtk_gesture_long_press_new();
      gtk_gesture_long_press_set_delay_factor(GTK_GESTURE_LONG_PRESS(gesture), 1.0);
      g_signal_connect (G_OBJECT(gesture) , "pressed", EVENT_CALLBACK (rename_f), (void *)gridView_p);
      gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(gesture));
      gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture), 
          GTK_PHASE_CAPTURE);
    }
    static void addGestureClickLongMenu(GtkWidget *self, GObject *item, GridView<Type> *gridView_p){
      g_object_set_data(G_OBJECT(self), "item", item);
      auto gesture = gtk_gesture_long_press_new();
      gtk_gesture_long_press_set_delay_factor(GTK_GESTURE_LONG_PRESS(gesture), 2.0);
      g_signal_connect (G_OBJECT(gesture) , "pressed", EVENT_CALLBACK (longPress_f), (void *)gridView_p);
      gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(gesture));
      gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture), 
          GTK_PHASE_CAPTURE);
    }
/*    
    static void addGestureClickDownLabel(GtkWidget *self, GObject *item, GridView<Type> *gridView_p){
      g_object_set_data(G_OBJECT(self), "item", item);
      auto gesture = gtk_gesture_click_new();
      gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),1); 
      // 1 for rename
      TRACE("addGestureClickDownLabel: self = %p, item=%p\n", self, item);
      g_signal_connect (G_OBJECT(gesture) , "pressed", EVENT_CALLBACK (label_f), (void *)gridView_p);
      gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(gesture));
      gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture), 
          GTK_PHASE_CAPTURE);

    }    
  */  
    static void addGestureClickDown(GtkWidget *self, GObject *item, GridView<Type> *gridView_p){
      g_object_set_data(G_OBJECT(self), "item", item);
      auto gesture = gtk_gesture_click_new();
      gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),1); 
      // 1 for select
      TRACE("addGestureClickDown: self = %p, item=%p\n", self, item);
      g_signal_connect (G_OBJECT(gesture) , "pressed", EVENT_CALLBACK (down_f), (void *)gridView_p);
      gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(gesture));
      gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture), 
          GTK_PHASE_CAPTURE);

    }    
    
    static void addGestureClickDown3(GtkWidget *self, GObject *item, GridView<Type> *gridView_p){
      g_object_set_data(G_OBJECT(self), "item", item);
      auto gesture = gtk_gesture_click_new();
      gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),3); 
      // 3 for select
      TRACE("addGestureClickDown: self = %p, item=%p\n", self, item);
      g_signal_connect (G_OBJECT(gesture) , "pressed", EVENT_CALLBACK (down_f), (void *)gridView_p);
      gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(gesture));
      gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture), 
          GTK_PHASE_CAPTURE);

    }    
    
    static void addGestureClickUp(GtkWidget *box, GObject *object, GridView<Type> *gridView_p){
      TRACE("addGestureClickUp; object=%p\n", object);
      g_object_set_data(G_OBJECT(box), "gridView_p", gridView_p);
      auto gesture = gtk_gesture_click_new();
      gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),1); 
      // 1 for action released.
      g_signal_connect (G_OBJECT(gesture) , "released", EVENT_CALLBACK (gridView_p->gridViewClick_f()), (void *)object);
      gtk_widget_add_controller(GTK_WIDGET(box), GTK_EVENT_CONTROLLER(gesture));
      gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture), 
          GTK_PHASE_CAPTURE);
    }   

    static gboolean
    openMenu(GtkEventController *eventController, GridView<Type> *gridView_p, double y){
      // This is main context, gridview changes would also be main context.
      if (!Child::validGridView(gridView_p)) return true;
      auto d = (Dnd<LocalDir > *)g_object_get_data(G_OBJECT(Child::mainWidget()), "Dnd");
      d->dragOn(true);
      
      auto box = gtk_event_controller_get_widget(eventController);
      GObject *object = NULL;
      if (box) object = G_OBJECT(g_object_get_data(G_OBJECT(box), "object"));
     // unselect all, no

      //auto selectionModel = gridView_p->selectionModel();
      //gtk_selection_model_unselect_all(GTK_SELECTION_MODEL(selectionModel));
      
      //bug race with pdf preview: selectWidget(box, gridView_p);
      //auto size = gridView_p->getSelectionSize();
      //if (size > 0) return false;
      
      auto selectionModel = gridView_p->selectionModel();
      GtkBitset *bitset = gtk_selection_model_get_selection (GTK_SELECTION_MODEL(selectionModel));
      auto size = gtk_bitset_get_size(bitset);
      TRACE("***  size is %d object is %p\n", size, object);
      //if (size == 1) return true;
      GList *selectionList = gridView_p->getSelectionList();
#if 0
      // It seems this bug has been fixed in gtk-4.18.4
      // gdk_x11 stuff is now deprecated.
#ifdef GDK_WINDOWING_X11
        // Gtk bug workaround
        //  Gtk-WARNING **: Broken accounting of active state for widget 
        GdkDisplay *displayGdk = gdk_display_get_default();
        Display *display = gdk_x11_display_get_xdisplay(displayGdk);
        GtkNative *native = gtk_widget_get_native(Child::mainWidget());
        GdkSurface *surface = gtk_native_get_surface(native);
        Window src_w = gdk_x11_surface_get_xid (surface);
        unsigned int src_width = gtk_widget_get_width(Child::mainWidget());
        unsigned int src_height = gtk_widget_get_height(Child::mainWidget());
        int i = round(y);
        
        //XWarpPointer(display, src_w, None, 0, 0, 0, 0, src_width-i, 0);        
        //XWarpPointer(display, src_w, src_w, 0, 0, src_width, src_height, src_width, src_height/2);        
        XWarpPointer(display, src_w, src_w, 0, 0, src_width, src_height, 0, src_height/2);        
#endif
#endif


      if (size == 1 ) {
        if (object == NULL){
          //auto item = GTK_LIST_ITEM(g_list_first (selectionList)->data); // item is GTK_LIST_ITEM
          auto info = G_FILE_INFO(g_list_first (selectionList)->data);
          gridView_p->placeMenu(info, gridView_p); 
        } else {
          gridView_p->placeMenu(object, gridView_p); // object is G_FILE_INFO
        }
      } else if (size > 1 ){
        auto info = G_FILE_INFO(g_list_first (selectionList)->data);
        auto menuBox2 = GTK_WIDGET(g_object_get_data(G_OBJECT(info), "menuBox2"));
        gridView_p->placeMenu( menuBox2, gridView_p);
      } else { // null selection list
        MainWindow<Type>::clickMenu(mainMenuButton, NULL);
      }

      Basic::freeSelectionList(selectionList);
   
      return true;

    }

    public:
    static gboolean
    menu_f(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              void *data){
      auto gridView_p = (GridView<Type> *)data;
      
      auto eventController = GTK_EVENT_CONTROLLER(self);
      auto event = gtk_event_controller_get_current_event(eventController);

      auto modType = gdk_event_get_modifier_state(event);

      TRACE("modType = 0x%x\n", modType);
      //if (modType & GDK_CONTROL_MASK) return false;
      if (modType & ((GDK_SHIFT_MASK & GDK_MODIFIER_MASK))) return false;
      return openMenu(eventController, gridView_p, y);
    }
  
   static bool selectWidget(GtkWidget *w, GridView<Type> *gridView_p, bool unselectOthers){
      auto store = gridView_p->store();
      guint positionF;
      auto listItem = GTK_LIST_ITEM(g_object_get_data(G_OBJECT(w), "item"));
      auto item = G_FILE_INFO(gtk_list_item_get_item(listItem));
      TRACE("selectWidget: self(w) = %p, item=%p\n", w, item);
      if (item) {
        auto path = Basic::getPath(item);
        auto dirPath = g_path_get_dirname(path);
        int flags = Settings::getInteger("flags", dirPath); 
        if (flags < 0) flags = 0;
       
        TRACE("selectWidget: path=%s flags = 0x%x\n", dirPath, flags);
        g_free(dirPath);
        auto found = LocalDir::findPositionModel(store, path,  &positionF, flags);
        if (!found){
          DBG("selectWidget(): %s not found\n", path);
          g_free(path);
          return false;
        } 
        TRACE("Found %s at %d\n", path, positionF);
        g_free(path);
        auto selectionModel = gridView_p->selectionModel();
        gtk_selection_model_select_item(selectionModel, positionF, unselectOthers);
        gtk_widget_grab_focus(GTK_WIDGET(gridView_p->view()));
        return false;
      }
      TRACE("*** selectWidget: item is null\n");
      return false;
   }

   static bool 
   skipRename(GtkGestureLongPress* self,void *data){
      auto gridView_p = (GridView<Type> *)data;
      auto xffmRoot = g_object_get_data(G_OBJECT(gridView_p->store()), "xffm::root");
      auto xffmFstab = g_object_get_data(G_OBJECT(gridView_p->store()), "xffm::fstab");
      if (xffmRoot || xffmFstab) return true;
  /*    auto selectionModel = gridView_p->selectionModel();
      auto eventController = GTK_EVENT_CONTROLLER(self);
      auto event = gtk_event_controller_get_current_event(eventController);
      auto modifierType = gdk_event_get_modifier_state (event);
      if (modifierType & ((GDK_CONTROL_MASK & GDK_MODIFIER_MASK))) return true;
      if (modifierType & ((GDK_SHIFT_MASK & GDK_MODIFIER_MASK))) return true;*/
      return false;
   }

/*
   static bool 
   doSingleSelection(GtkGestureClick* self,void *data){
      auto w = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
      auto eventController = GTK_EVENT_CONTROLLER(self);
      auto event = gtk_event_controller_get_current_event(eventController);
      auto modType = gdk_event_get_modifier_state(event);
      auto gridView_p = (GridView<Type> *)data;
      
      if (modType & GDK_CONTROL_MASK) {
        auto xffmRoot = g_object_get_data(G_OBJECT(gridView_p->store()), "xffm::root");
        auto xffmFstab = g_object_get_data(G_OBJECT(gridView_p->store()), "xffm::fstab");
        bool unselectOthers = (xffmRoot || xffmFstab);
        if (unselectOthers) {
          DBG("unselectOthers yes\n");
          selectWidget(w, gridView_p, unselectOthers);
          return true;
        }
          DBG("unselectOthers no\n");
      }
      return false;
   }
*/
/*
   static gboolean
   box_f(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              void *data){
      TRACE("box_f \n");
      if (doSingleSelection(self, data)) return true;
      return false;
   }
*/
   static gboolean
   longPress_f(GtkGestureLongPress* self,
              gint n_press,
              gdouble x,
              gdouble y,
              void *data){
      auto gridView_p = (GridView<Type> *)data;
      auto currentSerial = Child::getSerial();
      if (longPressSerial != currentSerial){ // Invalid read of size 4
        TRACE("longPress_f(): Current serial mismatch %d != %d. Dropping longPress_f.\n", 
            currentSerial, longPressSerial);
        return true;
      }
      return openMenu(GTK_EVENT_CONTROLLER(self), gridView_p, y);
   }


   static gboolean
   rename_f(GtkGestureLongPress* self,
              gint n_press,
              gdouble x,
              gdouble y,
              void *data){
      //if (doSingleSelection(self, data)) return true;
      //auto button = gtk_gesture_single_get_button (GTK_GESTURE_SINGLE(self));
      //if (skipRename(self, data)) return true;
      if (skipRename(self, data)) return true;
      TRACE("rename_f \n");
      auto w = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
      auto item = GTK_LIST_ITEM(g_object_get_data(G_OBJECT(w), "item")); 
      auto info = G_FILE_INFO(gtk_list_item_get_item(item)); 
      auto path = Basic::getPath(info);
      dialogPath<mvResponse>::action(path);
      g_free(path);
      return true;
   }  

/*   static gboolean
   label_f(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              void *data){
      //if (doSingleSelection(self, data)) return true;
      auto button = gtk_gesture_single_get_button (GTK_GESTURE_SINGLE(self));
      if (skipRename(self, data)) return true;
      TRACE("label_f button %d n_press=%d\n", button, n_press);

      return true;
   }  */
  
   static gboolean
    down_f(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              void *data){
      auto button = gtk_gesture_single_get_button (GTK_GESTURE_SINGLE(self));
      TRACE("down_f button %d\n", button);
      auto w = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
      auto eventController = GTK_EVENT_CONTROLLER(self);
      auto event = gtk_event_controller_get_current_event(eventController);
      auto modType = gdk_event_get_modifier_state(event);
      auto gridView_p = (GridView<Type> *)data;
      longPressSerial = Child::getSerial();
      gridView_p->x(x);
      gridView_p->y(y);
      graphene_rect_t bounds;
      if (!gtk_widget_compute_bounds (w, Child::mainWidget(), &bounds)) {
        DBG("** Error:: down_f() should not happen. \n");
        return false;
      }
      gridView_p->X(x + bounds.origin.x);
      gridView_p->Y(y + bounds.origin.y);
      TRACE("down at widget %lf, %lf (origin %lf,%lf) mainWidget %lf, %lf \n", 
          x,y,bounds.origin.x,bounds.origin.y,
          gridView_p->X(),gridView_p->Y());

      auto d = (Dnd<LocalDir > *)g_object_get_data(G_OBJECT(Child::mainWidget()), "Dnd");

      if (button == 1){
        auto store = gridView_p->store();
        if (g_object_get_data(G_OBJECT(store), "xffm::root")){
          d->dragOn(true);
        } else {
          if (g_object_get_data(G_OBJECT(store), "xffm::fstab")){
            d->dragOn(true);
          } else d->dragOn(false);
        }
      } else {
        d->dragOn(true);
      }
//      if (doSingleSelection(self, data)) return true;
/*      if (modType & GDK_CONTROL_MASK) {
        auto xffmRoot = g_object_get_data(G_OBJECT(gridView_p->store()), "xffm::root");
        auto xffmFstab = g_object_get_data(G_OBJECT(gridView_p->store()), "xffm::fstab");
        bool unselectOthers = (xffmRoot || xffmFstab);
        selectWidget(w, gridView_p, unselectOthers);
        return false;
      }*/
      if (modType & ((GDK_SHIFT_MASK & GDK_MODIFIER_MASK))) {
        return false;
      }
  

      //gridView_p->dndOn = true;
      //TRACE("dnd %d\n", gridView_p->dndOn);
      //auto selectionModel = gridView_p->selectionModel();
      //gtk_selection_model_unselect_all(GTK_SELECTION_MODEL(selectionModel));

      if (modType & ((GDK_CONTROL_MASK & GDK_MODIFIER_MASK)) || button == 3) {
        // if button 3 and item is not selected then true, if item is selected then false
        if (button != 3) selectWidget(w, gridView_p, false);
      } else {
        selectWidget(w, gridView_p, true);
      }
      //return false; 
      return true;
    }

    static GdkPaintable *getCutTexture(GFileInfo *info, int size){
      auto texture = Texture<bool>::getShadedIcon(info, size, size, NULL);   
      auto paintable =Texture<bool>::addEmblem(texture, EMBLEM_CUT, size, size);
      g_object_unref(texture); // XXX currently the paintable is not hashed.
      return paintable;
    }




    static GtkWidget *emblemedImage(const char *name, const char *path, GFileInfo *info, int size){
      // Only for the hidden + backup items. Applies background mask.
      bool hidden = (name[0] == '.' && name[1] != '.');
      if (!hidden) hidden = g_file_info_get_is_hidden(info);


#if 0
      // check in  clipboard
      // Disabling because it may have caused the deadlock on 2025-02-06
      if(strcmp(name, "..") &&  clipboard_t::isCut(path)) {
        auto paintable = getCutTexture(info, size);
        auto image = gtk_image_new_from_paintable(paintable);
        g_object_unref(paintable); // XXX currently the paintable is not hashed.
        return GTK_WIDGET(image);
      }
#endif
#if 0
      // XXX This is too buggy, since copy emblem remains after pasted
      //     and no longer in copy clipboard. Disabling now.
      if(strcmp(name, "..") &&   clipboard_t::isCopy(path)) {
        auto texture = Texture<bool>::getShadedIcon(info, size, size, EMBLEM_COPY);   
        auto image = gtk_image_new_from_paintable(GDK_PAINTABLE(texture));
        g_object_unref(texture); // XXX currently the paintable is not hashed.
        return GTK_WIDGET(image);
      }
#endif
      bool backup = ( name[strlen(name)-1] == '~');
      if (!backup) backup = g_file_info_get_is_backup(info);
      if (hidden || backup )  {
          auto texture = Texture<bool>::getShadedIcon(info, size, size, backup?EMBLEM_BAK:NULL);   
          auto image = gtk_image_new_from_paintable(GDK_PAINTABLE(texture));
          g_object_unref(texture); // XXX currently the paintable is not hashed.
          return GTK_WIDGET(image);
        // Texture reference is kept in hashtable.
      }

      // My work source code
      GtkWidget *image = sourceCode(name, info, size);
      return image;

    }

    static GtkWidget *sourceCode(const char *name, GFileInfo *info, int size){
      GtkWidget *image = NULL;
      image = sourceCodeImageDirect(name, info, size);
      if (image) return image;
      
      const char *cSrc[] = {"c", "C", "cc", "CC", NULL};
      image = sourceCodeImage(name, info, size, EMBLEM_C, cSrc);
      if (image) return image;

      const char *fSrc[] = {"f", "F", "f90", "F90", "f95", "F95",NULL};
      image = sourceCodeImage(name, info, size, EMBLEM_F, fSrc);
      if (image) return image;

      const char *iSrc[] = {"i", "I",NULL};
      image = sourceCodeImage(name, info, size, EMBLEM_I, iSrc);
      if (image) return image;
      
      const char *hSrc[] = {"h", "H", "HH", "hh", NULL};
      image = sourceCodeImage(name, info, size, EMBLEM_H, hSrc);
      if (image) return image;
      
      const char *oSrc[] = {"o", "O", "obj", "OBJ", "dbg", "DBG", NULL};
      image = sourceCodeImage(name, info, size, EMBLEM_O, oSrc);
      if (image) return image;
      return NULL;
    }

    static GtkWidget *sourceCodeImageDirect(const char *name, GFileInfo *info, int size)
    {
      if (strncmp(name, "Makefile", strlen("Makefile"))==0){
          auto texture = Texture<bool>::addEmblem(info, EMBLEM_M, size, size);   
          auto image = gtk_image_new_from_paintable(GDK_PAINTABLE(texture));
          return image;
      }
      return NULL;
    }

    static GtkWidget *sourceCodeImage(const char *name, GFileInfo *info, int size, 
        const char *emblem, const char **src){
      auto extension = strrchr(name, '.');
      if (!extension) return NULL;
      for (auto p=src; p && *p; p++){
        if (extension+1 != NULL && strcmp(extension+1, *(p)) == 0){
          auto texture = Texture<bool>::addEmblem(info, emblem, size, size);   
          auto image = gtk_image_new_from_paintable(GDK_PAINTABLE(texture));
          return image;
        }
      }
      return NULL;
    }
    
    static GtkWidget *execImage(const char *name, GFileInfo *info, int size){

      return NULL;
    }
    static GtkWidget *previewImage(GFileInfo *info, const char *path, bool *doPreview_p){
      double scaleFactor = 1.;
      auto doPreview = Preview<bool>::doPreview(info);
      *doPreview_p = doPreview;
      if (doPreview) {
        // Try to load reference to texture from hash table.
        auto texture = Preview<bool>::readThumbnail(path);
        if (texture){
          // is it current?
          auto textureTime = Preview<bool>::thumbnailMtime(path);
          // does not work even with attribute set: g_file_info_get_modification_date_time
          struct stat st;
          stat(path, &st);
          if (st.st_mtime > textureTime){
            //Must do over
            return NULL;
          } else {TRACE("hash preview is OK %ld <= %ld\n", st.st_mtime, textureTime);}
          return GTK_WIDGET(gtk_image_new_from_paintable(GDK_PAINTABLE(texture)));
        }
      }
      return NULL;
    }
      
  };
}
#endif
