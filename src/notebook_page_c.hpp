#ifndef NOTEBOOK_PAGE_C_HPP
#define NOTEBOOK_PAGE_C_HPP
#include <gtk/gtk.h>
#include "widgets_c.hpp"

class notebook_page_c:public widgets_c {
    public:
        notebook_page_c(GtkWidget *);
    protected:
        void set_treemodel(GtkTreeModel *)
    private:
        GtkListStore *list_store;

        GtkWidget *notebook;
        GtkWidget *icon_view;           // drawing area
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



};
#endif
