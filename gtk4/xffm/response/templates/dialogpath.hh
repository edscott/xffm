#ifndef DIALOGPATH_HH
#define DIALOGPATH_HH
// Simple entry dialog, no file chooser.

namespace xf {

template <class Type>
class dialogPath {
    static DialogEntry<Type> *getObject(const char *path){
      auto dialogObject = new DialogEntry<Type>;
      dialogObject->setParent(GTK_WINDOW(Child::mainWidget()));
      auto dialog = dialogObject->dialog();
      auto entry = GTK_ENTRY( g_object_get_data(G_OBJECT(dialog),"entry"));
      g_object_set_data(G_OBJECT(entry), "path", g_strdup(path));

      dialogObject->subClass()->setDefaults(dialog, dialogObject->label());
      return dialogObject;
    }  

public:
    static void action(const char *path){    
      auto dialogObject = getObject(path);
      dialogObject->run();
    }
#if 0
    static void action(const char *path, double x_, double y_){
      DBG("action %lf,%lf\n", x_, y_);
      int x = x_;
      int y = y_;    
      auto dialogObject = getObject(path);
      auto dialog = dialogObject->dialog();
/* This does not work because x_, y_ are relative to the gtk label,
 * we would need to get the X_, y_ relative to the root window.*/
      gtk_widget_realize(GTK_WIDGET(dialog));
      GtkNative *native = gtk_widget_get_native(GTK_WIDGET(dialog));
      GdkSurface *surface = gtk_native_get_surface(native);
      Window w = gdk_x11_surface_get_xid (surface);
      GdkDisplay *display = gtk_widget_get_display (GTK_WIDGET(dialog));
      Display *d = gdk_x11_display_get_xdisplay(display);
      XMoveWindow(d, w, x, y);
      dialogObject->run();
    }
#endif
 };
}
#endif
