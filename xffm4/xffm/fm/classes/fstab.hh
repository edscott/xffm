#ifndef XF_FSTAB_HH 
#define XF_FSTAB_HH

/*
msgid "eCryptfs Volume"
msgid "SSHFS Remote Synchronization Folder"
msgid "SFTP over SSH2"
msgid "SFTP (via SSH)"
msgid "Secure FTP (SFTP)"
msgid "Mount"
msgid "Mount %s"
msgid "Mount point"
msgid "Mount Point"
msgid "Mount Volume"
msgid "Mount user-space filesystems (FUSE)"
msgid "FUSE Volume"
msgid "Mount Helper"
msgid "Mount user-space filesystems (FUSE)"
msgid "Mount local disks and devices"
msgid "Unix networks (NFS)"
msgid "CIFS Volume"
msgid "NFS Network Volume"
msgid "NFS remote directory"
*/

namespace xf {
//template <class Type> class FstabPopUp;


//static pthread_mutex_t mntmutex = PTHREAD_MUTEX_INITIALIZER;
//static GMutex *infomutex=NULL;

class FstabDir {

  public:
    static GtkMultiSelection *fstabSelectionModel(void){
      GError *error_ = NULL;
      auto store = g_list_store_new(G_TYPE_FILE_INFO);
      g_object_set_data(G_OBJECT(store), "xffm::fstab", GINT_TO_POINTER(1));

      auto flags = Settings::getInteger("Disk Mounter", "flags", 0);
//      auto flags = Settings::getInteger("flags", "Disk Mounter",0);
    
      auto upFile = g_file_new_for_path(g_get_home_dir());
      auto info = g_file_query_info(upFile, "standard::", G_FILE_QUERY_INFO_NONE, NULL, &error_);
      g_file_info_set_attribute_object(info, "standard::file", G_OBJECT(upFile));
      g_file_info_set_attribute_object(info, "xffm::root", G_OBJECT(upFile));
      g_file_info_set_name(info, "..");
      g_file_info_set_icon(info, g_themed_icon_new(GO_UP));
      //g_list_store_insert(store, 0, G_OBJECT(info));
      g_list_store_insert_sorted(store, G_OBJECT(info), LocalDir::compareFunction, GINT_TO_POINTER(flags));

      //  RootView<Type>::addXffmItem(treeModel);
      linuxAddPartitionItems(store);
      addFstabItems(store);      
      return LocalDir::getSelectionModel(G_LIST_MODEL(store), true, 0);
    }
      
    static bool findPositionModel2(GListModel *model, const char *path, guint *positionM){

        guint n = g_list_model_get_n_items(model);
        for (guint i=0; i<n; i++){
          auto info = G_FILE_INFO(g_list_model_get_object (model, i));
          auto name = g_strdup(g_file_info_get_name(info));
          if (strchr(name, '\n')) *strchr(name, '\n') = 0;
          auto basename = g_path_get_basename(path);
          TRACE("Fstab::findPositionModel2: %d) %s, %s\n", i, name, basename);
          if (strcmp(basename, name)==0) {
            g_free(basename);
            *positionM = i;
            TRACE("Eureka! found at %d\n", i);
            return true;
          }
          g_free(basename);
          g_free(name);
        }
        return false;
      }

  private:
    static void // Linux
    linuxAddPartitionItems (GListStore *store) {
        FILE *partitions = fopen ("/proc/partitions", "r");
        if(!partitions) return;

        gchar line[1024];
        memset (line, 0, 1024);
        while(fgets (line, 1023, partitions) && !feof (partitions)) {
            char *path = FstabUtil::getPartitionPath(line);
            if (!path) continue; // not a partition path line...
            if (!g_path_is_absolute(path)){
                ERROR_("fstab/view.hh::partition path should be absolute: %s\n", path);
                continue;
            }
            addPartition(store, path);
            g_free(path);
            //g_free(fstype);
            memset (line, 0, 1024);
        }
        fclose (partitions);
        return;
    }

