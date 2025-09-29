#ifndef XF_WINDOW_HH
#define XF_WINDOW_HH
#define NOTEBOOK_CALLBACK(X)  G_CALLBACK((void (*)(GtkNotebook *,GtkWidget *, guint, gpointer)) X)

#include <gio/gio.h>
namespace xf {
class Type;

template <class Type> 
class MainWindow: public FMbuttonBox {
    using clipboard_t = ClipBoard<LocalDir>;
// We need to inherit FMbuttonBox so as to instantiate object.
private:
    bool doFind_ = false;
    GtkPopover *menu_ = NULL;
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
    GtkWindow *mainWindow(void){return mainWindow_;}
    MainWindow(const gchar *path, bool doFind){
        mainWindow_p = (void *)this;
        doFind_ = doFind;
        createWindow(); 
        Child::mainWidget(GTK_WIDGET(mainWindow_));
        
        //g_object_set_data(G_OBJECT(mainWindow_), "MainWindow", this);
        addKeyController(GTK_WIDGET(mainWindow_));
          // for page: startDeviceMonitor();
        auto box = contentBox(path);
        g_object_set_data(G_OBJECT(mainWindow_), "frame", box);
        
        gtk_window_set_child(mainWindow_, box);
        addPage(path);
        showWindow();
          
        
        g_object_set_data(G_OBJECT(mainWindow_), "MainWindow", this);
    }

