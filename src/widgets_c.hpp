#ifndef WIDGETS_C_HPP
#define WIDGETS_C_HPP
#include <iostream>
#include "intl.h"
#include "window_c.hpp"


using namespace std;

// This class creates all necessary widgets for class view_c 
// and defines their characteristics.

class widgets_c {
    public:
        widgets_c(window_c *);
    protected:

        GtkWidget *page_child_box;
        GtkWidget *page_label_box;
        GtkWidget *page_label_icon_box;
        GtkWidget *page_label;
        GtkWidget *page_label_button;
        GtkWidget *menu_label_box;
        GtkWidget *menu_label;
        GtkWidget *menu_image;
        GtkWidget *pathbar;
        GtkWidget *vpane;
        GtkWidget *top_scrolled_window;
        GtkWidget *bottom_scrolled_window;
        GtkWidget *diagnostics;	        // diagnostics text area
        GtkWidget *status;	        // status text area
        GtkWidget *rename;		// rename entry box
        GtkWidget *button_space;	// little button space
        GtkWidget *clear_button;	// clear text area button
        GtkWidget *size_scale;

    private:
        void create();
        void setup_diagnostics(void);
        void setup_scolled_windows(void);
        void setup_size_scale(void);
        void setup_image_button(GtkWidget *, const gchar *, const gchar *);
        window_c *window_p;

};

#endif
