#ifndef LOCALDIR_HH
#define LOCALDIR_HH
namespace xf {

  typedef struct {
    int flags;
    int count;
    int total;
    GRegex *regex;
    const char *regexp;
  } filterData_t;
  
  class LocalDir {
    using clipboard_t = ClipBoard<LocalDir>;

    public:

      static GtkMultiSelection *xfSelectionModel(const char *path, filterData_t *filterData ){
       // This section adds the up icon.
        auto flags = Settings::getInteger(path, "flags", 0);

        TRACE("path=%s up=%s\n", path, up);
        auto store = g_list_store_new(G_TYPE_FILE_INFO);
        GError *error_ = NULL;
        g_object_set_data(G_OBJECT(store), "xffm::local", GINT_TO_POINTER(1));

        // up icon
        auto up = g_path_get_dirname(path);
        auto upFile = g_file_new_for_path(up);
        auto info = g_file_query_info(upFile, "standard::", G_FILE_QUERY_INFO_NONE, NULL, &error_);
        //Important: if this is not set, then the GFile cannot be obtained from the GFileInfo:
        g_file_info_set_attribute_object(info, "standard::file", G_OBJECT(upFile));
        if (strcmp(path, up)==0) {
          g_file_info_set_attribute_object(info, "xffm::root", G_OBJECT(upFile));
        }
        g_file_info_set_attribute_object(info, "xffm::up", G_OBJECT(upFile));
        g_free(up);
        g_file_info_set_name(info, "..");
        g_file_info_set_icon(info, g_themed_icon_new(GO_UP));
        g_list_store_insert(store, 0, G_OBJECT(info));


        GFile *file = g_file_new_for_path(path); // unreffed with monitor destroy.
        g_object_set_data(G_OBJECT(store), "file", file);
        GFileEnumerator *dirEnum = 
          g_file_enumerate_children (file,"standard::,G_FILE_ATTRIBUTE_TIME_MODIFIED,G_FILE_ATTRIBUTE_TIME_CREATED",G_FILE_QUERY_INFO_NONE,NULL, &error_);
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
            ERROR_("*** Error::g_file_enumerator_iterate: %s\n", error_->message);
            return NULL;
          }
          if (!outInfo || !outChild) break;
          //Important: if this is not set, then the GFile cannot be obtained from the GFileInfo:
          g_file_info_set_attribute_object(outInfo, "standard::file", G_OBJECT(outChild));
          
          g_list_store_insert_sorted(store, G_OBJECT(outInfo), compareFunction, GINT_TO_POINTER(flags));
          TRACE("insert path=%s info=%p\n", g_file_get_path(outChild), outInfo);
          auto _path = g_file_get_path(outChild);
          setPaintableIcon(outInfo, _path);
          g_free(_path);
        } while (true);

        /* moved to gridview.hh 
        auto monitor = g_file_monitor_directory (file, G_FILE_MONITOR_WATCH_MOVES, NULL,&error_);
        g_object_set_data(G_OBJECT(monitor), "file", file);
        Child::addMonitor(monitor);

        TRACE("monitor=%p file=%p store=%p\n", monitor, file, store);
        if (error_){
            ERROR_("g_file_monitor_directory(%s) failed: %s\n",
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
        */

