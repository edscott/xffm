#ifndef WINDOW_C_HPP
#define WINDOW_C_HPP
#include <gtk/gtk.h>
#include "tooltip_c.hpp"

class window_c: public tooltip_c {
    public:
        window_c(void);
        GtkWidget *get_notebook(void);
    private:
        GtkWidget *notebook;
        GtkWidget *window;

};

#endif