#ifndef CHILD_HH
#define CHILD_HH
static GHashTable *gridViewHash = NULL;
static GHashTable *childHash = NULL;
static GHashTable *monitorHash = NULL;
static pthread_mutex_t serialMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t childMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t gridViewMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t monitorMutex = PTHREAD_MUTEX_INITIALIZER;

static GtkWidget *_mainWidget = NULL;
namespace xf {
  class Child {
    public:
    
    static void mainWidget(GtkWidget *value){_mainWidget = value;}
    static GtkWidget * mainWidget(void){
      if (!_mainWidget){
        ERROR_("***Child::mainWidget() returns NULL\n");
        exit(1);
      }
      return _mainWidget;
    }
    static GtkWidget * mainWidget0(void){
      return _mainWidget;
    }
      
    static void 
    addMonitor(void *monitor){
      pthread_mutex_lock(&monitorMutex);
      if (!monitorHash) monitorHash = g_hash_table_new(g_direct_hash, g_direct_equal);
      g_hash_table_insert(monitorHash, monitor, GINT_TO_POINTER(1));
      pthread_mutex_unlock(&monitorMutex);
    }
    static void 
    removeMonitor(void *monitor){
      pthread_mutex_lock(&monitorMutex);
      if (!monitorHash) monitorHash = g_hash_table_new(g_direct_hash, g_direct_equal);
      if (g_hash_table_lookup(monitorHash, monitor)){
        g_hash_table_remove(monitorHash, monitor);
      }
      pthread_mutex_unlock(&monitorMutex);
    }   
    static bool
    validMonitor(void *monitor){
      if (!monitor) return false;
      int retval = 0;
      if (!monitorHash) return false;
        
      pthread_mutex_lock(&monitorMutex);
      retval = GPOINTER_TO_INT(g_hash_table_lookup(monitorHash,monitor));
      pthread_mutex_unlock(&monitorMutex);
      return (bool) retval;
    }

    static bool tryLockGridView(void){
      return pthread_mutex_trylock(&gridViewMutex);
    }

    static void lockGridView(const char *m){
      TRACE("lockGridView: %s\n", m);
      lockGridView();
    }

    static void unlockGridView(const char *m){
      TRACE("unlockGridView: %s\n", m);
      unlockGridView();
    }

    static void lockGridView(void){
      pthread_mutex_lock(&gridViewMutex);
    }
    static void unlockGridView(void){
      pthread_mutex_unlock(&gridViewMutex);
    }

    static GList *getGridViewList(void){
      if (!gridViewHash) return NULL;
      return g_hash_table_get_keys(gridViewHash);
    }
    
    static void 
    addGridView(void *gridView_p){
      lockGridView("addGridView");
      if (!gridViewHash) {
        gridViewHash = g_hash_table_new(g_direct_hash, g_direct_equal);
      }
      g_hash_table_insert(gridViewHash, gridView_p, GINT_TO_POINTER(1));
      unlockGridView("addGridView");
    }
    static void 
    removeGridView(void *gridView_p){
      if (!gridView_p) return;
      lockGridView("removeGridView");
      if (!gridViewHash) gridViewHash = g_hash_table_new(g_direct_hash, g_direct_equal);
      if (g_hash_table_lookup(gridViewHash, gridView_p)){
        g_hash_table_remove(gridViewHash, gridView_p);
      }
      unlockGridView("removeGridView");
    } 

    static bool
    validGridView(void *gridView_p){
      // This is main context, any new gridview should also come from main context.
      if (!gridView_p) return false;
      
      int retval = 0;
      if (!gridViewHash) return false;
      // lock must be obtained by calling thread! lockGridView("validGridView");
      retval = GPOINTER_TO_INT(g_hash_table_lookup(gridViewHash,gridView_p));
      // lock must be obtained by calling thread! unlockGridView("validGridView");
      return (bool) retval;
    }

    static void 
    add(GtkWidget *child){
      pthread_mutex_lock(&childMutex);
      if (!childHash) childHash = g_hash_table_new(g_direct_hash, g_direct_equal);
      g_hash_table_insert(childHash, child, GINT_TO_POINTER(1));
      pthread_mutex_unlock(&childMutex);
    }
    static void 
    remove(GtkWidget *child){
      pthread_mutex_lock(&childMutex);
      if (!childHash) childHash = g_hash_table_new(g_direct_hash, g_direct_equal);
      if (g_hash_table_lookup(childHash, child)){
        g_hash_table_remove(childHash, child);
      }
      pthread_mutex_unlock(&childMutex);
    }
    static bool
    valid(GtkWidget *child){
      if (!child) return false;
      pthread_mutex_lock(&childMutex);
      int retval = 0;
      if (childHash) retval = GPOINTER_TO_INT(g_hash_table_lookup(childHash,child));
      pthread_mutex_unlock(&childMutex);
      return (bool) retval;
    }
    static GtkSelectionModel *selection(void){
      auto child =  Child::getChild();
      return selection(child);
    }

