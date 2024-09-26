#ifndef XF_WINDOW_HH
#define XF_WINDOW_HH
#define NOTEBOOK_CALLBACK(X)  G_CALLBACK((void (*)(GtkNotebook *,GtkWidget *, guint, gpointer)) X)

#include <gio/gio.h>
namespace xf {

class MainWindow: public FMbuttonBox {
// We need to inherit FMbuttonBox so as to instantiate object.
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
        //g_object_set_data(G_OBJECT(mainWindow_), "MainWindow", this);
        addKeyController(GTK_WIDGET(mainWindow_));
          // for page: startDeviceMonitor();
        auto box = contentBox(path);
        gtk_window_set_child(mainWindow_, box);
        addPage(path);
        showWindow();
        g_object_set_data(G_OBJECT(MainWidget), "MainWindow", this);
    }

    ~MainWindow(void){
       // for each page: g_file_monitor_cancel(deviceMonitor_);
    } 
// Free functions (for signals)
public:
    static void
    on_new_page(GtkButton *button, void *data){
        MainWindow *w = (MainWindow *)data;
        auto child = Child::getChild();
        w->addPage(Child::getWorkdir(child));
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
        auto vpane = GTK_WIDGET(g_object_get_data(G_OBJECT(child), "vpane"));
        auto input = GTK_WIDGET(g_object_get_data(G_OBJECT(child), "input"));
       // auto promptBox = GTK_WIDGET(g_object_get_data(G_OBJECT(input), "promptBox"));
        bool termKey = (keyval >= GDK_KEY_space && keyval <= GDK_KEY_asciitilde);
        bool upArrow = (keyval == GDK_KEY_Up || keyval == GDK_KEY_KP_Up);
        bool switchKey = (keyval == GDK_KEY_Tab || keyval == GDK_KEY_Escape);


        //if (gtk_widget_get_visible(promptBox) && !gtk_widget_is_focus(input)) {
        if (!gtk_widget_is_focus(input)) {
          TRACE("window on_keypress,  focusing input\n");
          if (switchKey) {
            gtk_widget_grab_focus(input);
            return TRUE;
          }
          if (termKey) {
            gtk_widget_grab_focus(input);
            Print::print(GTK_TEXT_VIEW(input), g_strdup_printf("%c", keyval));
            return TRUE;
          }
          if (upArrow && gtk_paned_get_position(GTK_PANED(vpane)) < 10) {
            gtk_widget_grab_focus(input);
            if (upArrow) History::up(GTK_TEXT_VIEW(input));   
            return TRUE;
          }      
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

    static gboolean
    presentDialog ( GtkEventControllerMotion* self,
                    gdouble x,
                    gdouble y,
                    gpointer data) 
    {
      TRACE("present dialog %p\n", MainDialog);
      if (MainDialog) gtk_window_present(MainDialog);
        return FALSE;
    }

    static
      void addMotionController(GtkWidget  *widget){
        auto controller = gtk_event_controller_motion_new();
        gtk_event_controller_set_propagation_phase(controller, GTK_PHASE_CAPTURE);
        gtk_widget_add_controller(GTK_WIDGET(widget), controller);
        g_signal_connect (G_OBJECT (controller), "enter", 
            G_CALLBACK (presentDialog), NULL);
    }
    
    void createWindow(void){
        mainWindow_ = GTK_WINDOW(gtk_window_new ());
        MainWidget = GTK_WIDGET(mainWindow_);
        addMotionController(MainWidget);
        g_object_set_data(G_OBJECT(mainWindow_), "windowObject", (void *)this);
        gtk_window_set_default_size(mainWindow_, windowW_, windowH_);
        return;
    }

    void showWindow(){

        GtkWidget *widget = GTK_WIDGET(mainWindow_);
        gtk_widget_realize(widget);
 
        Basic::setAsDialog(widget, "xffm", "Xffm");

        gtk_widget_realize(GTK_WIDGET(mainWindow_));
        auto input = Child::getInput();
        gtk_widget_grab_focus(GTK_WIDGET(input));
        
        
        
        gtk_window_present (mainWindow_);

    }

    void addChild(GtkBox *child){
      pageList_ = g_list_append(pageList_, child);

    }
    
    GtkWidget *tabLabel(const gchar *path, void *data){
      MainWindow *w = (MainWindow *)data;
      auto tabBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
      gtk_widget_set_hexpand(GTK_WIDGET(tabBox), FALSE);
      gtk_widget_set_vexpand(GTK_WIDGET(tabBox), FALSE);
      gchar *tag = path? g_path_get_basename(path):g_strdup(".");
      GtkWidget *label = gtk_label_new(tag);
      g_free(tag);
      Basic::boxPack0(tabBox, label,  FALSE, FALSE, 0);

      auto close = Basic::newButton(WINDOW_CLOSE, _("Close"));
      g_signal_connect(G_OBJECT(close), "clicked", 
              BUTTON_CALLBACK(w->on_zap_page), data);    
      g_object_set_data(G_OBJECT(tabBox), "close", close);
      g_object_set_data(G_OBJECT(tabBox), "label", label);
      
      Basic::boxPack0(tabBox, GTK_WIDGET(close),  FALSE, FALSE, 0);
      return GTK_WIDGET(tabBox);
    }
public:
    void addPage(const gchar *path){
      auto page = new FMpage(path);
      auto child = page->childBox();
      g_object_set_data(G_OBJECT(child), "page", page);
      Child::add(GTK_WIDGET(child));

      //GtkBox *child = this->mkPageBox(path);
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(child), "output"));
      Print::reference_textview(output);
      addChild(child);
      auto label = tabLabel(path, (void *)this);
      auto close = g_object_get_data(G_OBJECT(label), "close");
      g_object_set_data(G_OBJECT(child), "close", close);

      auto num = gtk_notebook_append_page (notebook_, GTK_WIDGET(child), label);
      gtk_widget_realize(GTK_WIDGET(child));
      Basic::flushGTK();
#ifdef ENABLE_MENU_CLASS
        auto pathbar_ = Child::getPathbar();
        auto myPathbarMenu = new Menu<PathbarMenu>;
        auto title = g_strconcat("<span color=\"blue\">", _("foo Navigation Toolbar"), "</span>", NULL);
        auto menu = myPathbarMenu->getMenu(title);
        g_free(title);
        g_object_set_data(G_OBJECT(pathbar_), "menu", menu);
        // Important: must use both of the following instructions:
        //gtk_popover_set_default_widget(menu, GTK_WIDGET(pathbar_));
        //gtk_popover_set_default_widget(menu, GTK_WIDGET(MainWidget));
        //gtk_widget_set_parent(GTK_WIDGET(menu), GTK_WIDGET(MainWidget));
        DBG("menu parent = %p (should be null)\n", gtk_widget_get_parent(GTK_WIDGET(menu)));
        gtk_widget_set_parent(GTK_WIDGET(menu), GTK_WIDGET(pathbar_));
        Print::addMenu(menu, GTK_WIDGET(pathbar_));
#endif      
      
      if (num >= 0) {
        while (num != gtk_notebook_get_current_page(notebook_)) 
          gtk_notebook_next_page(notebook_);
      }
      Workdir::setWorkdir(path, true);
      gtk_widget_grab_focus(GTK_WIDGET(Child::getInput()));
     
    }
private: 
    void zapPage(void){
      TRACE("zapPage...\n");

      auto num = gtk_notebook_get_current_page(notebook_);
      auto child = gtk_notebook_get_nth_page(notebook_, num);
      Child::remove(child);

      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(child), "output"));
      Print::unreference_textview(output);
      GList *item = g_list_find(pageList_, child);
      pageList_ = g_list_remove(pageList_, child);
      if (g_list_length(pageList_) == 0){
        gtk_widget_set_visible (GTK_WIDGET(mainWindow_), FALSE);
        gtk_window_destroy(mainWindow_);
        exitDialogs = true;
        //exit(0);
      }
      
      // Clear page history
      auto pathbar = GTK_BOX(g_object_get_data(G_OBJECT(child ), "pathbar"));
      auto historyBack = (GList *)g_object_get_data(G_OBJECT(pathbar), "historyBack");
      auto historyNext = (GList *)g_object_get_data(G_OBJECT(pathbar), "historyNext");
      if (historyBack){
        for (GList *l=historyBack; l && l->data; l=l->next) g_free(l->data);
        g_list_free(historyBack);
      }
      if (historyNext){
        for (GList *l=historyNext; l && l->data; l=l->next) g_free(l->data);
        g_list_free(historyNext);
      }

      // Get VPane object from child widget (box)
      auto page = (FMpage *) g_object_get_data(G_OBJECT(child), "page");
      gtk_notebook_remove_page(notebook_, gtk_notebook_get_current_page(notebook_));
      delete(page);