    static void addListItem(GListStore *store, void *data){
        auto mnt_struct = (struct mntent *)data;
        TRACE ("nmnt_fsname=%s, \nmnt_dir= %s, \nmnt_type=%s, \nmnt_opts=%s\n",
                    mnt_struct->mnt_fsname, 
                    mnt_struct->mnt_dir, 
                    mnt_struct->mnt_type, 
                    mnt_struct->mnt_opts);

        //gboolean mounted = isMounted(mnt_struct->mnt_dir);
        /*char *text = g_strdup_printf("%s (%s):\n%s\n%s",
                        mnt_struct->mnt_dir, 
                        mnt_struct->mnt_type, 
                        mnt_struct->mnt_fsname, 
                        mnt_struct->mnt_opts);*/

        auto label = g_strdup_printf("%s", mnt_struct->mnt_dir);
        auto utf_name = Basic::utf_string(label);
        TRACE("label=%s, utf_name=%s\n", label, utf_name);
        g_free(label);
        
        const char *path = mnt_struct->mnt_dir;
        GFile *file = g_file_new_for_path(path);
        GError *error_ = NULL;
        auto info = g_file_query_info(file, "standard::", G_FILE_QUERY_INFO_NONE, NULL, &error_);
        g_file_info_set_attribute_object(info, "standard::file", G_OBJECT(file));          
        g_file_info_set_attribute_object(info, "xffm::fstabMount", G_OBJECT(file));
        auto base = (strcmp(utf_name, "/")==0)?g_strdup(utf_name): g_path_get_basename(utf_name);
        g_file_info_set_name(info, base);
        g_free(base);
        g_free(utf_name);
        TRACE("info name=%s\n", g_file_info_get_name(info));

        FstabUtil::setMountableIcon(info, path);
        auto flags = Settings::getInteger("Disk Mounter","flags", 0);
        g_list_store_insert_sorted(store, G_OBJECT(info), LocalDir::compareFunction, GINT_TO_POINTER(flags));

    } 

    static void clearFstabList(GList *list){
      for (auto l=list; l && l->data; l= l->next){          
          auto mnt_struct = (struct mntent *)l->data;
          g_free(mnt_struct->mnt_fsname);
          g_free(mnt_struct->mnt_dir);
          g_free(mnt_struct->mnt_type);
          g_free(mnt_struct->mnt_opts);
          g_free(mnt_struct);
      }
      g_list_free(list);
    }

    static void
    addFstabItems(GListStore *store){
        auto list = getFstabItems();
        for (auto l=list; l && l->data; l= l->next){
          addListItem(store, l->data);
        }
        clearFstabList(list);
    }

    static const gchar **mntTypes(void){
        // Valid mount types...
        static const gchar *types[]={
            "ext2",
            "ext3",
            "ext4",
            "nfs",
            "nfs3",
            "nfs4",
            "ntfs",
            "ntfs-3g",
            "fuse",
            "fuse3",
            NULL
        };
        return types;
    }

