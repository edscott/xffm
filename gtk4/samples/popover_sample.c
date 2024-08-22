/* 
 * Build:
 * gcc `pkg-config --cflags gtk+-3.0` popover_sample.c -o popover_sample `pkg-config --libs gtk+-3.0`
 * gcc `pkg-config --cflags gtk4` popover_sample.c -o popover_sample `pkg-config --libs gtk4`
 */

#include <gtk/gtk.h>
// No gtkApp: keep it simple.

static void
activate(GtkWidget *self, gpointer data) { 
  char *string = data;
  fprintf(stderr,"activate: %s\n", string);
  GtkPopover *menu = g_object_get_data(G_OBJECT(self), "menu");
  gtk_popover_popdown(menu);
  if (strcmp(string, "quit")==0){
    fprintf(stderr,"goodbye.\n");
    GtkWindow *window = g_object_get_data(G_OBJECT(menu), "window");
    if (window) gtk_window_destroy(window);
    else fprintf(stderr, "activate():: programming error\n");
  }
  return;
}

static GtkWidget *mkMenu(const gchar **items, GCallback *callback, void **data){
  GtkWidget *vbox;
  GtkWidget *menu = gtk_popover_new ();
  gtk_popover_set_has_arrow(GTK_POPOVER(menu), FALSE);
  gtk_widget_add_css_class (GTK_WIDGET(menu), "inquire" );
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

  GCallback *q = callback;
  void **r = data;
  for (const gchar **p=items; p && *p; p++, q++, r++){
    GtkWidget *item = gtk_button_new_with_label(*p);
    gtk_button_set_has_frame(GTK_BUTTON(item), FALSE);
    gtk_box_append (GTK_BOX (vbox), item);
    g_signal_connect (G_OBJECT (item), "clicked", *q, *r);
    g_object_set_data(G_OBJECT(item), "menu", menu);
  }
      
  gtk_popover_set_child (GTK_POPOVER (menu), vbox);
  return menu;

}

static void 
init(void){
  gtk_init ();
  GtkCssProvider *css_provider = gtk_css_provider_new();
  gtk_css_provider_load_from_string (css_provider, 
  "\
  .inquire {\
    background-color: #00ff00;\
    color: #000000;\
  }\
  .main {\
    background-color: #ff0000;\
    color: #00ff00;\
  }\
  ");
  gtk_style_context_add_provider_for_display(gdk_display_get_default(),GTK_STYLE_PROVIDER(css_provider),GTK_STYLE_PROVIDER_PRIORITY_USER); 
  return;
}

static GtkWidget *
mkButton(const char *iconName){
  GtkWidget *button = gtk_menu_button_new();
  gtk_menu_button_set_icon_name(GTK_MENU_BUTTON(button), iconName);
  gtk_widget_set_valign (button, GTK_ALIGN_CENTER);
  gtk_widget_set_halign (button, GTK_ALIGN_CENTER);
  return button;
}

int main (int argc, char *argv[])
{
  init ();
    
  GtkWidget *window = gtk_window_new ();
  gtk_window_set_default_size (GTK_WINDOW (window), 640, 480);
  gtk_widget_add_css_class (GTK_WIDGET(window), "main" );
  
  GtkWidget *button = mkButton("open-menu-symbolic");
  gtk_window_set_child(GTK_WINDOW(window), button);

  const gchar *items[]={"Hello World 1","Hello World 2","Hello World 3","quit",NULL};
  GCallback callbacks[]={G_CALLBACK(activate), G_CALLBACK(activate), G_CALLBACK(activate), G_CALLBACK(activate), NULL};
  void *data[]={(void *)"test1", (void *)"test12", (void *)"test123", (void *)"quit", NULL};
  GtkWidget *menu = mkMenu(items, callbacks, data);
  g_object_set_data(G_OBJECT(menu), "window", window);
  
  gtk_menu_button_set_popover (GTK_MENU_BUTTON (button), menu);

  gtk_window_present (GTK_WINDOW(window));
  //        gtk_popover_popup(GTK_POPOVER(menu));// crash

  while (g_list_model_get_n_items (gtk_window_get_toplevels ()) > 0)
    g_main_context_iteration (NULL, TRUE);

  return 0;
}

