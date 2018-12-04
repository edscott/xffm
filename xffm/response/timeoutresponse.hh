#ifndef XF_TIMEOUTRESPONSE_HH
#define XF_TIMEOUTRESPONSE_HH
#include <time.h>

       time_t time(time_t *tloc);
       
namespace xf {
template <class Type>
class TimeoutResponse {
    using pixbuf_c = Pixbuf<double>;
    using gtk_c = Gtk<double>;
    using util_c = Util<double>;
public:
    static void
    dialog(GtkWindow *parent, const gchar *message, const gchar *icon){
        if (!icon) icon = "emblem-important";
        if (!message) message = "<span size=\"larger\" color=\"blue\">Custom message markup appears <span color=\"red\">here</span></span>";
         // Create the widgets
         auto dialog = GTK_WINDOW(gtk_window_new (GTK_WINDOW_TOPLEVEL));
         //gtk_window_set_keep_above (dialog,TRUE);
         if (!parent) parent = GTK_WINDOW(mainWindow);
         gtk_window_set_transient_for(dialog, parent);
         //gtk_window_set_position(dialog, GTK_WIN_POS_MOUSE);
         gtk_window_move(dialog, 0,0);

         auto vbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
         gtk_container_add (GTK_CONTAINER (dialog), GTK_WIDGET(vbox));
         auto label = GTK_LABEL(gtk_label_new (""));
	 auto markup = 
	    g_strdup_printf("   <span color=\"blue\" size=\"larger\"><b>%s</b></span>   ", message);           
         gtk_label_set_markup(label, markup);
         g_free(markup);
         
         // Add the label, and show everything we have added
        auto pixbuf = Pixbuf<Type>::get_pixbuf(icon, -48);
        if (pixbuf) {
            auto image = gtk_image_new_from_pixbuf(pixbuf);
            if (image) {
                gtk_box_pack_start(vbox, image, FALSE, FALSE,0);
                gtk_widget_show (image);
            }
        }
         gtk_box_pack_start(vbox, GTK_WIDGET(label), FALSE, FALSE,0);
         g_signal_connect (G_OBJECT (dialog), "delete-event", 
                 EVENT_CALLBACK (delete_event), NULL);

	 gtk_window_set_type_hint(dialog, GDK_WINDOW_TYPE_HINT_DIALOG);
         gtk_window_set_modal(dialog, TRUE);
         gtk_window_set_transient_for (dialog,GTK_WINDOW(mainWindow));
        
	 gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
         gtk_widget_show_all(GTK_WIDGET(dialog));
         time_t now = time(NULL);
         while (time(NULL) < now + 3) {
            while (gtk_events_pending()) gtk_main_iteration();
          //  usleep(100000);
         }


         //void **arg = (void **)calloc(2, sizeof(void *));
         //g_timeout_add_seconds (4, zapit, (void *) dialog);
         zapit((void *)dialog);   

         return;
    }
private:
    static gboolean
    zapit(void *data){
        auto dialog = GTK_WIDGET(data);
        gtk_widget_hide(GTK_WIDGET(dialog));
        gtk_widget_destroy(GTK_WIDGET(dialog));
        return FALSE;
    }

        
    static gboolean delete_event (GtkWidget *widget,
               GdkEvent  *event,
               gpointer   user_data){
	gtk_widget_hide(widget);
        //gtk_widget_destroy(GTK_WIDGET(dialog));
 	return TRUE;
    }
    
};
}

#endif

