#ifndef MENU_POPOVER_HH
#define MENU_POPOVER_HH
#include "dialogsignals.hh"
namespace xf {
template <class Type> class Notebook;
template <class Type> class Page;
template <class Type>
class MenuPopoverSignals {
    using run_c = Run<Type>;
public:
    static void
    root(GtkMenuItem *menuItem, gpointer data)
    {
	auto notebook_p = (Dialog<Type> *)g_object_get_data(G_OBJECT(mainWindow), "dialogObject");

        const gchar *path = notebook_p->workdir();
        auto page = (Page<Type> *)notebook_p->currentPageObject();
        page->setPageWorkdir("/");
        page->view()->loadModel("/");
    }

    static void
    home(GtkMenuItem *menuItem, gpointer data)
    {
	auto notebook_p = (Dialog<Type> *)g_object_get_data(G_OBJECT(mainWindow), "dialogObject");

        const gchar *path = notebook_p->workdir();
        auto page = (Page<Type> *)notebook_p->currentPageObject();
        page->setPageWorkdir(g_get_home_dir());
        page->view()->loadModel(g_get_home_dir());
    }

    static void
    fstab(GtkMenuItem *menuItem, gpointer data)
    {
	auto notebook_p = (Dialog<Type> *)g_object_get_data(G_OBJECT(mainWindow), "dialogObject");

        const gchar *path = notebook_p->workdir();
        auto page = (Page<Type> *)notebook_p->currentPageObject();
        page->setPageWorkdir(g_get_home_dir());
        page->view()->loadModel("xffm:fstab");
    }

    static void
    pkg(GtkMenuItem *menuItem, gpointer data)
    {
	auto notebook_p = (Dialog<Type> *)g_object_get_data(G_OBJECT(mainWindow), "dialogObject");

        const gchar *path = notebook_p->workdir();
        auto page = (Page<Type> *)notebook_p->currentPageObject();
        page->setPageWorkdir(g_get_home_dir());
        page->view()->loadModel("xffm:pkg");
    }

    static void
    trash(GtkMenuItem *menuItem, gpointer data)
    {
	auto notebook_p = (Dialog<Type> *)g_object_get_data(G_OBJECT(mainWindow), "dialogObject");

        const gchar *path = notebook_p->workdir();
        auto page = (Page<Type> *)notebook_p->currentPageObject();
        page->setPageWorkdir(g_get_home_dir());
	auto name = g_strdup_printf("%s/.local/share/Trash/files", g_get_home_dir());
        page->setPageWorkdir(name);
        page->view()->loadModel(name);
        g_free(name);
    }

    static void run(Notebook<Type> *notebook_p, const gchar *command){
        const gchar *path = notebook_p->workdir();
        auto page = (Page<Type> *)notebook_p->currentPageObject();
        if (!path || !g_file_test(path, G_FILE_TEST_IS_DIR)) path = g_get_home_dir();
	gchar *c = g_strdup(command);
        gchar *oldDir = g_get_current_dir ();
        chdir(path);
        pid_t child = run_c::thread_run(page->output(), c, FALSE);
	page->newRunButton(c, child);
        g_free(c);
        chdir(oldDir);
        g_free(oldDir);
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
	auto notebook_p = (Dialog<Type> *)g_object_get_data(G_OBJECT(mainWindow), "dialogObject");

        if (userTerminal){
            run(notebook_p, userTerminal);
            g_free(userTerminal);
        } else {
            run(notebook_p, "xterm -vb");
        }
    }
   
    static void
    newWindow(GtkMenuItem *menuItem, gpointer data)
    {
	auto xffm = g_find_program_in_path("xffm");
	if (!xffm) {
	    ERROR("Cannot find xffm in path\n");
	    return;
	}
	auto notebook_p = (Dialog<Type> *)g_object_get_data(G_OBJECT(mainWindow), "dialogObject");

        auto page = (Page<Type> *)notebook_p->currentPageObject();
	run(notebook_p, xffm);
	g_free(xffm);
    }

