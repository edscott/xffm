/* 
 * Build:
 * gcc `pkg-config --cflags gtk4` color_sample.c -o color_sample `pkg-config --libs gtk4`
 */

#include <gtk/gtk.h>
GtkWidget *MainWidget;
static void
setColor(GObject *source_object, GAsyncResult *res, gpointer data){
  GtkColorDialog *dialog =(GtkColorDialog*)source_object;
  GError *error_=NULL;
  GdkRGBA *color = gtk_color_dialog_choose_rgba_finish (dialog, res, &error_);
  fprintf(stderr,"setColor: r=%f, g=%f, b=%f, a=%f\n",
      color->red, color->green, color->blue, color->alpha);
  g_free(color);
}

static void
activate(GtkWidget *self, gpointer data) { 
  char *string = data;
  fprintf(stderr,"clicked\n");
  GtkColorDialog *dialog = gtk_color_dialog_new();
  gtk_color_dialog_set_modal (dialog, TRUE);
  gtk_color_dialog_choose_rgba (dialog, GTK_WINDOW(MainWidget), 
      NULL, NULL, setColor, NULL);

  return;
}

static GtkWidget *
mkButton(const char *iconName){
  GtkWidget *button = gtk_button_new();
  gtk_button_set_icon_name(GTK_BUTTON(button), iconName);
  gtk_widget_set_valign (button, GTK_ALIGN_CENTER);
  gtk_widget_set_halign (button, GTK_ALIGN_CENTER);
  return button;
}

int main (int argc, char *argv[])
{
  gtk_init ();
    
  MainWidget = gtk_window_new ();
  GtkWidget *button = mkButton("open-menu-symbolic");
  gtk_window_set_child(GTK_WINDOW(MainWidget), button);

   
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK(activate), NULL);

  gtk_window_present (GTK_WINDOW(MainWidget));

  while (g_list_model_get_n_items (gtk_window_get_toplevels ()) > 0)
    g_main_context_iteration (NULL, TRUE);

  return 0;
}

