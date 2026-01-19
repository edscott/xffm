#ifndef DIALOGS_HH
#define DIALOGS_HH

namespace xf
{
template <class Type> class Gtk;

template <class Type>
class Dialogs{
private:
    static void
    responseCancel (GtkWidget * button, gpointer data) {
        g_object_set_data(G_OBJECT(data), "response", GINT_TO_POINTER(0));
        gtk_widget_hide(GTK_WIDGET(data));
        MainDialog = NULL;
        gtk_dialog_response(GTK_DIALOG(data), 0);
    }
    static void
    responseYes (GtkWidget * button, gpointer data) {
        g_object_set_data(G_OBJECT(data), "response", GINT_TO_POINTER(1));
        gtk_widget_hide(GTK_WIDGET(data));
        MainDialog = NULL;
        gtk_dialog_response(GTK_DIALOG(data), 1);
    }

public:

    static GtkWidget *yesNo(const gchar *message){
        auto dialog = Dialogs<Type>::quickHelp(mainWindow, message, "dialog-question");
        auto buttonBox = (GtkBox *)g_object_get_data(G_OBJECT(dialog), "buttonBox");
        auto button = Gtk<Type>::dialog_button("greenball", _("Yes"));
        g_signal_connect (button, "clicked", G_CALLBACK (responseYes), dialog);
        compat<bool>::boxPack0(buttonBox, GTK_WIDGET(button), FALSE, FALSE,0);
        
        
        gtk_dialog_run(GTK_DIALOG(dialog));
        MainDialog = GTK_WINDOW(dialog);
        return dialog;
    }

    static GtkWidget *
    overwriteCancel (const gchar *message)
    {
      GtkWindow *parent = mainWindow;
      const gchar *icon = "dialog-question";
      const gchar *title=_("Confirm");
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
     

      // Ensure that the dialog box is destroyed when the user responds
      g_signal_connect_swapped (dialog, "response",
                G_CALLBACK (closeQuickDialog),
                dialog);


      // Add the label, and show everything we have added
      auto vbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
      g_object_set_data(G_OBJECT(dialog), "vbox", vbox);
      auto hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));

      compat<bool>::boxPack0(GTK_BOX(content_area), GTK_WIDGET(vbox), FALSE, FALSE,0);
      compat<bool>::boxPack0(GTK_BOX(vbox), GTK_WIDGET(hbox), TRUE, TRUE,0);

      if (icon){
        auto pixbuf = Pixbuf<Type>::getPixbuf(icon, -48);
        if (pixbuf) {
            auto image = gtk_image_new_from_pixbuf(pixbuf);
            if (image) {
                compat<bool>::boxPack0(hbox, image, FALSE, FALSE,0);
                gtk_widget_show (image);
            }
        }
        hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        compat<bool>::boxPack0(GTK_BOX(vbox), GTK_WIDGET(hbox), TRUE, TRUE,0);
      }
      auto label = GTK_LABEL(gtk_label_new (""));
      gtk_label_set_markup(label, message);
      compat<bool>::boxPack0(hbox, GTK_WIDGET(label), TRUE, TRUE,0);

      hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
      compat<bool>::boxPack0(vbox, GTK_WIDGET(hbox), FALSE, FALSE,0);
 
      auto check = gtk_check_button_new_with_label(_("Apply to all"));
      compat<bool>::boxPack0(hbox, GTK_WIDGET(check), FALSE, FALSE,0);
      g_object_set_data(G_OBJECT(dialog), "all", check);

      auto button = Gtk<Type>::dialog_button("greenball", _("Overwrite"));
      compat<bool>::boxPack0(hbox, GTK_WIDGET(button), FALSE, FALSE,0);
      g_signal_connect (button, "clicked", G_CALLBACK (responseYes), dialog);

      button = Gtk<Type>::dialog_button("redball", _("Cancel"));
      compat<bool>::boxPack0(hbox, GTK_WIDGET(button), FALSE, FALSE,0);
      g_signal_connect (button, "clicked", G_CALLBACK (responseCancel), dialog);

      gtk_widget_show_all(GTK_WIDGET(dialog));
      gtk_dialog_run(GTK_DIALOG(dialog));
        
      MainDialog = GTK_WINDOW(dialog);

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
    quickCancel (GtkWindow *parent, 
            const gchar *message, 
            const gchar *icon="dialog-question", 
            const gchar *title=_("Confirm"))
    {
        auto dialog = quickDialogCancel(parent, message, icon, title);
        gtk_widget_show_all (GTK_WIDGET(dialog));
        MainDialog = GTK_WINDOW(dialog);
        return dialog;
    }

