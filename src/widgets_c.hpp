#ifndef WIDGETS_C_HPP
#define WIDGETS_C_HPP
#include <iostream>
#include "intl.h"
#include "utility_c.hpp"
#include "utility_c.hpp"


using namespace std;

// This class creates all necessary widgets for class view_c 
// and defines their characteristics.

class widgets_c: public utility_c {
    public:
        widgets_c(void);
    protected:
	GtkWidget * 
	    mk_little_button (const gchar *, 
		    void *, 
		    void *, 
		    const gchar *);
	void 
	    add_custom_tooltip(GtkWidget *widget, 
		    GtkWidget *image,
		    const gchar *tooltip_text);

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
        void setup_clear_button(void);

};

#endif
