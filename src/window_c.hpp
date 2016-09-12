#ifndef WINDOW_C_HPP
#define WINDOW_C_HPP
#include "xffm+.h"

#include "gtk_c.hpp"
#include "utility_c.hpp"

class window_c {
    public:
        window_c(gtk_c *);
        ~window_c(void);
        GtkNotebook *get_notebook(void);
        gtk_c *get_gtk_p(void);
	void create_new_page(const gchar *);
	void go_home(void);
        void *get_active_view_p(void);
        GtkWindow *get_window(void);	
        gboolean is_view_in_list(void *);
        void add_view_to_list(void *);
        void remove_view_from_list(void *);
    private:
        void set_up_view_signals(void *);
        GtkNotebook *notebook;
        GtkWidget *window;
        GtkWidget *new_tab_button;

        utility_c *utility_p;
        gtk_c *gtk_p;
                
        GList *view_list;
        pthread_mutex_t view_list_mutex;



};

#endif
