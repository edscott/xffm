  
#ifndef FACTORYLOCAL_HH
#define FACTORYLOCAL_HH
// object is list_item
// item_get_item is GFileInfo
// item_get child is GtkWidget
namespace xf {
template <class DirectoryClass>
  class Factory {
    public:
      static void
      factoryTeardown(GtkSignalListItemFactory *self, GObject *object, GridView<DirectoryClass> *gridView_p){
        TRACE("factoryTeardown...\n");
        GtkListItem *list_item = GTK_LIST_ITEM(object);
        GtkBox *vbox = GTK_BOX(gtk_list_item_get_child( list_item ));
        // vbox is null on new updateGridview() (new Gridview).
        // unparent() also fails on close xffm...
        // if (vbox && GTK_IS_WIDGET(vbox)) gtk_widget_unparent(GTK_WIDGET(vbox));
        gtk_list_item_set_child(list_item, NULL);
     }

      static void
      factorySetup(GtkSignalListItemFactory *self, GObject *object, GridView<DirectoryClass> *gridView_p){
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
        
        Basic::addMotionController(labelBox);
        Basic::addMotionController(hlabelBox);

        Basic::addMotionController(imageBox);
        addGestureClickDown(imageBox, object, gridView_p);
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
      factoryUnbind(GtkSignalListItemFactory *self, GObject *object, GridView<DirectoryClass> *gridView_p){
        TRACE("factoryUnbind...\n");
        //THREADPOOL->clear();
        //Child::incrementSerial();
     
         // cleanup on regen:
        auto imageBox = GTK_BOX(g_object_get_data(object, "imageBox"));
        auto oldImage = gtk_widget_get_first_child(GTK_WIDGET(imageBox));
        if (oldImage) gtk_widget_unparent(oldImage);
     }


      /* The bind function for the factory */
      static void
      factoryBind(GtkSignalListItemFactory *factory, GObject *object, GridView<DirectoryClass> *gridView_p)
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
        auto size = Settings::getInteger("xfterm", "iconsize");
        // allocated:
        auto path = Basic::getPath(info);       
        char *name = g_strdup(g_file_info_get_name(info));

        TRACE("factory bind name= %s\n", name);

        if (size < 0) size = 48;
        double scaleFactor = 1.0;
        GdkPaintable *texture = NULL;
        GtkWidget *image = NULL;
        image = backupImage(name, info, size);
        //if (!image) image = emblemedImage(name, info, size);

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
            auto emblem = "emblem-symbolic-link";
            struct stat st;
            if (stat(path, &st) < 0) emblem = "emblem-broken";
            texture = Texture<bool>::addEmblem(info,  emblem, scaleFactor*size, scaleFactor*size);
          } else { // simple
            auto gfiletype= g_file_info_get_file_type (info);
            switch (gfiletype){
              default:
                texture = Texture<bool>::load(info); // Loads icon from icontheme.
                break;
              case (G_FILE_TYPE_MOUNTABLE): // does not work FIXME: use fstab routines
                {
                  bool isMounted = false; // FIXME with fstab routine
                  texture = Texture<bool>::addEmblem(info,  
                      isMounted?"emblem-greenball":"emblem-blueball", 
                      scaleFactor*size, scaleFactor*size);
                }
                break;
            }
          }
          image = gtk_image_new_from_paintable(GDK_PAINTABLE(texture));
          // Texture is not referenced in hash table. 
          g_object_unref(texture);        
        }
        if (image) gtk_widget_set_size_request(image, size*scaleFactor, size*scaleFactor);
        else {DBG("Should not happen: image is NULL\n");}
        // cleanup on regen:
        //auto oldImage = gtk_widget_get_first_child(GTK_WIDGET(imageBox));
        //if (oldImage) gtk_widget_unparent(oldImage);
        Basic::boxPack0(GTK_BOX(imageBox), GTK_WIDGET(image), FALSE, FALSE, 0);    
        

        if (type == G_FILE_TYPE_DIRECTORY ) {
          // not much use, really.
          //DirectoryClass::addDirectoryTooltip(image, info);
        }

