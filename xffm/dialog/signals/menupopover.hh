#ifndef MENU_POPOVER_SIGNALS_HH
#define MENU_POPOVER_SIGNALS_HH
#include "common/run.hh"

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
            run(notebook_p, "xterm");
        }
    }

    static void
    search(GtkMenuItem *menuItem, gpointer data)
    {
        // get current directory
        auto notebook_p = (Notebook<Type> *)g_object_get_data(G_OBJECT(data), "notebook_p");
        const gchar *path = notebook_p->workdir();
        if (!path || !g_file_test(path, G_FILE_TEST_IS_DIR)) path = g_get_home_dir();
	gchar *find = g_strdup_printf("xfterm --find \"%s\"", path);
        run(notebook_p, find);
        g_free(find);
    }

    static void
    finish(GtkMenuItem *menuItem, gpointer data)
    {
        TRACE("finish\n");
        // The following does not retrieve the main window...
        //GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(menuItem));
        gtk_widget_hide(mainWindow);
        while (gtk_events_pending()) gtk_main_iteration();
        gtk_main_quit();
        exit(0);
    }


};


}



#endif