        return getSelectionModel(G_LIST_MODEL(store), true, filterData);
      }


    static void setPaintableIcon(GFileInfo *info, const char *path){
        if (FstabUtil::isMounted(path) || FstabUtil::isInFstab(path)) {
          FstabUtil::setMountableIcon(info, path);
        }
        auto bookmarks_p = (Bookmarks *) bookmarksObject;
        
        TRACE("isBookmarked(%s) = %d\n", path, bookmarks_p->isBookmarked(path));
        if (bookmarks_p->isBookmarked(path)){
          bookmarks_p->setBookmarkIcon(info, path);
        }
    }

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
        auto store = g_object_get_data(G_OBJECT(self), "store");
        auto xffmRoot = g_object_get_data(G_OBJECT(store), "xffm::root");
        auto xffmFstab = g_object_get_data(G_OBJECT(store), "xffm::fstab");
        TRACE("selection changed position=%d, items=%d data=%p isRoot=%d\n", 
            position, n_items, data, xffmRoot);
      
        GtkBitset *bitset = gtk_selection_model_get_selection (self);
        auto size = gtk_bitset_get_size(bitset);

        
        // Unselect up icon
        auto list = G_LIST_MODEL(self);        
        auto info = G_FILE_INFO(g_list_model_get_item (list, 0));
        auto name = g_file_info_get_name(info);
        if (gtk_selection_model_is_selected(self, 0) && strcmp(name, "..")==0) {
          size--;
          TRACE("selection_changed:: unselecting up\n");
          gtk_selection_model_unselect_item(self, 0);
        }
        
        // Only single selection allowed in xffm::root/fstab
        if (xffmRoot || xffmFstab){
          if (size > 1) {
            gtk_selection_model_unselect_all(self);
          }
        }
       return;
      }
      
    public:   
      static GtkMultiSelection *getSelectionModel(GListModel *store, bool skip0, filterData_t *filterData){
        GtkFilter *filter = 
          GTK_FILTER(gtk_custom_filter_new ((GtkCustomFilterFunc)filterFunction, filterData, NULL));

        GtkFilterListModel *filterModel = gtk_filter_list_model_new(G_LIST_MODEL(store), filter);        
        // FIXME leak: s      
        GtkMultiSelection *s = gtk_multi_selection_new(G_LIST_MODEL(filterModel));
        g_object_set_data(G_OBJECT(store), "selectionModel", s);
        g_object_set_data(G_OBJECT(s), "store", store);

        g_signal_connect(G_OBJECT(s), "selection-changed", G_CALLBACK(selection_changed), 
            skip0?s:NULL);
       
        if (filterData && filterData->flags & 0x100 && filterData->count > 0){
          char buffer[256];
          char buffer2[32];
          snprintf (buffer2, 256, "%d/%d", filterData->count, filterData->total);
          snprintf (buffer, 256, "%s [%s]", _("Regular expression"), filterData->regexp);
          // Show text will cause new tabs to open with show text to full window.
          //Print::showText(Child::getOutput());
          char buffer3[256];
          snprintf(buffer3, 256,_("%s (%s Filtered)"), buffer, buffer2);  
          Print::printInfo(Child::getOutput(), g_strconcat(buffer3,"\n", NULL));
        }


        return s;
      }

    public:
/*      static int
      getMaxNameLen(const char *path){
        GError *error_ = NULL;
        GFile *file = g_file_new_for_path(path);
        GFileEnumerator *dirEnum = 
          g_file_enumerate_children (file,"standard::,G_FILE_ATTRIBUTE_TIME_MODIFIED,G_FILE_ATTRIBUTE_TIME_CREATED",G_FILE_QUERY_INFO_NONE,NULL, &error_);
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
            ERROR_("*** Error::g_file_enumerator_iterate: %s\n", error_->message);
            return 0;
          }
          if (!outInfo || !outChild) break;
          if (strlen(g_file_info_get_name(outInfo)) > maxNameLen)
            maxNameLen = strlen(g_file_info_get_name(outInfo));
        } while (true);
        g_object_unref(file); // ?
        return maxNameLen;
      }*/

      static int getHiddenCount(GListModel *listModel, int flags, int limit){
        int count = 0;
        auto n = g_list_model_get_n_items(listModel);
        for (guint i=0; i<limit; i++){
          auto info = G_FILE_INFO(g_list_model_get_item(listModel, i)); // GFileInfo
          int showHidden = flags & 0x01;
          int showBackups = flags & 0x02;
          TRACE("flags=0x%x: showHidden = 0x%x, showBackups=0x%x\n", showHidden, showBackups);
          if (showHidden != 0x01 && g_file_info_get_is_hidden(info)) count++;
          if (showBackups != 0x02 && g_file_info_get_is_backup(info)) count++;

          //if (showHidden == 0x0 && g_file_info_get_is_hidden(info)) count++;
          //if (showBackups == 0x0 && g_file_info_get_is_backup(info)) count++;
        }
        return count;
      }
    public:
      
