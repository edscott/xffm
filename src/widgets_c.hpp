#ifndef WIDGETS_C_HPP
#define WIDGETS_C_HPP
#include <iostream>
#include "xffm+.h"
#include "gtk_c.hpp"
#include "pathbar_c.hpp"



using namespace std;

// This class creates all necessary widgets for class view_c 
// and defines their characteristics.

class widgets_c: virtual utility_c, public gtk_c{
    public:
        widgets_c(data_c *, void *, GtkNotebook *);
        ~widgets_c(void);

        GtkWidget *get_iconview(void);
        GtkWidget *get_vpane(void);
        GtkWidget *get_status(void);
        GtkWidget *get_diagnostics(void);
        GtkWidget *get_button_space(void);
        GtkWidget *get_status_icon(void);
        GtkWidget *get_iconview_icon(void);
        GtkWidget *get_status_button(void);
        GtkWidget *get_status_label(void);
	
	void *get_window_v(void);
    protected:
        void pack();
	void set_spinner(gboolean);

        GtkWidget *get_page_child(void);
        GtkWidget *get_page_button(void);
        GtkWidget *get_pathbar(void);
        GtkWidget *get_page_label(void);
        GtkWidget *get_page_label_icon_box(void);
        GtkWidget *get_page_label_spinner_box(void);
        GtkWidget *get_clear_button(void);
        GtkToggleButton *get_hidden_button(void);
	GtkNotebook *get_notebook(void);
	
	void update_pathbar(const gchar *);
        void set_status_label(const gchar *);

    private:
        void create();
        void setup_scolled_windows(void);
        void setup_size_scale(void);
        pathbar_c *pathbar_p;
	
        GtkIconView *icon_view;          
        GtkNotebook *notebook;
        GtkWidget *page_spinner;
        GtkWidget *page_child;
        GtkWidget *page_label_box;
        GtkWidget *page_label_spinner_box;
        GtkWidget *page_label_icon_box;
        GtkWidget *page_label;
        GtkWidget *page_label_button;
        GtkWidget *vpane;
        GtkWidget *top_scrolled_window;
        GtkWidget *bottom_scrolled_window;
        GtkWidget *diagnostics;	        // diagnostics text area
        GtkWidget *iconview_icon;
        GtkWidget *status_icon;
        GtkWidget *status;	        // lpterm input area
        GtkWidget *status_label;	// status text area
        GtkWidget *status_button;
        GtkWidget *status_box;
        GtkWidget *rename;		// rename entry box
        GtkWidget *button_space;	// little button space
        GtkWidget *clear_button;	// clear text area button
        GtkWidget *hidden_button;	// show hidden toggle
        GtkWidget *size_scale;

        void *window_v;

};

#endif
