#ifndef SIGNAL_ACTION_C_HPP
#define SIGNAL_ACTION_C_HPP
#include "xffm+.h"

class signal_action_c {
    public:
        signal_action_c(GtkApplication *);
        GtkApplication *get_app(void);
        GMenuModel *get_signal_menu_model(void);
    private:
        GtkApplication *app;
        void add_actions(GtkApplication *);
        void create_menu_model(GtkApplication *);
        GMenuModel *signal_menu_model; 

};

#endif