    static GtkWidget *
    quickDialogCancel (GtkWindow *parent, 
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
/*
     if (parent){
         g_signal_connect_swapped(dialog, "response", 
                    G_CALLBACK (gtk_widget_show),
                    parent);
     }
     */
     g_signal_connect_swapped (dialog, "response",
                G_CALLBACK (closeQuickDialog),
                dialog);


     // Add the label, and show everything we have added
     auto vbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
     g_object_set_data(G_OBJECT(dialog), "vbox", vbox);
     
     compat<bool>::boxPack0(GTK_BOX(content_area), GTK_WIDGET(vbox), TRUE, TRUE,0);
     
     if (icon){
        auto pixbuf = Pixbuf<Type>::getPixbuf(icon, -48);
        if (pixbuf) {
            auto image = gtk_image_new_from_pixbuf(pixbuf);
            if (image) {
                compat<bool>::boxPack0(vbox, image, FALSE, FALSE,0);
                gtk_widget_show (image);
            }
        }
     }
     auto hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
     compat<bool>::boxPack0(vbox, GTK_WIDGET(hbox), FALSE, FALSE,0);
     compat<bool>::boxPack0(hbox, GTK_WIDGET(label), FALSE, FALSE,0);

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

        
        
     MainDialog = GTK_WINDOW(dialog);

     return dialog;
    }

    static GtkWidget *
    quickHelp (GtkWindow *parent, 
            const gchar *message, 
            const gchar *icon="dialog-information", 
            const gchar *title=_("Help"))
    {
        auto dialog = quickDialog(parent, message, icon, title);
        gtk_widget_show_all (GTK_WIDGET(dialog));
        MainDialog = GTK_WINDOW(dialog);
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
     g_object_set_data(G_OBJECT(dialog),"parent", parent);
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
/*
     if (parent){
         g_signal_connect_swapped(dialog, "response", 
                    G_CALLBACK (gtk_widget_show),
                    parent);
     }
     */
     g_signal_connect_swapped (dialog, "response",
                G_CALLBACK (closeQuickDialog),
                dialog);


     // Add the label, and show everything we have added
     auto vbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
     g_object_set_data(G_OBJECT(dialog), "vbox", vbox);
     
     
     if (icon){
        auto pixbuf = Pixbuf<Type>::getPixbuf(icon, -48);
        if (pixbuf) {
            auto image = gtk_image_new_from_pixbuf(pixbuf);
            if (image) {
                compat<bool>::boxPack0(vbox, image, FALSE, FALSE,0);
                gtk_widget_show (image);
            }
        }
     }
     compat<bool>::boxPack0(GTK_BOX(content_area), GTK_WIDGET(vbox), TRUE, TRUE,0);
     auto hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
     compat<bool>::boxPack0(vbox, GTK_WIDGET(hbox), TRUE, TRUE, 0);
     auto button = Gtk<Type>::dialog_button(WINDOW_CLOSE, "");
     compat<bool>::boxPack0(hbox, GTK_WIDGET(label), TRUE, TRUE,0);
     auto vbox2 = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
     compat<bool>::boxPack0(hbox, GTK_WIDGET(button), FALSE, FALSE,0);
     g_signal_connect (button, "clicked",
                G_CALLBACK (onQuickCancel),
                dialog);

     //auto vbox2 = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
   
     //gtk_box_pack_end(vbox, GTK_WIDGET(vbox2), FALSE, FALSE,0);
     //auto hbox2 = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
     //gtk_box_pack_end(vbox2, GTK_WIDGET(hbox2), FALSE, FALSE,0);
     //auto button = Gtk<Type>::dialog_button("redball", _("Close"));
     //gtk_box_pack_end(hbox2, GTK_WIDGET(button), FALSE, FALSE,0);
     //g_object_set_data(G_OBJECT(dialog), "buttonBox", (void *)hbox2); 

        
        
     MainDialog = GTK_WINDOW(dialog);

     return dialog;
    }


private:

    static void
    onQuickCancel (GtkWidget * button, gpointer data) {
        auto dialog = GTK_DIALOG(data);
        MainDialog = NULL;
        gtk_dialog_response(dialog, GTK_RESPONSE_CANCEL );
    }
    
    static void
    closeQuickDialog(GtkWidget *widget, GdkEventKey * event, void *data){
        TRACE("closeQuickDialog\n");
        gtk_widget_hide(widget);
        auto parent = GTK_WINDOW(g_object_get_data(G_OBJECT(widget), "parent"));
        if (parent && GTK_IS_WINDOW(parent)) {
          gtk_window_present_with_time(parent, GDK_CURRENT_TIME);
        }
        gtk_widget_destroy(widget);
        MainDialog = NULL;
        
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
