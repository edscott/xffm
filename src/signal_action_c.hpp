#ifndef SIGNAL_ACTION_C_HPP
#define SIGNAL_ACTION_C_HPP
#include "xffm+.h"
#include "data_c.hpp"
#include "gtk_c.hpp"

class signal_action_c {
    public:
        signal_action_c(data_c *);
        GMenuModel *get_signal_menu_model(void);
       // void set_signal_action_parameter(void *);
        GtkWidget *get_menu(void);
    protected:
    private:
        void add_actions(GtkApplication *);
        void create_menu_model(GtkApplication *);
        GMenuModel *signal_menu_model; 
	data_c *data_p;

        GtkWidget *menu;
};

#endif
