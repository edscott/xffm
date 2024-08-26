#ifndef CHILD_HH
#define CHILD_HH
namespace xf {
  class Child {
    public:
    static GtkBox *vButtonBox(void){
      return GTK_BOX(g_object_get_data(G_OBJECT(MainWidget), "buttonBox"));
    }
    static GtkTextView *getCurrentInput(void){
      auto child = getCurrentChild();
      return GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(child), "input"));
    }
    static GtkTextView *getCurrentTextView(void){
      auto child = getCurrentChild();
      return GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(child), "output"));
    }
    static GtkPaned *getCurrentPane(void){
      auto child = getCurrentChild();
      auto vpane = GTK_PANED(g_object_get_data(G_OBJECT(child), "vpane"));
      return vpane;
    }

    static GtkBox *getCurrentButtonSpace(void){
      auto child = getCurrentChild();
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

  };
} 
#endif
