#ifndef ROOTDIR_HH
#define ROOTDIR_HH
namespace xf {

  class rootDir {
    public:
      static GtkMultiSelection *rootSelectionModel(void){
        GError *error_ = NULL;
        int count = 0;
        Bookmarks::initBookmarks();
        auto store = g_list_store_new(G_TYPE_FILE_INFO);
        g_object_set_data(G_OBJECT(store), "xffm::root", GINT_TO_POINTER(1));

        auto flags = Settings::getInteger("flags", _("Bookmarks"));
        if (flags < 0) flags = 0;

        // fstab icon
        {
          GFile *file = g_file_new_for_path(g_get_home_dir());
          auto info = g_file_query_info(file, "standard::", G_FILE_QUERY_INFO_NONE, NULL, &error_);
          g_file_info_set_attribute_object(info, "standard::file", G_OBJECT(file));   
          g_file_info_set_icon(info, g_themed_icon_new("drive-harddisk"));
          g_file_info_set_name(info, _("Disk Mounter"));
          g_list_store_insert(store, count++, G_OBJECT(info));
          g_file_info_set_attribute_object (info, "xffm::fstab", G_OBJECT(file));
        }

        // ecryptfs icon
        {
          auto size = Settings::getInteger("xfterm", "iconsize");
          if (size < 0) size = 48;
          double scaleFactor = (size == 24)? 0.75 : 1.0;
          
          GFile *file = g_file_new_for_path(g_get_home_dir());
          auto text = g_strdup_printf("%s ecryptfs", _("New"));
          auto info = g_file_query_info(file, "standard::", G_FILE_QUERY_INFO_NONE, NULL, &error_);
          //auto iconPath = Texture<bool>::findIconPath("folder");
          auto paintable = Texture<bool>::addEmblem("folder", "emblem-start-here", scaleFactor*size, scaleFactor*size);
          g_file_info_set_attribute_object(info, "xffm:paintable", G_OBJECT(paintable));      
          
          g_file_info_set_attribute_object(info, "standard::file", G_OBJECT(file));   
          g_file_info_set_name(info, text);
          g_list_store_insert(store, count++, G_OBJECT(info));
          g_file_info_set_attribute_object (info, "xffm::ecryptfs", G_OBJECT(file));
        }
        // trash icon
        {
          auto trashDir = g_strdup_printf("%s/.local/share/Trash/files", g_get_home_dir());
          auto trashIcon = (g_file_test(trashDir, G_FILE_TEST_IS_DIR))? 
              "user-trash-full" : "user-trash";
          GFile *file = (g_file_test(trashDir, G_FILE_TEST_IS_DIR))? 
              g_file_new_for_path(trashDir) : g_file_new_for_path(g_get_home_dir());
          
          auto info = g_file_query_info(file, "standard::", G_FILE_QUERY_INFO_NONE, NULL, &error_);
          g_file_info_set_attribute_object(info, "standard::file", G_OBJECT(file));   
          g_file_info_set_icon(info, g_themed_icon_new(trashIcon));
          g_file_info_set_name(info, _("Trash bin"));
          g_list_store_insert(store, count++, G_OBJECT(info));
          g_file_info_set_attribute_object (info, "xffm::trash", G_OBJECT(file));
        }

        // saved ecryptfs mount points
        auto items = EFS<bool>::getSavedItems();
        for (auto p=items; p && *p; p++){
          if (!g_file_test(*p, G_FILE_TEST_EXISTS)) continue;
          GFile *file = g_file_new_for_path(*p);
          auto info = g_file_query_info(file, "standard::", G_FILE_QUERY_INFO_NONE, NULL, &error_);
          g_file_info_set_attribute_object(info, "standard::file", G_OBJECT(file));          
          auto basename = g_path_get_basename(*p);
          auto utf_name = Basic::utf_string(basename);
          g_file_info_set_name(info, utf_name);
           
          int size = Settings::getInteger("xfterm", "iconsize");
          //const char *iconPath = Texture<bool>::findIconPath("folder-remote");
          const char *ball = "emblem-noaccess";
          if (FstabUtil::isMounted(*p)) ball = "emblem-greenball";
          auto paintable = Texture<bool>::addEmblem("folder-remote", ball, size, size);
          g_file_info_set_attribute_object(info, "xffm:paintable", G_OBJECT(paintable));
          g_file_info_set_attribute_object (info, "xffm::ecryptfs", G_OBJECT(file));
          g_file_info_set_attribute_object (info, "xffm::efs", G_OBJECT(file));
          
          g_list_store_insert_sorted(store, G_OBJECT(info), LocalDir::compareFunction, GINT_TO_POINTER(flags));
          //g_list_store_insert(store, 0, G_OBJECT(info));
          //Important: if this is not set, then the GFile cannot be obtained from the GFileInfo:
      
          DBG("EFS->%s path=%s\n", utf_name, *p);
          g_free(utf_name);
          g_free(basename);
          g_strfreev(items);
        }
        
        // bookmarks
        {

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
            g_list_store_insert_sorted(store, G_OBJECT(info), LocalDir::compareFunction, GINT_TO_POINTER(flags));
            //g_list_store_insert(store, 0, G_OBJECT(info));
            //Important: if this is not set, then the GFile cannot be obtained from the GFileInfo:
            g_file_info_set_attribute_object(info, "standard::file", G_OBJECT(file));          
            g_file_info_set_attribute_object (info, "xffm::bookmark", G_OBJECT(file));
          }
        }
        return LocalDir::getSelectionModel(G_LIST_MODEL(store), false, 0);
      }

  };

}
#endif

