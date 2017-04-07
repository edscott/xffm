#ifndef MENU_C_HPP
#define MENU_C_HPP
#include "xffm+.h"
#include "gtk_c.hpp"

class menu_c: public gtk_c  {
    public:
        menu_c(void);
        ~menu_c(void);
	virtual gboolean popup(GtkTreePath *);
        virtual gboolean popup(void);
        gboolean view_popup(void);
        GtkMenu *get_view_menu(void){return view_menu;}
 	
    protected:
    private:
        GtkMenu *view_menu;
        void create_menu(void);

};

#endif
