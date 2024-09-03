#ifndef LOCALDIR_HH
#define LOCALDIR_HH
namespace xf {
  class LocalDir {

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
      
      static GtkMultiSelection *standardSelectionModel(const char *path){
        // This does not have the up icon 
        auto gfile = g_file_new_for_path(path);
        GtkDirectoryList *dList = 
//          gtk_directory_list_new("", gfile); 
          gtk_directory_list_new("standard::", gfile); 
        return getSelectionModel(G_LIST_MODEL(dList));

      }
    public:
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
        g_file_info_set_icon(info, g_themed_icon_new(GO_UP));
        g_list_store_insert(store, 0, G_OBJECT(info));
        g_file_info_set_attribute_object(info, "standard::file", G_OBJECT(upFile));

        GFile *file = g_file_new_for_path(path);
        GFileEnumerator *dirEnum = 
          g_file_enumerate_children (file,"standard::,G_FILE_ATTRIBUTE_TIME_MODIFIED",G_FILE_QUERY_INFO_NONE,NULL, &error_);
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
         /* if (g_file_info_get_file_type(outInfo) == G_FILE_TYPE_REGULAR){
            auto path = g_file_get_path(outChild);
            //DBG("start preview thread for %s\n", path);
            pthread_t thread;
            pthread_create(&thread, NULL, getPreview, (void *)path);
          }*/
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
          TRACE("adding bookmark %p -> %s\n", p, p->path);
          if (!g_path_is_absolute(p->path)) continue;
          if (!g_file_test(p->path, G_FILE_TEST_EXISTS)) {
              TRACE("Bookmark %s does not exist\n", p->path);
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
    
    public:

    static void 
    addLabelTooltip(GtkWidget *label, const char *path){
      auto name = g_path_get_basename(path);
      auto fileInfo = UtilBasic::fileInfo(path);
      auto mimetype = MimeMagic::mimeMagic(path); 
      char *hidden = NULL;
      char *backup = NULL;
      if (name[0] == '.' && name[1] != '.'){
        hidden = g_strconcat("\n", _("Hidden file"), NULL);
      } else hidden = g_strdup("");
      if (UtilBasic::backupType(path)){
        backup = g_strconcat("\n", _("Backup file"), NULL);
      } else backup = g_strdup("");

      auto markup = g_strconcat("<span color=\"blue\"><b>", name, "</b></span>\n",
          _("Type:")," ",fileInfo, "\n", 
          _("Mimetype:")," ",mimetype,  
          hidden,backup,
          NULL);

      gtk_widget_set_tooltip_markup(label, markup);
      g_free(fileInfo);
      g_free(mimetype);
      g_free(markup);
      g_free(name);
      g_free(hidden);
      g_free(backup);
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
////////////////////////////////////////////////
    static gchar *
    sizeString (size_t size) {
        if (size > 1024 * 1024 * 1024){
            return g_strdup_printf("%ld GB", size/(1024 * 1024 * 1024));
        }
        if (size > 1024 * 1024){
            return g_strdup_printf("%ld MB", size/(1024 * 1024));
        }
        if (size > 1024){
            return g_strdup_printf("%ld KB", size/(1024));
        }
        return g_strdup_printf("%ld ", size);
    }
    static gchar *
    dateString (time_t the_time) {
        //pthread_mutex_lock(&dateStringMutex);

#ifdef HAVE_LOCALTIME_R
            struct tm t_r;
#endif
            struct tm *t;

#ifdef HAVE_LOCALTIME_R
            t = localtime_r (&the_time, &t_r);
#else
            t = localtime (&the_time);
#endif
            gchar *date_string=
                g_strdup_printf ("%04d/%02d/%02d  %02d:%02d", t->tm_year + 1900,
                     t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min);
        //pthread_mutex_unlock(&dateStringMutex);

        return date_string;
    }

  };
}
#endif

