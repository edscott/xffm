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
          memset(&st, 0, sizeof(struct stat));
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
    /*    GtkSorter *sorter = 
          GTK_SORTER(gtk_custom_sorter_new((GCompareDataFunc)compareFunction, NULL, NULL));
        GtkSortListModel *sortModel = gtk_sort_list_model_new(G_LIST_MODEL(filterModel), sorter);
        GtkMultiSelection *s = gtk_multi_selection_new(G_LIST_MODEL(sortModel));*/

        GtkMultiSelection *s = gtk_multi_selection_new(G_LIST_MODEL(filterModel));
        g_object_set_data(G_OBJECT(store), "selectionModel", s);
        g_object_set_data(G_OBJECT(s), "store", store);

        g_signal_connect(G_OBJECT(s), "selection-changed", G_CALLBACK(selection_changed), NULL);
        return s;
      }
      
      static GtkMultiSelection *standardSelectionModel(const char *path){
        // This does not have the up icon 
        auto gfile = g_file_new_for_path(path);
        GtkDirectoryList *dList = gtk_directory_list_new("standard::", gfile); 
        gtk_directory_list_set_monitored(dList, true);

        return getSelectionModel(G_LIST_MODEL(dList));
      }

    public:
      static int
      getMaxNameLen(const char *path){
        GError *error_ = NULL;
        GFile *file = g_file_new_for_path(path);
        GFileEnumerator *dirEnum = 
          g_file_enumerate_children (file,"standard::,G_FILE_ATTRIBUTE_TIME_MODIFIED",G_FILE_QUERY_INFO_NONE,NULL, &error_);
        if (error_) {
          TRACE("*** Error::g_file_enumerate_children: %s\n", error_->message);
          Print::printError(Child::getOutput(), g_strdup(error_->message));
          g_error_free(error_);
          return 0;
        }
        GFile *outChild = NULL;
        GFileInfo *outInfo = NULL;
        int k = 1;
        int maxNameLen = 0;
        do {
          g_file_enumerator_iterate (dirEnum, &outInfo, &outChild,
              NULL, // GCancellable* cancellable,
              &error_);
          if (error_) {
            DBG("*** Error::g_file_enumerator_iterate: %s\n", error_->message);
            return 0;
          }
          if (!outInfo || !outChild) break;
          if (strlen(g_file_info_get_name(outInfo)) > maxNameLen)
            maxNameLen = strlen(g_file_info_get_name(outInfo));
        } while (true);
        return maxNameLen;
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
        g_file_info_set_icon(info, g_themed_icon_new(GO_UP));
        g_list_store_insert(store, 0, G_OBJECT(info));
        //Important: if this is not set, then the GFile cannot be obtained from the GFileInfo:
        g_file_info_set_attribute_object(info, "standard::file", G_OBJECT(upFile));

        GFile *file = g_file_new_for_path(path); // XXX mem leak
        GFileEnumerator *dirEnum = 
          g_file_enumerate_children (file,"standard::,G_FILE_ATTRIBUTE_TIME_MODIFIED",G_FILE_QUERY_INFO_NONE,NULL, &error_);
        if (error_) {
          TRACE("*** Error::g_file_enumerate_children: %s\n", error_->message);
          Print::printError(Child::getOutput(), g_strdup(error_->message));
          g_error_free(error_);
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
          //Important: if this is not set, then the GFile cannot be obtained from the GFileInfo:
          g_file_info_set_attribute_object(outInfo, "standard::file", G_OBJECT(outChild));
          
          void *flags = NULL; // FIXME: this will determine sort order
          g_list_store_insert_sorted(store, G_OBJECT(outInfo), compareFunction, flags);
          TRACE("insert path=%s info=%p\n", g_file_get_path(outChild), outInfo);
        } while (true);

        auto monitor = g_file_monitor_directory (file, G_FILE_MONITOR_WATCH_MOVES, NULL,&error_);

        DBG("monitor=%p file=%p store=%p\n", monitor, file, store);
        if (error_){
            ERROR("g_file_monitor_directory(%s) failed: %s\n",
                    path, error_->message);
            g_error_free(error_);
            //g_object_unref(file);
            //file=NULL;
            // return;
        } else {
          g_signal_connect (monitor, "changed", 
                G_CALLBACK (changed_f), (void *)store);
        }
        g_object_set_data(G_OBJECT(store), "monitor", monitor);
        return getSelectionModel(G_LIST_MODEL(store));
      }

      static bool findPosition(GListStore *store, const char *path, guint *positionF, bool verbose){
        GFileInfo *infoF = g_file_info_new();
        auto name = g_path_get_basename(path);
        g_file_info_set_name(infoF, name);
        auto found = g_list_store_find_with_equal_func(store, infoF, equal_f, positionF);
        g_free(name);
        g_object_unref(infoF);
        if (found){
          if (verbose) {DBG("%s found at position %d\n", path, *positionF);}
        } else {
          if (verbose) {DBG("%s not found by GFileInfo\n", path);  }
        }
        return found;
      }

      static void insert(GListStore *store, const char *path, bool verbose){
          GError *error_ = NULL;
          //void *flags = GINT_TO_POINTER(0x100); // FIXME: this will determine sort order
          void *flags = NULL; // FIXME: this will determine sort order
          auto file = g_file_new_for_path(path);
          GFileInfo *infoF = g_file_query_info (file,
              "standard::,G_FILE_ATTRIBUTE_TIME_MODIFIED,owner::,user::", 
              G_FILE_QUERY_INFO_NONE, NULL, &error_);

          //Important: if this is not set, then the GFile cannot be obtained from the GFileInfo:
          g_file_info_set_attribute_object(infoF, "standard::file", G_OBJECT(file));
          if (verbose) {DBG("insert():path = %s,infoF=%p\n", path, infoF);}
          if (error_){
            DBG("Error: %s\n", error_->message);
            g_error_free(error_);
            g_object_unref(infoF);
            return;
          }
          g_list_store_insert_sorted(store, G_OBJECT(infoF), compareFunction, flags);
      }
   /*   static void toggleSelect(GListStore *store, guint positionF){
          auto s = GTK_SELECTION_MODEL(g_object_get_data(G_OBJECT(store), "selectionModel"));
          if (gtk_selection_model_is_selected (s, positionF)){
            gtk_selection_model_unselect_item (s, positionF);
            gtk_selection_model_select_item (s, positionF, false);
          } else {
            gtk_selection_model_select_item (s, positionF, false);
            gtk_selection_model_unselect_item (s, positionF);
          }
      }*/

      static void
      changed_f ( GFileMonitor* self,  
          // This runs in main context, I presume.
          GFile* first, GFile* second, //same as GioFile * ?
          GFileMonitorEvent event, 
          void *data){

        // Switch to pause monitor execution.
        pthread_mutex_lock(&monitorMutex);   
        auto active = g_object_get_data(G_OBJECT(self), "active");
        pthread_mutex_unlock(&monitorMutex);   
        if (!active) {
          DBG("monitor %p inactive\n", self);
          return;
        }

        GListStore *store = G_LIST_STORE(data);
        auto child = (GtkWidget *)g_object_get_data(G_OBJECT(store), "child");
        if (!child){
          DBG("localdir.hh::changed_f(): this should not happen\n");
          exit(1);
        }
        DBG("*** monitor changed_f call position=%d...\n", 0);
        gchar *f= first? g_file_get_path (first):g_strdup("--");
        gchar *s= second? g_file_get_path (second):g_strdup("--");

      /*  GError *error_=NULL;
        GFileInfo *infoF = first? g_file_query_info (first, "standard::,G_FILE_ATTRIBUTE_TIME_MODIFIED", 
            G_FILE_QUERY_INFO_NONE, NULL, &error_):NULL;
        if (error_){
          DBG("Error: %s\n", error_->message);
          g_error_free(error_);
          return;
        }*/

        /* if (!active){
             DBG("monitor_f(): monitor not currently active.\n");
             return;
        }
        if (p->view()->serial() != p->serial()){
            DBG("LocalMonitor::changeItem() serial out of sync (%d != %d)\n",p->view()->serial(), p->serial());
            return;
        }*/

        bool verbose = false;
        guint positionF;
        void *flags = NULL; // FIXME: this will determine sort order
        if (verbose) DBG("monitor thread %p...\n", g_thread_self());
         switch (event){
            case G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED:
              {
                if (verbose) {DBG("Received  ATTRIBUTE_CHANGED (%d): \"%s\", \"%s\"\n", event, f, s);}
                auto found = findPosition(store, f, &positionF, verbose);
                if (found) {
                    Child::incrementSerial(child);
                  g_list_store_remove(store, positionF);
                    Child::incrementSerial(child);
                  insert(store, f, verbose);                        
                }

                //p->restat_item(f);
              } 
                break;
            case G_FILE_MONITOR_EVENT_PRE_UNMOUNT:
                if (verbose) {DBG("Received  PRE_UNMOUNT (%d): \"%s\", \"%s\"\n", event, f, s);}
                break;
            case G_FILE_MONITOR_EVENT_UNMOUNTED:
                if (verbose) {DBG("Received  UNMOUNTED (%d): \"%s\", \"%s\"\n", event, f, s);}
                break;

            case G_FILE_MONITOR_EVENT_DELETED:
            case G_FILE_MONITOR_EVENT_MOVED_OUT:
                {
                  if (verbose) {DBG("Received DELETED  (%d): \"%s\", \"%s\"\n", event, f, s);}  
                  auto found = findPosition(store, f, &positionF, verbose);
                  if (found) {
                    Child::incrementSerial(child);
                    g_list_store_remove(store, positionF);
                  }
                }
                //p->remove_item(first);
                //p->updateFileCountLabel();
                break;

            case G_FILE_MONITOR_EVENT_CREATED:
            case G_FILE_MONITOR_EVENT_MOVED_IN:
                {
                  if (verbose) {DBG("Received  CREATED (%d): \"%s\", \"%s\"\n", event, f, s);}
                  if (!g_file_test(f, G_FILE_TEST_EXISTS)){
                    if (verbose) {DBG("Ghost file: %s\n", f);}
                    g_free(f);
                    g_free(s);
                    return;
                  }
                  Child::incrementSerial(child);
                  insert(store, f, verbose);
                }
                break;
            case G_FILE_MONITOR_EVENT_CHANGED:
            {
                if (verbose) {DBG("monitor_f(): Received  CHANGED (%d): \"%s\", \"%s\"\n", event, f, s);}
                //p->restat_item(f);
            } break;
            case G_FILE_MONITOR_EVENT_MOVED:
            case G_FILE_MONITOR_EVENT_RENAMED:
            {
                if (verbose) {DBG("Received  MOVED (%d): \"%s\", \"%s\"\n", event, f, s);}
                auto found = findPosition(store, f, &positionF, verbose);
                if (found){
                  Child::incrementSerial(child);
                  g_list_store_remove(store, positionF);
                  Child::incrementSerial(child);
                  insert(store, s, verbose);           
                }
            }
                  



                //p->add2reSelect(f); // Only adds to selection list if item is selected.
                //p->remove_item(first); 

                //if (!isInModel(p->treeModel(), s)){
                    //p->add_new_item(second);
                //} //else p->restat_item(s);
                break;
            case G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT:
                if (verbose) {DBG("Received  CHANGES_DONE_HINT (%d): \"%s\", \"%s\"\n", event, f, s);}
                //p->reSelect(f); // Will only select if in selection list (from move).
                break;       
        }
        g_free(f);
        g_free(s);
       

      }


      static gboolean equal_f (gconstpointer a, gconstpointer b){
        auto A = G_FILE_INFO(a);
        auto B = G_FILE_INFO(b);
        auto nameA = g_file_info_get_name(A);
        auto nameB = g_file_info_get_name(B);
        TRACE("compare \"%s\" with \"%s\"\n", nameA, nameB);
        if (strcmp(nameA, nameB) == 0) return true;
        return false;
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
          auto utf_name = Basic::utf_string(basename);
          g_file_info_set_name(info, utf_name);
          g_free(basename);
          g_free(utf_name);
          g_file_info_set_icon(info, g_themed_icon_new(EMBLEM_BOOKMARK));
          g_list_store_insert(store, 0, G_OBJECT(info));
          //Important: if this is not set, then the GFile cannot be obtained from the GFileInfo:
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
        
        bool verbose = (flags & 0x100);

        GFileInfo *infoA = G_FILE_INFO(a);
        GFileInfo *infoB = G_FILE_INFO(b);
        auto typeA = g_file_info_get_file_type(infoA);
        auto typeB = g_file_info_get_file_type(infoB);

        if (verbose) DBG("--1\n");
        if (strcmp(g_file_info_get_name(infoA), "..")==0) return -1;
        if (strcmp(g_file_info_get_name(infoB), "..")==0) return 1;
        
        GFile *fileA = G_FILE(g_file_info_get_attribute_object(infoA, "standard::file"));
        GFile *fileB = G_FILE(g_file_info_get_attribute_object(infoB, "standard::file"));
        if (verbose) DBG("--2\n");

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
        if (verbose) DBG("--3\n");

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
        if (verbose) DBG("--4\n");
        // by name 
        if (descending) return -strcasecmp(nameA, nameB);
        return strcasecmp(nameA, nameB);
    }
    
    public:

    static void 
    addLabelTooltip(GtkWidget *label, const char *path){
      auto name = g_path_get_basename(path);
      // This forks without limit:
      // auto fileInfo = UtilBasic::fileInfo(path);
      // This takes too long in old box:
      // auto mimetype = MimeMagic::mimeMagic(path); 
      char *hidden = NULL;
      char *backup = NULL;
      if (name[0] == '.' && name[1] != '.'){
        hidden = g_strconcat("\n", _("Hidden file"), NULL);
      } else hidden = g_strdup("");
      if (UtilBasic::backupType(path)){
        backup = g_strconcat("\n", _("Backup file"), NULL);
      } else backup = g_strdup("");

      auto markup = g_strconcat("<span color=\"blue\"><b>", name, "</b></span>\n",
//          _("Type:")," ",fileInfo, "\n", 
//          _("Mimetype:")," ",mimetype,  
          hidden,backup,
          NULL);

      gtk_widget_set_tooltip_markup(label, markup);
//      g_free(fileInfo);
//      g_free(mimetype);
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
        Basic::concat(&t, ":\n");
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
          Basic::concat(&t, "<span color=\"blue\" size=\"x-small\">  ");
          Basic::concat(&t, g_file_info_get_name(outInfo));
          auto subtype = g_file_info_get_file_type(outInfo);
          if (subtype == G_FILE_TYPE_DIRECTORY )Basic::concat(&t, "/");
          if (subtype == G_FILE_TYPE_SYMBOLIC_LINK)Basic::concat(&t, "*");
          Basic::concat(&t, "</span>\n");
          if (++k >= limit) {
            Basic::concat(&t, "<span color=\"red\">");
            Basic::concat(&t, _("More..."));
            Basic::concat(&t, "</span>\n");
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

