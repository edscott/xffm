#ifndef WINDOW_C_HPP
#define WINDOW_C_HPP
#include "xffm+.h"

#include "data_c.hpp"
#include "gtk_c.hpp"
#include "utility_c.hpp"

class window_c: public gtk_c {
    public:
        window_c(data_c *);
        ~window_c(void);
	data_c *get_data_p(void);
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
	
    private:
        GtkNotebook *notebook;
        GtkWidget *window;
        GtkWidget *new_tab_button;
	
                
        GList *view_list;
        pthread_mutex_t view_list_mutex;


        gchar *tooltip_path_string;
        GtkWidget *tt_window;

	data_c *data_p;


};

#endif
