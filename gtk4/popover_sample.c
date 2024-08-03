/* 
 * Build:
 * gcc `pkg-config --cflags gtk+-3.0` popover_sample.c -o popover_sample `pkg-config --libs gtk+-3.0`
 * gcc `pkg-config --cflags gtk4` popover_sample.c -o popover_sample `pkg-config --libs gtk4`
 */

#include <gtk/gtk.h>

int main (int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *button, *popover, *vbox;

  gtk_init ();

  window = gtk_window_new ();
  //gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  //gtk_container_set_border_width (GTK_CONTAINER (window), 5);
  gtk_window_set_default_size (GTK_WINDOW (window), 640, 480);

  button = gtk_menu_button_new();
  gtk_menu_button_set_icon_name(GTK_MENU_BUTTON(button), "open-menu-symbolic");
  gtk_widget_set_valign (button, GTK_ALIGN_CENTER);
  gtk_widget_set_halign (button, GTK_ALIGN_CENTER);
  //gtk_container_add (GTK_CONTAINER (window), button);
  gtk_window_set_child(GTK_WINDOW(window), button);
  
      GMenu *menuModel = g_menu_new(); 
      GMenuItem *item;
      const gchar *items[]={"Hello World 1","Hello World 2","Hello World 3",NULL};
      for (const gchar **p=items; p && *p; p++){
        item = g_menu_item_new (*p, NULL);
        g_menu_append_item(menuModel, item);
      }
      popover = gtk_popover_menu_new_from_model(G_MENU_MODEL(menuModel));

      gtk_menu_button_set_popover (GTK_MENU_BUTTON (button), popover);

/*  gtk_popover_menu_add_child (GTK_POPOVER_MENU(popover), gtk_label_new ("Hello World 1"), "Hello World 1");
  gtk_popover_menu_add_child (GTK_POPOVER_MENU(popover), gtk_label_new ("Hello World 2"), "Hello World 2");
  gtk_popover_menu_add_child (GTK_POPOVER_MENU(popover), gtk_label_new ("Hello World 3"), "Hello World 3");*/

/*  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_append (GTK_BOX (vbox), gtk_label_new ("Hello World 1"));
  gtk_box_append (GTK_BOX (vbox), gtk_label_new ("Hello World 2"));
  gtk_box_append (GTK_BOX (vbox), gtk_label_new ("Hello World 3"));

  popover = gtk_popover_new (button);
  gtk_container_add (GTK_CONTAINER (popover), vbox);
  gtk_menu_button_set_popover (GTK_MENU_BUTTON (button), popover);*/


  gtk_window_present (GTK_WINDOW(window));

//  gtk_widget_show_all (window);

  while (g_list_model_get_n_items (gtk_window_get_toplevels ()) > 0)
    g_main_context_iteration (NULL, TRUE);

  return 0;
}