    ~MainWindow(void){
      //gtk_widget_unparent(GTK_WIDGET(menu_));
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
        bool deleteKey = (keyval == GDK_KEY_Delete);

        if (!gtk_widget_is_focus(input) && deleteKey) {
          TRACE("delete key now\n");
          auto gridView_p = (GridView<LocalDir> *) Child::getGridviewObject();
          auto selectionList = gridView_p->getSelectionList();
          if (selectionList){
            Dialogs::rmList(selectionList);
          }
    
          return TRUE;
        }

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
          //if (upArrow && gtk_paned_get_position(GTK_PANED(vpane)) < 10) {
          if (upArrow) {
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
    clearCSS ( GtkEventControllerMotion* self,
                    gdouble x,
                    gdouble y,
                    gpointer data) 
    {
      auto gridview_p = (GridView<LocalDir> *)Child::getGridviewObject();
      if (gridview_p){
        TRACE("Leaving window\n");
        Dnd<LocalDir>::resetGridviewCSS(gridview_p);
      }
      // deprecated auto topDialog = Basic::topDialog();
      // deprecated if (topDialog) gtk_window_present(topDialog);
        return FALSE;
    }

#if 0
    // deprecated
    static gboolean
    presentDialog ( GtkEventControllerMotion* self,
                    gdouble x,
                    gdouble y,
                    gpointer data) 
    {
      auto topDialog = Basic::topDialog();
      TRACE("present dialog %p\n", topDialog);
      if (topDialog) gtk_window_present(topDialog);
        return FALSE;
    }

#endif
    
    static gboolean updateButtons( GtkEventControllerMotion* self,
                    double x, double y, void *data) {
        auto c = (clipboard_t *)g_object_get_data(G_OBJECT(Child::mainWidget()), "ClipBoard");
        if (pasteButton) {
          gtk_widget_set_sensitive(GTK_WIDGET(pasteButton), c->validClipBoard());
        }
        return FALSE;
    }

    static
      void addMotionController(GtkWidget  *widget){
        auto controller = gtk_event_controller_motion_new();
        gtk_event_controller_set_propagation_phase(controller, GTK_PHASE_CAPTURE);
        gtk_widget_add_controller(GTK_WIDGET(widget), controller);
        g_signal_connect (G_OBJECT (controller), "enter", 
            G_CALLBACK (updateButtons), NULL);

    }

    static void notify(GObject* self, GParamSpec *pspec, void *data){
      auto call = g_param_spec_get_name(pspec);
      if (!call) return;
      //if (Basic::isXffmPaned()) return;

      if (strcmp(call, "default-width")==0 || strcmp(call, "default-height")==0){
        TRACE("Xinstance = %s paned=%d\n", Xname_, Basic::isXffmPaned());
        int w;
        int h;
        g_object_get (self, "default-width", &w, "default-height", &h, NULL);
        Settings::setInteger("window", "width", w);
        Settings::setInteger("window", "height", h);
        TRACE("call = %s w=%d h=%d\n", call,w,h);
      }
      return;
    }
    
    void createWindow(void){
        mainWindow_ = GTK_WINDOW(gtk_window_new ());
        //addMotionController(mainWindow_);
        auto dropController = Dnd<LocalDir>::createDropController(NULL);
        gtk_widget_add_controller (GTK_WIDGET(mainWindow_), GTK_EVENT_CONTROLLER (dropController));

        g_object_set_data(G_OBJECT(mainWindow_), "windowObject", (void *)this);

        // GTK4 bug workaround. drag motion will be limited to maximum
        // size of window + 45 in x and y.
        // So we give it a big size, so drag motion does not fail
        // and then put it to the correct user saved size.
         gtk_window_set_default_size(mainWindow_, 5000, 5000);
        // This bug may have been fixed, but you really cannot
        // count on that...
      
        // Set to default dialog size: at showWindow()
        return;
    }

    void showWindow(){
        GtkWidget *widget = GTK_WIDGET(mainWindow_);
        Basic::setAsDialog(mainWindow_);
        if (doFind_) return; // No X resources for main xffm window.
        gtk_widget_realize(widget);
 

        gtk_widget_realize(GTK_WIDGET(mainWindow_));
        auto input = Child::getInput();
        gtk_widget_grab_focus(GTK_WIDGET(input));
        
        g_signal_connect(G_OBJECT(mainWindow_), "notify", G_CALLBACK(notify), mainWindow_);
        
        // Only setting the default size to last setting screws things
        // up when last window was in i3 non dialog mode. 
        // XXX Instead we need to figure out how to determine
        //     if window is in dialog mode or not before saving
        //     window width and height...
        //     Ask on i3 list.
         auto w = Settings::getInteger("window", "width", windowW_);
         auto h = Settings::getInteger("window", "height", windowH_);
         gtk_window_set_default_size(mainWindow_, w, h);
        //gtk_window_set_default_size(mainWindow_, windowW_, windowH_);

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
      char *tag = Child::getTabname(path);
      GtkWidget *label = gtk_label_new(tag);
      g_free(tag);

      auto regexLabel = gtk_label_new("");
      Basic::boxPack0(tabBox, regexLabel,  FALSE, FALSE, 0);
      g_object_set_data(G_OBJECT(tabBox), "regexLabel", regexLabel);

      auto descending = Texture<bool>::getPicture(EMBLEM_DESCENDING);
      gtk_box_append(GTK_BOX(tabBox), GTK_WIDGET(descending));
      g_object_set_data(G_OBJECT(tabBox), "descending", descending);
      auto flags = Settings::getInteger(path, "flags", 0x40);
      gtk_widget_set_visible(GTK_WIDGET(descending), flags & 0x04);

      Basic::boxPack0(tabBox, label,  FALSE, FALSE, 0);
      
      auto hidden = Texture<bool>::getPicture(EMBLEM_HIDDEN);
      gtk_box_append(GTK_BOX(tabBox), GTK_WIDGET(hidden));
      g_object_set_data(G_OBJECT(tabBox), "hidden", hidden);
      gtk_widget_set_visible(GTK_WIDGET(hidden), flags & 0x01);
      

      auto bak = gtk_label_new("~");
      gtk_label_set_markup(GTK_LABEL(bak), "<span color=\"green\">~</span>");
      gtk_box_append(GTK_BOX(tabBox), GTK_WIDGET(bak));
      g_object_set_data(G_OBJECT(tabBox), "bak", bak);
      gtk_widget_set_visible(GTK_WIDGET(bak), flags & 0x02);

      auto close = Basic::newButton(WINDOW_CLOSE, _("Close"));
      g_signal_connect(G_OBJECT(close), "clicked", 
              BUTTON_CALLBACK(w->on_zap_page), data);    
      g_object_set_data(G_OBJECT(tabBox), "close", close);
      g_object_set_data(G_OBJECT(tabBox), "label", label);
      
      Basic::boxPack0(tabBox, GTK_WIDGET(close),  FALSE, FALSE, 0);
      return GTK_WIDGET(tabBox);
    }

public:
    
  static void resetAdj(int value){
        auto page_p = (FMpage *)Child::page();
        auto scrollW = page_p->gridScrolledWindow();
        auto adjustment = gtk_scrolled_window_get_vadjustment(scrollW);
        gtk_adjustment_set_value(adjustment, value);
  }
private:
  
    static void *resetPosition(void *data){
        auto arg = (void **)data;
        auto path = (const char *)arg[0];
        auto value_p = (double *)arg[1];
        auto adjustment = GTK_ADJUSTMENT(arg[2]);

        Basic::flushGTK();
        TRACE("*** new scrollW, new adjustment %p value  to %lf\n",  adjustment, *value_p);
        gtk_adjustment_set_value(adjustment, *value_p);
        return NULL;

    }

private:


      static void *reloadIt(void *data){
        auto arg = (void **)data;
        auto path = (const char *)arg[0];
        auto value_p = (double *)arg[1];
        auto adjustment = GTK_ADJUSTMENT(arg[2]);
        TRACE("*** reloadIt workdir is %s\n", path);

        Workdir<Type>::setWorkdir(path);

        Basic::context_function(resetPosition, arg);
        g_free(arg[0]);
        g_free(arg[1]);
        g_free(arg);
        return NULL;
      }

public:

      static void *threadReload(void *data){
        auto c = (clipboard_t *)g_object_get_data(G_OBJECT(Child::mainWidget()), "ClipBoard");
        TRACE("threadReload waiting for condition...\n");
        c->conditionWait();
        TRACE("Go ahead condition received.\n");
        
        Basic::context_function(reloadIt, data);
        return NULL;
      }
    

    static void update(char *path){
        auto page_p = (FMpage *)Child::page();
        auto scrollW = page_p->gridScrolledWindow();
        auto adjustment = gtk_scrolled_window_get_vadjustment(scrollW);
        auto value = gtk_adjustment_get_value(adjustment);
        auto value_p = (double *)calloc(1, sizeof(double));
        *value_p = value;
        
        void **arg = (void **)calloc(4, sizeof(void *));
        arg[0] = (void *) path;
        arg[1] = (void *)value_p;
        arg[2] = (void *)adjustment;

        TRACE("*** page=%p, scrolledWindow=%p, adjustmentValue= %lf\n", page_p, scrollW, value);
        // get scroll position arg[1] = ; 

        pthread_t thread;
        pthread_create(&thread, NULL, threadReload, arg);
        pthread_detach(thread);

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
      gtk_notebook_set_tab_reorderable (notebook_, GTK_WIDGET(child), true);
      gtk_widget_realize(GTK_WIDGET(child));
      //Basic::flushGTK();
#ifdef ENABLE_MENU_CLASS
#warning "ENABLE_MENU_CLASS active"
        auto pathbar_ = Child::getPathbar();
        auto myPathbarMenu = new Menu<PathbarMenu<Type> >;
        auto title = g_strconcat("<span color=\"blue\">", "foo Navigation Toolbar", "</span>", NULL);
        auto menu = myPathbarMenu->getMenu(title);
        g_free(title);
        g_object_set_data(G_OBJECT(pathbar_), "menu", menu);
        TRACE("menu parent = %p (should be null)\n", gtk_widget_get_parent(GTK_WIDGET(menu)));
        gtk_widget_set_parent(GTK_WIDGET(menu), GTK_WIDGET(pathbar_));
        Print::addMenu(menu, GTK_WIDGET(pathbar_));
#endif      
      
      if (num >= 0) {
        while (num != gtk_notebook_get_current_page(notebook_)) 
          gtk_notebook_next_page(notebook_);
      }
      Workdir<LocalDir>::setWorkdir(path, true);
      gtk_widget_grab_focus(GTK_WIDGET(Child::getInput()));
     
    }
    void hideWindow(void){
        gtk_widget_set_visible (GTK_WIDGET(mainWindow_), FALSE);
        Basic::flushGTK();
    }
private: 

    void zapPage(GtkWidget *child){

      auto page = (FMpage *) g_object_get_data(G_OBJECT(child), "page");
      auto gridview_p = (GridView<LocalDir> *) Child::getGridviewObject(child);
      GList *item = g_list_find(pageList_, child);
      pageList_ = g_list_remove(pageList_, child);
      gtk_notebook_remove_page(notebook_, gtk_notebook_get_current_page(notebook_));
      delete(page);
      delete gridview_p;
      Child::remove(child);

      if (g_list_length(pageList_) == 0){
        hideWindow();
        if (Basic::isXffmFloating()){
          auto width = gtk_widget_get_size(Child::mainWidget(),  GTK_ORIENTATION_HORIZONTAL);
          auto height = gtk_widget_get_size(Child::mainWidget(),  GTK_ORIENTATION_VERTICAL);
          //gtk_widget_set_visible(Child::mainWidget(), FALSE);
          Settings::setInteger("window", "width", width);
          Settings::setInteger("window", "height", height);
          TRACE("set saved size %d,%d\n", width, height);
        }
        gtk_widget_unparent(GTK_WIDGET(menu_));
        gtk_window_destroy(mainWindow_);
        exitDialogs = true;
      }
      Basic::flushGTK();
    
      
    }
    
    void zapPage(void){
      TRACE("zapPage... need to unparent gridview menu\n");

      auto num = gtk_notebook_get_current_page(notebook_);
      auto child = gtk_notebook_get_nth_page(notebook_, num);
      zapPage(child);
    }

    void switchPage (gint new_page) {
      // HMM. it seems that gtk internal does not really do this in main context...
      // hide all close buttons
      for (GList *l=pageList_; l && l->data; l=l->next){
        GtkWidget *box = (GtkWidget *)l->data;
        GtkWidget *w = GTK_WIDGET(g_object_get_data(G_OBJECT(box), "close"));
        gtk_widget_set_visible (w, FALSE);
      }

      // Show current close button
      auto child = gtk_notebook_get_nth_page(notebook_, new_page);
      TRACE("switchPage %p\n", child);
      auto close = GTK_WIDGET(g_object_get_data(G_OBJECT(child), "close"));
      auto input = GTK_WIDGET(g_object_get_data(G_OBJECT(child), "input"));
      gtk_widget_set_visible (close, TRUE);
      
      auto gridview_p = (GridView<Type> *)Child::getGridviewObject(child);
      if (gridview_p) {
        int flags = gridview_p->flags();
        const char *regexp = gridview_p->regexp();
        Child::setWindowTitle(child, flags, regexp);   
        TRACE("switchPage setWindowTitle %p, 0x%x, %s\n", child, flags, regexp);
      } else {
         TRACE("switchPage setWindowTitle %p\n", child);
         Child::setWindowTitle(child);   
      }
     
      auto selection = Child::selection(child);
      if (!selection) {
        gtk_widget_set_sensitive(GTK_WIDGET(cutButton), false);
        gtk_widget_set_sensitive(GTK_WIDGET(copyButton), false);
        gtk_widget_set_sensitive(GTK_WIDGET(pasteButton), false);
      } else {
        auto c = (clipboard_t *)g_object_get_data(G_OBJECT(mainWindow_), "ClipBoard");
        gtk_widget_set_sensitive(GTK_WIDGET(pasteButton), c->validClipBoard());
        auto bitset = gtk_selection_model_get_selection(selection);
        gtk_widget_set_sensitive(GTK_WIDGET(cutButton), (gtk_bitset_get_size(bitset) > 0));
        gtk_widget_set_sensitive(GTK_WIDGET(copyButton), (gtk_bitset_get_size(bitset) > 0));
      }

      /*
      auto gridview_p = (GridView<Type> *)Child::getGridviewObject(child);
      auto path = g_strdup(Child::getWorkdir(child));
      void *arg[]={(void *)path, (void *)child, NULL};
      Basic::context_function(switchReload, arg); 
      g_free(path);
      */
      gtk_widget_grab_focus(GTK_WIDGET(input));
      
    }

    static void mainPaste(GtkButton * button, void *data){
      auto gridView_p = (GridView<LocalDir> *) Child::getGridviewObject();
      auto target = g_strdup(gridView_p->path());
      auto c = (clipboard_t *)g_object_get_data(G_OBJECT(Child::mainWidget()), "ClipBoard");
      if (!c->validClipBoard()){
        // Should not happen.
        Print::printWarning(Child::getOutput(), g_strconcat(_("Invalid clip"), "\n", NULL));
        return;
      }

      cpDropResponse::performPasteAsync(target);
      g_free(target);      
    }

    static void mainCut(GtkButton * button, void *data){
      
      auto gridView_p = (GridView<LocalDir> *) Child::getGridviewObject();
      auto selectionList = gridView_p->getSelectionList();
      auto c =(clipboard_t *)g_object_get_data(G_OBJECT(Child::mainWidget()), "ClipBoard");
      if (selectionList){
        c->cutClipboardList(selectionList);
        gtk_selection_model_unselect_all(Child::selection());
        update(g_strdup(Child::getWorkdir()));
        Basic::freeSelectionList(selectionList);
       }
      return;
    }

    static void mainCopy(GtkButton * button, void *data){
      auto gridView_p = (GridView<LocalDir> *) Child::getGridviewObject();
      auto selectionList = gridView_p->getSelectionList();
      auto c =(clipboard_t *)g_object_get_data(G_OBJECT(Child::mainWidget()), "ClipBoard");
      if (selectionList){
        c->copyClipboardList(selectionList);
        gtk_selection_model_unselect_all(Child::selection());
        // No need to reload since copy items not emblemed (factory.hh)
        //update(g_strdup(Child::getWorkdir()));
        Basic::freeSelectionList(selectionList);
      }
      return;
    }

    void mkNotebook(){
      notebook_ = GTK_NOTEBOOK(gtk_notebook_new());
      mainNotebook = notebook_;
      
      g_object_set_data(G_OBJECT(mainWindow_), "notebook", notebook_);
#if 0
      gtk_notebook_set_scrollable (notebook_, TRUE);
#else
        gtk_notebook_set_scrollable (notebook_, false);
#endif

      longPressImage_ = gtk_label_new("");
      gtk_widget_set_visible (GTK_WIDGET(longPressImage_), FALSE);
      auto text = g_strdup_printf("<span color=\"red\">%s</span>",_("Long press time"));
      gtk_label_set_markup(GTK_LABEL(longPressImage_),text);
      g_free(text);

      auto actionWidget = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
      auto tabButtonBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
      auto menuButtonBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));

      cutButton = Basic::newButton(EMBLEM_CUT, _("Cut")); // global button (xffm.h)
      gtk_widget_set_sensitive(GTK_WIDGET(cutButton), false);    
      copyButton = Basic::newButton(EMBLEM_COPY, _("Copy")); // global button (xffm.h)
      gtk_widget_set_sensitive(GTK_WIDGET(copyButton), false);    
      pasteButton = Basic::newButton(EMBLEM_PASTE, _("Paste")); // global button (xffm.h)
      gtk_widget_set_sensitive(GTK_WIDGET(pasteButton), false);    
      auto newTabButton = Basic::newButton(EMBLEM_NEW_TAB, _("New Tab"));
      auto newMenuButton = Basic::newButton(OPEN_MENU, _("Main menu"));
      mainMenuButton = newMenuButton;// global button (xffm.h)
      auto myMainMenu = new Menu<MainMenu<Type> >(_("Main menu"));
      menu_ = myMainMenu->mkMenu(NULL, 
              (void *)MenuCallbacks<Type>::gestureRegexp,
              _("Match regular expression"));
      TRACE("menu popover = %p\n", menu_);
      gtk_widget_set_parent(GTK_WIDGET(menu_), GTK_WIDGET(newMenuButton));
      g_object_set_data(G_OBJECT(newMenuButton), "menu", menu_);
      delete myMainMenu;

      /*
      auto newMenuButton = Basic::newMenuButton(OPEN_MENU, NULL);
      auto myMainMenu = new Menu<MainMenu<Type> >(_("Main menu"));
      myMainMenu->setMenu(newMenuButton);
      delete myMainMenu;*/
      g_signal_connect(G_OBJECT(pasteButton), "clicked", 
              BUTTON_CALLBACK(mainPaste), (void *)this);   
      g_signal_connect(G_OBJECT(cutButton), "clicked", 
              BUTTON_CALLBACK(mainCut), (void *)this);   
      g_signal_connect(G_OBJECT(copyButton), "clicked", 
              BUTTON_CALLBACK(mainCopy), (void *)this);   


      g_signal_connect(G_OBJECT(newTabButton), "clicked", 
              BUTTON_CALLBACK(on_new_page), (void *)this);    
      g_signal_connect (notebook_, "switch-page", 
                NOTEBOOK_CALLBACK (on_switch_page), (void *)this);
      g_signal_connect (newMenuButton, "clicked", 
                G_CALLBACK (clickMenu), NULL);
      
      gtk_widget_set_hexpand(GTK_WIDGET(actionWidget), FALSE);

      Basic::boxPack0(tabButtonBox, GTK_WIDGET(longPressImage_),  TRUE, FALSE, 0);
      Basic::boxPack0(tabButtonBox, GTK_WIDGET(cutButton),  TRUE, FALSE, 0);
      Basic::boxPack0(tabButtonBox, GTK_WIDGET(copyButton),  TRUE, FALSE, 0);
      Basic::boxPack0(tabButtonBox, GTK_WIDGET(pasteButton),  TRUE, FALSE, 0);
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
      g_object_set_data(G_OBJECT(mainWindow_), "notebook", notebook_);

      auto hbox2 = this->mkVbuttonBox(mainWindow_);  // More precise.  
      Basic::boxPack0(mainBox, GTK_WIDGET(hbox2),  FALSE, FALSE, 0);

      return GTK_WIDGET(mainBox);
    }

public:
    static void clickMenu(GtkButton *widget, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(widget), "menu"));
      gtk_widget_realize(GTK_WIDGET(menu));
      auto gridView_p = (GridView<Type> *)Child::getGridviewObject();
      setupMenu(menu, gridView_p);
      char *basename = Child::getTabname(gridView_p->path());

