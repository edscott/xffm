
#include "widgets_c.hpp"
static void
widget_callback(GtkWidget *widget, gpointer data){
    void (*callback)(void *, void *) = g_object_get_data(G_OBJECT(widget), "callback");
    void *object = g_object_get_data(G_OBJECT(widget), "object");
    (*callback)(object, data);
    return;
} 

void 
widgets_c::setup_callback(void *object, GtkWidget *widget, 
	    const gchar sig, void *callback, void *data)
{
    g_object_set_data(G_OBJECT(widget), "callback", callback);
    g_object_set_data(G_OBJECT(widget), "object", object);
    g_signal_connect ((gpointer) widget, sig, 
	    G_CALLBACK (widget_callback), data);
}

void 
widgets_c::add_custom_tooltip(GtkWidget *widget, GtkWidget *image const gchar *tooltip_text){
    //FIXME
}

GtkWidget *
widgets_c::mk_little_button (const gchar * icon_id, void *callback, void *callback_data, const gchar * tooltip_text){
    GtkWidget *button = gtk_button_new ();
    gtk_widget_set_can_focus (button, FALSE);
    gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
    GtkWidget *image = (icon_name==NULL)? NULL:
        gtk_image_new_from_icon_name (icon_name, GTK_ICON_SIZE_SMALL_TOOLBAR);
    if (image) {
        gtk_container_add (GTK_CONTAINER (button), image);
        gtk_widget_show (image);
    }
    if(tooltip_text && strlen (tooltip_text)) {
        add_custom_tooltip(button, image, tooltip_text);
    }
    if(callback) {
	//setup_callback((void *) this, button, "button-press-event", callback, callback_data);
	setup_callback((void *) this, button, "clicked", callback, callback_data);
    return button;
}  

