#ifndef VIEW_C_HPP
#define VIEW_C_HPP
#include <gtk/gtk.h>
#include <pthread.h>

class notebook_page_c {
    public:
        notebook_page_c(GtkWidget *);
    protected:

    private:
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

class view_c:public notebook_page_c {
    public:
        view_c::view_c(void);
        view_c::~view_c(void);

    protected:

    private:
        void init(void);

        GtkListStore *list_store;
        gchar *workdir;		
        pthread_mutex_t *population_mutex;
        pthread_cond_t *population_cond;
        gint population_condition;
        pthread_rwlock_t *population_lock;

};

#endif
