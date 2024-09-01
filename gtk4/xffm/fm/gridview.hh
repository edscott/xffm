#ifndef GRIDVIEW_HH
#define GRIDVIEW_HH
#include "texture.hh"
//#define GESTURECLICK_F(X) (gboolean (*)(GtkGestureClick *, gint, gdouble, gdouble, void *) X) 
namespace xf {
  class GridView  {
  private:
  public:
      /*GridView(void *gestureClick){
        gestureClick_ = gestureClick;
      }*/
      static GtkWidget *
      getGridView(const char *path, void *gridViewClick_f){
        GtkMultiSelection *selection_model = NULL;
        if (strcmp(path, "xffm:root")==0) {
          selection_model = rootSelectionModel();
        } else {
          // Create the initial GtkDirectoryList (G_LIST_MODEL).
          selection_model = xfSelectionModel(path);
          //selection_model = standardSelectionModel(path);     
        }
       
        // GtkListItemFactory implements GtkSignalListItemFactory, which can be connected to
        // bind, setup, teardown and unbind
        GtkListItemFactory *factory = gtk_signal_list_item_factory_new();

        /* Connect handler to the factory.
         */
        g_signal_connect( factory, "setup", G_CALLBACK(factorySetup), NULL );
        g_signal_connect( factory, "bind", G_CALLBACK(factoryBind), gridViewClick_f);

        GtkWidget *view;
        /* Create the view.
         */
        view = gtk_grid_view_new(GTK_SELECTION_MODEL(selection_model), factory);
        gtk_grid_view_set_max_columns(GTK_GRID_VIEW(view), 20);
        //gtk_grid_view_set_min_columns(GTK_GRID_VIEW(view), 10);
        gtk_widget_add_css_class(view, "gridviewColors");
        gtk_grid_view_set_enable_rubberband(GTK_GRID_VIEW(view), TRUE);
        return view;
      }

  private:
    
