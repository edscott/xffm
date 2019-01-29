#ifndef XF_PKGMODEL__HH
# define XF_PKGMODEL__HH
//#ifdef HAVE_LIBXML2

namespace xf
{
static pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;
static GSList *l_list=NULL;
static pthread_cond_t l_signal = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t l_mutex = PTHREAD_MUTEX_INITIALIZER;
static int l_condition=0;

template <class Type>
class PkgModel  {

    using pixbuf_c = Pixbuf<double>;
    using util_c = Util<double>;
public:


    static gboolean
    loadModel (GtkTreeModel *treeModel)
    {
	WARN("mk_tree_model:: model = %p\n", treeModel);
        while (gtk_events_pending()) gtk_main_iteration();
 	GtkTreeIter iter;
	if (gtk_tree_model_get_iter_first (treeModel, &iter)){
	    while (gtk_list_store_remove (GTK_LIST_STORE(treeModel),&iter));
	}
	addXffmItem(treeModel);
#ifdef HAVE_PKG
        addPortsItem(treeModel);      
        addCacheItem(treeModel);      
#endif
        addSearchItem(treeModel);
        addPackages(treeModel);
	return TRUE;
    }

    static gboolean
    loadModel (GtkTreeModel *treeModel, GtkTreePath *tpath, const gchar *path)
    {
        WARN("pkg loadModel...\n");
        if (!strncmp(path,"xffm:pkg",strlen("xffm:pkg"))) return FALSE;
        WARN("pkg2 loadModel...\n");
        if (strcmp(path,"xffm:pkg")==0) return loadModel(treeModel);
        WARN("pkg22 loadModel...\n");
        if (strcmp(path,"xffm:pkg:search")==0){
        WARN("pkg222 loadModel...\n");
            auto markup = 
                g_strdup_printf("<span color=\"blue\" size=\"larger\"><b>%s</b></span>", "pacman -Ss");

            auto entryResponse = new(EntryResponse<Type>)(GTK_WINDOW(mainWindow), _("Search"), NULL);
            entryResponse->setResponseLabel(markup);
            g_free(markup);
            entryResponse->setEntryLabel(_("String"));
            auto response = entryResponse->runResponse();
            delete entryResponse;
            TRACE("response=%s\n", response);
            if (!response) return FALSE;
            g_strstrip(response);
            if (strlen(response)){
                WARN("search string: %s\n", response);
            }
	    return TRUE;
        }

	return FALSE;
    }


    static void 
    addPackages(GtkTreeModel *treeModel){
        const gchar *command;
#ifdef HAVE_PACMAN
        command = "pacman -Q";
#endif
        GSList *pkg_list = get_command_listing(command, FALSE);
        pkg_list = g_slist_reverse(pkg_list);

	auto icon_name = "package-x-generic/SE/" PKG_EMBLEM "/2.0/225";
	auto highlight_name = "package-x-generic/SE/" PKG_EMBLEM "/2.0/225";
        auto treeViewPixbuf = Pixbuf<Type>::get_pixbuf(icon_name,  -24);
	auto normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  -48);
	auto highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  -48);   
        GtkTreeIter iter;
            DBG("pacman: %s\n", "reloading pkg icons...");
        for (auto l=pkg_list; l && l->data; l=l->next){
            TRACE("pacman: %s\n", (gchar *)l->data);
            auto name = g_strconcat("xffm:pkg:",(const gchar *)l->data, NULL);

            gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
            gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
                    DISPLAY_NAME, (const gchar *)l->data,
                    ICON_NAME, icon_name,
                    PATH, name,
                    TREEVIEW_PIXBUF, treeViewPixbuf, 
                    DISPLAY_PIXBUF, normal_pixbuf,
                    NORMAL_PIXBUF, normal_pixbuf,
                    HIGHLIGHT_PIXBUF, highlight_pixbuf,
                    TOOLTIP_TEXT,name,

                    -1);

