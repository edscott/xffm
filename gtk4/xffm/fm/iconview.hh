#ifndef XF_ICONVIEW_HH
#define XF_ICONVIEW_HH
#include "texture.hh"
namespace xf {
    
  class IconView {
    private:

    public:
      static void
      updateGridView(GtkWidget *child, const char *path){
        auto topScrolledWindow = GTK_SCROLLED_WINDOW(g_object_get_data(G_OBJECT(child), "topScrolledWindow"));
        //auto old = gtk_scrolled_window_get_child(topScrolledWindow);
        //if (old && GTK_IS_WIDGET(old)) gtk_widget_unparent(old);
        auto view = getGridView(path);
        gtk_scrolled_window_set_child(topScrolledWindow, view);

      }
      static GtkWidget *
      getGridView(const char *path){
        GFile *gfile = g_file_new_for_path(path);
        // Create the initial GtkDirectoryList (G_LIST_MODEL).
        const char *attributes[]={
          "type",
          "symbolic_icon",
          "is_backup",
          "is-hidden",
          "is-symlink",
          "time_modified",
          "name",
          "size",
          "sort_order",
          "icon",
          "symbolic_icon",
          "symlink_target",
          NULL};
        auto attribute = g_strdup("");
        bool first = true;
        for (const char **p=attributes; p && *p; p++){
          if (!first){
            UtilBasic::concat(&attribute, ",");
          }
          first = false;
          UtilBasic::concat(&attribute, "standard::");
          UtilBasic::concat(&attribute, *p);
        }
        GtkDirectoryList *dList = 
          gtk_directory_list_new(attribute, gfile); 
        
        // XXX insert test... does not work 
        //    invalid cast from GtkDirectoryList to G_LIST_STORE
    /*    GFile *home = g_file_new_for_path(g_get_home_dir());
        GFileInfo *info = g_file_query_info (home, "standard::", G_FILE_QUERY_INFO_NONE, NULL, NULL);
        GListModel *model = G_LIST_MODEL(dList);
        g_list_store_insert (G_LIST_STORE (model), 0, info);*/
       
        //g_free(attribute); 
        // Chain link GtkDirectoryList to a GtkFilterListModel.
        GtkFilter *filter = 
          GTK_FILTER(gtk_custom_filter_new ((GtkCustomFilterFunc)filterFunction, NULL, NULL));
        GtkFilterListModel *filterModel = gtk_filter_list_model_new(G_LIST_MODEL(dList), filter);
        // Chain link GtkFilterListModel to a GtkSortListModel.
        // Directories first, and alphabeta.
        GtkSorter *sorter = 
          GTK_SORTER(gtk_custom_sorter_new((GCompareDataFunc)compareFunction, NULL, NULL));
        GtkSortListModel *sortModel = gtk_sort_list_model_new(G_LIST_MODEL(filterModel), sorter);

        // Chain link GtkFilterListModel to a GtkSortListModel.
        //GtkSorter *sorter = 
          //GTK_SORTER(gtk_custom_sorter_new((GCompareDataFunc)compareFunction, NULL, NULL));
        //GtkSortListModel *sortModel = gtk_sort_list_model_new(G_LIST_MODEL(filterModel), sorter);
        
        // Chain link GtkSortListModel to a GtkMultiSelection.
        GtkMultiSelection *selection_model = gtk_multi_selection_new(G_LIST_MODEL(sortModel));

/*        
        while (gtk_directory_list_is_loading(dList)) {
          DBG("gtk_directory_list_is_loading...\n");
           while (g_main_context_pending(NULL)) g_main_context_iteration(NULL, TRUE);
          //Util::flushGTK();
        }
        auto num = g_list_model_get_n_items(G_LIST_MODEL(dList));
        DBG("gtk_directory_list_is_loading done: items=%d\n", num);
        for (int i=0; i<num; i++){
          auto info = G_FILE_INFO(g_list_model_get_item(G_LIST_MODEL(dList), i));
        }*/
        
       
        // GtkListItemFactory implements GtkSignalListItemFactory, which can be connected to
        // bind, setup, teardown and unbind
        GtkListItemFactory *factory = gtk_signal_list_item_factory_new();

        /* Connect handler to the factory.
         */
        g_signal_connect( factory, "setup", G_CALLBACK(factorySetup), NULL );
        g_signal_connect( factory, "bind", G_CALLBACK(factoryBind), NULL);

        GtkWidget *view;
        /* Create the view.
         */
        view = gtk_grid_view_new(GTK_SELECTION_MODEL(selection_model), factory);
        gtk_widget_add_css_class(view, "xficons");
        gtk_grid_view_set_enable_rubberband(GTK_GRID_VIEW(view), TRUE);
        return view;
      }
    private:
      static gboolean
      filterFunction(GObject *object, void *data){
        GFileInfo *info = G_FILE_INFO(object);
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
      
      static void
      factorySetup(GtkSignalListItemFactory *self, GObject *object, void *data){
        GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
        GtkWidget *label = gtk_label_new( "" );
        GtkWidget *imageBox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
        addMotionController(imageBox);

        //GtkWidget *image = gtk_image_new_from_icon_name("text-x-generic");
            
        gtk_box_append(GTK_BOX(vbox), imageBox);
        g_object_set_data(G_OBJECT(vbox), "imageBox", imageBox);

        gtk_box_append(GTK_BOX(vbox), label);
        gtk_widget_set_halign (label,GTK_ALIGN_FILL);
        gtk_widget_set_vexpand(GTK_WIDGET(label), FALSE);
        gtk_widget_set_margin_top(GTK_WIDGET(label), 0);
        gtk_widget_set_margin_bottom(GTK_WIDGET(label), 0);

        g_object_set_data(G_OBJECT(vbox),"label", label);

        GtkListItem *list_item = GTK_LIST_ITEM(object);
        gtk_list_item_set_child(list_item, vbox);
      }

      /* The bind function for the factory */
      static void
      factoryBind(GtkSignalListItemFactory *self, GObject *object, void *data)
      {
        auto list_item =GTK_LIST_ITEM(object);
        auto vbox = GTK_BOX(gtk_list_item_get_child( list_item ));
        auto info = G_FILE_INFO(gtk_list_item_get_item(list_item));

       /* does not work:
        * GFile *gfile = g_file_enumerator_get_container(G_FILE_ENUMERATOR(info));
        auto path = g_file_get_path(gfile);
        DBG("gfile path: %s\n",path);
        g_free(path);*/
       
        GtkWidget *imageBox = GTK_WIDGET(g_object_get_data(G_OBJECT(vbox), "imageBox"));
        auto w = gtk_widget_get_first_child (imageBox);
        if (w) gtk_widget_unparent(w);
        
        int scaleFactor = 1;
        char *name = g_strdup(g_file_info_get_name(info));
        auto texture = Texture::load(name);
        if (texture) scaleFactor = 2;
        if (!texture) {
          //texture = Texture::loadIconName("emblem-archlinux");
          texture = Texture::load(info);
        }
        if (!texture) {
            TRACE("Iconmview::load(): Texture::load(info) == NULL\n");
        }
          
        
        GtkWidget *image = gtk_image_new_from_paintable(GDK_PAINTABLE(texture));
        auto size = Settings::getInteger("xfterm", "iconsize");
        if (size < 0) size = 48;
        gtk_widget_set_size_request(image, size*scaleFactor, size*scaleFactor);
        //gtk_widget_set_sensitive(GTK_WIDGET(image), FALSE);
        // no go: gtk_widget_add_css_class(GTK_WIDGET(image), "pathbarboxNegative");
        // ok: gtk_widget_add_css_class(GTK_WIDGET(imageBox), "pathbarboxNegative");
        // more or less: gtk_widget_add_css_class(GTK_WIDGET(vbox), "pathbarboxNegative");
        gtk_box_append(GTK_BOX(imageBox), image);

        auto label = GTK_LABEL(g_object_get_data(G_OBJECT(vbox), "label"));

        if (name && strlen(name) > 15){
          name[15] = 0;
          name[14] ='~';
        }
        char *markup = g_strconcat("<span size=\"small\">", name, "</span>", NULL);
        gtk_label_set_markup( GTK_LABEL( label ), markup );
        g_free(name);
        g_free(markup);
      }

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
  };
}
#endif
