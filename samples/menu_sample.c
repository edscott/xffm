/* Obsolete now, because menu class will now become a class template with GTK type (gtk menu button or simple widget)
 * Build:
 * c++ -ggdb `pkg-config --cflags gtk4` menu_sample.c -o menu_sample `pkg-config --libs gtk4`
 */

#include <gtk/gtk.h>
#include "../xffm/menu.hh"
GtkWidget *MainWidget;
namespace xf {
  class ExampleMenuClass {
    public:
    const char **keys(void){
      static const char *keys_[] = { // Order is important.
        ("Open in New Tab"), 
        ("Open in New Window"),  
        ("Edit"), 
        ("Copy"), 
        ("Cut"), 
        ("Paste"), 
        ("Delete"), 
        ("Selection"), 
        ("Select All"), 
        ("Match regular expression"), 
        ("Close"),   
        NULL     
      };
      return keys_;
    }
    MenuInfo_t *iconNames(void){
      static MenuInfo_t menuIconNames_[] = { // Need not be complete with regards to keys_.
        {("Edit"), (void *)"application-certificate"},
        {("New"), (void *)"text-x-preview"},
        {("Open in New Tab"),  (void *)"image-x-generic"},
        {NULL, NULL}
      }; 
      return menuIconNames_;
    }
    MenuInfo_t *callbacks(void){
      static MenuInfo_t menuCallbacks_[] = { // Need not be complete with regards to keys_.
        {("New"), (void *)callback1},
        {("Open in New Tab"),  (void *)callback2},
        {("Open in New Window"),  (void *)callback1},
        {("Copy"),  (void *)callback2},
        {("Cut"),  (void *)callback1},
        {("Paste"),  (void *)callback2},
        {("Delete"),  (void *)callback1},
        {("Select All"),  (void *)callback2},
        {("Match regular expression"),  (void *)callback1},
        {("Close"),  (void *)close},     
        {NULL, NULL}
      };
      return menuCallbacks_;
    }
    MenuInfo_t *data(void){
      static MenuInfo_t menuData_[] = { // Need not be complete with regards to keys_ nor menuCallbacks_.
        {("Open in New Tab"), GINT_TO_POINTER(1)},
        {("Open in New Window"),  GINT_TO_POINTER(2)},
        {("New"),  GINT_TO_POINTER(3)},
        {("Copy"),  GINT_TO_POINTER(4)},
        {NULL, NULL}
      };
      return menuData_;
    }
    static void
    callback1(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu"));
      gtk_popover_popdown(menu);
      // Do your thing.
      auto key = (const char *)g_object_get_data(G_OBJECT(button), "key");
      fprintf(stderr, "callback1:: key=%s  data=%p\n", key, data);
    }
    static void
    callback2(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu"));
      gtk_popover_popdown(menu);
      // Do your thing.
      auto key = (const char *)g_object_get_data(G_OBJECT(button), "key");
      fprintf(stderr, "callback2:: key=%s  data=%p\n", key, data);
    }
    static void
    close(GtkButton *button, void *data){
      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu"));
      gtk_popover_popdown(menu);
      // Do your thing.
      exit(0);
    }
    private:

  };
}

static GtkMenuButton *
mkMenuButton(const char *iconName){
  GtkMenuButton *button = GTK_MENU_BUTTON(gtk_menu_button_new());
  gtk_menu_button_set_icon_name(button, iconName);
  gtk_widget_set_valign (GTK_WIDGET(button), GTK_ALIGN_CENTER);
  gtk_widget_set_halign (GTK_WIDGET(button), GTK_ALIGN_CENTER);
  return button;
}

int main (int argc, char *argv[])
{
  gtk_init ();
    
  MainWidget = gtk_window_new ();
  GtkMenuButton *button = mkMenuButton("open-menu-symbolic");
  gtk_window_set_child(GTK_WINDOW(MainWidget), GTK_WIDGET(button));  

  //For defining a menu for menu button
  auto myMenu = new xf::Menu<xf::ExampleMenuClass>("Example menu");
  myMenu->setMenu(button);
  // For defining a menu for ordinary widget, parent and default widget may be the same.
  //myMenu->setMenu(widget, parent);
  delete(myMenu);
      
  // Important when not a  gtk_menu_button but widget with gesture callback:
  // gtk_popover_set_default_widget(menu, button);
  // gtk_widget_set_parent(GTK_WIDGET(menu), button);
  //g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK(activate), NULL);
  gtk_window_present (GTK_WINDOW(MainWidget));

  while (g_list_model_get_n_items (gtk_window_get_toplevels ()) > 0)
    g_main_context_iteration (NULL, TRUE);

  return 0;
}