    static GList *
    getFstabItems (void) {
        GList *list = NULL;
        const gchar *files[] = { "/etc/fstab", "/etc/mtab", NULL };
        struct mntent *mnt_struct;
        for(auto p = files; p && *p; p++) {
            TRACE ("FSTAB:  parsing %s\n", *p);
            FILE *fstab_fd = setmntent (*p, "r");
            struct mntent mntbuf;
            gchar buf[2048]; 
            while ((mnt_struct = getmntent_r (fstab_fd, &mntbuf, buf, 2048)) 
                    != NULL) 
            {
                if(strcmp (*p, "/etc/mtab") == 0) {
                    TRACE ("MTAB: setting MTAB type for %s\n", "foo");
                } else {
                }
                gboolean ok = FALSE;
                for (auto t=mntTypes(); t && *t; t++){
                    if (strcmp(mnt_struct->mnt_type, *t)==0) {
                        TRACE("\"%s\" ? \"%s\"\n", mnt_struct->mnt_type, *t);
                        ok = TRUE; 
                        break;
                    }
                }
                for (auto q=list; q && q->data; q=q->next){
                    // Just list first item, the one in fstab has preference.
                    auto v = (struct mntent *)q->data;
                    if (strcmp(v->mnt_dir, mnt_struct->mnt_dir) == 0) ok = FALSE;
                }
                if (!ok) {
                    TRACE("%s: not ok\n", mnt_struct->mnt_type);
                    continue;
                }

                //xfdir_p->gl[i].pathv = g_strdup (mnt_struct->mnt_dir);
                TRACE ("FSTAB: %s: mnt_fsname=%s, mnt_dir= %s, mnt_type=%s, mnt_opts=%s\n",
                        *p, mnt_struct->mnt_fsname, 
                        mnt_struct->mnt_dir, 
                        mnt_struct->mnt_type, 
                        mnt_struct->mnt_opts);

                if(strstr (mnt_struct->mnt_opts, "user")) {
                    //SET_USER_TYPE (xfdir_p->gl[i].en->type);
                }
                /* set type */
                auto mnt = (struct mntent *)calloc(1,sizeof(struct mntent));
                if (!mnt) {
                    ERROR_("getFstabItems():: calloc: %s\n", strerror(errno));
                    return NULL;
                }
                mnt->mnt_fsname = g_strdup(mnt_struct->mnt_fsname);
                mnt->mnt_dir = g_strdup(mnt_struct->mnt_dir); 
                mnt->mnt_type = g_strdup(mnt_struct->mnt_type); 
                mnt->mnt_opts = g_strdup(mnt_struct->mnt_opts);
                TRACE("append %s\n", mnt_struct->mnt_dir);
                
                list = g_list_append(list, (void *) mnt);
            }
            (void)endmntent (fstab_fd);
        }
        return list;
    }

    static void
    addPartition(GListStore *store, const gchar *path){
        if (!path){
            ERROR_("fstab/view.hh::addPartition: path cannot be null\n");
            return;
        }
#if 1
        gchar *basename = g_path_get_basename(path);
        auto label = FstabUtil::e2Label(basename);
        gboolean mounted = isMounted(path);
        if (label){
            gchar *g = g_strdup_printf("%s\n(%s)", basename, label);
            g_free(label);
            label = g;
           g_free(basename);
        } else {
           label = basename;
        }
        auto utf_name = Basic::utf_string(label);
        g_free(label);
#else

        char *basename = g_path_get_basename(path);
        auto utf_name = Basic::utf_string(basename);
        g_free(basename);
#endif
        GFile *file = g_file_new_for_path(path);
        GError *error_ = NULL;
        auto info = g_file_query_info(file, "standard::", G_FILE_QUERY_INFO_NONE, NULL, &error_);
        g_file_info_set_attribute_object(info, "standard::file", G_OBJECT(file));          
        g_file_info_set_attribute_object(info, "xffm::fstabMount", G_OBJECT(file));
        g_file_info_set_name(info, utf_name);
        g_free(utf_name);

            
        FstabUtil::setMountableIcon(info, path);        
        auto flags = Settings::getInteger("Disk Mounter","flags", 0);
      

        g_list_store_insert_sorted(store, G_OBJECT(info), LocalDir::compareFunction, GINT_TO_POINTER(flags));

        // gchar *mntDir = getMntDir(path);// not used here, maybe used for properties (FIXME)

        // auto fstype = fsType(path); // not used here, maybe used for properties (FIXME)       
        // gchar *fileInfo = Basic::fileInfo(path);// not used here, maybe used for properties (FIXME)
        /* text not used in info set up. 
         * maybe used later for properties (FIXME)
        char *text = g_strdup_printf("** %s (%s):\n%s\n%s %s\n%s",
                        basename, 
                        label?label:_("No Label"),
                        fstype?fstype:_("There is no file system available (unformatted)"),
                        _("Mount point:"), mounted?mntDir:_("Not mounted"),
                        fileInfo);
                        */
        //g_free(mntDir);
        //g_free(fstype);



        //auto id = partition2Id(path);// XXX not used
        



        //g_free(id);
        // path is constant
        //g_free(text);
    }

