#include <gtk/gtk.h>
#include "signals_c.hpp"


static void
callback_w(GtkWidget *widget, gpointer data){
    void (*callback)(void *, void *) = (void (*)(void*, void*))
        g_object_get_data(G_OBJECT(widget), "callback");
    (*callback)(widget, data);
    return;
} 

void 
signals_c::setup_callback(void *object, GtkWidget *widget, 
	    const gchar *signal, void *callback, void *data)
{
    g_object_set_data(G_OBJECT(widget), "callback", callback);
    g_object_set_data(G_OBJECT(widget), "object", object);
    g_signal_connect ((gpointer) widget, signal, 
	    G_CALLBACK (callback_w), data);
}


static void
event_callback_w(GtkWidget *widget, GdkEvent  *event, gpointer data){
    void (*callback)(void *, void *, void *) = (void (*)(void*, void*, void*))
        g_object_get_data(G_OBJECT(widget), "callback");
    (*callback)(widget, event, data);
    return;
} 

void 
signals_c::setup_event_callback(void *object, GtkWidget *widget, 
	    const gchar *signal, void *callback, void *data)
{
    g_object_set_data(G_OBJECT(widget), "callback", callback);
    g_object_set_data(G_OBJECT(widget), "object", object);
    g_signal_connect ((gpointer) widget, signal, 
	    G_CALLBACK (event_callback_w), data);
}

