#include "gtk_c.hpp"
#include "intl.h"
GHashTable *gtk_c::iconname_hash=NULL;

gint 
gtk_c::get_icon_size(const gchar *name){
    if (strcmp(name, "..")==0) return GTK_ICON_SIZE_DND;
    return GTK_ICON_SIZE_DIALOG;
}

gtk_c::gtk_c(void){
    if (!iconname_hash){
        iconname_hash = g_hash_table_new(g_str_hash, g_str_equal);
        populate_iconname_hash();
    }
    //fprintf(stderr, "gtk_c constructor OK\n");
}


gtk_c::~gtk_c(void){
    DBG("gtk_c::~gtk_c\n");
}

void
gtk_c::populate_iconname_hash(void){
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
}

void
gtk_c::setup_image_button (GtkWidget *button, const gchar *icon_name, const gchar *icon_tip){
    gtk_widget_set_can_focus (button, FALSE);
    gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
    GtkWidget *image = gtk_image_new_from_icon_name (icon_name, GTK_ICON_SIZE_SMALL_TOOLBAR);
    GdkPixbuf *pixbuf = 
            pixbuf_c::get_pixbuf(icon_name, GTK_ICON_SIZE_SMALL_TOOLBAR);
    if (image) {
        gtk_container_add (GTK_CONTAINER (button), image);
        // On page remove, reference count will be decreased
        // Last reference belongs to the pixbuf hash
        g_object_ref(G_OBJECT(pixbuf));
        gtk_widget_show (image);
    }
    // Elaborate tooltip
    custom_tooltip(button, pixbuf, icon_tip);
    // Simple tooltip:
    // gtk_widget_set_tooltip_text (button, icon_tip);
    
}  

GtkWidget *
gtk_c::new_add_page_tab(GtkWidget *notebook, GtkWidget **new_button_p){
    GtkWidget *page_child_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *page_label_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *page_label_icon_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *page_label_button = gtk_button_new ();
    gtk_box_pack_start (GTK_BOX (page_label_box), page_label_icon_box, TRUE, TRUE, 0);
    gtk_box_pack_end (GTK_BOX (page_label_box), page_label_button, TRUE, TRUE, 0);
    gtk_widget_show_all (page_label_box);
    gtk_widget_show (page_child_box);
    gtk_notebook_append_page (GTK_NOTEBOOK(notebook), page_child_box, page_label_box);
    gtk_notebook_set_tab_reorderable (GTK_NOTEBOOK(notebook), page_child_box, FALSE);
    setup_image_button(page_label_button, "list-add", _("Open a new tab (Ctrl+T)"));
    if (new_button_p) *new_button_p = page_label_button;
    return page_child_box;
}

void
gtk_c::set_bin_image(GtkWidget *bin, const gchar *icon_id, gint size){
    if (!bin || !GTK_IS_WIDGET(bin)) {
        g_warning("rfm_set_bin_image(): incorrect function call\n");
        return;
    }
    GtkWidget *box = gtk_bin_get_child(GTK_BIN(bin));
    GtkWidget *icon = (GtkWidget *)g_object_get_data(G_OBJECT(bin),"icon");

    if (icon){
        gtk_container_remove(GTK_CONTAINER(box), icon);
    }
    if(icon_id) {
        GdkPixbuf *pb = pixbuf_c::get_pixbuf (icon_id, size);
        GtkWidget *image = gtk_image_new_from_pixbuf (pb);
	gtk_box_pack_start(GTK_BOX(box), image, FALSE, FALSE,0);
        g_object_set_data(G_OBJECT(bin), "icon", image);
	gtk_widget_show(image);
	g_object_unref(pb);
    } else {
        g_object_set_data(G_OBJECT(bin), "icon", NULL);
    }
}

void 
gtk_c::set_bin_markup(GtkWidget *bin, const char *text){
    if (!bin || !GTK_IS_WIDGET(bin)) {
        g_warning("rfm_set_bin_markup(): incorrect function call\n");
        return;
    }
    GtkLabel *label = (GtkLabel *)g_object_get_data(G_OBJECT(bin), "label");
    gtk_label_set_markup(label, (text)?text:"");
}

void
gtk_c::set_bin_contents(GtkWidget *bin, const char *icon_id, const char *text, gint size){
    GtkWidget *child = gtk_bin_get_child(GTK_BIN(bin));
    if (child) gtk_container_remove(GTK_CONTAINER(bin), child);
    child = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_size_request (child, -1, 12);
    gtk_container_add(GTK_CONTAINER(bin), child);
                            
    GtkWidget *label = gtk_label_new("");
    g_object_set_data(G_OBJECT(bin), "label", label);
    gtk_box_pack_end(GTK_BOX(child), label, TRUE, FALSE,0);
    
    set_bin_markup(bin, text);
    set_bin_image(bin, icon_id, size);
    gtk_widget_show_all (bin);
}


GtkWidget * 
gtk_c::menu_item_new(const gchar *icon_id, const gchar *text)
{
    GdkPixbuf *pb = (icon_id)? pixbuf_c::get_pixbuf (icon_id, GTK_ICON_SIZE_SMALL_TOOLBAR): NULL;    
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

GtkWidget *
gtk_c::hbox_new(gboolean homogeneous, gint spacing){
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, spacing);
    gtk_box_set_homogeneous (GTK_BOX(box),homogeneous);
    return box;
}

GtkWidget *
gtk_c::vbox_new(gboolean homogeneous, gint spacing){
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, spacing);
    gtk_box_set_homogeneous (GTK_BOX(box),homogeneous);
    return box;
}


GtkWidget *
gtk_c::dialog_button (const char *icon_id, const char *text) {
    GtkWidget *button = gtk_button_new ();
    set_bin_contents(button, icon_id, text, SIZE_BUTTON);
    return button;

}


GtkWidget *
gtk_c::toggle_button (const char *icon_id, const char *text) {
    GtkWidget *button = gtk_toggle_button_new ();
    set_bin_contents(button, icon_id, text, SIZE_BUTTON);
    return button;
}

GtkMenu *
gtk_c::mk_menu(const gchar **data, void (*menu_callback)(GtkWidget *, gpointer)){
    
    GtkMenu *menu = GTK_MENU(gtk_menu_new());
    const gchar **p = data;
    gint i;
    for (i=0;p && *p; p++,i++){
        const gchar *icon_name = (const gchar *)g_hash_table_lookup(iconname_hash, _(*p));
        GtkWidget *v = menu_item_new(icon_name, _(*p));
        g_object_set_data(G_OBJECT(v), "menu", (void *)menu);
        gtk_container_add (GTK_CONTAINER (menu), v);
        g_signal_connect ((gpointer) v, "activate", G_CALLBACK (*menu_callback), (void *)_(*p));
        gtk_widget_show (v);
    }
    gtk_widget_show (GTK_WIDGET(menu));
    return menu;
}