// selectionModel_
// G_LIST_MODEL
// GTK_SELECTION_MODEL      
      static bool findPositionModel2(GListModel *model, const char *path, guint *positionM){

        guint n = g_list_model_get_n_items(model);
        for (guint i=0; i<n; i++){
          auto info = G_FILE_INFO(g_list_model_get_object (model, i));
          auto name = g_file_info_get_name(info);
          auto basename = g_path_get_basename(path);
          if (strcmp(basename, name)==0) {
            g_free(basename);
            *positionM = i;
            TRACE("Eureka! found at %d\n", i);
            return true;
          }
          g_free(basename);
        }
        return false;
      }

    public:
      
      static bool findPositionStore(GListStore *store, const char *path, guint *positionS){ 
        // result will be offset by hidden items.
        GFileInfo *infoF = g_file_info_new();
        auto name = g_path_get_basename(path);
        char *utfName=NULL;
        if (g_object_get_data(G_OBJECT(store), "xffm::fstab")) {
          auto label = FstabUtil::e2Label(name);
          if (label){
              gchar *g = g_strdup_printf("%s\n(%s)", name, label);
              g_free(label);
              label = g;
              g_free(name);
          } else {
             label = name;
          }
          utfName = Basic::utf_string(label);
          g_free(label);
        } else {
          utfName = Basic::utf_string(name);
          g_free(name);
        }
        g_file_info_set_name(infoF, utfName);
        auto found = g_list_store_find_with_equal_func(store, infoF, equal_f, positionS);
        g_free(utfName);
        g_object_unref(infoF);
        if (found){
          TRACE("%s found at position %d\n", path, *positionS);
        } else {

          TRACE("%s not found by GFileInfo\n", path); 
        }
        return found;
      }
      


    public:
      

      static void insert(GListStore *store, const char *path, bool verbose){
        GError *error_ = NULL;
        auto dirPath = g_path_get_dirname(path);
        auto flags = Settings::getInteger(dirPath, "flags", 0);
        g_free(dirPath);

        auto file = g_file_new_for_path(path);
        GFileInfo *infoF = g_file_query_info (file,
            "standard::,G_FILE_ATTRIBUTE_TIME_MODIFIED,G_FILE_ATTRIBUTE_TIME_CREATED,owner::,user::", 
            G_FILE_QUERY_INFO_NONE, NULL, &error_);

        setPaintableIcon(infoF, path);

        if (error_){
          TRACE("Error: %s\n", error_->message);
          g_error_free(error_);
          return;
        }

        //Important: if this is not set, then the GFile cannot be obtained from the GFileInfo:
        g_file_info_set_attribute_object(infoF, "standard::file", G_OBJECT(file));
        if (verbose) {TRACE("insert():path = %s,infoF=%p\n", path, infoF);}
        g_list_store_insert_sorted(store, G_OBJECT(infoF), compareFunction, GINT_TO_POINTER(flags));
      }