      bool haveRegexp = gridView_p->regexp() && strlen(gridView_p->regexp());
      auto mark2 = haveRegexp? 
             g_strconcat("<span color=\"blue\">", 
             gridView_p->regexp(),"</span> ", NULL):
             g_strdup("");

      auto markup = g_strconcat(mark2,"<b><span color=\"red\">",basename,
             "</span>", "</b>", NULL);
      gtk_popover_popup(menu);
      auto label = GTK_LABEL(g_object_get_data(G_OBJECT(menu), "titleLabel"));
      gtk_label_set_markup(label, markup);
      g_free(basename);
      g_free(markup);
      g_free(mark2);

    }
private:

    static void setupMenu(GtkPopover *popover, GridView<Type> *gridView_p){
      auto path = gridView_p->path();
      g_object_set_data(G_OBJECT(popover), "gridView_p", gridView_p);
      GridviewMenu<Type> d;
      /*for (auto keys = d.keys(); keys && *keys; keys++){
        auto item = g_object_get_data(G_OBJECT(popover), *keys);
        gtk_widget_set_visible(GTK_WIDGET(item), false);
      }*/
      const char *show[]={ " ", _("Select All"),_("Match regular expression"),
        _("Show"), _("Hidden files"), _("Backup files"), _("Sort mode"), _("Descending"),
        _("Name"), _("Date"), _("Size"), _("File type"), _("Apply modifications"), 
        NULL};

      const char *onlyLocal[] = {_("Add bookmark"), _("Remove bookmark"), _("Paste"), 
        _("Select All"),_("Match regular expression"),
        _("Show"), _("Hidden files"), _("Backup files"), 
        _("Date"), _("Size"),_("File type"),
        NULL};

      auto trash = g_object_get_data(G_OBJECT(popover), _("Empty trash bin"));
      auto trashDir = g_strdup_printf("%s/.local/share/Trash/files", g_get_home_dir());
      gtk_widget_set_visible(GTK_WIDGET(trash), g_file_test(trashDir, G_FILE_TEST_IS_DIR));
      g_free(trashDir);
      
 /*     if (g_object_get_data(G_OBJECT(store), "xffm::fstab")){
        for (auto p = onlyLocal; p && *p; p++){
          auto widget = g_object_get_data(G_OBJECT(popover), *p);
          if (widget) gtk_widget_set_visible(GTK_WIDGET(widget), false);      
        }
        for (auto p = fstabHide; p && *p; p++){
          auto widget = g_object_get_data(G_OBJECT(popover), *p);
          if (widget) gtk_widget_set_visible(GTK_WIDGET(widget), false);      
        }
      }*/

      

      
      auto addB = g_object_get_data(G_OBJECT(popover), _("Add bookmark"));
      auto removeB = g_object_get_data(G_OBJECT(popover), _("Remove bookmark"));
      auto paste = g_object_get_data(G_OBJECT(popover), _("Paste"));
      //auto nopaste = g_object_get_data(G_OBJECT(popover), _("Clipboard is empty."));
      //gtk_widget_set_visible(GTK_WIDGET(nopaste), false);
      auto c = (clipboard_t *)g_object_get_data(G_OBJECT(Child::mainWidget()), "ClipBoard");
      gtk_widget_set_visible(GTK_WIDGET(paste), c->validClipBoard());
      
      if (removeB) gtk_widget_set_visible(GTK_WIDGET(removeB), Bookmarks::isBookmarked(path));
      if (addB) gtk_widget_set_visible(GTK_WIDGET(addB), !Bookmarks::isBookmarked(path));
      for (auto p=show; p && *p; p++){
        TRACE("show visible: %s\n", *p);
        auto widget = g_object_get_data(G_OBJECT(popover), *p);
        if (widget){
          //TRACE("show %s:%p\n", *p, widget);
          gtk_widget_set_visible(GTK_WIDGET(widget), true);
        } else {
          
          TRACE("* Warning: cannot find widget \"%s\" to show.\n", *p);
        }
      }
      auto configFlags = Settings::getInteger(gridView_p->path(), "flags", 0x40);
      auto apply = g_object_get_data(G_OBJECT(popover), _("Apply modifications"));
      gtk_widget_set_sensitive(GTK_WIDGET(apply), configFlags != gridView_p->flags());

      GtkWidget *widget;
      //auto flags = gridView_p->flags();
      const char *checks[]={_("Hidden files"), _("Backup files"),_("Descending"), _("Date"), _("Size"), _("File type"), _("Name"), NULL};   
      int bits[]={ 0x01, 0x02, 0x04, 0x08, 0x10, 0x20,0x40,0};
      for (int i=0; checks[i] != NULL; i++){
        int bit = bits[i];
        int status = bit & gridView_p->flags();
        widget = GTK_WIDGET(g_object_get_data(G_OBJECT(popover), checks[i]));
        TRACE("checks1: %s widget=%p\n", checks[i], widget);
        if (gridView_p->flags() == 0 && i==6) status = true;
        gtk_check_button_set_active(GTK_CHECK_BUTTON(widget), status);
        TRACE("checks2: %s widget=%p\n", checks[i], widget);
      }

      auto store = gridView_p->store();
      TRACE("xffm::root\n");
      if (g_object_get_data(G_OBJECT(store), "xffm::root")){
        for (auto p = onlyLocal; p && *p; p++){
          auto widget = g_object_get_data(G_OBJECT(popover), *p);
          if (widget) gtk_widget_set_visible(GTK_WIDGET(widget), false);      
        }
        return;
      }
      TRACE("xffm::fstab\n");
      if (g_object_get_data(G_OBJECT(store), "xffm::fstab")){
        for (auto p = onlyLocal; p && *p; p++){
          auto widget = g_object_get_data(G_OBJECT(popover), *p);
          if (widget) gtk_widget_set_visible(GTK_WIDGET(widget), false);      
        }
        return;
      }
      TRACE("return\n");

      
    }
};



}
#endif