    static GtkSelectionModel *selection(GtkWidget *child){
      if (!valid(child)) return NULL; // Page has disappeared.
      auto selection = g_object_get_data(G_OBJECT(child), "selection");
      if (selection) return GTK_SELECTION_MODEL(selection);
      return NULL;
    }

    static void *page(GtkWidget *child){
      return g_object_get_data(G_OBJECT(child), "page");
    }

    static void *page(void){
      return page(getChild());
    }



    static void *getGridviewObject(void){
      auto child =  Child::getChild();
      return (void *)g_object_get_data(G_OBJECT(child), "GridviewObject");
    }

    static void *getGridviewObject(GtkWidget *child){
      if (!valid(child)) return NULL; // Page has disappeared.
      return (void *)g_object_get_data(G_OBJECT(child), "GridviewObject");
    }

    static void setGridviewObject(void *object){
      auto child =  Child::getChild();
      g_object_set_data(G_OBJECT(child), "GridviewObject", object);
    }

    static void setGridview(GtkWidget *view){
      auto child =  Child::getChild();
      auto gridScrolledWindow = getGridScrolledWindow();
      
      // Lets see if this works around the gtk inner race bug...
      auto oldView = gtk_scrolled_window_get_child(gridScrolledWindow);
        if (oldView && G_IS_OBJECT(oldView)){
        auto controller1 = g_object_get_data(G_OBJECT(oldView),"MotionController");
        auto controller2 = g_object_get_data(G_OBJECT(oldView),"ClickView1");
        auto controller3 = g_object_get_data(G_OBJECT(oldView),"ClickView3");
        gtk_event_controller_reset(GTK_EVENT_CONTROLLER(controller1));
        gtk_event_controller_reset(GTK_EVENT_CONTROLLER(controller2));
        gtk_event_controller_reset(GTK_EVENT_CONTROLLER(controller3));
        gtk_widget_remove_controller(oldView, GTK_EVENT_CONTROLLER(controller1));
        gtk_widget_remove_controller(oldView, GTK_EVENT_CONTROLLER(controller2));
        gtk_widget_remove_controller(oldView, GTK_EVENT_CONTROLLER(controller3));
        // Now when oldView is automatically unreffed, it has no controllers. 
        // Maybe we should also do this pre cleanup in the gridview
        // factory, just to be safe.
      } 
      gtk_scrolled_window_set_child(gridScrolledWindow, view);
      g_object_set_data(G_OBJECT(child), "gridview", view);
      g_object_set_data(G_OBJECT(view), "child", child);
    }

    static void *getGridview(void){
      return getGridviewObject();
    }

    static void *getGridview(GtkWidget *child){
      return getGridviewObject(child);
    }

    static const int getSerial(void){
      pthread_mutex_lock(&serialMutex);
      auto child =  Child::getChild();
      if (!valid(child)) return -1; // Page has disappeared.
      auto serial = g_object_get_data(G_OBJECT(child), "serial");
      pthread_mutex_unlock(&serialMutex);
      return GPOINTER_TO_INT(serial);
    }

    static const int getSerial(GtkWidget *child){
      if (!valid(child)) return -1; // Page has disappeared.
      pthread_mutex_lock(&serialMutex);
      auto serial = g_object_get_data(G_OBJECT(child), "serial");
      pthread_mutex_unlock(&serialMutex);

      return GPOINTER_TO_INT(serial);
    }

    static int incrementSerial(GtkWidget *child){
      if (!valid(child)) return -1; // Page has disappeared.
      pthread_mutex_lock(&serialMutex);
      auto serial = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(child), "serial"));
      g_object_set_data(G_OBJECT(child), "serial", GINT_TO_POINTER(++serial));      
      pthread_mutex_unlock(&serialMutex);
      return serial;
    }

    static int incrementSerial(void){
      pthread_mutex_lock(&serialMutex);
      auto child =  Child::getChild();
      auto serial = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(child), "serial"));
      g_object_set_data(G_OBJECT(child), "serial", GINT_TO_POINTER(++serial));
      pthread_mutex_unlock(&serialMutex);
      return serial;
    }

    static const gchar *getWorkdir(GtkWidget *child){
      TRACE("getWorkdir...\n");
      if (!Child::mainWidget()) return NULL;
      if (!child) return g_get_home_dir();
      if (!valid(child)) return g_get_home_dir(); // Page has disappeared.
      return (const gchar *)g_object_get_data(G_OBJECT(child), "path");
    }
    static const gchar *getWorkdir(void){
      auto child =  Child::getChild();
      return getWorkdir(child);
    }
    static void *getPrompt(void){
      auto child =  Child::getChild();
      return g_object_get_data(G_OBJECT(child), "prompt");
    }
    static
    void setWindowTitle(GtkWidget *child){
        auto path = (const char *) g_object_get_data(G_OBJECT(child), "path");
        gchar *gg = get_terminal_name(path);
        auto user = g_get_user_name();
        auto host = g_strdup(g_get_host_name());
        if (strchr(host, '.')) *strchr(host, '.')=0;
        gchar *g = g_strconcat(user,"@",host,":",gg, NULL);
        g_free(host);
        g_free(gg); 
        gtk_window_set_title(GTK_WINDOW(Child::mainWidget()), g);
        g_free(g);
        auto basename = g_path_get_basename(path);
        auto notebook = GTK_NOTEBOOK(g_object_get_data(G_OBJECT(Child::mainWidget()), "notebook"));
        auto tabWidget = gtk_notebook_get_tab_label(notebook, child);
        auto label = GTK_LABEL(g_object_get_data(G_OBJECT(tabWidget), "label"));
        gtk_label_set_markup(label, basename);
        g_free(basename);
    }
    static GtkBox *vButtonBox(void){
      return GTK_BOX(g_object_get_data(G_OBJECT(Child::mainWidget()), "buttonBox"));
    }
    static GtkTextView *getDollar(void){
      auto child = getChild();
      return GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(child), "dollar"));
    }
    static GtkTextView *getInput(void){
      auto child = getChild();
      return GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(child), "input"));
    }
