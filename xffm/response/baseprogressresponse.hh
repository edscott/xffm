#ifndef XF_BASEPROGRESSRESPONSE_HH
#define XF_BASEPROGRESSRESPONSE_HH
namespace xf {
template <class Type>
class BaseProgressResponse {
    using pixbuf_c = Pixbuf<double>;
    using gtk_c = Gtk<double>;
    using util_c = Util<double>;
public:
    static GtkWindow *
    dialog(const gchar *message, const gchar *icon){
         // Create the widgets
         auto dialog = GTK_WINDOW(gtk_window_new (GTK_WINDOW_TOPLEVEL));
         //gtk_window_set_transient_for (dialog,GTK_WINDOW(mainWindow));
         gtk_window_set_type_hint (dialog,GDK_WINDOW_TYPE_HINT_DIALOG);
         auto vbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
         gtk_container_add (GTK_CONTAINER (dialog), GTK_WIDGET(vbox));
         auto label = GTK_LABEL(gtk_label_new (""));
	 g_object_set_data(G_OBJECT(dialog), "label", label);
	 auto markup = 
	    g_strdup_printf("   <span color=\"blue\" size=\"larger\"><b>%s</b></span>   ", message);           
         gtk_label_set_markup(label, markup);
         g_free(markup);
         
         // Add the label, and show everything we have added
         if (icon){
            auto pixbuf = Pixbuf<Type>::get_pixbuf(icon, -96);
            if (pixbuf) {
                auto image = gtk_image_new_from_pixbuf(pixbuf);
                if (image) {
                    gtk_box_pack_start(vbox, image, FALSE, FALSE,0);
                    gtk_widget_show (image);
                }
            }
         }
         gtk_box_pack_start(vbox, GTK_WIDGET(label), FALSE, FALSE,0);
         auto progress = GTK_PROGRESS_BAR(gtk_progress_bar_new());
	 g_object_set_data(G_OBJECT(dialog), "progress", progress);
	 g_object_set_data(G_OBJECT(progress), "label", label);

         gtk_box_pack_start(vbox, GTK_WIDGET(progress), FALSE, FALSE,0);


	 gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
         return dialog;
    }
};
}

#endif

