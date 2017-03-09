#include "menu_c.hpp"

menu_c::~menu_c(void){
    g_hash_table_destroy(iconname_hash);
}

menu_c::menu_c(data_c *data0): gtk_c(data0){
    iconname_hash = g_hash_table_new(g_str_hash, g_str_equal);
    g_hash_table_insert(iconname_hash, _("Open terminal"), (void *)"terminal");
    g_hash_table_insert(iconname_hash, _("Execute Shell Command"), (void *)"execute");
    g_hash_table_insert(iconname_hash, _("Paste"), (void *)"edit-paste");
    g_hash_table_insert(iconname_hash, _("Add bookmark"), (void *)"list-add");
    g_hash_table_insert(iconname_hash, _("Remove bookmark"), (void *)"list-remove");
    g_hash_table_insert(iconname_hash, _("Search"), (void *)"find");
    g_hash_table_insert(iconname_hash, _("Close"), (void *)"cancel");
    g_hash_table_insert(iconname_hash, _("Exit"), (void *)"window-close");
    g_hash_table_insert(iconname_hash, _("Open with"), (void *)"execute");
    g_hash_table_insert(iconname_hash, _("Cut"), (void *)"cut");
    g_hash_table_insert(iconname_hash, _("Copy"), (void *)"copy");
    g_hash_table_insert(iconname_hash, _("Delete"), (void *)"delete");
    g_hash_table_insert(iconname_hash, _("Shred"), (void *)"dialog-warning");
    g_hash_table_insert(iconname_hash, _("bcrypt"), (void *)"emblem-keyhole");
    g_hash_table_insert(iconname_hash, _("Open in New Tab"), (void *)"open");
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
   const gchar *directory_items[]={
        N_("Open in New Tab"),
        N_("Cut"),
        N_("Copy"),
        N_("Paste"),
        N_("Delete"),
        N_("Shred"),
        N_("bcrypt"),
         NULL};
    const gchar *selection_items[]={
        N_("Open with"),
        N_("Cut"),
        N_("Copy"),
        N_("Delete"),
        N_("Shred"),
        N_("bcrypt"),
         NULL};


    view_menu = mk_menu(view_items);
    selection_menu = mk_menu(selection_items);
    directory_menu = mk_menu(directory_items);
}

GtkMenu *
menu_c::mk_menu(const gchar **data){
    
    GtkMenu *menu = GTK_MENU(gtk_menu_new());
    const gchar **p = data;
    gint i;
    for (i=0;p && *p; p++,i++){
        const gchar *icon_name = (const gchar *)g_hash_table_lookup(iconname_hash, _(*p));
        GtkWidget *v = menu_item_new(icon_name, _(*p));
        gtk_container_add (GTK_CONTAINER (menu), v);
        g_signal_connect ((gpointer) v, "activate", G_CALLBACK (menu_option), (void *)_(*p));
        gtk_widget_show (v);
    }
    gtk_widget_show (GTK_WIDGET(menu));
    return menu;
}


GtkWidget *
menu_c::menu_item_new(const gchar *icon_id, const gchar *text)
{
    GdkPixbuf *pb = (icon_id)? get_pixbuf (icon_id, GTK_ICON_SIZE_SMALL_TOOLBAR): NULL;    
    GtkWidget *w = gtk_menu_item_new_with_label ("");
    GtkWidget *replacement = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
    GtkWidget *label = gtk_bin_get_child(GTK_BIN(w));
    if (label && GTK_IS_WIDGET(label)) {
        g_object_ref(label);
        gtk_container_remove(GTK_CONTAINER(w), label);
    }

    if (pb){
        GtkWidget *image = gtk_image_new_from_pixbuf (pb);
        gtk_widget_show (image);
        gtk_box_pack_start(GTK_BOX(replacement), image, FALSE,FALSE,0);
        g_object_set_data(G_OBJECT(w), "image", image);
	g_object_unref(pb);
    }
    if (label && GTK_IS_WIDGET(label)) {
        gtk_label_set_markup(GTK_LABEL(label), text);
        gtk_box_pack_start(GTK_BOX(replacement), label, FALSE,FALSE,3);
        g_object_set_data(G_OBJECT(w), "label", label);
        gtk_widget_show(label);
        g_object_unref(label);
    }
    gtk_widget_show(replacement);
    gtk_container_add(GTK_CONTAINER(w), replacement);
    return w;
}