private:

      static gboolean equal_f (gconstpointer a, gconstpointer b){
        auto A = G_FILE_INFO(a);
        auto B = G_FILE_INFO(b);
        auto nameA = g_file_info_get_name(A);
        auto nameB = g_file_info_get_name(B);
        TRACE("compare \"%s\" with \"%s\"\n", nameA, nameB);
        if (strcmp(nameA, nameB) == 0) return true;
        return false;
      }
    public:      
    private:
      static gboolean
      filterFunction(GObject *object, void *data){
        auto filterData = (filterData_t *)data;
        auto flags = filterData? filterData->flags: 0;
        auto regex = filterData? filterData->regex: NULL;
        GFileInfo *info = G_FILE_INFO(object);
        bool showHidden = flags & 0x01;
        bool showbackups = flags & 0x02;
        bool doRegex = flags & 0x100;
        auto hidden = g_file_info_get_is_hidden(info);
        auto backup = g_file_info_get_is_backup(info);
        auto name = g_file_info_get_name(info);

        
         //* not working. 
        auto c =(clipboard_t *)clipBoardObject;
        bool isCut = false;
        if (c) {
          auto path = Basic::getPath(info);
          isCut = c->isCutItem(path);
          if (isCut) {TRACE("item %s is cut\n", path);}
          g_free(path);
          if (isCut) return false;
        }
       
        // First do the regex, if applicable.
        if (doRegex && regex){
          // skip ".."
          bool match;
          if (strcmp(name, "..") == 0) match = true;
          else match = g_regex_match (regex, name, (GRegexMatchFlags)0, NULL);
          
          if (!match) {
            filterData->count++;
            TRACE("not match %s, count=%d\n", name, filterData->count);
          } else TRACE("name: match=%s\n", match?"true":"false");
          filterData->total++;

          return match;
        }
        
        if (hidden){
          if (showHidden) return true;
          return false;
        }
        if (backup){
          if (showbackups) return true;
          return false;
        }
        return TRUE;
      }
  
    public: 
      // flags :
    // 0x01 : by date _("Hidden files")
    // 0x02 : by size _("Backup files")
    // 0x04 : descending _("Descending")
    // 0x08 : _("Date")
    // 0x10 : _("Size")
    // 0x20 : _("File type")
    // 0x100 : _("Regular expression") : not for sorting, just for prevous filter... TODO
    static gint 
    compareFunction(const void *a, const void *b, void *data){
        auto flags = GPOINTER_TO_INT(data);
        bool byDate = (flags & 0x08);
        bool bySize = (flags & 0x10);
        bool descending = (flags & 0x04);
        bool fileType = (flags & 0x20);
        bool byRegexp = (flags & 0x100);
        // mutually exclusive, byDate, bySize.

        TRACE("*** compareFunction flags=0x%x byDate=%d, bySize=%d, descending=%d, fileType=%d\n", flags, byDate, bySize, descending, fileType);
        

        GFileInfo *infoA = G_FILE_INFO(a);
        GFileInfo *infoB = G_FILE_INFO(b);

        if (g_file_info_get_attribute_object (infoA, "xffm::up")){
           return -1;          
        }
        if (g_file_info_get_attribute_object (infoB, "xffm::up")){
           return (1);           
        }
        if (g_file_info_get_attribute_object (infoA, "xffm::fstab")){
          return -1;          
        }
        if (g_file_info_get_attribute_object (infoB, "xffm::fstab")){
          return 1;          
        }
        if (g_file_info_get_attribute_object (infoA, "xffm::ecryptfs")){
          return -1;          
        }
        if (g_file_info_get_attribute_object (infoB, "xffm::ecryptfs")){
          return 1;          
        }
        if (g_file_info_get_attribute_object (infoA, "xffm::trash")){
          return -1;          
        }
        if (g_file_info_get_attribute_object (infoB, "xffm::trash")){
          return 1;          
        }
       

        auto typeA = g_file_info_get_file_type(infoA);
        auto typeB = g_file_info_get_file_type(infoB);
        auto nameA = g_file_info_get_name(infoA);
        auto nameB = g_file_info_get_name(infoB);
        struct stat stA;
        struct stat stB;      

        if (byDate || bySize) {
          auto pathA = Basic::getPath(infoA);
          auto pathB = Basic::getPath(infoB);
          stat(pathA, &stA);
          stat(pathB, &stB);
          g_free(pathA);
          g_free(pathB);
        }

            // this is not freaking working. Seems the monitor is not
            // sending the GFileInfo with time stamps.
          /*  TRACE("bydata1\n");
            auto dateTimeA = g_file_info_get_modification_date_time(infoA);
            auto dateTimeB = g_file_info_get_modification_date_time(infoB);
            auto value = g_date_time_compare(dateTimeA, dateTimeB);*/
        

        TRACE("--1\n");
        if (strcmp(g_file_info_get_name(infoA), "..")==0) return -1;
        if (strcmp(g_file_info_get_name(infoB), "..")==0) return 1;
        
        if (fileType){
          auto extA = strrchr(nameA, '.');
          auto extB = strrchr(nameB, '.');
          if (!extA) extA = ".";
          if (!extB) extB = ".";
          auto value = strcasecmp(extA, extB);
          //  error if (value == 0) value = strcasecmp(extA, extB);
          if (value == 0){
            value = strcasecmp(nameA, nameB);
          }
          if (descending) return -value;
          return value;
        }


        GFile *fileA = G_FILE(g_file_info_get_attribute_object(infoA, "standard::file"));
        GFile *fileB = G_FILE(g_file_info_get_attribute_object(infoB, "standard::file"));
        TRACE("--2\n");

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
        TRACE("--3\n");

        if (a_cond && b_cond) {
            // directory comparison by name is default;
           if (byDate) {
              if (descending) return (stB.st_mtime - stA.st_mtime);
              return (stA.st_mtime - stB.st_mtime);
           } else {
                if (descending) return -strcasecmp(nameA, nameB);
                return strcasecmp(nameA, nameB);
            }
        }
        // by date
        if (byDate){
              if (descending) return (stB.st_mtime - stA.st_mtime);
              return (stA.st_mtime - stB.st_mtime);
        } else if (bySize){
          if (descending) return stB.st_size - stA.st_size;
          return stA.st_size - stB.st_size;
        } 
        TRACE("--4\n");
        // by name 
        if (descending) return -strcasecmp(nameA, nameB);
        return strcasecmp(nameA, nameB);
    }
    
    public:
/*
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
            ERROR_("*** Error::g_file_enumerator_iterate: %s\n", error_->message);
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
  */
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

