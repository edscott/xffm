#include "menu_c.hpp"

menu_c::~menu_c(void){
    gtk_widget_destroy(GTK_WIDGET(view_menu));
}

menu_c::menu_c(void){
    create_menu();
    
}

static void menu_option(GtkWidget *menu_item, gpointer data){
    const gchar *what = (const gchar *)data;
    fprintf(stderr, "menu option: %s\n", what);
    if (strcmp(what, _("Open terminal"))==0){}
    else if (strcmp(what, _("Execute Shell Command"))==0){}
    else if (strcmp(what, _("Paste"))==0){}
    else if (strcmp(what, _("Add bookmark"))==0){}
    else if (strcmp(what, _("Remove bookmark"))==0){}
    else if (strcmp(what, _("Search"))==0){}
    else if (strcmp(what, _("Close"))==0){}
    else if (strcmp(what, _("Exit"))==0){ _exit(123);}
    //} else if (strcmp(what, )==0){}
    
}

void
menu_c::create_menu(void){
    const gchar *view_items[]={
        N_("Open terminal"),
        N_("Execute Shell Command"),
        N_("Paste"),
        N_("Add bookmark"),
        N_("Remove bookmark"),
        N_("Search"),
        N_("Close"),
        N_("Exit"),
        NULL};
    view_menu = mk_menu(view_items, menu_option);
//    view_menu = mk_menu(view_items, menu_option);
}
gboolean
menu_c::view_popup(void){
    fprintf(stderr, "xfdir_c::popup: general popup\n");
    gtk_menu_popup(view_menu, NULL, NULL, NULL, NULL, 3, gtk_get_current_event_time());
    return TRUE;
}

gboolean
menu_c::popup(void){view_popup(); return TRUE;}

gboolean
menu_c::popup(GtkTreePath *tpath){view_popup(); return TRUE;}

