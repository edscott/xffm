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
        GtkDirectoryList *dList = gtk_directory_list_new("standard::type", gfile); // G_LIST_MODEL
//        GtkDirectoryList *dList = gtk_directory_list_new(NULL, gfile); // G_LIST_MODEL
        while (gtk_directory_list_is_loading(dList)) {
          DBG("gtk_directory_list_is_loading...\n");
           while (g_main_context_pending(NULL)) g_main_context_iteration(NULL, TRUE);
          //Util::flushGTK();
        }
        auto num = g_list_model_get_n_items(G_LIST_MODEL(dList));
        DBG("gtk_directory_list_is_loading done: items=%d\n", num);
        for (int i=0; i<num; i++){
          auto info = G_FILE_INFO(g_list_model_get_item(G_LIST_MODEL(dList), i));
        }
        
        GtkNoSelection *selection_model = gtk_no_selection_new(G_LIST_MODEL(dList));
        GtkMultiSelection *selection_model2 = gtk_multi_selection_new(G_LIST_MODEL(dList));
              /*while (gtk_directory_list_is_loading(dList)) {
              }
              int num = g_list_model_get_n_items(G_LIST_MODEL(dList));
              fprintf(stderr, "gtk_directory_list_is_loading done: items=%d\n", num);*/
       
        // GtkListItemFactory implements GtkSignalListItemFactory, which can be connected to
        // bind, setup, teardown and unbind
        GtkListItemFactory *factory = gtk_signal_list_item_factory_new();

        /* Connect handler to the factory.
         */
        g_signal_connect( factory, "setup", G_CALLBACK(factory3_setup), NULL );
        g_signal_connect( factory, "bind", G_CALLBACK(factory3_bind), NULL);

        GtkWidget *view;
        /* Create the view.
         */
        view = gtk_grid_view_new( GTK_SELECTION_MODEL( selection_model2 ), factory );
        gtk_widget_add_css_class(view, "xficons");
        gtk_grid_view_set_enable_rubberband(GTK_GRID_VIEW(view), TRUE);
        return view;
      }
    private:
  static void
  factory3_setup(GtkSignalListItemFactory *self, GObject *object, void *data){
    GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *label = gtk_label_new( "" );
    GtkWidget *imageBox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

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
  factory3_bind(GtkSignalListItemFactory *self, GObject *object, void *data)
  {
    auto list_item =GTK_LIST_ITEM(object);
    auto vbox = GTK_BOX(gtk_list_item_get_child( list_item ));
    auto info = G_FILE_INFO(gtk_list_item_get_item(list_item));

    auto type = g_file_info_get_file_type(info);
    const char *iconLo = NULL;
    const char *iconHi = NULL;
    GFile *z = G_FILE(g_file_info_get_attribute_object(info, "standard::file"));
    auto path = g_file_get_path(z);
    switch (type){
      case G_FILE_TYPE_UNKNOWN:
        iconLo = "default";
        break;
      case G_FILE_TYPE_REGULAR:
        iconLo = "application-x-generic";
        break;
      case G_FILE_TYPE_DIRECTORY:
              
        if (strcmp(path, g_get_home_dir())==0) iconLo = "user-home";
        else iconLo = "folder";
        break;
      case G_FILE_TYPE_SYMBOLIC_LINK:
        iconLo = "emblem-symbolic-link";
        break;
      case G_FILE_TYPE_SPECIAL:
        iconLo = "application-x-generic";
        break;
      case G_FILE_TYPE_SHORTCUT: // Windows
        iconLo = "emblem-symbolic-link";
        break;
      case G_FILE_TYPE_MOUNTABLE:
        iconLo = "drive-harddisk";
        break;
    }
    g_free(path);
    if (iconLo == NULL) iconLo ="application-certificate";
    auto texture = Texture::load(iconLo);

    GtkWidget *imageBox = GTK_WIDGET(g_object_get_data(G_OBJECT(vbox), "imageBox"));
    auto w = gtk_widget_get_first_child (imageBox);
    if (w) gtk_widget_unparent(w);
    
    
//    auto texture = Texture::load("/usr/share/icons/Adwaita/scalable/mimetypes/application-certificate.svg");
    GtkWidget *image = gtk_image_new_from_paintable(GDK_PAINTABLE(texture));
    auto size = Settings::getInteger("xfterm", "iconsize");
    if (size < 0) size = 48;
    gtk_widget_set_size_request(image, size, size);
    gtk_box_append(GTK_BOX(imageBox), image);

    auto label = GTK_LABEL(g_object_get_data(G_OBJECT(vbox), "label"));

    char *name = g_strdup(g_file_info_get_name(info));
    if (name && strlen(name) > 15){
      name[15] = 0;
      name[14] ='~';
    }
    char *markup = g_strconcat("<span size=\"small\">", name, "</span>", NULL);
    gtk_label_set_markup( GTK_LABEL( label ), markup );
    g_free(name);
    g_free(markup);
  }

  };
}
#endif
