#include "gtk_c.hpp"
#include "intl.h"

void
gtk_c::setup_image_button (GtkWidget *button, const gchar *icon_name, const gchar *icon_tip){
    gtk_widget_set_can_focus (button, FALSE);
    gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
    GtkWidget *image = gtk_image_new_from_icon_name (icon_name, GTK_ICON_SIZE_SMALL_TOOLBAR);
    GdkPixbuf *pixbuf = 
            get_pixbuf(icon_name, GTK_ICON_SIZE_SMALL_TOOLBAR);
    if (image) {
        gtk_container_add (GTK_CONTAINER (button), image);
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
    //g_signal_connect (G_OBJECT (page_label_button), "clicked", G_CALLBACK (callback), data);
    return page_child_box;
}

