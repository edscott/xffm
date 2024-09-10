#ifndef GRIDVIEW_HH
#define GRIDVIEW_HH
#include "texture.hh"
//#define GESTURECLICK_F(X) (gboolean (*)(GtkGestureClick *, gint, gdouble, gdouble, void *) X) 
namespace xf {
  template <class DirectoryClass>
  class GridView  {
  private:
  public:
      /*GridView(void *gestureClick){
        gestureClick_ = gestureClick;
      }*/
      static GtkWidget *
      getGridView(const char *path, void *gridViewClick_f){
        auto child = Child::getChild();
        GtkMultiSelection *selection_model = NULL;
        if (strcmp(path, "xffm:root")==0) {
          selection_model = DirectoryClass::rootSelectionModel();
        } else {
          // Create the initial GtkDirectoryList (G_LIST_MODEL).
          selection_model = DirectoryClass::xfSelectionModel(path);
          //selection_model = DirectoryClass::standardSelectionModel(path);     
        }
       
        // GtkListItemFactory implements GtkSignalListItemFactory, which can be connected to
        // bind, setup, teardown and unbind
        GtkListItemFactory *factory = gtk_signal_list_item_factory_new();
        g_object_set_data(G_OBJECT(factory), "child", child);

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
      factoryBind(GtkSignalListItemFactory *factory, GObject *object, void *gridViewClick_f)
      {
        auto child = GTK_WIDGET(g_object_get_data(G_OBJECT(factory), "child"));
        auto list_item =GTK_LIST_ITEM(object);
        auto box = GTK_BOX(gtk_list_item_get_child( list_item ));
        auto list = UtilBasic::getChildren(box);
        for (auto l=list; l && l->data; l=l->next){
          gtk_widget_unparent(GTK_WIDGET(l->data));
        }

        auto imageBox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        g_object_set_data(G_OBJECT(box), "imageBox", imageBox);
        gtk_widget_add_css_class(imageBox, "gridviewBox");

        auto labelBox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_add_css_class(labelBox, "gridviewBox");
        g_object_set_data(G_OBJECT(box), "labelBox", labelBox);
        
        auto info = G_FILE_INFO(gtk_list_item_get_item(list_item));
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
        //Texture::findIconPath(info);
        
        /*if (!texture) {
            TRACE("Iconmview::load(): Texture::load(info) == NULL\n");
        }*/
          
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

        UtilBasic::boxPack0(GTK_BOX(imageBox), GTK_WIDGET(image), FALSE, FALSE, 0);    
        UtilBasic::boxPack0(GTK_BOX(labelBox), label, FALSE, FALSE, 0);    
        UtilBasic::boxPack0(GTK_BOX(box), imageBox, FALSE, FALSE, 0);    
        UtilBasic::boxPack0(GTK_BOX(box), labelBox, FALSE, FALSE, 0);    

        if (Settings::getInteger("xfterm", "iconsize") == 24 && strcmp(name, "..")){
          // file information string
          auto hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
          gtk_widget_add_css_class(hbox, "gridviewBox");
          auto props = gtk_label_new("");

          struct stat st;
          lstat(path, &st);
       
          auto m1 = UtilBasic::statInfo(&st);

          char *markup = g_strdup("<span size=\"small\" color=\"blue\">");
          Print::concat(&markup, m1);
          Print::concat(&markup, "</span>");
          
          g_free(m1);
          gtk_label_set_markup(GTK_LABEL(props), markup);
          g_free(markup);

          UtilBasic::boxPack0(GTK_BOX(box), props, FALSE, FALSE, 0);  
          UtilBasic::boxPack0(GTK_BOX(box), GTK_WIDGET(hbox), FALSE, TRUE, 0);    


        } else {
        }
        g_free(name);
        addMotionController(labelBox);
        addMotionController(imageBox);
        addGestureClick(imageBox, object, gridViewClick_f);

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

    
    private:

  };
}
#endif
