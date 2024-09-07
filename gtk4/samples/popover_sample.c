/* 
 * Build:
 * gcc -ggdb `pkg-config --cflags gtk4` popover_sample.c -o popover_sample `pkg-config --libs gtk4` -lSM -lICE -lX11 -lXext
 * gcc -ggdb `pkg-config --cflags gtk4` popover_sample.c -o popover_sample `pkg-config --libs gtk4 `
 */
# include <gdk/x11/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <gtk/gtk.h>
// No gtkApp: keep it simple.
    static void setAsDialog(GtkWidget *widget, const char *Xname, const char *Xclass){
        bool OK = false;
 #ifdef GDK_WINDOWING_X11
        GdkDisplay *displayGdk = gdk_display_get_default();
        if (GDK_IS_X11_DISPLAY (displayGdk)) {
          OK = true;
          Display *display = gdk_x11_display_get_xdisplay(displayGdk);
          XClassHint *wm_class = (XClassHint *)calloc(1, sizeof(XClassHint));
          wm_class->res_name = g_strdup(Xname);
          wm_class->res_class = g_strdup(Xclass);

          GtkNative *native = gtk_widget_get_native(widget);
          GdkSurface *surface = gtk_native_get_surface(native);
          Window w = gdk_x11_surface_get_xid (surface);
          XSetClassHint(display, w, wm_class);

          Atom atom = gdk_x11_get_xatom_by_name_for_display (displayGdk, "_NET_WM_WINDOW_TYPE_DIALOG");
          Atom atom0 = gdk_x11_get_xatom_by_name_for_display (displayGdk, "_NET_WM_WINDOW_TYPE");
          XChangeProperty (display, w,
            atom0, XA_ATOM, 
            32, PropModeReplace,
            (guchar *)&atom, 1);
        }
#endif
#ifdef GDK_WINDOWING_WAYLAND
//#warning "Compiling for Wayland (unstable)"
        OK = true;
#endif        
#ifdef GDK_WINDOWING_WIN32
#warning "Compiling for Windows (unstable)"
        OK = true;
#endif
        if (!OK) {
          g_error ("Unsupported GDK backend");
          exit(1);
        }
   }

static void
activate(GtkWidget *self, gpointer data) { 
  char *string = data;
  fprintf(stderr,"activate: %s\n", string);
  GtkPopover *menu = g_object_get_data(G_OBJECT(self), "menu");
  gtk_popover_popdown(menu);
  if (strcmp(string, "quit")==0){
    fprintf(stderr,"goodbye.\n");
    GtkWindow *window = g_object_get_data(G_OBJECT(menu), "window");
    if (window) gtk_window_destroy(window);
    else fprintf(stderr, "activate():: programming error\n");
  }
  return;
}

static GtkWidget *mkMenu(const gchar **items, GCallback *callback, void **data){
  GtkWidget *vbox;
  GtkWidget *menu = gtk_popover_new ();
  gtk_popover_set_has_arrow(GTK_POPOVER(menu), FALSE);
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

  GCallback *q = callback;
  void **r = data;
  for (const gchar **p=items; p && *p; p++, q++, r++){
    GtkWidget *item = gtk_button_new_with_label(*p);
    gtk_button_set_has_frame(GTK_BUTTON(item), FALSE);
    gtk_box_append (GTK_BOX (vbox), item);
    g_signal_connect (G_OBJECT (item), "clicked", *q, *r);
    g_object_set_data(G_OBJECT(item), "menu", menu);
  }
      
  gtk_popover_set_child (GTK_POPOVER (menu), vbox);
  return menu;

}

static GtkWidget *
mkButton(const char *iconName){
  GtkWidget *button = gtk_menu_button_new();
  gtk_menu_button_set_icon_name(GTK_MENU_BUTTON(button), iconName);
  gtk_widget_set_valign (button, GTK_ALIGN_CENTER);
  gtk_widget_set_halign (button, GTK_ALIGN_CENTER);
  return button;
}

