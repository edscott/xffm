#ifndef MENU_POPOVER_HH
#define MENU_POPOVER_HH

namespace xf {
template <class Type> class Notebook;
template <class Type> class Page;
template <class Type>
class MenuPopoverSignals {
    using run_c = Run<Type>;
public:
    static void
    noop(GtkMenuItem *menuItem, gpointer data)
    {
        DBG("noop...\n");
    }

    static void
    root(GtkMenuItem *menuItem, gpointer data)
    {
        Fm<Type>::getCurrentPage()->setPageWorkdir("/");
        Fm<Type>::getCurrentView()->loadModel("/");
    }

    static void
    home(GtkMenuItem *menuItem, gpointer data)
    {
        Fm<Type>::getCurrentPage()->setPageWorkdir(g_get_home_dir());
        Fm<Type>::getCurrentView()->loadModel(g_get_home_dir());
    }

    static void
    fstab(GtkMenuItem *menuItem, gpointer data)
    {
        Fm<Type>::getCurrentPage()->setPageWorkdir(g_get_home_dir());
        Fm<Type>::getCurrentView()->loadModel("xffm:fstab");
    }

    static void
    pkg(GtkMenuItem *menuItem, gpointer data)
    {
        Fm<Type>::getCurrentPage()->setPageWorkdir(g_get_home_dir());
        Fm<Type>::getCurrentView()->loadModel("xffm:pkg");
    }

    static void
    trash(GtkMenuItem *menuItem, gpointer data)
    {
	auto name = g_strdup_printf("%s/.local/share/Trash/files", g_get_home_dir());
        Fm<Type>::getCurrentPage()->setPageWorkdir(name);
        Fm<Type>::getCurrentView()->loadModel(name);
        g_free(name);
    }

    static void runWd(const gchar *workdir, const gchar *command){
        if (!workdir) workdir = Fm<Type>::getCurrentWorkdir();
	gchar *c = g_strdup(command);
        gchar *oldDir = g_get_current_dir ();
        chdir(workdir);
        pid_t child = run_c::thread_run(Fm<Type>::getCurrentTextview(), c, FALSE);
	Fm<Type>::getCurrentPage()->newRunButton(c, child);
        g_free(c);
        chdir(oldDir);
        g_free(oldDir);
    }

    static void run(Notebook<Type> *notebook_p, const gchar *command){
        runWd(notebook_p?notebook_p->workdir():NULL, command);
        return;
        /*
        const gchar *path = notebook_p->workdir();
        if (!path || !g_file_test(path, G_FILE_TEST_IS_DIR)) path = g_get_home_dir();
	gchar *c = g_strdup(command);
        gchar *oldDir = g_get_current_dir ();
        chdir(path);
        pid_t child = run_c::thread_run(Fm<Type>::getCurrentTextview(), c, FALSE);
	Fm<Type>::getCurrentPage()->newRunButton(c, child);
        g_free(c);
        chdir(oldDir);
        g_free(oldDir);
        */
    }
    static void
    terminal(GtkMenuItem *menuItem, gpointer data)
    {
        gchar *userTerminal = NULL;
        const gchar *terminal = getenv("TERMINAL");
        if (terminal) {
            userTerminal = g_strdup(terminal);
            if (strchr(userTerminal, ' ')) *(strchr(userTerminal, ' ')) = 0;
            gchar *g = g_find_program_in_path(userTerminal);
            if (!g) {
                g_free(userTerminal);
                userTerminal = NULL;
            } else {
                g_free(g);
                g_free(userTerminal);
                userTerminal = g_strdup(terminal);
            }
        }
        if (userTerminal){
            run(Fm<Type>::getCurrentNotebook(), userTerminal);
            g_free(userTerminal);
        } else {
            run(Fm<Type>::getCurrentNotebook(), "xterm -vb");
        }
    }
   
   
    static void
    newWindow(GtkMenuItem *menuItem, gpointer data)
    {
	auto program = g_find_program_in_path((const gchar *) data);
	if (!program) {
	    ERROR("menupopover.hh::Cannot find % in path\n", (const gchar *) data);
	    return;
	}
	run(Fm<Type>::getCurrentNotebook(), program);
	g_free(program);
    }

    static void
    plainRun(GtkMenuItem *menuItem, gpointer data){
	run(Fm<Type>::getCurrentNotebook(), (const gchar *)data);
    }

    static void
    open(GtkMenuItem *menuItem, gpointer data)
    {
        auto diff = (const gchar *) data;
	auto program = g_find_program_in_path(diff);
	if (!program) {
	    ERROR("menupopover.hh::Cannot find %s in path\n", diff);
	    return;
	}
        if (strcmp((const gchar *) data, "xffm")==0){
            const gchar *path = Fm<Type>::getCurrentNotebook()->workdir(); 
            auto g = g_strdup_printf("%s %s", program, path);
            g_free(program);
            program = g;
        }

	run(Fm<Type>::getCurrentNotebook(), program);
	g_free(program);
    }

    static void
    search(GtkMenuItem *menuItem, gpointer data)
    {
        // get current directory

        const gchar *path = Fm<Type>::getCurrentNotebook()->workdir();
        if (!path || !g_file_test(path, G_FILE_TEST_IS_DIR)) path = g_get_home_dir();
	gchar *find = g_strdup_printf("xffm --find \"%s\"", path);
        run(Fm<Type>::getCurrentNotebook(), find);
        g_free(find);
    }

