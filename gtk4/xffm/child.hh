#ifndef CHILD_HH
#define CHILD_HH
static GHashTable *childHash = NULL;
namespace xf {
  class Child {
    public:
    static void 
    add(GtkWidget *child){
      if (!childHash) childHash = g_hash_table_new(g_direct_hash, g_direct_equal);
      g_hash_table_insert(childHash, child, GINT_TO_POINTER(1));
    }
    static void 
    remove(GtkWidget *child){
      if (!childHash) childHash = g_hash_table_new(g_direct_hash, g_direct_equal);
      if (g_hash_table_lookup(childHash, child)){
        g_hash_table_remove(childHash, child);
      }
    }
    static bool
    valid(GtkWidget *child){
      return ((bool) g_hash_table_lookup(childHash,child));
    }


    static void setGridview(GtkWidget *view){
      auto child =  Child::getCurrentChild();
      auto gridScrolledWindow = getGridScrolledWindow();
      gtk_scrolled_window_set_child(gridScrolledWindow, view);
      g_object_set_data(G_OBJECT(child), "gridview", view);
      g_object_set_data(G_OBJECT(view), "child", child);
    }

    static void *getGridview(void){
      auto child =  Child::getCurrentChild();
      return g_object_get_data(G_OBJECT(child), "gridview");
    }

    static void *getGridview(GtkWidget *child){
      return g_object_get_data(G_OBJECT(child), "gridview");
    }

    static const int getSerial(void){
      auto child =  Child::getCurrentChild();
      return getSerial(child);
    }

    static const int getSerial(GtkWidget *child){
      return GPOINTER_TO_INT(g_object_get_data(G_OBJECT(child), "serial"));
    }

    static void incrementSerial(void){
      auto child =  Child::getCurrentChild();
      auto serial = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(child), "serial"));
      g_object_set_data(G_OBJECT(child), "serial", GINT_TO_POINTER(++serial));
    }

    static const gchar *getWorkdir(GtkWidget *child){
      TRACE("getWorkdir...\n");
      if (!MainWidget) return NULL;
      return (const gchar *)g_object_get_data(G_OBJECT(child), "path");
    }
    static const gchar *getWorkdir(void){
      auto child =  Child::getCurrentChild();
      return getWorkdir(child);
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
    static GtkTextView *getCurrentInput(void){
      auto child = getCurrentChild();
      return GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(child), "input"));
    }
    static GtkTextView *getCurrentTextView(void){ // deprecated
      return getCurrentOutput();
    }
    static GtkTextView *getCurrentOutput(void){
      auto child = getCurrentChild();
      return GTK_TEXT_VIEW(getOutput(child));
    }
    static GtkTextView *getOutput(GtkWidget *child){
      return GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(child), "output"));
    }
    static GtkPaned *getCurrentPane(void){
      auto child = getCurrentChild();
      auto vpane = GTK_PANED(g_object_get_data(G_OBJECT(child), "vpane"));
      return vpane;
    }

    static GtkBox *getCurrentButtonSpace(void){
      auto child = getCurrentChild();
      return GTK_BOX(getButtonSpace(child));
    }

    static GtkBox *getButtonSpace(GtkWidget *child){
      return GTK_BOX(g_object_get_data(G_OBJECT(child), "buttonSpace"));
    }

    static GtkWidget *getCurrentChild(void){
      //DBG("getCurrentChild...\n");
      if (!MainWidget) return NULL;
      auto notebook = GTK_NOTEBOOK(g_object_get_data(G_OBJECT(MainWidget), "notebook"));
      int num = gtk_notebook_get_current_page(notebook);
      GtkWidget *child = gtk_notebook_get_nth_page (notebook, num);
      return child;
    }
    static GtkBox *getPathbar(void){
      auto child = getCurrentChild();
      return GTK_BOX(g_object_get_data(G_OBJECT(child), "pathbar"));
    }
    static GtkScrolledWindow *getGridScrolledWindow(void){
      auto child = getCurrentChild();
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
  static gchar *
  utf_string (const gchar * t) {
      if(!t) { return g_strdup (""); }
      if(g_utf8_validate (t, -1, NULL)) { return g_strdup (t); }
      /* so we got a non-UTF-8 */
      /* but is it a valid locale string? */
      gchar *actual_tag;
      actual_tag = g_locale_to_utf8 (t, -1, NULL, NULL, NULL);
      if(actual_tag)
          return actual_tag;
      /* So it is not even a valid locale string... 
       * Let us get valid utf-8 caracters then: */
      const gchar *p;
      actual_tag = g_strdup ("");
      for(p = t; *p; p++) {
          // short circuit end of string:
          gchar *r = g_locale_to_utf8 (p, -1, NULL, NULL, NULL);
          if(g_utf8_validate (p, -1, NULL)) {
              gchar *qq = g_strconcat (actual_tag, p, NULL);
              g_free (actual_tag);
              actual_tag = qq;
              break;
          } else if(r) {
              gchar *qq = g_strconcat (actual_tag, r, NULL);
              g_free (r);
              g_free (actual_tag);
              actual_tag = qq;
              break;
          }
          // convert caracter to utf-8 valid.
          gunichar gu = g_utf8_get_char_validated (p, 2);
          if(gu == (gunichar) - 1) {
              gu = g_utf8_get_char_validated ("?", -1);
          }
          gchar outbuf[8];
          memset (outbuf, 0, 8);
          gint outbuf_len = g_unichar_to_utf8 (gu, outbuf);
          if(outbuf_len < 0) {
              ERROR ("utf_string: unichar=%d char =%c outbuf_len=%d\n", gu, p[0], outbuf_len);
          }
          gchar *qq = g_strconcat (actual_tag, outbuf, NULL);
          g_free (actual_tag);
          actual_tag = qq;
      }
      return actual_tag;
  }
    private:
    static     gchar *
    get_terminal_name (const char *path) {
        gchar *iconname;
        if(!path) {
            iconname = utf_string (g_get_host_name());
        } else if(g_path_is_absolute(path) &&
                g_file_test (path, G_FILE_TEST_EXISTS)) {
            gchar *basename = g_path_get_basename (path);
            gchar *pathname = g_strdup (path);
            gchar *b = utf_string (basename);   // non chopped
            chop_excess (pathname);
            gchar *q = utf_string (pathname);   // non chopped

            g_free (basename);
            g_free (pathname);
            //iconname = g_strconcat (display_host, ":  ", b, " (", q, ")", NULL);
            iconname = g_strconcat (b, " (", q, ")", NULL);
            g_free (q);
            g_free (b);
        } else {
            iconname = utf_string (path);
            chop_excess (iconname);
        }

        return (iconname);
    }

  };
} 
#endif
