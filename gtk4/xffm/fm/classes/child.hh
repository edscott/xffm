#ifndef CHILD_HH
#define CHILD_HH
static GHashTable *gridViewHash = NULL;
static GHashTable *childHash = NULL;
static pthread_mutex_t serialMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t childMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t gridViewMutex = PTHREAD_MUTEX_INITIALIZER;
namespace xf {
  class Child {
    public:
    static bool tryLockGridView(void){
      return pthread_mutex_trylock(&gridViewMutex);
    }

    static void lockGridView(void){
      pthread_mutex_lock(&gridViewMutex);
    }
    static void unlockGridView(void){
      pthread_mutex_unlock(&gridViewMutex);
    }
    static void 
    addGridView(void *gridView_p){
      lockGridView();
      if (!gridViewHash) gridViewHash = g_hash_table_new(g_direct_hash, g_direct_equal);
      g_hash_table_insert(gridViewHash, gridView_p, GINT_TO_POINTER(1));
      unlockGridView();
    }
    static void 
    removeGridView(void *gridView_p){
      lockGridView();
      if (!gridViewHash) gridViewHash = g_hash_table_new(g_direct_hash, g_direct_equal);
      if (g_hash_table_lookup(gridViewHash, gridView_p)){
        g_hash_table_remove(gridViewHash, gridView_p);
      }
      unlockGridView();
    }   
    static bool
    validGridView(void *gridView_p){
      if (!gridView_p) return false;
      int retval = 0;
      if (gridViewHash) retval = GPOINTER_TO_INT(g_hash_table_lookup(gridViewHash,gridView_p));
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
      if (!MainWidget) return NULL;
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
        gtk_window_set_title(GTK_WINDOW(MainWidget), g);
        g_free(g);
        auto basename = g_path_get_basename(path);
        auto notebook = GTK_NOTEBOOK(g_object_get_data(G_OBJECT(MainWidget), "notebook"));
        auto tabWidget = gtk_notebook_get_tab_label(notebook, child);
        auto label = GTK_LABEL(g_object_get_data(G_OBJECT(tabWidget), "label"));
        gtk_label_set_markup(label, basename);
        g_free(basename);
    }
    static GtkBox *vButtonBox(void){
      return GTK_BOX(g_object_get_data(G_OBJECT(MainWidget), "buttonBox"));
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
      //DBG("getChild...\n");
      if (!MainWidget) return NULL;
      auto notebook = GTK_NOTEBOOK(g_object_get_data(G_OBJECT(MainWidget), "notebook"));
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
