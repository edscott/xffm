#ifndef WINDOW_C_HPP
#define WINDOW_C_HPP
#include <gtk/gtk.h>
#include "gtk_c.hpp"

class window_c {
    public:
        window_c(void);
        ~window_c(void);
        GtkWidget *get_notebook(void);
        GtkWidget *get_add_child(void);
        GtkWidget *get_new_button(void);
    private:
        GtkWidget *notebook;
        GtkWidget *window;
        utility_c *utility_p;
        gtk_c *gtk_p;
        GtkWidget *add_child;
        GtkWidget *new_button;

};

#endif