    //////////////////////////////////
    static gchar *
    partition2Id(const gchar *partition){ // disk partition only
       if (!g_file_test("/dev/disk/by-id", G_FILE_TEST_IS_DIR)){
           return g_strdup("partition2Id(): not -e /dev/disk/by-id ");
       }
        
        gchar *base = g_path_get_basename(partition);

        const gchar *command = "ls -l /dev/disk/by-id";
        FILE *pipe = popen (command, "r");
        if(pipe == NULL) {
            ERROR_("fstab/view.hh::Cannot pipe from %s\n", command);
            return NULL;
        }
        gchar line[256];
        memset(line, 0, 256);
        gchar *id = NULL;
        while (fgets (line, 255, pipe) && !feof(pipe)) {
            if (!strstr(line, base)) {
                TRACE("%s not in %s\n", base, line);
                continue;
            }
            gchar **f = g_strsplit(line, "->", 2);
            if (!strstr(f[1], base)){
                TRACE("%s not in %s\n", base, f[1]); 
                g_strfreev(f);
                continue;
            }
            g_strstrip(f[0]);
            if (!strrchr(f[0], ' ')){
                ERROR_("fstab/view.hh::partition2Id(): no space-chr in id\n");
                continue;
            }
            id = g_path_get_basename(strrchr(f[0], ' ')+1);
            g_strfreev(f);
            break;
        }
        pclose (pipe);
        TRACE("partition2Id() %s->%s\n", partition, id);
        return id;
    }

    static gchar *
    getMntDir (const gchar * mnt_fsname) {
        if (!mnt_fsname) return NULL;
        FILE *fstab_fd = setmntent ("/etc/mtab", "r");
        if(!fstab_fd)
            return NULL;
        struct mntent *mnt_struct;
        gchar *mnt_dir = NULL;
        struct mntent mntbuf;
        gchar buf[2048]; 
        while ((mnt_struct = getmntent_r (fstab_fd, &mntbuf, buf, 2048)) != NULL) {
            if(strcmp (mnt_fsname, mnt_struct->mnt_fsname) == 0) {
                // hit: multiple entries use first listed 
                // user types have preference and use last listed 
                if(strstr (mnt_struct->mnt_opts, "user")) {
                    g_free (mnt_dir);
                    mnt_dir = g_strdup (mnt_struct->mnt_dir);
                }
                if(!mnt_dir) {
                    mnt_dir = g_strdup (mnt_struct->mnt_dir);
                }
            }
        }
        (void)endmntent (fstab_fd);
        return mnt_dir;
    }


    static gchar *
    fsType(const gchar *partitionPath){
        auto lsblk = g_find_program_in_path("lsblk");
        if (!lsblk) return NULL;
        gchar *command = g_strdup_printf("%s -no FSTYPE %s", lsblk, partitionPath);
        g_free(lsblk);
        FILE *pipe = popen (command, "r");
        if(pipe == NULL) {
            ERROR_("fstab/view.hh::Cannot pipe from %s\n", command);
            g_free(command);
            return NULL;
        }
        g_free(command);

        gchar line[256];
        memset(line, 0, 256);
        while (fgets (line, 255, pipe) && !feof(pipe)) {
            if (strchr(line,'\n')) *strchr(line,'\n') = 0;
            if (strstr(line, "swap")) return NULL;
            if (strcmp(line, "")==0) return NULL;
            break;
        }
        pclose (pipe);
        return g_strdup(line);
    }

  public:

