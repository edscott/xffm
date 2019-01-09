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
    home(GtkMenuItem *menuItem, gpointer data)
    {
        auto notebook_p = (Notebook<Type> *)g_object_get_data(G_OBJECT(data), "notebook_p");
        const gchar *path = notebook_p->workdir();
        auto page = (Page<Type> *)notebook_p->currentPageObject();
        page->setPageWorkdir(g_get_home_dir());
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
        auto notebook_p = (Notebook<Type> *)g_object_get_data(G_OBJECT(data), "notebook_p");
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
        auto notebook_p = (Notebook<Type> *)g_object_get_data(G_OBJECT(data), "notebook_p");
        auto page = (Page<Type> *)notebook_p->currentPageObject();
	run(notebook_p, xffm);
	g_free(xffm);
    }

    static void
    search(GtkMenuItem *menuItem, gpointer data)
    {
        // get current directory
        auto notebook_p = (Notebook<Type> *)g_object_get_data(G_OBJECT(data), "notebook_p");
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
        // The following does not retrieve the main window...
        //GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(menuItem));
        gtk_widget_hide(GTK_WIDGET(mainWindow));
        while (gtk_events_pending()) gtk_main_iteration();
        gtk_main_quit();
        exit(0);
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
        menuItem_t item[]={
            {N_("Home"), (void *)MenuPopoverSignals<Type>::home, (void *) menuButton_},
            {N_("Open terminal"), (void *)MenuPopoverSignals<Type>::terminal, (void *) menuButton_},
            {N_("Open a New Window"), (void *)MenuPopoverSignals<Type>::newWindow, (void *) menuButton_},
            //{N_("Execute Shell Command"), (void *)MenuPopoverSignals<Type>::shell, (void *) menuButton_},
            {N_("Search"), (void *)MenuPopoverSignals<Type>::search, (void *) menuButton_},
            {N_("Exit"), (void *)MenuPopoverSignals<Type>::finish, (void *) menuButton_},
            {NULL}};
        
        auto menu = GTK_MENU(gtk_menu_new());
        auto p = item;
        gint i;
        for (i=0;p && p->label; p++,i++){
            GtkWidget *v = gtk_menu_item_new_with_label (_(p->label));
            gtk_container_add (GTK_CONTAINER (menu), v);
            g_signal_connect ((gpointer) v, "activate", MENUITEM_CALLBACK (p->callback), p->callbackData);
            gtk_widget_show (v);
        }
        gtk_widget_show (GTK_WIDGET(menu));
        return menu;
    }
   /* static void
    shell(GtkMenuItem *menuItem, gpointer data)
    {
    }*/



};


}



#endif
