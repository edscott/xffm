#ifndef DIALOGS_HH
#define DIALOGS_HH

namespace xf
{

template <class Type>
class Dialogs{

public:
    
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
        //gtk_window_get_size(dialog, &windowW, &windowH);
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


 /*   static void
    quick_help (GtkWindow *parent, const gchar *message){
        auto dialog = quickHelp_(parent, message, NULL, NULL );
        gtk_widget_show_all (GTK_WIDGET(dialog));
    }

    static GtkWidget *
    quickHelp (GtkWindow *parent, const gchar *message, const gchar *icon){
        auto dialog = quickHelp_(parent, message, icon, _("Help"));
        gtk_widget_show_all (GTK_WIDGET(dialog));
        return dialog;
    }*/

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
     dialog = gtk_dialog_new_with_buttons (title,
					   parent,
					   flags,
					   _("Cancel"),
					   GTK_RESPONSE_NONE,
					   NULL);
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
     
     gtk_container_add (GTK_CONTAINER (content_area), GTK_WIDGET(vbox));
     if (icon){
	auto pixbuf = Pixbuf<Type>::get_pixbuf(icon, -48);
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
        

     return dialog;
    }


private:
    
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
