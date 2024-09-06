#ifndef UTILWORKDIR_HH
#define UTILWORKDIR_HH
#include "fm/pathbarmenu.hh"

#include "texture.hh"
#include "fm/gridview.hh"
namespace xf {
  //template <class GridviewClass, class PathbarClass>
  class Workdir  {
    private:
      static void  updateGridView(const char *path){
        // On creating a new GtkGridView, we send pointer to function to process directory change (gridViewClick).
        TRACE("update updateGridView\n");
        Child::incrementSerial();
        auto view = GridView<LocalDir>::getGridView(path, (void *)gridViewClick);
        Child::setGridview(view);
      }

      static void  updatePathbar(bool addHistory, void *pathbar_go){
        UtilPathbar::updatePathbar(addHistory, pathbar_go);
      }
      static void  updatePathbar(const gchar *path, GtkBox *pathbar, 
          bool addHistory, void *pathbar_go){
        UtilPathbar::updatePathbar(path, pathbar, addHistory, pathbar_go);
      }

    public:
    static const gchar *getWorkdir(GtkWidget *child){
      return Child::getWorkdir(child);
    }
    static const gchar *getWorkdir(void){
      return Child::getWorkdir();
    }

    static bool pleaseWait(void){
      auto size = Thread::threadPoolSize();
      if (size > 0) {
        
        char buffer[4096];
        snprintf(buffer, 4096, " %s\n%s: %d (%s).\n",
              _("There are unfinished jobs: please wait until they are finished."),
              _("Parallel threads:"), size, _("Unfinished Jobs in Queue"));
        TRACE("%s", buffer);
        Print::printInfo(Child::getCurrentOutput(), "emblem-important", g_strdup(buffer));
        return true;
      }
      return false;
    }

    static bool setWorkdir(const gchar *path, GtkWidget *child){
      TRACE("setWorkdir...A\n");
      if (pleaseWait()) return false;
      auto wd = (char *)getWorkdir(child);
      if (strcmp(wd, path)){
        g_free(wd);
        g_object_set_data(G_OBJECT(child), "path", g_strdup(path));
      }
      updatePathbar(false, (void *)pathbar_go);
      updateGridView(path);
      return true;
    }
    static bool setWorkdir(const gchar *path){
      TRACE("setWorkdir...A\n");
      if (pleaseWait()) return false;
      auto child = Child::getCurrentChild();
      auto wd = (gchar *)g_object_get_data(G_OBJECT(child), "path");
      g_free(wd);
      g_object_set_data(G_OBJECT(child), "path", g_strdup(path));
      updatePathbar(true, (void *)pathbar_go);
      updateGridView(path);
      return true;
    }
    static bool setWorkdir(const gchar *path, bool updateHistory){
      TRACE("setWorkdir B...path=%s\n",path);
      if (pleaseWait()) return false;
      auto child = Child::getCurrentChild();
      auto wd = (gchar *)g_object_get_data(G_OBJECT(child), "path");
      g_free(wd);
      g_object_set_data(G_OBJECT(child), "path", g_strdup(path));
      updatePathbar(updateHistory, (void *)pathbar_go);
      updateGridView(path);
      return true;
    }
    static bool setWorkdir(const gchar *path, GtkBox *pathbar, bool updateHistory){
      TRACE("setWorkdir...C\n");
      if (pleaseWait()) return false;
      if (!MainWidget) return false;
      auto child = GTK_WIDGET(g_object_get_data(G_OBJECT(pathbar), "child"));
      auto wd = (gchar *)g_object_get_data(G_OBJECT(child), "path");
      g_free(wd);
      g_object_set_data(G_OBJECT(child), "path", g_strdup(path));
      UtilBasic::setWindowTitle(child);
      updatePathbar(path, pathbar, updateHistory, (void *)pathbar_go);
      updateGridView(path);
      return true;
    }

    static gboolean
    gridViewClick(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer object){

      auto eventController = GTK_EVENT_CONTROLLER(self);
      auto event = gtk_event_controller_get_current_event(eventController);
      
      auto modType = gdk_event_get_modifier_state(event);

      TRACE("modType = 0x%x\n", modType);
      if (modType & GDK_CONTROL_MASK) return FALSE;
      if (modType & GDK_SHIFT_MASK) return FALSE;
      
      TRACE("gestureClick; object=%p button=%d\n", object,
          gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(self)));