    static void addGestureClick(GtkWidget *imageBox, GObject *object, void *gridViewClick_f){
      TRACE("addGestureClick; object=%p\n", object);
      auto gesture = gtk_gesture_click_new();
      gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),1); 
      // 1 for action released, 3 for popover pressed
      // Add a different gtk_gesture_click_new for 3 and menu.
      g_signal_connect (G_OBJECT(gesture) , "pressed", EVENT_CALLBACK (gridViewClick_f), (void *)object);
//      g_signal_connect (G_OBJECT(gesture) , "pressed", EVENT_CALLBACK (gestureClick_), (void *)object);
//      g_signal_connect (G_OBJECT(gesture) , "released", EVENT_CALLBACK (gestureClick_), (void *)object);
      gtk_widget_add_controller(GTK_WIDGET(imageBox), GTK_EVENT_CONTROLLER(gesture));
      gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture), 
          GTK_PHASE_CAPTURE);
    }    

    /*    static gboolean
    gestureClick(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer object){

      auto eventController = GTK_EVENT_CONTROLLER(self);
      auto event = gtk_event_controller_get_current_event(eventController);
      
      auto modType = gdk_event_get_modifier_state(event);

      TRACE("modType = 0x%x\n", modType);
      if (modType & GDK_CONTROL_MASK) return FALSE;
      if (modType & GDK_SHIFT_MASK) return FALSE;
      
      TRACE("gestureClick; object=%p button=%d\n", object,
          gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(self)));

      auto info = G_FILE_INFO(gtk_list_item_get_item(GTK_LIST_ITEM(object)));
      auto file = G_FILE(g_file_info_get_attribute_object (info, "standard::file"));
      auto root = g_file_info_get_attribute_object (info, "xffm::root");
      if (root){
        Workdir::setWorkdir("xffm:root");
        return TRUE;
      }
      TRACE("gestureClick; file=%p\n", file);
      auto path = g_file_get_path(file);
      TRACE("gestureClick; path=%p\n", path);
      TRACE("click on %s\n", path);
      auto type = g_file_info_get_file_type(info);
      if ((type == G_FILE_TYPE_DIRECTORY )||(symlinkToDir(info, type))) {
        TRACE("Go to action...\n");
        auto child = UtilBasic::getCurrentChild();
        Workdir::setWorkdir(path);
      } else {
        DBG("mimetype action...\n");
      }
      g_free(path);
      return TRUE;
    }*/

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
      factoryBind(GtkSignalListItemFactory *self, GObject *object, void *gridViewClick_f)
      {
        auto list_item =GTK_LIST_ITEM(object);
        auto box = GTK_BOX(gtk_list_item_get_child( list_item ));
        auto list = UtilBasic::getChildren(box);
        for (auto l=list; l && l->data; l=l->next){
          gtk_widget_unparent(GTK_WIDGET(l->data));
        }
        
        auto info = G_FILE_INFO(gtk_list_item_get_item(list_item));
        auto file = G_FILE(g_file_info_get_attribute_object (info, "standard::file"));

        
        int scaleFactor = 1;
        char *name = g_strdup(g_file_info_get_name(info));
        auto path = g_file_get_path(file);
        auto texture = Texture::load(path);
        g_free(path);
        if (texture) scaleFactor = 2;
        if (!texture) {
          // FIXME: this only if not cairo modified ;
          texture = Texture::load(info);
        }
        // XXX: put all icons in hash (debug)
        Texture::findIconPath(info);
        
        /*if (!texture) {
            TRACE("Iconmview::load(): Texture::load(info) == NULL\n");
        }*/
          
        auto size = Settings::getInteger("xfterm", "iconsize");
        auto type = g_file_info_get_file_type(info);
     //   if ((type == G_FILE_TYPE_DIRECTORY )||(symlinkToDir(info, type))) {
        if (name[0] == '.' && name[1] != '.') {

          // We have texture...
          // What's the icon name?
          // We have 
          //   1. get icon names
          //   2. find icon source file in theme or
          //   3. find icon source file in search path
          //   4. must have backup in xffm+ icons.
          //   5. get the cairo surface
          //   6. apply mask
          //   7. render paintable
          //   8. return GtkImage
          //   9. For symlinks, add symlink emblem.
          //   10. For executables, add exe emblem.
          auto *iconPath = Texture::findIconPath(info);
          //if (!iconPath) iconPath = "/usr/share/icons/Adwaita/scalable/mimetypes/application-certificate.svg";
          if (iconPath) texture = Texture::getSvgPaintable(iconPath, size, size);   

        }
        GtkWidget *image = gtk_image_new_from_paintable(GDK_PAINTABLE(texture));
        if (type == G_FILE_TYPE_DIRECTORY ) {
          addDirectoryTooltip(image, info);
        }


        if (size < 0) size = 48;
        gtk_widget_set_size_request(image, size*scaleFactor, size*scaleFactor);

   

        //auto label = GTK_LABEL(g_object_get_data(G_OBJECT(vbox), "label"));

        if (name && strlen(name) > 15){
          name[15] = 0;
          name[14] ='~';
        }
        char *markup = g_strconcat("<span size=\"small\">", name, "</span>", NULL);
        auto label = gtk_label_new("");
        gtk_label_set_markup(GTK_LABEL(label), markup);
        g_free(markup);

        auto imageBox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_add_css_class(imageBox, "gridviewBox");
        auto labelBox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_add_css_class(labelBox, "gridviewBox");

        UtilBasic::boxPack0(GTK_BOX(imageBox), GTK_WIDGET(image), FALSE, FALSE, 0);    
        UtilBasic::boxPack0(GTK_BOX(box), imageBox, FALSE, FALSE, 0);    
        UtilBasic::boxPack0(GTK_BOX(labelBox), label, FALSE, FALSE, 0);    
        UtilBasic::boxPack0(GTK_BOX(box), labelBox, FALSE, FALSE, 0);    
        if (Settings::getInteger("xfterm", "iconsize") == 24 && strcmp(name, "..")){
          // file information string
          auto hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
          auto props = gtk_label_new("");
          char *markup = g_strconcat("<span size=\"small\" color=\"blue\">", "foo bar", "</span>", NULL);
          gtk_label_set_markup(GTK_LABEL(props), markup);
          g_free(markup);
          UtilBasic::boxPack0(GTK_BOX(hbox), props, FALSE, FALSE, 0);    
          UtilBasic::boxPack0(GTK_BOX(box), GTK_WIDGET(hbox), FALSE, FALSE, 0);    
        }
        g_free(name);
        addMotionController(labelBox);
        addMotionController(imageBox);
        addGestureClick(imageBox, object, gridViewClick_f);

/*
        char *markup = g_strconcat("<span size=\"small\">", name, "</span>", NULL);
        gtk_label_set_markup( GTK_LABEL( label ), name );
//        gtk_label_set_markup( GTK_LABEL( label ), markup );
        g_free(name);
        g_free(markup);*/
        
        // HACK
   /*     auto owner = gtk_widget_get_parent(GTK_WIDGET(vbox));
        graphene_rect_t bounds;
        if (!gtk_widget_compute_bounds(owner, MainWidget, &bounds)){
          DBG("Error: gtk_widget_compute_bounds\n");
        } else {
          DBG("owner=%p, width=%lf height=%lf\n", owner, bounds.size.width, bounds.size.height);
        }
        gtk_widget_set_size_request(owner, 24,24);
        gtk_widget_set_vexpand(GTK_WIDGET(owner), FALSE);
        gtk_widget_set_hexpand(GTK_WIDGET(owner), FALSE);
        gtk_widget_compute_bounds(owner, owner, &bounds);
          DBG("owner=%p, width=%lf height=%lf\n", owner, bounds.size.width, bounds.size.height);
*/
        

      }


      static GtkMultiSelection *getSelectionModel(GListModel *store){
        GtkFilter *filter = 
          GTK_FILTER(gtk_custom_filter_new ((GtkCustomFilterFunc)filterFunction, NULL, NULL));
        GtkFilterListModel *filterModel = gtk_filter_list_model_new(G_LIST_MODEL(store), filter);
        // Chain link GtkFilterListModel to a GtkSortListModel.
        // Directories first, and alphabeta.
        GtkSorter *sorter = 
          GTK_SORTER(gtk_custom_sorter_new((GCompareDataFunc)compareFunction, NULL, NULL));
        GtkSortListModel *sortModel = gtk_sort_list_model_new(G_LIST_MODEL(filterModel), sorter);
        GtkMultiSelection *s = gtk_multi_selection_new(G_LIST_MODEL(sortModel));
        g_signal_connect(G_OBJECT(s), "selection-changed", G_CALLBACK(selection_changed), NULL);
        return s;
      }

      static void selection_changed ( GtkSelectionModel* self,
            guint position,
            guint n_items,
            void *data) {
        DBG("selection changed position=%d, items=%d\n", position, n_items);
        if (gtk_selection_model_is_selected(self, 0)){
          gtk_selection_model_unselect_item(self, 0);
        }
        return;
      }

      
      static GtkMultiSelection *standardSelectionModel(const char *path){
        // This does not have the up icon 
        auto gfile = g_file_new_for_path(path);
        GtkDirectoryList *dList = 
//          gtk_directory_list_new("", gfile); 
          gtk_directory_list_new("standard::", gfile); 
        return getSelectionModel(G_LIST_MODEL(dList));

      }

      static GtkMultiSelection *xfSelectionModel(const char *path){
       // This section adds the up icon.

        auto up = g_path_get_dirname(path);
        TRACE("path=%s up=%s\n", path, up);
        auto store = g_list_store_new(G_TYPE_FILE_INFO);
        auto upFile = g_file_new_for_path(up);
        GError *error_ = NULL;
        auto info = g_file_query_info(upFile, "standard::", G_FILE_QUERY_INFO_NONE, NULL, &error_);
        if (strcmp(path, up)==0) {
          g_file_info_set_attribute_object(info, "xffm::root", G_OBJECT(upFile));
        }
        g_free(up);
        g_file_info_set_name(info, "..");
        g_file_info_set_icon(info, g_themed_icon_new("up"));
        g_list_store_insert(store, 0, G_OBJECT(info));
        g_file_info_set_attribute_object(info, "standard::file", G_OBJECT(upFile));

        GFile *file = g_file_new_for_path(path);
        GFileEnumerator *dirEnum = 
          g_file_enumerate_children (file,"standard::",G_FILE_QUERY_INFO_NONE,NULL, &error_);
        if (error_) {
          DBG("*** Error::g_file_enumerate_children: %s\n", error_->message);
          return NULL;
        }
        GFile *outChild = NULL;
        GFileInfo *outInfo = NULL;
        int k = 1;
        do {
          g_file_enumerator_iterate (dirEnum, &outInfo, &outChild,
              NULL, // GCancellable* cancellable,
              &error_);
          if (error_) {
            DBG("*** Error::g_file_enumerator_iterate: %s\n", error_->message);
            return NULL;
          }
          if (!outInfo || !outChild) break;
          g_file_info_set_attribute_object(outInfo, "standard::file", G_OBJECT(outChild));
          TRACE("insert path (%s)\n", g_file_get_path(outChild));
          g_list_store_insert(store, k++, G_OBJECT(outInfo));
        } while (true);
        return getSelectionModel(G_LIST_MODEL(store));
      }

      static GtkMultiSelection *rootSelectionModel(void){
        GError *error_ = NULL;
        Bookmarks::initBookmarks();
        auto store = g_list_store_new(G_TYPE_FILE_INFO);
        auto list = Bookmarks::bookmarksList();
        for (auto l=list; l && l->data; l=l->next){
          auto p = (bookmarkItem_t *)l->data;
          if (!p->path) continue;
          DBG("adding bookmark %p -> %s\n", p, p->path);
          if (!g_path_is_absolute(p->path)) continue;
          if (!g_file_test(p->path, G_FILE_TEST_EXISTS)) {
              DBG("Bookmark %s does not exist\n", p->path);
              continue;
          }
          GFile *file = g_file_new_for_path(p->path);
          auto info = g_file_query_info(file, "standard::", G_FILE_QUERY_INFO_NONE, NULL, &error_);
          auto basename = g_path_get_basename(p->path);
          auto utf_name = UtilBasic::utf_string(basename);
          g_file_info_set_name(info, utf_name);
          g_free(basename);
          g_free(utf_name);
          g_file_info_set_icon(info, g_themed_icon_new(EMBLEM_BOOKMARK));
          g_list_store_insert(store, 0, G_OBJECT(info));
          g_file_info_set_attribute_object(info, "standard::file", G_OBJECT(file));          
        }
        return getSelectionModel(G_LIST_MODEL(store));
      }

    private:
      static gboolean
      filterFunction(GObject *object, void *data){
        GFileInfo *info = G_FILE_INFO(object);
        return TRUE;
        if (strcmp(g_file_info_get_name(info), "..")==0) return TRUE;
        return !g_file_info_get_is_hidden(info);
      }
      static
      void addMotionController(GtkWidget  *widget){
        auto controller = gtk_event_controller_motion_new();
        gtk_event_controller_set_propagation_phase(controller, GTK_PHASE_CAPTURE);
        gtk_widget_add_controller(GTK_WIDGET(widget), controller);
        g_signal_connect (G_OBJECT (controller), "enter", 
            G_CALLBACK (negative), NULL);
        g_signal_connect (G_OBJECT (controller), "leave", 
            G_CALLBACK (positive), NULL);
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
        UtilBasic::flushGTK();
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
        UtilBasic::flushGTK();
        return FALSE;
    }

    
    public:
      static bool
      symlinkToDir(GFileInfo* info, GFileType type){
        if (type == G_FILE_TYPE_SYMBOLIC_LINK){ 
          const char *path = g_file_info_get_symlink_target(info);
          struct stat st;
          stat(path, &st);
          if (S_ISDIR(st.st_mode)) return true;
        }
        return false;
      }
    private:

    // flags :
    // 0x01 : by date
    // 0x02 : by size
    // 0x04 : descending
    static gint 
    compareFunction(const void *a, const void *b, void *data){
        auto flags = GPOINTER_TO_INT(data);
        bool byDate = (flags & 0x01);
        bool bySize = (flags & 0x02);
        bool descending = (flags & 0x04);

        GFileInfo *infoA = G_FILE_INFO(a);
        GFileInfo *infoB = G_FILE_INFO(b);
        auto typeA = g_file_info_get_file_type(infoA);
        auto typeB = g_file_info_get_file_type(infoB);

        if (strcmp(g_file_info_get_name(infoA), "..")==0) return -1;
        if (strcmp(g_file_info_get_name(infoB), "..")==0) return 1;
        
        GFile *fileA = G_FILE(g_file_info_get_attribute_object(infoA, "standard::file"));
        GFile *fileB = G_FILE(g_file_info_get_attribute_object(infoB, "standard::file"));

        // compare by name, directories or symlinks to directories on top
        TRACE("compare %s --- %s\n", g_file_info_get_name(infoA), g_file_info_get_name(infoB));
        //XXX ".." is not a part of the dList...
        //if (strcmp(xd_a->d_name, "..")==0) return -1;
        //if (strcmp(xd_b->d_name, "..")==0) return 1;

        gboolean a_cond = FALSE;
        gboolean b_cond = FALSE;

        a_cond = ((typeA == G_FILE_TYPE_DIRECTORY )||(symlinkToDir(infoA, typeA)));
        b_cond = ((typeB == G_FILE_TYPE_DIRECTORY )||(symlinkToDir(infoB, typeB)));

        if (a_cond && !b_cond) return -1; 
        if (!a_cond && b_cond) return 1;

        auto nameA = g_file_info_get_name(infoA);
        auto nameB = g_file_info_get_name(infoB);
        if (a_cond && b_cond) {
            // directory comparison by name is default;
           if (byDate) {
              auto dateTimeA = g_file_info_get_modification_date_time(infoA);
              auto dateTimeB = g_file_info_get_modification_date_time(infoB);
              auto value = g_date_time_compare(dateTimeA, dateTimeB);
              g_free(dateTimeA);
              g_free(dateTimeB);
              return value;
           } else {
                if (descending) return -strcasecmp(nameA, nameB);
                return strcasecmp(nameA, nameB);
            }
        }
        // by date
        if (byDate){
          auto dateTimeA = g_file_info_get_modification_date_time(infoA);
          auto dateTimeB = g_file_info_get_modification_date_time(infoB);
          auto value = g_date_time_compare(dateTimeA, dateTimeB);
          g_free(dateTimeA);
          g_free(dateTimeB);
          if (descending) return -value;
          return value;
        } else if (bySize){
          auto sizeA = g_file_info_get_size(infoA);
          auto sizeB = g_file_info_get_size(infoB);
          if (descending) return sizeB - sizeA;
          return sizeA - sizeB;
        } 
        // by name 
        if (descending) return -strcasecmp(nameA, nameB);
        return strcasecmp(nameA, nameB);
    }
    
    static void
    addDirectoryTooltip(GtkWidget *image, GFileInfo *info){
      auto name = g_file_info_get_name(info);
      
      auto file = G_FILE(g_file_info_get_attribute_object(info, "standard::file"));
      GError *error_ = NULL;
      GFileEnumerator *dirEnum = 
          g_file_enumerate_children (file,"standard::name,standard::type",G_FILE_QUERY_INFO_NONE,NULL, &error_);
      if (error_){
        gtk_widget_set_tooltip_markup(image, error_->message);
        g_error_free(error_);
      } else {
        int limit = 20;
        int k=0;
        char *t = g_strdup_printf(_("Contents of '%s'"), name);
//            char *t = g_strdup_printf("%s:",_("contents"));
        UtilBasic::concat(&t, ":\n");
        do {
          GFile *outChild = NULL;
          GFileInfo *outInfo = NULL;
          g_file_enumerator_iterate (dirEnum, &outInfo, &outChild,
              NULL, // GCancellable* cancellable,
              &error_);
          if (error_) {
            DBG("*** Error::g_file_enumerator_iterate: %s\n", error_->message);
            g_error_free(error_);
          }
          if (!outInfo || !outChild) break;
          UtilBasic::concat(&t, "<span color=\"blue\" size=\"x-small\">  ");
          UtilBasic::concat(&t, g_file_info_get_name(outInfo));
          auto subtype = g_file_info_get_file_type(outInfo);
          if (subtype == G_FILE_TYPE_DIRECTORY )UtilBasic::concat(&t, "/");
          if (subtype == G_FILE_TYPE_SYMBOLIC_LINK)UtilBasic::concat(&t, "*");
          UtilBasic::concat(&t, "</span>\n");
          if (++k >= limit) {
            UtilBasic::concat(&t, "<span color=\"red\">");
            UtilBasic::concat(&t, _("More..."));
            UtilBasic::concat(&t, "</span>\n");
            break;
          }  
        } while (true);
        g_file_enumerator_close(dirEnum, NULL, NULL);
        gtk_widget_set_tooltip_markup(image, t);
        g_free(t);
      }
    }


  };
}
#endif