            g_free(l->data);
            g_free(name);
        }
        g_slist_free(pkg_list);
        
    }

    static gboolean enableDragSource(void){ return FALSE;}
    static gboolean enableDragDest(void){ return FALSE;}

    static const gchar *
    get_xfdir_iconname(void){
        return "emblem-downloads/SE/" PKG_EMBLEM "/2.0/225";
    }
    static void
    addXffmItem(GtkTreeModel *treeModel){
 	GtkTreeIter iter;
	// Root
	auto name = "xffm:root";
	auto utf_name = util_c::utf_string(".");
	auto icon_name = "go-up";
	auto highlight_name = "go-up/NW/go-up-symbolic/2.0/225";
        auto treeViewPixbuf = Pixbuf<Type>::get_pixbuf(icon_name,  -24);
	auto normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  -48);
	auto highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  -48);   
	gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
	gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
		DISPLAY_NAME, utf_name,
		ICON_NAME, icon_name,
                PATH, name,
                TREEVIEW_PIXBUF, treeViewPixbuf, 
		DISPLAY_PIXBUF, normal_pixbuf,
		NORMAL_PIXBUF, normal_pixbuf,
		HIGHLIGHT_PIXBUF, highlight_pixbuf,
		TOOLTIP_TEXT,"xffm:root",

		-1);
	g_free(utf_name);
    }

    static void
    addPortsItem(GtkTreeModel *treeModel){
 	GtkTreeIter iter;
	// Home
	auto name = g_get_home_dir();
	auto icon_name = "folder/SE/" PKG_EMBLEM;
	auto highlight_name = "folder/NE/document-open/2.0/225";
        auto treeViewPixbuf = Pixbuf<Type>::get_pixbuf(icon_name,  -24);
	auto normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  -48);
	auto highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  -48);   

	gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
	gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
		DISPLAY_NAME, "ports",
                PATH, "/usr/ports",
                ICON_NAME, icon_name,
                TREEVIEW_PIXBUF, treeViewPixbuf, 
		DISPLAY_PIXBUF, normal_pixbuf,
		NORMAL_PIXBUF, normal_pixbuf,
		HIGHLIGHT_PIXBUF, highlight_pixbuf,
		TOOLTIP_TEXT,"/usr/ports",
		-1);
    }


    static void
    addCacheItem(GtkTreeModel *treeModel){
 	GtkTreeIter iter;
	// Home
	auto name = g_get_home_dir();
	auto icon_name = "folder/SE/" PKG_EMBLEM;
	auto highlight_name = "folder/NE/document-open/2.0/225";
        auto treeViewPixbuf = Pixbuf<Type>::get_pixbuf(icon_name,  -24);
	auto normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  -48);
	auto highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  -48);   

	gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
	gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
		DISPLAY_NAME, "cache",
                PATH, "/var/cache/pkg",
		ICON_NAME, icon_name,
                TREEVIEW_PIXBUF, treeViewPixbuf, 
		DISPLAY_PIXBUF, normal_pixbuf,
		NORMAL_PIXBUF, normal_pixbuf,
		HIGHLIGHT_PIXBUF, highlight_pixbuf,
		TOOLTIP_TEXT,"/var/cache/pkg",
		-1);
    }
    
   
    static void
    addSearchItem(GtkTreeModel *treeModel){
 	GtkTreeIter iter;
	// Home
	auto name = g_get_home_dir();
	auto icon_name = "system-search";
	auto highlight_name = "system-search/NE/" PKG_EMBLEM "/2.0/225";
        auto treeViewPixbuf = Pixbuf<Type>::get_pixbuf(icon_name,  -24);
	auto normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  -48);
	auto highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  -48);   

	gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
	gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
		DISPLAY_NAME, _("Search"),
                PATH, "xffm:pkg:search",
		ICON_NAME, icon_name,
                TREEVIEW_PIXBUF, treeViewPixbuf, 
		DISPLAY_PIXBUF, normal_pixbuf,
		NORMAL_PIXBUF, normal_pixbuf,
		HIGHLIGHT_PIXBUF, highlight_pixbuf,
		TOOLTIP_TEXT,_("Search"),
		-1);
    }

    static GSList *add_pacman_search_item(GSList *pkg_list, const gchar *line){
        if (!strchr(line,'\n')) return pkg_list;
     //   fprintf(stderr, "DBG:%s", line);
    //     rfm_threaded_diagnostics(widgets_p,NULL,g_strdup(line));
        //*strchr(line,'\n')=0;
        if (*line != ' '){
            gchar **a = g_strsplit(line, " ", -1);
            // check a
            gchar *p = strchr(a[0], '/');
            // check p
            p++;
            auto path = g_strdup(p);
            pkg_list=g_slist_prepend(pkg_list,path);
            /*
            // local or remote?
            gchar *c = g_strdup_printf("pacman -Q %s", p);
            FILE *pipe = popen(c, "r");
            g_free(c);
            if (pipe){
                gchar line[256];
                memset (line, 0, 256);
                if (fgets(line, 255, pipe)) SET_LOCAL_TYPE(en->type);
                fclose(pipe);
            }
            g_hash_table_replace(installed_hash, g_strdup(en->path), g_strdup(line));
            */
            g_strfreev(a);
        } else {
            //the rest is tooltip material   
         /*   record_entry_t *en = pkg_list->data;
            gchar *tip = g_hash_table_lookup(installed_hash, en->path);
            gchar *new_tip = g_strconcat ((tip)?tip:"", line, NULL);
            g_hash_table_replace(installed_hash, g_strdup(en->path), new_tip);*/
        }
        return pkg_list;
    }

