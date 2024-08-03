#ifndef XF_WINDOW_HH
#define XF_WINDOW_HH
#define NOTEBOOK_CALLBACK(X)  G_CALLBACK((void (*)(GtkNotebook *,GtkWidget *, guint, gpointer)) X)
#include "fmpage.hh"

namespace xf {


template <class VbuttonClass, class PageClass> 
class MainWindow: public VbuttonClass, public PageClass {
  using mainWindow_c = MainWindow<VbuttonClass, PageClass>;
// We need to inherit VbuttonClass so as to instantiate object.
private:
    GList *pageList_=NULL;
    GtkWindow *mainWindow_ = NULL;
    GtkNotebook *notebook_;
    double windowH_ = 400;    
    double windowW_ = 400*1.618;    
    GList *run_button_list=NULL;
    pthread_mutex_t *rbl_mutex; // run button list mutex

    GtkWidget *longPressImage_=NULL;
    GHashTable *pageHash_=NULL;
// Constructor  
public:
    GtkNotebook *notebook(void) {return notebook_;}
//    GtkNotebook *getNotebook(void) {return notebook_;}
    MainWindow(const gchar *path){
        createWindow(); 
        addKeyController(GTK_WIDGET(mainWindow_));
          // for page: startDeviceMonitor();
        auto box = contentBox(path);
        gtk_window_set_child(mainWindow_, box);
        addPage(path);
        showWindow();
    }

    ~MainWindow(void){
       // for each page: g_file_monitor_cancel(deviceMonitor_);
    }
// Free functions (for signals)
public:
    static void
    on_new_page(GtkButton *button, void *data){
        MainWindow *w = (MainWindow *)data;
        w->addPage(Util::getWorkdir());
    }
    static void
    on_zap_page(GtkButton *button, void *data){
        MainWindow *w = (MainWindow *)data;
        w->zapPage();
    }
    static void
    on_switch_page(GtkNotebook *notebook,
                   GtkWidget   *page,
                   guint        new_page,
                   gpointer     data){
        MainWindow *w = (MainWindow *)data;
        TRACE("new page=%d\n", new_page);
        w->switchPage(new_page);
    }
    static gboolean
    on_keypress (GtkEventControllerKey* self,
          guint keyval,
          guint keycode,
          GdkModifierType state,
          gpointer data){
        
        MainWindow *w = (MainWindow *)data;
        auto notebook = GTK_NOTEBOOK(w->notebook());
        auto num = gtk_notebook_get_current_page(notebook);
        auto child = gtk_notebook_get_nth_page(notebook, num); //page box
        auto input = GTK_WIDGET(g_object_get_data(G_OBJECT(child), "input"));
        bool termKey = (keyval >= GDK_KEY_space && keyval <= GDK_KEY_asciitilde);
        if (!gtk_widget_is_focus(input)) {
          gtk_widget_grab_focus(input);
          if (termKey) Util::print(GTK_TEXT_VIEW(input), g_strdup_printf("%c", keyval));
        }
        return FALSE;
    }

private:
    void addKeyController(GtkWidget  *widget){
        auto keyController = gtk_event_controller_key_new();
        gtk_event_controller_set_propagation_phase(keyController, GTK_PHASE_CAPTURE);
        gtk_widget_add_controller(GTK_WIDGET(widget), keyController);
        g_signal_connect (G_OBJECT (keyController), "key-pressed", 
            G_CALLBACK (this->on_keypress), (void *)this);
    }

    void createWindow(void){
        mainWindow_ = GTK_WINDOW(gtk_window_new ());
        MainWidget = GTK_WIDGET(mainWindow_);
        g_object_set_data(G_OBJECT(mainWindow_), "windowObject", (void *)this);
        gtk_window_set_default_size(mainWindow_, windowW_, windowH_);
        return;
    }

    void showWindow(){

        GtkWidget *widget = GTK_WIDGET(mainWindow_);
        gtk_widget_realize(widget);
        bool OK = false;

#ifdef GDK_WINDOWING_X11
        GdkDisplay *displayGdk = gdk_display_get_default();
        if (GDK_IS_X11_DISPLAY (displayGdk)) {
          OK = true;
          Display *display = gdk_x11_display_get_xdisplay(displayGdk);
          XClassHint *wm_class = (XClassHint *)calloc(1, sizeof(XClassHint));
          wm_class->res_name = g_strdup("xffm");
          wm_class->res_class = g_strdup("Xffm");

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
        gtk_window_present (mainWindow_);

    }

    void addPage(const gchar *path){
      
      GtkBox *child = this->mkPageBox(path);
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(child), "output"));
      Util::reference_textview(output);
      pageList_ = g_list_append(pageList_, child);
      auto label = tabLabel(path, (void *)this);
      auto close = g_object_get_data(G_OBJECT(label), "close");
      g_object_set_data(G_OBJECT(child), "close", close);

      auto num = gtk_notebook_append_page (notebook_, GTK_WIDGET(child), label);
      gtk_widget_realize(GTK_WIDGET(child));
      Util::flushGTK();
      
      if (num >= 0) {
        while (num != gtk_notebook_get_current_page(notebook_)) 
          gtk_notebook_next_page(notebook_);
      }
     
    }