    static void
    finish(GtkMenuItem *menuItem, gpointer data)
    {
        TRACE("finish\n");
        DialogSignals<Type>::delete_event(GTK_WIDGET(mainWindow), NULL, NULL);
        //gtk_widget_hide(GTK_WIDGET(mainWindow));
        //while (gtk_events_pending()) gtk_main_iteration();
        //gtk_main_quit();
        //exit(0);
    }


};


template <class Type>
class MenuPopover {
public:
    MenuPopover(void) {

        menuButton_ = Gtk<Type>::newMenuButton("open-menu-symbolic", _("Open menu"));
#ifdef TRY_POPOVER
        auto popover = createPopover(GTK_WIDGET(menuButton_));
        gtk_menu_button_set_popover(menuButton_, GTK_WIDGET(popover));
#else
        auto menu = createMenu();
        gtk_menu_button_set_popup (menuButton_, GTK_WIDGET(menu));
        gtk_widget_set_can_focus (GTK_WIDGET(menuButton_), FALSE);
	gtk_button_set_relief (GTK_BUTTON(menuButton_), GTK_RELIEF_NONE);
#endif
 
        gtk_widget_show(GTK_WIDGET(menuButton_));
    }
    GtkMenuButton *menuButton(){ return menuButton_;}
protected:
    
    GtkMenuButton *menuButton_;

public:
    static gboolean
    toggleGroupItem(GtkCheckMenuItem *menuItem, const gchar *group, const gchar *item)
    {
        gboolean value; 
        if (Settings<Type>::getSettingInteger(group, item) > 0){
            value = FALSE;
        } else {
            value = TRUE;
        }
        gtk_check_menu_item_set_active(menuItem, value);
        Settings<Type>::setSettingInteger(group, item, value);
        auto notebook_p = Fm<Type>::getCurrentNotebook();
	gint pages = gtk_notebook_get_n_pages (Fm<Type>::getCurrentNotebook()->notebook());
	for (int i=0; i<pages; i++){
            auto page = notebook_p->currentPageObject(i);
            auto view = page->view();
            view->reloadModel();
	}
        return value;
    }

private:

    static void
    toggleItem(GtkCheckMenuItem *menuItem, gpointer data)
    {
        auto item = (const gchar *)data;
	toggleGroupItem(menuItem, "LocalView", item);
    }

    GtkMenu *createMenu(void){
	Settings<Type>::readSettings();
        menuItem_t item[]={
            {N_("View as list"), (void *)toggleView, (void *)"TreeView", "window"},
            {N_("Show hidden files"), (void *)toggleItem, (void *) "ShowHidden", "LocalView"},
            {N_("Show Backup Files"), (void *)toggleItem, (void *) "ShowBackups", "LocalView"},
            {N_("Sort data in descending order"), (void *)toggleItem, (void *) "Descending", "LocalView"},

            {N_("Sort by date"), (void *)MenuPopoverSignals<Type>::noop, (void *) NULL, NULL},
            {N_("Sort by size"), (void *)MenuPopoverSignals<Type>::noop, (void *) NULL, NULL},
            {N_("Exit"), (void *)MenuPopoverSignals<Type>::finish, (void *) menuButton_},
            {NULL}};
	const gchar *key[]={
            "Home Directory",
            "Disk Image Mounter",
            "Software Updater",
            "Trash bin",
            "Open terminal",
            "Open a New Window",
            "Search",
            "Exit",
            NULL
        };
        const gchar *keyIcon[]={
            "go-home",
            "folder-remote",
            "x-package-repository",
            "user-trash",

	    "utilities-terminal",
            "window-new",
            "system-search",
            "application-exit",
            NULL
        };
       
        
        auto popup = new(Popup<Type>)(item, key, keyIcon, TRUE);
        auto menu = popup->menu();


	auto title = GTK_MENU_ITEM(g_object_get_data(G_OBJECT(menu), "title"));
	gtk_widget_set_sensitive(GTK_WIDGET(title), FALSE);
	gchar *markup = g_strdup_printf("<span color=\"blue\" size=\"large\">%s</span>", _("Main menu"));
	Gtk<Type>::menu_item_content(title, NULL, markup, -24);
	g_free(markup);
        gtk_widget_show(GTK_WIDGET(title));
	
        g_signal_connect (G_OBJECT(menuButton_), "clicked", G_CALLBACK(updateMenu), g_object_get_data(G_OBJECT(menu), "View as list"));

        gtk_widget_show (GTK_WIDGET(menu));
        return menu;
    }
   /* static void
    shell(GtkMenuItem *menuItem, gpointer data)
    {
    }*/
private:
    static void
    toggleView(GtkCheckMenuItem *menuItem, gpointer data)
    {
        auto item = (const gchar *)data;
        isTreeView = !isTreeView;
        gtk_check_menu_item_set_active(menuItem, isTreeView);
        Settings<Type>::setSettingInteger("window", "TreeView", isTreeView);
        auto notebook_p = Fm<Type>::getCurrentNotebook();
	gint pages = gtk_notebook_get_n_pages (notebook_p->notebook());
	for (int i=0; i<pages; i++){
            auto page = notebook_p->currentPageObject(i);
            auto view = page->view();
#if 1
	    if (isTreeView){
		// hide iconview, show treeview
		gtk_widget_hide(GTK_WIDGET(view->page()->topScrolledWindow()));
		gtk_widget_show(GTK_WIDGET(view->page()->treeScrolledWindow()));
	    } else {
		// hide treeview, show iconview
		gtk_widget_hide(GTK_WIDGET(view->page()->treeScrolledWindow()));
		gtk_widget_show(GTK_WIDGET(view->page()->topScrolledWindow()));
	    }
#else
	    view->reloadModel();
#endif
	}
    }
    static void
    updateMenu(GtkButton *button, void *data){
        auto state = Settings<Type>::getSettingInteger("window", "TreeView");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(data), state);

    }



};


}



#endif