#ifdef HAVE_PACMAN
    static GSList *add_pacman_item(GSList *pkg_list, gchar *line){
        if (!strchr(line,'\n')) return pkg_list;
        TRACE("add_pacman_item(): %s", line);
        *strchr(line,'\n')=0;
        gchar **a = g_strsplit(line, " ", -1);
        if (!a[1]) {
            TRACE("add_pacman_item(): no vector...\n");
            g_strfreev(a);
            return pkg_list;
        }
        auto path = g_strdup(a[0]);
        TRACE("add_pacman_item(): a0=%s, version=%s\n",a[0], a[1]);
        
        pkg_list=g_slist_prepend(pkg_list,path);
        g_strfreev(a);
        return pkg_list;
    }
#endif


//	auto name = "xffm:pkg";
//	auto utf_name = util_c::utf_string(_("Software Updater"));

    static gboolean check_exit(const gchar *line){
        TRACE("check_exit(): %s", line);
        if(strncmp (line, "Tubo-id exit:", strlen ("Tubo-id exit:")) == 0) {
            // all stdout output has been processed.
            pthread_mutex_lock(&(l_mutex));
            l_condition = 1;
            pthread_mutex_unlock(&(l_mutex));
            TRACE("check_exit() Now signalling\n");
            pthread_cond_signal(&(l_signal));
            return TRUE;
        }
        return FALSE;
    }


    static void io_search_stdout(void *user_data, void *line, int childFD){
        if (check_exit((const gchar *)line)){
            //rfm_operate_stdout(user_data, line, childFD);
                DBG("io_search_stdout(): %s\n", line);
            return;
        }
#ifdef HAVE_PACMAN
        l_list = add_pacman_search_item(l_list, (const gchar *)line);
        return;
#endif
#if 0
        if (pkg) l_list = add_search_item(l_list, line, user_data);
        else if (emerge) l_list = add_emerge_search_item(l_list, line, user_data);
        else if (zypper) l_list = add_zypper_search_item(l_list, line, user_data);
        else if (yum) l_list = add_yum_search_item(l_list, line, user_data);
        else if (apt) l_list = add_apt_search_item(l_list, line, user_data);
        else if (pacman) l_list = add_pacman_search_item(l_list, line, user_data);
        else fprintf(stderr, "io_search_stdout(): no command process associated!\n");
#endif
        ERROR("io_search_stdout(): no command process associated!\n");
        
    }

    static void io_thread_stdout(void *user_data, void *line, int childFD){
        if (check_exit((const gchar *)line)){
            //rfm_operate_stdout(user_data, line, childFD);
            DBG("io_thread_stdout(): %s\n", line);
            return;
        }
#ifdef HAVE_PACMAN
        l_list = add_pacman_item(l_list, (gchar *)line);
        return;
#endif
#if 0
        if (pkg) l_list = add_pkg_item(l_list, line);
        else if (emerge) l_list = add_emerge_item(l_list, line);
        else if (rpm) l_list = add_rpm_item(l_list, line);
        else if (dpkg) l_list = add_apt_item(l_list, line);
        else if (pacman) l_list = add_pacman_item(l_list, line);
#endif
        ERROR("io_thread_stdout(): no command process associated!\n");
    }

    static GSList *
    get_command_listing(const gchar *command, gboolean search){
        if (!command) return NULL;
        if (pthread_mutex_trylock(&db_mutex)!=0){
            DBG(_("Currently busy\n"));
            return NULL;
        }
        l_list = NULL;
        TRACE("command is \"%s\"\n", command);
        // 1. create condition control structure
        //listing_control_t *l = get_listing_control();
        //l->list = &pkg_list;
        //l->search = search;
        // 2. create i/o thread
        gchar **arg = g_strsplit(command, " ", -1);
        if (!arg) {
            pthread_mutex_unlock(&db_mutex);
            return NULL;
        }
        //gchar **p=arg;for(;p && *p;p++)TRACE(stderr, "arg=\"%s\"\n", *p);
        l_condition=0;
        //if (search) 
          //  rfm_thread_run_argv_full (widgets_p, arg, FALSE, NULL, io_search_stdout, NULL, NULL);
        //else 
            //rfm_thread_run_argv_full (widgets_p, arg, FALSE, NULL, io_thread_stdout, NULL, NULL);
        
        if (search) 
            Run<Type>::thread_runReap(NULL,(const gchar**)arg, io_search_stdout, NULL, NULL);
        else 
            Run<Type>::thread_runReap(NULL,(const gchar**)arg, io_thread_stdout, NULL, NULL);
        g_strfreev(arg);
        // 3. wait on condition
        pthread_mutex_lock(&(l_mutex));
        if (!l_condition){
            TRACE( "waiting for signal\n");
            pthread_cond_wait(&(l_signal), &(l_mutex));
            TRACE("got signal!\n");
        }
        pthread_mutex_unlock(&(l_mutex));
        pthread_mutex_unlock(&db_mutex);
        TRACE("command listing routine is done...\n");
        return l_list;
    }

 
  
};
}
#endif