      auto info = G_FILE_INFO(gtk_list_item_get_item(GTK_LIST_ITEM(object)));
      auto file = G_FILE(g_file_info_get_attribute_object (info, "standard::file"));
      auto root = g_file_info_get_attribute_object (info, "xffm::root");
      if (root){
        setWorkdir("xffm:root");
        return TRUE;
      }
      TRACE("gestureClick; file=%p\n", file);
      auto path = g_file_get_path(file);
      TRACE("gestureClick; path=%p\n", path);
      TRACE("click on %s\n", path);
      auto type = g_file_info_get_file_type(info);
      if ((type == G_FILE_TYPE_DIRECTORY )||(LocalDir::symlinkToDir(info, type))) {
        TRACE("Go to action...\n");
        auto child = UtilBasic::getCurrentChild();
        setWorkdir(path);
      } else {
        DBG("mimetype action...\n");
        new OpenWith<bool>(GTK_WINDOW(MainWidget), path);
      }
      g_free(path);
      return TRUE;
    }

    static gboolean
    pathbar_go (
              GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer data ) 
    {
        auto pathbar = GTK_BOX(data);
        auto eventBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
        auto name = (char *) g_object_get_data(G_OBJECT(eventBox), "name");
        auto path = (char *) g_object_get_data(G_OBJECT(eventBox), "path");
        auto button = gtk_gesture_single_get_button(GTK_GESTURE_SINGLE(self));
          TRACE("pathbar goto... name=%s, path=%s\n", name, path);
        if (button == 1){
          TRACE("pathbar goto...\n");
          //if (strcmp(path, "xffm:root")==0) setWorkdir(g_get_home_dir(), pathbar, true);
          //else setWorkdir(path, pathbar, true);
          setWorkdir(path, pathbar, true);
          return TRUE;
        }
        if (button == 3){
          DBG("workdir.hh:: FIXME pathbar menu...use menu class template\n");
          DBG("workdir.hh:: path=%s, name=%s\n", path, name);
          void *object = g_object_get_data(G_OBJECT(pathbar), "menu");
          if (!object){

            DBG("***Error: g_object_get_data(G_OBJECT(pathbar), \"menu\") == NULL\n");
            // fucking popovers... this will crash app.
/*
            GtkPopover *menu = GTK_POPOVER(gtk_popover_new());
            g_object_set_data(G_OBJECT(pathbar), "menu", menu);
            auto label = gtk_label_new("foo bar");
            gtk_popover_set_child(menu, label);
            gtk_popover_set_default_widget(menu, GTK_WIDGET(pathbar));
            gtk_widget_set_parent(GTK_WIDGET(menu), GTK_WIDGET(pathbar));
*/

            /*auto myPathbarMenu = new Menu<PathbarMenu>;
            auto title = g_strconcat("<span color=\"blue\">", _("Navigation Toolbar"), "</span>", NULL);
            auto pathbarMenu = myPathbarMenu->getMenu(title);
            g_free(title);
            g_object_set_data(G_OBJECT(pathbar), "menu", pathbarMenu);
            // Important: must use both of the following instructions:
            gtk_popover_set_default_widget(pathbarMenu, GTK_WIDGET(pathbar));
            gtk_widget_set_parent(GTK_WIDGET(pathbarMenu), GTK_WIDGET(pathbar));
            delete myPathbarMenu; 
            g_object_set_data(G_OBJECT(pathbar), "menu", pathbarMenu);*/
            
          } else
          {
            auto pathbarMenu = GTK_POPOVER(g_object_get_data(G_OBJECT(pathbar), "menu"));
            Menu<PathbarMenu>::setTitle(pathbarMenu, path);
            gtk_popover_popup(pathbarMenu);
          }
              

              
          return TRUE;
        }
        //TRACE("pathbar_go...name=%s, path=%s button=%d\n", name, path, button);
        return FALSE;
        /*
        
        if (event->button == 1) {
            pathbar_p->pathbar_ok(eventBox);
        }

        if (event->button == 3) {
            auto view = pathbar_p->pathbarView();
            const gchar *path = pathbar_p->getClickPath(eventBox);
            TRACE("***clickpath=%s\n", path);
            GtkMenu *menu = NULL;
            if (g_file_test(path, G_FILE_TEST_IS_DIR)){ 
                menu = LocalPopUp<Type>::popUp();
                Popup<Type>::setWidgetData(menu, "path", path);
                g_object_set_data(G_OBJECT(menu),"view", NULL);
                BaseSignals<Type>::configureViewMenu(LOCALVIEW_TYPE);
            } else {
                // do xffm:root menu
                RootPopUp<Type>::resetPopup();
                menu = RootPopUp<Type>::popUp();
            }
            if (menu) {
                gtk_menu_popup_at_pointer (menu, (const GdkEvent *)event);
            }          
        }

        return FALSE;*/
    }
   

  };
}
#endif
