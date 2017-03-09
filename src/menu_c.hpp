#ifndef MENU_C_HPP
#define MENU_C_HPP
#include "xffm+.h"
#include "gtk_c.hpp"

class menu_c: public gtk_c  {
    public:
        menu_c(data_c *);
        ~menu_c(void);
        virtual GtkMenu *get_view_menu(void){return view_menu;}
        virtual GtkMenu *get_selected_menu(void){return selection_menu;}
        virtual GtkMenu *get_directory_menu(void){return directory_menu;}
    protected:
    private:
        GtkMenu *view_menu;
        GtkMenu *selection_menu;
        GtkMenu *directory_menu;
        void create_menu(void);
        GtkMenu *mk_menu(const gchar **);
        GtkWidget *menu_item_new(const gchar *, const gchar *);
        GHashTable *iconname_hash;

};

#endif
