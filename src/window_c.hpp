#ifndef WINDOW_C_HPP
#define WINDOW_C_HPP
#include <gtk/gtk.h>
#include "gtk_c.hpp"
#include "signals_c.hpp"
#include "utility_c.hpp"

class window_c {
    public:
        window_c(void);
        ~window_c(void);
        GtkWidget *get_notebook(void);
        GtkWidget *get_new_tab_child(void);
        GtkWidget *get_new_tab_button(void);
        
        void add_view_to_list(void *);
        void remove_view_from_list(void *);
    private:
        void set_up_view_signals(void *);
        GtkWidget *notebook;
        GtkWidget *window;
        GtkWidget *new_tab_child;
        GtkWidget *new_tab_button;

        utility_c *utility_p;
        gtk_c *gtk_p;
        signals_c *signals_p;
                
        GList *view_list;
        pthread_mutex_t view_list_mutex;

};

#endif
