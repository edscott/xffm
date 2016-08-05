#include <gtk/gtk.h>

/*
GtkIconView Example
gcc -o iconview iconview.c `pkg-config --libs --cflags gtk+-2.0`
Reference: http://library.gnome.org/devel/gtk/stable/GtkIconView.html
*/
enum
{
  COL_DISPLAY_NAME,
  COL_PIXBUF,
  NUM_COLS
};

/* Create the ListStore and fill it with required data */
GtkTreeModel *
create_and_fill_model (void)
{
  GtkListStore *list_store;
  GdkPixbuf *p1, *p2;
  GtkTreeIter iter;
  GError *err = NULL;
  int i = 0;

  p1 = gdk_pixbuf_new_from_file ("image1.png", &err);
                            /* No error checking is done here */
  p2 = gdk_pixbuf_new_from_file ("image2.png", &err);
   
  list_store = gtk_list_store_new (NUM_COLS, G_TYPE_STRING, GDK_TYPE_PIXBUF);

  do {
    gtk_list_store_append (list_store, &iter);
    gtk_list_store_set (list_store, &iter, COL_DISPLAY_NAME, "Image1",
                        COL_PIXBUF, p1, -1);
    gtk_list_store_append (list_store, &iter);
    gtk_list_store_set (list_store, &iter, COL_DISPLAY_NAME, "Image2",
                        COL_PIXBUF, p2, -1);
  } while (i++ < 100);

  return GTK_TREE_MODEL (list_store);
}

int main (int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *icon_view;
  GtkWidget *scrolled_window;
   
  gtk_init (&argc, &argv);
   
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
  gtk_window_set_title (GTK_WINDOW (window), "Simple Icon View Example");
  gtk_container_set_border_width (GTK_CONTAINER (window), 10);
  gtk_widget_set_size_request (window, 600, 400);
   
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (window), scrolled_window);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window),
                                       GTK_SHADOW_IN);

  icon_view = gtk_icon_view_new_with_model (create_and_fill_model ());
  gtk_container_add (GTK_CONTAINER (scrolled_window), icon_view);
   
  gtk_icon_view_set_text_column (GTK_ICON_VIEW (icon_view),
                                 COL_DISPLAY_NAME);
  gtk_icon_view_set_pixbuf_column (GTK_ICON_VIEW (icon_view), COL_PIXBUF);
  gtk_icon_view_set_selection_mode (GTK_ICON_VIEW (icon_view),
                                    GTK_SELECTION_MULTIPLE);
   
  gtk_widget_show_all (scrolled_window);
  gtk_widget_show (window);
   
  gtk_main ();
   
  return 0;
}