    static gboolean
    isInFstab (const gchar *path) {
        if (!path){
            ERROR_("fstab/view.hh::isInFstab() path is null\n");
            return FALSE;
        }
        struct mntent *mnt_struct;
        FILE *fstab_fd;
        gboolean result = FALSE;
        if((fstab_fd = setmntent ("/etc/fstab", "r")) == NULL) {
            ERROR_ ("fstab/view.hh::isInFstab(): Unable to open %s\n", "/etc/fstab");
            return result;
        }

        struct mntent mntbuf;
        gchar buf[2048]; 
        while ((mnt_struct = getmntent_r (fstab_fd, &mntbuf, buf, 2048)) != NULL) {
            if(strstr (mnt_struct->mnt_type, MNTTYPE_SWAP))
                continue;
            if(!g_file_test (mnt_struct->mnt_dir, G_FILE_TEST_IS_DIR))
                continue;

            TRACE("isInFstab():%s --->  %s   or   %s\n", 
                    path, mnt_struct->mnt_dir, mnt_struct->mnt_fsname);

            if(strcmp (path, mnt_struct->mnt_dir) == 0 || 
                    strcmp (path, mnt_struct->mnt_fsname) == 0) {
                if (FstabUtil::getFstabType (mnt_struct->mnt_type)) result = TRUE;
                TRACE("isInFstab():%s ---> %d %s\n", 
                        mnt_struct->mnt_fsname, result, mnt_struct->mnt_type);
                break;
            }
        }

        (void)endmntent (fstab_fd);
        return result;
    }
  
    static gboolean
    isMounted (const gchar *mnt_fsname) {

        if(!mnt_fsname) {
            ERROR_ ("fstab/view.hh::isMounted() mnt_point != NULL not met!\n");
            return FALSE;
        }
        gchar *mnt_point;
        if (g_path_is_absolute(mnt_fsname)) {
            mnt_point = realpath(mnt_fsname, NULL);
        } else {
            mnt_point = g_strdup(mnt_fsname);
        }
        TRACE("test for mount status: %s\n", mnt_point);
        
        struct mntent *m;
        //const gchar *mnttab;
        FILE *tab_file;

        // try both /etc/mtab and /proc/mounts 
        const gchar *mfile[]={"/proc/mounts", "/etc/mtab", NULL};
        //const gchar *mfile[]={"/proc/mounts", NULL};
        const gchar **pfile;
        for (pfile=mfile; pfile && *pfile; pfile++){
            TRACE("From /proc/mounts and /etc/mtab: %s\n", *pfile);
            if((tab_file = fopen (*pfile, "r")) == NULL) {
                ERROR_("%s: %s\n", strerror(ENOENT), *pfile);
                continue;
            }
            fclose(tab_file);
            tab_file = setmntent (*pfile, "r");

            if(!tab_file) {
                perror ("setmntent:");
                g_free(mnt_point);
                return FALSE;
            }
            struct mntent mntbuf;
            gchar buf[2048]; 
            while ((m = getmntent_r (tab_file, &mntbuf, buf, 2048)) != NULL) {        
                TRACE(".isMounted():%s:  %s  or  %s\n", mnt_point, m->mnt_dir, m->mnt_fsname);
                if((mnt_point && strcmp (m->mnt_dir, mnt_point) == 0) || 
                   (mnt_point && strcmp (m->mnt_fsname, mnt_point) == 0)) {
                   //(mnt_fsname && strcmp (m->mnt_fsname, mnt_fsname) == 0)) {
                    TRACE("%s ..isMounted(): mnt_dir=%s  mnt_fsname=%s mnt_point=%s\n", 
                            *pfile, m->mnt_dir, m->mnt_fsname, mnt_point);
                    endmntent (tab_file);
                    g_free(mnt_point);
                    return TRUE;
                }
            }
            endmntent (tab_file);
        }
        g_free(mnt_point);
        return FALSE;
    }
};
}
#endif