        char buffer[128];
        if (size == 24){
          gtk_widget_set_visible(GTK_WIDGET(label), false);
          gtk_widget_set_visible(GTK_WIDGET(hlabel), true);
          auto format = g_strdup_printf("%%-%ds", gridView_p->maxNameLen());
          TRACE("size 24, format=%s\n", format);
          snprintf(buffer, 128, (const char *)format, name);

          auto markup = g_strdup_printf("<span size=\"%s\">%s</span>",  "x-small", buffer);

          //gtk_label_set_markup(label, markup);
          //g_free(markup);

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
          const char *n_p = name;
          if (strlen(name) > 13) {
            char *b=strdup("");
            do {
              char part[14];
              memset(part, 0, 14);
              strncpy(part, n_p, 13);
              part[13] = 0;
              Basic::concat(&b, part);
              if (n_p[13] != 0) Basic::concat(&b, "<span color=\"red\">-</span>\n");
              else break;
              n_p += 13;
            } while (strlen(n_p)>13);
            Basic::concat(&b, n_p);

            snprintf(buffer, 128, "%s", b);
            g_free(b);
          } else snprintf(buffer, 128, "%s", name);
        
          const char *sizeS = "x-small";
          if (size <= 96) sizeS = "small";
          else if (size <= 156) sizeS = "medium";
          else if (size <= 192) sizeS = "large";
          else sizeS = "x-large";
          auto markup = g_strdup_printf("<span size=\"%s\">%s</span>", sizeS, buffer);
          gtk_label_set_markup(label, markup);
          g_free(markup);
          DirectoryClass::addLabelTooltip(GTK_WIDGET(label), path);
        }
        // gtk drag
        GtkDragSource *source = gtk_drag_source_new ();
        g_signal_connect (source, "prepare", G_CALLBACK (Dnd<DirectoryClass>::image_drag_prepare),gridView_p);
        
        g_signal_connect (source, "drag-begin", G_CALLBACK (Dnd<DirectoryClass>::image_drag_begin), image);
        gtk_widget_add_controller (image, GTK_EVENT_CONTROLLER (source));
        


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

    private:
    
