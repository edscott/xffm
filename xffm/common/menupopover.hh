#ifndef MENU_POPOVER_HH
#define MENU_POPOVER_HH

#undef TRY_POPOVER
//#define TRY_POPOVER 1
#include "common/run.hh"

namespace xf {
template <class Type> class Notebook;
template <class Type> class Page;
template <class Type>
class MenuPopover {
    using run_c = Run<Type>;
public:
    MenuPopover(void) {

       menuButton_ = GTK_MENU_BUTTON(gtk_menu_button_new());
#ifdef TRY_POPOVER
       auto popover = createPopover(GTK_WIDGET(menuButton_));
       gtk_menu_button_set_popover(menuButton_, GTK_WIDGET(popover));
#else
       auto menu = createMenu();
       gtk_menu_button_set_popup (menuButton_, GTK_WIDGET(menu));
#endif
 
       gtk_widget_show(GTK_WIDGET(menuButton_));
    }
    GtkMenuButton *menuButton(){ return menuButton_;}
    protected:
    
    GtkMenuButton *menuButton_;
    private:
#ifdef TRY_POPOVER
    GtkPopover *createPopover(GtkWidget *widget){
        menuItem_t item[]={
            {N_("Home"), (void *)home, (void *) menuButton_},
            {N_("Open terminal"), (void *)terminal, (void *) menuButton_},
            {N_("Execute Shell Command"), (void *)shell, (void *) menuButton_},
            {N_("Search"), (void *)search, (void *) menuButton_},
            {N_("Exit"), (void *)finish, (void *) menuButton_},
            {NULL}};
        auto popOver = GTK_POPOVER(gtk_popover_new (widget));
        
        auto menuPopover = GTK_POPOVER_MENU(gtk_popover_menu_new());
        //g_object_set(G_OBJECT(menuPopover), "name", "main", NULL);

        auto p = item;
        gint i;
        for (i=0;p && p->label && i<1; p++,i++){
            GtkWidget *v = gtk_menu_item_new_with_label (_(p->label));
            gtk_container_add (GTK_CONTAINER (menuPopover), v);
            g_signal_connect ((gpointer) v, "activate", MENUITEM_CALLBACK (p->callback), p->callbackData);
            gtk_widget_show (v);
        }

        gtk_widget_show (GTK_WIDGET(menuPopover));
        gtk_container_add (GTK_CONTAINER (popOver), GTK_WIDGET(menuPopover));
        gtk_widget_show (GTK_WIDGET(popOver));

        return popOver;
    }
#else

    GtkMenu *createMenu(void){
        menuItem_t item[]={
            {N_("Home"), (void *)home, (void *) menuButton_},
            {N_("Open terminal"), (void *)terminal, (void *) menuButton_},
            {N_("Execute Shell Command"), (void *)shell, (void *) menuButton_},
            {N_("Search"), (void *)search, (void *) menuButton_},
            {N_("Exit"), (void *)finish, (void *) menuButton_},
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
#endif
     static void
     home(GtkMenuItem *menuItem, gpointer data)
    {
        DBG("home\n");
    /*    window_c *window_p = (window_c *)data;
        window_p->go_home();*/

    }

    static void
     terminal(GtkMenuItem *menuItem, gpointer data)
    {
        DBG("terminal\n");
        // get current view

   /*     window_c *window_p = (window_c *)data;
        view_c *view_p =(view_c *)window_p->get_active_view_p();
        view_p->get_lpterm_p()->open_terminal();*/
    }

    static void
     shell(GtkMenuItem *menuItem, gpointer data)
    {
        DBG("shell\n");
    /*    window_c *window_p = (window_c *)data;
        window_p->shell_dialog();*/
    }

    static void
    search(GtkMenuItem *menuItem, gpointer data)
    {
        DBG("Search\n");
        // get current directory
        auto notebook_p = (Notebook<Type> *)g_object_get_data(G_OBJECT(data), "notebook_p");
        const gchar *path = notebook_p->workdir();
        auto page_p = (Page<Type> *)notebook_p->currentPageObject();
        if (!path || !g_file_test(path, G_FILE_TEST_IS_DIR)) path = g_get_home_dir();
	gchar *c = g_strdup_printf("xffind \"%s\"", path);
        pid_t child = run_c::thread_run(page_p->output(), c, FALSE);
	page_p->newRunButton(c, child);
        g_free(c);
    }

    static void
    finish(GtkMenuItem *menuItem, gpointer data)
    {
        DBG("finish\n");
        GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(menuItem));
        gtk_widget_hide(window);
        while (gtk_events_pending()) gtk_main_iteration();
        gtk_main_quit();
        _exit(123);
    }


};


}



#endif
