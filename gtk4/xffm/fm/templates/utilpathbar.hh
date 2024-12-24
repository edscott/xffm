#ifndef UTILPATHBAR_HH
#define UTILPATHBAR_HH
namespace xf {
  
  
  template <class Type>
  class UtilPathbar{
    public:
    ///////////////////   pathbar  ///////////////////////////////////
    static void 
    updatePathbar(bool updateHistory, void *pathbar_go_f){
        TRACE("Utilpathbar:: updatePathbar1\n");
        const gchar *path = Child::getWorkdir();
        GtkBox *pathbar = Child::getPathbar();
        auto gridView_p = Child::getGridviewObject();
        g_object_set_data(G_OBJECT(pathbar), "gridView_p", gridView_p);
        BasicPathbar::updatePathbar(path, pathbar, updateHistory, pathbar_go_f);
    }
    static void 
    updatePathbar(const gchar *path, GtkBox *pathbar, bool updateHistory, void *pathbar_go_f){
        TRACE("Utilpathbar:: updatePathbar2\n");
        auto gridView_p = Child::getGridviewObject();
        g_object_set_data(G_OBJECT(pathbar), "gridView_p", gridView_p);
        BasicPathbar::updatePathbar(path, pathbar, updateHistory, pathbar_go_f);
    }
  };
}
#endif