    static void addGestureClickMenu(GtkWidget *box, GObject *object, GridView<DirectoryClass> *gridView_p){
      TRACE("addGestureClickMenu; object=%p\n", gridView_p);
      auto gesture = gtk_gesture_click_new();
      gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),3); 
      // 3 for popover pressed
      g_signal_connect (G_OBJECT(gesture) , "released", EVENT_CALLBACK (menu_f), (void *)gridView_p);
      gtk_widget_add_controller(GTK_WIDGET(box), GTK_EVENT_CONTROLLER(gesture));
      gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture), 
          GTK_PHASE_CAPTURE);

    }    
    
    static void addGestureClickDown(GtkWidget *self, GObject *item, GridView<DirectoryClass> *gridView_p){
      g_object_set_data(G_OBJECT(self), "item", item);
      auto gesture = gtk_gesture_click_new();
      gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),1); 
      // 1 for select
      TRACE("addGestureClickDown: self = %p, item=%p\n", self, item);
      g_signal_connect (G_OBJECT(gesture) , "pressed", EVENT_CALLBACK (down_f), (void *)gridView_p);
      gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(gesture));
      gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture), 
          GTK_PHASE_BUBBLE);

    }    
    
    static void addGestureClickDown3(GtkWidget *self, GObject *item, GridView<DirectoryClass> *gridView_p){
      g_object_set_data(G_OBJECT(self), "item", item);
      auto gesture = gtk_gesture_click_new();
      gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),3); 
      // 3 for select
      TRACE("addGestureClickDown: self = %p, item=%p\n", self, item);
      g_signal_connect (G_OBJECT(gesture) , "pressed", EVENT_CALLBACK (down_f), (void *)gridView_p);
      gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(gesture));
      gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture), 
          GTK_PHASE_BUBBLE);

    }    
    
    static void addGestureClickUp(GtkWidget *box, GObject *object, GridView<DirectoryClass> *gridView_p){
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
    public:
    static gboolean
    menu_f(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              void *data){
      auto gridView_p = (GridView<DirectoryClass> *)data;
      
      auto eventController = GTK_EVENT_CONTROLLER(self);
      auto event = gtk_event_controller_get_current_event(eventController);
      auto box = gtk_event_controller_get_widget(eventController);
      GObject *object = NULL;
      if (box) object = G_OBJECT(g_object_get_data(G_OBJECT(box), "object"));

      auto modType = gdk_event_get_modifier_state(event);

      TRACE("modType = 0x%x\n", modType);
      //if (modType & GDK_CONTROL_MASK) return false;
      if (modType & GDK_SHIFT_MASK) return false;

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
      if (size == 1 ) {
        if (object == NULL){
          //auto item = GTK_LIST_ITEM(g_list_first (selectionList)->data); // item is GTK_LIST_ITEM
          auto info = G_FILE_INFO(g_list_first (selectionList)->data);
          gridView_p->placeMenu(info, gridView_p); 
        } else {
          gridView_p->placeMenu(object, gridView_p); // object is G_FILE_INFO
        }
      } else {
#ifdef GDK_WINDOWING_X11
        // Gtk bug workaround
        GdkDisplay *displayGdk = gdk_display_get_default();
        Display *display = gdk_x11_display_get_xdisplay(displayGdk);
        GtkNative *native = gtk_widget_get_native(MainWidget);
        GdkSurface *surface = gtk_native_get_surface(native);
        Window src_w = gdk_x11_surface_get_xid (surface);
        unsigned int src_width = gtk_widget_get_width(MainWidget);
        int i = round(x);
        XWarpPointer(display, src_w, None, 0, 0, 0, 0, src_width-i, 0);        
#endif
        auto info = G_FILE_INFO(g_list_first (selectionList)->data);
        auto menuBox2 = GTK_WIDGET(g_object_get_data(G_OBJECT(info), "menuBox2"));
        gridView_p->placeMenu( menuBox2, gridView_p);
      }
      Basic::freeSelectionList(selectionList);
   
      return true;
    }
  
   static bool selectWidget(GtkWidget *w, GridView<DirectoryClass> *gridView_p, bool unselectOthers){
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
       
        TRACE("selectWidget: path= %s flags = 0x%x\n", dirPath, flags);
        g_free(dirPath);
        auto found = LocalDir::findPositionModel(store, path,  &positionF, flags);
        if (!found){
          TRACE("gridViewClick(): %s not found\n", path);
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

   static gboolean
    down_f(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              void *data){
      auto w = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
      auto eventController = GTK_EVENT_CONTROLLER(self);
      auto event = gtk_event_controller_get_current_event(eventController);
      auto modType = gdk_event_get_modifier_state(event);
      auto gridView_p = (GridView<DirectoryClass> *)data;
      gridView_p->x(x);
      gridView_p->y(y);
      if (modType & GDK_CONTROL_MASK) {
        selectWidget(w, gridView_p, false);
        return false;
      }
      if (modType & GDK_SHIFT_MASK) {
        return false;
      }
  

      //gridView_p->dndOn = true;
      //TRACE("dnd %d\n", gridView_p->dndOn);
      //auto selectionModel = gridView_p->selectionModel();
      //gtk_selection_model_unselect_all(GTK_SELECTION_MODEL(selectionModel));
      selectWidget(w, gridView_p, true);
      //return false; 
      return true;
    }
    static GtkWidget *backupImage(const char *name, GFileInfo *info, int size){
      // Only for the hidden + backup items. Applies background mask.
      bool hidden = (name[0] == '.' && name[1] != '.');
      if (!hidden) hidden = g_file_info_get_is_hidden(info);
      bool backup = ( name[strlen(name)-1] == '~');
      if (!backup) backup = g_file_info_get_is_backup(info);
      if (hidden || backup )  {
        auto *iconPath = Texture<bool>::findIconPath(info);
        // Only for the hidden + backup items. Applies background mask.
        if (iconPath){
          auto texture = Texture<bool>::getShadedIcon2(iconPath, size, size, backup?"emblem-bak":NULL);   
          auto image = gtk_image_new_from_paintable(GDK_PAINTABLE(texture));
          g_object_unref(texture); // XXX currently the paintable is not hashed.
          return GTK_WIDGET(image);
        }
        // Texture reference is kept in hashtable.
      }
      return NULL;
    }
    static GtkWidget *execImage(const char *name, GFileInfo *info, int size){
   /*   bool exec = g_file_info_get_is_backup(info);
      if (hidden || backup )  {
        auto *iconPath = Texture<bool>::findIconPath(info);
        // Only for the hidden + backup items. Applies background mask.
        if (iconPath){
          auto texture = Texture<bool>::getShadedIcon2(iconPath, size, size, backup?"emblem-bak":NULL);   
          auto image = gtk_image_new_from_paintable(GDK_PAINTABLE(texture));
          g_object_unref(texture); // XXX currently the paintable is not hashed.
          return GTK_WIDGET(image);
        }
        // Texture reference is kept in hashtable.
      }*/
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
