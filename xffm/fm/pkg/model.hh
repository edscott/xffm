#ifndef XF_PKGMODEL__HH
# define XF_PKGMODEL__HH

#ifdef HAVE_EMERGE 
# include "emerge.hh"
#else
# ifdef HAVE_PACMAN
#  include "pacman.hh"
# else
#  include "pkg.hh"
# endif
#endif

namespace xf
{
static pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;
static GList *l_list=NULL;
static pthread_cond_t l_signal = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t l_mutex = PTHREAD_MUTEX_INITIALIZER;
static int l_condition=0;

template <class Type>
class PkgModel  {

    using pixbuf_c = Pixbuf<double>;
    using util_c = Util<double>;
public:

    static void
    addPkgItem(GtkTreeModel *treeModel){
#if !defined HAVE_PACMAN && !defined HAVE_EMERGE && !defined HAVE_PKG
# warning "Package manager only with pkg, emerge or pacman"
	return;
#endif
 	GtkTreeIter iter;
	auto name = "xffm:pkg";
	auto utf_name = util_c::utf_string(_("Software Updater"));

	//auto icon_name = "emblem-downloads/SE//2.0/225";

	auto icon_name = g_strconcat("emblem-downloads/SE/", PKG_EMBLEM, "/2.0/225", NULL);
	auto highlight_name = g_strconcat(icon_name, "/NE/folder-open/2.0/225", NULL);

        auto treeViewPixbuf = Pixbuf<Type>::get_pixbuf(icon_name,  -24);
	auto normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  -48);
	auto highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  -48); 
	auto tooltipText = g_strdup_printf("%s",
		_("Add or remove software installed on the system"));

	gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
	gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
		DISPLAY_NAME, PKG_EXEC, //utf_name,
                PATH, name,
		ICON_NAME, icon_name,
                TREEVIEW_PIXBUF, treeViewPixbuf, 
		DISPLAY_PIXBUF, normal_pixbuf,
		NORMAL_PIXBUF, normal_pixbuf,
		HIGHLIGHT_PIXBUF, highlight_pixbuf,
		TOOLTIP_TEXT,_("Add or remove software installed on the system"),
		-1);
	g_free(utf_name);
	g_free(icon_name);
	g_free(highlight_name);
    }


    static void
    clearModel(GtkTreeModel *treeModel){
        while (gtk_events_pending()) gtk_main_iteration();
 	GtkTreeIter iter;
	if (gtk_tree_model_get_iter_first (treeModel, &iter)){
	    while (gtk_list_store_remove (GTK_LIST_STORE(treeModel),&iter));
	}
    }

    static gboolean
    loadModel (GtkTreeModel *treeModel)
    {
	TRACE("mk_tree_model:: model = %p\n", treeModel);
        clearModel(treeModel);
	addXffmItem(treeModel);
#ifdef HAVE_EMERGE
        Emerge<Type>::addDirectories(treeModel);      
#else
# ifdef HAVE_PACMAN
        Pkg<Type>::addDirectories(treeModel);      
# else // pkg
        Pkg<Type>::addDirectories(treeModel);         
# endif
#endif
        addSearchItem(treeModel);
        addPackages(treeModel);
	return TRUE;
    }

    static gboolean
    loadModel (GtkTreeModel *treeModel, const gchar *path)
    {
        if (strncmp(path,"xffm:pkg",strlen("xffm:pkg"))!=0) return FALSE;
        if (strcmp(path,"xffm:pkg")==0) return loadModel(treeModel);
        if (strcmp(path,"xffm:pkg:search")==0){

            return loadSearch(treeModel);
        }

	return FALSE;
    }

    static gboolean
    loadSearch(GtkTreeModel *treeModel){
        auto markup = 
            g_strdup_printf("<span color=\"blue\" size=\"larger\"><b>%s</b></span>", 
                    PKG_SEARCH);

        auto entryResponse = new(EntryResponse<Type>)(GTK_WINDOW(mainWindow), _("Search"), NULL);
        entryResponse->setResponseLabel(markup);
        g_free(markup);
        entryResponse->setEntryLabel(_("String"));
        auto response = entryResponse->runResponseInsensitive();
        delete entryResponse;
	auto dialog_p = (Dialog<Type> *)g_object_get_data(G_OBJECT(mainWindow), "xffm");
	auto page = dialog_p->currentPageObject();
	page->updateStatusLabel(_("Waiting for search results"));

	while (gtk_events_pending())gtk_main_iteration();	
        TRACE("response=%s\n", response);
        if (!response) {
	    gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), TRUE);
	    return FALSE;
	}
        g_strstrip(response);
        gint count = 0;
        if (strlen(response)){

            TRACE("search string: %s\n", response);
	    gchar *command;
	    GList *pkg_list;
            GHashTable *installedHash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
#ifdef HAVE_PACMAN
            command = g_strdup_printf("%s %s", PKG_SEARCH_LOCAL, response);
            TRACE("command: %s\n", command);
            // Installed stuff
            pkg_list = get_command_listing(command, TRUE);
            g_free(command);
            pkg_list = g_list_reverse(pkg_list);
            for (auto l=pkg_list; l && l->data; l=l->next){
                TRACE("installed: %s\n", (gchar *)l->data);
                g_hash_table_insert(installedHash, l->data, GINT_TO_POINTER(1));
            }
            count += g_list_length(pkg_list);
            g_list_free(pkg_list);
#endif
            // Repository stuff
            command = g_strdup_printf("%s %s", PKG_SEARCH, response);
	    TRACE("command=%s\n",command); 
            pkg_list = get_command_listing(command, TRUE);
            g_free(command);
            count += g_list_length(pkg_list);

            if (count) {
                clearModel(treeModel);
                addSearchItem(treeModel);
                pkg_list = g_list_reverse(pkg_list);
                for (auto l=pkg_list; l && l->data; l=l->next){
		    addResultPackage(treeModel, (const gchar *)l->data, 
			    g_hash_table_lookup(installedHash,l->data));
	            g_free(l->data);
                }
                g_list_free(pkg_list);
	    
		gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), TRUE);
		auto m = g_strdup_printf("%s %d", "Matching results...", count);
		page->updateStatusLabel(m);
		g_free(m);
		g_hash_table_destroy(installedHash);
		
                return TRUE;
            }
            g_hash_table_destroy(installedHash);
            Gtk<Type>::quickHelp(mainWindow, _("No results"), "dialog-warning");
        } 
	page->updateStatusLabel(NULL);
	gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), TRUE);
        return FALSE;
    }

    static void
    addResultPackage(GtkTreeModel *treeModel, const gchar *package, void *local){
	auto icon_name = "package-x-generic/NW/" PKG_EMBLEM "/2.0/225";
	auto highlight_name = "package-x-generic/NW/" PKG_EMBLEM "/2.0/225";
	if (local) {
	    icon_name = "package-x-generic/NW/" "greenball" "/2.0/225";
	    highlight_name = "package-x-generic/NW/" "greenball" "/2.0/225";
	}
	auto treeViewPixbuf = Pixbuf<Type>::get_pixbuf(icon_name,  -24);
	auto normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  -48);
	auto highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  -48);   
	GtkTreeIter iter;
	gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
	gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
		DISPLAY_NAME, strchr(package, '/')?
		    strchr(package, '/')+1:package,
		ICON_NAME, icon_name,
		PATH, package,
		TREEVIEW_PIXBUF, treeViewPixbuf, 
		DISPLAY_PIXBUF, normal_pixbuf,
		NORMAL_PIXBUF, normal_pixbuf,
		HIGHLIGHT_PIXBUF, highlight_pixbuf,
		TOOLTIP_TEXT,package,

		-1);
    }

    static void 
    addPackages(GtkTreeModel *treeModel){
        GList *pkg_list = get_command_listing(PKG_LIST, FALSE);
        pkg_list = g_list_reverse(pkg_list);

	auto icon_name = "package-x-generic/NW/" "greenball" "/2.0/225";
	auto highlight_name = "package-x-generic/NW/" "greenball" "/2.0/225";
        auto treeViewPixbuf = Pixbuf<Type>::get_pixbuf(icon_name,  -24);
	auto normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  -48);
	auto highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  -48);   
        GtkTreeIter iter;
            TRACE("pacman: %s\n", "reloading pkg icons...");
        for (auto l=pkg_list; l && l->data; l=l->next){
	    auto package = (const gchar *)l->data;
            TRACE("pacman: %s\n", (gchar *)l->data);
            auto name = g_strconcat("xffm:pkg:",(const gchar *)l->data, NULL);

            gtk_list_store_append (GTK_LIST_STORE(treeModel), &iter);
            gtk_list_store_set (GTK_LIST_STORE(treeModel), &iter, 
                    DISPLAY_NAME, strchr(package, '/')?
				strchr(package, '/')+1:package,
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
        g_list_free(pkg_list);
        
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
                TRACE("io_search_stdout(): %s\n", line);
            return;
        }
#ifdef HAVE_EMERGE
        l_list = Emerge<Type>::addSearchItems(l_list, (const gchar *)line);
        return;
#else
# ifdef HAVE_PACMAN
        l_list = Pacman<Type>::addSearchItems(l_list, (const gchar *)line);
        return;
# else // pkg
        l_list = Pkg<Type>::addSearchItems(l_list, (const gchar *)line);
        return;
# endif
#endif
        
    }

    static void io_thread_stdout(void *user_data, void *line, int childFD){
        if (check_exit((const gchar *)line)){
            TRACE("io_thread_stdout(): %s\n", line);
            return;
        }
#ifdef HAVE_EMERGE 
        l_list = Emerge<Type>::addPackage(l_list, (const gchar *)line);
        return;
#else
# ifdef HAVE_PACMAN
        l_list = Pacman<Type>::addPackage(l_list, (const gchar *)line);
        return;
# else // pkg
        l_list = Pacman<Type>::addPackage(l_list, (const gchar *)line);
        return;
# endif
#endif
    }

    static GList *
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

	l_list = Util<Type>::sortList(l_list);
        return l_list;
    }

 
  
};
}
#endif