    static void
    search(GtkMenuItem *menuItem, gpointer data)
    {
        // get current directory
	auto notebook_p = (Dialog<Type> *)g_object_get_data(G_OBJECT(mainWindow), "dialogObject");

        const gchar *path = notebook_p->workdir();
        if (!path || !g_file_test(path, G_FILE_TEST_IS_DIR)) path = g_get_home_dir();
	gchar *find = g_strdup_printf("xffm --find \"%s\"", path);
        run(notebook_p, find);
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

        menuButton_ = GTK_MENU_BUTTON(gtk_menu_button_new());
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
    private:

    GtkMenu *createMenu(void){
	Settings<Type>::readSettings();
        menuItem_t item[]={
            {N_("View as list"), (void *)toggleView, (void *)"TreeView", "window"},
#if 0
            //{N_("Root folder"), (void *)MenuPopoverSignals<Type>::root, (void *) menuButton_},
            {N_("Home Directory"), (void *)MenuPopoverSignals<Type>::home, (void *) menuButton_},
            {N_("Disk Image Mounter"), (void *)MenuPopoverSignals<Type>::fstab, (void *) menuButton_},
            {N_("Software Updater"), (void *)MenuPopoverSignals<Type>::pkg, (void *) menuButton_},
            {N_("Trash bin"), (void *)MenuPopoverSignals<Type>::trash, (void *) menuButton_},
            {N_("Open terminal"), (void *)MenuPopoverSignals<Type>::terminal, (void *) menuButton_},
            {N_("Open a New Window"), (void *)MenuPopoverSignals<Type>::newWindow, (void *) menuButton_},
            {N_("Search"), (void *)MenuPopoverSignals<Type>::search, (void *) menuButton_},
#endif
            {N_("Exit"), (void *)MenuPopoverSignals<Type>::finish, (void *) menuButton_},
            {NULL}};
       
	auto menu =  BasePopUp<Type>::createPopup(item);
	auto title = GTK_MENU_ITEM(g_object_get_data(G_OBJECT(menu), "title"));
	gtk_widget_set_sensitive(GTK_WIDGET(title), FALSE);
	gchar *markup = g_strdup_printf("<span color=\"blue\" size=\"large\">%s</span>", _("Main menu"));
	Gtk<Type>::menu_item_content(title, NULL, markup, -24);
	g_free(markup);
        gtk_widget_show(GTK_WIDGET(title));
#if 0
        auto menu = GTK_MENU(gtk_menu_new());
        auto p = item;
        gint i;
        for (i=0;p && p->label; p++,i++){
            GtkWidget *v = gtk_menu_item_new_with_label (_(p->label));
            gtk_container_add (GTK_CONTAINER (menu), v);
            g_signal_connect ((gpointer) v, "activate", MENUITEM_CALLBACK (p->callback), p->callbackData);
            gtk_widget_show (v);
        }
#endif
#if 10
	const gchar *smallKey[]={
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
        const gchar *smallIcon[]={
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
        gint i=0;
        for (auto k=smallKey; k && *k; k++, i++){
            auto mItem = (GtkMenuItem *)g_object_get_data(G_OBJECT(menu), *k);
	    if (!mItem) continue;
            auto markup = g_strdup_printf("<span size=\"small\">%s</span>", _(*k));
	    Gtk<Type>::menu_item_content(mItem, smallIcon[i], markup, -16);
	    g_free(markup);
        }
#endif
	
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
        auto notebook_p = (Notebook<Type> *)g_object_get_data(G_OBJECT(mainWindow), "xffm");
	gint pages = gtk_notebook_get_n_pages (notebook_p->notebook());
	for (int i=0; i<pages; i++){
            auto page = notebook_p->currentPageObject(i);
            auto view = page->view();
            view->reloadModel();
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