    void zapPage(){
      DBG("zapPage...\n");
      GtkWidget *child = Util::getCurrentChild();
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(child), "output"));
      Util::unreference_textview(output);
      GList *item = g_list_find(pageList_, child);
      pageList_ = g_list_remove(pageList_, child);
      if (g_list_length(pageList_) == 0){
        gtk_widget_set_visible (GTK_WIDGET(mainWindow_), FALSE);
        exit(0);
      }
      // Get VPane object from child widget (box)
      Vpane *vpane_object =  (Vpane *)g_object_get_data(G_OBJECT(child), "vpane_object");
      Prompt *prompt_object =  (Prompt *)g_object_get_data(G_OBJECT(child), "prompt_object");
      gtk_notebook_remove_page(notebook_, gtk_notebook_get_current_page(notebook_));
      if (vpane_object) delete(vpane_object);
      if (prompt_object) delete(prompt_object);
      
    }

    void switchPage (gint new_page) {
      // hide all close buttons
      for (GList *l=pageList_; l && l->data; l=l->next){
        GtkWidget *box = (GtkWidget *)l->data;
        GtkWidget *w = GTK_WIDGET(g_object_get_data(G_OBJECT(box), "close"));
        gtk_widget_set_visible (w, FALSE);
      }

      // Show current close button
      auto child = gtk_notebook_get_nth_page(notebook_, new_page);
      auto close = GTK_WIDGET(g_object_get_data(G_OBJECT(child), "close"));
      gtk_widget_set_visible (close, TRUE);
      
      Util::setWindowTitle();     
    }


    GtkWidget *tabLabel(const gchar *path, void *data){
      MainWindow *w = (MainWindow *)data;
      auto tabBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
      gtk_widget_set_hexpand(GTK_WIDGET(tabBox), FALSE);
      gtk_widget_set_vexpand(GTK_WIDGET(tabBox), FALSE);
      gchar *tag = path? g_path_get_basename(path):g_strdup(".");
      GtkWidget *label = gtk_label_new(tag);
      g_free(tag);
      Util::boxPack0(tabBox, label,  FALSE, FALSE, 0);

      auto close = Util::newButton(WINDOW_CLOSE, _("Close"));
      g_signal_connect(G_OBJECT(close), "clicked", 
              BUTTON_CALLBACK(w->on_zap_page), data);    
      g_object_set_data(G_OBJECT(tabBox), "close", close);
      g_object_set_data(G_OBJECT(tabBox), "label", label);
      
      Util::boxPack0(tabBox, GTK_WIDGET(close),  FALSE, FALSE, 0);
      return GTK_WIDGET(tabBox);
    }
    

    void mkNotebook(){
      notebook_ = GTK_NOTEBOOK(gtk_notebook_new());
      g_object_set_data(G_OBJECT(MainWidget), "notebook", notebook_);
      gtk_notebook_set_scrollable (notebook_, TRUE);

      longPressImage_ = gtk_label_new("");
      gtk_widget_set_visible (GTK_WIDGET(longPressImage_), FALSE);
      auto text = g_strdup_printf("<span color=\"red\">%s</span>",_("Long press time"));
      gtk_label_set_markup(GTK_LABEL(longPressImage_),text);
      g_free(text);

      auto actionWidget = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
      auto tabButtonBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));

      auto newTabButton = Util::newButton("list-add", _("New Tab"));


      g_signal_connect(G_OBJECT(newTabButton), "clicked", 
              BUTTON_CALLBACK(mainWindow_c::on_new_page), (void *)this);    
      g_signal_connect (notebook_, "switch-page", 
                NOTEBOOK_CALLBACK (on_switch_page), (void *)this);
      
      gtk_widget_set_hexpand(GTK_WIDGET(actionWidget), FALSE);

      Util::boxPack0(tabButtonBox, GTK_WIDGET(longPressImage_),  TRUE, FALSE, 0);
      Util::boxPack0(tabButtonBox, GTK_WIDGET(newTabButton),  TRUE, FALSE, 0);
      //Util::boxPack0(tabButtonBox, GTK_WIDGET(this->menuButton()),  TRUE, FALSE, 0);
      Util::boxPack0(actionWidget, GTK_WIDGET(tabButtonBox),  TRUE, FALSE, 0);

      gtk_notebook_set_action_widget (notebook_, GTK_WIDGET(actionWidget), GTK_PACK_END);

    }

    GtkWidget *contentBox(const gchar *path){
      pthread_mutexattr_t r_attr;
      pthread_mutexattr_init(&r_attr);
      pthread_mutexattr_settype(&r_attr, PTHREAD_MUTEX_RECURSIVE);
      rbl_mutex = (pthread_mutex_t *)calloc(1, sizeof(pthread_mutex_t));
      pthread_mutex_init(rbl_mutex, &r_attr);

      auto mainBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
      auto hbox1 = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
      Util::boxPack0(mainBox, GTK_WIDGET(hbox1),  TRUE, TRUE, 0);
      gtk_widget_set_hexpand(GTK_WIDGET(hbox1), TRUE);

      mkNotebook();
      Util::boxPack0(hbox1, GTK_WIDGET(notebook_),  TRUE, TRUE, 0);
      g_object_set_data(G_OBJECT(MainWidget), "notebook", notebook_);

      auto hbox2 = this->mkVbuttonBox();  // More precise.  
//      auto hbox2 = VbuttonClass::mkVbuttonBox(); // This works too, but less clear.   
      Util::boxPack0(mainBox, GTK_WIDGET(hbox2),  FALSE, FALSE, 0);

      return GTK_WIDGET(mainBox);
    }

};



}
#endif