/*    static GtkTextView *getCurrentTextView(void){ // deprecated
      return getCurrentOutput();
    }*/
     
    static GtkTextView *getOutput(GtkWidget *child){
      if (!valid(child)) return NULL; // Page has disappeared.
      return GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(child), "output"));
    }
    
    static GtkTextView *getOutput(){
      auto child = getChild();
      return GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(child), "output"));
    }
    static GtkPaned *getPane(void){
      auto child = getChild();
      auto vpane = GTK_PANED(g_object_get_data(G_OBJECT(child), "vpane"));
      return vpane;
    }

    static GtkBox *getButtonSpace(void){
      auto child = getChild();
      return GTK_BOX(getButtonSpace(child));
    }

    static GtkBox *getButtonSpace(GtkWidget *child){
      if (!valid(child)) return NULL; // Page has disappeared.
      return GTK_BOX(g_object_get_data(G_OBJECT(child), "buttonSpace"));
    }

    static GtkWidget *getChild(void){
      if (!Child::mainWidget()) return NULL;
      auto notebook = GTK_NOTEBOOK(g_object_get_data(G_OBJECT(Child::mainWidget()), "notebook"));
      int num = gtk_notebook_get_current_page(notebook);
      GtkWidget *child = gtk_notebook_get_nth_page (notebook, num);
      return child;
    }
    static GtkBox *getPathbar(void){
      auto child = getChild();
      return GTK_BOX(g_object_get_data(G_OBJECT(child), "pathbar"));
    }
    static GtkScrolledWindow *getGridScrolledWindow(void){
      auto child = getChild();
      return GTK_SCROLLED_WINDOW(g_object_get_data(G_OBJECT(child), "gridScrolledWindow"));
    }


#define MAX_PATH_LABEL 40
#define MAX_PATH_START_LABEL 18
    static const gchar *
    chop_excess (gchar * b) {
        // chop excess length...

        const gchar *home = g_get_home_dir();
        gchar *bb;
        if (strncmp(home, b, strlen(home))==0){
            if (strlen(home) == strlen(b)) return b;
            bb = g_strconcat("~/", b + strlen(home)+1, NULL);
        } else {
            bb = g_strdup(b);
        }
        
        int len = strlen (bb);

        if(len < MAX_PATH_LABEL) {
            strcpy(b, bb);
            g_free(bb);
            return (b);
        }
            
        bb[MAX_PATH_START_LABEL - 3] = 0;

        gchar *g = g_strconcat(bb, "...", b + (len - MAX_PATH_LABEL + MAX_PATH_START_LABEL), NULL);
        strcpy (b, g);
        g_free(bb);
        g_free(g);

        return b;
    }
    private:
    static     gchar *
    get_terminal_name (const char *path) {
        gchar *iconname;
        if(!path) {
            iconname = Basic::utf_string (g_get_host_name());
        } else if(g_path_is_absolute(path) &&
                g_file_test (path, G_FILE_TEST_EXISTS)) {
            gchar *basename = g_path_get_basename (path);
            gchar *pathname = g_strdup (path);
            gchar *b = Basic::utf_string (basename);   // non chopped
            chop_excess (pathname);
            gchar *q = Basic::utf_string (pathname);   // non chopped

            g_free (basename);
            g_free (pathname);
            //iconname = g_strconcat (display_host, ":  ", b, " (", q, ")", NULL);
            iconname = g_strconcat (b, " (", q, ")", NULL);
            g_free (q);
            g_free (b);
        } else {
            iconname = Basic::utf_string (path);
            chop_excess (iconname);
        }

        return (iconname);
    }

  };
} 
#endif