//      Basic::flushGTK();
//      gtk_widget_grab_focus(GTK_WIDGET(Print::getInput()));
      
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
      auto input = GTK_WIDGET(g_object_get_data(G_OBJECT(child), "input"));
      gtk_widget_set_visible (close, TRUE);
      
      Child::setWindowTitle(child);    
      gtk_widget_grab_focus(GTK_WIDGET(input));
      
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
      auto menuButtonBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));

      auto newTabButton = Basic::newButton("list-add", _("New Tab"));
      auto newMenuButton = Basic::newMenuButton("open-menu", NULL);

      auto myMainMenu = new Menu<MainMenu>(_("Main menu"));
      myMainMenu->setMenu(newMenuButton);
      delete myMainMenu;

      g_signal_connect(G_OBJECT(newTabButton), "clicked", 
              BUTTON_CALLBACK(on_new_page), (void *)this);    
      g_signal_connect (notebook_, "switch-page", 
                NOTEBOOK_CALLBACK (on_switch_page), (void *)this);
      
      gtk_widget_set_hexpand(GTK_WIDGET(actionWidget), FALSE);

      Basic::boxPack0(tabButtonBox, GTK_WIDGET(longPressImage_),  TRUE, FALSE, 0);
      Basic::boxPack0(tabButtonBox, GTK_WIDGET(newTabButton),  TRUE, FALSE, 0);
      Basic::boxPack0(menuButtonBox, GTK_WIDGET(newMenuButton),  TRUE, FALSE, 0);
      Basic::boxPack0(actionWidget, GTK_WIDGET(tabButtonBox),  TRUE, FALSE, 0);
      Basic::boxPack0(actionWidget, GTK_WIDGET(menuButtonBox),  TRUE, FALSE, 0);

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
      Basic::boxPack0(mainBox, GTK_WIDGET(hbox1),  TRUE, TRUE, 0);
      gtk_widget_set_hexpand(GTK_WIDGET(hbox1), TRUE);

      mkNotebook();
      Basic::boxPack0(hbox1, GTK_WIDGET(notebook_),  TRUE, TRUE, 0);
      g_object_set_data(G_OBJECT(MainWidget), "notebook", notebook_);

      auto hbox2 = this->mkVbuttonBox();  // More precise.  
      Basic::boxPack0(mainBox, GTK_WIDGET(hbox2),  FALSE, FALSE, 0);

      return GTK_WIDGET(mainBox);
    }

private:
};



}
#endif
