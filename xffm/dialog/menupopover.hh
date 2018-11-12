#ifndef MENU_POPOVER_HH
#define MENU_POPOVER_HH
#include "signals/menupopover.hh"

namespace xf {
template <class Type> class Notebook;
template <class Type> class Page;

template <class Type>
class MenuPopover {
public:
    MenuPopover(void) {

        menuButton_ = GTK_MENU_BUTTON(gtk_menu_button_new());
#ifdef TRY_POPOVER
        auto popover = createPopover(GTK_WIDGET(menuButton_));
        gtk_menu_button_set_popover(menuButton_, GTK_WIDGET(popover));
#else
        auto menu = createMenu();
        gtk_menu_button_set_popup (menuButton_, GTK_WIDGET(menu));
        gtk_widget_set_can_focus (GTK_WIDGET(menuButton_), FALSE);
	gtk_button_set_relief (GTK_BUTTON(menuButton_), GTK_RELIEF_NONE);
#endif
 
        gtk_widget_show(GTK_WIDGET(menuButton_));
    }
    GtkMenuButton *menuButton(){ return menuButton_;}
    protected:
    
    GtkMenuButton *menuButton_;
    private:

    GtkMenu *createMenu(void){
        menuItem_t item[]={
            {N_("Home"), (void *)MenuPopoverSignals<Type>::home, (void *) menuButton_},
            {N_("Open terminal"), (void *)MenuPopoverSignals<Type>::terminal, (void *) menuButton_},
            //{N_("Execute Shell Command"), (void *)MenuPopoverSignals<Type>::shell, (void *) menuButton_},
            {N_("Search"), (void *)MenuPopoverSignals<Type>::search, (void *) menuButton_},
            {N_("Exit"), (void *)MenuPopoverSignals<Type>::finish, (void *) menuButton_},
            {NULL}};
        
        auto menu = GTK_MENU(gtk_menu_new());
        auto p = item;
        gint i;
        for (i=0;p && p->label; p++,i++){
            GtkWidget *v = gtk_menu_item_new_with_label (_(p->label));
            gtk_container_add (GTK_CONTAINER (menu), v);
            g_signal_connect ((gpointer) v, "activate", MENUITEM_CALLBACK (p->callback), p->callbackData);
            gtk_widget_show (v);
        }
        gtk_widget_show (GTK_WIDGET(menu));
        return menu;
    }
   /* static void
    shell(GtkMenuItem *menuItem, gpointer data)
    {
    }*/



};


}



#endif
