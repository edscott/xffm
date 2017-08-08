#ifndef WINDOW_C_HPP
#define WINDOW_C_HPP
#include "xffm+.h"

#include "menu_c.hpp"
#include "utility_c.hpp"

class window_c: public menu_c {
    public:
        window_c(GtkApplication *);
        ~window_c(void);
        GtkNotebook *get_notebook(void);
	void create_new_page(const gchar *);
	void go_home(void);
        void *get_active_view_p(void);
        GtkWindow *get_window(void);	
        gboolean is_view_in_list(void *);
        void add_view_to_list(void *);
        void remove_view_from_list(void *);

        void set_tt_window(GtkWidget *, const gchar *);
        const gchar *get_tooltip_path_string(void);
        void set_tooltip_path_string(const gchar *);
        GtkWidget *get_tt_window(void);
	GtkApplication *app; 
        
	void shell_dialog(void);
        static GtkWidget *window;
    private:
        GtkNotebook *notebook;
        GtkWidget *new_tab_button;
	
                
        GList *view_list;
        pthread_mutex_t view_list_mutex;


        gchar *tooltip_path_string;
        GtkWidget *tt_window;

        GMenuModel *signal_menu_model;
        void create_menu_model(void);
        void add_actions(void);

};

#endif
