#ifndef DIALOGS_HH
#define DIALOGS_HH

namespace xf
{
template <class Type> class Gtk;

template <class Type>
class Dialogs{
private:
    static void
    responseYes (GtkWidget * button, gpointer data) {
	g_object_set_data(G_OBJECT(data), "response", GINT_TO_POINTER(1));
	gtk_widget_hide(GTK_WIDGET(data));
	gtk_dialog_response(GTK_DIALOG(data), GTK_RESPONSE_YES);
    }

public:

    static GtkWidget *yesNo(const gchar *message){
	auto dialog = Dialogs<Type>::quickHelp(mainWindow, message, "dialog-question");
	auto buttonBox = (GtkBox *)g_object_get_data(G_OBJECT(dialog), "buttonBox");
	auto button = Gtk<Type>::dialog_button("greenball", _("Yes"));
	g_signal_connect (button, "clicked", G_CALLBACK (responseYes), dialog);
	gtk_box_pack_start(buttonBox, GTK_WIDGET(button), FALSE, FALSE,0);
	
	
	gtk_dialog_run(GTK_DIALOG(dialog));
	return dialog;
    }
    
    static void
    placeDialog(GtkWindow *dialog){
        gtk_widget_realize(GTK_WIDGET(dialog));
        Display *display = gdk_x11_display_get_xdisplay(gdk_display_get_default());
        Drawable w = gdk_x11_window_get_xid(gtk_widget_get_window(GTK_WIDGET(dialog)));
        Window root;
        Window child;
        gint mouseX, mouseY, childX, childY;
        guint rootW, rootH;
        guint mask;
        XQueryPointer(display, w, &root, &child, &mouseX, &mouseY, &childX, &childY, &mask);
        guint windowW,windowH;
        getWindowDimensions(w, &windowW, &windowH);
        getRootDimensions(&rootW, &rootH);
        TRACE("*** rootW,H= (%d,%d) window=(%d,%d) mouse=(%d,%d)\n", 
                rootW,rootH,windowW,windowH, mouseX,mouseY);
        if (mouseX+windowW > rootW) mouseX = rootW - windowW;
        if (mouseY+windowH > rootH) mouseY = rootH - windowH;
        TRACE("***2 rootW,H= (%d,%d) window=(%d,%d) mouse=(%d,%d)\n", 
                rootW,rootH,windowW,windowH, mouseX,mouseY);

        gtk_window_move(dialog, mouseX, mouseY);
    }


    static GtkWidget *
    quickHelp (GtkWindow *parent, 
            const gchar *message, 
            const gchar *icon="dialog-information", 
            const gchar *title=_("Help"))
    {
        auto dialog = quickDialog(parent, message, icon, title);
        gtk_widget_show_all (GTK_WIDGET(dialog));
        return dialog;
    }


    static GtkWidget *
    quickDialog (GtkWindow *parent, 
            const gchar *message, 
            const gchar *icon, 
            const gchar *title)
    {
     GtkWidget *dialog = NULL;
     GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

     // Create the widgets
     dialog = gtk_dialog_new ();
     if (parent) {
        gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
        gtk_window_set_transient_for (GTK_WINDOW (dialog), 
                GTK_WINDOW (parent));
     } else {
        gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
     }
     gtk_window_set_type_hint(GTK_WINDOW(dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
     
     auto content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
     auto label = GTK_LABEL(gtk_label_new (""));
     gtk_label_set_markup(label, message);
     

     // Ensure that the dialog box is destroyed when the user responds

     if (parent){
         g_signal_connect_swapped(dialog, "response", 
                    G_CALLBACK (gtk_widget_show),
		    parent);
     }
     g_signal_connect_swapped (dialog, "response",
		G_CALLBACK (closeQuickDialog),
		dialog);


     // Add the label, and show everything we have added
     auto vbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
     g_object_set_data(G_OBJECT(dialog), "vbox", vbox);
     
     gtk_box_pack_start(GTK_BOX(content_area), GTK_WIDGET(vbox), TRUE, TRUE,0);
     
     if (icon){
	auto pixbuf = Pixbuf<Type>::getPixbuf(icon, -48);
        if (pixbuf) {
            auto image = gtk_image_new_from_pixbuf(pixbuf);
	    if (image) {
	        gtk_box_pack_start(vbox, image, FALSE, FALSE,0);
	        gtk_widget_show (image);
            }
	}
     }
     auto hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
     gtk_box_pack_start(vbox, GTK_WIDGET(hbox), FALSE, FALSE,0);
     gtk_box_pack_start(hbox, GTK_WIDGET(label), FALSE, FALSE,0);

     auto vbox2 = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
   
     gtk_box_pack_end(vbox, GTK_WIDGET(vbox2), FALSE, FALSE,0);
     auto hbox2 = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
     gtk_box_pack_end(vbox2, GTK_WIDGET(hbox2), FALSE, FALSE,0);
     auto button = Gtk<Type>::dialog_button("redball", _("Cancel"));
     gtk_box_pack_end(hbox2, GTK_WIDGET(button), FALSE, FALSE,0);
     g_object_set_data(G_OBJECT(dialog), "buttonBox", (void *)hbox2); 
     g_signal_connect (button, "clicked",
		G_CALLBACK (onQuickCancel),
		dialog);

        

     return dialog;
    }


private:

    static void
    onQuickCancel (GtkWidget * button, gpointer data) {
        auto dialog = GTK_DIALOG(data);
        gtk_dialog_response(dialog, GTK_RESPONSE_CANCEL );
    }
    
    static void
    closeQuickDialog(GtkWidget *widget, GdkEventKey * event, void *data){
	TRACE("closeQuickDialog\n");
	gtk_widget_hide(widget);
	gtk_widget_destroy(widget);
    }


    static void 
    getWindowDimensions(Drawable drawable, guint *windowW, guint *windowH){
        
        gint x, y;
        guint d, border;
        Window root;
        Display *display = gdk_x11_display_get_xdisplay(gdk_display_get_default());

        XGetGeometry(display, drawable, &root,
                &x, &y, 
                windowW, windowH, 
                &border, &d);
        return;
    }

    static void 
    getRootDimensions(guint *rootW, guint *rootH){
        
        Drawable drawable = gdk_x11_get_default_root_xwindow ();
        gint x, y;
        guint d, border;
        Window root;
        Display *display = gdk_x11_display_get_xdisplay(gdk_display_get_default());

        XGetGeometry(display, drawable, &root,
                &x, &y, 
                rootW, rootH, 
                &border, &d);
        return;
    }
    
};
}
#endif
