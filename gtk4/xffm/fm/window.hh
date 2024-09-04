#ifndef XF_WINDOW_HH
#define XF_WINDOW_HH
#define NOTEBOOK_CALLBACK(X)  G_CALLBACK((void (*)(GtkNotebook *,GtkWidget *, guint, gpointer)) X)

#include "mainMenu.hh"
#include "fmpage.hh"
#include <gio/gio.h>
namespace xf {

template <class VbuttonClass, class PageClass> 
class MainWindow: public VbuttonClass, public UtilBasic {
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
        auto child = Util::getCurrentChild();
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
            Util::print(GTK_TEXT_VIEW(input), g_strdup_printf("%c", keyval));
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
 
        Util::setAsDialog(widget, "xffm", "Xffm");

        gtk_widget_realize(GTK_WIDGET(mainWindow_));
        auto input = Util::getCurrentInput();
        gtk_widget_grab_focus(GTK_WIDGET(input));
        
        
        
        gtk_window_present (mainWindow_);

    }

    void addPage(const gchar *path){
      auto page = new PageClass(path);
      auto child = page->childBox();
      g_object_set_data(G_OBJECT(child), "page", page);

      //GtkBox *child = this->mkPageBox(path);
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
      Util::setWorkdir(path, true);
      gtk_widget_grab_focus(GTK_WIDGET(Util::getCurrentInput()));
     
    }
 
    void zapPage(){
      TRACE("zapPage...\n");

      auto num = gtk_notebook_get_current_page(notebook_);
      auto child = gtk_notebook_get_nth_page(notebook_, num);
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(child), "output"));
      Util::unreference_textview(output);
      GList *item = g_list_find(pageList_, child);
      pageList_ = g_list_remove(pageList_, child);
      if (g_list_length(pageList_) == 0){
        gtk_widget_set_visible (GTK_WIDGET(mainWindow_), FALSE);
        gtk_window_destroy(mainWindow_);
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
      auto page = (PageClass *) g_object_get_data(G_OBJECT(child), "page");
      gtk_notebook_remove_page(notebook_, gtk_notebook_get_current_page(notebook_));
      delete(page);
//      Util::flushGTK();
//      gtk_widget_grab_focus(GTK_WIDGET(Util::getCurrentInput()));
      
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
      
      setWindowTitle(child);    
      gtk_widget_grab_focus(GTK_WIDGET(input));
      
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
      auto menuButtonBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));

      auto newTabButton = Util::newButton("list-add", _("New Tab"));
      auto newMenuButton = Util::newMenuButton("open-menu", NULL);

      auto myMainMenu = new Menu<MainMenu>;
      auto markup = g_strdup_printf("<span color=\"blue\"><b>%s</b></span>", _("Main menu"));
      auto menu = myMainMenu->getMenu(markup);
      g_free(markup);
      delete myMainMenu;
      
      gtk_menu_button_set_popover (newMenuButton, GTK_WIDGET(menu));  



      g_signal_connect(G_OBJECT(newTabButton), "clicked", 
              BUTTON_CALLBACK(on_new_page), (void *)this);    
      g_signal_connect (notebook_, "switch-page", 
                NOTEBOOK_CALLBACK (on_switch_page), (void *)this);
      
      gtk_widget_set_hexpand(GTK_WIDGET(actionWidget), FALSE);

      Util::boxPack0(tabButtonBox, GTK_WIDGET(longPressImage_),  TRUE, FALSE, 0);
      Util::boxPack0(tabButtonBox, GTK_WIDGET(newTabButton),  TRUE, FALSE, 0);
      Util::boxPack0(menuButtonBox, GTK_WIDGET(newMenuButton),  TRUE, FALSE, 0);
      Util::boxPack0(actionWidget, GTK_WIDGET(tabButtonBox),  TRUE, FALSE, 0);
      Util::boxPack0(actionWidget, GTK_WIDGET(menuButtonBox),  TRUE, FALSE, 0);

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

private:
};



}
#endif