static gboolean
gestureCallback (
        GtkGestureClick* self,
        gint n_press,
        gdouble x,
        gdouble y,
        gpointer data ) 
{
  //GtkWidget *box = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
  GtkWidget *box = GTK_WIDGET(data);
  GtkPopover *popover = GTK_POPOVER(g_object_get_data(G_OBJECT(box), "popover"));

  GtkLabel *label = GTK_LABEL(g_object_get_data(G_OBJECT(box), "label"));
  const char *text = gtk_label_get_label(label);
  fprintf(stderr, "label=%p text=%s\n", label, text);
  GtkLabel *popoverLabel = GTK_LABEL(g_object_get_data(G_OBJECT(popover), "label"));
  char *markup = g_strconcat("foo:", text, NULL);
  gtk_label_set_markup(popoverLabel, markup);
  g_free(markup);
  gtk_popover_popup(popover);
  return TRUE;
}

int main (int argc, char *argv[])
{
  gtk_init ();
    
  GtkWidget *window = gtk_window_new ();
  gtk_window_set_default_size (GTK_WINDOW (window), 300, 100);
  gtk_widget_add_css_class (GTK_WIDGET(window), "main" );
  
  GtkWidget *button = mkButton("open-menu-symbolic");
  GtkBox *box = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10));
  gtk_window_set_child(GTK_WINDOW(window), GTK_WIDGET(box));
  GtkBox *vbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
  gtk_box_append(vbox, button);
  gtk_box_append(box, GTK_WIDGET(vbox));


  
  for (int i=0; i<3; i++){
    vbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
    GtkWidget *label = gtk_label_new("");
    char *markup = g_strdup_printf("<b>label %d</b>", i+1);
    gtk_label_set_markup(GTK_LABEL(label), markup);
    g_free(markup);
    g_object_set_data(G_OBJECT(vbox), "label", label);
  fprintf(stderr, "label(%d)=%p\n", i+1, label);
    gtk_box_append(vbox, label);
    gtk_box_append(box, GTK_WIDGET(vbox));

    // For popovers not associated to a GtkButtonMenu:
    // XXX One popover for each widget, to be set with
    //     gtk_popover_set_child(), Parent for popover
    //     should be set to the parent of such widget.
    // XXX One gesture click for each widget, with
    //     reference to the popover associated with
    //     such widget.
    // If you use only one popover or one gesture click
    // for multiple widgets, bad things will happen.
    //
    GtkWidget *popover = gtk_popover_new();
    GtkWidget *label2 = gtk_label_new("foo");
    gtk_popover_set_child(GTK_POPOVER(popover), label2);
    gtk_widget_set_parent(popover, GTK_WIDGET(vbox));
    
    g_object_set_data(G_OBJECT(popover), "label", label2);
    g_object_set_data(G_OBJECT(vbox), "popover", popover);

    GtkGestureClick *gesture = GTK_GESTURE_CLICK(gtk_gesture_click_new());
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),3);
    g_signal_connect (G_OBJECT(gesture) , "released", G_CALLBACK (gestureCallback), (void *)vbox);
    gtk_widget_add_controller(GTK_WIDGET(vbox), GTK_EVENT_CONTROLLER(gesture));

  }



  const gchar *items[]={"Hello World 1","Hello World 2","Hello World 3","quit",NULL};
  GCallback callbacks[]={G_CALLBACK(activate), G_CALLBACK(activate), G_CALLBACK(activate), G_CALLBACK(activate), NULL};
  void *data[]={(void *)"test1", (void *)"test12", (void *)"test123", (void *)"quit", NULL};
  GtkWidget *menu = mkMenu(items, callbacks, data);
  g_object_set_data(G_OBJECT(menu), "window", window);
  
  gtk_menu_button_set_popover (GTK_MENU_BUTTON (button), menu);
  gtk_widget_realize(window);
  setAsDialog(window, "test", "Test");

  gtk_window_present (GTK_WINDOW(window));
  //        gtk_popover_popup(GTK_POPOVER(menu));// crash

  while (g_list_model_get_n_items (gtk_window_get_toplevels ()) > 0)
    g_main_context_iteration (NULL, TRUE);

  return 0;
}

